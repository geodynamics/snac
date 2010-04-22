/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003,
**	Pururav Thoutireddy,
**	Californian Institute of Technology, 1200 East California Boulevard, Pasadena, California, 91125, USA.
**      110 Victoria Street, Melbourne, Victoria, 3053, Australia.
**	University of Texas, 1 University Station, Austin, Texas, 78712, USA.
**
** Authors:
**	Pururav Thoutireddy, Staff Scientist, Caltech
**      Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
**	Stevan M. Quenette, Visitor in Geophysics, Caltech.
**	Luc Lavier, Research Scientist, The University of Texas. (luc@utig.ug.utexas.edu)
**	Luc Lavier, Research Scientist, Caltech.
**
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2, or (at your option) any
** later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**
** $Id: RemeshNodes.c 3259 2006-11-09 20:06:31Z EunseoChoi $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "EntryPoint.h"
#include "Mesh.h"
#include "Context.h"
#include "Remesh.h"
#include "Register.h"
#include "Utils.h"

#include <string.h>
#include <assert.h>
#include <float.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_linalg.h>


void _SnacRemesher_RecoverNodes( void* _context ) {
	Snac_Context*			context = (Snac_Context*)_context;
	SnacRemesher_Context*	contextExt = ExtensionManager_Get( context->extensionMgr,
															   context,
															   SnacRemesher_ContextHandle );
	Mesh*					mesh = context->mesh;
	Node_LocalIndex			newNode_i;

	/*
	** Populate arrays for recovered fields using the SPR method.
	*/
	for( newNode_i = 0; newNode_i < mesh->nodeLocalCount; newNode_i++ )
		SnacRemesher_RecoverNode( context, contextExt, newNode_i );
}


void _SnacRemesher_InterpolateNodes( void* _context ) {
	Snac_Context*			context = (Snac_Context*)_context;
	SnacRemesher_Context*	contextExt = ExtensionManager_Get( context->extensionMgr,
															   context,
															   SnacRemesher_ContextHandle );
	Mesh*					mesh = context->mesh;
	SnacRemesher_Mesh*		meshExt = ExtensionManager_Get( context->meshExtensionMgr,
															mesh,
															SnacRemesher_MeshHandle );
	NodeLayout*				nLayout = mesh->layout->nodeLayout;
	Node_LocalIndex			newNode_i;
	IndexSet*				extNodes;

	void interpolateNode( void* _context, Node_LocalIndex newNodeInd, Element_DomainIndex dEltInd );

	/*
	** Free any owned arrays that may still exist from the last node interpolation.
	*/

	FreeArray( meshExt->externalNodes );
	meshExt->nExternalNodes = 0;

	/*
	** Scoot over all the new nodes and find the old element in which each one resides, then interpolate.
	*/

	/* Create an index set for storing any external nodes. */
	extNodes = IndexSet_New( mesh->nodeLocalCount );

	for( newNode_i = 0; newNode_i < mesh->nodeLocalCount; newNode_i++ ) {
		Node_LocalIndex		dNodeInd;
		unsigned				nElements;
		Element_DomainIndex*	elements;
		Coord				newPoint;
		unsigned				elt_i;

		/* Extract the new node's coordinate. */
		Vector_Set( newPoint, meshExt->newNodeCoords[newNode_i] );

		/* Find the closest old node. */
		dNodeInd = findClosestNode( context, newPoint, newNode_i );

		/* Grab incident elements. */
		{
			Node_GlobalIndex	gNodeInd;

			gNodeInd = Mesh_NodeMapDomainToGlobal( mesh, dNodeInd );
			nElements = nLayout->nodeElementCount( nLayout, gNodeInd );
			if( nElements ) {
				elements = Memory_Alloc_Array( Element_DomainIndex, nElements, "SnacRemesher" );
				nLayout->buildNodeElements( nLayout, gNodeInd, elements );
			}
			else {
				elements = NULL;
			}
		}

		/* Convert global element indices to domain. */
		for( elt_i = 0; elt_i < nElements; elt_i++ ) {
			elements[elt_i] = Mesh_ElementMapGlobalToDomain( mesh, elements[elt_i] );
		}

		/* Which of the incident elements contains the node? */
		for( elt_i = 0; elt_i < nElements; elt_i++ ) {
			if( elements[elt_i] >= mesh->elementDomainCount ) {
				continue;
			}

			if( pointInElement( context, newPoint, elements[elt_i] ) ) {
				break;
			}
		}

		/* Did we find the element? */
		if( elt_i < nElements ) {
			/* If so, call a function to locate the tetrahedra and interpolate. */
			interpolateNode( context, newNode_i, elements[elt_i] );
		}
		else {
			/* If not, then the new node finds itself outside the old mesh.  In this scenario, we cannot interpolate
			   the nodal values with any accuracy (without knowing more about the physical problem).  So, we will leave
			   the node with its old values and mark this node as not being interpolated so the user may deal with it. */

			/* Stash the node index. */
			IndexSet_Add( extNodes, newNode_i );

			/* Copy across the old value. Note that this should be done using some other provided copy method. */
			memcpy( (unsigned char*)meshExt->newNodes + newNode_i * mesh->nodeExtensionMgr->finalSize,
				(unsigned char*)mesh->node + newNode_i * mesh->nodeExtensionMgr->finalSize,
				mesh->nodeExtensionMgr->finalSize );

			/* assert(0); */
		}

		/* Free element array. */
		FreeArray( elements );
	}

	/* Dump the external nodes and delete the set. */
	IndexSet_GetMembers( extNodes, &meshExt->nExternalNodes, &meshExt->externalNodes );
	Stg_Class_Delete( extNodes );
}


#define det3_sub( x1, x2, x3 ) ( \
	(x1)[0] * ( (x2)[1] * (x3)[2] - (x3)[1] * (x2)[2] ) - \
	(x1)[1] * ( (x2)[0] * (x3)[2] - (x3)[0] * (x2)[2] ) + \
	(x1)[2] * ( (x2)[0] * (x3)[1] - (x3)[0] * (x2)[1] ) )


#define TetrahedronVolume( x1, x2,  x3, x4 ) ( \
	( det3_sub( (x2), (x3), (x4) ) - \
	  det3_sub( (x1), (x3), (x4) ) + \
	  det3_sub( (x1), (x2), (x4) ) - \
	  det3_sub( (x1), (x2), (x3) ) \
	) / 6.0f )


#define isLargerThanZero( val, tol ) \
	(((val) >= -(tol) && (val) <= (tol)) ? 0.0 : (val))


#define isLargerThanZero2( val, ref, tol ) \
	(((val) >= -fabs( ref ) * (tol) && (val) <= fabs( ref ) * tol) ? 0.0 : (val))


void interpolateNode( void* _context, Node_LocalIndex newNodeInd, Element_DomainIndex dEltInd ) {
	Snac_Context*			context = (Snac_Context*)_context;
	SnacRemesher_Context*	contextExt = ExtensionManager_Get( context->extensionMgr,
															   context,
															   SnacRemesher_ContextHandle );
	Mesh*					mesh = context->mesh;
	SnacRemesher_Mesh*		meshExt = ExtensionManager_Get( context->meshExtensionMgr,
															mesh,
															SnacRemesher_MeshHandle );
	NodeLayout*				nLayout = mesh->layout->nodeLayout;
	unsigned				nEltNodes;
	Node_DomainIndex*		eltNodes;
	Coord					newNodeCoord;
	Coord					crds[8];
	double					weights[4];
	unsigned				tetNodeInds[4];
	unsigned				eltNode_i;

	/* Extract the element's node indices.  Note that there should always be eight of these. */
	{
		Element_GlobalIndex	gEltInd;

		nEltNodes = 8;
		eltNodes = Memory_Alloc_Array( Node_DomainIndex, nEltNodes, "SnacRemesher" );
		gEltInd = Mesh_ElementMapDomainToGlobal( mesh, dEltInd );
		nLayout->buildElementNodes( nLayout, gEltInd, eltNodes );
	}

	/* Convert global node indices to domain. */
	{
		unsigned	eltNode_i;

		for( eltNode_i = 0; eltNode_i < nEltNodes; eltNode_i++ ) {
			eltNodes[eltNode_i] = Mesh_NodeMapGlobalToDomain( mesh, eltNodes[eltNode_i] );
		}
	}

	/* Extract the new node's coordinate. */
	Vector_Set( newNodeCoord, meshExt->newNodeCoords[newNodeInd] );

	/* Copy coordinates. */
	for( eltNode_i = 0; eltNode_i < nEltNodes; eltNode_i++ )
		memcpy( crds[eltNode_i], mesh->nodeCoord[eltNodes[eltNode_i]], sizeof(Coord) );

	if( !_HexaEL_FindTetBarycenter( crds, newNodeCoord, weights, tetNodeInds, INCLUSIVE_UPPER_BOUNDARY, NULL, 0 ) )
		abort();

	SnacRemesher_InterpolateNode( context, contextExt,
				      newNodeInd, dEltInd, 0,
				      tetNodeInds, weights,
				      meshExt->newNodes );

	/* Free the element node array. */
	FreeArray( eltNodes );
}


/*
** Interpolate an individual node using a cartesian method or a spherical method.
*/

void _SnacRemesher_InterpolateNode( void* _context,
				    unsigned nodeInd, unsigned elementInd, unsigned tetInd,
				    unsigned* tetNodeInds, double* weights,
				    Snac_Node* dstNodes )
{
	Snac_Context*	context = (Snac_Context*)_context;
	Mesh*		mesh = context->mesh;
	NodeLayout*			nLayout = mesh->layout->nodeLayout;
	Snac_Node*	dstNode = (Snac_Node*)ExtensionManager_At( context->mesh->nodeExtensionMgr,
								   dstNodes,
								   nodeInd );
	unsigned	tetNode_i;

	/* Extract the element's node indices.  Note that there should always be eight of these. */
	Node_DomainIndex*	eltNodes;
	unsigned int nEltNodes = 8;
	{
		Element_GlobalIndex	gEltInd;

		eltNodes = Memory_Alloc_Array( Node_DomainIndex, nEltNodes, "SnacRemesher" );
		gEltInd = Mesh_ElementMapDomainToGlobal( mesh, elementInd );
		nLayout->buildElementNodes( nLayout, gEltInd, eltNodes );
	}

	/* Convert global node indices to local. */
	{
		unsigned	eltNode_i;

		for( eltNode_i = 0; eltNode_i < nEltNodes; eltNode_i++ ) {
			eltNodes[eltNode_i] = Mesh_NodeMapGlobalToDomain( mesh, eltNodes[eltNode_i] );
		}
	}

	/* Clear the velocity. */
	dstNode->velocity[0] = 0.0;
	dstNode->velocity[1] = 0.0;
	dstNode->velocity[2] = 0.0;

	dstNode->strainSPR[0] = 0.0;
	dstNode->strainSPR[1] = 0.0;
	dstNode->strainSPR[2] = 0.0;
	dstNode->strainSPR[3] = 0.0;
	dstNode->strainSPR[4] = 0.0;
	dstNode->strainSPR[5] = 0.0;

	dstNode->stressSPR[0] = 0.0;
	dstNode->stressSPR[1] = 0.0;
	dstNode->stressSPR[2] = 0.0;
	dstNode->stressSPR[3] = 0.0;
	dstNode->stressSPR[4] = 0.0;
	dstNode->stressSPR[5] = 0.0;

	dstNode->material_ISPR = 0.0;

	dstNode->densitySPR = 0.0;

	/* Loop over each contributing node. */
	for( tetNode_i = 0; tetNode_i < 4; tetNode_i++ ) {
		Snac_Node*	srcNode;

		/* Where is this contibution coming from? */

		srcNode = Snac_Node_At( context, eltNodes[tetNodeInds[tetNode_i]] );

		/* Add the contribution. */ // TODO: Do spherical part.
		dstNode->velocity[0] += srcNode->velocity[0] * weights[tetNode_i];
		dstNode->velocity[1] += srcNode->velocity[1] * weights[tetNode_i];
		dstNode->velocity[2] += srcNode->velocity[2] * weights[tetNode_i];

		dstNode->strainSPR[0] += srcNode->strainSPR[0] * weights[tetNode_i];
		dstNode->strainSPR[1] += srcNode->strainSPR[1] * weights[tetNode_i];
		dstNode->strainSPR[2] += srcNode->strainSPR[2] * weights[tetNode_i];
		dstNode->strainSPR[3] += srcNode->strainSPR[3] * weights[tetNode_i];
		dstNode->strainSPR[4] += srcNode->strainSPR[4] * weights[tetNode_i];
		dstNode->strainSPR[5] += srcNode->strainSPR[5] * weights[tetNode_i];

		dstNode->stressSPR[0] += srcNode->stressSPR[0] * weights[tetNode_i];
		dstNode->stressSPR[1] += srcNode->stressSPR[1] * weights[tetNode_i];
		dstNode->stressSPR[2] += srcNode->stressSPR[2] * weights[tetNode_i];
		dstNode->stressSPR[3] += srcNode->stressSPR[3] * weights[tetNode_i];
		dstNode->stressSPR[4] += srcNode->stressSPR[4] * weights[tetNode_i];
		dstNode->stressSPR[5] += srcNode->stressSPR[5] * weights[tetNode_i];

		dstNode->material_ISPR += srcNode->material_ISPR * weights[tetNode_i];

		dstNode->densitySPR += srcNode->densitySPR * weights[tetNode_i];
	}
}


/* Populate field variables by SPR */
void _SnacRemesher_RecoverNode( void* _context, unsigned node_lI )
{
	Snac_Context*			context = (Snac_Context*)_context;
	Mesh*					mesh = context->mesh;
	MeshLayout*				layout = (MeshLayout*)mesh->layout;
	HexaMD*					decomp = (HexaMD*)layout->decomp;
	NodeLayout*				nLayout = layout->nodeLayout;

	Snac_Node*				node = Snac_Node_At( context, node_lI );
	Coord*					coord = Snac_NodeCoord_P( context, node_lI );
	Index 					nodeElementCount = context->mesh->nodeElementCountTbl[node_lI];
	Index 					nodeElement_I;
	Element_DomainIndex*	elements;
	gsl_matrix*				matA;
	gsl_vector* 			vecaStrain[6];
	gsl_vector*				vecaStress[6];
	gsl_vector*				vecaMaterial_I;
	gsl_vector*				vecaDensity;
	gsl_vector* 			vecbStrain[6];
	gsl_vector* 			vecbStress[6];
	gsl_vector* 			vecbMaterial_I;
	gsl_vector*				vecbDensity;
	Index 	 	 	 	 	i,j; 
	IJK						ijk;
	Node_GlobalIndex		node_gI = _MeshDecomp_Node_LocalToGlobal1D( decomp, node_lI );
	Node_GlobalIndex		gNodeI = decomp->nodeGlobal3DCounts[0];
	Node_GlobalIndex		gNodeJ = decomp->nodeGlobal3DCounts[1];
	Node_GlobalIndex		gNodeK = decomp->nodeGlobal3DCounts[2];
	Node_GlobalIndex		intNode_gI;
	Node_DomainIndex		intNode_lI;
	unsigned int			isBoundaryNode=0, patchCenterNum=1, patchCenterI, patchCenterList[2];

	RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );
	
	/* If boundary node, find a (topologically?) nearest interior node. 
	   Loosely following 
	       Khoei and Gharehbaghi, 2007, The superconvergent patch recovery techinique and data transfer operators in 3D plasticity problems,
	       Finite Elements in Analysis and Design 43 (2007) 630-- 648. */
	if( (gNodeI>2) && (ijk[0]==0) ) {
		ijk[0] += 1;
		isBoundaryNode = 1;
	}
	if( (gNodeI>2) && (ijk[0]==decomp->nodeGlobal3DCounts[0]-1) ) {
		ijk[0] -= 1;
		isBoundaryNode = 1;
	}
	if( (gNodeJ>2) && (ijk[1]==0) ) {
		ijk[1] += 1;
		isBoundaryNode = 1;
	}
	if( (gNodeJ>2) && (ijk[1]==decomp->nodeGlobal3DCounts[1]-1) ) {
		ijk[1] -= 1;
		isBoundaryNode = 1;
	}
	if( (gNodeK>2) && (ijk[2]==0) ) {
		ijk[2] += 1;
		isBoundaryNode = 1;
	}
	if( (gNodeK>2) && (ijk[2]==decomp->nodeGlobal3DCounts[2]-1) ) {
		ijk[2] -= 1;
		isBoundaryNode = 1;
	}

	/* node_lI itself always becomes a patch center,
	 and if the current node is a boundary node, find an interior node and add it to the patch center list. */
	patchCenterList[0] = node_lI; 
	if( isBoundaryNode ) {
		patchCenterNum=2;
		intNode_gI = ijk[0]+gNodeI*ijk[1]+gNodeI*gNodeJ*ijk[2];
		patchCenterList[1] = Mesh_NodeMapGlobalToLocal( mesh, intNode_gI );
	}

	/* initialize gsl vectors and matrix. */
	matA = gsl_matrix_alloc(4,4); gsl_matrix_set_zero( matA );
	vecaMaterial_I = gsl_vector_alloc(4); gsl_vector_set_zero( vecaMaterial_I );
	vecbMaterial_I = gsl_vector_alloc(4); gsl_vector_set_zero( vecbMaterial_I );
	vecaDensity = gsl_vector_alloc(4); gsl_vector_set_zero( vecaDensity );
	vecbDensity = gsl_vector_alloc(4); gsl_vector_set_zero( vecbDensity );
	for(i=0;i<6;i++) {
		vecaStrain[i] = gsl_vector_alloc(4); gsl_vector_set_zero( vecaStrain[i] );
		vecaStress[i] = gsl_vector_alloc(4); gsl_vector_set_zero( vecaStress[i] );
		vecbStrain[i] = gsl_vector_alloc(4); gsl_vector_set_zero( vecbStrain[i] );
		vecbStress[i] = gsl_vector_alloc(4); gsl_vector_set_zero( vecbStress[i] );
	}
	
	/* For each patch center */
	for( patchCenterI=0; patchCenterI < patchCenterNum; patchCenterI++ ) {
		/* For each incident element, find inicident tets. */
		for( nodeElement_I = 0; nodeElement_I < nodeElementCount; nodeElement_I++ ) {
			Element_DomainIndex		element_dI = context->mesh->nodeElementTbl[patchCenterList[patchCenterI]][nodeElement_I];

			if( element_dI < mesh->elementDomainCount ) {
				Index elementTetra_I;
				Snac_Element* element = Snac_Element_At( context, element_dI );

				/* Extract the element's node indices.  Note that there should always be eight of these. */
				{
					Element_GlobalIndex	element_gI;
					
					elements = Memory_Alloc_Array( Node_DomainIndex, nodeElementCount, "SnacRemesher" );
					element_gI = Mesh_ElementMapDomainToGlobal( mesh, element_dI );
					nLayout->buildElementNodes( nLayout, element_gI, elements );
				}
				
				/* Convert global node indices to domain. */
				{
					unsigned	eltNode_i;
					
					for( eltNode_i = 0; eltNode_i < nodeElementCount; eltNode_i++ ) {
						elements[eltNode_i] = Mesh_NodeMapGlobalToDomain( mesh, elements[eltNode_i] );
					}
				}

				/* For each incident tetrahedron in the incident element,
				   add up contributions to P, A, and b as in Zienkiewicz and Zhu (1992), p. 1336 */
				for( elementTetra_I = 0; elementTetra_I < Node_Element_Tetrahedra_Count;elementTetra_I++ ) {
					Tetrahedra_Index	tetra_I = NodeToTetra[nodeElement_I][elementTetra_I];
					Coord	tCrds[4];
					double 	positionP[4] = {1.0,0.0,0.0,0.0};
					Index 	ii,jj;

					/* Extract the tetrahedron's coordinates. */
					Vector_Set( tCrds[0], mesh->nodeCoord[elements[TetraToNode[tetra_I][0]]] );
					Vector_Set( tCrds[1], mesh->nodeCoord[elements[TetraToNode[tetra_I][1]]] );
					Vector_Set( tCrds[2], mesh->nodeCoord[elements[TetraToNode[tetra_I][2]]] );
					Vector_Set( tCrds[3], mesh->nodeCoord[elements[TetraToNode[tetra_I][3]]] );
					
					for(ii=1;ii<4;ii++)
						for(jj=0;jj<4;jj++)
							positionP[ii] += (0.25f * tCrds[jj][ii-1]);
					
					for(ii=0;ii<4;ii++) {
						double tmp;
						tmp = gsl_vector_get(vecbStrain[0],ii) + positionP[ii]*element->tetra[tetra_I].strain[0][0];
						gsl_vector_set(vecbStrain[0],ii,tmp);
						tmp = gsl_vector_get(vecbStrain[1],ii) + positionP[ii]*element->tetra[tetra_I].strain[1][1];
						gsl_vector_set(vecbStrain[1],ii,tmp);
						tmp = gsl_vector_get(vecbStrain[2],ii) + positionP[ii]*element->tetra[tetra_I].strain[2][2];
						gsl_vector_set(vecbStrain[2],ii,tmp);
						tmp = gsl_vector_get(vecbStrain[3],ii) + positionP[ii]*element->tetra[tetra_I].strain[0][1];
						gsl_vector_set(vecbStrain[3],ii,tmp);
						tmp = gsl_vector_get(vecbStrain[4],ii) + positionP[ii]*element->tetra[tetra_I].strain[0][2];
						gsl_vector_set(vecbStrain[4],ii,tmp);
						tmp = gsl_vector_get(vecbStrain[5],ii) + positionP[ii]*element->tetra[tetra_I].strain[1][2];
						gsl_vector_set(vecbStrain[5],ii,tmp);

						tmp = gsl_vector_get(vecbStress[0],ii) + positionP[ii]*element->tetra[tetra_I].stress[0][0];
						gsl_vector_set(vecbStress[0],ii,tmp);
						tmp = gsl_vector_get(vecbStress[1],ii) + positionP[ii]*element->tetra[tetra_I].stress[1][1];
						gsl_vector_set(vecbStress[1],ii,tmp);
						tmp = gsl_vector_get(vecbStress[2],ii) + positionP[ii]*element->tetra[tetra_I].stress[2][2];
						gsl_vector_set(vecbStress[2],ii,tmp);
						tmp = gsl_vector_get(vecbStress[3],ii) + positionP[ii]*element->tetra[tetra_I].stress[0][1];
						gsl_vector_set(vecbStress[3],ii,tmp);
						tmp = gsl_vector_get(vecbStress[4],ii) + positionP[ii]*element->tetra[tetra_I].stress[0][2];
						gsl_vector_set(vecbStress[4],ii,tmp);
						tmp = gsl_vector_get(vecbStress[5],ii) + positionP[ii]*element->tetra[tetra_I].stress[1][2];
						gsl_vector_set(vecbStress[5],ii,tmp);

/* 						tmp = gsl_vector_get(vecbMaterial_I,ii) + positionP[ii]*((double)(element->tetra[tetra_I].material_I)+0.5); */
						tmp = gsl_vector_get(vecbMaterial_I,ii) + positionP[ii]*pow(10.0,(double)(element->tetra[tetra_I].material_I));
						gsl_vector_set(vecbMaterial_I,ii,tmp); 

						tmp = gsl_vector_get(vecbDensity,ii) + positionP[ii]*element->tetra[tetra_I].density;
						gsl_vector_set(vecbDensity,ii,tmp); 
					
						for(jj=0;jj<4;jj++) {
							tmp = gsl_matrix_get(matA,ii,jj) + positionP[ii]*positionP[jj];
							gsl_matrix_set(matA,ii,jj,tmp);
						}
					} /* end of verteces of a tet. */
				} /* end of incident tets. */
			} /* if within my domain */
		} /* end of incident elements. */
	} /* end of patchCenterI */
		
	/* compute parameter vectors. */
	{
		int s;
		gsl_permutation * p = gsl_permutation_alloc (4);
     
		gsl_linalg_LU_decomp (matA, p, &s);
			
		for(i=0;i<6;i++) {
			gsl_linalg_LU_solve (matA, p, vecbStrain[i], vecaStrain[i]);
			gsl_linalg_LU_solve (matA, p, vecbStress[i], vecaStress[i]);
		}
		gsl_linalg_LU_solve (matA, p, vecbMaterial_I, vecaMaterial_I);
		gsl_linalg_LU_solve (matA, p, vecbDensity, vecaDensity);
		/* 			printf ("x = \n"); */
		/* 			gsl_vector_fprintf (stdout, x, "%g"); */
		gsl_permutation_free (p);
	}	

	/* zero the arrays to store recovered field. */
	/* probably not necessary. */
	/* 		for(i=0;i<6;i++) { */
	/* 			node->strainSPR[i] = 0.0f; */
	/* 			node->stressSPR[i] = 0.0f; */
	/* 		} */

	/* Recover using the parameter vectors. */
	for(j=0;j<6;j++) {
		node->strainSPR[j] = gsl_vector_get(vecaStrain[j],0);
		node->stressSPR[j] = gsl_vector_get(vecaStress[j],0);
		for(i=0;i<3;i++) {
			node->strainSPR[j] += gsl_vector_get(vecaStrain[j],i+1)*(*coord)[i];
			node->stressSPR[j] += gsl_vector_get(vecaStress[j],i+1)*(*coord)[i];
		}
	}

	node->material_ISPR = gsl_vector_get(vecaMaterial_I,0);
	for(i=0;i<3;i++) 
		node->material_ISPR += gsl_vector_get(vecaMaterial_I,i+1)*(*coord)[i];

	node->densitySPR = gsl_vector_get(vecaDensity,0);
	for(i=0;i<3;i++) 
		node->densitySPR += gsl_vector_get(vecaDensity,i+1)*(*coord)[i];

	/* free gsl vectors and matrix. */
	gsl_matrix_free( matA );
	gsl_vector_free( vecaMaterial_I );
	gsl_vector_free( vecbMaterial_I );
	gsl_vector_free( vecaDensity );
	gsl_vector_free( vecbDensity );
	for(i=0;i<6;i++) {
		gsl_vector_free( vecaStrain[i] );
		gsl_vector_free( vecaStress[i] );
		gsl_vector_free( vecbStrain[i] );
		gsl_vector_free( vecbStress[i] );
	}

	/* Free the element node array. */
	FreeArray( elements );

	/* end of recovery. */
}

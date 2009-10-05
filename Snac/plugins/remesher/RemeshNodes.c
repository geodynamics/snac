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


void _SnacRemesher_InterpolateNodes( void* _context ) {
	Snac_Context*		context = (Snac_Context*)_context;
	Mesh*				mesh = context->mesh;
	SnacRemesher_Mesh*	meshExt = ExtensionManager_Get( context->meshExtensionMgr,
															mesh,
															SnacRemesher_MeshHandle );
	NodeLayout*			nLayout = mesh->layout->nodeLayout;
	Node_LocalIndex		newNode_i;
	IndexSet*			extNodes;

	void interpolateNode( void* _context, Node_LocalIndex newNodeInd, Element_DomainIndex dEltInd );
	void SPR( void* _context );

	/*
	** Free any owned arrays that may still exist from the last node interpolation.
	*/

	FreeArray( meshExt->externalNodes );
	meshExt->nExternalNodes = 0;

	
	/*
	** Populate arrays for recovered fields using the SPR method.
	*/
	SPR( context );

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
	Mesh*				mesh = context->mesh;
	SnacRemesher_Mesh*		meshExt = ExtensionManager_Get( context->meshExtensionMgr,
									mesh,
									SnacRemesher_MeshHandle );
	NodeLayout*			nLayout = mesh->layout->nodeLayout;
	unsigned				nEltNodes;
	Node_DomainIndex*		eltNodes;
	Coord				newNodeCoord;
	Coord			crds[8];
	double			weights[4];
	unsigned		tetNodeInds[4];
	unsigned		eltNode_i;

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

#if 0
	/* Loop over 10 sub tets in a brick element, work out the tetra it is in. */
	for( tet_i = 0; tet_i < nTets; tet_i++ ) {
		Coord	tCrds[4];
		double	TOL = 0.0;

		/* Extract the tetrahedron's coordinates. */
		Vector_Set( tCrds[0], mesh->nodeCoord[eltNodes[nSub[tet_i * 4 + 0]]] );
		Vector_Set( tCrds[1], mesh->nodeCoord[eltNodes[nSub[tet_i * 4 + 1]]] );
		Vector_Set( tCrds[2], mesh->nodeCoord[eltNodes[nSub[tet_i * 4 + 2]]] );
		Vector_Set( tCrds[3], mesh->nodeCoord[eltNodes[nSub[tet_i * 4 + 3]]] );

		dett = TetrahedronVolume( tCrds[0], tCrds[1], tCrds[2], tCrds[3] );
		det[0] = TetrahedronVolume( tCrds[1], tCrds[3], tCrds[2], newNodeCoord );
		det[1] = TetrahedronVolume( tCrds[2], tCrds[3], tCrds[0], newNodeCoord );
		det[2] = TetrahedronVolume( tCrds[0], tCrds[3], tCrds[1], newNodeCoord );
		det[3] = TetrahedronVolume( tCrds[0], tCrds[1], tCrds[2], newNodeCoord );

		dett = isLargerThanZero( dett, TOL );
		det[0] = isLargerThanZero2( det[0], dett, TOL );
		det[1] = isLargerThanZero2( det[1], dett, TOL );
		det[2] = isLargerThanZero2( det[2], dett, TOL );
		det[3] = isLargerThanZero2( det[3], dett, TOL );
		dett = det[0] + det[1] + det[2] + det[3];

		/* TODO: Convert to a journal command. */
		/* Found if all det are greater than zero. */
		Journal_Firewall( (dett != 0.0 ), "processor %d: element_lI=%d tetra=%d dett is zero!!\napexes: (%e %e %e) (%e %e %e) (%e %e %e) (%e %e %e)\nnewNodeCoord=(%e %e %e)\n",
				  context->rank, dEltInd, tet_i,
				  tCrds[0][0],tCrds[0][1],tCrds[0][2],tCrds[1][0],tCrds[1][1],tCrds[1][2],
				  tCrds[2][0],tCrds[2][1],tCrds[2][2],tCrds[3][0],tCrds[3][1],tCrds[3][2],
				  newNodeCoord[0],newNodeCoord[0],newNodeCoord[0]);
		if ( det[0] >= 0.0 && det[1] >= 0.0 && det[2] >= 0.0 && det[3] >= 0.0 ) {
			break;
		}
	}

	/* Did we find the tetrahedron? */
	if( tet_i < nTets ) {
		Node_DomainIndex	tetNodeInds[4];
		double			shape[4];
		unsigned		tNode_i;

		/* Calculate the shape funcs and remap the tetrahedron node indices. */
		for( tNode_i = 0; tNode_i < 4; tNode_i++ ) {
			shape[tNode_i] = det[tNode_i] / dett;
			tetNodeInds[tNode_i] = eltNodes[nSub[tet_i * 4 + tNode_i]];
		}

		/* Interpolate the node. */

		/*
		** This is where we should be calling the entry points for node interpolation.
		*/

		SnacRemesher_InterpolateNode( context, contextExt,
					      newNodeInd, dEltInd, tet_i,
					      tetNodeInds, shape,
					      meshExt->newNodes );
	}
	else {
		/* If not, then something unpleasant is going on, and I don't know what it is. */
		for( tet_i = 0; tet_i < nTets; tet_i++ ) {
			Coord	tCrds[4];
			double	TOL = 0.0;

			/* Extract the tetrahedron's coordinates. */
			Vector_Set( tCrds[0], mesh->nodeCoord[eltNodes[nSub[tet_i * 4 + 0]]] );
			Vector_Set( tCrds[1], mesh->nodeCoord[eltNodes[nSub[tet_i * 4 + 1]]] );
			Vector_Set( tCrds[2], mesh->nodeCoord[eltNodes[nSub[tet_i * 4 + 2]]] );
			Vector_Set( tCrds[3], mesh->nodeCoord[eltNodes[nSub[tet_i * 4 + 3]]] );

			dett = TetrahedronVolume( tCrds[0], tCrds[1], tCrds[2], tCrds[3] );
			det[0] = TetrahedronVolume( tCrds[1], tCrds[3], tCrds[2], newNodeCoord );
			det[1] = TetrahedronVolume( tCrds[2], tCrds[3], tCrds[0], newNodeCoord );
			det[2] = TetrahedronVolume( tCrds[0], tCrds[3], tCrds[1], newNodeCoord );
			det[3] = TetrahedronVolume( tCrds[0], tCrds[1], tCrds[2], newNodeCoord );

			dett = isLargerThanZero( dett, TOL );
			det[0] = isLargerThanZero2( det[0], dett, TOL );
			det[1] = isLargerThanZero2( det[1], dett, TOL );
			det[2] = isLargerThanZero2( det[2], dett, TOL );
			det[3] = isLargerThanZero2( det[3], dett, TOL );
			dett = det[0] + det[1] + det[2] + det[3];

			Journal_Printf( context->debug, "processor %d: element_lI=%d/%d tetra=%d\napexes: (%e %e %e) (%e %e %e) (%e %e %e) (%e %e %e)\nnewNodeCoord=(%e %e %e) dets: %e %e %e %e (%e)\n\n",
					context->rank, dEltInd, mesh->elementDomainCount,tet_i,
					tCrds[0][0],tCrds[0][1],tCrds[0][2],tCrds[1][0],tCrds[1][1],tCrds[1][2],
					tCrds[2][0],tCrds[2][1],tCrds[2][2],tCrds[3][0],tCrds[3][1],tCrds[3][2],
					newNodeCoord[0],newNodeCoord[1],newNodeCoord[2],det[0],det[1],det[2],det[3],dett);

		}
		Journal_Firewall( 0, "processor %d: element_lI=%d tetra=%d failed to interpolate although a matching element was found!!\n",
				  context->rank, dEltInd, tet_i );
	}
#endif

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

	dstNode->plStrainSPR = 0.0;

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

	}
}


void SPR( void* _context )
{
	Snac_Context*		context = (Snac_Context*)_context;
	Mesh*				mesh = context->mesh;
	NodeLayout*			nLayout = mesh->layout->nodeLayout;
	Node_LocalIndex		node_lI;

	/* Populate field variables by SPR */
	for( node_lI = 0; node_lI < mesh->nodeLocalCount; node_lI++ ) {
		Snac_Node*				node = Snac_Node_At( context, node_lI );
		Coord*					coord = Snac_NodeCoord_P( context, node_lI );
		Index 					nodeElementCount = context->mesh->nodeElementCountTbl[node_lI];
		Index 					nodeElement_I;
		Element_DomainIndex*	elements;
		gsl_matrix*				matA;
		gsl_vector* 			vecaStrain[6];
		gsl_vector*				vecaStress[6];
		gsl_vector*				vecaplStrain;
		gsl_vector* 			vecbStrain[6];
		gsl_vector* 			vecbStress[6];
		gsl_vector* 			vecbplStrain;
		Index 	 	 	 	 	i,j; 
		
		// initialize gsl vectors and matrix.
		matA = gsl_matrix_alloc(4,4); gsl_matrix_set_zero( matA );
		vecaplStrain = gsl_vector_alloc(4); gsl_vector_set_zero( vecaplStrain );
		vecbplStrain = gsl_vector_alloc(4); gsl_vector_set_zero( vecbplStrain );
		for(i=0;i<6;i++) {
			vecaStrain[i] = gsl_vector_alloc(4); gsl_vector_set_zero( vecaStrain[i] );
			vecaStress[i] = gsl_vector_alloc(4); gsl_vector_set_zero( vecaStress[i] );
			vecbStrain[i] = gsl_vector_alloc(4); gsl_vector_set_zero( vecbStrain[i] );
			vecbStress[i] = gsl_vector_alloc(4); gsl_vector_set_zero( vecbStress[i] );
		}
			
		/* For each incident element, find inicident tets. */
		for( nodeElement_I = 0; nodeElement_I < nodeElementCount; nodeElement_I++ ) {
			Element_DomainIndex		element_dI = context->mesh->nodeElementTbl[node_lI][nodeElement_I];

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

/* 						tmp = gsl_vector_get(vecbplStrain,i) + positionP[i]*plasticElement->tetra[tetra_I].stress[1][2]; */
/* 						gsl_vector_set(vecStress[5],i,tmp); */
						for(jj=0;jj<4;jj++) {
							tmp = gsl_matrix_get(matA,ii,jj) + positionP[ii]*positionP[jj];
							gsl_matrix_set(matA,ii,jj,tmp);
						}
					}
				} // end of incident tets.
			} // if within my domain
		} // end of incident elements.
		
		// compute parameter vectors.
		{
			int s;
			gsl_permutation * p = gsl_permutation_alloc (4);
     
			gsl_linalg_LU_decomp (matA, p, &s);
			
			for(i=0;i<6;i++) {
				gsl_linalg_LU_solve (matA, p, vecbStrain[i], vecaStrain[i]);
				gsl_linalg_LU_solve (matA, p, vecbStress[i], vecaStress[i]);
			}
/* 			printf ("x = \n"); */
/* 			gsl_vector_fprintf (stdout, x, "%g"); */
			gsl_permutation_free (p);
		}	

		// zero the arrays to store recovered field.
		// probably not necessary.
/* 		for(i=0;i<6;i++) { */
/* 			node->strainSPR[i] = 0.0f; */
/* 			node->stressSPR[i] = 0.0f; */
/* 		} */

		// Recover using the parameter vectors.
		for(j=0;j<6;j++) {
			node->strainSPR[j] = gsl_vector_get(vecaStrain[j],0);
			node->stressSPR[j] = gsl_vector_get(vecaStress[j],0);
			for(i=0;i<3;i++) {
				node->strainSPR[j] += gsl_vector_get(vecaStrain[j],i+1)*(*coord)[i];
				node->stressSPR[j] += gsl_vector_get(vecaStress[j],i+1)*(*coord)[i];
			}
		}

		// free gsl vectors and matrix.
		gsl_matrix_free( matA );
		gsl_vector_free( vecaplStrain );
		gsl_vector_free( vecbplStrain );
		for(i=0;i<6;i++) {
			gsl_vector_free( vecaStrain[i] );
			gsl_vector_free( vecaStress[i] );
			gsl_vector_free( vecbStrain[i] );
			gsl_vector_free( vecbStress[i] );
		}
		/* Free the element node array. */
		FreeArray( elements );
	} // end of recovery.
}



#if 0

#define DIM 3
#define NODES_PER_ELEMENT 8

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif


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

extern double isLargerThanZero( double, double );
extern double isLargerThanZero2( double, double, double );
extern void xyz2tprCoord( Coord, Coord* );
extern void tpr2xyzCoord( Coord, Coord* );
extern void xyz2tprVel( Coord, Coord, Coord* );
extern void tpr2xyzVel( Coord, Coord, Coord* );

/* For a given tetra_I (only interesting in first 5), what is the en_I of a given node of that tetra */
const int nsub[] = {
	0, 2, 3, 7,
	0, 1, 2, 5,
	4, 7, 5, 0,
	5, 7, 6, 2,
	5, 7, 2, 0,
	3, 7, 4, 6,
	4, 0, 3, 1,
	6, 2, 1, 3,
	1, 5, 6, 4,
	1, 6, 3, 4
};


void _SnacRemesher_InterpolateNode(
	void*			_context,
	Index			contribution,
	double			shapeFunction[4],
	Node_LocalIndex		node_lI,
	Node_LocalIndex		fromNode_lI,
	Partition_Index		fromNode_rn_I,
	Node_ShadowIndex	fromNode_sI,
	Snac_Node*		newNode,
	Snac_Node**		shadowNode )
{
	Snac_Context*			context = (Snac_Context*)_context;
	SnacRemesher_Context*			contextExt = ExtensionManager_Get(
		context->extensionMgr,
		context,
		SnacRemesher_ContextHandle );
	SnacRemesher_Mesh*		meshExt = ExtensionManager_Get(
		context->meshExtensionMgr,
		context->mesh,
		SnacRemesher_MeshHandle );
	Snac_Node*			node = (Snac_Node*)ExtensionManager_At( context->mesh->nodeExtensionMgr, newNode, node_lI );
	Snac_Node*			fromNode;
	Index				d;
	Coord                           temp,temp1,temp2,newNodeCoordS;

	/* Zero out on first addition */
	if( contribution == 0 ) {
		memset( node->velocity, 0, sizeof(double) * DIM );
	}
	/* Deference source node (local/shadow) */
	if( fromNode_lI < context->mesh->nodeLocalCount ) {
		fromNode = Snac_Node_At( context, fromNode_lI );
	}
	else {
		fromNode = (Snac_Node*)ExtensionManager_At( context->mesh->nodeExtensionMgr, shadowNode[fromNode_rn_I], fromNode_sI );
	}
	if(meshExt->meshType == SnacRemesher_Spherical) {
		xyz2tprVel( meshExt->newNodeCoord[node_lI], fromNode->velocity, &temp1 );
		for( d = 0; d < DIM; d++ ) {
			temp2[d] = temp1[d] * shapeFunction[contribution];
		}
		xyz2tprCoord( meshExt->newNodeCoord[node_lI], &newNodeCoordS );
		tpr2xyzVel( newNodeCoordS, temp2, &temp );
		node->velocity[0] += temp[0];
		node->velocity[1] += temp[1];
		node->velocity[2] += temp[2];
	}
	else {
		for( d = 0; d < DIM; d++ ) {
			node->velocity[d] += fromNode->velocity[d] * shapeFunction[contribution];
		}
	}
}

void _SnacRemesher_InterpolateNodes( void* _context ) {
	Snac_Context*			context = (Snac_Context*)_context;
	SnacRemesher_Mesh*		meshExt = ExtensionManager_Get(
		context->meshExtensionMgr,
		context->mesh,
		SnacRemesher_MeshHandle );

	void _SnacRemesher_InterpolateNodes_Cartesian( void* _context );
	void _SnacRemesher_InterpolateNodes_Spherical( void* _context );

	if(meshExt->meshType == SnacRemesher_Spherical ) {
		_SnacRemesher_InterpolateNodes_Spherical( context );
		return;
	}
	else {
		_SnacRemesher_InterpolateNodes_Cartesian( context );
		return;
	}
}

void _SnacRemesher_InterpolateNodes_Cartesian( void* _context ) {
	Snac_Context*			context = (Snac_Context*)_context;
	SnacRemesher_Context*		contextExt = ExtensionManager_Get(
		context->extensionMgr,
		context,
		SnacRemesher_ContextHandle );
	Mesh*					mesh = context->mesh;
	HexaMD*				decomp = (HexaMD*)mesh->layout->decomp;
	SnacRemesher_Mesh*		meshExt = ExtensionManager_Get(
		context->meshExtensionMgr,
		mesh,
		SnacRemesher_MeshHandle );
	int*				ind;
	Index				d;
	Element_LocalIndex      elX,elY,elZ;
	Element_LocalIndex      nodeX,nodeY,nodeZ;
	Element_NodeIndex		elementNodeCount;
	double				xc[DIM*NODES_PER_ELEMENT];
	PartitionIndex			rn_I;
	Node_LocalIndex			node_lI,node_lN;
	unsigned int                            All_passed;
	double                          TOL;
	int                    count;
	const int                          shadowDepth = ( decomp->shadowDepth > 3) ? decomp->shadowDepth : 3;
	const int                          numSearchElement = 2*shadowDepth-1;
	const int                          numSearchDepth = shadowDepth-1;
	const Element_LocalIndex		nElementX=decomp->elementLocal3DCounts[decomp->rank][0];
	const Element_LocalIndex		nElementY=decomp->elementLocal3DCounts[decomp->rank][1];
	const Element_LocalIndex		nElementZ=decomp->elementLocal3DCounts[decomp->rank][2];
	const Node_LocalIndex		nNodeX=decomp->nodeLocal3DCounts[decomp->rank][0];
	const Node_LocalIndex		nNodeY=decomp->nodeLocal3DCounts[decomp->rank][1];
	const Node_LocalIndex		nNodeZ=decomp->nodeLocal3DCounts[decomp->rank][2];

	Journal_DPrintf( context->debug, "In: %s\n", __func__ );

	ind = (int*)malloc( sizeof( int ) * meshExt->nodeLocalCount );
	memset( ind, 0, sizeof( int ) * meshExt->nodeLocalCount );

	/* for all nodes, interpolate velocity; loop over local elements */
	All_passed = 0;
	TOL= 0.0f;
	while(!All_passed) {
		for(nodeZ=0; nodeZ < nNodeZ; nodeZ++)
			for(nodeY=0; nodeY < nNodeY; nodeY++)
				for(nodeX=0; nodeX < nNodeX; nodeX++) {
					Element_NodeIndex       en_N;
					double                  elembbox[2][DIM];
					Node_LocalIndex**       elementNodeTbl = context->mesh->elementNodeTbl;
					Element_LocalIndex      searchElement_lI,searchElement_lJ,searchElement_lK;
					Element_LocalIndex      element_lN;

					int				found;
					Tetrahedra_Index		tetra_I;
					Coord				newNodeCoord;

					/* Ensure we haven't already done this node */
					node_lI = nodeX + nodeY*nNodeX + nodeZ*nNodeX*nNodeY;
					if( ind[node_lI] ) {
						continue;
					}
					elX = (nodeX <= 1)? 0 : (nodeX-1);
					elY = (nodeY <= 1)? 0 : (nodeY-1);
					elZ = (nodeZ <= 1)? 0 : (nodeZ-1);

					newNodeCoord[0] = meshExt->newNodeCoord[node_lI][0];
					newNodeCoord[1] = meshExt->newNodeCoord[node_lI][1];
					newNodeCoord[2] = meshExt->newNodeCoord[node_lI][2];

					for(searchElement_lK=0;searchElement_lK<numSearchElement;searchElement_lK++) {
						if( (elZ+searchElement_lK-numSearchDepth) < 0 || (elZ+searchElement_lK-numSearchDepth) >= nElementZ )
							continue;
						for(searchElement_lJ=0;searchElement_lJ<numSearchElement;searchElement_lJ++) {
							if( (elY+searchElement_lJ-numSearchDepth) < 0 || (elY+searchElement_lJ-numSearchDepth) >= nElementY )
								continue;
							for(searchElement_lI=0;searchElement_lI<numSearchElement;searchElement_lI++) {
								if( (elX+searchElement_lI-numSearchDepth) < 0 || (elX+searchElement_lI-numSearchDepth) >= nElementX )
									continue;
								element_lN = (elX+searchElement_lI-numSearchDepth) + (elY+searchElement_lJ-numSearchDepth)*nElementX
									+ (elZ+searchElement_lK-numSearchDepth)*nElementY*nElementX;

								if( ind[node_lI] ) {
									break;
								}

								elementNodeCount = context->mesh->elementNodeCountTbl[element_lN];
								if( elementNodeCount != (unsigned)NODES_PER_ELEMENT ) {
									printf( "elementNodeCount != NODES_PER_ELEMENT  element_lN = %d", element_lN );
									assert( 0 );
								}

								/* create a local copy of the veocity and coords for each of the element's nodes of the old mesh */
								for( en_N = 0; en_N < NODES_PER_ELEMENT; en_N++) {
									node_lN = elementNodeTbl[element_lN][en_N];
									for( d = 0; d < DIM; d++ ) {
										xc[en_N*DIM+d] = Mesh_CoordAt( context->mesh, node_lN )[d];
									}
									//xc[en_N*DIM+2] = meshExt->initialNodeCoord[elementNodeTbl[element_lN][en_N]][2];
								}

								/* Calculate element's bounding box */
								for( d = 0; d < DIM; d++ ) {
									elembbox[0][d] = DBL_MAX;
									elembbox[1][d] = DBL_MIN;
								}
								for( en_N = 0; en_N < NODES_PER_ELEMENT; en_N++ ) {
									for( d = 0; d < DIM; ++d ) {
										elembbox[0][d] = MIN( elembbox[0][d], xc[en_N*DIM+d] );
										elembbox[1][d] = MAX( elembbox[1][d], xc[en_N*DIM+d] );
									}
								}

								/* If new mesh's current node is outside the bounding box, next loop */
								if( newNodeCoord[0] < elembbox[0][0]-0.5f*(elembbox[1][0]-elembbox[0][0]) ||
								    newNodeCoord[0] > elembbox[1][0]+0.5f*(elembbox[1][0]-elembbox[0][0]) ||
								    newNodeCoord[1] < elembbox[0][1]-0.5f*(elembbox[1][1]-elembbox[0][1]) ||
								    newNodeCoord[1] > elembbox[1][1]+0.5f*(elembbox[1][1]-elembbox[0][1]) ||
								    newNodeCoord[2] < elembbox[0][2]-0.5f*(elembbox[1][2]-elembbox[0][2]) ||
								    newNodeCoord[2] > elembbox[1][2]+0.5f*(elembbox[1][2]-elembbox[0][2]) ) {
									continue;
								}

								/* loop over 10 sub tets in a brick element, work out the tetra it is in, and then interpolate */
								found = 0;
								for( tetra_I = 0; tetra_I < Tetrahedra_Count && !found; tetra_I++ ) {
									double				x1[DIM];
									double				x2[DIM];
									double				x3[DIM];
									double				x4[DIM];
									double				dett;
									double				det[4];

									for( d = 0; d < DIM; d++ ) {
										x1[d] = xc[nsub[tetra_I*4]*DIM+d];
										x2[d] = xc[nsub[tetra_I*4+1]*DIM+d];
										x3[d] = xc[nsub[tetra_I*4+2]*DIM+d];
										x4[d] = xc[nsub[tetra_I*4+3]*DIM+d];
									}

									dett = TetrahedronVolume( x1, x2, x3, x4 );
									det[0] = TetrahedronVolume( x2, x4, x3, newNodeCoord );
									det[1] = TetrahedronVolume( x3, x4, x1, newNodeCoord );
									det[2] = TetrahedronVolume( x1, x4, x2, newNodeCoord );
									det[3] = TetrahedronVolume( x1, x2, x3, newNodeCoord );

									dett = isLargerThanZero( dett, TOL );
									det[0] = isLargerThanZero2( det[0], dett, TOL );
									det[1] = isLargerThanZero2( det[1], dett, TOL );
									det[2] = isLargerThanZero2( det[2], dett, TOL );
									det[3] = isLargerThanZero2( det[3], dett, TOL );
									dett = det[0] + det[1] + det[2] + det[3];

									assert( dett != 0.0 );
									if( dett < 0 )	{
										printf( "Determinant evaluation is wrong node=%d\t xt[0]=%g \t xt[1]=%g \t xt[t]=%g\n",
											en_N,
											newNodeCoord[0],
											newNodeCoord[1],
											newNodeCoord[2]);
										continue;
									}

									/* found if all det are greater than zero */
									if ( det[0] >= 0.0f && det[1] >= 0.0f && det[2] >= 0.0f && det[3] >= 0.0f ) {
										found = 1;
									}
									//fprintf(stderr,"%s: node_lI=%d element_lN=%d ind=%d foun=%d newCoord=%e %e %e\n\tdett=%e %e %e %e\n",__func__,node_lI,element_lN,ind[node_lI],found,newNodeCoord[0],newNodeCoord[1],newNodeCoord[2],det[0],det[1],det[2],dett);

									if( found ) {
										double				shape[4];
										Index				tNode_I;

										/* mark, such that we dont do it again */
										ind[node_lI] = 1;

										/* Calculate the shape funcs */
										for( tNode_I = 0; tNode_I < 4; tNode_I++ ) {
											shape[tNode_I] = det[tNode_I] / dett;
										}

										/* Assign proper values of velocities and temperatures from old mesh to new mesh */
										for( tNode_I = 0; tNode_I < 4; tNode_I++ ) {

											if( elementNodeTbl[element_lN][nsub[tetra_I*4+tNode_I]] >
											    context->mesh->nodeLocalCount ) {
												int i;

												printf( "element_lN: %u, nsub[tetra_I*4+tNode_I]: %u\n",
													element_lN,
													nsub[tetra_I*4+tNode_I] );
												printf( "elementNodeTbl: %p, { ",
													elementNodeTbl );
												for( i = 0; i < 8; i++ ) {
													printf( "%u, ",
														elementNodeTbl[element_lN][i] );
												}
												printf( "\n" );
												assert(
													elementNodeTbl[element_lN][nsub[tetra_I*4+tNode_I]] >
													context->mesh->nodeLocalCount );
											}

											SnacRemesher_InterpolateNode(
												context,
												contextExt,
												tNode_I,
												shape,
												node_lI,
												elementNodeTbl[element_lN][nsub[tetra_I*4+tNode_I]],
												0,
												0,
												meshExt->newNode,
												0 );
										}
										break;
									}
								}
							}
						}
					}
					/*
					** Loop over shadow elements
					*/
					for( rn_I = 0; rn_I < meshExt->neighbourRankCount; rn_I++ ) {
						Element_ShadowIndex		element_sN;

						/* Loop over all the shadow elements to find one including node_lI */
						for( element_sN = 0; element_sN < meshExt->shadowElementRemoteCount[rn_I]; element_sN++ ) {
							Element_GlobalIndex		gElement_N = meshExt->shadowElementRemoteArray[rn_I][element_sN];
							Node_Index*			shadowElementNodesN = (Node_Index*)malloc( sizeof(Node_Index) * NODES_PER_ELEMENT );
							Index				en_N;
							double				elembbox[2][DIM];
							Node_GlobalIndex	elementNodeN[8];

							if(ind[node_lI] == 1)
								break;

							/* Figure out node index */
							_HexaEL_BuildCornerIndices( context->mesh->layout->elementLayout, gElement_N, elementNodeN );
							for( en_N = 0; en_N < NODES_PER_ELEMENT; en_N++ ) {
								Node_GlobalIndex    node_gN = elementNodeN[en_N];
								Index               node_lN = Mesh_NodeMapGlobalToLocal( context->mesh, node_gN );
								unsigned int        found = 0;
								if( node_lN < meshExt->nodeLocalCount ) {
									shadowElementNodesN[en_N] = node_lN;
									found = 1;
									elementNodeN[en_N] = node_lN;
								}
								else {
									Node_ShadowIndex		node_sI;
									for( node_sI = 0; node_sI < meshExt->shadowNodeRemoteCount[rn_I]; node_sI++ ) {
										if( node_gN == meshExt->shadowNodeRemoteArray[rn_I][node_sI] ) {
											shadowElementNodesN[en_N] = context->mesh->nodeGlobalCount + node_sI;
											found = 1;
										}
									}
								}
								assert( found ); /* we expected this to be a shadow node, but wasn't in list! */
							}

							for( en_N = 0; en_N < NODES_PER_ELEMENT; en_N++ ) {
								node_lN = shadowElementNodesN[en_N];
								if( node_lN < context->mesh->nodeDomainCount ) {
									for( d = 0; d < DIM; d++ ) {
										xc[en_N*DIM+d] = Mesh_CoordAt( context->mesh, node_lN )[d];
									}
								}
								else {
									node_lN -= context->mesh->nodeGlobalCount;
									for( d = 0; d < DIM; d++ ) {
										xc[en_N*DIM+d] = meshExt->shadowNodeRemoteCoord[rn_I][node_lN][d];
									}
								}
							}
							/* Calculate element's bounding box */
							for( d = 0; d < DIM; ++d ) {
								elembbox[0][d] = DBL_MAX;
								elembbox[1][d] = DBL_MIN;
							}
							for( en_N = 0; en_N < NODES_PER_ELEMENT; en_N++ ) {
								for( d = 0; d < DIM; ++d ) {
									elembbox[0][d] = MIN( elembbox[0][d], xc[en_N*DIM+d] );
									elembbox[1][d] = MAX( elembbox[1][d], xc[en_N*DIM+d] );
								}
							}
							/* If new mesh's current node is outside the bounding box, next loop */
							if( newNodeCoord[0] < elembbox[0][0]-0.5f*(elembbox[1][0]-elembbox[0][0]) ||
							    newNodeCoord[0] > elembbox[1][0]+0.5f*(elembbox[1][0]-elembbox[0][0]) ||
							    newNodeCoord[1] < elembbox[0][1]-0.5f*(elembbox[1][1]-elembbox[0][1]) ||
							    newNodeCoord[1] > elembbox[1][1]+0.5f*(elembbox[1][1]-elembbox[0][1]) ||
							    newNodeCoord[2] < elembbox[0][2]-0.5f*(elembbox[1][2]-elembbox[0][2]) ||
							    newNodeCoord[2] > elembbox[1][2]+0.5f*(elembbox[1][2]-elembbox[0][2]) )
							{
								//assert(0);
								continue;
							}

							/* loop over 10 sub tets in a brick element, work out the tetra it is in, and then interpolate */
							found = 0;
							for( tetra_I = 0; tetra_I < Tetrahedra_Count && !found; tetra_I++ ) {
								double				x1[DIM];
								double				x2[DIM];
								double				x3[DIM];
								double				x4[DIM];
								double				dett;
								double				det[4];

								for( d = 0; d < DIM; d++ ) {
									x1[d] = xc[nsub[tetra_I*4]*DIM+d];
									x2[d] = xc[nsub[tetra_I*4+1]*DIM+d];
									x3[d] = xc[nsub[tetra_I*4+2]*DIM+d];
									x4[d] = xc[nsub[tetra_I*4+3]*DIM+d];
								}
								dett = TetrahedronVolume( x1, x2, x3, x4 );
								det[0] = TetrahedronVolume( x2, x4, x3, newNodeCoord );
								det[1] = TetrahedronVolume( x3, x4, x1, newNodeCoord );
								det[2] = TetrahedronVolume( x1, x4, x2, newNodeCoord );
								det[3] = TetrahedronVolume( x1, x2, x3, newNodeCoord );

								dett = isLargerThanZero( dett, TOL );
								det[0] = isLargerThanZero2( det[0], dett, TOL );
								det[1] = isLargerThanZero2( det[1], dett, TOL );
								det[2] = isLargerThanZero2( det[2], dett, TOL );
								det[3] = isLargerThanZero2( det[3], dett, TOL );
								dett = det[0] + det[1] + det[2] + det[3];

								Journal_Firewall( dett != 0.0, context->snacError, "node_lI=%d element_sN=%d me=%d TOL=%e newCoord=%e %e %e\nx1(%d)=   %e %e %e\nx2(%d)=   %e %e %e \nx3(%d)=   %e %e %e\nx4(%d)=   %e %e %e\ndett=%e (%e %e %e %e)\n", node_lI,element_sN,context->rank,TOL, newNodeCoord[0],newNodeCoord[1],newNodeCoord[2],nsub[tetra_I*4],x1[0],x1[1],x1[2],nsub[tetra_I*4+1],x2[0],x2[1],x2[2],nsub[tetra_I*4+2],x3[0],x3[1],x3[2],nsub[tetra_I*4+3],x4[0],x4[1],x4[2],dett,det[0],det[1],det[2],det[3]);

								if( dett < 0 ) {
									continue;
								}

								/* found if all det are greater than zero */
								if( det[0] >= 0.0f && det[1] >= 0.0f && det[2] >= 0.0f && det[3] >= 0.0f ) {
									found = 1;
								}
								if( found ) {
									double				shape[4];
									Index				tNode_I;

									/* mark, such that we dont do it again */
									ind[node_lI] = 1;

									/* Calculate the shape funcs */
									for( tNode_I = 0; tNode_I < 4; tNode_I++ ) {
										shape[tNode_I] = det[tNode_I] / dett;
									}

									/* Assign proper values of velocities and temperatures from old mesh to new mesh */
									for( tNode_I = 0; tNode_I < 4; tNode_I++ ) {
										SnacRemesher_InterpolateNode(
											context,
											contextExt,
											tNode_I,
											shape,
											node_lI,
											shadowElementNodesN[nsub[tetra_I*4+tNode_I]],
											rn_I,
											shadowElementNodesN[nsub[tetra_I*4+tNode_I]] -
											context->mesh->nodeGlobalCount,
											meshExt->newNode,
											meshExt->shadowNodeRemote );
									}
									break;
								}
							}
							if( shadowElementNodesN )
								free( shadowElementNodesN );
						}
					}
				}
		All_passed = 1;
		count = 0;
		for( node_lI = 0; node_lI < meshExt->nodeLocalCount; node_lI++ ) {
			//fprintf(stderr,"Interpolating Nodal values rank=%d Node #%d ind=%d\n",context->rank,node_lI,ind[node_lI]);
			if( ind[node_lI] == 0 ) {
				Journal_DPrintf( context->debug,
						 " timeStep=%d rank=%d Node #%d cannot find tetra, New coord=%e %e %e  coord=%e %e %e diff=%e %e %e\n",
						 context->timeStep,context->rank,node_lI,
						 meshExt->newNodeCoord[node_lI][0],meshExt->newNodeCoord[node_lI][1],meshExt->newNodeCoord[node_lI][2],
						 mesh->nodeCoord[node_lI][0],mesh->nodeCoord[node_lI][1],mesh->nodeCoord[node_lI][2],
						 meshExt->newNodeCoord[node_lI][0]-mesh->nodeCoord[node_lI][0],
						 meshExt->newNodeCoord[node_lI][1]-mesh->nodeCoord[node_lI][1],
						 meshExt->newNodeCoord[node_lI][2]-mesh->nodeCoord[node_lI][2]
					);

				All_passed = 0;
				count++;
				//fprintf(stderr,"me=%d failed node=%d\n",context->rank,node_lI);
			}
		}
		if(TOL==0.0)TOL = 1.0e-10;
		else TOL *= 10.0;
		Journal_DPrintf( context->debug," In: %s,rank=%d Increased tolerance: %e, the number of failed nodde: %d\n", __func__, context->rank, TOL,count);
		Journal_Firewall( TOL <= 1.0e-03, context->snacError," In: %s,rank=%d Increased tolerance: %e, the number of failed nodde: %d\n", __func__, context->rank, TOL,count);
	}
	free( ind );
}



void _SnacRemesher_InterpolateNodes_Spherical( void* _context ) {
	Snac_Context*			context = (Snac_Context*)_context;
	SnacRemesher_Context*		contextExt = ExtensionManager_Get(
		context->extensionMgr,
		context,
		SnacRemesher_ContextHandle );
	Mesh*					mesh = context->mesh;
	HexaMD*				decomp = (HexaMD*)mesh->layout->decomp;
	SnacRemesher_Mesh*		meshExt = ExtensionManager_Get(
		context->meshExtensionMgr,
		mesh,
		SnacRemesher_MeshHandle );
	int*				ind;
	Index				d;
	Element_LocalIndex      elX,elY,elZ;
	Element_LocalIndex      nodeX,nodeY,nodeZ;
	Element_NodeIndex		elementNodeCount;
	double				xc[DIM*NODES_PER_ELEMENT];
	double				xcs[DIM*NODES_PER_ELEMENT];
	PartitionIndex			rn_I;
	Node_LocalIndex			node_lI,node_lN;
	unsigned int                            All_passed;
	double                          TOL;
	int                             count;
	const int                          shadowDepth = ( decomp->shadowDepth > 3) ? decomp->shadowDepth : 3;
	const int                          numSearchElement = 2*shadowDepth-1;
	const int                          numSearchDepth = shadowDepth-1;
	const Element_LocalIndex		nElementX=decomp->elementLocal3DCounts[decomp->rank][0];
	const Element_LocalIndex		nElementY=decomp->elementLocal3DCounts[decomp->rank][1];
	const Element_LocalIndex		nElementZ=decomp->elementLocal3DCounts[decomp->rank][2];
	const Node_LocalIndex		nNodeX=decomp->nodeLocal3DCounts[decomp->rank][0];
	const Node_LocalIndex		nNodeY=decomp->nodeLocal3DCounts[decomp->rank][1];
	const Node_LocalIndex		nNodeZ=decomp->nodeLocal3DCounts[decomp->rank][2];

	Journal_DPrintf( context->debug, "In: %s\n", __func__ );

	ind = (int*)malloc( sizeof( int ) * meshExt->nodeLocalCount );
	memset( ind, 0, sizeof( int ) * meshExt->nodeLocalCount );

	/* for all nodes, interpolate velocity; loop over local elements */
	All_passed = 0;
	TOL= 0.0f;
	while(!All_passed) {
		for(nodeZ=0; nodeZ < nNodeZ; nodeZ++)
			for(nodeY=0; nodeY < nNodeY; nodeY++)
				for(nodeX=0; nodeX < nNodeX; nodeX++) {
					Element_NodeIndex       en_N;
					double                  elembbox[2][DIM];
					Node_LocalIndex**       elementNodeTbl = context->mesh->elementNodeTbl;
					Coord                   temp1,temp2;
					Element_LocalIndex      searchElement_lI,searchElement_lJ,searchElement_lK;
					Element_LocalIndex      element_lN;

					int				found;
					Tetrahedra_Index		tetra_I;
					Coord				newNodeCoord;
					Coord				newNodeCoordS;

					/* Ensure we haven't already done this node */
					node_lI = nodeX + nodeY*nNodeX + nodeZ*nNodeX*nNodeY;
					if( ind[node_lI] ) {
						continue;
					}
					elX = (nodeX <= 1)? 0 : (nodeX-1);
					elY = (nodeY <= 1)? 0 : (nodeY-1);
					elZ = (nodeZ <= 1)? 0 : (nodeZ-1);

					/* otherwise, try to interpolate old mesh's values for the new coordinates */
					newNodeCoord[0] = meshExt->newNodeCoord[node_lI][0];
					newNodeCoord[1] = meshExt->newNodeCoord[node_lI][1];
					newNodeCoord[2] = meshExt->newNodeCoord[node_lI][2];
					xyz2tprCoord( newNodeCoord, &newNodeCoordS );
					newNodeCoordS[1] /= 6371000.0;

					for(searchElement_lK=0;searchElement_lK<numSearchElement;searchElement_lK++) {
						if( (elZ+searchElement_lK-numSearchDepth) < 0 || (elZ+searchElement_lK-numSearchDepth) >= nElementZ )
							continue;
						for(searchElement_lJ=0;searchElement_lJ<numSearchElement;searchElement_lJ++) {
							if( (elY+searchElement_lJ-numSearchDepth) < 0 || (elY+searchElement_lJ-numSearchDepth) >= nElementY )
								continue;
							for(searchElement_lI=0;searchElement_lI<numSearchElement;searchElement_lI++) {
								if( (elX+searchElement_lI-numSearchDepth) < 0 || (elX+searchElement_lI-numSearchDepth) >= nElementX )
									continue;
								element_lN = (elX+searchElement_lI-numSearchDepth) + (elY+searchElement_lJ-numSearchDepth)*nElementX
									+ (elZ+searchElement_lK-numSearchDepth)*nElementY*nElementX;

								if( ind[node_lI] ) {
									break;
								}

								elementNodeCount = context->mesh->elementNodeCountTbl[element_lN];
								if( elementNodeCount != (unsigned)NODES_PER_ELEMENT ) {
									printf( "elementNodeCount != NODES_PER_ELEMENT  element_lN = %d", element_lN );
									assert( 0 );
								}

								/* create a local copy of the veocity and coords for each of the element's nodes of the old mesh */
								for( en_N = 0; en_N < NODES_PER_ELEMENT; en_N++) {

									node_lN = elementNodeTbl[element_lN][en_N];
									for( d = 0; d < DIM; d++ ) {
										xc[en_N*DIM+d] = Mesh_CoordAt( context->mesh, node_lN )[d];
									}
									// (0,1,2) <=> (x,y,z) <=> (fi,r,theta)
									temp1[0] = xc[en_N*DIM+0];
									temp1[1] = xc[en_N*DIM+1];
									temp1[2] = xc[en_N*DIM+2];
									xyz2tprCoord( temp1, &temp2 );
									xcs[en_N*DIM+0] = temp2[0];
									xcs[en_N*DIM+1] = temp2[1]/6371000.0;
									xcs[en_N*DIM+2] = temp2[2];
								}

								/* Calculate element's bounding box */
								for( d = 0; d < DIM; d++ ) {
									elembbox[0][d] = DBL_MAX;
									elembbox[1][d] = DBL_MIN;
								}
								for( en_N = 0; en_N < NODES_PER_ELEMENT; en_N++ ) {
									for( d = 0; d < DIM; ++d ) {
										elembbox[0][d] = MIN( elembbox[0][d], xcs[en_N*DIM+d] );
										elembbox[1][d] = MAX( elembbox[1][d], xcs[en_N*DIM+d] );
									}
								}
								/* If new mesh's current node is outside the bounding box, next loop */
								if(
									newNodeCoordS[0] < elembbox[0][0]-0.5f*(elembbox[1][0]-elembbox[0][0]) ||
									newNodeCoordS[0] > elembbox[1][0]+0.5f*(elembbox[1][0]-elembbox[0][0]) ||
									newNodeCoordS[1] < elembbox[0][1]-0.5f*(elembbox[1][1]-elembbox[0][1]) ||
									newNodeCoordS[1] > elembbox[1][1]+0.5f*(elembbox[1][1]-elembbox[0][1]) ||
									newNodeCoordS[2] < elembbox[0][2]-0.5f*(elembbox[1][2]-elembbox[0][2]) ||
									newNodeCoordS[2] > elembbox[1][2]+0.5f*(elembbox[1][2]-elembbox[0][2]) ) {
									continue;
								}
								/* if the current coord is very close to the initial ones, just copy those initial values rather than bother to compute determinents and add error */
								if( ( meshExt->newNodeCoord[node_lI][0] >= mesh->nodeCoord[node_lI][0] - TOL*(elembbox[1][0]-elembbox[0][0]) && meshExt->newNodeCoord[node_lI][0] <= mesh->nodeCoord[node_lI][0] + TOL*(elembbox[1][0]-elembbox[0][0]) ) &&
								    ( meshExt->newNodeCoord[node_lI][1] >= mesh->nodeCoord[node_lI][1] - TOL*(elembbox[1][1]-elembbox[0][1]) && meshExt->newNodeCoord[node_lI][1] <= mesh->nodeCoord[node_lI][1] + TOL*(elembbox[1][1]-elembbox[0][1]) ) &&
								    ( meshExt->newNodeCoord[node_lI][2] >= mesh->nodeCoord[node_lI][2] - TOL*(elembbox[1][2]-elembbox[0][2]) && meshExt->newNodeCoord[node_lI][2] <= mesh->nodeCoord[node_lI][2] + TOL*(elembbox[1][2]-elembbox[0][2]) ) ) {
									ind[node_lI] = 1;
									continue;
								}

								/* loop over 10 sub tets in a brick element, work out the tetra it is in, and then interpolate */
								found = 0;
								for( tetra_I = 0; tetra_I < Tetrahedra_Count && !found; tetra_I++ ) {
									double				x1[DIM];
									double				x2[DIM];
									double				x3[DIM];
									double				x4[DIM];
									double				dett;
									double				det[4];


									for( d = 0; d < DIM; d++ ) {
										x1[d] = xcs[nsub[tetra_I*4]*DIM+d];
										x2[d] = xcs[nsub[tetra_I*4+1]*DIM+d];
										x3[d] = xcs[nsub[tetra_I*4+2]*DIM+d];
										x4[d] = xcs[nsub[tetra_I*4+3]*DIM+d];
									}

									dett = TetrahedronVolume( x1, x2, x3, x4 );
									det[0] = TetrahedronVolume( x2, x4, x3, newNodeCoordS );
									det[1] = TetrahedronVolume( x3, x4, x1, newNodeCoordS );
									det[2] = TetrahedronVolume( x1, x4, x2, newNodeCoordS );
									det[3] = TetrahedronVolume( x1, x2, x3, newNodeCoordS );

									dett = isLargerThanZero2( dett, dett, TOL );
									det[0] = isLargerThanZero2( det[0], dett, TOL );
									det[1] = isLargerThanZero2( det[1], dett, TOL );
									det[2] = isLargerThanZero2( det[2], dett, TOL );
									det[3] = isLargerThanZero2( det[3], dett, TOL );
									dett = det[0] + det[1] + det[2] + det[3];

									assert( dett != 0.0 );
									if( dett <= 0 )	{
										continue;
									}

									/* found if all det are greater than zero */
									if ( det[0] >= 0.0f && det[1] >= 0.0f && det[2] >= 0.0f && det[3] >= 0.0f ) {
										found = 1;
									}

									if( found ) {
										double				shape[4];
										Index				tNode_I;

										/* mark, such that we dont do it again */
										ind[node_lI] = 1;

										/* Calculate the shape funcs */
										for( tNode_I = 0; tNode_I < 4; tNode_I++ ) {
											shape[tNode_I] = det[tNode_I] / dett;
										}

										/* Assign proper values of velocities and temperatures from old mesh to new mesh */
										for( tNode_I = 0; tNode_I < 4; tNode_I++ ) {

											if( elementNodeTbl[element_lN][nsub[tetra_I*4+tNode_I]] >
											    context->mesh->nodeLocalCount ) {
												int i;

												printf( "element_lI: %u, nsub[tetra_I*4+tNode_I]: %u\n",
													element_lN,
													nsub[tetra_I*4+tNode_I] );
												printf( "elementNodeTbl: %p, { ",
													elementNodeTbl );
												for( i = 0; i < 8; i++ ) {
													printf( "%u, ",
														elementNodeTbl[element_lN][i] );
												}
												printf( "\n" );
												assert(
													elementNodeTbl[element_lN][nsub[tetra_I*4+tNode_I]] >
													context->mesh->nodeLocalCount );
											}

											SnacRemesher_InterpolateNode(
												context,
												contextExt,
												tNode_I,
												shape,
												node_lI,
												elementNodeTbl[element_lN][nsub[tetra_I*4+tNode_I]],
												0,
												0,
												meshExt->newNode,
												0 );
										}
										break;
									}
#if 0
									if(context->rank==0 && node_lI==32)
										fprintf(stderr,"Found? %d TOL=%e timeStep=%d me=%d element_lI=%d node_lI=%d tetra_I=%d\nxc=(%e %e %e) (%e %e %e) (%e %e %e) (%e %e %e)\nnewX=%e %e %e bbox=(%e %e) (%e %e) (%e %e)\ndett=%e %e %e %e %e\n\n",
											found,TOL,context->timeStep,context->rank,element_lN,node_lI,tetra_I,
											x1[0],x1[1],x1[2],x2[0],x2[1],x2[2],x3[0],x3[1],x3[2],x4[0],x4[1],x4[2],
											newNodeCoordS[0],newNodeCoordS[1],newNodeCoordS[2],
											elembbox[0][0],elembbox[1][0],elembbox[0][1],elembbox[1][1],elembbox[0][2],elembbox[1][2],
											det[0],det[1],det[2],det[3],dett);
#endif
								}
							}
						}
					}
					/*
					** Loop over shadow elements
					*/
					for( rn_I = 0; rn_I < meshExt->neighbourRankCount; rn_I++ ) {
						Element_ShadowIndex		element_sN;

						/* Loop over all the shadow elements to find one including node_lI */
						for( element_sN = 0; element_sN < meshExt->shadowElementRemoteCount[rn_I]; element_sN++ ) {
							Element_GlobalIndex		gElement_N = meshExt->shadowElementRemoteArray[rn_I][element_sN];
							Node_Index*			shadowElementNodesN = (Node_Index*)malloc( sizeof(Node_Index) * NODES_PER_ELEMENT );
							Index				en_N;
							double				elembbox[2][DIM];
							Node_GlobalIndex	elementNodeN[8];

							if(ind[node_lI] == 1)
								break;

							/* Figure out node index */
							_HexaEL_BuildCornerIndices( context->mesh->layout->elementLayout, gElement_N, elementNodeN );
							for( en_N = 0; en_N < NODES_PER_ELEMENT; en_N++ ) {
								Node_GlobalIndex    node_gN = elementNodeN[en_N];
								Index               node_lN = Mesh_NodeMapGlobalToLocal( context->mesh, node_gN );
								unsigned int        found = 0;
								if( node_lN < meshExt->nodeLocalCount ) {
									shadowElementNodesN[en_N] = node_lN;
									found = 1;
									elementNodeN[en_N] = node_lN;
								}
								else {
									Node_ShadowIndex		node_sI;
									for( node_sI = 0; node_sI < meshExt->shadowNodeRemoteCount[rn_I]; node_sI++ ) {
										if( node_gN == meshExt->shadowNodeRemoteArray[rn_I][node_sI] ) {
											shadowElementNodesN[en_N] = context->mesh->nodeGlobalCount + node_sI;
											found = 1;
										}
									}
								}
								assert( found ); /* we expected this to be a shadow node, but wasn't in list! */
							}

							/* create a local copy of the temp, vel, and coords for each of the element's nodes of the old mesh */
							for( en_N = 0; en_N < NODES_PER_ELEMENT; en_N++ ) {
								/* copying nodal field values and nodal coordinates of the old mesh for each hex element into local
								   array */
								node_lN = shadowElementNodesN[en_N];
								if( node_lN < context->mesh->nodeLocalCount ) {
									for( d = 0; d < DIM; d++ ) {
										xc[en_N*DIM+d] = Mesh_CoordAt( context->mesh, node_lN )[d];
									}
								}
								else {
									node_lN -= context->mesh->nodeGlobalCount;
									for( d = 0; d < DIM; d++ ) {
										xc[en_N*DIM+d] = meshExt->shadowNodeRemoteCoord[rn_I][node_lN][d];
									}
								}
								// (0,1,2) <=> (x,y,z) <=> (fi,r,theta)
								temp1[0] = xc[en_N*DIM+0];
								temp1[1] = xc[en_N*DIM+1];
								temp1[2] = xc[en_N*DIM+2];
								xyz2tprCoord( temp1, &temp2 );
								xcs[en_N*DIM+0] = temp2[0];
								xcs[en_N*DIM+1] = temp2[1]/6371000.0;
								xcs[en_N*DIM+2] = temp2[2];
							}

							/* Calculate element's bounding box */
							for( d = 0; d < DIM; ++d ) {
								elembbox[0][d] = DBL_MAX;
								elembbox[1][d] = DBL_MIN;
							}
							for( en_N = 0; en_N < NODES_PER_ELEMENT; en_N++ ) {
								for( d = 0; d < DIM; ++d ) {
									elembbox[0][d] = MIN( elembbox[0][d], xcs[en_N*DIM+d] );
									elembbox[1][d] = MAX( elembbox[1][d], xcs[en_N*DIM+d] );
								}
							}
							/* If new mesh's current node is outside the bounding box, next loop */
							if( newNodeCoordS[0] < elembbox[0][0]-0.5f*(elembbox[1][0]-elembbox[0][0]) ||
							    newNodeCoordS[0] > elembbox[1][0]+0.5f*(elembbox[1][0]-elembbox[0][0]) ||
							    newNodeCoordS[1] < elembbox[0][1]-0.5f*(elembbox[1][1]-elembbox[0][1]) ||
							    newNodeCoordS[1] > elembbox[1][1]+0.5f*(elembbox[1][1]-elembbox[0][1]) ||
							    newNodeCoordS[2] < elembbox[0][2]-0.5f*(elembbox[1][2]-elembbox[0][2]) ||
							    newNodeCoordS[2] > elembbox[1][2]+0.5f*(elembbox[1][2]-elembbox[0][2]) )
							{
								continue;
							}

							/* loop over 10 sub tets in a brick element, work out the tetra it is in, and then interpolate */
							found = 0;
							for( tetra_I = 0; tetra_I < Tetrahedra_Count && !found; tetra_I++ ) {
								double				x1[DIM];
								double				x2[DIM];
								double				x3[DIM];
								double				x4[DIM];
								double				dett;
								double				det[4];

								for( d = 0; d < DIM; d++ ) {
									x1[d] = xcs[nsub[tetra_I*4]*DIM+d];
									x2[d] = xcs[nsub[tetra_I*4+1]*DIM+d];
									x3[d] = xcs[nsub[tetra_I*4+2]*DIM+d];
									x4[d] = xcs[nsub[tetra_I*4+3]*DIM+d];
								}

								dett = TetrahedronVolume( x1, x2, x3, x4 );
								det[0] = TetrahedronVolume( x2, x4, x3, newNodeCoordS );
								det[1] = TetrahedronVolume( x3, x4, x1, newNodeCoordS );
								det[2] = TetrahedronVolume( x1, x4, x2, newNodeCoordS );
								det[3] = TetrahedronVolume( x1, x2, x3, newNodeCoordS );

								dett = isLargerThanZero2( dett, dett, TOL );
								det[0] = isLargerThanZero2( det[0], dett, TOL );
								det[1] = isLargerThanZero2( det[1], dett, TOL );
								det[2] = isLargerThanZero2( det[2], dett, TOL );
								det[3] = isLargerThanZero2( det[3], dett, TOL );
								dett = det[0] + det[1] + det[2] + det[3];

								Journal_Firewall( dett != 0.0, context->snacError, "node_lI=%d element_sN=%d me=%d TOL=%e newCoord=%e %e %e\nx1(%d)=   %e %e %e\nx2(%d)=   %e %e %e \nx3(%d)=   %e %e %e\nx4(%d)=   %e %e %e\ndett=%e (%e %e %e %e)\n", node_lI,element_sN,context->rank,TOL, newNodeCoordS[0],newNodeCoordS[1],newNodeCoordS[2],nsub[tetra_I*4],x1[0],x1[1],x1[2],nsub[tetra_I*4+1],x2[0],x2[1],x2[2],nsub[tetra_I*4+2],x3[0],x3[1],x3[2],nsub[tetra_I*4+3],x4[0],x4[1],x4[2],dett,det[0],det[1],det[2],det[3]);
								if( dett < 0 ) {
									continue;
								}

								/* found if all det are greater than zero */
								if( det[0] >= 0.0f && det[1] >= 0.0f && det[2] >= 0.0f && det[3] >= 0.0f ) {
									found = 1;
								}
								if( found ) {
									double				shape[4];
									Index				tNode_I;

									/* mark, such that we dont do it again */
									ind[node_lI] = 1;

									/* Calculate the shape funcs */
									for( tNode_I = 0; tNode_I < 4; tNode_I++ ) {
										shape[tNode_I] = det[tNode_I] / dett;
									}

									/* Assign proper values of velocities and temperatures from old mesh to new mesh */
									for( tNode_I = 0; tNode_I < 4; tNode_I++ ) {
										SnacRemesher_InterpolateNode(
											context,
											contextExt,
											tNode_I,
											shape,
											node_lI,
											shadowElementNodesN[nsub[tetra_I*4+tNode_I]],
											rn_I,
											shadowElementNodesN[nsub[tetra_I*4+tNode_I]] -
											context->mesh->nodeGlobalCount,
											meshExt->newNode,
											meshExt->shadowNodeRemote);
									}
									break;
								}
							}
							if( shadowElementNodesN )
								free( shadowElementNodesN );
						}
					}
				}
		All_passed = 1;
		count = 0;
		for( node_lI = 0; node_lI < meshExt->nodeLocalCount; node_lI++ )
		{
			if( ind[node_lI] == 0 ) {
				All_passed = 0;
				count++;
			}
		}
		if(TOL==0.0)TOL = 1.0e-10;
		else TOL *= 10.0;
		Journal_DPrintf( context->debug," In: %s,rank=%d Increased tolerance: %e, the number of failed nodde: %d\n", __func__, context->rank, TOL,count);
		//Journal_Firewall( TOL <= 1.0e-03, context->snacError," In: %s,rank=%d Increased tolerance: %e, the number of failed nodde: %d\n", __func__, context->rank, TOL,count);
	}

	free( ind );

}

#endif

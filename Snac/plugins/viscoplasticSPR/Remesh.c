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
** $Id: Remesh.c 3250 2006-10-23 06:15:18Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "Node.h"
#include "Element.h"
#include "Mesh.h"
#include "Remesh.h"
#include "Register.h"
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_linalg.h>


void _SnacViscoPlastic_RecoverNode( void* _context, unsigned nodeInd )
{
	Snac_Context*			context = (Snac_Context*)_context;
	Mesh*					mesh = context->mesh;
	MeshLayout*				layout = (MeshLayout*)mesh->layout;
	HexaMD*					decomp = (HexaMD*)layout->decomp;
	NodeLayout*				nLayout = layout->nodeLayout;

	Snac_Node*				node = Snac_Node_At( context, nodeInd );
	SnacViscoPlastic_Node*	nodeExt = ExtensionManager_Get( context->mesh->nodeExtensionMgr, 
															node, 
															SnacViscoPlastic_NodeHandle );
	Coord*					coord = Snac_NodeCoord_P( context, nodeInd );

	Index 					nodeElementCount = context->mesh->nodeElementCountTbl[nodeInd];
	Index 					nodeElement_I;
	Element_DomainIndex*	elements;
	gsl_matrix*				matA;
	gsl_vector* 			vecaPlStrain;
	gsl_vector* 			vecbPlStrain;
	gsl_vector* 			vecaViscosity;
	gsl_vector* 			vecbViscosity;
	Index 	 	 	 	 	i; 
	IJK						ijk;
	Node_GlobalIndex		node_gI = _MeshDecomp_Node_LocalToGlobal1D( decomp, nodeInd );
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
	patchCenterList[0] = nodeInd; 
	if( isBoundaryNode ) {
		patchCenterNum=2;
		intNode_gI = ijk[0]+gNodeI*ijk[1]+gNodeI*gNodeJ*ijk[2];
		patchCenterList[1] = Mesh_NodeMapGlobalToLocal( mesh, intNode_gI );
	}
		
	/* initialize gsl vectors and matrix. */
	matA = gsl_matrix_alloc(4,4); gsl_matrix_set_zero( matA );
	vecaPlStrain = gsl_vector_alloc(4); gsl_vector_set_zero( vecaPlStrain );
	vecbPlStrain = gsl_vector_alloc(4); gsl_vector_set_zero( vecbPlStrain );
	vecaViscosity = gsl_vector_alloc(4); gsl_vector_set_zero( vecaViscosity );
	vecbViscosity = gsl_vector_alloc(4); gsl_vector_set_zero( vecbViscosity );
			
	/* For each patch center */
	for( patchCenterI=0; patchCenterI < patchCenterNum; patchCenterI++ ) {
		/* For each incident element, find inicident tets. */
		for( nodeElement_I = 0; nodeElement_I < nodeElementCount; nodeElement_I++ ) {
			Element_DomainIndex		element_dI = context->mesh->nodeElementTbl[patchCenterList[patchCenterI]][nodeElement_I];

			if( element_dI < mesh->elementDomainCount ) {
				Index elementTetra_I;
				Snac_Element* 				element = Snac_Element_At( context, element_dI );
				SnacViscoPlastic_Element*		elementExt = ExtensionManager_Get( context->mesh->elementExtensionMgr, 
																				   element, 
																				   SnacViscoPlastic_ElementHandle );

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
						double tmp = gsl_vector_get(vecbPlStrain,ii) + positionP[ii]*elementExt->plasticStrain[tetra_I];
						gsl_vector_set(vecbPlStrain,ii,tmp);
						/* 					tmp = gsl_vector_get(vecbViscosity,ii) + positionP[ii]*elementExt->viscosity[tetra_I]; */
						/* 					gsl_vector_set(vecbViscosity,ii,tmp);  */
					
						for(jj=0;jj<4;jj++) {
							tmp = gsl_matrix_get(matA,ii,jj) + positionP[ii]*positionP[jj];
							gsl_matrix_set(matA,ii,jj,tmp);
						}
					} /* end of verteces of a tet. */
/* 					if(context->rank==0 && nodeInd==5375) */
/* 						fprintf(stderr,"rank:%d node:%d elem:%d tet:%d plStrain: %e\n", */
/* 								context->rank,nodeInd,element_dI,tetra_I,elementExt->plasticStrain[tetra_I]); */
				} /* end of incident tets. */
			} /* if within my domain */
		} /* end of incident elements. */
	} /* end of patchCenterI */

		
	/* compute parameter vectors. */
	{
		int s;
		gsl_permutation * p = gsl_permutation_alloc (4);
     
		gsl_linalg_LU_decomp (matA, p, &s);
		gsl_linalg_LU_solve (matA, p, vecbPlStrain, vecaPlStrain);
/* 		gsl_linalg_LU_solve (matA, p, vecbViscosity, vecaViscosity); */
		gsl_permutation_free (p);
	}	

	/* Recover using the parameter vectors. */
	nodeExt->plStrainSPR = gsl_vector_get(vecaPlStrain,0);
/* 	nodeExt->viscositySPR = gsl_vector_get(vecaViscosity,0); */
	for(i=0;i<3;i++) {
		nodeExt->plStrainSPR += gsl_vector_get(vecaPlStrain,i+1)*(*coord)[i];
/* 		nodeExt->viscositySPR += gsl_vector_get(vecaViscosity,i+1)*(*coord)[i]; */
	}

/* 	if(context->rank==0) */
/* 		fprintf(stderr,"rank:%d node:%d plStrainSPR:%e\n",context->rank,nodeInd, */
/* 				nodeExt->plStrainSPR); */

	/* free gsl vectors and matrix. */
	gsl_matrix_free( matA );
	gsl_vector_free( vecaPlStrain );
	gsl_vector_free( vecbPlStrain );
	gsl_vector_free( vecaViscosity );
	gsl_vector_free( vecbViscosity );

	/* Free the element node array. */
	FreeArray( elements );
	
	/* end of recovery. */
}


void _SnacViscoPlastic_InterpolateNode( void* _context,
								   unsigned nodeInd, unsigned elementInd, unsigned tetInd, 
								   unsigned* tetNodeInds, double* weights, 
								   Snac_Node* dstNodes )
{
	Snac_Context*			context = (Snac_Context*)_context;
	Mesh*					mesh = context->mesh;
	NodeLayout*				nLayout = mesh->layout->nodeLayout;
	Snac_Node*				dstNode = (Snac_Node*)ExtensionManager_At( context->mesh->nodeExtensionMgr, 
																	   dstNodes, 
																	   nodeInd );
	SnacViscoPlastic_Node*		dstNodeExt = ExtensionManager_Get( context->mesh->nodeExtensionMgr, 
															   dstNode, 
															   SnacViscoPlastic_NodeHandle );
	Index					tetNode_i;

/* 	SnacViscoPlastic_Node_Print( dstNodeExt ); */

	/* Extract the element's node indices.  Note that there should always be eight of these. */
	Node_DomainIndex*	eltNodes;
	unsigned int nEltNodes = 8;
	{
		Element_GlobalIndex	gEltInd;

		eltNodes = Memory_Alloc_Array( Node_DomainIndex, nEltNodes, "SnacViscoPlastic" );
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

	/* Clear the plastic strain. */
	dstNodeExt->plStrainSPR = 0.0;
/* 	dstNodeExt->viscositySPR = 0.0; */

	/* Loop over each contributing node. */
/* 	fprintf(stderr,"node=%d ",nodeInd ); */
	for( tetNode_i = 0; tetNode_i < 4; tetNode_i++ ) {

		/* Where is this contibution coming from? */
		Snac_Node* srcNode = Snac_Node_At( context, eltNodes[tetNodeInds[tetNode_i]] );
		SnacViscoPlastic_Node*	srcNodeExt = ExtensionManager_Get( context->mesh->nodeExtensionMgr, 
															   srcNode, 
															   SnacViscoPlastic_NodeHandle );

		dstNodeExt->plStrainSPR += srcNodeExt->plStrainSPR * weights[tetNode_i];
/* 		dstNodeExt->viscositySPR += srcNodeExt->viscositySPR * weights[tetNode_i]; */

/* 		fprintf(stderr,"%d:(%d %e %e)  ",tetNode_i, eltNodes[tetNodeInds[tetNode_i]], */
/* 				srcNodeExt->plStrainSPR, weights[tetNode_i]); */
	}
	if( dstNodeExt->plStrainSPR < 0 ) dstNodeExt->plStrainSPR = 0.0;
/* 	fprintf(stderr," %e\n", dstNodeExt->plStrainSPR); */

}


void _SnacViscoPlastic_InterpolateElement( void* _context, 
									  Element_LocalIndex dstEltInd,
									  Tetrahedra_Index dstTetInd )
{
	Snac_Context* 			context = (Snac_Context*)_context;
	Snac_Element*			dstElt = Snac_Element_At( context, dstEltInd );
	SnacViscoPlastic_Element*	dstEltExt = ExtensionManager_Get( context->mesh->elementExtensionMgr, 
															  dstElt,
															  SnacViscoPlastic_ElementHandle );
	double				 	plasticStrain = 0.0;
/* 	double				 	viscosity = 0.0; */
	Index 					i;
	
/*  	fprintf(stderr,"element:%d tet=%d\n",dstEltInd,dstTetInd);  */
	for(i=0;i<4;i++) {
		Snac_Node*			node = Snac_Element_Node_P( context, dstEltInd, TetraToNode[dstTetInd][i] );
		SnacViscoPlastic_Node*	nodeExt = ExtensionManager_Get( context->mesh->nodeExtensionMgr, 
															node, 
															SnacViscoPlastic_NodeHandle );
		plasticStrain += 0.25f*nodeExt->plStrainSPR;
/* 		viscosity += 0.25f*nodeExt->viscositySPR; */

/* 		if( nodeExt->plStrainSPR != 0.0 ) */
/* 		  fprintf(stderr,"\t%d:(%d %e)\n", i, TetraToNode[dstTetInd][i], nodeExt->plStrainSPR); */
	}
	dstEltExt->plasticStrain[dstTetInd] = plasticStrain;
/* 	dstEltExt->viscosity[dstTetInd] = viscosity; */
}

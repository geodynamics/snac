/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003,
**	Pururav Thoutireddy,
**	Californian Institute of Technology, 1200 East California Boulevard, Pasadena, California, 91125, USA.
**	110 Victoria Street, Melbourne, Victoria, 3053, Australia.
**	University of Texas, 1 University Station, Austin, Texas, 78712, USA.
**
** Authors:
**	Pururav Thoutireddy, Staff Scientist, Caltech
**	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
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
** $Id: RemeshElements.c 3259 2006-11-09 20:06:31Z EunseoChoi $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "EntryPoint.h"
#include "Mesh.h"
#include "RemesherElement.h"
#include "Context.h"
#include "Remesh.h"
#include "Register.h"
#include "Utils.h"

#include <string.h>
#include <math.h>
#include <assert.h>
#include <float.h>

void _SnacRemesher_InterpolateElements( void* _context ) {
	Snac_Context*			context = (Snac_Context*)_context;
	SnacRemesher_Context*	contextExt = ExtensionManager_Get( context->extensionMgr, 
																   context, 
																   SnacRemesher_ContextHandle );
	Mesh*					mesh = context->mesh;
	SnacRemesher_Mesh*		meshExt = ExtensionManager_Get( context->meshExtensionMgr,
															mesh, 
															SnacRemesher_MeshHandle );
	HexaMD*					decomp = (HexaMD*)mesh->layout->decomp;
	Element_LocalIndex		element_lI;
	Tetrahedra_Index		tet_I;

	Element_DomainIndex	neldI =  decomp->elementDomain3DCounts[0];
	Element_DomainIndex	neldJ =  decomp->elementDomain3DCounts[1];
	Element_DomainIndex	neldK =  decomp->elementDomain3DCounts[2];

	void createBarycenterGrids( void* _context );
	void computeCoefficients( void* _context );
	void computeBaryCoords( void* _context );

	unsigned int 	have_xneg_shadow, have_xpos_shadow;
	unsigned int 	have_yneg_shadow, have_ypos_shadow;
	unsigned int 	have_zneg_shadow, have_zpos_shadow;
	int     		rankPartition[3];

	/* Decompose rank into partitions in each axis */
	rankPartition[2] = context->rank/(decomp->partition3DCounts[0]*decomp->partition3DCounts[1]);
	rankPartition[1] = (context->rank-rankPartition[2]*decomp->partition3DCounts[0]*decomp->partition3DCounts[1]) / decomp->partition3DCounts[0];
	rankPartition[0] = context->rank-rankPartition[2]*decomp->partition3DCounts[0]*decomp->partition3DCounts[1] - rankPartition[1] * decomp->partition3DCounts[0];
	
	have_xneg_shadow = (((decomp->partition3DCounts[0]>1)&&(rankPartition[0]>0))?1:0);
	have_xpos_shadow = (((decomp->partition3DCounts[0]>1)&&(rankPartition[0]<(decomp->partition3DCounts[0]-1)))?1:0);
	have_yneg_shadow = (((decomp->partition3DCounts[1]>1)&&(rankPartition[1]>0))?1:0);
	have_ypos_shadow = (((decomp->partition3DCounts[1]>1)&&(rankPartition[1]<(decomp->partition3DCounts[1]-1)))?1:0);
	have_zneg_shadow = (((decomp->partition3DCounts[2]>1)&&(rankPartition[2]>0))?1:0);
	have_zpos_shadow = (((decomp->partition3DCounts[2]>1)&&(rankPartition[2]<(decomp->partition3DCounts[2]-1)))?1:0);

	meshExt->local_range_min[0] = (have_xneg_shadow?decomp->shadowDepth:0);
	meshExt->local_range_max[0] = (have_xpos_shadow?(neldI-1-decomp->shadowDepth):(neldI-1));
	meshExt->local_range_min[1] = (have_yneg_shadow?decomp->shadowDepth:0);
	meshExt->local_range_max[1] = (have_ypos_shadow?(neldJ-1-decomp->shadowDepth):(neldJ-1));
	meshExt->local_range_min[2] = (have_zneg_shadow?decomp->shadowDepth:0);
	meshExt->local_range_max[2] = (have_zpos_shadow?(neldK-1-decomp->shadowDepth):(neldK-1));

	/* Create old and new grids of barycenters */
	createBarycenterGrids( context );
	
	/*
	** Free any owned arrays that may still exist from the last node interpolation.
	*/
	FreeArray( meshExt->externalElements );
	meshExt->nExternalElements = 0;
	
	/* Compute coefficients for barycentric coordinates. */
	computeCoefficients( context );

	/* Compute barycentric coordinates of a new local grid w.r.t. the old domain grid. */
	computeBaryCoords( context );

	/* Finally, interpolate on each node in the new local barycenter mesh. */
	for( element_lI = 0; element_lI < mesh->elementLocalCount; element_lI++ )
		for(tet_I=0;tet_I<Tetrahedra_Count;tet_I++) 
			SnacRemesher_InterpolateElement( context, contextExt,
							element_lI, tet_I,
							meshExt->newElements,
							meshExt->barcord[element_lI].elnum,
							meshExt->barcord[element_lI].tetnum );
	
	/* Copy interpolated element variables stored in newElements
	   to the origianl element array. */
	for( element_lI = 0; element_lI < mesh->elementLocalCount; element_lI++ )
		for(tet_I=0;tet_I<Tetrahedra_Count;tet_I++) 
			SnacRemesher_CopyElement( context, contextExt, element_lI, tet_I, 
									  meshExt->newElements ); 
			/* _SnacRemesher_CopyElement( context, element_lI, tet_I, */
			/* 						  meshExt->newElements ); */

}

 
void _SnacRemesher_UpdateElements( void* _context ) {

	Snac_Context*			context = (Snac_Context*)_context;
	Element_LocalIndex	element_lI;

	/* Update all the elements, and in the process work out this processor's minLengthScale */
	for( element_lI = 0; element_lI < context->mesh->elementLocalCount; element_lI++ ) {
		double elementMinLengthScale;
		
		KeyCall( context, context->updateElementK, Snac_UpdateElementMomentum_CallCast* )
			( KeyHandle(context,context->updateElementK),
			  context,
			  element_lI,
			  &elementMinLengthScale );
		if( elementMinLengthScale < context->minLengthScale ) {
			context->minLengthScale = elementMinLengthScale;
		}
	}
}


/*
  Interpolate an element's tetrahedra.
  Note that srcEltInd and srcTetInd are those of the old barycenter grid.
*/
void _SnacRemesher_InterpolateElement( void* _context, 
					Element_LocalIndex	dstEltInd, 
					Tetrahedra_Index	tetInd, 
					SnacRemesher_Element*	dstEltArray, 
					Element_DomainIndex	srcEltInd, 
					Tetrahedra_Index	srcTetInd )
{

	Snac_Context*		context = (Snac_Context*)_context;
	Mesh*			mesh = context->mesh;
	SnacRemesher_Mesh*	meshExt = ExtensionManager_Get( context->meshExtensionMgr,
								mesh, 
								SnacRemesher_MeshHandle );
	HexaMD*			decomp = (HexaMD*)mesh->layout->decomp;
	SnacRemesher_Element* 	dstElt = &dstEltArray[dstEltInd];
	Element_DomainIndex 	eltdI[8],eldI,eldJ,eldK;
	double 			dblMaterial_I;
	Index 			coef_I;

	Element_DomainIndex	neldI =  decomp->elementDomain3DCounts[0];
	Element_DomainIndex	neldJ =  decomp->elementDomain3DCounts[1];
	Element_DomainIndex	neldK =  decomp->elementDomain3DCounts[2];

	enum			{ threeD, xy, undefined } geomType;
	
	geomType = undefined;
	if( neldI>1 && neldJ>1 && neldK>1 ) geomType = threeD;
	else if( neldI>1 && neldJ>1 && neldK==1 ) geomType = xy;
	
	switch(geomType) {
	case undefined:
		Journal_Firewall( 0, context->snacError, "Remeshing is currently allowed only for xy or threeD!!\n");
	case threeD:
		/* Decompose srcEltInd into ijk indexes. */
		eldI = (srcEltInd % neldI);
		eldJ = (((srcEltInd-eldI)/neldI) % neldJ);
		eldK = ((srcEltInd-eldI-eldJ*neldI)/(neldI*neldJ));
		
		/* Eight-node hex defined on the old barycenter grid. */
		eltdI[0] = eldI     + eldJ*neldI     + eldK*neldI*neldJ;
		eltdI[1] = (eldI+1) + eldJ*neldI     + eldK*neldI*neldJ;
		eltdI[2] = (eldI+1) + (eldJ+1)*neldI + eldK*neldI*neldJ;
		eltdI[3] = eldI     + (eldJ+1)*neldI + eldK*neldI*neldJ;
		eltdI[4] = eldI     + eldJ*neldI     + (eldK+1)*neldI*neldJ;
		eltdI[5] = (eldI+1) + eldJ*neldI     + (eldK+1)*neldI*neldJ;
		eltdI[6] = (eldI+1) + (eldJ+1)*neldI + (eldK+1)*neldI*neldJ;
		eltdI[7] = eldI     + (eldJ+1)*neldI + (eldK+1)*neldI*neldJ;

		memset( dstElt->tetra[tetInd].strain, 0, sizeof(StrainTensor) );
		memset( dstElt->tetra[tetInd].stress, 0, sizeof(StressTensor) );
		dblMaterial_I = 0.0;
		dstElt->tetra[tetInd].density = 0.0;

		for(coef_I=0;coef_I<4;coef_I++) {
			/* The actual src elements are the apexes of the tet in the old barycenter grid. */
			/* fprintf(stderr,"dstEl=%d dstTet=%d from src: %d %d\n",dstEltInd,tetInd,eltdI[TetraToNode[srcTetInd][coef_I]],meshExt->orderedToDomain[eltdI[TetraToNode[srcTetInd][coef_I]]]); */
			Snac_Element* srcElt = Snac_Element_At( context, meshExt->orderedToDomain[eltdI[TetraToNode[srcTetInd][coef_I]]] );

			dstElt->tetra[tetInd].strain[0][0] += meshExt->barcord[dstEltInd].L[coef_I]*srcElt->tetra[tetInd].strain[0][0];
			dstElt->tetra[tetInd].strain[1][1] += meshExt->barcord[dstEltInd].L[coef_I]*srcElt->tetra[tetInd].strain[1][1];
			dstElt->tetra[tetInd].strain[2][2] += meshExt->barcord[dstEltInd].L[coef_I]*srcElt->tetra[tetInd].strain[2][2];
			dstElt->tetra[tetInd].strain[0][1] += meshExt->barcord[dstEltInd].L[coef_I]*srcElt->tetra[tetInd].strain[0][1];
			dstElt->tetra[tetInd].strain[0][2] += meshExt->barcord[dstEltInd].L[coef_I]*srcElt->tetra[tetInd].strain[0][2];
			dstElt->tetra[tetInd].strain[1][2] += meshExt->barcord[dstEltInd].L[coef_I]*srcElt->tetra[tetInd].strain[1][2];
			
			dstElt->tetra[tetInd].stress[0][0] += meshExt->barcord[dstEltInd].L[coef_I]*srcElt->tetra[tetInd].stress[0][0];
			dstElt->tetra[tetInd].stress[1][1] += meshExt->barcord[dstEltInd].L[coef_I]*srcElt->tetra[tetInd].stress[1][1];
			dstElt->tetra[tetInd].stress[2][2] += meshExt->barcord[dstEltInd].L[coef_I]*srcElt->tetra[tetInd].stress[2][2];
			dstElt->tetra[tetInd].stress[0][1] += meshExt->barcord[dstEltInd].L[coef_I]*srcElt->tetra[tetInd].stress[0][1];
			dstElt->tetra[tetInd].stress[0][2] += meshExt->barcord[dstEltInd].L[coef_I]*srcElt->tetra[tetInd].stress[0][2];
			dstElt->tetra[tetInd].stress[1][2] += meshExt->barcord[dstEltInd].L[coef_I]*srcElt->tetra[tetInd].stress[1][2];

			dblMaterial_I += meshExt->barcord[dstEltInd].L[coef_I]*srcElt->tetra[tetInd].material_I;
			dstElt->tetra[tetInd].density += meshExt->barcord[dstEltInd].L[coef_I]*srcElt->tetra[tetInd].density;
		}
		dstElt->tetra[tetInd].strain[1][0] = dstElt->tetra[tetInd].strain[0][1];
		dstElt->tetra[tetInd].strain[2][0] = dstElt->tetra[tetInd].strain[0][2];
		dstElt->tetra[tetInd].strain[2][1] = dstElt->tetra[tetInd].strain[1][2];
		dstElt->tetra[tetInd].stress[1][0] = dstElt->tetra[tetInd].stress[0][1];
		dstElt->tetra[tetInd].stress[2][0] = dstElt->tetra[tetInd].stress[0][2];
		dstElt->tetra[tetInd].stress[2][1] = dstElt->tetra[tetInd].stress[1][2];
		dstElt->tetra[tetInd].material_I = (Material_Index)dblMaterial_I;
		break;
	case xy:
		/* Decompose srcEltInd into ijk indexes. */
		eldI = (srcEltInd % neldI);
		eldJ = (((srcEltInd-eldI)/neldI) % neldJ);
		
		/* Eight-node hex defined on the old barycenter grid. */
		eltdI[0] = eldI     + eldJ*neldI;     
		eltdI[1] = (eldI+1) + eldJ*neldI;
		eltdI[2] = (eldI+1) + (eldJ+1)*neldI;
		eltdI[3] = eldI     + (eldJ+1)*neldI;
		
		memset( dstElt->tetra[tetInd].strain, 0, sizeof(StrainTensor) );
		memset( dstElt->tetra[tetInd].stress, 0, sizeof(StressTensor) );
		dblMaterial_I = 0.0;
		dstElt->tetra[tetInd].density = 0.0;

		for(coef_I=0;coef_I<3;coef_I++) {
			/* The actual src elements are the apexes of the tet in the old barycenter grid. */
			Snac_Element* srcElt = Snac_Element_At( context, meshExt->orderedToDomain[eltdI[TriToNode[srcTetInd][coef_I]]] );

			dstElt->tetra[tetInd].strain[0][0] += meshExt->barcord[dstEltInd].L[coef_I]*srcElt->tetra[tetInd].strain[0][0];
			dstElt->tetra[tetInd].strain[1][1] += meshExt->barcord[dstEltInd].L[coef_I]*srcElt->tetra[tetInd].strain[1][1];
			dstElt->tetra[tetInd].strain[2][2] += meshExt->barcord[dstEltInd].L[coef_I]*srcElt->tetra[tetInd].strain[2][2];
			dstElt->tetra[tetInd].strain[0][1] += meshExt->barcord[dstEltInd].L[coef_I]*srcElt->tetra[tetInd].strain[0][1];
			dstElt->tetra[tetInd].strain[0][2] += meshExt->barcord[dstEltInd].L[coef_I]*srcElt->tetra[tetInd].strain[0][2];
			dstElt->tetra[tetInd].strain[1][2] += meshExt->barcord[dstEltInd].L[coef_I]*srcElt->tetra[tetInd].strain[1][2];
			
			dstElt->tetra[tetInd].stress[0][0] += meshExt->barcord[dstEltInd].L[coef_I]*srcElt->tetra[tetInd].stress[0][0];
			dstElt->tetra[tetInd].stress[1][1] += meshExt->barcord[dstEltInd].L[coef_I]*srcElt->tetra[tetInd].stress[1][1];
			dstElt->tetra[tetInd].stress[2][2] += meshExt->barcord[dstEltInd].L[coef_I]*srcElt->tetra[tetInd].stress[2][2];
			dstElt->tetra[tetInd].stress[0][1] += meshExt->barcord[dstEltInd].L[coef_I]*srcElt->tetra[tetInd].stress[0][1];
			dstElt->tetra[tetInd].stress[0][2] += meshExt->barcord[dstEltInd].L[coef_I]*srcElt->tetra[tetInd].stress[0][2];
			dstElt->tetra[tetInd].stress[1][2] += meshExt->barcord[dstEltInd].L[coef_I]*srcElt->tetra[tetInd].stress[1][2];

			dblMaterial_I += meshExt->barcord[dstEltInd].L[coef_I]*srcElt->tetra[tetInd].material_I;
			dstElt->tetra[tetInd].density += meshExt->barcord[dstEltInd].L[coef_I]*srcElt->tetra[tetInd].density;
		}
		dstElt->tetra[tetInd].strain[1][0] = dstElt->tetra[tetInd].strain[0][1];
		dstElt->tetra[tetInd].strain[2][0] = dstElt->tetra[tetInd].strain[0][2];
		dstElt->tetra[tetInd].strain[2][1] = dstElt->tetra[tetInd].strain[1][2];
		dstElt->tetra[tetInd].stress[1][0] = dstElt->tetra[tetInd].stress[0][1];
		dstElt->tetra[tetInd].stress[2][0] = dstElt->tetra[tetInd].stress[0][2];
		dstElt->tetra[tetInd].stress[2][1] = dstElt->tetra[tetInd].stress[1][2];
		dstElt->tetra[tetInd].material_I = (Material_Index)dblMaterial_I;
		break;
	}

}

/*
  Copy interpolated values stored in newElement array 
  back to the original element array. 
*/
void _SnacRemesher_CopyElement( void*					_context, 
								Element_LocalIndex		EltInd, 
								Tetrahedra_Index		tetInd, 
								SnacRemesher_Element*	elementArray )
{

	Snac_Context*		context = (Snac_Context*)_context;
	SnacRemesher_Element* srcElt;
	Snac_Element* dstElt;

	Index i,j;

	srcElt = &elementArray[EltInd];
	dstElt = Snac_Element_At( context, EltInd );
	for( i=0; i<3; i++ )
		for( j=0; j<3; j++ ) {
			dstElt->tetra[tetInd].strain[i][j] = srcElt->tetra[tetInd].strain[i][j]; 
			dstElt->tetra[tetInd].stress[i][j] = srcElt->tetra[tetInd].stress[i][j]; 
		}
	dstElt->tetra[tetInd].density = srcElt->tetra[tetInd].density;
	dstElt->tetra[tetInd].material_I = srcElt->tetra[tetInd].material_I;

}


void createBarycenterGrids( void* _context )
{
	Snac_Context*			context = (Snac_Context*)_context;
	Mesh*					mesh = context->mesh;
	SnacRemesher_Mesh*		meshExt = ExtensionManager_Get( context->meshExtensionMgr,
															mesh, 
															SnacRemesher_MeshHandle );
	MeshLayout*             layout = (MeshLayout*)mesh->layout;
	HexaMD*                 decomp = (HexaMD*)layout->decomp;
	NodeLayout*				nLayout = mesh->layout->nodeLayout;
	Element_LocalIndex		element_lI;
	Element_DomainIndex		element_dI;
	double					tempBC[mesh->elementDomainCount][3];

	/* Loop over the new elements. */
	for( element_lI = 0; element_lI < mesh->elementLocalCount; element_lI++ ) {
		unsigned			nEltNodes;
		Node_DomainIndex*	eltNodes;
        Element_GlobalIndex	gEltInd;

		/* Extract the element's node indices.  Note that there should always be eight of these. */
		{
			nEltNodes = 8;
			eltNodes = Memory_Alloc_Array( Node_DomainIndex, nEltNodes, "SnacRemesher" );
			gEltInd = Mesh_ElementMapLocalToGlobal( mesh, element_lI );
			nLayout->buildElementNodes( nLayout, gEltInd, eltNodes );
		}
		
		/* Convert global node indices to local. */
		{
			unsigned	eltNode_i;
			
			meshExt->newBarycenters[element_lI][0] = 0.0;
			meshExt->newBarycenters[element_lI][1] = 0.0;
			meshExt->newBarycenters[element_lI][2] = 0.0;
			for( eltNode_i = 0; eltNode_i < nEltNodes; eltNode_i++ ) {
				eltNodes[eltNode_i] = Mesh_NodeMapGlobalToLocal( mesh, eltNodes[eltNode_i] );
				meshExt->newBarycenters[element_lI][0] += (meshExt->newNodeCoords[eltNodes[eltNode_i]][0]/nEltNodes);
				meshExt->newBarycenters[element_lI][1] += (meshExt->newNodeCoords[eltNodes[eltNode_i]][1]/nEltNodes);
				meshExt->newBarycenters[element_lI][2] += (meshExt->newNodeCoords[eltNodes[eltNode_i]][2]/nEltNodes);
			}
			/* if(context->rank==13) */
			/* 	fprintf(stderr,"me=%d el=%d newBC=%e %e %e\n",context->rank,element_lI, */
			/* 			meshExt->newBarycenters[element_lI][0],meshExt->newBarycenters[element_lI][1], */
			/* 			meshExt->newBarycenters[element_lI][2]); */

            /* Special treatment to test.
               Since some boundary elements of the new mesh fail to find containing old element,
               push the barycenters "inward" by a small amount so that these elements are forced
               to be inside the old mesh. */
            RegularMeshUtils_Element_1DTo3D( decomp, gEltInd, &elgI, &elgJ, &elgK ); /* Decompose gEltInd into ijk indexes. */
            /* The factor 1.0e-3 is totally experimental.
               To minimize error this treatment introduces into barycentric interpolation,
               a minimum value would have to be found and used. */
            if( elgI == 0 )        meshExt->newBarycenters[element_lI][0] += 1.0e-3 * fabs(meshExt->newBarycenters[element_lI][0]);
            if( elgI == nelgI-1 )  meshExt->newBarycenters[element_lI][0] -= 1.0e-3 * fabs(meshExt->newBarycenters[element_lI][0]);
            if( elgJ == 0 )        meshExt->newBarycenters[element_lI][1] += 1.0e-3 * fabs(meshExt->newBarycenters[element_lI][1]);
            if( elgJ == nelgJ-1 )  meshExt->newBarycenters[element_lI][1] -= 1.0e-3 * fabs(meshExt->newBarycenters[element_lI][1]);
            if( elgK == 0 )        meshExt->newBarycenters[element_lI][2] += 1.0e-3 * fabs(meshExt->newBarycenters[element_lI][2]);
            if( elgK == nelgK-1 )  meshExt->newBarycenters[element_lI][2] -= 1.0e-3 * fabs(meshExt->newBarycenters[element_lI][2]);
		}
	}

	/* Loop over the old elements. */
	for( element_dI = 0; element_dI < mesh->elementDomainCount; element_dI++ ) {
		unsigned			nEltNodes;
		Node_DomainIndex*	eltNodes;
		//Element_GlobalIndex		element_gI;
		
		/* Extract the element's node indices.  Note that there should always be eight of these. */
		{
			Element_GlobalIndex	gEltInd;
			
			nEltNodes = 8;
			eltNodes = Memory_Alloc_Array( Node_DomainIndex, nEltNodes, "SnacRemesher" );
			gEltInd = Mesh_ElementMapDomainToGlobal( mesh, element_dI );
			nLayout->buildElementNodes( nLayout, gEltInd, eltNodes );
		}
		
		/* Convert global node indices to domain. */
		{
			unsigned	eltNode_i;
			tempBC[element_dI][0] = 0.0;
			tempBC[element_dI][1] = 0.0;
			tempBC[element_dI][2] = 0.0;
			for( eltNode_i = 0; eltNode_i < nEltNodes; eltNode_i++ ) {
				eltNodes[eltNode_i] = Mesh_NodeMapGlobalToDomain( mesh, eltNodes[eltNode_i] );
				tempBC[element_dI][0] += (mesh->nodeCoord[eltNodes[eltNode_i]][0]/nEltNodes);
				tempBC[element_dI][1] += (mesh->nodeCoord[eltNodes[eltNode_i]][1]/nEltNodes);
				tempBC[element_dI][2] += (mesh->nodeCoord[eltNodes[eltNode_i]][2]/nEltNodes);
			}
		}
	}
	/* Sort the tempBC array for a structured BC grid. */
	{
		Element_LocalIndex  nLElI = decomp->elementLocal3DCounts[context->rank][0];
		Element_LocalIndex  nLElJ = decomp->elementLocal3DCounts[context->rank][1];
		Element_DomainIndex nDElI = decomp->elementDomain3DCounts[0];
		Element_DomainIndex nDElJ = decomp->elementDomain3DCounts[1];
		Element_DomainIndex nDElK = decomp->elementDomain3DCounts[2];

		unsigned int ghostElementCount = 0;
		unsigned int dI,dJ,dK;

		for( dK=0; dK < nDElK; dK++ )
			for( dJ=0; dJ < nDElJ; dJ++ )
				for( dI=0; dI < nDElI; dI++ ) {
					Element_DomainIndex orderedDomainID = dI + dJ * nDElI + dK * nDElI * nDElJ;

					/* if this domain element is within the "local" bounds,
					   copy the corresponding local element's barycenter to 
					   the current reordered domain index. */
					if( dI >= meshExt->local_range_min[0] && dI <= meshExt->local_range_max[0] &&
					    dJ >= meshExt->local_range_min[1] && dJ <= meshExt->local_range_max[1] &&
					    dK >= meshExt->local_range_min[2] && dK <= meshExt->local_range_max[2] )
						{
							Element_LocalIndex localElNum =
								(dI-meshExt->local_range_min[0]) +
								(dJ-meshExt->local_range_min[1])*nLElI +
								(dK-meshExt->local_range_min[2])*nLElI*nLElJ;
							/* if(context->rank==13)  */
							/* 	fprintf(stderr,"%d/%d %d/%d\n",orderedDomainID,mesh->elementDomainCount-1, */
							/* 			localElNum,mesh->elementLocalCount-1);  */
							memcpy( meshExt->oldBarycenters[orderedDomainID], tempBC[localElNum], sizeof(double)*3 ); 
							meshExt->orderedToDomain[orderedDomainID] = localElNum;
						}
					else { /* if this domain element is a shadow,
							  copy the corresponding domain element's barycenter to 
							  the current reordered domain index. */
						/* if(context->rank==13) */
						/* 	fprintf(stderr,"%d<-(%d/%d) count:%d\n",orderedDomainID,mesh->elementLocalCount+ghostElementCount,mesh->elementDomainCount-1,ghostElementCount); */
						memcpy( meshExt->oldBarycenters[orderedDomainID], tempBC[mesh->elementLocalCount+ghostElementCount], sizeof(double)*3 );
						meshExt->orderedToDomain[orderedDomainID] = mesh->elementLocalCount+ghostElementCount;
						ghostElementCount++;
					}
					/* if(context->rank==13) */
					/* 	fprintf(stderr,"me=%d el=%d oldBC=%e %e %e\n",context->rank,orderedDomainID, */
					/* 			meshExt->oldBarycenters[orderedDomainID][0], */
					/* 			meshExt->oldBarycenters[orderedDomainID][1], */
					/* 			meshExt->oldBarycenters[orderedDomainID][2]); */
				}
		assert( mesh->elementDomainCount==(mesh->elementLocalCount+ghostElementCount) );
	}
}


void computeCoefficients( void* _context )
{
	Snac_Context*			context = (Snac_Context*)_context;
	Mesh*					mesh = context->mesh;
	SnacRemesher_Mesh*		meshExt = ExtensionManager_Get( context->meshExtensionMgr,
															mesh, 
															SnacRemesher_MeshHandle );
	MeshLayout*             layout = (MeshLayout*)mesh->layout;
	HexaMD*                 decomp = (HexaMD*)layout->decomp;
	Element_DomainIndex		element_dI;
	Element_DomainIndex		eldI,eldJ,eldK; 
	Element_DomainIndex		neldI =  decomp->elementDomain3DCounts[0];
	Element_DomainIndex		neldJ =  decomp->elementDomain3DCounts[1];
	Element_DomainIndex		neldK =  decomp->elementDomain3DCounts[2];
	enum					{ threeD, xy, undefined } geomType;
	
	/* Loop over the old domain elements. */
	geomType = undefined;
	if( neldI>1 && neldJ>1 && neldK>1 ) geomType = threeD;
	else if( neldI>1 && neldJ>1 && neldK==1 ) geomType = xy;
	
	switch(geomType) {
	case undefined:
		Journal_Firewall( 0, context->snacError, "Remeshing is currently allowed only for xy or threeD!!\n");
	case xy:
		for( eldJ = 0; eldJ < neldJ-1; eldJ++ ) 
			for( eldI = 0; eldI < neldI-1; eldI++ ) {
				Coord				hexCrds[4];
				Tetrahedra_Index	tri_I;
				Index				coef_i;
				
				/* Old domain grid is constructed: i.e., ghost elements are included in this grid. */
				element_dI = eldI+eldJ*neldI;
					
				/* Four-node element defined. */
				Vector_Set( hexCrds[0], meshExt->oldBarycenters[eldI     + eldJ*neldI    ] );
				Vector_Set( hexCrds[1], meshExt->oldBarycenters[(eldI+1) + eldJ*neldI    ] );
				Vector_Set( hexCrds[2], meshExt->oldBarycenters[(eldI+1) + (eldJ+1)*neldI] );
				Vector_Set( hexCrds[3], meshExt->oldBarycenters[eldI     + (eldJ+1)*neldI] );
				
				for(tri_I=0;tri_I<2;tri_I++) {
					/*
					  f(r)=lambda0(r)*f(r0)+lambda1(r)*f(r1)+lambda2(r)*f(r2)
					  
					  lambda(r)=inv(T)*(r-r2), 
					  where lambda is the vector of first two coefficients (lambda3=1.0-(lambda1+lambda2),
					  r is the position vector, r2 is the position of the third apex, and finally T is given by
					  [Ta Tb] = [x0-x2 x1-x2]
					  [Tc Td]   [y0-y2 y1-y2]
					*/
					double Ta = hexCrds[TriToNode[tri_I][0]][0] - hexCrds[TriToNode[tri_I][2]][0];
					double Tb = hexCrds[TriToNode[tri_I][1]][0] - hexCrds[TriToNode[tri_I][2]][0];
					double Tc = hexCrds[TriToNode[tri_I][0]][1] - hexCrds[TriToNode[tri_I][2]][1];
					double Td = hexCrds[TriToNode[tri_I][1]][1] - hexCrds[TriToNode[tri_I][2]][1];
					
					double detT = Ta*Td-Tb*Tc;
					/*
					  lambda_i(x,y,z) = invT_ij * (r-r2)_j 
					                  = invT_i0*r_0+invT_i1*r_1 - (invT_i0*r2_0+invT_i1*r2_1)
                   	                  = coef_i0*r_0 + coef_i1*r_1 + coef_i2
					*/
					/* lambda0 coeffs */
					meshExt->barcoef[element_dI].coef[tri_I][0][0] = Td/detT;
					meshExt->barcoef[element_dI].coef[tri_I][0][1] = -Tb/detT;
					meshExt->barcoef[element_dI].coef[tri_I][0][2] = 0.0;
					/* lambda1 coeffs */
					meshExt->barcoef[element_dI].coef[tri_I][1][0] = -Tc/detT;
					meshExt->barcoef[element_dI].coef[tri_I][1][1] = Ta/detT;
					meshExt->barcoef[element_dI].coef[tri_I][1][2] = 0.0;
					/* lambda2 not needed because L2 = 1.0 - (L0 + L1) */
						
					for(coef_i=0;coef_i<2;coef_i++) {
						meshExt->barcoef[element_dI].coef[tri_I][0][2] -= meshExt->barcoef[element_dI].coef[tri_I][0][coef_i]*hexCrds[TriToNode[tri_I][2]][coef_i];
						meshExt->barcoef[element_dI].coef[tri_I][1][2] -= meshExt->barcoef[element_dI].coef[tri_I][1][coef_i]*hexCrds[TriToNode[tri_I][2]][coef_i];
					}
				}
			}
		break;
	case threeD:
		for( eldK = 0; eldK < neldK-1; eldK++ ) 
			for( eldJ = 0; eldJ < neldJ-1; eldJ++ ) 
				for( eldI = 0; eldI < neldI-1; eldI++ ) {
					Coord			hexCrds[8];
					Tetrahedra_Index	tet_I;
					Index			coef_i;
					
					/* Old domain grid is constructed: i.e., ghost elements are included in this grid. */
					element_dI = eldI+eldJ*neldI+eldK*neldI*neldJ;
					
					/* Eight-node hex defined. */
					Vector_Set( hexCrds[0], meshExt->oldBarycenters[eldI     + eldJ*neldI     + eldK*neldI*neldJ    ] );
					Vector_Set( hexCrds[1], meshExt->oldBarycenters[(eldI+1) + eldJ*neldI     + eldK*neldI*neldJ    ] );
					Vector_Set( hexCrds[2], meshExt->oldBarycenters[(eldI+1) + (eldJ+1)*neldI + eldK*neldI*neldJ    ] );
					Vector_Set( hexCrds[3], meshExt->oldBarycenters[eldI     + (eldJ+1)*neldI + eldK*neldI*neldJ    ] );
					Vector_Set( hexCrds[4], meshExt->oldBarycenters[eldI     + eldJ*neldI     + (eldK+1)*neldI*neldJ] );
					Vector_Set( hexCrds[5], meshExt->oldBarycenters[(eldI+1) + eldJ*neldI     + (eldK+1)*neldI*neldJ] );
					Vector_Set( hexCrds[6], meshExt->oldBarycenters[(eldI+1) + (eldJ+1)*neldI + (eldK+1)*neldI*neldJ] );
					Vector_Set( hexCrds[7], meshExt->oldBarycenters[eldI     + (eldJ+1)*neldI + (eldK+1)*neldI*neldJ] );
					
					for(tet_I=0;tet_I<5;tet_I++) {
						/*
						  f(r)=lambda1(r)*f(r1)+lambda2(r)*f(r2)+lambda3(r)*f(r3)+lambda4(r)*f(r4)
						  
						  lambda(r)=inv(T)*(r-r4), 
						  where lambda is the vector of first three coefficients (lambda4=1.0-sum(lambda,1 to 3)).
						  r is the position vector, r4 is the position of the fourth apex, and finally T is given by
						  [Ta Tb Tc]   [x0-x3 x1-x3 x2-x3]
						  [Td Te Tf] = [y0-y3 y1-y3 y2-y3]
						  [Tg Th Tk]   [z0-z3 z1-z3 z2-z3]
						*/
						double Ta = hexCrds[TetraToNode[tet_I][0]][0] - hexCrds[TetraToNode[tet_I][3]][0];
						double Tb = hexCrds[TetraToNode[tet_I][1]][0] - hexCrds[TetraToNode[tet_I][3]][0];
						double Tc = hexCrds[TetraToNode[tet_I][2]][0] - hexCrds[TetraToNode[tet_I][3]][0];
						double Td = hexCrds[TetraToNode[tet_I][0]][1] - hexCrds[TetraToNode[tet_I][3]][1];
						double Te = hexCrds[TetraToNode[tet_I][1]][1] - hexCrds[TetraToNode[tet_I][3]][1];
						double Tf = hexCrds[TetraToNode[tet_I][2]][1] - hexCrds[TetraToNode[tet_I][3]][1];
						double Tg = hexCrds[TetraToNode[tet_I][0]][2] - hexCrds[TetraToNode[tet_I][3]][2];
						double Th = hexCrds[TetraToNode[tet_I][1]][2] - hexCrds[TetraToNode[tet_I][3]][2];
						double Tk = hexCrds[TetraToNode[tet_I][2]][2] - hexCrds[TetraToNode[tet_I][3]][2];
						
						double detT = Ta*(Te*Tk-Tf*Th) + Tb*(Tf*Tg-Tk*Td) + Tc*(Td*Th-Te*Tg);
						/*
						  lambda_i(x,y,z) = invT_ij * (r-r4)_j 
						  = invT_i1*r_1+invT_i2*r_2+invT_i3*r_3 - (invT_i1*r4_1+invT_i2*r4_2+invT_i3*r4_3)
						  = coef_i1*r_1 + coef_i2*r_2 + coef_i3*r_3 + coef_i4
						*/
						/* lambda1 coeffs */
						meshExt->barcoef[element_dI].coef[tet_I][0][0] = (Te*Tk-Tf*Th)/detT;
						meshExt->barcoef[element_dI].coef[tet_I][0][1] = (Tc*Th-Tb*Tk)/detT;
						meshExt->barcoef[element_dI].coef[tet_I][0][2] = (Tb*Tf-Tc*Te)/detT;
						meshExt->barcoef[element_dI].coef[tet_I][0][3] = 0.0;
						/* lambda2 coeffs */
						meshExt->barcoef[element_dI].coef[tet_I][1][0] = (Tf*Tg-Td*Tk)/detT;
						meshExt->barcoef[element_dI].coef[tet_I][1][1] = (Ta*Tk-Tc*Tg)/detT;
						meshExt->barcoef[element_dI].coef[tet_I][1][2] = (Tc*Td-Ta*Tf)/detT;
						meshExt->barcoef[element_dI].coef[tet_I][1][3] = 0.0;
						/* lambda3 coeffs */
						meshExt->barcoef[element_dI].coef[tet_I][2][0] = (Td*Th-Te*Tg)/detT;
						meshExt->barcoef[element_dI].coef[tet_I][2][1] = (Tg*Tb-Ta*Th)/detT;
						meshExt->barcoef[element_dI].coef[tet_I][2][2] = (Ta*Te-Tb*Td)/detT;
						meshExt->barcoef[element_dI].coef[tet_I][2][3] = 0.0;
						/* L4 not needed because L4 = 1.0 - (L1 + L2 + L3) */
						
						for(coef_i=0;coef_i<3;coef_i++) {
							meshExt->barcoef[element_dI].coef[tet_I][0][3] -= meshExt->barcoef[element_dI].coef[tet_I][0][coef_i]*hexCrds[TetraToNode[tet_I][3]][coef_i];
							meshExt->barcoef[element_dI].coef[tet_I][1][3] -= meshExt->barcoef[element_dI].coef[tet_I][1][coef_i]*hexCrds[TetraToNode[tet_I][3]][coef_i];
							meshExt->barcoef[element_dI].coef[tet_I][2][3] -= meshExt->barcoef[element_dI].coef[tet_I][2][coef_i]*hexCrds[TetraToNode[tet_I][3]][coef_i];
						}
					}
				}
	break;
	default:
		Journal_Firewall( 0, context->snacError, "Unknown 2D_type: Remeshing is currently allowed only for dim >= 2!!\n");
	} /* end of switch(2D_types) */
}


void computeBaryCoords( void* _context )
{
	Snac_Context*			context = (Snac_Context*)_context;
	Mesh*					mesh = context->mesh;
	SnacRemesher_Mesh*		meshExt = ExtensionManager_Get( context->meshExtensionMgr,
															mesh, 
															SnacRemesher_MeshHandle );
	MeshLayout*             layout = (MeshLayout*)mesh->layout;
	HexaMD*                 decomp = (HexaMD*)layout->decomp;
	Element_LocalIndex		element_lI;
	Element_DomainIndex		element_dI;

	Element_DomainIndex		ellI,ellJ,ellK; 
	Element_DomainIndex		nellI = decomp->elementLocal3DCounts[decomp->rank][0];
	Element_DomainIndex		nellJ = decomp->elementLocal3DCounts[decomp->rank][1];
	Element_DomainIndex		nellK = decomp->elementLocal3DCounts[decomp->rank][2];
	Element_DomainIndex		neldI =  decomp->elementDomain3DCounts[0];
	Element_DomainIndex		neldJ =  decomp->elementDomain3DCounts[1];
	Element_DomainIndex		neldK =  decomp->elementDomain3DCounts[2];
	enum					{ threeD, xy, undefined } geomType;
	
	int dist_compare( const void* dist1, const void* dist2 ); 

	/* First find a containing tet in the grid of old hex's barycenters 
	   and compute barycentric coordinates. */
	geomType = undefined;
	if( neldI>1 && neldJ>1 && neldK>1 ) geomType = threeD;
	else if( neldI>1 && neldJ>1 && neldK==1 ) geomType = xy;
	
	switch(geomType) {
	case undefined:
		Journal_Firewall( 0, context->snacError, "Remeshing is currently allowed only for xy or threeD!!\n");
	case threeD:
		for( ellK = 0; ellK < nellK; ellK++ ) 
			for( ellJ = 0; ellJ < nellJ; ellJ++ ) 
				for( ellI = 0; ellI < nellI; ellI++ ) {
					unsigned			found_tet;
					
					Element_DomainIndex	eldI,eldJ,eldK; 
					Element_DomainIndex	neldI =  decomp->elementDomain3DCounts[0];
					Element_DomainIndex	neldJ =  decomp->elementDomain3DCounts[1];
					Element_DomainIndex	neldK =  decomp->elementDomain3DCounts[2];
					Element_DomainIndex	mindI,mindJ,mindK,maxdI,maxdJ,maxdK,searchI;
					Tetrahedra_Index	tet_I;
					
					Element_DomainIndex	closest_dI=0;
					Tetrahedra_Index	closest_tI=0;
					double				lambda_sqrd = 0.0;
					double				lambda_sqrd_min = 1e+16;
					
					/* Grid of new barycenters is constructed without ghost elements included. */
					element_lI = ellI+ellJ*nellI+ellK*nellI*nellJ;
					
					/* Start searching near the element in old barycenter grid 
					   of which domain element number is equal to element_lI. */ 
					searchI = decomp->shadowDepth;
					mindI = (((meshExt->local_range_min[0]+ellI)<=searchI)?0:((meshExt->local_range_min[0]+ellI)-searchI));
					mindJ = (((meshExt->local_range_min[1]+ellJ)<=searchI)?0:((meshExt->local_range_min[1]+ellJ)-searchI));
					mindK = (((meshExt->local_range_min[2]+ellK)<=searchI)?0:((meshExt->local_range_min[2]+ellK)-searchI));
					maxdI = ((((meshExt->local_range_min[0]+ellI)+searchI)>=neldI)?(neldI-1):((meshExt->local_range_min[0]+ellI)+searchI));
					maxdJ = ((((meshExt->local_range_min[1]+ellJ)+searchI)>=neldJ)?(neldJ-1):((meshExt->local_range_min[1]+ellJ)+searchI));
					maxdK = ((((meshExt->local_range_min[2]+ellK)+searchI)>=neldK)?(neldK-1):((meshExt->local_range_min[2]+ellK)+searchI));
					if(mindI==maxdI) maxdI++;
					if(mindJ==maxdJ) maxdJ++;
					if(mindK==maxdK) maxdK++;
					
					found_tet = 0;
					
					for( eldK = mindK; eldK < maxdK; eldK++ ) {
						for( eldJ = mindJ; eldJ < maxdJ; eldJ++ ) {
							for( eldI = mindI; eldI < maxdI; eldI++ ) {
								element_dI = eldI+eldJ*neldI+eldK*neldI*neldJ;
								for(tet_I=0;tet_I<5;tet_I++) {
									Index	coef_I;
									double 	lambda[4];
									double 	tol_error = 1e-6;
									
									lambda[3] = 1.0;
									for(coef_I=0;coef_I<3;coef_I++) {
										lambda[coef_I] =
											meshExt->barcoef[element_dI].coef[tet_I][coef_I][0] * meshExt->newBarycenters[element_lI][0] +
											meshExt->barcoef[element_dI].coef[tet_I][coef_I][1] * meshExt->newBarycenters[element_lI][1] +
											meshExt->barcoef[element_dI].coef[tet_I][coef_I][2] * meshExt->newBarycenters[element_lI][2] +
											meshExt->barcoef[element_dI].coef[tet_I][coef_I][3];
										lambda[3] -= lambda[coef_I];
									}
									for(coef_I=0;coef_I<4;coef_I++) {
										if( fabs(lambda[coef_I]) < tol_error ) lambda[coef_I] = 0.0;
									}
									/* Keep track of closest element in case the current new barycenter is outside of the old grid. */
									lambda_sqrd = 0.0;
									for(coef_I=0;coef_I<4;coef_I++) 
										lambda_sqrd += lambda[coef_I]*lambda[coef_I];
									if( lambda_sqrd < lambda_sqrd_min ) {
										lambda_sqrd_min = lambda_sqrd;
										closest_dI = element_dI;
										closest_tI = tet_I;
									}
									
									/* See if the barycenter is inside this tet. */
									if( (lambda[0] >= -tol_error && lambda[0] <= (1.0+tol_error)) &&
										(lambda[1] >= -tol_error && lambda[1] <= (1.0+tol_error)) &&
										(lambda[2] >= -tol_error && lambda[2] <= (1.0+tol_error)) &&
										(lambda[3] >= -tol_error && lambda[3] <= (1.0+tol_error)) ) {
										found_tet = 1;
										memcpy( meshExt->barcord[element_lI].L, lambda, sizeof(double)*4);
										meshExt->barcord[element_lI].elnum = element_dI;
										meshExt->barcord[element_lI].tetnum = tet_I;

										break; /* break tet loop */
									}
								} /* end of tet loop */
								if( found_tet ) break; /* break eldI loop */
							}
							if( found_tet ) break; /* break eldJ loop */
						}
						if( found_tet ) break; /* break eldK loop */
					}
					
					/* see if the point is outside of the old mesh.
					   Assumed is that the current new barycenter is at least closest to "closest_dI" element. */
					if( !found_tet ) { 
						Index 					dim_I;
						Coord					hexCrds[8];
						struct dist_id_pair 			dist_apexes[4];
						double 					dist_sum;
						Index 					apex_I;
						
						/* Decompose closest_dI into ijk indexes. */
						eldI = closest_dI % neldI;
						eldJ = ((closest_dI-eldI)/neldI) % neldJ;
						eldK = (closest_dI-eldI-eldJ*neldI)/(neldI*neldJ);
						
						/* Eight-node hex defined. */
						Vector_Set( hexCrds[0], meshExt->oldBarycenters[eldI+eldJ*neldI+eldK*neldI*neldJ] );
						Vector_Set( hexCrds[1], meshExt->oldBarycenters[eldI+1+eldJ*neldI+eldK*neldI*neldJ] );
						Vector_Set( hexCrds[2], meshExt->oldBarycenters[eldI+1+(eldJ+1)*neldI+eldK*neldI*neldJ] );
						Vector_Set( hexCrds[3], meshExt->oldBarycenters[eldI+(eldJ+1)*neldI+eldK*neldI*neldJ] );
						Vector_Set( hexCrds[4], meshExt->oldBarycenters[eldI+eldJ*neldI+(eldK+1)*neldI*neldJ] );
						Vector_Set( hexCrds[5], meshExt->oldBarycenters[eldI+1+eldJ*neldI+(eldK+1)*neldI*neldJ] );
						Vector_Set( hexCrds[6], meshExt->oldBarycenters[eldI+1+(eldJ+1)*neldI+(eldK+1)*neldI*neldJ] );
						Vector_Set( hexCrds[7], meshExt->oldBarycenters[eldI+(eldJ+1)*neldI+(eldK+1)*neldI*neldJ] );
						
						/* compute distances to tet apexes to find the closest three (supposedly forming the closest face).*/
						for( apex_I = 0; apex_I < 4; apex_I++ ) {
							double tmp = 0.0;
							for( dim_I = 0; dim_I < 3; dim_I++ ) 
								tmp += pow((meshExt->newBarycenters[element_lI][dim_I]-hexCrds[TetraToNode[closest_tI][apex_I]][dim_I]),2.0);
							dist_apexes[apex_I].dist = sqrt(tmp);
							dist_apexes[apex_I].id = apex_I;
						}
						qsort( (void *)dist_apexes, 4, sizeof(struct dist_id_pair), dist_compare ); /* dist arrary sorted in the ascending order. */
						
						dist_sum = 0.0;
						for( apex_I = 0; apex_I < 3; apex_I++ ) 
							dist_sum += (1.0/dist_apexes[apex_I].dist);
						
						found_tet = 1;
						for( apex_I = 0; apex_I < 3; apex_I++ ) 
							meshExt->barcord[element_lI].L[dist_apexes[apex_I].id] = (1.0/dist_apexes[apex_I].dist)/dist_sum;
						meshExt->barcord[element_lI].L[dist_apexes[3].id] = 0.0;
						meshExt->barcord[element_lI].elnum = closest_dI;
						meshExt->barcord[element_lI].tetnum = closest_tI;
					}
				} /* end of local element loop */
		break;
	case xy:
		for( ellJ = 0; ellJ < nellJ; ellJ++ ) 
			for( ellI = 0; ellI < nellI; ellI++ ) {
				unsigned			found_tet;
					
				Element_DomainIndex	eldI,eldJ; 
				Element_DomainIndex	neldI =  decomp->elementDomain3DCounts[0];
				Element_DomainIndex	neldJ =  decomp->elementDomain3DCounts[1];
				Element_DomainIndex	mindI,mindJ,maxdI,maxdJ,searchI;
				Tetrahedra_Index	tri_I;
				
				Element_DomainIndex	closest_dI=0;
				Tetrahedra_Index	closest_tI=0;
				double				lambda_sqrd = 0.0;
				double				lambda_sqrd_min = 1e+16;
				
				/* Grid of new barycenters is constructed without ghost elements included. */
				element_lI = ellI+ellJ*nellI;
				
				/* Start searching near the element in old barycenter grid 
				   of which domain element number is equal to element_lI. */ 
				searchI = decomp->shadowDepth;
				mindI = (((meshExt->local_range_min[0]+ellI)<=searchI)?0:((meshExt->local_range_min[0]+ellI)-searchI));
				mindJ = (((meshExt->local_range_min[1]+ellJ)<=searchI)?0:((meshExt->local_range_min[1]+ellJ)-searchI));
				maxdI = ((((meshExt->local_range_min[0]+ellI)+searchI)>=neldI)?(neldI-1):((meshExt->local_range_min[0]+ellI)+searchI));
				maxdJ = ((((meshExt->local_range_min[1]+ellJ)+searchI)>=neldJ)?(neldJ-1):((meshExt->local_range_min[1]+ellJ)+searchI));
				found_tet = 0;
				
				/* if(context->rank==1) { */
				/* 	fprintf(stderr,"Looking %d (ijk:%d/%d %d/%d)\n",element_lI,ellI,nellI-1,ellJ,nellJ-1); */
				/* 	fprintf(stderr,"           %d to %d/%d %d to %d/%d)\n",mindI,maxdI,neldI-1,mindJ,maxdJ,neldJ-1); */
				/* } */
				for( eldJ = mindJ; eldJ < maxdJ; eldJ++ ) {
					for( eldI = mindI; eldI < maxdI; eldI++ ) {
						element_dI = eldI+eldJ*neldI;
						for(tri_I=0;tri_I<2;tri_I++) {
							Index	coef_I;
							double 	lambda[4];
							double 	tol_error = 1e-4;
									
							lambda[2] = 1.0;
							lambda[3] = 0.0; /* dummy, not to make separate barcord array for different dimensions. */
							for(coef_I=0;coef_I<2;coef_I++) {
								lambda[coef_I] =
									meshExt->barcoef[element_dI].coef[tri_I][coef_I][0] * meshExt->newBarycenters[element_lI][0] +
									meshExt->barcoef[element_dI].coef[tri_I][coef_I][1] * meshExt->newBarycenters[element_lI][1] +
									meshExt->barcoef[element_dI].coef[tri_I][coef_I][2];
								lambda[2] -= lambda[coef_I];
							}
							
							/* Keep track of closest element in case the current new barycenter is outside of the old grid. */
							lambda_sqrd = 0.0;
							for(coef_I=0;coef_I<3;coef_I++) 
								lambda_sqrd += lambda[coef_I]*lambda[coef_I];
							if( lambda_sqrd < lambda_sqrd_min ) {
								lambda_sqrd_min = lambda_sqrd;
								closest_dI = element_dI;
								closest_tI = tri_I;
							}
							
							/* See if the barycenter is inside this tet. */
							if( (lambda[0] >= -tol_error && lambda[0] <= (1.0+tol_error)) &&
								(lambda[1] >= -tol_error && lambda[1] <= (1.0+tol_error)) &&
								(lambda[2] >= -tol_error && lambda[2] <= (1.0+tol_error)) ) {
								found_tet = 1;
								memcpy( meshExt->barcord[element_lI].L, lambda, sizeof(double)*4);
								meshExt->barcord[element_lI].elnum = element_dI;
								meshExt->barcord[element_lI].tetnum = tri_I; /* note that this is the id of triangle in the xy case.*/
							}
							if( found_tet ) break; /* break tri loop */
						} /* end of tri loop */
						if( found_tet ) break; /* break eldI loop */
					} /* end of eldI loop */
					if( found_tet ) break; /* break eldJ loop */
				} /* end of eldJ loop */
	
				/* see if the point is outside of the old mesh.
				   Assumed is that the current new barycenter is at least closest to "closest_dI" element. */
				if( !found_tet ) { 
					Index 					dim_I;
					Coord					hexCrds[4];
					struct dist_id_pair 	dist_apexes[3];
					double 					dist_sum;
					Index 					apex_I;
					
					/* Decompose closest_dI into ijk indexes. */
					eldI = closest_dI % neldI;
					eldJ = ((closest_dI-eldI)/neldI) % neldJ;
					
					/* Four-node element defined. */
					Vector_Set( hexCrds[0], meshExt->oldBarycenters[eldI+eldJ*neldI] );
					Vector_Set( hexCrds[1], meshExt->oldBarycenters[eldI+1+eldJ*neldI] );
					Vector_Set( hexCrds[2], meshExt->oldBarycenters[eldI+1+(eldJ+1)*neldI] );
					Vector_Set( hexCrds[3], meshExt->oldBarycenters[eldI+(eldJ+1)*neldI] );
					
					/* compute distances to tri apexes to find the closest two (supposedly forming the closest edge).*/
					for( apex_I = 0; apex_I < 3; apex_I++ ) {
						double tmp = 0.0;
						for( dim_I = 0; dim_I < 2; dim_I++ ) 
							tmp += pow((meshExt->newBarycenters[element_lI][dim_I]-hexCrds[TriToNode[closest_tI][apex_I]][dim_I]),2.0);
						dist_apexes[apex_I].dist = sqrt(tmp);
						dist_apexes[apex_I].id = apex_I;
					}
					qsort( (void *)dist_apexes, 3, sizeof(struct dist_id_pair), dist_compare ); /* dist arrary sorted in the ascending order. */
					
					dist_sum = 0.0;
					for( apex_I = 0; apex_I < 2; apex_I++ ) 
						dist_sum += (1.0/dist_apexes[apex_I].dist);
					
					found_tet = 1;
					for( apex_I = 0; apex_I < 2; apex_I++ ) 
						meshExt->barcord[element_lI].L[dist_apexes[apex_I].id] = (1.0/dist_apexes[apex_I].dist)/dist_sum;
					meshExt->barcord[element_lI].L[dist_apexes[2].id] = 0.0;
					meshExt->barcord[element_lI].L[3] = 0.0; /* dummy in the xy case. */
					meshExt->barcord[element_lI].elnum = closest_dI;
					meshExt->barcord[element_lI].tetnum = closest_tI; /* note that this is the id of triangle in the xy case.*/
				}
			} /* end of local element loop */
		break;
	} /* end of switch(geomType). */
}

int dist_compare( const void* dist1, const void* dist2 ) {
	double v1 = ((struct dist_id_pair *)dist1)->dist;
	double v2 = ((struct dist_id_pair *)dist2)->dist;
	return (v1<v2) ? -1 : (v1>v2) ? 1 : 0;
};

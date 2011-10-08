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
	Element_DomainIndex		element_dI;
	Tetrahedra_Index		tet_I;

	Element_DomainIndex		ellI,ellJ,ellK; 
	Element_DomainIndex		nellI = decomp->elementLocal3DCounts[decomp->rank][0];
	Element_DomainIndex		nellJ = decomp->elementLocal3DCounts[decomp->rank][1];
	Element_DomainIndex		nellK = decomp->elementLocal3DCounts[decomp->rank][2];
	Element_DomainIndex		eldI,eldJ,eldK; 
	Element_DomainIndex		neldI =  decomp->elementDomain3DCounts[0];
	Element_DomainIndex		neldJ =  decomp->elementDomain3DCounts[1];
	Element_DomainIndex		neldK =  decomp->elementDomain3DCounts[2];
	
	void Tet_Barycenter( Coord tetCrds[4], Coord center );
	void createBarycenterGrids( void* _context );
	int dist_compare( const void* dist1, const void* dist2 ); 

	/* Create old and new grids of barycenters */
	createBarycenterGrids( context );
	
	/*
	** Free any owned arrays that may still exist from the last node interpolation.
	*/
	FreeArray( meshExt->externalElements );
	meshExt->nExternalElements = 0;
	
	/*
	** Interpolate all new elements.
	*/
	
	/* Compute coefficients for barycentric coordinates. */
	for( eldI = 0; eldI < neldI-1; eldI++ ) 
		for( eldJ = 0; eldJ < neldJ-1; eldJ++ ) 
			for( eldK = 0; eldK < neldK-1; eldK++ ) {
				Coord				hexCrds[8];
				Tetrahedra_Index	tet_I;
				Index				coef_i;
				
				/* Old domain grid is constructed: i.e., ghost elements are included in this grid. */
				element_dI = eldI+eldJ*neldI+eldK*neldI*neldJ;

				/* Eight-node hex defined. */
				Vector_Set( hexCrds[0], meshExt->oldBarycenters[eldI+eldJ*neldI+eldK*neldI*neldJ] );
				Vector_Set( hexCrds[1], meshExt->oldBarycenters[eldI+1+eldJ*neldI+eldK*neldI*neldJ] );
				Vector_Set( hexCrds[2], meshExt->oldBarycenters[eldI+1+(eldJ+1)*neldI+eldK*neldI*neldJ] );
				Vector_Set( hexCrds[3], meshExt->oldBarycenters[eldI+(eldJ+1)*neldI+eldK*neldI*neldJ] );
				Vector_Set( hexCrds[4], meshExt->oldBarycenters[eldI+eldJ*neldI+(eldK+1)*neldI*neldJ] );
				Vector_Set( hexCrds[5], meshExt->oldBarycenters[eldI+1+eldJ*neldI+(eldK+1)*neldI*neldJ] );
				Vector_Set( hexCrds[6], meshExt->oldBarycenters[eldI+1+(eldJ+1)*neldI+(eldK+1)*neldI*neldJ] );
				Vector_Set( hexCrds[7], meshExt->oldBarycenters[eldI+(eldJ+1)*neldI+(eldK+1)*neldI*neldJ] );
				
				for(tet_I=0;tet_I<5;tet_I++) {
					/*
					  [Ta Tb Tc] = [x0-x3 x1-x3 x2-x3]
					  [Td Te Tf] = [y0-y3 y1-y3 y2-y3]
					  [Tg Th Tk] = [z0-z3 z1-z3 z2-z3]
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
					  Li(x,y,z) = a0*x+a1*y+a2*z+a3
					*/
					/* L1 coeffs */
					meshExt->barcoef[element_dI].coef[tet_I][0][0] = (Te*Tk-Tf*Th)/detT;
					meshExt->barcoef[element_dI].coef[tet_I][0][1] = (Tc*Th-Tb*Tk)/detT;
					meshExt->barcoef[element_dI].coef[tet_I][0][2] = (Tb*Tf-Tc*Te)/detT;
					meshExt->barcoef[element_dI].coef[tet_I][0][3] = 0.0;
					/* L2 coeffs */
					meshExt->barcoef[element_dI].coef[tet_I][1][0] = (Tf*Tg-Td*Tk)/detT;
					meshExt->barcoef[element_dI].coef[tet_I][1][1] = (Ta*Tk-Tc*Tg)/detT;
					meshExt->barcoef[element_dI].coef[tet_I][1][2] = (Tc*Td-Ta*Tf)/detT;
					meshExt->barcoef[element_dI].coef[tet_I][1][3] = 0.0;
					/* L3 coeffs */
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
	
	/* First find a containing tet in the grid of old hex's barycenters 
	   and compute barycentric coordinates. */
	for( ellK = 0; ellK < nellK; ellK++ ) 
		for( ellJ = 0; ellJ < nellJ; ellJ++ ) 
			for( ellI = 0; ellI < nellI; ellI++ ) {
				unsigned			found_tet;
				
				Element_DomainIndex	eldI,eldJ,eldK; 
				Element_DomainIndex	neldI =  decomp->elementDomain3DCounts[0];
				Element_DomainIndex	neldJ =  decomp->elementDomain3DCounts[1];
				Element_DomainIndex	neldK =  decomp->elementDomain3DCounts[2];
				Element_DomainIndex	mindI,mindJ,mindK,maxdI,maxdJ,maxdK,searchI;

				double				lambda_sqrd = 0.0;
				double				lambda_sqrd_min = 1e+16;
				Element_DomainIndex	closest_dI=0;
				Tetrahedra_Index	closest_tI=0;

				/* Grid of new barycenters is constructed without ghost elements included. */
				element_lI = ellI+ellJ*nellI+ellK*nellI*nellJ;
				
				/* Start searching near the element in old barycenter grid 
				   of which domain element number is equal to element_lI. */ 
				searchI = 2;
				mindI = (((ellI-searchI)<0)?0:(ellI-searchI));
				mindJ = (((ellJ-searchI)<0)?0:(ellJ-searchI));
				mindK = (((ellK-searchI)<0)?0:(ellK-searchI));
				maxdI = (((ellI+searchI)>=neldI)?(neldI-1):(ellI+searchI));
				maxdJ = (((ellJ+searchI)>=neldJ)?(neldJ-1):(ellJ+searchI));
				maxdK = (((ellK+searchI)>=neldK)?(neldK-1):(ellK+searchI));
				found_tet = 0;
				for( eldK = mindK; eldK < maxdK; eldK++ ) {
					for( eldJ = mindJ; eldJ < maxdJ; eldJ++ ) {
						for( eldI = mindI; eldI < maxdI; eldI++ ) {
							element_dI = eldI+eldJ*neldI+eldK*neldI*neldJ;
							for(tet_I=0;tet_I<5;tet_I++) {
								Index	coef_I;
								double 	lambda[4];
								double 	tol_error = 1e-4;
								lambda[3] = 1.0;
								for(coef_I=0;coef_I<3;coef_I++) {
									lambda[coef_I] =
										meshExt->newBarycenters[element_lI][0]*meshExt->barcoef[element_dI].coef[tet_I][coef_I][0] +
										meshExt->newBarycenters[element_lI][1]*meshExt->barcoef[element_dI].coef[tet_I][coef_I][1] +
										meshExt->newBarycenters[element_lI][2]*meshExt->barcoef[element_dI].coef[tet_I][coef_I][2] +
										meshExt->barcoef[element_dI].coef[tet_I][coef_I][3];
									lambda[3] -= lambda[coef_I];
								}
								/* Keep track of closest element in case the current new barycenter is outside of the old grid. */
								for(coef_I=0;coef_I<3;coef_I++) 
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
							}
							if( found_tet ) break; /* break eldI loop */
						}
						if( found_tet ) break; /* break eldJ loop */
					}
					if( found_tet ) break; /* break eldK loop */
				} /* end of domain element search */

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
					
					/* compute distances to tet apexes to find the closest three (supposedly forming the closes face).*/
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
			} /* end of loop oever new barycenters. */
	
	/* Finally, ready to interpolation for every element in the new "Local" mesh. */
	for( element_lI = 0; element_lI < mesh->elementLocalCount; element_lI++ ) 
		for(tet_I=0;tet_I<5;tet_I++) 
			SnacRemesher_InterpolateElement( context, contextExt,
											 element_lI, tet_I,
											 meshExt->newElements,
											 meshExt->barcord[element_lI].elnum, 
											 meshExt->barcord[element_lI].tetnum );
	
}

int dist_compare( const void* dist1, const void* dist2 ) {
	double v1 = ((struct dist_id_pair *)dist1)->dist;
	double v2 = ((struct dist_id_pair *)dist2)->dist;
	return (v1<v2) ? -1 : (v1>v2) ? 1 : 0;
};

void createBarycenterGrids( void* _context ) {

	Snac_Context*			context = (Snac_Context*)_context;
	Mesh*					mesh = context->mesh;
	SnacRemesher_Mesh*		meshExt = ExtensionManager_Get( context->meshExtensionMgr,
												  mesh, 
												  SnacRemesher_MeshHandle );
	NodeLayout*				nLayout = mesh->layout->nodeLayout;
	Element_LocalIndex		element_lI;
	Element_DomainIndex		element_dI;

	/* Loop over the new elements. */
	for( element_lI = 0; element_lI < mesh->elementLocalCount; element_lI++ ) {
		unsigned			nEltNodes;
		Node_DomainIndex*	eltNodes;
		
		/* Extract the element's node indices.  Note that there should always be eight of these. */
		{
			Element_GlobalIndex	gEltInd;
			
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
		
		/* Convert global node indices to local. */
		{
			unsigned	eltNode_i;
			
			meshExt->oldBarycenters[element_dI][0] = 0.0;
			meshExt->oldBarycenters[element_dI][1] = 0.0;
			meshExt->oldBarycenters[element_dI][2] = 0.0;
			for( eltNode_i = 0; eltNode_i < nEltNodes; eltNode_i++ ) {
				eltNodes[eltNode_i] = Mesh_NodeMapGlobalToLocal( mesh, eltNodes[eltNode_i] );
				meshExt->oldBarycenters[element_dI][0] += (mesh->nodeCoord[eltNodes[eltNode_i]][0]/nEltNodes);
				meshExt->oldBarycenters[element_dI][1] += (mesh->nodeCoord[eltNodes[eltNode_i]][1]/nEltNodes);
				meshExt->oldBarycenters[element_dI][2] += (mesh->nodeCoord[eltNodes[eltNode_i]][2]/nEltNodes);
			}
		}
	}
		
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
void _SnacRemesher_InterpolateElement( void*				_context, 
									   Element_LocalIndex	dstEltInd, 
									   Tetrahedra_Index		tetInd, 
									   Snac_Element*		dstEltArray, 
									   Element_DomainIndex	srcEltInd, 
									   Tetrahedra_Index		srcTetInd )
{

	Snac_Context*		context = (Snac_Context*)_context;
	Mesh*				mesh = context->mesh;
	SnacRemesher_Mesh*	meshExt = ExtensionManager_Get( context->meshExtensionMgr,
															mesh, 
															SnacRemesher_MeshHandle );
	HexaMD*				decomp = (HexaMD*)mesh->layout->decomp;
	Snac_Element*		dstElt = (Snac_Element*)ExtensionManager_At( context->mesh->elementExtensionMgr, 
																	 dstEltArray, 
																	 dstEltInd );
	Element_DomainIndex eltdI[8],eldI,eldJ,eldK;
	double 				dblMaterial_I;
	Index 				coef_I;

	Element_DomainIndex	neldI =  decomp->elementDomain3DCounts[0];
	Element_DomainIndex	neldJ =  decomp->elementDomain3DCounts[1];

	/* Decompose srcEltInd into ijk indexes. */
	eldI = srcEltInd % neldI;
	eldJ = ((srcEltInd-eldI)/neldI) % neldJ;
	eldK = (srcEltInd-eldI-eldJ*neldI)/(neldI*neldJ);
	
	/* Eight-node hex defined on the old barycenter grid. */
	eltdI[0] = srcEltInd;
	eltdI[1] = (eldI+1) + eldJ*neldI     + eldK*neldI*neldJ;
	eltdI[2] = (eldI+1) + (eldJ+1)*neldI + eldK*neldI*neldJ;
	eltdI[3] = eldI     + (eldJ+1)*neldI + eldK*neldI*neldJ;
	eltdI[4] = eldI     + eldJ*neldI     + (eldK+1)*neldI*neldJ;
	eltdI[5] = (eldI+1) + eldJ*neldI     + (eldK+1)*neldI*neldJ;
	eltdI[6] = (eldI+1) + (eldJ+1)*neldI + (eldK+1)*neldI*neldJ;
	eltdI[7] = eldI     + (eldJ+1)*neldI + (eldK+1)*neldI*neldJ;

	memset( dstElt->tetra[tetInd].strain, 0, sizeof(double)*3*3 );
	memset( dstElt->tetra[tetInd].stress, 0, sizeof(double)*3*3 );
	dblMaterial_I = 0.0;
	dstElt->tetra[tetInd].density = 0.0;
	for(coef_I=0;coef_I<4;coef_I++) {
		/* The actual src elements are the apexes of the tet in the old barycenter grid. */
		Snac_Element* srcElt = Snac_Element_At( context, eltdI[TetraToNode[srcTetInd][coef_I]] );

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
	dstElt->tetra[tetInd].material_I = (Material_Index)dblMaterial_I;

}


#if 0
void _SnacRemesher_InterpolateElement( void*				_context, 
									   Element_LocalIndex	dstEltInd, 
									   Tetrahedra_Index		dstTetInd, 
									   Snac_Element*		dstEltArray, 
									   Element_DomainIndex	srcEltInd, 
									   Tetrahedra_Index		srcTetInd )
{

	Snac_Context*	context = (Snac_Context*)_context;
	Snac_Element*	dstElt = (Snac_Element*)ExtensionManager_At( context->mesh->elementExtensionMgr, 
								     dstEltArray, 
								     dstEltInd );
	Snac_Element*	srcElt = Snac_Element_At( context, srcEltInd );

	/* Copy the whole structure from the nearest tet in the old mesh. */
	memcpy( &(dstElt->tetra[dstTetInd]), &(srcElt->tetra[srcTetInd]), sizeof(Snac_Element_Tetrahedra) );

}
#endif


/*
** Calculate the barycenter of a tetrahedron.
*/

void Tet_Barycenter( Coord tetCrds[4], Coord center ) {
	unsigned	tet_i;
	
	center[0] = tetCrds[0][0];
	center[1] = tetCrds[0][1];
	center[2] = tetCrds[0][2];
	
	for( tet_i = 1; tet_i < 4; tet_i++ ) {
		center[0] += tetCrds[tet_i][0];
		center[1] += tetCrds[tet_i][1];
		center[2] += tetCrds[tet_i][2];
	}
	
	center[0] *= 0.25;
	center[1] *= 0.25;
	center[2] *= 0.25;
}





#if 0

#define DIM 3

#ifndef SQR
	#define SQR(a) ((a)*(a))
#endif
#ifndef VECDIST3
	#define VECDIST3(a,b)		( sqrt( SQR((a)[0]-(b)[0]) + SQR((a)[1]-(b)[1]) + SQR((a)[2]-(b)[2])  ) )
#endif

void _SnacRemesher_InterpolateElement(
		void*			_context,
		Element_LocalIndex	element_lI,
		Tetrahedra_Index	tetra_I,
		Element_LocalIndex	fromElement_lI,
		Partition_Index		fromElement_rn_I,
		Element_ShadowIndex	fromElement_sI,
		Tetrahedra_Index	fromTetra_I,
		Snac_Element*		newElement,
		Snac_Element**		shadowElement )
{
	Snac_Context*				context = (Snac_Context*)_context;
	Snac_Element*				element = (Snac_Element*)ExtensionManager_At(
                                                                             context->mesh->elementExtensionMgr,
                                                                             newElement,
                                                                             element_lI );
	Snac_Element*				fromElement;

	/* Deference source element (local/shadow) */
	if( fromElement_lI < context->mesh->elementLocalCount ) {
		fromElement = Snac_Element_At( context, fromElement_lI );
	}
	else {
		fromElement = (Snac_Element*)ExtensionManager_At(
                                                         context->mesh->elementExtensionMgr,
                                                         shadowElement[fromElement_rn_I],
                                                         fromElement_sI );
	}

	element->tetra[tetra_I].strain[0][0] = fromElement->tetra[fromTetra_I].strain[0][0];
	element->tetra[tetra_I].strain[1][1] = fromElement->tetra[fromTetra_I].strain[1][1];
	element->tetra[tetra_I].strain[2][2] = fromElement->tetra[fromTetra_I].strain[2][2];
	element->tetra[tetra_I].strain[0][1] = fromElement->tetra[fromTetra_I].strain[0][1];
	element->tetra[tetra_I].strain[0][2] = fromElement->tetra[fromTetra_I].strain[0][2];
	element->tetra[tetra_I].strain[1][2] = fromElement->tetra[fromTetra_I].strain[1][2];

	element->tetraStress[tetra_I][0][0] = fromElement->tetraStress[fromTetra_I][0][0];
	element->tetraStress[tetra_I][1][1] = fromElement->tetraStress[fromTetra_I][1][1];
	element->tetraStress[tetra_I][2][2] = fromElement->tetraStress[fromTetra_I][2][2];
	element->tetraStress[tetra_I][0][1] = fromElement->tetraStress[fromTetra_I][0][1];
	element->tetraStress[tetra_I][0][2] = fromElement->tetraStress[fromTetra_I][0][2];
	element->tetraStress[tetra_I][1][2] = fromElement->tetraStress[fromTetra_I][1][2];
}


void _SnacRemesher_InterpolateElements( void* _context ) {
	Snac_Context*			context = (Snac_Context*)_context;
	SnacRemesher_Context*	contextExt = ExtensionManager_Get(
                                                       context->extensionMgr,
                                                       context,
                                                       SnacRemesher_ContextHandle );
	Mesh*				mesh = (Mesh*)context->mesh;
	SnacRemesher_Mesh*		meshExt = ExtensionManager_Get(
                                                           context->meshExtensionMgr,
                                                           mesh,
                                                           SnacRemesher_MeshHandle );
	MeshLayout*			layout = mesh->layout;
	HexaMD*				decomp = (HexaMD*)layout->decomp;
	Topology*			topology = layout->elementLayout->topology;
	ElementLayout*			elementLayout = layout->elementLayout;
	Element_LocalIndex		element_lI;

	Partition_Index			rn_I;
	Coord**				sbc;
	double*				xbc;
	int				n,i,d;

	Journal_DPrintf( context->debug, "In: %s\n", __func__ );

	/* Calculate the barycentric coords for each tetra of the old mesh */
	xbc = calloc( DIM * Tetrahedra_Count * mesh->elementLocalCount, sizeof(double) );
	for( element_lI = 0; element_lI < mesh->elementLocalCount; element_lI++ ) {
		Tetrahedra_Index		tetra_I;

		for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
			double				*x0;
			double				*x1;
			double				*x2;
			double				*x3;

			x0=Snac_Element_NodeCoord( context, element_lI, TetraToNode[tetra_I][0] );
			x1=Snac_Element_NodeCoord( context, element_lI, TetraToNode[tetra_I][1] );
			x2=Snac_Element_NodeCoord( context, element_lI, TetraToNode[tetra_I][2] );
			x3=Snac_Element_NodeCoord( context, element_lI, TetraToNode[tetra_I][3] );
			for( d = 0; d < DIM; d++ ) {
				xbc[(element_lI * Tetrahedra_Count + tetra_I) * DIM + d]= 0.25 * (x0[d] + x1[d] + x2[d] + x3[d]);
			}
		}
	}

	/* Calculate the barycentric coords for each tetra of the shadows */
	sbc = (Coord**)malloc( sizeof(Coord*) * meshExt->neighbourRankCount );
	for( rn_I = 0; rn_I < meshExt->neighbourRankCount; rn_I++ ) {
		Element_ShadowIndex		element_sI;

		sbc[rn_I] = (Coord*)malloc( sizeof(Coord) * meshExt->shadowElementRemoteCount[rn_I] * Tetrahedra_Count );

		for( element_sI = 0; element_sI < meshExt->shadowElementRemoteCount[rn_I]; element_sI++ ) {
			Element_GlobalIndex		element_gI = meshExt->shadowElementRemoteArray[rn_I][element_sI];
			Node_GlobalIndex		elementNode[8];
			Coord				coord[8];
			Element_NodeIndex		en_I;
			Tetrahedra_Index		tetra_I;

			/* Build a temporary table of the coordinates of the points of this element. */
			_HexaEL_BuildCornerIndices( elementLayout, element_gI, elementNode );
			for( en_I = 0; en_I < 8; en_I++ ) {
				Node_GlobalIndex	node_gI = elementNode[en_I];
				Node_LocalIndex		node_lI = Mesh_NodeMapGlobalToLocal( mesh, node_gI );

				/* Grab the coordinates of the element node. We do this by working out if the node is local and if
				   so, directly copying the value from the mesh. If it isn't, it'll be in the shadow arrays...
				   search for it and copy it. */
				if( node_lI < meshExt->nodeLocalCount ) {
					coord[en_I][0] = mesh->nodeCoord[node_lI][0];
					coord[en_I][1] = mesh->nodeCoord[node_lI][1];
					coord[en_I][2] = mesh->nodeCoord[node_lI][2];
				}
				else {
					/*Partition_Index		rn_I2;

					for( rn_I2 = 0; rn_I2 < meshExt->neighbourRankCount; rn_I2++ ) {*/
						Node_ShadowIndex		node_sI;

						for( node_sI = 0; node_sI < meshExt->shadowNodeRemoteCount[rn_I]; node_sI++ ) {
							if( node_gI == meshExt->shadowNodeRemoteArray[rn_I][node_sI] ) {
								coord[en_I][0] = meshExt->shadowNodeRemoteCoord[rn_I][node_sI][0];
								coord[en_I][1] = meshExt->shadowNodeRemoteCoord[rn_I][node_sI][1];
								coord[en_I][2] = meshExt->shadowNodeRemoteCoord[rn_I][node_sI][2];
							}
						}
					/*}*/
				}
			}

			for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
				for( d = 0; d < DIM; d++ ) {
					sbc[rn_I][element_sI * Tetrahedra_Count + tetra_I][d] = 0.25 * (
						coord[TetraToNode[tetra_I][0]][d] +
						coord[TetraToNode[tetra_I][1]][d] +
						coord[TetraToNode[tetra_I][2]][d] +
						coord[TetraToNode[tetra_I][3]][d] );
				}
			}
		}
	}

	/* For each tetrahedra of the new mesh, calculate barycentric coords */
	for( element_lI = 0; element_lI < context->mesh->elementLocalCount; element_lI++ ) {
		Tetrahedra_Index		tetra_I;
		Element_NeighbourIndex*		en = mesh->elementNeighbourTbl[element_lI];
		Element_NeighbourIndex		enCount = mesh->elementNeighbourCountTbl[element_lI];

		for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
			Element_NeighbourIndex		en_I;
			Element_LocalIndex		closest_element;
			Tetrahedra_Index		closest_tetra;
			Element_ShadowIndex		closest_sElement;
			Partition_Index			closest_rn;
			double				newxc[3];
			double				oldxc[3];
			double				selfdist;
			Tetrahedra_Index		tetra_J;
			double				dist2;
			double				dist;

			dist2 = selfdist = DBL_MAX;

			/* Calculation of the barycenter for the tetra of new mesh */
			memset( newxc, 0, sizeof(double) * 3 );
			for( i = 0; i < 4; i++ ) {
				n = TetraToNode[tetra_I][i];
				for( d = 0; d < DIM; d++) {
					newxc[d]+=0.25 * meshExt->newNodeCoord[mesh->elementNodeTbl[element_lI][n]][d];
				}
			}

			/* Find the best tetra (in the sense of barycenter distance) of this element in the old mesh... this
			   will be our starting-point/initial-compare. */
			closest_element = closest_tetra = 0;
			closest_rn = (unsigned)-1;
			for( tetra_J = 0; tetra_J < Tetrahedra_Count; tetra_J++ ) {
				for( d = 0; d < DIM; d++ ){
					oldxc[d] = xbc[(element_lI * Tetrahedra_Count + tetra_J) * DIM + d];
				}
				dist = VECDIST3( newxc, oldxc );
				if( tetra_I == tetra_J ){
					selfdist = dist;
				}
				if( dist2 > dist ) {
					closest_element = element_lI;
					closest_tetra = tetra_J;
					dist2 = dist;
				}
			}

			/* Find the best tetra (in the sense of barycenter distance) of this element's neighbours in the old
			   mesh, using the self-comparison above as a starting-point. */
			for( en_I = 0; en_I < enCount; en_I++ ) {
				Element_LocalIndex		enElement_lI = en[en_I];

				/* The element neighbour is a local element */
				if( enElement_lI < mesh->elementLocalCount ) {
					Journal_DPrintf(
						contextExt->debugElements,
						"Element %03u's neighbour %02u (g%03u) is a local neighbour\n",
						mesh->elementL2G[element_lI],
						en_I,
						mesh->elementL2G[enElement_lI] );

					for( tetra_J = 0; tetra_J < Tetrahedra_Count; tetra_J++ ) {
						for( d = 0; d  < DIM; d++ ) {
							oldxc[d] = xbc[(enElement_lI * Tetrahedra_Count + tetra_J) * DIM + d];
						}
						dist = VECDIST3( newxc, oldxc );
						if( dist2 > dist ) {
							closest_element = enElement_lI;
							closest_tetra = tetra_J;
							dist2 = dist;
						}
					}
				}
				else { /* The element neighbour is a shadow element or non-existant */
					Element_GlobalIndex		tmpEN[mesh->elementNeighbourCountTbl[element_lI]];
					Element_GlobalIndex		element_gI = decomp->elementMapLocalToGlobal(
                                                                                         decomp,
                                                                                         element_lI );
					Element_GlobalIndex		enElement_gI;

					topology->buildNeighbours( topology, element_gI, tmpEN );
					enElement_gI = tmpEN[en_I];

					/* The element neighbour is a shadow */
					if( enElement_gI < meshExt->elementGlobalCount ) {
						Bool				found;

						Journal_DPrintf(
							contextExt->debugElements,
							"Element %03u's neighbour %02u (g%03u) is a shadow neighbour\n",
							mesh->elementL2G[element_lI],
							en_I,
							enElement_gI );

						/* Search through the shadow lists for the element */
						found = False;
						for( rn_I = 0; rn_I < meshExt->neighbourRankCount && !found; rn_I++ ) {
							Element_ShadowIndex		element_sI;

							for(
								element_sI = 0;
								element_sI < meshExt->shadowElementRemoteCount[rn_I] && !found;
								element_sI++ ) {
								Journal_DPrintf(
                                                contextExt->debugElements,
                                                "enElement_gI == meshExt->ShadowElementArrayRemote[%u][%u] => %u == %u\n",
                                                rn_I,
                                                element_sI,
                                                enElement_gI,
                                                meshExt->shadowElementRemoteArray[rn_I][element_sI] );

								if( enElement_gI == meshExt->shadowElementRemoteArray[rn_I][element_sI] ) {
                                    /* Element found in shadow lists. Find if it has a closer
									   tetra */
									for( tetra_J = 0; tetra_J < Tetrahedra_Count; tetra_J++ ) {
										for( d = 0; d  < DIM; d++ ) {
											oldxc[d] = sbc[rn_I][element_sI * Tetrahedra_Count + tetra_J][d];
										}
										dist = VECDIST3( newxc, oldxc );
										if( dist2 > dist ) {
											closest_rn = rn_I;
											closest_sElement = element_sI;
											closest_element = (unsigned)-1;
											closest_tetra = tetra_J;
											dist2 = dist;
										}
									}
									found = True;
								}
							}
						}

						/* Essentially the enElement_gI should be found in the shadow listings, else there
						   is a big coding error! */
						assert( found );
					}
					else {
						Journal_DPrintf(
							contextExt->debugElements,
							"Element g%03u's neighbour %02u (g%03u) is out of bounds\n",
							mesh->elementL2G[element_lI],
							en_I,
							enElement_gI );
						continue;
					}
				}
			}

            if( dist2 > selfdist ) {
                Journal_DPrintf(
                                contextExt->debugElements,
                                "Distance between barycentres of old and new tetrahadras cannot be bigger than" );
                Journal_DPrintf(
                                contextExt->debugElements,
                                "barycentric distance of the same tetrahedra in old and new mesh\n\n" );
            }


			/* transfer */
			SnacRemesher_InterpolateElement(
                                            context,
                                            contextExt,
                                            element_lI,
                                            tetra_I,
                                            closest_element,
                                            closest_rn,
                                            closest_sElement,
                                            closest_tetra,
                                            meshExt->newElement,
                                            meshExt->shadowElementRemote );
		}
	}

#if 0
    for( element_lI = 0; element_lI < mesh->elementLocalCount; element_lI++ ) {
        Snac_Element*		element = (Snac_Element*)Extension_At(
                                                                  context->mesh->elementExtensionMgr,
                                                                  meshExt->newElement,
                                                                  element_lI );
        Tetrahedra_Index	tetra_I;

        for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
            Journal_DPrintf(
                            contextExt->debugElements,
                            "Element-tetra g%03u-%u: stress: %g %g %g %g %g %g\n",
                            mesh->elementL2G[element_lI],
                            tetra_I,
                            element->tetraStress[tetra_I][0][0],
                            element->tetraStress[tetra_I][1][1],
                            element->tetraStress[tetra_I][2][2],
                            element->tetraStress[tetra_I][0][1],
                            element->tetraStress[tetra_I][0][2],
                            element->tetraStress[tetra_I][1][2] );
            Journal_DPrintf(
                            contextExt->debugElements,
                            "Element-tetra g%03u-%u: strain: %g %g %g %g %g %g\n",
                            mesh->elementL2G[element_lI],
                            tetra_I,
                            element->tetra[tetra_I].strain[0][0],
                            element->tetra[tetra_I].strain[1][1],
                            element->tetra[tetra_I].strain[2][2],
                            element->tetra[tetra_I].strain[0][1],
                            element->tetra[tetra_I].strain[0][2],
                            element->tetra[tetra_I].strain[1][2] );
        }
    }
#endif

	for( rn_I = 0; rn_I < meshExt->neighbourRankCount; rn_I++ ) {
		free( sbc[rn_I] );
	}
	free( sbc );
	free( xbc );
}

#endif

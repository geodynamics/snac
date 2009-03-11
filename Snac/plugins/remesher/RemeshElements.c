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
	Mesh*				mesh = context->mesh;
	SnacRemesher_Mesh*		meshExt = ExtensionManager_Get( context->meshExtensionMgr,
															mesh, 
															SnacRemesher_MeshHandle );
	NodeLayout*			nLayout = mesh->layout->nodeLayout;
	IndexSet*				extElements;
	Element_LocalIndex		newElt_i;
	
	void Tet_Barycenter( Coord tetCrds[4], Coord center );
	void interpolateElement( void*			_context, 
							 Element_LocalIndex	newEltInd, 
							 Tetrahedra_Index	newTetInd, 
							 Coord			newEltBC, 
							 Element_DomainIndex	dEltInd );
	void findClosestTet( void*			_context, 
						 Element_LocalIndex	newEltInd, 
						 Tetrahedra_Index	newTetInd, 
						 Coord			newEltBC, 
						 Element_DomainIndex	dEltInd,
						 unsigned *minTetInd, double *minTetDist);
		
	
	/*
	** Free any owned arrays that may still exist from the last node interpolation.
	*/
	
	FreeArray( meshExt->externalElements );
	meshExt->nExternalElements = 0;
	
	
	/*
	** Interpolate all new elements.
	*/
	
	/* Create an index set for storing any external elements. */
	extElements = IndexSet_New( mesh->elementLocalCount );
	
	/* Loop over the new elements. */
	for( newElt_i = 0; newElt_i < mesh->elementLocalCount; newElt_i++ ) {
		unsigned			nEltNodes;
		Node_DomainIndex*	eltNodes;
		unsigned			tet_i;
		
		/* Extract the element's node indices.  Note that there should always be eight of these. */
		{
			Element_GlobalIndex	gEltInd;
			
			nEltNodes = 8;
			eltNodes = Memory_Alloc_Array( Node_DomainIndex, nEltNodes, "SnacRemesher" );
			gEltInd = Mesh_ElementMapLocalToGlobal( mesh, newElt_i );
			nLayout->buildElementNodes( nLayout, gEltInd, eltNodes );
		}
		
		/* Convert global node indices to local. */
		{
			unsigned	eltNode_i;
			
			for( eltNode_i = 0; eltNode_i < nEltNodes; eltNode_i++ ) {
				eltNodes[eltNode_i] = Mesh_NodeMapGlobalToLocal( mesh, eltNodes[eltNode_i] );
			}
		}
		
		for( tet_i = 0; tet_i < Tetrahedra_Count; tet_i++ ) {
			Coord				tetCrds[4];
			Coord				bc;
			Node_DomainIndex		dNodeInd;
			unsigned				nElements;
			Element_DomainIndex*	elements;
			unsigned				elt_i;
			double minTetDist, minDist;
			unsigned minTetInd, minEltInd;
			
			/* Extract the tetrahedron's coordinates. */
			Vector_Set( tetCrds[0], meshExt->newNodeCoords[eltNodes[TetraToNode[tet_i][0]]] );
			Vector_Set( tetCrds[1], meshExt->newNodeCoords[eltNodes[TetraToNode[tet_i][1]]] );
			Vector_Set( tetCrds[2], meshExt->newNodeCoords[eltNodes[TetraToNode[tet_i][2]]] );
			Vector_Set( tetCrds[3], meshExt->newNodeCoords[eltNodes[TetraToNode[tet_i][3]]] );
			
			/* Calculate the barycenter of this tetrahedron. */
			Tet_Barycenter( tetCrds, bc );
			
			/* Locate the closest node to the barycenter. We use index zero of the tethrahedra coordinates
			   as a reference. */
			dNodeInd = findClosestNodeInElement( context, bc, nEltNodes, eltNodes );
			
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
			
			minDist = 1.0e+21;
			for( elt_i = 0; elt_i < nElements; elt_i++ ) {
				if( elements[elt_i] >= context->mesh->elementDomainCount )
					continue;

				findClosestTet( context, newElt_i, tet_i, bc, elements[elt_i], &minTetInd, &minTetDist );
				if( minTetDist < minDist ) {
					minDist = minTetDist;
					minEltInd = elements[elt_i];
				}
			}
			/* Interpolate the element's tetrahedra. */
			SnacRemesher_InterpolateElement( context, contextExt, 
							 newElt_i, tet_i, 
							 meshExt->newElements, 
							 minEltInd, minTetInd );
#if 0
			/* Which of the incident elements contains the node? */
			for( elt_i = 0; elt_i < nElements; elt_i++ ) {
				fprintf(stderr,"newElt_i=%d tet_i=%d dNodeInd=%d elt_i=%d element=%d bc=%e %e %e\n",
						newElt_i,tet_i,dNodeInd,elt_i,elements[elt_i],
						bc[0],bc[1],bc[2]);
				if( elements[elt_i] >= mesh->elementDomainCount ) {
					continue;
				}
				
				/* fprintf(stderr,"newElt_i=%d tet_i=%d element=%d dNodeInd=%d nElements=%d\n", */
/* 						newElt_i,tet_i,elements[elt_i],dNodeInd,nElements); */
				if( pointInElement( context, bc, elements[elt_i] ) ) {
					break;
				}
			}
			

			/* Did we find the element? */
			if( elt_i < nElements ) {
				/* If so, call a function to locate the tetrahedra and interpolate. */
				interpolateElement( context, newElt_i, tet_i, bc, elements[elt_i] );
			}
			else {

				/* If not, then the new element's tetrahedron's barycenter is outside the old mesh.  Keep existing
				   tetrahedral values and mark this element as having incomplete tetrahedra. */
				
				/* Stash the indices. */
				IndexSet_Add( extElements, newElt_i );
				
				/* Copy across the old value. Note that this should be done using some other provided copy method. */
				memcpy( (unsigned char*)meshExt->newElements + newElt_i * mesh->elementExtensionMgr->finalSize, 
					   (unsigned char*)mesh->element + newElt_i * mesh->elementExtensionMgr->finalSize, 
					   mesh->elementExtensionMgr->finalSize );
				//assert(0);
			}
#endif
			
			/* Free the element array. */
			FreeArray( elements );
		}
		
		/* Free resources. */
		FreeArray( eltNodes );
	}
	
	/* Dump the external nodes and delete the set. */
	IndexSet_GetMembers( extElements, &meshExt->nExternalElements, &meshExt->externalElements );
	Stg_Class_Delete( extElements );
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
** Locate the tetrahedra the barycenter falls in and interpolate from there.
*/

void findClosestTet( void*			_context, 
					 Element_LocalIndex	newEltInd, 
					 Tetrahedra_Index	newTetInd, 
					 Coord			newEltBC, 
					 Element_DomainIndex	dEltInd,
					 unsigned *minTetInd, double *minTetDist)
{
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
	unsigned				tet_i;
	
	void Tet_Barycenter( Coord tetCrds[4], Coord center );
	
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
	
	/* Loop over 10 sub tets in a brick element, calculate the distance from its barycenter to the supplied one. */
	{
		for( tet_i = 0; tet_i < Tetrahedra_Count; tet_i++ ) {
			Coord	tCrds[4];
			Coord	bc;
			double	dist;
			
			/* Extract the tetrahedron's coordinates. */
			Vector_Set( tCrds[0], mesh->nodeCoord[eltNodes[TetraToNode[tet_i][0]]] );
			Vector_Set( tCrds[1], mesh->nodeCoord[eltNodes[TetraToNode[tet_i][1]]] );
			Vector_Set( tCrds[2], mesh->nodeCoord[eltNodes[TetraToNode[tet_i][2]]] );
			Vector_Set( tCrds[3], mesh->nodeCoord[eltNodes[TetraToNode[tet_i][3]]] );
			
			/* Calc the barycenter. */
			Tet_Barycenter( tCrds, bc );
			
			/* Calc the difference. */
			Vector_Sub( bc, bc, newEltBC );
			dist = Vector_Mag( bc );
			
			if( tet_i == 0 ) {
				(*minTetDist) = dist;
				(*minTetInd) = 0;
			}
			else if( dist < (*minTetDist) ) {
				(*minTetDist) = dist;
				(*minTetInd) = tet_i;
			}
		}
	}
	
	/* Free the element node array. */
	FreeArray( eltNodes );
}



/*
** Locate the tetrahedra the barycenter falls in and interpolate from there.
*/

void interpolateElement( void*			_context, 
					Element_LocalIndex	newEltInd, 
					Tetrahedra_Index	newTetInd, 
					Coord			newEltBC, 
					Element_DomainIndex	dEltInd )
{
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
	unsigned				minTetInd;
	unsigned				tet_i;
	
	void Tet_Barycenter( Coord tetCrds[4], Coord center );
	
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
	
	/* Loop over 10 sub tets in a brick element, calculate the distance from its barycenter to the supplied one. */
	{
		double minDist;
		
		for( tet_i = 0; tet_i < Tetrahedra_Count; tet_i++ ) {
			Coord	tCrds[4];
			Coord	bc;
			double	dist;
			
			/* Extract the tetrahedron's coordinates. */
			Vector_Set( tCrds[0], mesh->nodeCoord[eltNodes[TetraToNode[tet_i][0]]] );
			Vector_Set( tCrds[1], mesh->nodeCoord[eltNodes[TetraToNode[tet_i][0]]] );
			Vector_Set( tCrds[2], mesh->nodeCoord[eltNodes[TetraToNode[tet_i][0]]] );
			Vector_Set( tCrds[3], mesh->nodeCoord[eltNodes[TetraToNode[tet_i][0]]] );
			
			/* Calc the barycenter. */
			Tet_Barycenter( tCrds, bc );
			
			/* Calc the difference. */
			Vector_Sub( bc, bc, newEltBC );
			dist = Vector_Mag( bc );
			
			if( tet_i == 0 ) {
				minDist = dist;
				minTetInd = 0;
			}
			else if( dist < minDist ) {
				minDist = dist;
				minTetInd = tet_i;
			}
		}
	}
	
	/* Interpolate the element's tetrahedra. */
	SnacRemesher_InterpolateElement( context, contextExt, 
					 newEltInd, newTetInd, 
					 meshExt->newElements, 
					 dEltInd, minTetInd );
	
	/* Free the element node array. */
	FreeArray( eltNodes );
}


/*
** Interpolate an element's tetrahedra.
*/

void _SnacRemesher_InterpolateElement( void*				_context, 
							    Element_LocalIndex	dstEltInd, 
							    Tetrahedra_Index	dstTetInd, 
							    Snac_Element*		dstEltArray, 
							    Element_DomainIndex	srcEltInd, 
							    Tetrahedra_Index	srcTetInd )
{
	Snac_Context*	context = (Snac_Context*)_context;
	Mesh*		mesh = context->mesh;
	Snac_Element*	dstElt = (Snac_Element*)ExtensionManager_At( context->mesh->elementExtensionMgr, 
								     dstEltArray, 
								     dstEltInd );
	Snac_Element*	srcElt = Snac_Element_At( context, srcEltInd );

	/* Copy the whole structure from the nearest tet in the old mesh. */
	memcpy( &(dstElt->tetra[dstTetInd]), &(srcElt->tetra[srcTetInd]), sizeof(Snac_Element_Tetrahedra) );

}


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

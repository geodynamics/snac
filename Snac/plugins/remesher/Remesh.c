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
** $Id: Remesh.c 3096 2005-07-13 23:51:01Z EunseoChoi $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "Mesh.h"
#include "Context.h"
#include "Remesh.h"
#include "Register.h"
#include "Utils.h"

#include <string.h>
#include <assert.h>


void _SnacRemesher_Remesh( void* _context, void* data ) {
	Snac_Context*				context = (Snac_Context*)_context;
	SnacRemesher_Context*			contextExt = ExtensionManager_Get(
							context->extensionMgr,
							context,
							SnacRemesher_ContextHandle );
	Bool 					remesh;

	Journal_DPrintf( context->debug, "In: %s\n", __func__ );

	switch( contextExt->condition ) {
		case SnacRemesher_OnTimeStep:
			/* Remeshing on multiples of "OnTimeStep", but don't remesh on loop 0. */
			Journal_Firewall( contextExt->OnTimeStep, context->snacError, 
					  "Invalid remesh timestep criterion." );
			remesh = (context->timeStep <= 1) ? False : 
				(context->timeStep % contextExt->OnTimeStep == 0) ? True : False;
			break;

		case SnacRemesher_OnMinLengthScale:
			remesh = (context->minLengthScale/context->initMinLengthScale < 
				  contextExt->onMinLengthScale) ? True : False;
			break;

		case SnacRemesher_OnBothTimeStepLength:
			Journal_Firewall( contextExt->OnTimeStep, context->snacError, 
					  "Invalid remesh timestep criterion." );
			remesh = (context->timeStep <= 1) ? False : 
			  (context->timeStep % contextExt->OnTimeStep == 0) ? True : False;
			remesh = remesh ? True : (context->minLengthScale/context->initMinLengthScale < 
						  contextExt->onMinLengthScale) ? True : False;
			break;

		case SnacRemesher_Off:
		default:
			remesh = False;
			break;
	}

	if( remesh ) {
		Mesh*					mesh = context->mesh;
		SnacRemesher_Mesh*			meshExt = ExtensionManager_Get(
								context->meshExtensionMgr,
								mesh,
								SnacRemesher_MeshHandle );

		Journal_Printf( context->snacInfo, "Remeshing!\n" );
		
		/*
		** If spherical coordinates are being used, then we'll need to convert the current mesh's cartesian coordinates 
		** to spherical coordinates first.
		*/
		
		if( meshExt->meshType == SnacRemesher_Spherical ) {
			unsigned	lNode_i;
			
			for( lNode_i = 0; lNode_i < mesh->nodeLocalCount; lNode_i++ ) {
				double	x = mesh->nodeCoord[lNode_i][0];
				double	y = mesh->nodeCoord[lNode_i][1];
				double	z = mesh->nodeCoord[lNode_i][2];
				
				mesh->nodeCoord[lNode_i][0] = SnacArcTan( y, x );
				mesh->nodeCoord[lNode_i][1] = sqrt( x * x + y * y + z * z );
				mesh->nodeCoord[lNode_i][2] = acos( z / meshExt->newNodeCoords[lNode_i][1] );
			}
		}
		
		/* Sync the mesh. */
		if( mesh->layout->decomp->procsInUse > 1 ) {
			Mesh_Sync( mesh );
		}
		
		/* Remesh the coordinates. */
		_SnacRemesher_NewCoords( context );
		
		/* Interpolate current nodal values onto new coordinates. */
		meshExt->newNodes = (Snac_Node*)ExtensionManager_Malloc( mesh->nodeExtensionMgr, mesh->nodeLocalCount );
		_SnacRemesher_InterpolateNodes( context );
		
		/* Interpolate current elemental values onto new coordinates. */
		meshExt->newElements = (Snac_Element*)ExtensionManager_Malloc( mesh->elementExtensionMgr, mesh->elementLocalCount );
		_SnacRemesher_InterpolateElements( context );
		
		/* Copy accross the new coord, node & element information to the current arrays. */
		memcpy( mesh->nodeCoord, meshExt->newNodeCoords, mesh->nodeLocalCount * sizeof(Coord) );
		memcpy( mesh->node, meshExt->newNodes, mesh->nodeExtensionMgr->finalSize * mesh->nodeLocalCount );
		memcpy( mesh->element, meshExt->newElements, mesh->elementExtensionMgr->finalSize * mesh->elementLocalCount );
		
		/* Free some space, as it won't be needed until the next remesh. */
		ExtensionManager_Free( mesh->nodeExtensionMgr, meshExt->newNodes );
		ExtensionManager_Free( mesh->elementExtensionMgr, meshExt->newElements );
		meshExt->newNodes = NULL;
		meshExt->newElements = NULL;
		
		
		/*
		** If in spherical mode, convert back to cartesian coordinates.
		*/
		
		if( meshExt->meshType == SnacRemesher_Spherical ) {
			unsigned	lNode_i;
			
			for( lNode_i = 0; lNode_i < mesh->nodeLocalCount; lNode_i++ ) {
				double	theta = mesh->nodeCoord[lNode_i][0];
				double	r = mesh->nodeCoord[lNode_i][1];
				double	phi = mesh->nodeCoord[lNode_i][2];
				
				mesh->nodeCoord[lNode_i][0] = r * sin( phi ) * cos( theta );
				mesh->nodeCoord[lNode_i][1] = r * sin( phi ) * sin( theta );
				mesh->nodeCoord[lNode_i][2] = r * cos( phi );
			}
		}
		
		/* Sync the mesh. */
		if( mesh->layout->decomp->procsInUse > 1 ) {
			Mesh_Sync( mesh );
		}
		
		/* dump info such as remeshing frequency, criterion, and the current time step */
		contextExt->remeshingCount++;
		_SnacRemesher_DumpInfo( context );
	}
}


#if 0
void _SnacRemesher_Sync( void* _context ) {
	Snac_Context*				context = (Snac_Context*)_context;
	Mesh*					mesh = context->mesh;
	SnacRemesher_Mesh*			meshExt = ExtensionManager_Get(
							context->meshExtensionMgr,
							mesh,
							SnacRemesher_MeshHandle );
	Partition_Index				i;

	SnacRemesher_Context*			contextExt = ExtensionManager_Get(
							context->extensionMgr,
							context,
							SnacRemesher_ContextHandle );

	Journal_DPrintf( context->debug, "In: %s\n", __func__ );

	/* Pack (local) shadows required to send to neighbour and send them, for each neighbour */
	Journal_DPrintf( contextExt->debugSync, "Top nodes shadow syncing starting.\n" );
	for( i = 0; i < meshExt->neighbourRankCount; i++ ) {
		Node_ShadowIndex			nCount = meshExt->shadowNodeLocalCount[i];
		Element_ShadowIndex			eCount = meshExt->shadowElementLocalCount[i];
		Node_ShadowIndex			node_sI;
		Element_ShadowIndex			element_sI;
		Coord					coordLocal[nCount];
		Snac_Node*				nodeLocal = (Snac_Node*)ExtensionManager_Malloc( mesh->nodeExtensionMgr, nCount );

		Journal_DPrintf( contextExt->debugSync, "Sending node %u coordinates to rank: %u; {", nCount, meshExt->neighbourRank[i] );
		for( node_sI = 0; node_sI < nCount; node_sI++ ) {
			coordLocal[node_sI][0] = mesh->nodeCoord[meshExt->shadowNodeLocalArray[i][node_sI]][0];
			coordLocal[node_sI][1] = mesh->nodeCoord[meshExt->shadowNodeLocalArray[i][node_sI]][1];
			coordLocal[node_sI][2] = mesh->nodeCoord[meshExt->shadowNodeLocalArray[i][node_sI]][2];
			Journal_DPrintf( contextExt->debugSync, "{ %g, %g, %g }, ", coordLocal[node_sI][0], coordLocal[node_sI][1], coordLocal[node_sI][2] );
		}
		Journal_DPrintf( contextExt->debugSync, "}\n" );

		MPI_Send(
			coordLocal,
			nCount * 3,
			MPI_DOUBLE,
			meshExt->neighbourRank[i],
			0,
			context->communicator );

		Journal_DPrintf( contextExt->debugSync, "Sending node %u to rank: %u\n", nCount, meshExt->neighbourRank[i] );
		for( node_sI = 0; node_sI < nCount; node_sI++ ) {
			memcpy(
				ExtensionManager_At( mesh->nodeExtensionMgr, nodeLocal, node_sI ),
				Mesh_NodeAt( mesh, meshExt->shadowNodeLocalArray[i][node_sI] ),
				mesh->nodeExtensionMgr->finalSize );
		}

		MPI_Send(
			nodeLocal,
			nCount * mesh->nodeExtensionMgr->finalSize,
			MPI_CHAR,
			meshExt->neighbourRank[i],
			1668,
			context->communicator );

		Journal_DPrintf( contextExt->debugSync, "Sending element %u to rank: %u\n", eCount, meshExt->neighbourRank[i] );
		for( element_sI = 0; element_sI < eCount; element_sI++ ) {
			MPI_Send(
				Mesh_ElementAt( mesh, meshExt->shadowElementLocalArray[i][element_sI] ),
				mesh->elementExtensionMgr->finalSize,
				MPI_CHAR,
				meshExt->neighbourRank[i],
				1669+element_sI,
				context->communicator );
		}

		ExtensionManager_Free( mesh->nodeExtensionMgr, nodeLocal );
	}

	/* Receive the associated shadows from each neighbour */
	for( i = 0; i < meshExt->neighbourRankCount; i++ ) {
		MPI_Status				status;
		Node_ShadowIndex			nCount = meshExt->shadowNodeRemoteCount[i];
		Element_ShadowIndex			eCount = meshExt->shadowElementRemoteCount[i];
		Node_ShadowIndex			node_sI;
		Element_ShadowIndex			element_sI;

		MPI_Recv(
			meshExt->shadowNodeRemoteCoord[i],
			nCount * 3,
			MPI_DOUBLE,
			meshExt->neighbourRank[i],
			0,
			context->communicator,
			&status );
		Journal_DPrintf( contextExt->debugSync, "Received node %u coordinates from rank: %u; {", nCount, meshExt->neighbourRank[i] );
		for( node_sI = 0; node_sI < nCount; node_sI++ ) {
			Journal_DPrintf( contextExt->debugSync,
				"{ %g, %g, %g }, ",
				meshExt->shadowNodeRemoteCoord[i][node_sI][0],
				meshExt->shadowNodeRemoteCoord[i][node_sI][1],
				meshExt->shadowNodeRemoteCoord[i][node_sI][2] );
		}
		Journal_DPrintf( contextExt->debugSync, "}\n" );

		MPI_Recv(
			meshExt->shadowNodeRemote[i],
			nCount * mesh->nodeExtensionMgr->finalSize,
			MPI_CHAR,
			meshExt->neighbourRank[i],
			1668,
			context->communicator,
			&status );
		Journal_DPrintf( contextExt->debugSync, "Received node %u from rank: %u\n", nCount, meshExt->neighbourRank[i] );

		for( element_sI = 0; element_sI < eCount; element_sI++ ) {
			MPI_Recv(
				ExtensionManager_At( mesh->elementExtensionMgr, meshExt->shadowElementRemote[i], element_sI ),
				mesh->elementExtensionMgr->finalSize,
				MPI_CHAR,
				meshExt->neighbourRank[i],
				1669+element_sI,
				context->communicator,
				&status );
		}
		Journal_DPrintf( contextExt->debugSync, "Received element %u from rank: %u\n", eCount, meshExt->neighbourRank[i] );
	}
	Journal_DPrintf( contextExt->debugSync, "Top nodes shadow syncing done.\n" );
}
#endif

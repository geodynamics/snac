/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003,
**	Steve Quenette, 110 Victoria Street, Melbourne, Victoria, 3053, Australia.
**	Californian Institute of Technology, 1200 East California Boulevard, Pasadena, California, 91125, USA.
**	University of Texas, 1 University Station, Austin, Texas, 78712, USA.
**
** Authors:
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
** Comments:
** 	Not sure ShadowElementSetLocal is needed as I think it now has the same values as ShadowElementSetCurrent.
**
** $Id: InitialConditions.c 3254 2006-10-26 08:25:11Z EunseoChoi $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>

#include "Snac/Snac.h"
#include "types.h"
#include "Mesh.h"
#include "Context.h"
#include "Register.h"
#include "InitialConditions.h"
#include "Utils.h"

#include <string.h>
#include <math.h>
#include <assert.h>

#ifndef PATH_MAX
	#define PATH_MAX 1024
#endif

void _SnacRemesher_InitialConditions( void* _context, void* data ) {
	Snac_Context*		context = (Snac_Context*)_context;
	Mesh*			mesh = context->mesh;
	HexaMD*			decomp = (HexaMD*)mesh->layout->decomp;
	SnacRemesher_Mesh*	meshExt = ExtensionManager_Get(
						context->meshExtensionMgr,
						mesh,
						SnacRemesher_MeshHandle );
	unsigned			lNode_i;
	IndexSet*			walls[6];
	IndexSet*			bulk;
	IndexSet*			topIntern;
	IndexSet*			botIntern;
	
	void _SnacRemesher_TriangulateXZPlane(
		void*		_context, 
		IndexSet*		planeNodes, 
		unsigned		gYInd, 
		unsigned*		nTriNodes, 
		unsigned**	triToDomain );
	
	
	/*
	** Firstly, backup all coords into the 'newNodeCoords' coord block.
	*/
	if( context->restartTimestep > 0 ) {
		FILE *fp;
		char path[PATH_MAX];
		double	x,y,z;

		sprintf(path, "%s/snac.initcoord.%d.%06d.restart",context->outputPath,context->rank,context->restartTimestep);
		Journal_Firewall( ( (fp=fopen(path,"r")) != NULL), context->snacError, "Can't find %s", path );
		for( lNode_i = 0; lNode_i < mesh->nodeLocalCount; lNode_i++ ) {
			fscanf( fp, "%le %le %le", &x,&y,&z);
			meshExt->newNodeCoords[lNode_i][0] = x;
			meshExt->newNodeCoords[lNode_i][1] = y;
			meshExt->newNodeCoords[lNode_i][2] = z;
		}
	}
	else {
		if( meshExt->meshType == SnacRemesher_Cartesian ) {
			for( lNode_i = 0; lNode_i < mesh->nodeLocalCount; lNode_i++ ) {
				Vector_Set( meshExt->newNodeCoords[lNode_i], mesh->nodeCoord[lNode_i] );
			}
		}
		else {
			/* Convert cartesian coordinates to spherical coordinates and store. */
			for( lNode_i = 0; lNode_i < mesh->nodeLocalCount; lNode_i++ ) {
				double	x = mesh->nodeCoord[lNode_i][0];
				double	y = mesh->nodeCoord[lNode_i][1];
				double	z = mesh->nodeCoord[lNode_i][2];
				
				meshExt->newNodeCoords[lNode_i][0] = SnacArcTan( y, x );
				meshExt->newNodeCoords[lNode_i][1] = sqrt( x * x + y * y + z * z );
				meshExt->newNodeCoords[lNode_i][2] = acos( z / meshExt->newNodeCoords[lNode_i][1] );
			}
		}
	}
	
	
	/*
	** Generate index arrays into local coordinate space for top and bottom (internal) walls.
	*/
	
	walls[0] = RegularMeshUtils_CreateGlobalLeftSet( mesh );
	walls[1] = RegularMeshUtils_CreateGlobalRightSet( mesh );
	walls[2] = RegularMeshUtils_CreateGlobalFrontSet( mesh );
	walls[3] = RegularMeshUtils_CreateGlobalBackSet( mesh );
	walls[4] = RegularMeshUtils_CreateGlobalTopSet( mesh );
	walls[5] = RegularMeshUtils_CreateGlobalBottomSet( mesh );
	bulk = IndexSet_New( mesh->nodeLocalCount );
	
	topIntern = IndexSet_New( mesh->nodeLocalCount );
	botIntern = IndexSet_New( mesh->nodeLocalCount );
	
	for( lNode_i = 0; lNode_i < mesh->nodeLocalCount; lNode_i++ ) {
		if( IndexSet_IsMember( walls[4], lNode_i ) ) {
			IndexSet_Add( topIntern, lNode_i );
		}
		else if( IndexSet_IsMember( walls[5], lNode_i ) )
		{
			IndexSet_Add( botIntern, lNode_i );
		}
		else {
			/* Also generate the bulk indices while we're at it. */
			IndexSet_Add( bulk, lNode_i );
		}
	}
	
	IndexSet_GetMembers( topIntern, &meshExt->nTopInternNodes, &meshExt->topInternToLocal );
	IndexSet_GetMembers( botIntern, &meshExt->nBotInternNodes, &meshExt->botInternToLocal );
	
	/* These walls aren't needed any more. */
	Stg_Class_Delete( walls[0] );
	Stg_Class_Delete( walls[1] );
	Stg_Class_Delete( walls[2] );
	Stg_Class_Delete( walls[3] );
	
	
	/*
	** Create triangulations for the top and bottom walls using domain indices.
	*/
	
	if( walls[4]->membersCount > 0 ) {
		_SnacRemesher_TriangulateXZPlane( 
			context, 
			walls[4], 
			decomp->nodeGlobal3DCounts[1] - 1, 
			&meshExt->nTopTriNodes, 
			&meshExt->topTriToDomain );
	}
	else {
		meshExt->nTopTriNodes = 0;
		meshExt->topTriToDomain = NULL;
	}
	
	if( walls[5]->membersCount > 0 ) {
		_SnacRemesher_TriangulateXZPlane( context, walls[5], 0, &meshExt->nBotTriNodes, &meshExt->botTriToDomain );
	}
	else {
		meshExt->nBotTriNodes = 0;
		meshExt->botTriToDomain = NULL;
	}
	
	/* Walls are no longer required. */
	Stg_Class_Delete( walls[4] );
	Stg_Class_Delete( walls[5] );
	
	
	/*
	** Build lists of y-lines for interpolating bulk nodes efficiently.
	*/
	
	{
		unsigned			nBulkNodes;
		Node_LocalIndex*	bulkNodes;
		unsigned			bulk_i;
		unsigned			nYLines;
		unsigned*			yLineX;
		unsigned*			yLineZ;
		
		/*
		** First, extract the number of y-lines and their x,z offsets.
		*/
		
		/* Extract the bulk local node indices and destroy the set. */
		IndexSet_GetMembers( bulk, &nBulkNodes, &bulkNodes );
		Stg_Class_Delete( bulk );
		
		/* Setup yLine info. */
		nYLines = 0;
		yLineX = Memory_Alloc_Array( unsigned, 1, "SnacRemesher" );
		yLineZ = Memory_Alloc_Array( unsigned, 1, "SnacRemesher" );
		
		/* Calculate the number of unique y-lines and their x, z offsets. */
		for( bulk_i = 0; bulk_i < nBulkNodes; bulk_i++ ) {
			Node_GlobalIndex	gNodeInd;
			IJK				ijk;
			unsigned			yLine_i;
			
			gNodeInd = Mesh_NodeMapLocalToGlobal( mesh, bulkNodes[bulk_i] );
			RegularMeshUtils_Node_1DTo3D( decomp, gNodeInd, &ijk[0], &ijk[1], &ijk[2] );
			
			/* Is this one unique? */
			for( yLine_i = 0; yLine_i < nYLines; yLine_i++ ) {
				if( yLineX[yLine_i] == ijk[0] && yLineZ[yLine_i] == ijk[2] ) {
					break;
				}
			}
			
			/* If not, skip. */
			if( yLine_i < nYLines ) {
				continue;
			}
			
			/* If so, add it to the list. */
			yLineX = Memory_Realloc_Array( yLineX, unsigned, nYLines + 1 );
			yLineZ = Memory_Realloc_Array( yLineZ, unsigned, nYLines + 1 );
			yLineX[nYLines] = ijk[0];
			yLineZ[nYLines] = ijk[2];
			nYLines++;
		}
		
		/* If there were none, then fix the arrays. */
		if( nYLines == 0 ) {
			Memory_Free( yLineX );
			yLineX = NULL;
			Memory_Free( yLineZ );
			yLineZ = NULL;
		}
		else {
			IndexSet*	locals;
			unsigned	yLine_i;
			
			/* If we're here, then there are local y-lines and we need to fill incident local nodes. Start
			   by setting up the arrays. */
			meshExt->nYLines = nYLines;
			meshExt->yLineLTerm = Memory_Alloc_Array( unsigned, nYLines, "SnacRemesher" );
			meshExt->yLineUTerm = Memory_Alloc_Array( unsigned, nYLines, "SnacRemesher" );
			meshExt->yLineNYInds = Memory_Alloc_Array( unsigned, nYLines, "SnacRemesher" );
			meshExt->yLineYInds = Memory_Alloc_Array( unsigned*, nYLines, "SnacRemesher" );
			meshExt->yLineLocalInds = Memory_Alloc_Array( unsigned*, nYLines, "SnacRemesher" );
			
			locals = IndexSet_New( mesh->nodeLocalCount );
			
			/* Loop over y-lines */
			for( yLine_i = 0; yLine_i < nYLines; yLine_i++ ) {
				IndexSet_RemoveAll( locals );
				
				/* Loop over local bulk nodes. */
				for( bulk_i = 0; bulk_i < nBulkNodes; bulk_i++ ) {
					Node_GlobalIndex	gNodeInd;
					IJK				ijk;
					
					gNodeInd = Mesh_NodeMapLocalToGlobal( mesh, bulkNodes[bulk_i] );
					RegularMeshUtils_Node_1DTo3D( decomp, gNodeInd, &ijk[0], &ijk[1], &ijk[2] );
					
					if( ijk[0] == yLineX[yLine_i] && ijk[2] == yLineZ[yLine_i] ) {
						IndexSet_Add( locals, bulkNodes[bulk_i] );
					}
				}
				
				/* Add them to the master arrays. */
				{
					unsigned	yLine_j;
					IJK		ijk;
					
					IndexSet_GetMembers( locals, &meshExt->yLineNYInds[yLine_i], &meshExt->yLineLocalInds[yLine_i] );
					meshExt->yLineYInds[yLine_i] = Memory_Alloc_Array( unsigned, 
						meshExt->yLineNYInds[yLine_i], 
						"SnacRemesher" );
					
					/* Extract the global y-index for each local index. */
					for( yLine_j = 0; yLine_j < meshExt->yLineNYInds[yLine_i]; yLine_j++ ) {
						Node_GlobalIndex	gNodeInd;
						
						gNodeInd = Mesh_NodeMapLocalToGlobal( mesh, meshExt->yLineLocalInds[yLine_i][yLine_j] );
						RegularMeshUtils_Node_1DTo3D( decomp, gNodeInd, &ijk[0], &ijk[1], &ijk[2] );
						
						meshExt->yLineYInds[yLine_i][yLine_j] = ijk[1];
					}
					
					/* Set up the terminals. */
					meshExt->yLineLTerm[yLine_i] = RegularMeshUtils_Node_Global3DToGlobal1D( decomp, 
						ijk[0], 
						0, 
						ijk[2] );
					meshExt->yLineUTerm[yLine_i] = RegularMeshUtils_Node_Global3DToGlobal1D( decomp, 
						ijk[0], 
						decomp->nodeGlobal3DCounts[1] - 1, 
						ijk[2] );
				}
			}
			
			Stg_Class_Delete( locals );
		}
		
		if( yLineX ) {
			Memory_Free( yLineX );
		}
		
		if( yLineZ ) {
			Memory_Free( yLineZ );
		}
	}
	
	
	/*
	** Currently, the terminals are stored in global index format.  We need to determine if the terminals are
	** actually locally stored or remotely.  If locally, then convert the global index to a local one.  If remotely, 
	** then add it to the 'sync' class and modify the global index to look in the right sync'ed place.
	*/
	
	{
		unsigned	yLine_i;
		SnacSync*	sync;
		unsigned	nLocals = 0;
		unsigned*	locals;
		unsigned	nRequired = 0;
		unsigned*	required;
		unsigned	z_i;
		unsigned*	srcMap;
		
		/* Create a new sync class. */
		sync = SnacSync_New( "SnacRemesher_SnacSync" );
		
		/* Initialise the required array. */
		required = Memory_Alloc_Array( unsigned, 1, "SnacSync" );
		
		for( yLine_i = 0; yLine_i < meshExt->nYLines; yLine_i++ ) {
			unsigned	term;
			
			/* Check the lower terminal. */
			term = Mesh_NodeMapGlobalToLocal( mesh, meshExt->yLineLTerm[yLine_i] );
			if( term < mesh->nodeLocalCount ) {
				/* Store local index. */
				meshExt->yLineLTerm[yLine_i] = term;
			}
			else {
				/* Add this terminal to the sync-remote array and modify its index to be:
					nodeLocalCount + remote-index */
				required = Memory_Realloc_Array( required, unsigned, nRequired + 1 );
				required[nRequired] = meshExt->yLineLTerm[yLine_i];
				meshExt->yLineLTerm[yLine_i] = mesh->nodeLocalCount + nRequired;
				nRequired++;
			}
			
			/* Check the upper terminal. */
			term = Mesh_NodeMapGlobalToLocal( mesh, meshExt->yLineUTerm[yLine_i] );
			if( term < mesh->nodeLocalCount ) {
				/* Store local index. */
				meshExt->yLineUTerm[yLine_i] = term;
			}
			else {
				/* Add this terminal to the sync-remote array and modify its index to be:
					nodeLocalCount + remote-index */
				required = Memory_Realloc_Array( required, unsigned, nRequired + 1 );
				required[nRequired] = meshExt->yLineUTerm[yLine_i];
				meshExt->yLineUTerm[yLine_i] = mesh->nodeLocalCount + nRequired;
				nRequired++;
			}
		}
		
		/* Fix up the required array. */
		if( nRequired == 0 ) {
			KillArray( required );
		}
		
		/* Begin the local array. */
		locals = Memory_Alloc_Array( unsigned, 1, "SnacSync" );
		
		/* Add local terminals. */
		for( z_i = 0; z_i < decomp->nodeGlobal3DCounts[2]; z_i++ ) {
			unsigned	x_i;
			
			for( x_i = 0; x_i < decomp->nodeGlobal3DCounts[0]; x_i++ ) {
				Node_GlobalIndex	gLower;
				Node_GlobalIndex	gUpper;
				Node_LocalIndex	lLower;
				Node_LocalIndex	lUpper;
				
				gLower = RegularMeshUtils_Node_Global3DToGlobal1D( decomp, x_i, 0, z_i );
				gUpper = RegularMeshUtils_Node_Global3DToGlobal1D( decomp, x_i, decomp->nodeGlobal3DCounts[1] - 1, z_i );
				lLower = Mesh_NodeMapGlobalToLocal( mesh, gLower );
				lUpper = Mesh_NodeMapGlobalToLocal( mesh, gUpper );
				
				if( lLower < mesh->nodeLocalCount ) {
					locals = Memory_Realloc_Array( locals, unsigned, nLocals + 1 );
					locals[nLocals] = gLower;
					nLocals++;
				}
				
				if( lUpper < mesh->nodeLocalCount ) {
					locals = Memory_Realloc_Array( locals, unsigned, nLocals + 1 );
					locals[nLocals] = gUpper;
					nLocals++;
				}
			}
		}
		
		/* If no locals, fix array. */
		if( nLocals == 0 ) {
			KillArray( locals );
		}
		
		/* Negotiate minimum comms. */
		SnacSync_Negotiate( sync, mesh->nodeGlobalCount, nLocals, locals, nRequired, required, decomp->communicator );
		
		/* Store the sync class. */
		meshExt->sync = sync;
		
		
		/*
		** Build a map to move source values from the received order (which is essentially anything) to the order
		** specified in 'required'.
		*/
		
		if( sync->netSource ) {
			unsigned	proc_i;
			unsigned	srcInd = 0;
			
			/* Alloc memory. */
			srcMap = Memory_Alloc_Array( unsigned, sync->netSource, "SnacSync" );
			
			/* Map 'em. */
			for( proc_i = 0; proc_i < sync->nProcs; proc_i++ ) {
				unsigned	src_i;
				
				for( src_i = 0; src_i < sync->nSource[proc_i]; src_i++ ) {
					unsigned	req_i;
					
					for( req_i = 0; req_i < nRequired; req_i++ ) {
						if( sync->source[proc_i][src_i] == required[req_i] ) {
							srcMap[srcInd] = req_i;
							break;
						}
					}
					
					srcInd++;
				}
			}
		}
		else {
			srcMap = NULL;
		}

		
		/*
		** Build a mapping from sink indices to local indices and initialise the send/recv component of the sync class.
		*/
		
		{
			unsigned*	snkMap;
			
			if( sync->netSink ) {
				unsigned	proc_i;
				unsigned	snkInd = 0;
				
				snkMap = Memory_Alloc_Array( unsigned, sync->netSink, "SnacRemesher" );
				
				for( proc_i = 0; proc_i < sync->nProcs; proc_i++ ) {
					unsigned	snk_i;
					
					for( snk_i = 0; snk_i < sync->nSink[proc_i]; snk_i++ ) {
						snkMap[snkInd] = Mesh_NodeMapGlobalToLocal( mesh, sync->sink[proc_i][snk_i] );
						snkInd++;
					}
				}
			}
			else {
				snkMap = NULL;
			}
			
			/* Initialise. */
			SnacSync_SendRecvInitialise( sync, sizeof(double), snkMap, sizeof(Coord), srcMap, sizeof(double) );
			
			/* Free resources. */
			FreeArray( snkMap );
			FreeArray( srcMap );
		}
		
		/* Free other arrays. */
		FreeArray( locals );
		FreeArray( required );
	}
}


void _SnacRemesher_TriangulateXZPlane(
	void*		_context, 
	IndexSet*		planeNodes, 
	unsigned		gYInd, 
	unsigned*		nTriNodes, 
	unsigned**	triToDomain )
{
	Snac_Context*		context = (Snac_Context*)_context;
	Mesh*			mesh = context->mesh;
	HexaMD*			decomp = (HexaMD*)mesh->layout->decomp;
	IJK				gSize;
	IJK				dSize;
	IJK				pos;
	
	
	/*
	** Definitley not the fastest way of doing this, but it should work none-the-less.
	*/
	
	gSize[0] = decomp->nodeGlobal3DCounts[0];
	gSize[1] = decomp->nodeGlobal3DCounts[1];
	gSize[2] = decomp->nodeGlobal3DCounts[2];
	pos[1] = gYInd;
	
	/* Calculate the domain x size. */
	for( pos[2] = 0; pos[2] < gSize[2]; pos[2]++ ) {
		unsigned	xMin, xMax;
		Bool		found = False;
		
		xMax = 0;
		
		for( pos[0] = 0; pos[0] < gSize[0]; pos[0]++ ) {
			Node_GlobalIndex	gNodeInd;
			Node_DomainIndex	dNodeInd;
			
			gNodeInd = RegularMeshUtils_Node_Global3DToGlobal1D( decomp, pos[0], pos[1], pos[2] );
			dNodeInd = Mesh_NodeMapGlobalToDomain( mesh, gNodeInd );
			
			if( !found && dNodeInd < mesh->nodeDomainCount ) {
				xMin = pos[0];
				found = True;
			}
			else if( found && dNodeInd >= mesh->nodeDomainCount ) {
				xMax = pos[0];
				break;
			}
		}
		
		if( found ) {
			if( xMax == 0 ) {
				dSize[0] = gSize[0] - xMin;
			}
			else {
				dSize[0] = xMax - xMin;
			}
			break;
		}
	}
	
	/* Calculate the domain z size. */
	for( pos[0] = 0; pos[0] < gSize[0]; pos[0]++ ) {
		unsigned	zMin, zMax;
		Bool		found = False;
		
		zMax = 0;
		
		for( pos[2] = 0; pos[2] < gSize[2]; pos[2]++ ) {
			Node_GlobalIndex	gNodeInd;
			Node_DomainIndex	dNodeInd;
			
			gNodeInd = RegularMeshUtils_Node_Global3DToGlobal1D( decomp, pos[0], pos[1], pos[2] );
			dNodeInd = Mesh_NodeMapGlobalToDomain( mesh, gNodeInd );
			
			if( !found && dNodeInd < mesh->nodeDomainCount ) {
				zMin = pos[2];
				found = True;
			}
			else if( found && dNodeInd >= mesh->nodeDomainCount ) {
				zMax = pos[2];
				break;
			}
		}
		
		if( found ) {
			if( zMax == 0 ) {
				dSize[2] = gSize[2] - zMin;
			}
			else {
				dSize[2] = zMax - zMin;
			}
			break;
		}
	}
	
	
	/*
	** Set up the destination values/arrays.
	*/
	
	*nTriNodes = (dSize[0] - 1) * (dSize[2] - 1) * 6;
	*triToDomain = Memory_Alloc_Array( Node_DomainIndex, *nTriNodes, "SnacRemesher" );
	
	
	/*
	** Do the triangulation.
	*/
	
	{
		unsigned	curTri = 0;
		
		for( pos[2] = 0; pos[2] < gSize[2] - 1; pos[2]++ ) {
			for( pos[0] = 0; pos[0] < gSize[0] - 1; pos[0]++ ) {
				Node_GlobalIndex	gNodeInd;
				Node_DomainIndex	dNodeInd[4];
				
				gNodeInd = RegularMeshUtils_Node_Global3DToGlobal1D( decomp, pos[0], pos[1], pos[2] );
				dNodeInd[0] = Mesh_NodeMapGlobalToDomain( mesh, gNodeInd );
				gNodeInd = RegularMeshUtils_Node_Global3DToGlobal1D( decomp, pos[0] + 1, pos[1], pos[2] );
				dNodeInd[1] = Mesh_NodeMapGlobalToDomain( mesh, gNodeInd );
				gNodeInd = RegularMeshUtils_Node_Global3DToGlobal1D( decomp, pos[0] + 1, pos[1], pos[2] + 1 );
				dNodeInd[2] = Mesh_NodeMapGlobalToDomain( mesh, gNodeInd );
				gNodeInd = RegularMeshUtils_Node_Global3DToGlobal1D( decomp, pos[0], pos[1], pos[2] + 1 );
				dNodeInd[3] = Mesh_NodeMapGlobalToDomain( mesh, gNodeInd );
				
				if( dNodeInd[0] < mesh->nodeDomainCount && dNodeInd[1] < mesh->nodeDomainCount && 
				    dNodeInd[2] < mesh->nodeDomainCount && dNodeInd[3] < mesh->nodeDomainCount )
				{
					/* If we are here then we've found a valid planar element and can proceed to triangulate it. */
					(*triToDomain)[(curTri * 3) + 0] = dNodeInd[0];
					(*triToDomain)[(curTri * 3) + 1] = dNodeInd[1];
					(*triToDomain)[(curTri * 3) + 2] = dNodeInd[3];
					curTri++;
					
					(*triToDomain)[(curTri * 3) + 0] = dNodeInd[1];
					(*triToDomain)[(curTri * 3) + 1] = dNodeInd[2];
					(*triToDomain)[(curTri * 3) + 2] = dNodeInd[3];
					curTri++;
				}
			}
		}
	}
}





#if 0
/*
** Removed for a rewrite.  Luke - 02/07/2005
*/


/*
static void _SnacRemesher_CheckForShadowNode(
		Snac_Context*			context,
		SnacRemesher_Context*		contextExt,
		Node_GlobalIndex		node_gI,
		Node_LocalIndex			node_lI,
		IndexSet**			ShadowNodeSetCurrent,
		IndexSet**			ShadowNodeSetLocal,
		IndexSet**			ShadowNodeSetRemote );

static void _SnacRemesher_CheckForShadowElement(
		Snac_Context*			context,
		SnacRemesher_Context*		contextExt,
		Element_GlobalIndex		element_gI,
		Element_LocalIndex		element_lI,
		IndexSet**			ShadowElementSetCurrent,
		IndexSet**			ShadowElementSetLocal,
		IndexSet**			ShadowElementSetRemote );
*/


void _SnacRemesher_InitialConditions( void* _context, void* data ) {
	Snac_Context*				context = (Snac_Context*)_context;
	Mesh*					mesh = context->mesh;
	MeshLayout*				layout = mesh->layout;
	Topology*				eTopology = layout->elementLayout->topology;
	HexaMD*					decomp = (HexaMD*)layout->decomp;
	SnacRemesher_Mesh*			meshExt = ExtensionManager_Get(
							context->meshExtensionMgr,
							mesh,
							SnacRemesher_MeshHandle );
	IndexSet**				gWallSet;
	Partition_Index				rn_I;
	Partition_Index				rnCount = meshExt->neighbourRankCount;
	IndexSet**				shadowNodeSetLocal;
	IndexSet**				shadowNodeSetRemote;
	IndexSet**				shadowElementSetLocal;
	IndexSet**				shadowElementSetRemote;
	Node_GlobalIndex			node_gI;
	Element_GlobalIndex			element_gI;
	
	Journal_Printf( context->debug, "In: %s\n", __func__ );
	
	/* If cartesion mesh, merely make a copy of initial node coordinates */
	if( meshExt->meshType == SnacRemesher_Cartesian || meshExt->meshType == SnacRemesher_Spherical ) {
		memcpy( meshExt->initialNodeCoord, mesh->nodeCoord, sizeof(Coord) * meshExt->nodeLocalCount );
	}
	/* Else, it is a spherical mesh (construct extensions prevents otherwise), convert each one */
	else {
		Node_LocalIndex				node_lI;
		
		for( node_lI = 0; node_lI < meshExt->nodeLocalCount; node_lI++ ) {
			Coord*					spherical = &meshExt->initialNodeCoord[node_lI];
			Coord*					cartesian = &mesh->nodeCoord[node_lI];
			double 					sqXsqY =
									(*cartesian)[0] * (*cartesian)[0] +
									(*cartesian)[1] * (*cartesian)[1];
			
			/* At this point in time, spehrical coordinates are kept as radians, not degress as it evaluates faster */
			
			/* r = sqrt( sq( x ) + sq( y ) + sq( z ) ) */
			(*spherical)[1] = sqrt( sqXsqY + (*cartesian)[2] * (*cartesian)[2] );
			
			/* phi = arcsin( sqrt( sq( x ) + sq( y ) ) / z ) */
			(*spherical)[2] = asin( (*cartesian)[1] / (*spherical)[1] );
			
			/* theta = arctan( y / x ) */
			(*spherical)[0] = atan( (*cartesian)[0] / (*cartesian)[2] );
			
			if( (*cartesian)[2] < 0.0 ) {
				(*spherical)[0] += M_PI;
			}
			else if( (*cartesian)[0] < 0.0 ) {
				(*spherical)[0] += 2 * M_PI;
			}
			
			if( (*spherical)[0] > M_PI )
				(*spherical)[0] = -2.0 * M_PI + (*spherical)[0];
		}
	}
	
	/* Obtain neighbour rank info */
	_HexaMD_Shadow_BuildProcs( decomp, meshExt->neighbourRank );
	
	/* Build local temporary sets of the global walls and top-internal shadow nodes */
	gWallSet = (IndexSet**)malloc( sizeof(IndexSet*) * 6 );
	gWallSet[0] = RegularMeshUtils_CreateGlobalLeftSet( mesh );
	gWallSet[1] = RegularMeshUtils_CreateGlobalRightSet( mesh );
	gWallSet[2] = RegularMeshUtils_CreateGlobalBottomSet( mesh );
	gWallSet[3] = RegularMeshUtils_CreateGlobalBackSet( mesh );
	gWallSet[4] = RegularMeshUtils_CreateGlobalFrontSet( mesh );
	gWallSet[5] = RegularMeshUtils_CreateGlobalTopSet( mesh ); /* TODO: positive radial dir for cartesian */
	shadowNodeSetLocal = (IndexSet**)malloc( sizeof(IndexSet*) * rnCount );
	shadowNodeSetRemote = (IndexSet**)malloc( sizeof(IndexSet*) * rnCount );
	shadowElementSetLocal = (IndexSet**)malloc( sizeof(IndexSet*) * rnCount );
	shadowElementSetRemote = (IndexSet**)malloc( sizeof(IndexSet*) * rnCount );
	for( rn_I = 0; rn_I < rnCount; rn_I++ ) {
		shadowNodeSetLocal[rn_I] = IndexSet_New( mesh->nodeLocalCount );
		shadowNodeSetRemote[rn_I] = IndexSet_New( mesh->nodeGlobalCount );
		shadowElementSetLocal[rn_I] = IndexSet_New( mesh->elementLocalCount );
		shadowElementSetRemote[rn_I] = IndexSet_New( mesh->elementGlobalCount );
	}
	
	/* Build the "wall", "topInternal" and "internal" local node sets, and the node shadow sets */
	for( node_gI = 0; node_gI < meshExt->nodeGlobalCount; node_gI++ ) {
		Node_LocalIndex				node_lI;
		
		node_lI = Mesh_NodeMapGlobalToLocal( mesh, node_gI );
		if( node_lI < meshExt->nodeLocalCount ) {
			if( IndexSet_IsMember( gWallSet[5], node_lI ) ) {
				IndexSet_Add( meshExt->topInternalSet, node_lI );
				/*dbPrint3(); fflush( stdout );

				_SnacRemesher_CheckForShadowNode(
					context,
					contextExt,
					node_gI,
					node_lI,
					ShadowNodeSetCurrent,
					ShadowNodeSetLocal,
					ShadowNodeSetRemote );*/
			}
			else if( IndexSet_IsMember( gWallSet[2], node_lI ) ) {
				IndexSet_Add( meshExt->bottomInternalSet, node_lI );
				/*dbPrint3_2(); fflush( stdout );

				_SnacRemesher_CheckForShadowNode(
					context,
					contextExt,
					node_gI,
					node_lI,
					ShadowNodeSetCurrent,
					ShadowNodeSetLocal,
					ShadowNodeSetRemote );*/
			}
			/* The walls (except for positive radial dir) remesh back to initial positions...
			   maintain as one local set */
			else if(
				IndexSet_IsMember( gWallSet[0], node_lI ) ||
				IndexSet_IsMember( gWallSet[1], node_lI ) ||
				IndexSet_IsMember( gWallSet[3], node_lI ) ||
				IndexSet_IsMember( gWallSet[4], node_lI ) )
			{
				IndexSet_Add( meshExt->wallSet, node_lI );
				/*dbPrint2();

				_SnacRemesher_CheckForShadowNode(
					context,
					contextExt,
					node_gI,
					node_lI,
					ShadowNodeSetCurrent,
					ShadowNodeSetLocal,
					ShadowNodeSetRemote );*/
			}
			/* The positive radial dir wall internal nodes are free...
			   maintain as one local set */

			/* Internal nodes are interpolated in the radial dir...
			   maintain as one set */
			else {
				IndexSet_Add( meshExt->internalSet, node_lI );
			}
		}
		else Journal_DPrintf( context->debug, "node_gI: %3u node_lI: %3u, not local\n", node_gI, node_lI );
	}
	
	/* Build the element shadow sets */
	for( element_gI = 0; element_gI < meshExt->elementGlobalCount; element_gI++ ) {
		Element_LocalIndex			element_lI;
		Element_GlobalIndex			en_I;

		element_lI = decomp->elementMapGlobalToLocal( decomp, element_gI );
		if( element_lI < meshExt->elementLocalCount ) {
			Element_GlobalIndex			eNbrCnt;
			eNbrCnt = eTopology->neighbourCount( eTopology, element_gI );
			if( eNbrCnt ) {
				Element_GlobalIndex*			eNbrs;

				eNbrs = (Element_GlobalIndex*)malloc( sizeof(Element_GlobalIndex) * eNbrCnt );
				eTopology->buildNeighbours( eTopology, element_gI, eNbrs );
				
				for( rn_I = 0; rn_I < meshExt->neighbourRankCount; rn_I++ ) {
					Partition_Index		rank;

					rank = meshExt->neighbourRank[rn_I];

					for( en_I = 0; en_I < eNbrCnt; en_I++ ) {
						if( eNbrs[en_I] < meshExt->elementGlobalCount &&
							IndexSet_IsMember( decomp->localElementSets[rank], eNbrs[en_I] ) &&
							!IndexSet_IsMember(
								decomp->localElementSets[context->rank],
								eNbrs[en_I] ) )
						{
							IndexSet_Add( shadowElementSetRemote[rn_I], eNbrs[en_I] );
						}
					}
				}

				if( eNbrs )
					free( eNbrs );
			}
		}
	}
	
	/* Resolve the "wall", "topInternal" and "internal" sets into array of indices (for speed) */
	IndexSet_GetMembers( meshExt->wallSet, &meshExt->wallCount, &meshExt->wallArray );
	IndexSet_GetMembers( meshExt->topInternalSet, &meshExt->topInternalCount, &meshExt->topInternalArray );
	IndexSet_GetMembers( meshExt->bottomInternalSet, &meshExt->bottomInternalCount, &meshExt->bottomInternalArray );
	IndexSet_GetMembers( meshExt->internalSet, &meshExt->internalCount, &meshExt->internalArray );
	
	/* Build shadow tables and arrays */
	meshExt->shadowNodeLocalCount = (Node_LocalIndex*)malloc( sizeof(Node_LocalIndex) * rnCount );
	meshExt->shadowNodeLocalArray = (Node_LocalIndex**)malloc( sizeof(Node_LocalIndex*) * rnCount );
	meshExt->shadowNodeRemoteCount = (Node_ShadowIndex*)malloc( sizeof(Node_ShadowIndex) * rnCount );
	meshExt->shadowNodeRemoteArray = (Node_GlobalIndex**)malloc( sizeof(Node_GlobalIndex*) * rnCount );
	meshExt->shadowNodeRemoteCoord = (Coord**)malloc( sizeof(Coord*) * rnCount );
	meshExt->shadowNodeRemote = (Snac_Node**)malloc( sizeof(Snac_Node*) * rnCount );
	meshExt->shadowElementLocalCount = (Element_LocalIndex*)malloc( sizeof(Element_LocalIndex) * rnCount );
	meshExt->shadowElementLocalArray = (Element_LocalIndex**)malloc( sizeof(Element_LocalIndex*) * rnCount );
	meshExt->shadowElementRemoteCount = (Element_ShadowIndex*)malloc( sizeof(Element_ShadowIndex) * rnCount );
	meshExt->shadowElementRemoteArray = (Element_GlobalIndex**)malloc( sizeof(Element_GlobalIndex*) * rnCount );
	meshExt->shadowElementRemote = (Snac_Element**)malloc( sizeof(Snac_Element*) * rnCount );
	for( rn_I = 0; rn_I < rnCount; rn_I++ ) {
		Element_LocalIndex		e_lI;
		Node_LocalIndex			n_lI;
		MPI_Status			status;
		Element_ShadowIndex		se_I;

		/* temp */
		meshExt->shadowNodeLocalCount[rn_I] = 0;
		meshExt->shadowNodeRemoteCount[rn_I] = 0;
		
		/* convert shadow elements to arrays */
		IndexSet_GetMembers(
			shadowElementSetRemote[rn_I],
			&meshExt->shadowElementRemoteCount[rn_I],
			&meshExt->shadowElementRemoteArray[rn_I] );
		/* transfer to neighbours - send size */
		MPI_Send(
			&meshExt->shadowElementRemoteCount[rn_I],
			1,
			MPI_INT,
			meshExt->neighbourRank[rn_I],
			0,
			decomp->communicator );
		/* send data */
		MPI_Send(
			meshExt->shadowElementRemoteArray[rn_I],
			meshExt->shadowElementRemoteCount[rn_I],
			MPI_INT,
			meshExt->neighbourRank[rn_I],
			7,
			decomp->communicator );
		/* recv size */
		MPI_Recv(
			&meshExt->shadowElementLocalCount[rn_I],
			1,
			MPI_INT,
			meshExt->neighbourRank[rn_I],
			0,
			decomp->communicator,
			&status );
		meshExt->shadowElementLocalArray[rn_I] = (Index*)malloc(
			sizeof(Element_LocalIndex) * meshExt->shadowElementLocalCount[rn_I] );
		/* recv data */
		MPI_Recv(
			meshExt->shadowElementLocalArray[rn_I],
			meshExt->shadowElementLocalCount[rn_I],
			MPI_INT,
			meshExt->neighbourRank[rn_I],
			7,
			decomp->communicator,
			&status );
		/* loop over data and convert to local indices */
		for( e_lI = 0; e_lI < meshExt->shadowElementLocalCount[rn_I]; e_lI++ ) {
			Index   element_lI = Mesh_ElementMapGlobalToLocal( mesh, meshExt->shadowElementLocalArray[rn_I][e_lI] );

			if( element_lI >= meshExt->elementLocalCount )
				assert( 0 ); /* Big trouble */
			
			meshExt->shadowElementLocalArray[rn_I][e_lI] = element_lI;
		}
		/* setup the remaining element info */
		meshExt->shadowElementRemote[rn_I] = (Snac_Element*)ExtensionManager_Malloc(
			mesh->elementExtensionMgr,
			meshExt->shadowElementRemoteCount[rn_I] );
		ExtensionManager_Memset(
			mesh->elementExtensionMgr,
			meshExt->shadowElementRemote[rn_I],
			0,
			meshExt->shadowElementRemoteCount[rn_I] );
		
		/* Loop over shadow elements to calculate node info */
		for( se_I = 0; se_I < meshExt->shadowElementRemoteCount[rn_I]; se_I++ ) {
			NodeLayout*     nodeLayout = mesh->layout->nodeLayout;
			Index		sen_I, seNodeCnt;
			Index*		seNodes;

			/* extract nodes */
			seNodeCnt = nodeLayout->elementNodeCount( nodeLayout, meshExt->shadowElementRemoteArray[rn_I][se_I] );
			seNodes = (Index*)malloc( sizeof(Index) * seNodeCnt );
			nodeLayout->buildElementNodes( nodeLayout, meshExt->shadowElementRemoteArray[rn_I][se_I], seNodes );

			for( sen_I = 0; sen_I < seNodeCnt; sen_I++ ) {
				Index localNode = Mesh_NodeMapGlobalToLocal( mesh, seNodes[sen_I] );

				if( localNode >= meshExt->nodeLocalCount ) {
					IndexSet_Add( shadowNodeSetRemote[rn_I], seNodes[sen_I] );
				}
			}

			if( seNodes )
				free( seNodes );
		}
		/* convert to array */
		IndexSet_GetMembers(
			shadowNodeSetRemote[rn_I],
			&meshExt->shadowNodeRemoteCount[rn_I],
			&meshExt->shadowNodeRemoteArray[rn_I] );
		/* transfer to neighbours - send size */
		MPI_Send(
			&meshExt->shadowNodeRemoteCount[rn_I],
			1,
			MPI_INT,
			meshExt->neighbourRank[rn_I],
			0,
			decomp->communicator );
		/* send data */
		MPI_Send(
			meshExt->shadowNodeRemoteArray[rn_I],
			meshExt->shadowNodeRemoteCount[rn_I],
			MPI_INT,
			meshExt->neighbourRank[rn_I],
			7,
			decomp->communicator );
		/* recv size */
		MPI_Recv(
			&meshExt->shadowNodeLocalCount[rn_I],
			1,
			MPI_INT,
			meshExt->neighbourRank[rn_I],
			0,
			decomp->communicator,
			&status );
		meshExt->shadowNodeLocalArray[rn_I] = (Index*)malloc( sizeof(Index) * meshExt->shadowNodeLocalCount[rn_I] );
		/* recv data */
		MPI_Recv(
			meshExt->shadowNodeLocalArray[rn_I],
			meshExt->shadowNodeLocalCount[rn_I],
			MPI_INT,
			meshExt->neighbourRank[rn_I],
			7,
			decomp->communicator,
			&status );
		/* loop over data and convert to local indices */
		for( n_lI = 0; n_lI < meshExt->shadowNodeLocalCount[rn_I]; n_lI++ ) {
			Index   node_lI = Mesh_NodeMapGlobalToLocal( mesh, meshExt->shadowNodeLocalArray[rn_I][n_lI] );

			if( node_lI >= meshExt->nodeLocalCount )
				assert( 0 ); /* something is terribly wrong */
			
			meshExt->shadowNodeLocalArray[rn_I][n_lI] = node_lI;
		}
		/* setup remaining node info */
		meshExt->shadowNodeRemoteCoord[rn_I] = (Coord*)malloc( sizeof(Coord) * meshExt->shadowNodeRemoteCount[rn_I] );
		memset( meshExt->shadowNodeRemoteCoord[rn_I], 0, sizeof(Coord) * meshExt->shadowNodeRemoteCount[rn_I] );
		meshExt->shadowNodeRemote[rn_I] = (Snac_Node*)ExtensionManager_Malloc(
			mesh->nodeExtensionMgr,
			meshExt->shadowNodeRemoteCount[rn_I] );
		ExtensionManager_Memset( mesh->nodeExtensionMgr,  meshExt->shadowNodeRemote[rn_I], 0, meshExt->shadowNodeRemoteCount[rn_I] );
	}
	
	/* Delete temporary sets */
	for( rn_I = 0; rn_I < rnCount; rn_I++ ) {
		Stg_Class_Delete( shadowElementSetRemote[rn_I]  );
		Stg_Class_Delete( shadowElementSetLocal[rn_I]  );
		Stg_Class_Delete( shadowNodeSetRemote[rn_I]  );
		Stg_Class_Delete( shadowNodeSetLocal[rn_I]  );
	}
	free( shadowElementSetRemote );
	free( shadowElementSetLocal );
	free( shadowNodeSetRemote );
	free( shadowNodeSetLocal );
	Stg_Class_Delete( gWallSet[5] );
	Stg_Class_Delete( gWallSet[4] );
	Stg_Class_Delete( gWallSet[3] );
	Stg_Class_Delete( gWallSet[2] );
	Stg_Class_Delete( gWallSet[1] );
	Stg_Class_Delete( gWallSet[0] );
	free( gWallSet );
}
#endif

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
** $Id: Build.c 3095 2005-07-13 09:50:46Z LukeHodkinson $
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
#include "Build.h"


void _SnacRemesher_Build( void* _context, void* data ) {
	Snac_Context*		context = (Snac_Context*)_context;
	Mesh*			mesh = context->mesh;
	SnacRemesher_Mesh*	meshExt = ExtensionManager_Get(
						context->meshExtensionMgr,
						mesh,
						SnacRemesher_MeshHandle );
	

	Journal_Printf( context->debug, "In: %s\n", __func__ );
	
	/* Alloc space for the remeshed node coords. */
	meshExt->newNodeCoords = Memory_Alloc_Array( Coord, mesh->nodeLocalCount, "SnacRemesher" );
	
	/* Set all else to empty values. */
	meshExt->newNodes = NULL;
	meshExt->newElements = NULL;
	meshExt->nExternalNodes = 0;
	meshExt->externalNodes = NULL;
	meshExt->nExternalElements = 0;
	meshExt->externalElements = NULL;
	meshExt->nTopInternNodes = 0;
	meshExt->topInternToLocal = NULL;
	meshExt->nBotInternNodes = 0;
	meshExt->botInternToLocal = NULL;
	meshExt->nTopTriNodes = 0;
	meshExt->topTriToDomain = NULL;
	meshExt->nBotTriNodes = 0;
	meshExt->botTriToDomain = NULL;
	meshExt->nYLines = 0;
	meshExt->yLineLTerm = NULL;
	meshExt->yLineUTerm = NULL;
	meshExt->yLineNYInds = NULL;
	meshExt->yLineYInds = NULL;
	meshExt->yLineLocalInds = NULL;
}



#if 0

void _SnacRemesher_Build( void* _context, void* data ) {
	Snac_Context*		context = (Snac_Context*)_context;
	Mesh*			mesh = context->mesh;
	SnacRemesher_Mesh*	meshExt = ExtensionManager_Get(
						context->meshExtensionMgr,
						mesh,
						SnacRemesher_MeshHandle );

	Journal_Printf( context->debug, "In: %s\n", __func__ );

	/* Keep local copies of the node count info */
	meshExt->nodeLocalCount = mesh->nodeLocalCount;
	meshExt->nodeDomainCount = mesh->nodeDomainCount;
	meshExt->nodeGlobalCount = mesh->nodeGlobalCount;
	meshExt->elementLocalCount = mesh->elementLocalCount;
	meshExt->elementDomainCount = mesh->elementDomainCount;
	meshExt->elementGlobalCount = mesh->elementGlobalCount;
	Journal_Printf(
		context->snacDebug,
		"Remesher is being told: nodeLocalCount: %u, nodeDomainCount: %u, nodeGlobalCount: %u\n",
		meshExt->nodeLocalCount,
		meshExt->nodeDomainCount,
		meshExt->nodeGlobalCount );
	Journal_Printf(
		context->snacDebug,
		"Remesher is being told:  elementLocalCount: %u, elementDomainCount %u, elementGlobalCount: %u\n",
		meshExt->elementLocalCount,
		meshExt->elementDomainCount,
		meshExt->elementGlobalCount );

	/* Alloc mem for initial coordinates array. We need only local coordinates here */
	meshExt->initialNodeCoord = (Coord*)malloc( sizeof(Coord) * meshExt->nodeLocalCount );

	/* Alloc mem for scratch space for calculating new interpolated nodes and elements. Include shadow nodes too.
	   TODO: node and element mem allocation SHOULD be a function provided by the Mesh class */
	meshExt->newNodeCoord = (Coord*)malloc( sizeof(Coord) * meshExt->nodeDomainCount );
	meshExt->newNode = (Snac_Node*)ExtensionManager_Malloc( context->mesh->nodeExtensionMgr, meshExt->nodeDomainCount );
	meshExt->newElement = (Snac_Element*)ExtensionManager_Malloc( context->mesh->elementExtensionMgr, meshExt->elementDomainCount );

	/* Alloc mem for knowing who our neighbour ranks are */
	meshExt->neighbourRankCount = _HexaMD_Shadow_ProcCount( (HexaMD*)mesh->layout->decomp );
	meshExt->neighbourRank = (Partition_Index*)malloc( sizeof(Partition_Index) * meshExt->neighbourRankCount );

	/* Top surface shadowing */
	meshExt->shadowNodeLocalCount = 0;
	meshExt->shadowNodeLocalArray = 0;
	meshExt->shadowNodeRemoteCount = 0;
	meshExt->shadowNodeRemoteArray = 0;
	meshExt->shadowNodeRemoteCoord = 0;
	meshExt->shadowNodeRemote = 0;
	meshExt->shadowElementLocalCount = 0;
	meshExt->shadowElementLocalArray = 0;
	meshExt->shadowElementRemoteCount = 0;
	meshExt->shadowElementRemoteArray = 0;
	meshExt->shadowElementRemote = 0;

	meshExt->topInternalSet = IndexSet_New( meshExt->nodeLocalCount );
	meshExt->bottomInternalSet = IndexSet_New( meshExt->nodeLocalCount );
	meshExt->wallSet = IndexSet_New( meshExt->nodeLocalCount );
	meshExt->internalSet = IndexSet_New( meshExt->nodeLocalCount );
	meshExt->topInternalCount = 0;
	meshExt->bottomInternalCount = 0;
	meshExt->wallCount = 0;
	meshExt->internalCount = 0;
	meshExt->topInternalArray = 0;
	meshExt->bottomInternalArray = 0;
	meshExt->wallArray = 0;
	meshExt->internalArray = 0;
}

#endif

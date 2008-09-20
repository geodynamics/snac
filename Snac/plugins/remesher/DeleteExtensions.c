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
** $Id: DeleteExtensions.c 3095 2005-07-13 09:50:46Z LukeHodkinson $
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
#include "DeleteExtensions.h"


void _SnacRemesher_DeleteExtensions( void* _context, void* data ) {
	Snac_Context*				context = (Snac_Context*)_context;
	Mesh*					mesh = context->mesh;
	SnacRemesher_Mesh*			meshExt = ExtensionManager_Get(
							context->meshExtensionMgr,
							mesh,
							SnacRemesher_MeshHandle );
	
	Journal_Printf( context->debug, "In: %s\n", __func__ );
	
	
	/*
	** Probably will not need to do this.
	*/
	
	if( meshExt->newNodes ) {
		ExtensionManager_Free( mesh->nodeExtensionMgr, meshExt->newNodes );
	}
	
	if( meshExt->newElements ) {
		ExtensionManager_Free( mesh->nodeExtensionMgr, meshExt->newElements );
	}
	
	
	/*
	** Kill the mesh extension.
	*/
	
	KillArray( meshExt->newNodeCoords );
	KillArray( meshExt->externalNodes );
	KillArray( meshExt->externalElements );
	KillArray( meshExt->topInternToLocal );
	KillArray( meshExt->botInternToLocal );
	KillArray( meshExt->topTriToDomain );
	KillArray( meshExt->botTriToDomain );
	KillArray( meshExt->yLineLTerm );
	KillArray( meshExt->yLineUTerm );
	KillArray( meshExt->yLineNYInds );
	KillArray2D( meshExt->nYLines, meshExt->yLineYInds );
	KillArray2D( meshExt->nYLines, meshExt->yLineLocalInds );
}

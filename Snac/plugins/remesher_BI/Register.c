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
** $Id: Register.c 3243 2006-10-12 09:04:00Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "EntryPoint.h"
#include "Context.h"
#include "Mesh.h"
#include "ConstructExtensions.h"
#include "Build.h"
#include "InitialConditions.h"
#include "Remesh.h"
#include "DeleteExtensions.h"
#include "Register.h"
#include <stdio.h>

/* Textual name of this class */
const Type SnacRemesher_Type = "SnacRemesher";

ExtensionInfo_Index SnacRemesher_ContextHandle;
ExtensionInfo_Index SnacRemesher_MeshHandle;

const Name SnacRemesher_EP_InterpolateNode =		"SnacRemesher_EP_InterpolateNode";
const Name SnacRemesher_EP_InterpolateElement =		"SnacRemesher_EP_InterpolateElement";


Index _SnacRemesher_Register( PluginsManager* pluginsMgr ) {
	return PluginsManager_Submit( pluginsMgr, 
				      SnacRemesher_Type, 
				      "0", 
				      _SnacRemesher_DefaultNew );
}


void* _SnacRemesher_DefaultNew( Name name ) {
	return _Codelet_New( sizeof(Codelet), 
			     SnacRemesher_Type, 
			     _Codelet_Delete, 
			     _Codelet_Print, 
			     _Codelet_Copy, 
			     _SnacRemesher_DefaultNew, 
			     _SnacRemesher_Construct, 
			     _Codelet_Build, 
			     _Codelet_Initialise, 
			     _Codelet_Execute, 
			     _Codelet_Destroy, 
			     name );
}


void _SnacRemesher_Construct( void* component, Stg_ComponentFactory* cf, void* data ) {
	Snac_Context*	context;

	/* Retrieve context. */
	context = (Snac_Context*)Stg_ComponentFactory_ConstructByName( cf, "context", Snac_Context, True, data ); 
	
	Journal_Printf( context->debug, "In: %s\n", __func__ );

	/* Add extensions to nodes, elements and the context */
	SnacRemesher_ContextHandle = ExtensionManager_Add(
		context->extensionMgr,
		SnacRemesher_Type,
		sizeof(SnacRemesher_Context) );
	SnacRemesher_MeshHandle = ExtensionManager_Add(
		context->meshExtensionMgr,
		SnacRemesher_Type,
		sizeof(SnacRemesher_Mesh) );

	Journal_Printf( context->debug, "\tcontext extension handle: %u\n", SnacRemesher_ContextHandle );
	Journal_Printf( context->debug, "\tmesh extension handle: %u\n", SnacRemesher_MeshHandle );

	/* Register new entry points to the context (which manages them) */
	Context_AddEntryPoint(
		context,
		SnacRemesher_EntryPoint_New( SnacRemesher_EP_InterpolateNode, SnacRemesher_InterpolateNode_CastType ) );
	Context_AddEntryPoint(
		context,
		SnacRemesher_EntryPoint_New( SnacRemesher_EP_InterpolateElement, SnacRemesher_InterpolateElement_CastType ) );

	/* Add extensions to the entry points */
	EntryPoint_Append(
		Context_GetEntryPoint( context, AbstractContext_EP_Build ),
		SnacRemesher_Type,
		_SnacRemesher_Build,
		SnacRemesher_Type );
	EntryPoint_Append(
		Context_GetEntryPoint( context, AbstractContext_EP_Initialise ),
		SnacRemesher_Type,
		_SnacRemesher_InitialConditions,
		SnacRemesher_Type );
	EntryPoint_Append(
		Context_GetEntryPoint( context, AbstractContext_EP_Sync ),
		SnacRemesher_Type,
		_SnacRemesher_Remesh,
		SnacRemesher_Type );
	EntryPoint_Append(
		Context_GetEntryPoint( context, SnacRemesher_EP_InterpolateNode ),
		SnacRemesher_Type,
		_SnacRemesher_InterpolateNode,
		SnacRemesher_Type );
	EntryPoint_Append(
		Context_GetEntryPoint( context, SnacRemesher_EP_InterpolateElement ),
		SnacRemesher_Type,
		_SnacRemesher_InterpolateElement,
		SnacRemesher_Type );
	EntryPoint_Append(
		Context_GetEntryPoint( context, AbstractContext_EP_DestroyExtensions ),
		SnacRemesher_Type,
		_SnacRemesher_DeleteExtensions,
		SnacRemesher_Type );

	/* Construct. */
	_SnacRemesher_ConstructExtensions( context, data );
}

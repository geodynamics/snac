/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003, Victorian Partnership for Advanced Computing (VPAC) Ltd, 110 Victoria Street, Melbourne, 3053, Australia.
**
** Authors:
**	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
**	Patrick D. Sunter, Software Engineer, VPAC. (pds@vpac.org)
**	Luke J. Hodkinson, Computational Engineer, VPAC. (lhodkins@vpac.org)
**	Siew-Ching Tan, Software Engineer, VPAC. (siew@vpac.org)
**	Alan H. Lo, Computational Engineer, VPAC. (alan@vpac.org)
**	Raquibul Hassan, Computational Engineer, VPAC. (raq@vpac.org)
**
**  This library is free software; you can redistribute it and/or
**  modify it under the terms of the GNU Lesser General Public
**  License as published by the Free Software Foundation; either
**  version 2.1 of the License, or (at your option) any later version.
**
**  This library is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
**  Lesser General Public License for more details.
**
**  You should have received a copy of the GNU Lesser General Public
**  License along with this library; if not, write to the Free Software
**  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
**
** $Id: MeshContext.c 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "StGermain.h"

#include "types.h"
#include "shortcuts.h"
#include "MeshContext.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


/* Textual name of this class */
const Type MeshContext_Type = "MeshContext";


MeshContext* _MeshContext_New(
		SizeT				sizeOfSelf,
		Type				type,
		Stg_Class_DeleteFunction*		_delete,
		Stg_Class_PrintFunction*		_print,
		Stg_Class_CopyFunction*		_copy, 
		Stg_Component_DefaultConstructorFunction*	_defaultConstructor,
		Stg_Component_ConstructFunction*			_construct,
		Stg_Component_BuildFunction*		_build,
		Stg_Component_InitialiseFunction*		_initialise,
		Stg_Component_ExecuteFunction*		_execute,
		Stg_Component_DestroyFunction*		_destroy,
		Name							name,
		Bool						initFlag,
		AbstractContext_SetDt*		_setDt,
		double				start,
		double				stop,
		MeshLayout*			meshLayout,
		SizeT				nodeSize,
		SizeT				elementSize,
		MPI_Comm			communicator,
		Dictionary*			dictionary )
{
	MeshContext* self;
	
	/* Allocate memory */
	self = (MeshContext*)_AbstractContext_New( sizeOfSelf, type, _delete, _print, _copy,
			_defaultConstructor, _construct, _build, _initialise, _execute, _destroy, name, initFlag,
			_setDt, start, stop, communicator, dictionary );
	
	/* General info */
	
	/* Virtual info */
	
	_MeshContext_Init( self, meshLayout, nodeSize, elementSize );
	
	return self;
}

void _MeshContext_Init( MeshContext* self, MeshLayout* meshLayout, SizeT nodeSize, SizeT elementSize ) {
	Dictionary* meshDict;

	/* General and Virtual info should already be set */
	
	/* MeshContext info */
	self->meshLayout = meshLayout;
	meshDict = Dictionary_Entry_Value_AsDictionary( Dictionary_Get( self->dictionary, "mesh" ) );
	self->mesh = Mesh_New( "Mesh", self->meshLayout, nodeSize, elementSize, self->extensionMgr_Register,
		meshDict ? meshDict : self->dictionary );
	self->meshExtensionMgr = ExtensionManager_New_OfExistingObject( "mesh", self->mesh );
	ExtensionManager_Register_Add( self->extensionMgr_Register, self->meshExtensionMgr ); 
	
	/* Add hooks to entry points */
	EntryPoint_Append( Context_GetEntryPoint( self, AbstractContext_EP_Build ), "defaultMeshBuild",
		_MeshContext_Build, MeshContext_Type );
	EntryPoint_Append( Context_GetEntryPoint( self, AbstractContext_EP_Initialise ), "defaultMeshICs",
		_MeshContext_InitialConditions,	MeshContext_Type );
}


void _MeshContext_Delete( void* meshContext ) {
	MeshContext* self = (MeshContext*)meshContext;
	
	/* NOTE: as ExtensionManager_Free() is currently coded (21 April 2004) the below
	3 lines are a bug - all ExtensionManager_Free does in this case is prematurely
	delete the mesh object itself. Not sure if something else
	should be there, or if ExtensionManager_Free() needs to be re-coded. */
	#if 0
	if( self->mesh ) {
		ExtensionManager_Free( self->meshExtensionMgr, self->mesh );
	}
	#endif
	Stg_Class_Delete( self->mesh );
	
	/* Stg_Class_Delete parent */
	_AbstractContext_Delete( meshContext );
}

void _MeshContext_Print( void* meshContext, Stream* stream ) {
	MeshContext* self = (MeshContext*)meshContext;
	
	/* General info */
	Journal_Printf( (void*) stream, "MeshContext (%p):\n", self );
	
	/* Virtual info */
	
	/* MeshContext info */
	Print( self->meshLayout, stream );
	Print( self->mesh, stream );
	Print( self->meshExtensionMgr, stream );
	
	/* Print parent */
	_AbstractContext_Print( meshContext, stream );
}


void _MeshContext_Build( Context* context, void* data ) {
	MeshContext* self = (MeshContext*)context;
	
	Journal_Printf( self->debug, "In: %s\n", __func__ );
	
	Build( self->mesh, data, False );
}

void _MeshContext_InitialConditions( Context* context, void* data ) {
	MeshContext* self = (MeshContext*)context;
	
	Journal_Printf( self->debug, "In: %s\n", __func__ );
	
	Initialise( self->mesh, data, False );
}

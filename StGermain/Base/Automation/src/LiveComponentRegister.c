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
**
**
** Assumptions:
**
** Comments:
**
** $Id: LiveComponentRegister.c 2745 2005-05-10 08:12:18Z RaquibulHassan $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdarg.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"

#include "types.h"
#include "shortcuts.h"
#include "Stg_Component.h"
#include "LiveComponentRegister.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>


/* Textual name of this class */
const Type LiveComponentRegister_Type = "LiveComponentRegister";
LiveComponentRegister* stgLiveComponentRegister = NULL;

LiveComponentRegister* _LiveComponentRegister_New( 
		SizeT					_sizeOfSelf, 
		Type					type,
		Stg_Class_DeleteFunction*			_delete,
		Stg_Class_PrintFunction*			_print, 
		Stg_Class_CopyFunction*			_copy )
{
	LiveComponentRegister *self = NULL;

	assert( _sizeOfSelf >= sizeof( LiveComponentRegister ) );
	self = (LiveComponentRegister*) _Stg_Class_New( _sizeOfSelf, type, _delete, _print, _copy );
	
	return self;
}
	
LiveComponentRegister* LiveComponentRegister_New(  )
{
	LiveComponentRegister *self = NULL;

	self = _LiveComponentRegister_New( sizeof( LiveComponentRegister ), 
					LiveComponentRegister_Type,
					_LiveComponentRegister_Delete,
					_LiveComponentRegister_Print,
					NULL);

	LiveComponentRegister_Init( self );

	if ( !stgLiveComponentRegister ) {
		stgLiveComponentRegister = self;
	}
	return self;
}

void _LiveComponentRegister_Init( LiveComponentRegister *self )
{
}
	
void LiveComponentRegister_Init( LiveComponentRegister *self )
{
	assert( self );

	self->componentList = Stg_ObjectList_New( );
}

/*--------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _LiveComponentRegister_Delete( void* liveComponentRegister )
{
	LiveComponentRegister *self = (LiveComponentRegister *) liveComponentRegister;

	assert( self );

	// TODO: add some debug printing about this...
	LiveComponentRegister_DeleteAll( self );
	Stg_Class_Delete( self->componentList );

	/* Note: this has to come after the LCRegister delete all, in case any of the
	components need to refer to it as part of their delete phase */
	if ( stgLiveComponentRegister == self ) {
		stgLiveComponentRegister = NULL;
	}

	_Stg_Class_Delete( self );
}

void _LiveComponentRegister_Print( void* liveComponentRegister, Stream* stream )
{
	LiveComponentRegister *self = (LiveComponentRegister *) liveComponentRegister;

	assert( self );
	
	/* General info */
	Journal_Printf( (void*) stream, "LiveComponentRegister (ptr): %p\n", self );
	
	/* Print parent class */
	_Stg_Class_Print( self, stream );
	
	Journal_Printf( (void*) stream, "componentList (ptr): %p\n", self->componentList );
	Stg_Class_Print( self->componentList, stream );
}

/*--------------------------------------------------------------------------------------------------------------------------
** Functions
*/

Index LiveComponentRegister_Add( LiveComponentRegister *self, Stg_Component *component )
{
	assert( self );
	return Stg_ObjectList_Append( self->componentList, component );
}
	
Stg_Component *LiveComponentRegister_Get( LiveComponentRegister *self, Name name )
{
	assert( self );
	
	return ( Stg_Component* ) Stg_ObjectList_Get( self->componentList, name );
}

Stg_Component *LiveComponentRegister_At( void* liveComponentRegister, Index index ) {
	LiveComponentRegister* self      = (LiveComponentRegister *) liveComponentRegister;
	assert( self );

	return ( Stg_Component* ) Stg_ObjectList_At( self->componentList, index );
}


Index LiveComponentRegister_RemoveOneComponentsEntry( void* liveComponentRegister, Name name ) {
	LiveComponentRegister* self      = (LiveComponentRegister *) liveComponentRegister;
	assert( self );

	/* Note: as specified in the header, we don't want to actually delete the component, just
	the entry */
	return Stg_ObjectList_Remove( self->componentList, name, KEEP );
}

	
unsigned int LiveComponentRegister_GetCount( LiveComponentRegister *self )
{
	assert( self );

	return self->componentList->count;
}


void LiveComponentRegister_BuildAll( void* liveComponentRegister, void* data ) {
	LiveComponentRegister* self      = (LiveComponentRegister *) liveComponentRegister;
	Stg_Component*             component;
	Index                  index;
	Index                  count     = LiveComponentRegister_GetCount( self );
	
	for ( index = 0 ; index < count ; index++ ) {
		component = LiveComponentRegister_At( self, index );
		Build( component, data, False );
	}
}


void LiveComponentRegister_InitialiseAll( void* liveComponentRegister, void* data ) {
	LiveComponentRegister* self      = (LiveComponentRegister *) liveComponentRegister;
	Stg_Component*             component;
	Index                  index;
	Index                  count     = LiveComponentRegister_GetCount( self );
	
	for ( index = 0 ; index < count ; index++ ) {
		component = LiveComponentRegister_At( self, index );
		Initialise( component, data, False );
	}
}


void LiveComponentRegister_DeleteAll( void* liveComponentRegister ) {
	LiveComponentRegister* self      = (LiveComponentRegister *) liveComponentRegister;
	Stg_Component*             component;
	Index                  index;
	
	/* Note: have to recompute count dynamically each loop in case a component deletes
	some other components */
	for ( index = 0 ; index < LiveComponentRegister_GetCount( self ) ; index++ ) {
		component = LiveComponentRegister_At( self, index );
		//printf( "Freeing component \"%s\" (type %s)\n", component->name, component->type );
		Stg_Class_Delete( component );
	}
	self->componentList->count = 0;
}


LiveComponentRegister* LiveComponentRegister_GetLiveComponentRegister() {
	return stgLiveComponentRegister;
}

void LiveComponentRegister_PrintSimilar( void* liveComponentRegister, Name name, Stream* stream, unsigned int number ) {
	LiveComponentRegister* self      = (LiveComponentRegister *) liveComponentRegister;

	Stg_ObjectList_PrintSimilar( self->componentList, name, stream, number );
}


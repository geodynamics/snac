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
** $Id: EntryPoint_Register.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdarg.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"
#include "Base/Automation/Automation.h"

#include "types.h"
#include "shortcuts.h"
#include "EntryPoint.h"
#include "EntryPoint_Register.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

/* Textual name of this class */
const Type EntryPoint_Register_Type = "EntryPoint_Register";


EntryPoint_Register* EntryPoint_Register_New( void ) {
	return _EntryPoint_Register_New( sizeof(EntryPoint_Register), EntryPoint_Register_Type, _EntryPoint_Register_Delete, _EntryPoint_Register_Print, NULL );
}

void EntryPoint_Register_Init( void* entryPoint_Register ) {
	EntryPoint_Register* self = (EntryPoint_Register*)entryPoint_Register;
	
	/* General info */
	self->type = EntryPoint_Register_Type;
	self->_sizeOfSelf = sizeof(EntryPoint_Register);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _EntryPoint_Register_Delete;
	self->_print = _EntryPoint_Register_Print;
	self->_copy = NULL;
	_Stg_Class_Init( (Stg_Class*)self );
	
	/* EntryPoint_Register info */
	_EntryPoint_Register_Init( self );
}

EntryPoint_Register* _EntryPoint_Register_New(
		SizeT				_sizeOfSelf,
		Type				type,
		Stg_Class_DeleteFunction*		_delete,
		Stg_Class_PrintFunction*		_print,
		Stg_Class_CopyFunction*		_copy )
{
	EntryPoint_Register* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(EntryPoint_Register) );
	self = (EntryPoint_Register*)_Stg_Class_New( _sizeOfSelf, type, _delete, _print, _copy );
	
	/* General info */
	
	/* Virtual info */
	
	_EntryPoint_Register_Init( self );
	
	return self;
}

void _EntryPoint_Register_Init( void* entryPoint_Register ) {
	EntryPoint_Register* self = (EntryPoint_Register*)entryPoint_Register;
	
	/* General and Virtual info should already be set */
	
	/* EntryPoint_Register info */
	self->count = 0;
	self->_size = 8;
	self->_delta = 8;
	self->entryPoint = Memory_Alloc_Array( EntryPoint*, self->_size, "EntryPoint_Register->entryPoint" );
	memset( self->entryPoint, 0, sizeof(EntryPoint*) * self->_size );
}

void _EntryPoint_Register_Delete( void* entryPoint_Register ) {
	EntryPoint_Register* self = (EntryPoint_Register*)entryPoint_Register;
	
	/* Assumes ownerships of the element types */
	if( self->entryPoint ) {
		EntryPoint_Index entryPoint_I;
		
		for( entryPoint_I = 0; entryPoint_I < self->count; entryPoint_I++ ) {
			Stg_Class_Delete( self->entryPoint[entryPoint_I] );
		}
		
		Memory_Free( self->entryPoint );
	}
	
	/* Stg_Class_Delete parent */
	_Stg_Class_Delete( self );
}

void _EntryPoint_Register_Print( void* entryPoint_Register, Stream* stream ) {
	EntryPoint_Register* self = (EntryPoint_Register*)entryPoint_Register;
	#ifdef DEBUG
		EntryPoint_Index entryPoint_I;
	#endif
	
	/* General info */
	Journal_Printf( (void*) stream, "EntryPoint_Register (ptr): %p\n", self );
	
	/* Print parent */
	_Stg_Class_Print( self, stream );
	
	/* Virtual info */
	
	/* EntryPoint_Register info */
	Journal_Printf( (void*) stream, "\tcount: %u\n", self->count );
	Journal_Printf( (void*) stream, "\t_size: %lu\n", self->_size );
	Journal_Printf( (void*) stream, "\t_delta: %lu\n", self->_delta );
	
	Journal_Printf( (void*) stream, "\tentryPoint (ptr): %p\n", self->entryPoint );
	Journal_Printf( (void*) stream, "\tentryPoint[0-%u]:\n", self->count );
	#ifdef DEBUG
		for( entryPoint_I = 0; entryPoint_I < self->count; entryPoint_I++ ) {
			Journal_Printf( (void*) stream, "entryPoint[%u]: ", entryPoint_I );
			Print( self->entryPoint[entryPoint_I], stream);
		}
	#else
		Journal_Printf( (void*) stream, "...\n" );
	#endif
	Journal_Printf( (void*) stream, "\t]\n" );
}

EntryPoint_Index EntryPoint_Register_Add( void* entryPoint_Register, void* entryPoint ) {
	EntryPoint_Register*	self = (EntryPoint_Register*)entryPoint_Register;
	EntryPoint_Index	handle;
	
	if( self->count >= self->_size ) {
		/*EntryPoint**	newEntryPoint;*/
		
		self->_size += self->_delta;
		self->entryPoint = Memory_Realloc_Array( self->entryPoint, EntryPoint*, self->_size );
	}
	
	handle = self->count;
	self->entryPoint[handle] = (EntryPoint*) entryPoint;
	self->count++;
	
	return handle;
}

EntryPoint_Index EntryPoint_Register_GetHandle( void* entryPoint_Register, Type type ) {
	EntryPoint_Register*	self = (EntryPoint_Register*)entryPoint_Register;
	EntryPoint_Index entryPoint_I;
	
	for( entryPoint_I = 0; entryPoint_I < self->count; entryPoint_I++ ) {
		if( self->entryPoint[entryPoint_I]->name == type ) {
			return entryPoint_I;
		}
	}
	for( entryPoint_I = 0; entryPoint_I < self->count; entryPoint_I++ ) {
		if( strcmp( self->entryPoint[entryPoint_I]->name, type ) == 0 ) {
			return entryPoint_I;
		}
	}
	return (unsigned)-1;
}

EntryPoint* _EntryPoint_Register_At( void* entryPoint_Register, EntryPoint_Index handle ) {
	EntryPoint_Register*	self = (EntryPoint_Register*)entryPoint_Register;
	
	return EntryPoint_Register_At( self, handle );
}


EntryPoint_Index EntryPoint_Register_Find( void* entryPoint_Register, void* entryPoint ) {
	EntryPoint_Register*	self = (EntryPoint_Register*)entryPoint_Register;
	EntryPoint_Index	ep_I;
	
	for( ep_I = 0; ep_I < self->count; ep_I++ ) {
		if( self->entryPoint[ep_I] == (EntryPoint*)entryPoint ) {
			return ep_I;
		}
	}
	
	return (EntryPoint_Index)-1;
}

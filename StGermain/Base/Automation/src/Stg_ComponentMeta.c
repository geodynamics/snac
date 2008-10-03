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
*/
/** \file
**  Role:
**	A class which encapsulates the instance meta data.
**
** Assumptions:
**
** Comments:
**
** $Id: Stg_ComponentMeta.h 3367 2005-12-09 07:39:53Z AlanLo $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdarg.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"

#include "Automation.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

const Type Stg_ComponentMeta_Value_Type = "Stg_ComponentMeta_Value";

Stg_ComponentMeta_Value* Stg_ComponentMeta_Value_New( Name name, Type metaType, const char* description, const char* defaultValue ) {
	Stg_ComponentMeta_Value* self;
	self = (Stg_ComponentMeta_Value*)_Stg_Object_New(
			sizeof( Stg_ComponentMeta_Value ),
			Stg_ComponentMeta_Value_Type,
			_Stg_ComponentMeta_Value_Delete,
			NULL,
			_Stg_ComponentMeta_Value_Copy,//NULL,
			name,
			NON_GLOBAL );
	self->metaType = metaType;
	self->description = description;
	self->defaultValue = defaultValue;
	self->haveValue = False;

	return self;
}
void _Stg_ComponentMeta_Value_Delete( void* instance ) {

	Stg_ComponentMeta_Value* self = (Stg_ComponentMeta_Value*) instance;

	/* Delete the specific ComponentMeta_Value members that were given memory. */
	if (self->description) Memory_Free((char*) self->description);
	if (self->defaultValue) Memory_Free((char*) self->defaultValue);

	_Stg_Object_Delete( instance );
}

void* _Stg_ComponentMeta_Value_Copy( void* instance, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap ) {

	Stg_ComponentMeta_Value*  self = (Stg_ComponentMeta_Value*)instance;

	Stg_ComponentMeta_Value*  newComponentMeta_Value;

	/* Copy the generic Stg_Object members */
        newComponentMeta_Value = _Stg_Object_Copy( instance, dest, deep, nameExt, ptrMap );

	/* Copy the Stg_ComponentMeta_Value specific members  that don't require memory*/
	newComponentMeta_Value->metaType = self->metaType;
       	newComponentMeta_Value->haveValue = self->haveValue;

	if(deep)
	{
		if( (newComponentMeta_Value->description = PtrMap_Find( ptrMap, (char*) self->description )) == NULL && self->description ) {
			newComponentMeta_Value->description = (const char*) Memory_Alloc_Array( char, strlen(self->description) + 1, "ComponentMeta_V->description" );
			strcpy((char*) newComponentMeta_Value->description, self->description);
			PtrMap_Append( ptrMap, (char*)newComponentMeta_Value->description, (char*)self->description);
		}
		if( (newComponentMeta_Value->defaultValue = PtrMap_Find( ptrMap, (char*)self->defaultValue )) == NULL && self->defaultValue ) {
			newComponentMeta_Value->defaultValue = (const char*) Memory_Alloc_Array( char, strlen(self->defaultValue) + 1, "ComponentMeta_V->defaultValue" );
			strcpy((char*) newComponentMeta_Value->defaultValue, self->defaultValue);
			PtrMap_Append( ptrMap, (char*) newComponentMeta_Value->defaultValue, (char*) self->defaultValue);
		}
	}
	else
	{
		newComponentMeta_Value->description = self->description;
		newComponentMeta_Value->defaultValue = self->defaultValue;
	}

	return newComponentMeta_Value;
}


const Type Stg_ComponentMeta_Type = "Stg_ComponentMeta";

Stg_ComponentMeta* Stg_ComponentMeta_New( Name name, Type metaType ) {
	Stg_ComponentMeta* self;

	self = (Stg_ComponentMeta*)_Stg_Object_New(
			sizeof( Stg_ComponentMeta ),
			Stg_ComponentMeta_Type,
			_Stg_ComponentMeta_Delete,
			_Stg_ComponentMeta_Print,
			_Stg_ComponentMeta_Copy,
			name,
			NON_GLOBAL );

	self->ioHandler = NULL;
	self->dict = NULL;
	self->metaType = metaType;
	self->isValid = True;
	self->project = NULL;
	self->location = NULL;
	self->web = NULL;
	self->copyright = NULL;
	self->license = NULL;
	self->parent = NULL;
	self->description = NULL;
	
	self->allDependencies = Stg_ObjectList_New();
	self->essentialDependencies = Stg_ObjectList_New();
	self->optionalDependencies = Stg_ObjectList_New();
	self->missingDependencies = Stg_ObjectList_New();
	self->unexpectedDependencies = Stg_ObjectList_New();

	self->allParams = Stg_ObjectList_New();
	
	return self;
}
void _Stg_ComponentMeta_Delete( void* instance ) {
	Stg_ComponentMeta* self = (Stg_ComponentMeta*)instance;

	if ( self->ioHandler ) {
		Stg_Class_Delete( self->ioHandler );
	}
	if ( self->dict ) {
		Stg_Class_Delete( self->dict );
	}
	Stg_Class_Delete( self->allDependencies );
	Stg_Class_Delete( self->essentialDependencies );
	Stg_Class_Delete( self->optionalDependencies );
	Stg_Class_Delete( self->missingDependencies );
	Stg_Class_Delete( self->unexpectedDependencies );
	Stg_Class_Delete( self->allParams );
}
	
void _Stg_ComponentMeta_Print( void* instance, Stream* stream ) {
	Stg_ComponentMeta* self = (Stg_ComponentMeta*)instance;
	int i;

	Journal_Printf( stream, "Name: %s\n", self->name );
	Journal_Printf( stream, "Type: %s\n", self->metaType );
	Journal_Printf( stream, "Project: %s\n", self->project );
	Journal_Printf( stream, "Location: %s\n", self->location );
	Journal_Printf( stream, "Project Web: %s\n", self->web );
	Journal_Printf( stream, "Copyright: %s\n", self->copyright );
	Journal_Printf( stream, "License: %s\n", self->license );
	Journal_Printf( stream, "Parent: %s\n", self->parent );
	Journal_Printf( stream, "Description: %s\n", self->description );
	Journal_Printf( stream, "isValid: %d\n", self->isValid );

	Journal_Printf( stream, "All dependencies\n" );
	Stream_Indent( stream );
	for ( i = 0; i < self->allDependencies->count; ++i ) {
		Journal_Printf( stream, "%s\n", ((Stg_ComponentMeta_Value*)Stg_ObjectList_At( self->allDependencies, i ))->name );
	}
	Stream_UnIndent( stream );
	
	Journal_Printf( stream, "Essential dependencies\n" );
	Stream_Indent( stream );
	for ( i = 0; i < self->essentialDependencies->count; ++i ) {
		Journal_Printf( stream, "%s\n", ((Stg_ComponentMeta_Value*)Stg_ObjectList_At( self->essentialDependencies, i ))->name );
	}
	Stream_UnIndent( stream );
	
	Journal_Printf( stream, "Optional dependencies\n" );
	Stream_Indent( stream );
	for ( i = 0; i < self->optionalDependencies->count; ++i ) {
		Journal_Printf( stream, "%s\n", ((Stg_ComponentMeta_Value*)Stg_ObjectList_At( self->optionalDependencies, i ))->name );
	}
	Stream_UnIndent( stream );
	
	Journal_Printf( stream, "Missing dependencies\n" );
	Stream_Indent( stream );
	for ( i = 0; i < self->missingDependencies->count; ++i ) {
		Journal_Printf( stream, "%s\n", ((Stg_ComponentMeta_Value*)Stg_ObjectList_At( self->missingDependencies, i ))->name );
	}
	Stream_UnIndent( stream );
	
	Journal_Printf( stream, "Unexpected dependencies\n" );
	Stream_Indent( stream );
	for ( i = 0; i < self->unexpectedDependencies->count; ++i ) {
		Journal_Printf( stream, "%s\n", ((Stg_ComponentMeta_Value*)Stg_ObjectList_At( self->unexpectedDependencies, i ))->name );
	}
	Stream_UnIndent( stream );
	
	Journal_Printf( stream, "All params\n" );
	Stream_Indent( stream );
	for ( i = 0; i < self->allParams->count; ++i ) {
		Journal_Printf( stream, "%s\n", ((Stg_ComponentMeta_Value*)Stg_ObjectList_At( self->allParams, i ))->name );
	}
	Stream_UnIndent( stream );
	
}

void* _Stg_ComponentMeta_Copy( void* instance, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap ) {
	return NULL;
}

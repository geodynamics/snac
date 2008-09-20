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
** $Id: NamedObject_Register.c 2428 2004-12-16 03:33:16Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


#include "types.h"
#include "forwardDecl.h"

#include "Memory.h"
#include "Class.h"
#include "Object.h"
#include "ObjectAdaptor.h"
#include "ObjectList.h"
#include "NamedObject_Register.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Stg_Class Administration members ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


const Type NamedObject_Register_Type = "NamedObject_Register";

NamedObject_Register*	NamedObject_Register_New( void ) {
	return _NamedObject_Register_New(
		sizeof(NamedObject_Register),
		NamedObject_Register_Type,
		_NamedObject_Register_Delete,
		_NamedObject_Register_Print,
		_NamedObject_Register_Copy );
}

NamedObject_Register*	_NamedObject_Register_New( 
		SizeT			_sizeOfSelf, 
		Type			type,
		Stg_Class_DeleteFunction*	_delete,
		Stg_Class_PrintFunction*	_print,
		Stg_Class_CopyFunction*	_copy ) 
{
	NamedObject_Register*	self;
	
	/* Allocate memory/General info */
	assert(_sizeOfSelf >= sizeof(NamedObject_Register));
	self = (NamedObject_Register*)_Stg_Class_New(
		_sizeOfSelf, 
		type, 
		_delete, 
		_print,
		_copy );
	
	/* Virtual info */
	
	/* Stg_Class info */
	_NamedObject_Register_Init( self );	

	return self;
}
	
void _NamedObject_Register_Init( NamedObject_Register* self ) {
	self->objects = Stg_ObjectList_New();
}

void _NamedObject_Register_Delete( void* namedObjectRegister ) {
	NamedObject_Register* self = (NamedObject_Register*)namedObjectRegister;
	
	Journal_DPrintf( Journal_Register( Debug_Type, NamedObject_Register_Type ), "In: %s()\n", __func__ );
	
	Stg_Class_Delete( self->objects ); 

	/* Stg_Class_Delete parent */
	_Stg_Class_Delete( self );
}
	
void _NamedObject_Register_Print( void* namedObjectRegister, struct Stream* stream ) {
	NamedObject_Register*	self = (NamedObject_Register*)namedObjectRegister;

	/* General info */
	Journal_Printf( stream, "NamedObject_Register (ptr): %p\n", self);
	Stream_Indent( stream );
	
	/* Use parent print */
	_Stg_Class_Print( self, stream );
	
	/* Print the list of registered objects */
	Print( self->objects, stream );	
	
	Stream_UnIndent( stream );
}


void* _NamedObject_Register_Copy( void* namedObjectRegister, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap ) {
	NamedObject_Register*	self = (NamedObject_Register*)namedObjectRegister;
	NamedObject_Register*	newNamedObjectRegister;
	
	newNamedObjectRegister = _Stg_Class_Copy( self, dest, deep, nameExt, ptrMap );
	
	Journal_Firewall( 
		deep, 
		Journal_Register( Error_Type, NamedObject_Register_Type ), 
		"Shallow copy not yet implemented\n" );
	if( deep ) {
		newNamedObjectRegister->objects = Stg_Class_Copy( self->objects, NULL, deep, nameExt, ptrMap );
	}
	
	return newNamedObjectRegister;
}


/* Public member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* Private member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

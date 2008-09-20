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
** $Id: Array.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"

#include "units.h"
#include "types.h"
#include "Array.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>


/* Class Administration members ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


/* Textual name of this class */
const Type Array_Type = "Array";


Array* Array_New( SizeT elementSize, Index count ) {
	return _Array_New( 
		sizeof(Array), 
		Array_Type, 
		_Array_Delete, 
		_Array_Print, 
		_Array_Copy, 
		elementSize,
		count );
}


void Array_Init( Array* self, SizeT elementSize, Index count ) {
	/* General info */
	self->type = Array_Type;
	self->_sizeOfSelf = sizeof(Array);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _Array_Delete;
	self->_print = _Array_Print;
	self->_copy = _Array_Copy;
	_Stg_Class_Init( (Stg_Class*)self );
	
	/* Array info */
	_Array_Init( self, elementSize, count );
}


Array* _Array_New(
		SizeT						_sizeOfSelf, 
		Type						type,
		Stg_Class_DeleteFunction*				_delete,
		Stg_Class_PrintFunction*				_print, 
		Stg_Class_CopyFunction*				_copy, 
		SizeT						elementSize, 
		Index						count )
{
	Array* self;
	
	/* Allocate memory */
	self = (Array*)_Stg_Class_New(
		_sizeOfSelf,
		type,
		_delete,
		_print, 
		_copy );
	
	/* General info */
	
	/* Virtual info */
	
	/* Array info */
	_Array_Init( self, elementSize, count );
	
	return self;
}


void _Array_Init( Array* self, SizeT elementSize, Index count ) {
	/* General and Virtual info should already be set */
	
	/* Array info */
	self->elementSize = elementSize;
	self->count = count;
	self->owner = self;
	self->ptr = Memory_Alloc_Array( char, self->elementSize * self->count, Array_Type );
	Journal_DFirewall( 
		self != NULL,  
		Journal_Register( ErrorStream_Type, Array_Type ), 
		"Allocating memory for the actualy data in Array failed!\n" );
}


void _Array_Delete( void* array ) {
	Array* self = (Array*)array;

	Journal_DFirewall( 
		self != NULL,  
		Journal_Register( ErrorStream_Type, Array_Type ), 
		"Attempting to \"%s\" delete a pointer of value NULL\n", Array_Type );
	
	/* Stg_Class_Delete the class itself */
	if( self->ptr && self->owner == self )
		Memory_Free( self->ptr );
	
	/* Stg_Class_Delete parent */
	_Stg_Class_Delete( self );
}


void _Array_Print( void* array, Stream* stream ) {
	Array* self = (Array*)array;
	
	Journal_DFirewall( 
		self != NULL,  
		Journal_Register( ErrorStream_Type, Array_Type ), 
		"Attempting to \"%s\" print a pointer of value NULL\n", Array_Type );

	/* Print parent */
	_Stg_Class_Print( self, stream );
	
	/* General info */
	Journal_Printf( stream, "Array (ptr): (%p)\n", self );
	
	/* Virtual info */
	
	/* Array info */
	Journal_Printf( stream, "\telementSize: %d\n", self->elementSize );
	Journal_Printf( stream, "\tcount: %d\n", self->count );
	Journal_Printf( stream, "\towner (ptr): %p\n", self->owner );
	Journal_Printf( stream, "\tptr (ptr): %p\n", self->ptr );
}


void* _Array_Copy( void* array, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap  ) {
	Array*			self = (Array*)array;
	Array*			newArray;
	
	Journal_DFirewall( 
		self != NULL,  
		Journal_Register( ErrorStream_Type, Array_Type ), 
		"Attempting to \"%s\" copy a pointer of value NULL\n", Array_Type );
	
	/* Copy parent */
	newArray = _Stg_Class_Copy( self, dest, deep, nameExt, ptrMap );
	
	newArray->elementSize = self->elementSize;
	newArray->count = self->count;
	
	if( deep ) {
		newArray->ptr = Memory_Alloc_Array( char, self->elementSize * self->count, Array_Type );
		Journal_DFirewall( 
			self != NULL,  
			Journal_Register( ErrorStream_Type, Array_Type ), 
			"Allocating memory for the actualy data in Array failed!\n" );
		
		newArray->owner = newArray;
		memcpy( newArray->ptr, self->ptr, newArray->elementSize * newArray->count );
	}
	else {
		newArray->owner = self;
		newArray->ptr = self->ptr;
	}
	
	return newArray;
}


/* Public member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


SizeT Array_ElementSizeFunc( void* array ) {
	Array* self = (Array*)array;

	Journal_DFirewall( 
		self != NULL,  
		Journal_Register( ErrorStream_Type, Array_Type ), 
		"Attempting to \"%s_ElementSize\" a pointer of value NULL\n", Array_Type );
	
	return Array_ElementSizeMacro( self );
}


Index Array_CountFunc( void* array ) {
	Array* self = (Array*)array;
		
	Journal_DFirewall( 
		self != NULL,  
		Journal_Register( ErrorStream_Type, Array_Type ), 
		"Attempting to \"%s_Count\" a pointer of value NULL\n", Array_Type );
	
	return Array_CountMacro( self );
}


Bool Array_IOwnFunc( void* array ) {
	Array* self = (Array*)array;
		
	Journal_DFirewall( 
		self != NULL,  
		Journal_Register( ErrorStream_Type, Array_Type ), 
		"Attempting to \"%s_IOwn\" a pointer of value NULL\n", Array_Type );
	
	return Array_IOwnMacro( self );
}


void* Array_OwnerFunc( void* array ) {
	Array* self = (Array*)array;
	
	Journal_DFirewall( 
		self != NULL,  
		Journal_Register( ErrorStream_Type, Array_Type ), 
		"Attempting to \"%s_Owner\" a pointer of value NULL\n", Array_Type );
	
	return Array_OwnerMacro( self );
}


void* Array_PtrFunc( void* array ) {
	Array* self = (Array*)array;
	
	Journal_DFirewall( 
		self != NULL,  
		Journal_Register( ErrorStream_Type, Array_Type ), 
		"Attempting to \"%s_Ptr\" a pointer of value NULL\n", Array_Type );
	
	return Array_PtrMacro( self );
}


void* Array_AtFunc( void* array, Index index ) {
	Array* self = (Array*)array;
		
	Journal_DFirewall( 
		self != NULL,  
		Journal_Register( ErrorStream_Type, Array_Type ), 
		"Attempting to \"%s_At\" a pointer of value NULL\n", Array_Type );
	
	/* Bounds check... important that it is done right here... the macro purposely calls the func on failure such that the
	   func can firewall (not-nice in macro). */
	Journal_Firewall( 
		index < Array_Count( self ),  
		Journal_Register( ErrorStream_Type, Array_Type ), 
		"Attempting to \"%s_At\" an array with an index out of bounds (index: %u. count: %u)\n", 
		Array_Type, 
		index, 
		Array_Count( self ) );
	
	return Array_AtMacro( self, index );
}


/* Private member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

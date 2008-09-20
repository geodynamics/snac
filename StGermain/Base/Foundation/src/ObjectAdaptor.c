
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
** $Id: ObjectAdaptor.c 2136 2004-09-30 02:47:13Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdarg.h>
#include "types.h"
#include "forwardDecl.h"

#include "Memory.h"
#include "Class.h"
#include "Object.h"
#include "ObjectAdaptor.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>


/* Class Administration members ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


const Type Stg_ObjectAdaptor_Type = "Stg_ObjectAdaptor";


Stg_ObjectAdaptor* Stg_ObjectAdaptor_NewOfClass( void* dataPtr, Name name, Bool iOwn, Bool isGlobal ) {
	return _Stg_ObjectAdaptor_New(
		sizeof(Stg_ObjectAdaptor), 
		Stg_ObjectAdaptor_Type, 
		_Stg_ObjectAdaptor_Delete,
		_Stg_ObjectAdaptor_Print, 
		_Stg_ObjectAdaptor_Copy,
		name,
		dataPtr,
		iOwn,
		isGlobal,
		True,
		NULL,
		NULL,
		NULL ); 
}


void Stg_ObjectAdaptor_InitOfClass( Stg_ObjectAdaptor* self, void* dataPtr, Name name, Bool iOwn, Bool isGlobal ) {
	/* General info */
	self->type = Stg_ObjectAdaptor_Type;
	self->_sizeOfSelf = sizeof(Stg_ObjectAdaptor);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _Stg_ObjectAdaptor_Delete;
	self->_print = _Stg_ObjectAdaptor_Print;
	self->_copy = _Stg_ObjectAdaptor_Copy;
	_Stg_Class_Init( (Stg_Class*)self );
	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );

	/* Stg_ObjectAdaptor info */
	_Stg_ObjectAdaptor_Init( self, dataPtr, iOwn, isGlobal, True, NULL, NULL, NULL );
}


Stg_ObjectAdaptor* Stg_ObjectAdaptor_NewOfPointer(
		void*						dataPtr, 
		Name						name, 
		Bool						iOwn,
		Bool						isGlobal,
		Stg_ObjectAdaptor_DeletePointerFunction*	ptrDelete,
		Stg_ObjectAdaptor_PrintPointerFunction*		ptrPrint,
		Stg_ObjectAdaptor_CopyPointerFunction*		ptrCopy )
{
	return _Stg_ObjectAdaptor_New(
		sizeof(Stg_ObjectAdaptor), 
		Stg_ObjectAdaptor_Type, 
		_Stg_ObjectAdaptor_Delete,
		_Stg_ObjectAdaptor_Print, 
		_Stg_ObjectAdaptor_Copy,
		name,
		dataPtr,
		iOwn,
		isGlobal,
		False,
		ptrDelete,
		ptrPrint,
		ptrCopy ); 
}


void Stg_ObjectAdaptor_InitOfPointer( 
		Stg_ObjectAdaptor* 				self, 
		void*						dataPtr, 
		Name						name, 
		Bool						iOwn,
		Bool						isGlobal,
		Stg_ObjectAdaptor_DeletePointerFunction*	ptrDelete,
		Stg_ObjectAdaptor_PrintPointerFunction*		ptrPrint,
		Stg_ObjectAdaptor_CopyPointerFunction*		ptrCopy )
{
	/* General info */
	self->type = Stg_ObjectAdaptor_Type;
	self->_sizeOfSelf = sizeof(Stg_ObjectAdaptor);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _Stg_ObjectAdaptor_Delete;
	self->_print = _Stg_ObjectAdaptor_Print;
	self->_copy = _Stg_ObjectAdaptor_Copy;
	_Stg_Class_Init( (Stg_Class*)self );
	_Stg_Object_Init( (Stg_Object*)self, name, GLOBAL );

	/* Stg_ObjectAdaptor info */
	_Stg_ObjectAdaptor_Init( self, dataPtr, iOwn, isGlobal, False, ptrDelete, ptrPrint, ptrCopy );
}


Stg_ObjectAdaptor* _Stg_ObjectAdaptor_New( 
		SizeT						_sizeOfSelf, 
		Type						type,
		Stg_Class_DeleteFunction*			_delete,
		Stg_Class_PrintFunction*			_print, 
		Stg_Class_CopyFunction*				_copy,
		Name						name,
		void*						dataPtr,
		Bool						iOwn,
		Bool						isGlobal,
		Bool						isStgClass,
		Stg_ObjectAdaptor_DeletePointerFunction*	ptrDelete,
		Stg_ObjectAdaptor_PrintPointerFunction*		ptrPrint,
		Stg_ObjectAdaptor_CopyPointerFunction*		ptrCopy )
{
	Stg_ObjectAdaptor* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(Stg_ObjectAdaptor) );
	self = (Stg_ObjectAdaptor*)_Stg_Object_New( _sizeOfSelf, type, _delete, _print, _copy, name, GLOBAL );
	
	/* General info */
	
	/* Virtual functions */
	
	/* Stg_ObjectAdaptor info */
	_Stg_ObjectAdaptor_Init( self, dataPtr, iOwn, isGlobal, isStgClass, ptrDelete, ptrPrint, ptrCopy );
	
	return self;
}


void _Stg_ObjectAdaptor_Init(
		Stg_ObjectAdaptor*				self, 
		void*						dataPtr, 
		Bool						iOwn, 
		Bool						isGlobal, 
		Bool						isStgClass,
		Stg_ObjectAdaptor_DeletePointerFunction*	ptrDelete,
		Stg_ObjectAdaptor_PrintPointerFunction*		ptrPrint,
		Stg_ObjectAdaptor_CopyPointerFunction*		ptrCopy )
{
	self->dataPtr = dataPtr;
	self->iOwn = iOwn;
	self->isGlobal = isGlobal;
	self->isStgClass = isStgClass;
	if( isStgClass ) {
		self->ptrDelete = NULL;
		self->ptrPrint = NULL;
		self->ptrCopy = NULL;
	}
	else {
		self->ptrDelete = ptrDelete;
		self->ptrPrint = ptrPrint;
		self->ptrCopy = ptrCopy;
	}
	
	Journal_Firewall( 
		(Bool)self->dataPtr, 
		Journal_Register( Error_Type, Stg_ObjectAdaptor_Type ), 
		"Constructing a Stg_ObjectAdaptor where there the data pointer is NULL is illegal.\n" );
	if( self->isGlobal ) {
		Journal_Firewall( 
			!self->iOwn, 
			Journal_Register( Error_Type, Stg_ObjectAdaptor_Type ), 
			"Constructing a Stg_ObjectAdaptor where both iOwn and isGlobal are true is illegal.\n" );
	}
}


void _Stg_ObjectAdaptor_Delete( void* objectAdaptor ) {
	Stg_ObjectAdaptor*	self = (Stg_ObjectAdaptor*)objectAdaptor;
	
	if( !self->isGlobal ) {
		if( self->isStgClass ) {
			if( self->iOwn && self->dataPtr ) {
				Stg_Class_Delete( self->dataPtr );
			}
		}
		else {
			if( self->iOwn && self->dataPtr ) {
				if( self->ptrDelete ) {
					self->ptrDelete( self->dataPtr );
				}
				Memory_Free( self->dataPtr );
			}
		}
	}
	self->dataPtr = NULL;
	
	/* Delete parent class */
	_Stg_Object_Delete( self );
}


void _Stg_ObjectAdaptor_Print( void* objectAdaptor, struct Stream* stream ) {
	Stg_ObjectAdaptor*	self = (Stg_ObjectAdaptor*)objectAdaptor;
	
	/* General info */
	Journal_Printf( stream, "Stg_ObjectAdaptor (ptr): %p\n", self );
	Stream_Indent( stream );
	
	_Stg_Object_Print( self, stream );
	
	Journal_Printf( stream, "iOwn: %s\n", self->iOwn ? "Yes" : "No" );
	Journal_Printf( stream, "isGlobal: %s\n", self->isGlobal ? "Yes" : "No" );
	Journal_Printf( stream, "isStgClass: %s\n", self->isStgClass ? "Yes" : "No" );
	
	if( self->isStgClass ) {
		Stg_Class_Print( self->dataPtr, stream );
	}
	else {
		if( self->ptrPrint ) {
			self->ptrPrint( self->dataPtr, stream );
		}
		else {
			Journal_Printf( stream, "Don't know how to print the pointer (it is not to a StGermain class)\n" );
		}
	}
	
	Stream_UnIndent( stream );
}


void* _Stg_ObjectAdaptor_Copy( void* objectAdaptor, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap ) {
	Stg_ObjectAdaptor* self = (Stg_ObjectAdaptor*)objectAdaptor;
	Stg_ObjectAdaptor* newTuple;

	newTuple = _Stg_Object_Copy( self, dest, deep, nameExt, ptrMap );
	
	newTuple->isGlobal = self->isGlobal;
	newTuple->isStgClass = self->isStgClass;
	
	Journal_Firewall( deep, Journal_Register( Error_Type, Stg_ObjectAdaptor_Type ), "Shallow copy not yet implemented\n" );
	if( deep ) {
		if( self->isGlobal ) {
			newTuple->iOwn = False;
			newTuple->dataPtr = self->dataPtr;
		}
		else {
			newTuple->iOwn = True;
			
			Journal_Firewall( 
				self->iOwn, 
				Journal_Register( Error_Type, Stg_ObjectAdaptor_Type ), 
				"Deep copy for Stg_ObjectAdaptors that do not own the non-global data is not yet implemented\n" );
			
			/* TODO: DOES NOT check if the object has already been copied (i.e. use ptrMap). Beacuse if we assume
			   ownership, we could not do so without an instance count machanism. */
			/* if not in ptr map */
			if( self->isStgClass ) {
				newTuple->dataPtr = Stg_Class_Copy( self->dataPtr, 0, deep, nameExt, ptrMap );
			}
			else {
				Journal_Firewall( 
					self->ptrCopy != NULL, 
					Journal_Register( Error_Type, Stg_ObjectAdaptor_Type ), 
					"Deep copy for pointer based Stg_ObjectAdaptors requires a copy function attached... not provided!\n" );
				newTuple->dataPtr = self->ptrCopy( self->dataPtr, 0, deep, nameExt, ptrMap );
			}
			/* else if in ptr map ... copy ptr value from there. */
		}
	}
	
	return newTuple;
}
	
/* Public member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


void* Stg_ObjectAdaptor_ObjectFunc( void* objectAdaptor ) {
	Stg_ObjectAdaptor* self = (Stg_ObjectAdaptor*)objectAdaptor;
	
	return Stg_ObjectAdaptor_ObjectMacro( self );
}


Bool Stg_ObjectAdaptor_IOwnFunc( void* objectAdaptor ) {
	Stg_ObjectAdaptor* self = (Stg_ObjectAdaptor*)objectAdaptor;
	
	return Stg_ObjectAdaptor_IOwnMacro( self );
}


Bool Stg_ObjectAdaptor_IsGlobalFunc( void* objectAdaptor ) {
	Stg_ObjectAdaptor* self = (Stg_ObjectAdaptor*)objectAdaptor;
	
	return Stg_ObjectAdaptor_IsGlobalMacro( self );
}


Bool Stg_ObjectAdaptor_IsStgClassFunc( void* objectAdaptor ) {
	Stg_ObjectAdaptor* self = (Stg_ObjectAdaptor*)objectAdaptor;
	
	return Stg_ObjectAdaptor_IsStgClassMacro( self );
}


/* Private member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

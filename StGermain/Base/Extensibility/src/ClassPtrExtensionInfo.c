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
** $Id: ClassPtrExtensionInfo.c 2915 2005-05-08 12:51:46Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdarg.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"
#include "Base/Automation/Automation.h"

#include "types.h"
#include "shortcuts.h"
#include "ExtensionManager.h"
#include "ExtensionInfo.h"
#include "ClassPtrExtensionInfo.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>


/* Class Administration members ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


/* Textual name of this class */
const Type ClassPtrExtensionInfo_Type = "ClassPtrExtensionInfo";


/** allocate and initialise a new ClassPtrExtensionInfo. */
ClassPtrExtensionInfo* ClassPtrExtensionInfo_New( const Name name, Stg_Class_CopyFunction* copyFunc, Index count ) {
	return _ClassPtrExtensionInfo_New( 
		sizeof(ClassPtrExtensionInfo), 
		ClassPtrExtensionInfo_Type, 
		_ClassPtrExtensionInfo_Delete,
		_ClassPtrExtensionInfo_Print, 
		_ClassPtrExtensionInfo_Copy, 
		_ClassPtrExtensionInfo_DataCopy,
		(Name)name,
		copyFunc,
		count );
}


void ClassPtrExtensionInfo_Init( void* arrayExtensionInfo, const Name name, Stg_Class_CopyFunction* copyFunc, Index count ) {
	ClassPtrExtensionInfo* self = (ClassPtrExtensionInfo*)arrayExtensionInfo;

	/* General info */
	self->type = ClassPtrExtensionInfo_Type;
	self->_sizeOfSelf = sizeof(ClassPtrExtensionInfo);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _ClassPtrExtensionInfo_Delete;
	self->_print = _ClassPtrExtensionInfo_Print;
	self->_copy = _ClassPtrExtensionInfo_Copy;
	self->_dataCopy = _ClassPtrExtensionInfo_DataCopy;
	
	/* ClassPtrExtensionInfo info */
	_ClassPtrExtensionInfo_Init( self, (Name)name, copyFunc, count );
}


ClassPtrExtensionInfo* _ClassPtrExtensionInfo_New( 
		SizeT 				_sizeOfSelf, 
		Type 				type, 
		Stg_Class_DeleteFunction* 	_delete,
		Stg_Class_PrintFunction*	_print,
		Stg_Class_CopyFunction*		_copy, 
		ExtensionInfo_DataCopyFunction* _dataCopy,
		Name 				name,
		Stg_Class_CopyFunction*		copyFunc,
		Index				count )
{
	ClassPtrExtensionInfo* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(ClassPtrExtensionInfo) );
	self = (ClassPtrExtensionInfo*)_ExtensionInfo_New( 
		_sizeOfSelf, 
		type, 
		_delete, 
		_print, 
		_copy, 
		_dataCopy,
		name, 
		sizeof(ClassPtrExtensionInfo_PtrClass), 
		count );
	
	/* General info */
	
	/* Virtual info */
	
	/* ClassPtrExtensionInfo info */
	_ClassPtrExtensionInfo_Init( self, name, copyFunc, count );
	
	return self;
}


void _ClassPtrExtensionInfo_Init( ClassPtrExtensionInfo* self, Name name, Stg_Class_CopyFunction* copyFunc, Index count ) {
	/* General and Virtual info should already be set */
	/* ClassPtrExtensionInfo info */
	_ExtensionInfo_Init( (ExtensionInfo*)self, name, sizeof(ClassPtrExtensionInfo_PtrClass), count );
}


void _ClassPtrExtensionInfo_Delete( void* arrayExtensionInfo ) {
	ClassPtrExtensionInfo*	self = (ClassPtrExtensionInfo*)arrayExtensionInfo;
		
	Journal_Firewall( 
		self != NULL,  
		Journal_Register( ErrorStream_Type, ExtensionInfo_Type ), 
		"Attempting to \"%s\" delete a pointer of value NULL\n", ClassPtrExtensionInfo_Type );
	
	/* Delete parent */
	_ExtensionInfo_Delete( self );
}


void _ClassPtrExtensionInfo_Print( void* arrayExtensionInfo, Stream* stream ) {
	ClassPtrExtensionInfo* self = (ClassPtrExtensionInfo*)arrayExtensionInfo;
		
	Journal_Firewall( 
		self != NULL,  
		Journal_Register( ErrorStream_Type, ExtensionInfo_Type ), 
		"Attempting to \"%s\" print a pointer of value NULL\n", ClassPtrExtensionInfo_Type );
	
	/* General info */
	Journal_Printf( (void*)stream, "ClassPtrExtensionInfo (%p):\n", self );
	_ExtensionInfo_Print( self, stream );
}


void* _ClassPtrExtensionInfo_Copy( void* extensionInfo, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap  ) {
	ClassPtrExtensionInfo*	self = (ClassPtrExtensionInfo*)extensionInfo;
	ClassPtrExtensionInfo*	newClassPtrExtensionInfo;

	/* Copy parent */
	newClassPtrExtensionInfo = _ExtensionInfo_Copy( self, dest, deep, nameExt, ptrMap );

	newClassPtrExtensionInfo->copyFunc = self->copyFunc;
	
	return newClassPtrExtensionInfo;
}

void* _ClassPtrExtensionInfo_DataCopy(
		void* extensionInfo,
		void* source,
		void* dest,
		Bool deep,
		Name nameExt,
		PtrMap* ptrMap )
{
	ClassPtrExtensionInfo* self = (ClassPtrExtensionInfo*) extensionInfo;
	ClassPtrExtensionInfo_PtrClass* srcArray = (ClassPtrExtensionInfo_PtrClass*)source;
	ClassPtrExtensionInfo_PtrClass* destArray = (ClassPtrExtensionInfo_PtrClass*)dest;
	int item_I;

	for ( item_I = 0; item_I < self->count; ++item_I ) {
		destArray[item_I].ptr = Stg_Generic_Copy( self->copyFunc, srcArray[item_I].ptr, NULL, deep, nameExt, ptrMap );
	}

	return dest;
}


/* Public member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* Private member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

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
** $Id: ExtensionInfo.c 3462 2006-02-19 06:53:24Z WalterLandry $
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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>


/* Class Administration members ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


/* Textual name of this class */
const Type ExtensionInfo_Type = "ExtensionInfo";

ExtensionInfo* _ExtensionInfo_New( 
		SizeT 				_sizeOfSelf, 
		Type 				type, 
		Stg_Class_DeleteFunction* 	_delete,
		Stg_Class_PrintFunction*	_print,
		Stg_Class_CopyFunction*		_copy,
		ExtensionInfo_DataCopyFunction*	_dataCopy,
		Name 				name, 
		SizeT 				size,
		Index				count )
{
	ExtensionInfo* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(ExtensionInfo) );
	self = (ExtensionInfo*)_Stg_Object_New( _sizeOfSelf, type, _delete, _print, _copy, name, NON_GLOBAL );
	
	/* General info */
	
	/* Virtual info */
	self->_dataCopy = _dataCopy;
	
	/* ExtensionInfo info */
	_ExtensionInfo_Init( self, name, size, count );
	
	return self;
}


void _ExtensionInfo_Init( ExtensionInfo* self, Name name, SizeT size, Index count ) {
	/* General and Virtual info should already be set */
	
	/* ExtensionInfo info */

	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );

	self->key = name;
	
	self->originalSize = size * count;
	self->size = ExtensionManager_Align( self->originalSize );
	self->isRegistered = False;
	self->itemSize = size;
	self->count = count;
	
	/* These are set via ExtensionInfo_Register (ie. after it is added to the ExtensionManager */
	self->offset = 0;
	self->extensionManager = NULL;
	self->handle = -1;

	self->data = NULL;
}


void _ExtensionInfo_Delete( void* extensionInfo ) {
	ExtensionInfo* self = (ExtensionInfo*)extensionInfo;
	
	Journal_DFirewall( 
			self != NULL,  
			Journal_Register( ErrorStream_Type, ExtensionInfo_Type ), 
			"Attempting to \"%s\" delete a pointer of value NULL\n", ExtensionInfo_Type );

	if ( self->data ) {
		Memory_Free( self->data );
	}
	
	/* Delete parent */
	_Stg_Object_Delete( self );
}


void _ExtensionInfo_Print( void* extensionInfo, Stream* stream ) {
	ExtensionInfo* self = (ExtensionInfo*)extensionInfo;
		
	Journal_DFirewall( 
			self != NULL,  
			Journal_Register( ErrorStream_Type, ExtensionInfo_Type ), 
			"Attempting to \"%s\" print a pointer of value NULL\n", ExtensionInfo_Type );
	
	/* General info */
	Journal_Printf( (void*)stream, "ExtensionInfo (%p):\n", self );
	_Stg_Object_Print( self, stream );
	Journal_Printf( (void*)stream, "\toriginalSize: %u\n", (unsigned int)self->originalSize );
	Journal_Printf( (void*)stream, "\tsize: %u\n", (unsigned int)self->size );
	Journal_Printf( (void*)stream, "\tisRegistered: %s\n", self->isRegistered ? "Yes" : "No" );
	Journal_Printf( (void*)stream, "\toffset: %u\n", (unsigned int)self->offset );
	Journal_Printf( (void*)stream, "\textensionManager: %p (ptr)\n", self->extensionManager );
	Journal_Printf( (void*)stream, "\thandle: %u\n", self->handle );
}

void* _ExtensionInfo_Copy( void* extensionInfo, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap  ) {
	ExtensionInfo*		self = (ExtensionInfo*)extensionInfo;
	ExtensionInfo*		newExtensionInfo;
	
	/* Copy parent */
	newExtensionInfo = _Stg_Object_Copy( self, dest, deep, nameExt, ptrMap );
	
	newExtensionInfo->_dataCopy = self->_dataCopy;
	newExtensionInfo->key = self->key;
	newExtensionInfo->originalSize = self->originalSize;
	newExtensionInfo->size = self->size;
	newExtensionInfo->isRegistered = self->isRegistered;
	newExtensionInfo->itemSize = self->itemSize;
	newExtensionInfo->count = self->count;
	newExtensionInfo->offset = self->offset;
	newExtensionInfo->handle = self->handle;	

	newExtensionInfo->extensionManager = Stg_Class_Copy( self->extensionManager, NULL, deep, nameExt, ptrMap );

	if ( self->data ) {
		newExtensionInfo->data = PtrMap_Find( ptrMap, self->data );
		Journal_Firewall(
			newExtensionInfo->data != NULL ,
			Journal_Register( Error_Type, __FILE__ ),
			"Copy Error: Attempting to copy ExtensionInfo before data\n" );
	}
	else {
		newExtensionInfo->data = NULL;
	}
	
	return newExtensionInfo;
}


/* Public member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


SizeT ExtensionInfo_OriginalSizeFunc( void* extensionInfo ) {
	ExtensionInfo* self = (ExtensionInfo*)extensionInfo;
	
	Journal_DFirewall( 
			self != NULL,  
			Journal_Register( ErrorStream_Type, ExtensionInfo_Type ), 
			"Attempting to \"%s_OriginalSize\" a pointer of value NULL\n", ExtensionInfo_Type );
	
	return ExtensionInfo_OriginalSizeMacro( self );
}


SizeT ExtensionInfo_SizeFunc( void* extensionInfo ) {
	ExtensionInfo* self = (ExtensionInfo*)extensionInfo;
	
	Journal_DFirewall( 
			self != NULL,  
			Journal_Register( ErrorStream_Type, ExtensionInfo_Type ), 
			"Attempting to \"%s_Size\" a pointer of value NULL\n", ExtensionInfo_Type );
	
	return ExtensionInfo_SizeMacro( self );
}

SizeT ExtensionInfo_ItemSizeFunc( void* extensionInfo ) {
	ExtensionInfo* self = (ExtensionInfo*)extensionInfo;
		
	Journal_DFirewall( 
			self != NULL,  
			Journal_Register( ErrorStream_Type, ExtensionInfo_Type ), 
			"Attempting to \"%s_ItemSize \" a pointer of value NULL\n", ExtensionInfo_Type );
	
	return ExtensionInfo_ItemSizeMacro( self );
}

SizeT ExtensionInfo_CountFunc( void* extensionInfo ) {
	ExtensionInfo* self = (ExtensionInfo*)extensionInfo;
		
	Journal_Firewall( 
			self != NULL,
			Journal_Register( ErrorStream_Type, ExtensionInfo_Type ), 
			"Attempting to \"%s_Count \" a pointer of value NULL\n", ExtensionInfo_Type );
	
	return ExtensionInfo_CountMacro( self );
}

SizeT ExtensionInfo_OffsetFunc( void* extensionInfo ) {
	ExtensionInfo* self = (ExtensionInfo*)extensionInfo;
		
	Journal_DFirewall( 
			self != NULL,  
			Journal_Register( ErrorStream_Type, ExtensionInfo_Type ), 
			"Attempting to \"%s_Offset\" a pointer of value NULL\n", ExtensionInfo_Type );
	Journal_DFirewall( 
			self->isRegistered,  
			Journal_Register( ErrorStream_Type, ExtensionInfo_Type ), 
			"Attempting to \"%s_Offset\" a non registered extension info\n", ExtensionInfo_Type );	
	
	return ExtensionInfo_OffsetMacro( self );
}

Bool ExtensionInfo_IsRegisteredFunc( void* extensionInfo ) {
	ExtensionInfo* self = (ExtensionInfo*)extensionInfo;
		
	Journal_DFirewall( 
			self != NULL,  
			Journal_Register( ErrorStream_Type, ExtensionInfo_Type ), 
			"Attempting to \"%s_IsRegistered\" a pointer of value NULL\n", ExtensionInfo_Type );
	
	return ExtensionInfo_IsRegisteredMacro( self );
}


void* ExtensionInfo_DataCopy( void* extensionInfo, void* source, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	ExtensionInfo* self = (ExtensionInfo*)extensionInfo;

	return self->_dataCopy( self, source, dest, deep, nameExt, ptrMap );
}

void ExtensionInfo_Register( 
	void*				extensionInfo,
	SizeT				offset,
	ExtensionManager*		extensionManager,
	ExtensionInfo_Index		handle,
	void*				data )
{
	ExtensionInfo* self = (ExtensionInfo*)extensionInfo;
		
	Journal_Firewall( 
			self != NULL,  
			Journal_Register( ErrorStream_Type, ExtensionInfo_Type ), 
			"Attempting to \"%s_Register\" a pointer of value NULL\n", ExtensionInfo_Type );

	self->isRegistered = True;
	self->offset = offset;
	self->extensionManager = extensionManager;
	self->handle = handle;
	self->data = data;
	
	Journal_DFirewall( 
			self->extensionManager != NULL,  
			Journal_Register( ErrorStream_Type, ExtensionInfo_Type ), 
			"Attempting to register an ExtensionInfo with an ExtensionManager that is NULL\n" );
	Journal_DFirewall( 
			self->handle != -1,  
			Journal_Register( ErrorStream_Type, ExtensionInfo_Type ), 
			"Attempting to register an ExtensionInfo with a handle that is invalid\n" );
	
	/* To consider... is there a way of checking that the handle is a sane value? SQ20050505 */
}

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
**
** Assumptions:
**
** Comments:
**
** $Id: PluginLoader.c 3192 2005-08-25 01:45:42Z AlanLo $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdarg.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"
#include "Base/Automation/Automation.h"

#include "types.h"
#include "shortcuts.h"
#include "PluginLoader.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#ifdef NOSHARED
	#define dlopen( x, y ) (NULL)
	#define dlsym( x, y ) (NULL)
	#define dlclose( x )
	#define dlerror() "blah"
	#define RTLD_LAZY 0
	#define RTLD_GLOBAL 0
#else
        #include <dlfcn.h>
#endif

#include <limits.h>


const Type PluginLoader_Type = "PluginLoader";

const char* PLUGININFO_PLUGIN_SUFFIX = "module.";

const char* PLUGININFO_REGISTER_SUFFIX = "_Register";
const char* PLUGININFO_GETMETADATA_SUFFIX = "_GetMetadata";
const char* PLUGININFO_GETNAME_SUFFIX = "_GetName";
const char* PLUGININFO_GETVERSION_SUFFIX = "_GetVersion";

const char* PLUGIN_DEPENDENCY_NAME_KEY = "plugin";
const char* PLUGIN_DEPENDENCY_VERSION_KEY = "version";
const char* PLUGIN_DEPENDENCY_URL_KEY = "url";
	
PluginLoader* PluginLoader_NewLocal( Name pluginName, Stg_ObjectList* directories ) {
	DLL_Handle			dllPtr;
	PluginLoader_Register_Func*		Register;
	PluginLoader_GetMetadata_Func*	GetMetadata;
	PluginLoader_GetName_Func*		GetName;
	PluginLoader_GetVersion_Func*		GetVersion;

	char* 				fileName = NULL;
	char*				fullPathName = NULL;
	int				fullPathLength = 0;
	int				length;

	Stream*				stream;
	Stream*				debug;
	Stream*				error;

	stream =  Journal_Register( Info_Type, "PluginLoaders" );
	debug =  Journal_Register( Debug_Type, "PluginLoaders" );
	error =  Journal_Register( Error_Type, "PluginLoaders" );
	(void)error; /* to suppress unused-but-set-variable warning. */

	assert( pluginName );

	/* Try the plugin name by itself (allows LD_LIBRARY_PATH) to take precendence */
	fileName = Memory_Alloc_Array( char, strlen(pluginName) + strlen(PLUGININFO_PLUGIN_SUFFIX) + strlen(MODULE_EXT) + 1, "filename" );
	sprintf( fileName, "%s%s%s", pluginName, PLUGININFO_PLUGIN_SUFFIX, MODULE_EXT );
	dllPtr = dlopen( fileName, RTLD_LAZY | RTLD_GLOBAL );

	/* If that fails, try prepending directories from the list of registered directories */
	if ( dllPtr == NULL ) {
		Index i;

		/* First print out a error message to debug why it didn't work */
		Journal_Printf( debug, "dlerror(): %s\n", dlerror() );
		
		for ( i = 0; i < directories->count; ++i ) {
			length = strlen(Stg_ObjectList_ObjectAt( directories, i )) + 1 + strlen(fileName) + 1;
			if ( fullPathLength < length ) {
				fullPathLength = length;
				fullPathName = Memory_Realloc_Array( fullPathName, char, fullPathLength );
			}
			PathJoin( fullPathName, 2, Stg_ObjectList_ObjectAt( directories, i ), fileName );
			Journal_Printf( debug, "Trying to find plugin: \"%s\"... ", pluginName );
			dllPtr = dlopen( fullPathName, RTLD_LAZY | RTLD_GLOBAL );
			if ( dllPtr ) {
				break;
			}
			else {
				Journal_Printf( debug, "dlerror(): %s\n", dlerror() );
			}
		}
	}
	Memory_Free( fileName );
	if ( fullPathName ) {
		Memory_Free( fullPathName );
	}

	if ( dllPtr == NULL ) {
		return NULL;
	}

	Register = (PluginLoader_Register_Func*)PluginLoader_LoadSymbol( dllPtr, pluginName, PLUGININFO_REGISTER_SUFFIX );
	if ( Register == NULL ) {
		dlclose( dllPtr );
		return NULL;
	}

	GetMetadata = (PluginLoader_GetMetadata_Func*)PluginLoader_LoadSymbol( dllPtr, pluginName, PLUGININFO_GETMETADATA_SUFFIX );
	GetName = (PluginLoader_GetName_Func*)PluginLoader_LoadSymbol( dllPtr, pluginName, PLUGININFO_GETNAME_SUFFIX );
	GetVersion = (PluginLoader_GetVersion_Func*)PluginLoader_LoadSymbol( dllPtr, pluginName, PLUGININFO_GETVERSION_SUFFIX );

	return PluginLoader_New( pluginName, dllPtr, Register, GetMetadata, GetName, GetVersion );
}	

PluginLoader* PluginLoader_New(
	Name				name,
	DLL_Handle 			dllPtr,
	PluginLoader_Register_Func*		Register,
	PluginLoader_GetMetadata_Func*	GetMetadata,
	PluginLoader_GetName_Func*		GetName,
	PluginLoader_GetVersion_Func*		GetVersion )
{
	return _PluginLoader_New( 
		sizeof(PluginLoader), 
		PluginLoader_Type, 
		_PluginLoader_Delete, 
		_PluginLoader_Print, 
		_PluginLoader_Copy,
		name,
		dllPtr,
		Register,
		GetMetadata,
		GetName,
		GetVersion );
}
	
void PluginLoader_Init( 
	void*				plugin,
	Name				name,
	DLL_Handle 			dllPtr,
	PluginLoader_Register_Func*		Register,
	PluginLoader_GetMetadata_Func*	GetMetadata,
	PluginLoader_GetName_Func*		GetName,
	PluginLoader_GetVersion_Func*		GetVersion )
{
	PluginLoader* self = (PluginLoader*)plugin;

	self->_sizeOfSelf = sizeof(PluginLoader);
	self->type = PluginLoader_Type;
	self->_deleteSelf = False;
	self->_delete = _PluginLoader_Delete;
	self->_print = _PluginLoader_Print;
	self->_copy = _PluginLoader_Copy;
	
	_PluginLoader_Init( self, name, dllPtr, Register, GetMetadata, GetName, GetVersion );
}
	
PluginLoader* _PluginLoader_New( 
	SizeT				_sizeOfSelf,
	Type				type,
	Stg_Class_DeleteFunction*	_delete,
	Stg_Class_PrintFunction*	_print,
	Stg_Class_CopyFunction*		_copy, 
	Name				name,
	DLL_Handle 			dllPtr,
	PluginLoader_Register_Func*		Register,
	PluginLoader_GetMetadata_Func*	GetMetadata,
	PluginLoader_GetName_Func*		GetName,
	PluginLoader_GetVersion_Func*		GetVersion )
{
	PluginLoader* self;

	assert( _sizeOfSelf >= sizeof(PluginLoader) );

	self = (PluginLoader*)_Stg_Object_New( _sizeOfSelf, type, _delete, _print, _copy, name, NON_GLOBAL );
	
	_PluginLoader_Init( self, name, dllPtr, Register, GetMetadata, GetName, GetVersion );

	return self;
}
	
void _PluginLoader_Init(
	PluginLoader*				self,
	Name				name, 
	DLL_Handle 			dllPtr,
	PluginLoader_Register_Func*		Register,
	PluginLoader_GetMetadata_Func*	GetMetadata,
	PluginLoader_GetName_Func*		GetName,
	PluginLoader_GetVersion_Func*		GetVersion )
{
	XML_IO_Handler*	ioHandler;
	const char*	metadata;
	
										
	_Stg_Class_Init( (Stg_Class*)self );
	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );

	self->dllPtr = dllPtr;
	self->Register = Register;
	self->GetMetadata = GetMetadata;
	self->GetName = GetName;
	self->GetVersion = GetVersion;
	
	self->_meta = Dictionary_New();

	if ( GetMetadata ) {
		metadata = GetMetadata();
		if ( metadata ) {
			ioHandler = XML_IO_Handler_New();
			IO_Handler_ReadAllFromBuffer( ioHandler, metadata, self->_meta );
			Stg_Class_Delete( ioHandler );
		}
	}
}
	
void _PluginLoader_Delete( void* plugin )
{
	PluginLoader* self = (PluginLoader*)plugin;

	dlclose( self->dllPtr );

	Stg_Class_Delete( self->_meta );

	_Stg_Object_Delete( self );
}
	
void _PluginLoader_Print( void* plugin, Stream* stream ) {
	PluginLoader* self = (PluginLoader*)plugin;

	Dictionary_Entry_Value* depList;
	Index count = 0;
	Index i;
	const char* version;

	Journal_Printf( stream, "PluginLoader: %s\n", self->name );
	Stream_Indent( stream );
	
	version = PluginLoader_GetVersion( self );
	if ( version == NULL ) {
		version = "Unknown";
	}
	Journal_Printf( stream, "Version: (version) %s\n", version );

	depList = PluginLoader_GetDependencies( self );
	if ( depList ) {
		count = Dictionary_Entry_Value_GetCount( depList );
	}
	for ( i = 0; i < count; ++i ) {
		Dictionary* dep;
		char* depName;
		char* depVersion;
		char* depUrl;
		
		dep = Dictionary_Entry_Value_AsDictionary( Dictionary_Entry_Value_GetElement( depList, i ) );
		depName = Dictionary_GetString_WithDefault( dep, (char*)PLUGIN_DEPENDENCY_NAME_KEY, "NULL" );
		depVersion = Dictionary_GetString_WithDefault( dep, (char*)PLUGIN_DEPENDENCY_VERSION_KEY, "Unknown" );
		depUrl = Dictionary_GetString_WithDefault( dep, (char*)PLUGIN_DEPENDENCY_URL_KEY, "None" );
	
		Journal_Printf( stream, "Depends on: %s, version=%s, url=%s\n", depName, depVersion, depUrl );
	}
	
	Stream_UnIndent( stream );
}

void* _PluginLoader_Copy( void* source, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	/* You better not do it */
	return NULL;
}

PluginLoader_Register_Func* PluginLoader_GetRegisterFunc( void* plugin ) {
	PluginLoader* self = (PluginLoader*)plugin;

	return self->Register;
}

Dictionary* PluginLoader_GetMetadata( void* plugin ) {
	PluginLoader* self = (PluginLoader*)plugin;
	
	return self->_meta;
}

const char* PluginLoader_GetName( void* plugin ) {
	PluginLoader* self = (PluginLoader*)plugin;

	if ( self->GetName ) {
		return self->GetName();
	}
	return self->name;
}

const char* PluginLoader_GetVersion( void* plugin ) {
	PluginLoader* self = (PluginLoader*)plugin;

	if ( self->GetVersion ) {
		return self->GetVersion();
	}
	return NULL;
}

/* other properties */
	
Dictionary_Entry_Value* PluginLoader_GetDependencies( void* plugin ) {
	PluginLoader* self = (PluginLoader*)plugin;

	return Dictionary_Get( self->_meta, "dependenices" );
}

Dictionary_Entry_Value* PluginLoader_GetValue( void* plugin, char* key ) {
	PluginLoader* self = (PluginLoader*)plugin;

	return Dictionary_Get( self->_meta, key );
}
	

void* PluginLoader_LoadSymbol( DLL_Handle dllPtr, Name pluginName, const char* suffix ) {
	char* symbolText;
	void* result;

	symbolText = Memory_Alloc_Array( char, strlen( pluginName ) + strlen( suffix ) + 2, "symbolName" );

	sprintf( symbolText, "%s%s",  pluginName, suffix );

	result = dlsym( dllPtr, symbolText );
	if( result == NULL ) {
		/* Try with a leading "_"... this is because on macx the dlcompat library can work either placing
		   this "_" for you and without and there is no easy way to know */
		sprintf( symbolText, "_%s%s", pluginName, suffix );
		result = dlsym( dllPtr, symbolText );
	}

	Memory_Free( symbolText );

	return result;
}


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
** $Id: PluginsManager.c 3851 2006-10-12 08:57:22Z SteveQuenette $
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
#include "PluginsManager.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#if NOSHARED
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

/* Textual name of this class */
const Type PluginsManager_Type = "PluginsManager";

/* For when compiling a single a single executable (no loading of plugins at runtime */
/* This is mainly for platforms which does not support dynamic libs */
void SingleRegister( void* context );


const char* Codelet_GetMetadata();


PluginsManager* PluginsManager_New( Dictionary* dictionary ) {
	return _PluginsManager_New( 
		sizeof(PluginsManager), 
		PluginsManager_Type, 
		_PluginsManager_Delete, 
		_PluginsManager_Print, 
		NULL, 
		dictionary );
}

void PluginsManager_Init( void* plugins, Dictionary* dictionary ) {
	PluginsManager* self = (PluginsManager*)plugins;
	
	/* General info */
	self->type = PluginsManager_Type;
	self->_sizeOfSelf = sizeof(PluginsManager);
	self->_deleteSelf = False;
	self->dictionary = dictionary;
	
	/* Virtual info */
	self->_delete = _PluginsManager_Delete;
	self->_print = _PluginsManager_Print;
	self->_copy = NULL;
	_Stg_Class_Init( (Stg_Class*)self );
	
	/* Plugins info */
	_PluginsManager_Init( self );
}

PluginsManager* _PluginsManager_New(
		SizeT				_sizeOfSelf,
		Type				type,
		Stg_Class_DeleteFunction*	_delete,
		Stg_Class_PrintFunction*	_print,
		Stg_Class_CopyFunction*		_copy, 
		Dictionary*			dictionary )
{
	PluginsManager* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(PluginsManager) );
	self = (PluginsManager*)_Stg_Class_New( _sizeOfSelf, type, _delete, _print, _copy );
	
	/* General info */
	self->dictionary = dictionary;
	
	/* Virtual info */
	
	_PluginsManager_Init( self );
	
	return self;
}

void _PluginsManager_Init( void* plugins ) {
	PluginsManager* self = (PluginsManager*)plugins;
	
	/* General and Virtual info should already be set */
	/* Plugins info */

	self->plugins = Stg_ObjectList_New();
	self->codelets = Stg_ObjectList_New();
	self->directories = Stg_ObjectList_New();

	if ( self->dictionary ) {
		Dictionary_Entry_Value* localLibDirList = Dictionary_Get( self->dictionary, "LD_LIBRARY_PATH" );
		if ( localLibDirList ) {
			Index i, count;
			char* dir;
			count = Dictionary_Entry_Value_GetCount( localLibDirList );
			for ( i = 0; i < count; ++i ) {
				dir = Dictionary_Entry_Value_AsString( Dictionary_Entry_Value_GetElement( localLibDirList, i ) );
				PluginsManager_AddDirectory( self, "FromDictionary", dir );
			}
		}
	}
}

void _PluginsManager_Delete( void* plugins ) {
	PluginsManager*         self = (PluginsManager*)plugins;
	LiveComponentRegister*  lcRegister = NULL;

	/* Note : We have to delete the codelets here rather than let the 
		component factory do it as they refer to static data inside the
		loaded plugin dlls, which are no longer available once we delete
		all in self->plugins. -- Main.PatrickSunter 18 May 2006 */
	if ( (lcRegister = LiveComponentRegister_GetLiveComponentRegister()) ) {
		/* sweep through codelets list and delete each from the lcRegister */
		Index codelet_I;
		
		for ( codelet_I = 0; codelet_I < self->codelets->count; ++codelet_I ) {
			LiveComponentRegister_RemoveOneComponentsEntry( lcRegister,
				((Stg_Object*)self->codelets->data[codelet_I])->name );
		}
	}
	Stg_ObjectList_DeleteAllObjects( self->codelets );
	Stg_Class_Delete( self->codelets );
	Stg_ObjectList_DeleteAllObjects( self->plugins );
	Stg_Class_Delete( self->plugins );
	Stg_ObjectList_DeleteAllObjects( self->directories );
	Stg_Class_Delete( self->directories );
	
	/* Stg_Class_Delete parent */
	_Stg_Class_Delete( self );
}

void _PluginsManager_Print( void* plugins, Stream* stream ) {
	PluginsManager* self = (PluginsManager*)plugins;
	Index i;
	
	/* General info */
	Journal_Printf( (void*) stream, "Plugins (ptr): %p\n", self );
	
	/* Print parent */
	_Stg_Class_Print( self, stream );

	if ( self->directories->count > 0 ) {
		Journal_Printf( stream, "Search Path:\n" );
		Stream_Indent( stream );
		for ( i = 0; i < self->directories->count; ++i ) {
			Journal_Printf( stream, "(path) %s\n", Stg_ObjectList_ObjectAt( self->directories, i ) );
		}
		Stream_UnIndent( stream );
	}

	if ( self->plugins->count > 0 ) {
		Journal_Printf( stream, "Loaded plugins:\n" );
		Stream_Indent( stream );
		for ( i = 0; i < self->plugins->count; ++i ) {
			PluginLoader* plugin = (PluginLoader*)Stg_ObjectList_At( self->plugins, i );
			Journal_Printf( stream, "%s\n", plugin->name );
			if ( PluginLoader_GetVersion( plugin ) ) {
				Journal_Printf( stream, "\t(version) %s\n", PluginLoader_GetVersion( plugin ) );
			}
		}
		Stream_UnIndent( stream );
	}
}


Dictionary_Entry_Value* PluginsManager_GetPluginsList( void* dict ) {
	Dictionary*			dictionary = (Dictionary*)dict;
	Dictionary_Entry_Value*		pluginsList = NULL;
	
	if (NULL == (pluginsList = Dictionary_Get( dictionary, "plugins" )) ) {
		/* Try deprecated value "extensions" second. */
		pluginsList = Dictionary_Get( dictionary, "extensions" );
	}

	return pluginsList;
}


void PluginsManager_Load( void* plugins, void* context, void* _dictionary ) {
	PluginsManager*			self = (PluginsManager*)plugins;
	Dictionary*			dictionary = (Dictionary*)_dictionary;
	unsigned int			entryCount;
	unsigned int			entry_I;
	Dictionary_Entry_Value*		pluginsVal;

	/* First add the directory list onto LD_LIBRARY_PATH so that it can potentially
	 * resolve the unknown symbols */
	char* curEnvPath;
	char* newEnvPath;
	Index newEnvPathLength = 0;
	Index dir_I;
	
#ifdef SINGLE_EXE
	SingleRegister( context );
#else
	for ( dir_I = 0; dir_I < self->directories->count; ++dir_I ) {
		newEnvPathLength += strlen( Stg_ObjectList_ObjectAt( self->directories, dir_I ) );
		/* Add one make space for the ':' inbetween the directories */
		newEnvPathLength += 1; 
	}
	curEnvPath = getenv("LD_LIBRARY_PATH");
	if ( curEnvPath ) {
		newEnvPathLength += strlen( curEnvPath );
	}

	if ( newEnvPathLength > 0 ) {
		/* Add one to make space for the Null Terminator '\0' */
		newEnvPathLength += 1;
		
		newEnvPath = Memory_Alloc_Array( char, newEnvPathLength, "LD_LIBRARY_PATH" );
		newEnvPath[0] = '\0';
		for ( dir_I = 0; dir_I < self->directories->count; ++dir_I ) {
			strcat( newEnvPath, Stg_ObjectList_ObjectAt( self->directories, dir_I ) );
			strcat( newEnvPath, ":" );
		}
		if ( curEnvPath ) {
			strcat( newEnvPath, curEnvPath );
		}
		setenv( "LD_LIBRARY_PATH", newEnvPath, 1 );
		Journal_Printf(
			Journal_Register( Debug_Type, "Plugins" ),
			"Using LD_LIBRARY_PATH=%s\n",
			newEnvPath );
		Memory_Free( newEnvPath );
	}

	pluginsVal = PluginsManager_GetPluginsList( dictionary );
	
	if( !pluginsVal ) {
		entryCount = 0;
	}
	else {
		entryCount = Dictionary_Entry_Value_GetCount( pluginsVal );
	}
	
	for( entry_I = 0; entry_I < entryCount; entry_I++ ) {
		Name		pluginName;
		pluginName = Dictionary_Entry_Value_AsString( Dictionary_Entry_Value_GetElement( pluginsVal, entry_I ) );

		if ( ! PluginsManager_LoadPlugin( self, pluginName, context ) ) {
			Journal_Firewall(
				0,
				Journal_Register( Info_Type, "Plugins" ),
				"Error. Unable to load plugin %s\n"
				"Please check that:-\n"
				"   1. Plugin %s(%smodule.so) is built\n"
				"   2. Plugin has function 'Index %s_Register( PluginsManager* pluginsManager )'\n"
				"   3. Plugin does not have undefined symbols.\n"
				"   4. $LD_LIBRARY_PATH is set\n"
				"   5. $LD_LIBRARY_PATH is set correctly\n"
				"You can get more information about what went wrong by adding the following two lines to your input file:\n"
				"	<param name=\"journal.debug\">true</param>\n"
				"	<param name=\"journal.debug.Plugins\">true</param>\n"
				"For more help visit http://csd.vpac.org/twiki/bin/view/Stgermain/FrequentlyAskedQuestions#Plugins_Won_t_Load_in_Parallel\n",
				pluginName, pluginName, pluginName, pluginName );
		}
	}
#endif /* ifdef SINGLE_EXE */
}

Bool PluginsManager_LoadPlugin( void* plugins, Name pluginName, void* _context ) {
	PluginsManager*	self = (PluginsManager*)plugins;
	PluginLoader* thePlugin;

	Dictionary_Entry_Value* dependencies;
	Index depCount = 0;
	Index dep_I;
	
	
	Stream* stream;
	Stream* debug;
	Stream* error;

	stream =  Journal_Register( Info_Type, "Plugins" );
	debug =  Journal_Register( Debug_Type, "Plugins" );
	error =  Journal_Register( Error_Type, "Plugins" );

	if ( Stg_ObjectList_Get( self->plugins, pluginName ) != NULL ) {
		Journal_Printf( debug, "Plugin %s already loaded\n", pluginName );
		return True;
	}

	/* Try local search first */
	thePlugin = PluginLoader_NewLocal( pluginName, self->directories );

	/* remote search? */

	if ( thePlugin == NULL ) {
		return False;
	}

	/* version check? */
	
	dependencies = PluginLoader_GetDependencies( thePlugin );
	if ( dependencies ) {
		depCount = Dictionary_Entry_Value_GetCount( dependencies );
	}

	for ( dep_I = 0; dep_I < depCount; ++dep_I ) {
		Dictionary* dep;
		Dictionary_Entry_Value* depName;
		Dictionary_Entry_Value* depVersion;
		Dictionary_Entry_Value* depUrl;
		char* name;
		char* version;
		char* url;

		dep = Dictionary_Entry_Value_AsDictionary( Dictionary_Entry_Value_GetElement( dependencies, dep_I ) );
		depName = Dictionary_Get( dep, (char*)PLUGIN_DEPENDENCY_NAME_KEY );
		depVersion = Dictionary_Get( dep, (char*)PLUGIN_DEPENDENCY_VERSION_KEY );
		depUrl = Dictionary_Get( dep, (char*)PLUGIN_DEPENDENCY_URL_KEY );

		if ( depName == NULL ) {
			Journal_Printf( debug, "No plugin name found in dependency entry\n" );
		}
		else {
			name = Dictionary_Entry_Value_AsString( depName );
			version = (depVersion == NULL) ? NULL : Dictionary_Entry_Value_AsString( depVersion );
			url = (depUrl == NULL) ? NULL : Dictionary_Entry_Value_AsString( depUrl );

			if ( ! PluginsManager_LoadPlugin( self, name, _context ) ) {
				Journal_Printf( stream, "Dependency %s failed to load\n", name );
				Stg_Class_Delete( thePlugin );
				return False;
			}
		}
	}
	
	thePlugin->Register( self );
	Stg_ObjectList_Append( self->plugins, thePlugin );

	return True;
}

void PluginsManager_AddDirectory( void* plugins, Name name, char* directory ) {
	PluginsManager*			self = (PluginsManager*)plugins;
	Bool				found;
	Index				i;
	
	/* Check that the directory isn't already added */
	found = False;
	for( i =  0; i < self->directories->count; i++ ){
		if( strcmp( directory, Stg_ObjectList_ObjectAt( self->directories, i ) ) == 0 ) {
			found = True;
		}
	}
	
	if( !found ) {
		Stg_ObjectList_PointerAppend( self->directories, StG_Strdup( directory ), name, 0, 0, 0 ); 
	}
}

Index PluginsManager_Submit( 
		void* plugins, 
		Name codeletName, 
		Name version,
		Stg_Component_DefaultConstructorFunction* defaultNew ) 
{
	PluginsManager* self = (PluginsManager*)plugins;

	void* codeletInstance = NULL;
	Index result;

	Stg_ComponentRegister_AddFunc( 
		Stg_ComponentRegister_Get_ComponentRegister(),
		codeletName,
		version,
		defaultNew,
		Codelet_GetMetadata );

	codeletInstance = defaultNew( codeletName );
	result = Stg_ObjectList_Append( self->codelets, codeletInstance );
	if ( LiveComponentRegister_GetLiveComponentRegister() ) {
		LiveComponentRegister_Add( 
				LiveComponentRegister_GetLiveComponentRegister(),
				codeletInstance );
	}
	
	return result;
}

void PluginsManager_ConstructPlugins( void* plugins, Stg_ComponentFactory* cf, void* data ) {
	PluginsManager* self = (PluginsManager*)plugins;
	int i;

	for ( i = 0; i < self->codelets->count; ++i ) {
		Stg_Component_Construct( self->codelets->data[i], cf, data, False );// True );
	}
}


Bool PluginsManager_ConstructPlugin( void* plugins, Name pluginName, Stg_ComponentFactory* cf, void* data ) {
	PluginsManager* self = (PluginsManager*)plugins;

	//Get the codelet for the plugin we're wanting to construct.
	void* codelet = Stg_ObjectList_Get(self->codelets, pluginName);
				
	if(codelet == NULL)
		return False;
	else
		Stg_Component_Construct( codelet, cf, data, False );

	return True;
}

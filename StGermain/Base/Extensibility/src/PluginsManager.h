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
**	Handles the loading of "Plugin" modules, which can extend the functionality or data structures of 
**	a main StGermain program.
**
** Assumptions:
**
** Comments:
**
** $Id: PluginsManager.h 4014 2007-02-23 02:15:16Z KathleenHumble $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Base_Extensibility_PluginsManager_h__
#define __Base_Extensibility_PluginsManager_h__
	

	/* Textual name of this class */
	extern const Type PluginsManager_Type;
	
	/* Plugins info */
	#define __PluginsManager \
		/* General info */ \
		__Stg_Class \
		\
		/* Virtual info */ \
		\
		/* Plugins info */ \
		Dictionary*	dictionary; \
		Stg_ObjectList* plugins; \
		Stg_ObjectList* codelets; 
		
	struct PluginsManager { __PluginsManager };
	
    /** Define a global list of plugin directories*/
     extern Stg_ObjectList*  pluginDirectories;	

	/* Create a new Plugins */
	PluginsManager* PluginsManager_New( Dictionary* dictionary );
	
	/* Initialise a Plugins */
	void PluginsManager_Init( void* plugins, Dictionary* dictionary );
	
	/* Creation implementation / Virtual constructor */
	PluginsManager* _PluginsManager_New( 
		SizeT				_sizeOfSelf,
		Type				type,
		Stg_Class_DeleteFunction*	_delete,
		Stg_Class_PrintFunction*	_print,
		Stg_Class_CopyFunction*		_copy, 
		Dictionary*			dictionary );
	
	/* Initialisation implementation */
	void _PluginsManager_Init( void* plugins );
	
	/* Stg_Class_Delete implementation */
	void _PluginsManager_Delete( void* plugins );
	
	/* Print implementation */
	void _PluginsManager_Print( void* plugins, Stream* stream );
	
	/** Get the plugins list from the dictionary */
	Dictionary_Entry_Value* PluginsManager_GetPluginsList( void* dict );
	
	/** Load the specified plugins from the dictionary */ 
	void PluginsManager_Load( void* plugins, void* _context, void* _dictionary );

	/** Load a plugin */
	Bool PluginsManager_LoadPlugin( void* plugins, Name pluginName, void* _context );
	
	/** Add a directory to the list of prefixes it will try if the "dlopen" fails on a plugin. It won't add the same dir
	    twice. */
	void PluginsManager_AddDirectory( Name name, char* directory );
	
	Index PluginsManager_Submit( 
			void* plugins, 
			Name codeletName, 
			Name version,
			Stg_Component_DefaultConstructorFunction* defaultNew );

	void PluginsManager_ConstructPlugins( void* plugins, Stg_ComponentFactory* cf, void* data );

	Bool PluginsManager_ConstructPlugin( void* plugins, Name pluginName, Stg_ComponentFactory* cf, void* data );

#endif /* __Base_Extensibility_PluginsManager_h__ */

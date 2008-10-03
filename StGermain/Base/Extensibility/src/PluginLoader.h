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
** $Id: PluginLoader.h 3192 2005-08-25 01:45:42Z AlanLo $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Base_Extensibility_PluginLoader_h__
#define __Base_Extensibility_PluginLoader_h__
	
	/** The prototype for the Register function in a plugin */
	typedef Index (PluginLoader_Register_Func) ( PluginsManager* pluginsManager );
	/** The prototype for the metadata function in a plugin */
	typedef const char* (PluginLoader_GetMetadata_Func) ( );
	/** The prototype for the name function in a plugin */
	typedef const char* (PluginLoader_GetName_Func) ( );
	/** The prototype for the version function in a plugin */
	typedef const char* (PluginLoader_GetVersion_Func) ( );

	/* Textual name of this class */
	extern const Type PluginLoader_Type;

	extern const char* PLUGIN_DEPENDENCY_NAME_KEY;
	extern const char* PLUGIN_DEPENDENCY_VERSION_KEY;
	extern const char* PLUGIN_DEPENDENCY_URL_KEY;
	
	/* PluginLoaders info */
	#define __PluginLoader \
		/* General info */ \
		__Stg_Object \
		\
		DLL_Handle			dllPtr; \
		PluginLoader_Register_Func*	Register; \
		PluginLoader_GetMetadata_Func*	GetMetadata; \
		PluginLoader_GetName_Func*	GetName; \
		PluginLoader_GetVersion_Func*	GetVersion; \
		Dictionary*			_meta;
		
	struct PluginLoader { __PluginLoader };

	PluginLoader* PluginLoader_NewLocal( Name pluginName, Stg_ObjectList* directories );

	/* Create a new PluginLoaders */
	PluginLoader* PluginLoader_New(
		Name				name,
		DLL_Handle 			dllPtr,
		PluginLoader_Register_Func*	Register,
		PluginLoader_GetMetadata_Func*	GetMetadata,
		PluginLoader_GetName_Func*	GetName,
		PluginLoader_GetVersion_Func*	GetVersion );
	
	/* Initialise a PluginLoaders */
	void PluginLoader_Init( 
		void*				plugin,
		Name				name,
		DLL_Handle 			dllPtr,
		PluginLoader_Register_Func*	Register,
		PluginLoader_GetMetadata_Func*	GetMetadata,
		PluginLoader_GetName_Func*	GetName,
		PluginLoader_GetVersion_Func*	GetVersion );
	
	/* Creation implementation / Virtual constructor */
	PluginLoader* _PluginLoader_New( 
		SizeT				_sizeOfSelf,
		Type				type,
		Stg_Class_DeleteFunction*	_delete,
		Stg_Class_PrintFunction*	_print,
		Stg_Class_CopyFunction*		_copy, 
		Name				name,
		DLL_Handle			dllPtr,
		PluginLoader_Register_Func*	Register,
		PluginLoader_GetMetadata_Func*	GetMetadata,
		PluginLoader_GetName_Func*	GetName,
		PluginLoader_GetVersion_Func*	GetVersion );
	
	/* Initialisation implementation */
	void _PluginLoader_Init(
		PluginLoader*			self,
		Name				name,
		DLL_Handle 			dllPtr,
		PluginLoader_Register_Func*	Register,
		PluginLoader_GetMetadata_Func*	GetMetadata,
		PluginLoader_GetName_Func*	GetName,
		PluginLoader_GetVersion_Func*	GetVersion );
	
	
	/* Stg_Class_Delete implementation */
	void _PluginLoader_Delete( void* plugin );
	
	/* Print implementation */
	void _PluginLoader_Print( void* plugin, Stream* stream );

	void* _PluginLoader_Copy( void* source, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );

	PluginLoader_Register_Func* PluginLoader_GetRegisterFunc( void* plugin );

	Dictionary* PluginLoader_GetMetadata( void* plugin );

	const char* PluginLoader_GetName( void* plugin );

	const char* PluginLoader_GetVersion( void* plugin );

	/* other properties */
	
	Dictionary_Entry_Value* PluginLoader_GetDependencies( void* plugin );

	Dictionary_Entry_Value* PluginLoader_GetValue( void* plugin, char* key );
	
	void* PluginLoader_LoadSymbol( DLL_Handle dllPtr, Name pluginName, const char* suffix );
	
#endif /* __Base_Extensibility_PluginLoader_h__ */

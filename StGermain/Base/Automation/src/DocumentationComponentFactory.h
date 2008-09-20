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
**
**
** Assumptions:
**
** Comments:
**
** $Id: DocumentationComponentFactory.h 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifndef __Base_Automation_DocumentationComponentFactory_h__
#define __Base_Automation_DocumentationComponentFactory_h__

	extern const Type DocumentationComponentFactory_Type;
	
	#define __DocumentationComponentFactory \
		/* General info */ \
		__Stg_ComponentFactory \
		\
		/* Virtual info */ \
		\
		/* Class info */ \
		
	struct DocumentationComponentFactory { __DocumentationComponentFactory };
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructor
	*/
	/* Creation implementation */
	DocumentationComponentFactory* DocumentationComponentFactory_New( Stg_ObjectList* registerRegister );
	
	DocumentationComponentFactory* _DocumentationComponentFactory_New( 
		SizeT                                                    _sizeOfSelf,
		Type                                                     type,
		Stg_Class_DeleteFunction*                                _delete,
		Stg_Class_PrintFunction*                                 _print,
		Stg_Class_CopyFunction*                                  _copy,
		Stg_ComponentFactory_GetDoubleFunc*                      getDouble,
		Stg_ComponentFactory_GetIntFunc*                         getInt,
		Stg_ComponentFactory_GetUnsignedIntFunc*                 getUnsignedInt,
		Stg_ComponentFactory_GetBoolFunc*                        getBool,
		Stg_ComponentFactory_GetStringFunc*                      getString,
		Stg_ComponentFactory_GetRootDictDoubleFunc*              getRootDictDouble,
		Stg_ComponentFactory_GetRootDictIntFunc*                 getRootDictInt,
		Stg_ComponentFactory_GetRootDictUnsignedIntFunc*         getRootDictUnsignedInt,
		Stg_ComponentFactory_GetRootDictBoolFunc*                getRootDictBool,
		Stg_ComponentFactory_GetRootDictStringFunc*              getRootDictString,
		Stg_ComponentFactory_ConstructByNameFunc*                constructByName,
		Stg_ComponentFactory_ConstructByKeyFunc*                 constructByKey,
		Stg_ComponentFactory_ConstructByNameWithKeyFallbackFunc* constructByNameWithKeyFallback,
		Stg_ComponentFactory_ConstructByListFunc*                constructByList,
		Stg_ObjectList*                                          registerRegister );
	
	/*-----------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _DocumentationComponentFactory_Delete( void* DocumentationComponentFactory );
	
	void _DocumentationComponentFactory_Print( void* DocumentationComponentFactory, Stream* stream );
	
	/*-----------------------------------------------------------------------------------------------------------------
	** Functions
	*/
	Dictionary_Entry_Value* _DocumentationComponentFactory_GetDictionaryValue( 
			void* cf, Name componentName, Dictionary_Entry_Key key, Dictionary_Entry_Value* defaultVal ) ;

	/** Function to get *numerical* values from component's dictionary
	 * if the key in the sub-dictionary gives a string then it assumes 
	 * that this is a key that points to the root dictionary */
	Dictionary_Entry_Value* _DocumentationComponentFactory_GetNumericalValue( void* cf, Name componentName, Dictionary_Entry_Key key, Dictionary_Entry_Value* defaultVal ) ;

	double _DocumentationComponentFactory_GetDouble( void* cf, Name componentName, Dictionary_Entry_Key key, const double defaultVal ) ;
	int _DocumentationComponentFactory_GetInt( void* cf, Name componentName, Dictionary_Entry_Key key, const int defaultVal ) ;
	unsigned int _DocumentationComponentFactory_GetUnsignedInt( void* cf, Name componentName, Dictionary_Entry_Key key, const unsigned int defaultVal);
	Bool _DocumentationComponentFactory_GetBool( void* cf, Name componentName, Dictionary_Entry_Key key, const Bool defaultVal ) ;
	char* _DocumentationComponentFactory_GetString( void* cf, Name componentName, Dictionary_Entry_Key key, const char* const defaultVal ) ;
	
	double _DocumentationComponentFactory_GetRootDictDouble( void* cf, Dictionary_Entry_Key key, const double defaultVal ) ;
	int _DocumentationComponentFactory_GetRootDictInt( void* cf, Dictionary_Entry_Key key, const int defaultVal ) ;
	unsigned int _DocumentationComponentFactory_GetRootDictUnsignedInt( void* cf, Dictionary_Entry_Key key, const unsigned int defaultVal);
	Bool _DocumentationComponentFactory_GetRootDictBool( void* cf, Dictionary_Entry_Key key, const Bool defaultVal ) ;
	char* _DocumentationComponentFactory_GetRootDictString( void* cf, Dictionary_Entry_Key key, const char* defaultVal ) ;

	Stg_Component* _DocumentationComponentFactory_ConstructByName( 
		void*			componentFactory, 
		Name			componentName, 
		Type			type, 
		Bool			isEssential,
		void*			data ) ;
	Stg_Component* _DocumentationComponentFactory_ConstructByKey( 
		void*			componentFactory, 
		Name			parentComponentName, 
		Dictionary_Entry_Key	componentKey, 
		Type			type, 
		Bool			isEssential,
		void*			data ) ;
	Stg_Component* _DocumentationComponentFactory_ConstructByNameWithKeyFallback( 
		void*			componentFactory, 
		Name			parentComponentName, 
		Name			componentTrialName, 
		Dictionary_Entry_Key	componentTrialKey, 
		Type			type, 
		Bool			isEssential,
		void*			data ) ;
	Stg_Component** _DocumentationComponentFactory_ConstructByList( 
		void*			componentFactory, 
		Name			parentComponentName, 
		Name			listName, 
		unsigned int		maxComponents,
		Type			type,
		Bool			isEssential,
		unsigned int*		componentCount,
		void*			data );

	void DocumentationComponentFactory_DocumentType( void* cf, Type type ) ;

#endif /* __Base_Automation_DocumentationComponentFactory_h__ */

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
** $Id: DocumentationComponentFactory.c 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdarg.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"

#include "types.h"
#include "shortcuts.h"
#include "Stg_Component.h"
#include "LiveComponentRegister.h"
#include "Stg_ComponentFactory.h"
#include "DocumentationComponentFactory.h"
#include "HierarchyTable.h"
#include "Stg_ComponentRegister.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define COMPONENT_TYPE_STRING_LENGTH 30

/* Textual name of this class */
const Type DocumentationComponentFactory_Type = "DocumentationComponentFactory";

/*--------------------------------------------------------------------------------------------------------------------------
** Constructor
*/
	
DocumentationComponentFactory* DocumentationComponentFactory_New( Stg_ObjectList* registerRegister ) {
	DocumentationComponentFactory *self = NULL;

	self = _DocumentationComponentFactory_New( 
			sizeof( DocumentationComponentFactory ), 
			DocumentationComponentFactory_Type,
			_DocumentationComponentFactory_Delete,
			_DocumentationComponentFactory_Print,
			NULL,
			_DocumentationComponentFactory_GetDouble,
			_DocumentationComponentFactory_GetInt,
			_DocumentationComponentFactory_GetUnsignedInt,
			_DocumentationComponentFactory_GetBool,
			_DocumentationComponentFactory_GetString,
			_DocumentationComponentFactory_GetRootDictDouble,
			_DocumentationComponentFactory_GetRootDictInt,
			_DocumentationComponentFactory_GetRootDictUnsignedInt,
			_DocumentationComponentFactory_GetRootDictBool,
			_DocumentationComponentFactory_GetRootDictString,
			_DocumentationComponentFactory_ConstructByName,
			_DocumentationComponentFactory_ConstructByKey,
			_DocumentationComponentFactory_ConstructByNameWithKeyFallback,
			_DocumentationComponentFactory_ConstructByList,
			registerRegister );

	return self;
}

/* Creation implementation */
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
		Stg_ObjectList*                                          registerRegister )
{
	DocumentationComponentFactory *self = NULL;

	assert( _sizeOfSelf >= sizeof( DocumentationComponentFactory ) );
	self = (DocumentationComponentFactory*) _Stg_ComponentFactory_New( 
			_sizeOfSelf,
			type, 
			_delete,
			_print,
			_copy,
			getDouble,
			getInt,
			getUnsignedInt,
			getBool,
			getString,
			getRootDictDouble,
			getRootDictInt,
			getRootDictUnsignedInt,
			getRootDictBool,
			getRootDictString,
			constructByName,
			constructByKey,
			constructByNameWithKeyFallback,
			constructByList,
			NULL,
			NULL,
			registerRegister );

	return self;
}

/*--------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _DocumentationComponentFactory_Delete( void* componentFactory ) {
	DocumentationComponentFactory *self = (DocumentationComponentFactory *) componentFactory;

	_Stg_ComponentFactory_Delete( self );
}
	
void _DocumentationComponentFactory_Print( void* componentFactory, Stream* stream ) {
	DocumentationComponentFactory *self = (DocumentationComponentFactory*) componentFactory;

	assert( self );
	
	/* General info */
	Journal_Printf( (void*) stream, "DocumentationComponentFactory (ptr): %p\n", self );
	
	/* Print parent class */
	_Stg_ComponentFactory_Print( self, stream );
}

/*--------------------------------------------------------------------------------------------------------------------------
** Functions
*/
double _DocumentationComponentFactory_GetDouble( void* cf, Name componentName, Dictionary_Entry_Key key, const double Default ) {
	DocumentationComponentFactory* self = (DocumentationComponentFactory*) cf;
	Journal_Printf( self->infoStream, "Parameter - Key '%s' - Type 'double' - ", key );
	Journal_PrintValue( self->infoStream, Default );
	return Default;
}

int _DocumentationComponentFactory_GetInt( void* cf, Name componentName, Dictionary_Entry_Key key, const int Default ) {
	DocumentationComponentFactory* self = (DocumentationComponentFactory*) cf;
	Journal_Printf( self->infoStream, "Parameter - Key '%s' - Type 'int' - ", key );
	Journal_PrintValue( self->infoStream, Default );
	return Default;
}	

unsigned int _DocumentationComponentFactory_GetUnsignedInt( void* cf, Name componentName, Dictionary_Entry_Key key, const unsigned int Default ) {
	DocumentationComponentFactory* self = (DocumentationComponentFactory*) cf;
	Journal_Printf( self->infoStream, "Parameter - Key '%s' - Type 'unsigned int' - ", key );
	Journal_PrintValue( self->infoStream, Default );
	return Default;
}	

Bool _DocumentationComponentFactory_GetBool( void* cf, Name componentName, Dictionary_Entry_Key key, const Bool Default ) {
	DocumentationComponentFactory* self = (DocumentationComponentFactory*) cf;
	Journal_Printf( self->infoStream, "Parameter - Key '%s' - Type 'Bool' - ", key );
	Journal_PrintBool( self->infoStream, Default );
	return Default;
}	

char* _DocumentationComponentFactory_GetString( void* cf, Name componentName, Dictionary_Entry_Key key, const char* Default ) {
	DocumentationComponentFactory* self = (DocumentationComponentFactory*) cf;
	Journal_Printf( self->infoStream, "Parameter - Key '%s' - Type 'string' - ", key );
	Journal_PrintString( self->infoStream, Default );
	return (char*)Default;
}

double _DocumentationComponentFactory_GetRootDictDouble( void* cf, Dictionary_Entry_Key key, const double Default ) {
	DocumentationComponentFactory* self = (DocumentationComponentFactory*) cf;
	Journal_Printf( self->infoStream, "Root Dictionary Parameter - Key '%s' - Type 'double' - ", key );
	Journal_PrintValue( self->infoStream, Default );
	return Default;
}

int _DocumentationComponentFactory_GetRootDictInt( void* cf, Dictionary_Entry_Key key, const int Default ) {
	DocumentationComponentFactory* self = (DocumentationComponentFactory*) cf;
	Journal_Printf( self->infoStream, "Root Dictionary Parameter - Key '%s' - Type 'int' - ", key );
	Journal_PrintValue( self->infoStream, Default );
	return Default;
}	

unsigned int _DocumentationComponentFactory_GetRootDictUnsignedInt( void* cf, Dictionary_Entry_Key key, const unsigned int Default ) {
	DocumentationComponentFactory* self = (DocumentationComponentFactory*) cf;
	Journal_Printf( self->infoStream, "Root Dictionary Parameter - Key '%s' - Type 'unsigned int' - ", key );
	Journal_PrintValue( self->infoStream, Default );
	return Default;
}	

Bool _DocumentationComponentFactory_GetRootDictBool( void* cf, Dictionary_Entry_Key key, const Bool Default ) {
	DocumentationComponentFactory* self = (DocumentationComponentFactory*) cf;
	Journal_Printf( self->infoStream, "Root Dictionary Parameter - Key '%s' - Type 'Bool' - ", key );
	Journal_PrintBool( self->infoStream, Default );
	return Default;
}	

char* _DocumentationComponentFactory_GetRootDictString( void* cf, Dictionary_Entry_Key key, const char* const Default ) {
	DocumentationComponentFactory* self = (DocumentationComponentFactory*) cf;
	Journal_Printf( self->infoStream, "Root Dictionary Parameter - Key '%s' - Type 'string' - ", key );
	Journal_PrintString( self->infoStream, Default );
	return (char*)Default;
}


Stg_Component* _DocumentationComponentFactory_ConstructByName( 
		void*			cf, 
		Name			componentName, 
		Type			type, 
		Bool			isEssential, 
		void*			data )
{
	DocumentationComponentFactory*    self              = (DocumentationComponentFactory*) cf;

	Journal_Printf( self->infoStream, "%sEssential Component - Type '%s' - Name = '%s'\n", 
			( isEssential ? "" : "Non-" ), type, componentName );

	return NULL;
}

Stg_Component* _DocumentationComponentFactory_ConstructByKey( 
		void*			cf, 
		Name			parentComponentName, 
		Dictionary_Entry_Key	componentKey, 
		Type			type, 
		Bool			isEssential,
		void*			data ) 
{
	DocumentationComponentFactory* self = (DocumentationComponentFactory*) cf;
	
	Journal_Printf( self->infoStream, "%sEssential Component - Type '%s' - Key = '%s'\n", 
			( isEssential ? "" : "Non-" ), type, componentKey );

	return NULL;
}

Stg_Component* _DocumentationComponentFactory_ConstructByNameWithKeyFallback( 
		void*			cf, 
		Name			parentComponentName, 
		Name			componentTrialName, 
		Dictionary_Entry_Key	fallbackKey, 
		Type			type, 
		Bool			isEssential,
		void*			data ) 
{
	DocumentationComponentFactory* self = (DocumentationComponentFactory*) cf;
	
	Journal_Printf( self->infoStream, "%sEssential Component - Type '%s' - Trial Name = '%s' - Fall back key = '%s'\n", 
			( isEssential ? "" : "Non-" ), type, componentTrialName, fallbackKey );

	return NULL;
}

Stg_Component** _DocumentationComponentFactory_ConstructByList( 
		void*			componentFactory, 
		Name			parentComponentName, 
		Name			listName, 
		unsigned int		maxComponents,
		Type			type,
		Bool			isEssential,
		unsigned int*		componentCount,
		void*			data )
{
	DocumentationComponentFactory* self = (DocumentationComponentFactory*)componentFactory;

	Journal_Printf( self->infoStream, "%sEssential Components of type '%s' in list named '%s'\n", 
			( isEssential ? "" : "Non-" ), type, listName );

	return NULL;
}

void DocumentationComponentFactory_DocumentType( void* cf, Name typeName ) {
	DocumentationComponentFactory*             self              = (DocumentationComponentFactory*) cf;
	Stg_Component_DefaultConstructorFunction*  defaultConstructor;
	Stg_Component*                             component;
	Stream*                                    stream            = self->infoStream;
	Type                                       type;

	type = HierarchyTable_GetTypePtrFromName( HierarchyTable_GetHierarchyTable(), typeName );
			
	Stream_Enable( stream, False );

	defaultConstructor = Stg_ComponentRegister_AssertGet( 
			Stg_ComponentRegister_Get_ComponentRegister(), type, "0" );

	component = (Stg_Component*) defaultConstructor( "componentName" );

	Stream_Enable( stream, True );
	Journal_Printf( stream, "Class '%s'\n", type );
	Stream_Indent( stream );

	Journal_Printf( stream, "Parent - %s\n", GetParent( type ) );

	Stg_Component_Construct( component, (Stg_ComponentFactory*)self, 0 /* dummy */, True );

	Stg_Class_Delete( component );
	Stream_UnIndent( self->infoStream );
}

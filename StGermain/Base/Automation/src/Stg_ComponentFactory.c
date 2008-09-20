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
** $Id: Stg_ComponentFactory.c 4075 2007-04-24 04:30:55Z PatrickSunter $
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
#include "Stg_ComponentRegister.h"
#include "HierarchyTable.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

/* Textual name of this class */
const Type Stg_ComponentFactory_Type = "Stg_ComponentFactory";

/*--------------------------------------------------------------------------------------------------------------------------
** Constructor
*/
/* Creation implementation */
Stg_ComponentFactory* _Stg_ComponentFactory_New( 
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
		Dictionary*                                              rootDict,
		Dictionary*                                              componentDict,
		Stg_ObjectList*                                          registerRegister )
{
	Stg_ComponentFactory *self = NULL;

	assert( _sizeOfSelf >= sizeof( Stg_ComponentFactory ) );
	self = (Stg_ComponentFactory*) _Stg_Class_New( _sizeOfSelf, type, _delete, _print, _copy );

	self->getDouble = getDouble;
	self->getInt = getInt;
	self->getUnsignedInt = getUnsignedInt;
	self->getBool = getBool;
	self->getString = getString;

	self->getRootDictDouble = getRootDictDouble;
	self->getRootDictInt = getRootDictInt;
	self->getRootDictUnsignedInt = getRootDictUnsignedInt;
	self->getRootDictBool = getRootDictBool;
	self->getRootDictString = getRootDictString;
	
	self->constructByName = constructByName;
	self->constructByKey = constructByKey;
	self->constructByNameWithKeyFallback  = constructByNameWithKeyFallback;
	self->constructByList = constructByList;

	self->rootDict = rootDict;
	self->componentDict = componentDict;
	self->registerRegister = registerRegister;
	self->infoStream = Journal_Register( InfoStream_Type, self->type );
	Stream_SetPrintingRank( self->infoStream, 0 );
	Stream_SetAutoFlush( self->infoStream, True );
		
	_Stg_ComponentFactory_Init( self );
	
	return self;
}
	
Stg_ComponentFactory* Stg_ComponentFactory_New( 
		Dictionary*                                          rootDict,
		Dictionary*                                          componentDict,
		Stg_ObjectList*                                      registerRegister )
{
	Stg_ComponentFactory *self = NULL;

	assert( rootDict );
	assert( registerRegister );
	self = _Stg_ComponentFactory_New( sizeof( Stg_ComponentFactory ), 
					Stg_ComponentFactory_Type,
					_Stg_ComponentFactory_Delete,
					_Stg_ComponentFactory_Print,
					NULL,
					_Stg_ComponentFactory_GetDouble,
					_Stg_ComponentFactory_GetInt,
					_Stg_ComponentFactory_GetUnsignedInt,
					_Stg_ComponentFactory_GetBool,
					_Stg_ComponentFactory_GetString,
					_Stg_ComponentFactory_GetRootDictDouble,
					_Stg_ComponentFactory_GetRootDictInt,
					_Stg_ComponentFactory_GetRootDictUnsignedInt,
					_Stg_ComponentFactory_GetRootDictBool,
					_Stg_ComponentFactory_GetRootDictString,
					_Stg_ComponentFactory_ConstructByName,
					_Stg_ComponentFactory_ConstructByKey,
					_Stg_ComponentFactory_ConstructByNameWithKeyFallback,
					_Stg_ComponentFactory_ConstructByList,
					rootDict,
					componentDict,
					registerRegister );

	return self;
}

void _Stg_ComponentFactory_Init( Stg_ComponentFactory *self )
{
	assert( self );
	
	self->LCRegister = LiveComponentRegister_New( );
}

void Stg_ComponentFactory_Init( Stg_ComponentFactory *self )
{
	assert( self );
	_Stg_ComponentFactory_Init( self );
}
	
/*--------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _Stg_ComponentFactory_Delete( void* cf )
{
	Stg_ComponentFactory *self = (Stg_ComponentFactory *) cf;

	assert( self );

	Stg_Class_Delete( self->LCRegister );
	_Stg_Class_Delete( self );
}
	
void _Stg_ComponentFactory_Print( void* cf, Stream* stream )
{
	Stg_ComponentFactory *self = (Stg_ComponentFactory*) cf;

	assert( self );
	
	/* General info */
	Journal_Printf( (void*) stream, "Stg_ComponentFactory (ptr): %p\n", self );
	
	/* Print parent class */
	_Stg_Class_Print( self, stream );
	
	Journal_Printf( (void*) stream, "LiveComponentRegister (ptr): %p\n", self->LCRegister );
	Print( self->LCRegister, stream );
}

/*--------------------------------------------------------------------------------------------------------------------------
** Functions
*/

void Stg_ComponentFactory_SetComponentDictionary( Stg_ComponentFactory *self, Dictionary *dict )
{
	assert( self );
	assert( dict );

	self->componentDict = dict;
}

void Stg_ComponentFactory_CreateComponents( Stg_ComponentFactory *self ) {
	Dictionary_Entry*                      componentDictEntry           = NULL;
	Dictionary*                            currComponentDict            = NULL;
	Type                                   componentType                = NULL;
	Name                                   componentName                = NULL;
	Stg_Component_DefaultConstructorFunction*  componentConstructorFunction;
	Index                                  component_I;
	Stream*                                stream;
	
	assert( self );
	
	stream = self->infoStream;
	if( self->componentDict ){
		Journal_Printf( stream, "\nCreating Stg_Components from the component-list\n\n" );
		Stream_Indent( stream );
	
		for( component_I = 0; component_I < Dictionary_GetCount( self->componentDict ) ; component_I++ ){
			componentDictEntry = self->componentDict->entryPtr[ component_I ];

			currComponentDict  = Dictionary_Entry_Value_AsDictionary( componentDictEntry->value );
			componentType      = Dictionary_GetString( currComponentDict, "Type" );
			componentName      = componentDictEntry->key;

			Journal_Firewall( 
					LiveComponentRegister_Get( self->LCRegister, componentName ) == NULL,
					Journal_Register( Error_Type, self->type ),
					"Error in func %s: Trying to instantiate two components with the name of '%s'\n"
					"Each component's name must be unique.\n",
					__func__, componentName );

			/* Print Message */
			Journal_Printf( stream, "Instantiating %s as %s\n", componentType, componentName );
			
			/* Get Default Constructor for this type */
			componentConstructorFunction = Stg_ComponentRegister_AssertGet( 
					Stg_ComponentRegister_Get_ComponentRegister(), componentType, "0" );

			/* Add to register */
			LiveComponentRegister_Add( self->LCRegister,
					componentConstructorFunction( componentName ) );
		}
		Stream_UnIndent( stream );
	}
	else{
		Journal_Printf( stream, "No Stg_Component List found..!\n" );
	}
}

void Stg_ComponentFactory_ConstructComponents( Stg_ComponentFactory* self, void* data ) {
	Dictionary_Entry*                      componentDictEntry           = NULL;
	Dictionary*                            currComponentDict            = NULL;
	Type                                   componentType                = NULL;
	Name                                   componentName                = NULL;
	Stg_Component*                         component                    = NULL;
	Index                                  component_I;
	Stream*                                stream;
	
	assert( self );
	
	stream = self->infoStream;

	if( self->componentDict ){
		Journal_Printf( stream, "\nConstructing Stg_Components from the live-component register\n\n" );
		Stream_Indent( stream );
	
		for( component_I = 0; component_I < Dictionary_GetCount( self->componentDict ) ; component_I++ ){
			componentDictEntry = self->componentDict->entryPtr[ component_I ];

			currComponentDict  = Dictionary_Entry_Value_AsDictionary( componentDictEntry->value );
			componentType      = Dictionary_GetString( currComponentDict, "Type" );
			componentName      = componentDictEntry->key;

			/* Grab component from register */
			component = LiveComponentRegister_Get( self->LCRegister, componentName );
			if( component && !component->isConstructed ){
				Stg_Component_Construct( component, self, data, True );
			}
		}
		Stream_UnIndent( stream );
	}
	else{
		Journal_Printf( stream, "No Stg_ComponentList found..!\n" );
	}
}

Dictionary_Entry_Value* _Stg_ComponentFactory_GetDictionaryValue( void* cf, Name componentName, Dictionary_Entry_Key key, Dictionary_Entry_Value* defaultVal ) {
	Stg_ComponentFactory*       self              = (Stg_ComponentFactory*) cf;
	Dictionary*             componentDict     = NULL;
	Dictionary*             thisComponentDict = NULL;
	Dictionary_Entry_Value* returnVal;
	Bool                    usedDefault       = False;
	Stream*                 errorStream       = Journal_Register( Error_Type, Stg_Component_Type );
	Stream*                 stream            = self->infoStream;

	Journal_Firewall( self != NULL, errorStream, "In func %s: Stg_ComponentFactory is NULL.\n", __func__ );

	Journal_PrintfL( stream, 2, "Getting parameter '%s': ", key );

	/* Get this Stg_Component's Dictionary */
	componentDict = self->componentDict;
	Journal_Firewall( componentDict != NULL, errorStream, 
			"In func %s: Stg_Component Factory's dictionary is NULL.\n", __func__ );
	thisComponentDict = Dictionary_GetDictionary( componentDict, componentName );
	Journal_Firewall( thisComponentDict != NULL, errorStream,
			"In func %s: Can't find sub-dictionary for component '%s'.\n", __func__, componentName );

	/* Get Value from dictionary */
	returnVal = Dictionary_Get( thisComponentDict, key );
	if ( !returnVal && defaultVal ) {
		returnVal = Dictionary_GetDefault( thisComponentDict, key, defaultVal );
		usedDefault = True;
	}

	/* Print Stuff */
	if ( usedDefault ) {
		Journal_PrintfL( stream, 2, "Using default value = " );
		if ( Stream_IsPrintableLevel( stream, 2 ) ) 
			Dictionary_Entry_Value_Print( returnVal, stream );
		Journal_PrintfL( stream, 2, "\n" );

		return returnVal;
	}
	else if ( returnVal ) {
		Journal_PrintfL( stream, 2, "Found - Value = " );
		if ( Stream_IsPrintableLevel( stream, 2 ) ) 
			Dictionary_Entry_Value_Print( returnVal, stream );
		Journal_PrintfL( stream, 2, "\n" );
	}
	else 
		Journal_PrintfL( stream, 2, "Not found.\n" );

	return returnVal;
}

Dictionary_Entry_Value* _Stg_ComponentFactory_GetNumericalValue( void* cf, Name componentName, Dictionary_Entry_Key key, Dictionary_Entry_Value* defaultVal ) {
	Stg_ComponentFactory*       self              = (Stg_ComponentFactory*) cf;
	Dictionary_Entry_Value* returnVal;
	Bool                    usedDefault       = False;
	Stream*                 errorStream       = Journal_Register( Error_Type, Stg_Component_Type );
	Stream*                 stream            = self->infoStream;

	returnVal = _Stg_ComponentFactory_GetDictionaryValue( self, componentName, key, defaultVal );

	/* Check to see whether the type is a string -
	 * if it is then assume that this is a dictionary key linking to the root dictionary */
	if ( returnVal ) {
		Dictionary_Entry_Key rootDictKey = Dictionary_Entry_Value_AsString( returnVal );
		Dictionary*          rootDict    = self->rootDict;

		/* Check if the number really is a string or not */
		if ( Stg_StringIsNumeric( rootDictKey ) )
			return returnVal;
		
		Journal_PrintfL( stream, 2, "Key '%s' points to key '%s' in the root dictionary: ", key, rootDictKey );

		Journal_Firewall( rootDict != NULL, errorStream, "Root Dictionary NULL in component factory.\n" );

		/* Get Value from dictionary */
		returnVal = Dictionary_Get( rootDict, rootDictKey );
		if ( !returnVal && defaultVal ) {
			returnVal = Dictionary_GetDefault( rootDict, rootDictKey, defaultVal );
			usedDefault = True;
		}

		/* Print Stuff */
		if ( usedDefault ) {
			Journal_PrintfL( stream, 2, "Using default value = " );
			if ( Stream_IsPrintableLevel( stream, 2 ) ) 
				Dictionary_Entry_Value_Print( returnVal, stream );
			Journal_PrintfL( stream, 2, "\n" );
			return returnVal;
		}
		else if ( returnVal ) {
			Journal_PrintfL( stream, 2, "Found - Value = " );
			if ( Stream_IsPrintableLevel( stream, 2 ) ) 
				Dictionary_Entry_Value_Print( returnVal, stream );
			Journal_PrintfL( stream, 2, "\n" );
		}
		else 
			Journal_PrintfL( stream, 2, "Not found.\n" );
	}

	return returnVal;
}

double Stg_ComponentFactory_GetDouble( void* cf, Name componentName, Dictionary_Entry_Key key, double defaultVal ) {
        Stg_ComponentFactory* self = (Stg_ComponentFactory*)cf;
        return self->getDouble( cf, componentName, key, defaultVal );
}
double _Stg_ComponentFactory_GetDouble( void* cf, Name componentName, Dictionary_Entry_Key key, double defaultVal ) {
	return Dictionary_Entry_Value_AsDouble( 
			_Stg_ComponentFactory_GetNumericalValue( cf, componentName, key, 
				Dictionary_Entry_Value_FromDouble( defaultVal )));
}
int Stg_ComponentFactory_GetInt( void* cf, Name componentName, Dictionary_Entry_Key key, int defaultVal ) {
	Stg_ComponentFactory* self = (Stg_ComponentFactory*)cf;
	return self->getInt( cf, componentName, key, defaultVal );
}
int _Stg_ComponentFactory_GetInt( void* cf, Name componentName, Dictionary_Entry_Key key, int defaultVal ) {
	return Dictionary_Entry_Value_AsInt( 
			_Stg_ComponentFactory_GetNumericalValue( cf, componentName, key, 
				Dictionary_Entry_Value_FromInt( defaultVal ) ) );
}	
unsigned int Stg_ComponentFactory_GetUnsignedInt( void* cf, Name componentName, Dictionary_Entry_Key key, unsigned int defaultVal ) {
	Stg_ComponentFactory* self = (Stg_ComponentFactory*)cf;
	return self->getUnsignedInt( cf, componentName, key, defaultVal );
}
unsigned int _Stg_ComponentFactory_GetUnsignedInt( void* cf, Name componentName, Dictionary_Entry_Key key, unsigned int defaultVal ) {
	unsigned int retVal;
	retVal = Dictionary_Entry_Value_AsUnsignedInt( 
			_Stg_ComponentFactory_GetNumericalValue( cf, componentName, key, 
				Dictionary_Entry_Value_FromUnsignedInt( defaultVal )));
	// TODO : Possible memory leak if defaultVal not added to the dictionary
	return retVal;
}	
Bool Stg_ComponentFactory_GetBool( void* cf, Name componentName, Dictionary_Entry_Key key, Bool defaultVal ) {
	Stg_ComponentFactory* self = (Stg_ComponentFactory*)cf;
	return self->getBool( cf, componentName, key, defaultVal );
}
Bool _Stg_ComponentFactory_GetBool( void* cf, Name componentName, Dictionary_Entry_Key key, Bool defaultVal ) {
	return Dictionary_Entry_Value_AsBool( 
			_Stg_ComponentFactory_GetDictionaryValue( cf, componentName, key, 
				Dictionary_Entry_Value_FromBool( defaultVal ) ) );
}	
char* Stg_ComponentFactory_GetString( void* cf, Name componentName, Dictionary_Entry_Key key, const char* const defaultVal ) {
	Stg_ComponentFactory* self = (Stg_ComponentFactory*)cf;
	return self->getString( cf, componentName, key, defaultVal );
}
char* _Stg_ComponentFactory_GetString( void* cf, Name componentName, Dictionary_Entry_Key key, const char* const defaultVal ) {
	return Dictionary_Entry_Value_AsString( 
			_Stg_ComponentFactory_GetDictionaryValue( cf, componentName, key, 
				Dictionary_Entry_Value_FromString( defaultVal ) ) );
}


double Stg_ComponentFactory_GetRootDictDouble( void* cf, Dictionary_Entry_Key key, const double defaultVal ) {
	Stg_ComponentFactory*    self              = (Stg_ComponentFactory*)cf;
	return self->getRootDictDouble( self, key, defaultVal );
}
double _Stg_ComponentFactory_GetRootDictDouble( void* cf, Dictionary_Entry_Key key, const double defaultVal ) {
	Stg_ComponentFactory*    self              = (Stg_ComponentFactory*)cf;
	
	Journal_PrintfL( self->infoStream, 2, "Getting double from root dictionary with key '%s' and default value '%g'\n",
			key, defaultVal );

	assert( self->rootDict );
	return Dictionary_GetDouble_WithDefault( self->rootDict, key, defaultVal );
}
int Stg_ComponentFactory_GetRootDictInt( void* cf, Dictionary_Entry_Key key, const int defaultVal ) {
	Stg_ComponentFactory*    self              = (Stg_ComponentFactory*)cf;
	return self->getRootDictInt( self, key, defaultVal );
}
int _Stg_ComponentFactory_GetRootDictInt( void* cf, Dictionary_Entry_Key key, const int defaultVal ) {
	Stg_ComponentFactory*    self              = (Stg_ComponentFactory*)cf;
	
	Journal_PrintfL( self->infoStream, 2, "Getting int from root dictionary with key '%s' and default value '%d'\n",
			key, defaultVal );

	assert( self->rootDict );
	return Dictionary_GetInt_WithDefault( self->rootDict, key, defaultVal );
}	
unsigned int Stg_ComponentFactory_GetRootDictUnsignedInt( void* cf, Dictionary_Entry_Key key, const unsigned int defaultVal ) {
	Stg_ComponentFactory*    self              = (Stg_ComponentFactory*)cf;
	return self->getRootDictUnsignedInt( self, key, defaultVal );
}
unsigned int _Stg_ComponentFactory_GetRootDictUnsignedInt( void* cf, Dictionary_Entry_Key key, const unsigned int defaultVal ) {
	Stg_ComponentFactory*    self              = (Stg_ComponentFactory*)cf;
	
	Journal_PrintfL( self->infoStream,2, "Getting unsigned int from root dictionary with key '%s' and default value '%u'\n",
			key, defaultVal );

	assert( self->rootDict );
	return Dictionary_GetUnsignedInt_WithDefault( self->rootDict, key, defaultVal );
}	
Bool Stg_ComponentFactory_GetRootDictBool( void* cf, Dictionary_Entry_Key key, const Bool defaultVal ) {
	Stg_ComponentFactory*    self              = (Stg_ComponentFactory*)cf;
	return self->getRootDictBool( self, key, defaultVal );
}
Bool _Stg_ComponentFactory_GetRootDictBool( void* cf, Dictionary_Entry_Key key, const Bool defaultVal ) {
	Stg_ComponentFactory*    self              = (Stg_ComponentFactory*)cf;
	
	Journal_PrintfL( self->infoStream, 2, "Getting Bool from root dictionary with key '%s' and default value '%s'\n",
			key, defaultVal ? "True" : "False" );

	assert( self->rootDict );
	return Dictionary_GetBool_WithDefault( self->rootDict, key, defaultVal );
}	
char* Stg_ComponentFactory_GetRootDictString( void* cf, Dictionary_Entry_Key key, const char* const defaultVal ) {
	Stg_ComponentFactory*    self              = (Stg_ComponentFactory*)cf;
	return self->getRootDictString( self, key, defaultVal );
}
char* _Stg_ComponentFactory_GetRootDictString( void* cf, Dictionary_Entry_Key key, const char* const defaultVal ) {
	Stg_ComponentFactory*    self              = (Stg_ComponentFactory*)cf;

	Journal_PrintfL( self->infoStream, 2, "Getting string from root dictionary with key '%s' and default value '%s'\n",
			key, defaultVal );

	assert( self->rootDict );
	return Dictionary_GetString_WithDefault( self->rootDict, key, defaultVal );
}


Stg_Component* _Stg_ComponentFactory_ConstructByName( void* cf, Name componentName, Type type, Bool isEssential, void* data ) {
	Stg_ComponentFactory*	self              = (Stg_ComponentFactory*)cf;
	Stg_Component*		component;
	Stream*			stream            = self->infoStream;

	Journal_PrintfL( stream, 2, "Looking for %sessential component '%s': ", (isEssential ? "" : "non-"), componentName );

	component = LiveComponentRegister_Get( self->LCRegister, componentName );

	/* Checking */
	if (component) {
		Journal_PrintfL( stream, 2, "Found.\n" );

		if ( !component->isConstructed ) {
			Journal_Printf( stream, "%s has not been constructed yet. Constructing now.\n", componentName );
			Stream_Indent( stream );
			Stg_Component_Construct( component, self, data, True );
			Stream_UnIndent( stream );
		}

		Stg_Class_CheckType( component, type );

		/* Note: I think we should disable the line below here since this function is only providing a reference
		to an existing object, not creating a brand new one, and it was thus stuffing up the count so the
		original object never got deleted. If we are going to do reference counting we need to adapt it across
		the board so whenever you request access to memory you delete your own reference.
		-- Main.PatrickSunter 18 May 2006 */
		Memory_CountInc( component );
	}
	else {
		Journal_PrintfL( stream, 2, "Not found.\n" );

		if ( isEssential ) {
			Stream* errorStream = Journal_Register( Error_Type, self->type );

			Journal_Printf( errorStream, "In func %s: Cannot find essential component '%s'.\n", __func__, componentName );

			Journal_Printf( errorStream, "Could you have meant one of these?\n" );

			Stream_Indent( errorStream );
			LiveComponentRegister_PrintSimilar( self->LCRegister, componentName, errorStream, 5 );
			abort();
		}
	}
	
	return component;
}

Stg_Component* _Stg_ComponentFactory_ConstructByKey( 
		void*			cf, 
		Name			parentComponentName, 
		Dictionary_Entry_Key	componentKey,
		Type			type, 
		Bool 			isEssential,
		void* 			data ) 
{
	Stg_ComponentFactory*	self              = (Stg_ComponentFactory*)cf;
	Dictionary*		thisComponentDict = NULL;
	Dictionary*		componentDict     = NULL;
	Name			componentName;
	Dictionary_Entry_Value*	componentEntryVal;
	Stream*			errorStream       = Journal_Register( Error_Type, self->type );

	Journal_Firewall( self != NULL, errorStream, "In func %s: Stg_Component is NULL.\n", __func__ );

	/* Get this Stg_Component's Dictionary */
	componentDict = self->componentDict;
	Journal_Firewall( componentDict != NULL, errorStream, 
			"In func %s: Stg_Component Factory's dictionary is NULL.\n", __func__ );
	thisComponentDict = Dictionary_GetDictionary( componentDict, parentComponentName );
	Journal_Firewall( thisComponentDict != NULL, errorStream,
			"In func %s: Can't find sub-dictionary for component '%s'.\n", __func__, parentComponentName );

	/* Get Dependency's Name */
	componentEntryVal = Dictionary_Get( thisComponentDict, componentKey );
	if ( componentEntryVal == NULL ) {
		Journal_Firewall( !isEssential, errorStream,
				"Stg_Component '%s' cannot find essential component with key '%s'.\n", parentComponentName, componentKey );
		Journal_PrintfL( self->infoStream, 2, "Stg_Component '%s' cannot find non-essential component with key '%s'.\n", parentComponentName, componentKey );
		return NULL;
	}
		
	componentName = Dictionary_Entry_Value_AsString( componentEntryVal );

	return self->constructByName( self, componentName, type, isEssential, data );
}

Stg_Component* _Stg_ComponentFactory_ConstructByNameWithKeyFallback( 
		void*			cf, 
		Name 			parentComponentName, 
		Name			componentTrialName, 
		Dictionary_Entry_Key	fallbackKey, 
		Type			type, 
		Bool			isEssential,
		void*			data ) 
{
	Stg_ComponentFactory*	self              = (Stg_ComponentFactory*)cf;
	Stg_Component*		component;
	Stream*			stream            = self->infoStream;

	Journal_PrintfL( stream, 2, "First Trying to find component by name '%s': ", componentTrialName );
	component = LiveComponentRegister_Get( self->LCRegister, componentTrialName );
	
	if (component) {
		Journal_PrintfL( stream, 2, "Found.\n" );

		if ( !component->isConstructed ) {
			Journal_Printf( stream, "%s has not been constructed yet. Constructing now.\n", componentTrialName );
			Stream_Indent( stream );
			Stg_Component_Construct( component, self, data, True );
			Stream_UnIndent( stream );
		}
	}
	else {
		Journal_PrintfL( stream, 2, "Not found.\n" );
		Journal_PrintfL( stream, 2, "Fallback - Trying to find component by key '%s'.\n", fallbackKey );

		component = self->constructByKey( self, parentComponentName, fallbackKey, type, isEssential, data );
	}
		
	return component;
}

Stg_Component** _Stg_ComponentFactory_ConstructByList( 
		void*			cf, 
		Name			parentComponentName, 
		Name			listName, 
		unsigned int		maxComponents,
		Type			type,
		Bool			isEssential,
		unsigned int*		componentCount,
		void*			data )
{
	Stg_ComponentFactory*	self              = (Stg_ComponentFactory*)cf;
	Dictionary*		thisComponentDict = NULL;
	Dictionary*		componentDict     = NULL;
	Name			componentName;
	Dictionary_Entry_Value*	list;
	Stg_Component**		componentList;
	Index			component_I;
	Stream*			errorStream       = Journal_Register( Error_Type, self->type );

	Journal_Firewall( self != NULL, errorStream, "In func %s: Stg_Component is NULL.\n", __func__ );

	/* Get this Stg_Component's Dictionary */
	componentDict = self->componentDict;
	Journal_Firewall( componentDict != NULL, errorStream, 
			"In func %s: Stg_Component Factory's dictionary is NULL.\n", __func__ );
	thisComponentDict = Dictionary_GetDictionary( componentDict, parentComponentName );
	Journal_Firewall( thisComponentDict != NULL, errorStream,
			"In func %s: Can't find sub-dictionary for component '%s'.\n", __func__, parentComponentName );
	
	Journal_PrintfL( self->infoStream, 2, "Looking for list '%s': ", listName );
	if (( list = Dictionary_Get( thisComponentDict, listName ) )) {
		Journal_PrintfL( self->infoStream, 2, "Found.\n" );

		*componentCount = MIN( Dictionary_Entry_Value_GetCount( list ), maxComponents );

		componentList = Memory_Alloc_Array( Stg_Component*, *componentCount, "componentList" );
	
		Stream_Indent( self->infoStream );
		for ( component_I = 0 ; component_I < *componentCount ; component_I++ ) {
			componentName = Dictionary_Entry_Value_AsString( Dictionary_Entry_Value_GetElement( list, component_I ) );

			componentList[ component_I ] = self->constructByName( self, componentName, type, isEssential, data );
		}
		Stream_UnIndent( self->infoStream );
	}
	else {
		Journal_PrintfL( self->infoStream, 2, "Not Found.\n" );

		Journal_Firewall( !isEssential, errorStream, "Stg_Component '%s' cannot find list '%s'.\n", 
				parentComponentName, listName );

		*componentCount = 0;
		componentList   = NULL;
	}

	return componentList;
}

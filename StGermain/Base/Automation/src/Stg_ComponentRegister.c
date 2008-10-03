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
** $Id: Stg_ComponentRegister.c 2745 2005-05-1 08:12:18Z RaquibulHassan $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdarg.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"

#include "types.h"
#include "shortcuts.h"
#include "Stg_Component.h"
#include "Stg_ComponentRegister.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>


/* Textual name of this class */
const Type Stg_ComponentRegister_Type = "Stg_ComponentRegister";

const Name Version = "0";
Stg_ComponentRegister *stgComponentRegister = NULL;

Stg_ComponentRegister *_Stg_ComponentRegister_New(
	SizeT					_sizeOfSelf, 
	Type					type,
	Stg_Class_DeleteFunction*		_delete,
	Stg_Class_PrintFunction*		_print, 
	Stg_Class_CopyFunction*			_copy )
{
	Stg_ComponentRegister *self = NULL;

	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(Stg_ComponentRegister) );
	self = (Stg_ComponentRegister*)_Stg_Class_New( _sizeOfSelf, type, _delete, _print, _copy );

	return self;
}
	
Stg_ComponentRegister *Stg_ComponentRegister_New(  )
{
	Stg_ComponentRegister *self = NULL;

	if( stgComponentRegister == NULL ){
		
		self = _Stg_ComponentRegister_New( sizeof( Stg_ComponentRegister ),
			Stg_ComponentRegister_Type,
			_Stg_ComponentRegister_Delete,
			_Stg_ComponentRegister_Print,
			NULL );
		Stg_ComponentRegister_Init( self );
	}
	else{
		self = stgComponentRegister;
	}

	return self;
}
	
/* Initialisation implementation */
void _Stg_ComponentRegister_Init( Stg_ComponentRegister* self )
{
	assert( self );
	
	self->constructors = BTree_New(
							constructorElementCompareFunction,
							NULL,
							constructorElementDeleteFunction,
							constructorElementPrintFunction,
							BTREE_NO_DUPLICATES);
}
	
void Stg_ComponentRegister_Init( Stg_ComponentRegister* self )
{
	assert( self );
	_Stg_ComponentRegister_Init( self );
}
	
/* Delete boundary condition layout implementation */
void _Stg_ComponentRegister_Delete( void* componentRegister )
{
	Stg_ComponentRegister *self = NULL;

	self = (Stg_ComponentRegister*) componentRegister;
	assert( self );

	Stg_Class_Delete( self->constructors );
	_Stg_Class_Delete( self );
}
	
void _Stg_ComponentRegister_Print( void* componentRegister, Stream* stream )
{
	Stg_ComponentRegister *self = NULL;

	self = ( Stg_ComponentRegister* ) componentRegister;

	assert( self );
	
		/* General info */
	Journal_Printf( (void*) stream, "Stg_ComponentRegister (ptr): %p\n", self );
	_Stg_Class_Print( self, stream );
	Journal_Printf( stream, "Constructors:\n" );
	Stg_Class_Print( self->constructors, stream );
}
	
/* Public member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int Stg_ComponentRegister_AddFunc( 
		Stg_ComponentRegister *self,
		Name componentType,
		Name version,
		Stg_Component_DefaultConstructorFunction *func,
		Stg_Component_Type_GetMetadataFunction* metadata )
{
	Stg_ComponentRegisterElement *element = NULL;

	assert( self );
	element = Memory_Alloc( Stg_ComponentRegisterElement, "Stg_ComponentRegisterElement" );

	element->componentType = StG_Strdup( componentType );
	element->defaultConstructor = func;
	element->metadata = metadata;
	element->version = StG_Strdup( version );

	Journal_Firewall( BTree_InsertNode( self->constructors, (void*)element, sizeof( Stg_ComponentRegisterElement ) ),
			Journal_Register( Error_Type, Stg_ComponentRegister_Type ), 
			"Error in func %s: Attempting to enter duplicate constructors for type '%s' (version '%s').\n"
			"This should only be done once per type.\n",
			__func__, componentType, version );

	return 1;
}

Stg_Component_DefaultConstructorFunction* Stg_ComponentRegister_Get( 
		Stg_ComponentRegister* self,
		Name                   componentType,
		Name                   version ) 
{
	BTreeNode *node = NULL;
	Stg_ComponentRegisterElement *element = NULL;
	SearchCriteria sc;

	assert( self );
	
	sc.type = componentType;
	sc.version = version;
	
	BTree_SetCompareFunction( self->constructors, constructorElementCompareFunction1 );
	node = BTree_FindNode( self->constructors, (void*)(&sc) );
	BTree_SetCompareFunction( self->constructors, constructorElementCompareFunction );
	if( node ){
		element = (Stg_ComponentRegisterElement*) node->data;
		if( element ){
			return (void*) element->defaultConstructor;
		}
	}
	
	return NULL;
}

Stg_Component_DefaultConstructorFunction* Stg_ComponentRegister_AssertGet( 
		Stg_ComponentRegister* self,
		Name                   componentType,
		Name                   version ) 
{
	Stg_Component_DefaultConstructorFunction* componentConstructorFunction;
	
	componentConstructorFunction = Stg_ComponentRegister_Get( self, componentType, version );

	/* If we cannot find the default construct for this componentType - then abort() with a nice message */
	if ( !componentConstructorFunction ) {
		Stream* errorStream = Journal_Register( Error_Type, self->type );

		Journal_Printf( errorStream, "Cannot find default constructor function for type '%s'\n", componentType );
		Journal_Printf( errorStream, "Could you have meant one of these?\n" );

		Stream_Indent( errorStream );
		Stg_ComponentRegister_PrintSimilar( self, componentType, errorStream, 5 );
		abort();
	}	

	return componentConstructorFunction;
}

Stg_Component_Type_GetMetadataFunction* Stg_ComponentRegister_GetMetadata(
		Stg_ComponentRegister* self,
		Name                   componentType,
		Name                   version ) 
{
	BTreeNode *node = NULL;
	Stg_ComponentRegisterElement *element = NULL;
	SearchCriteria sc;

	assert( self );

	sc.type = componentType;
	sc.version = version;

	BTree_SetCompareFunction( self->constructors, constructorElementCompareFunction1 );
	node = BTree_FindNode( self->constructors, (void*)(&sc) );
	BTree_SetCompareFunction( self->constructors, constructorElementCompareFunction );

	if( node ){
		element = (Stg_ComponentRegisterElement*) node->data;
		if( element ){
			return (void*) element->metadata;
		}
	}

	assert(0);

	return NULL;
}

Stg_ComponentRegister *Stg_ComponentRegister_Get_ComponentRegister( )
{
	if( stgComponentRegister ){
		return stgComponentRegister;
	}
	else{
		return NULL;
	}
}

/* Private member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int constructorElementCompareFunction( void *data1, void *data2 )
{
	Stg_ComponentRegisterElement *d1 = NULL, *d2 = NULL;
	char key[1024];
	char key1[1024];

	d1 = (Stg_ComponentRegisterElement*)data1;
	d2 = (Stg_ComponentRegisterElement*)data2;

	if (d1 == NULL || d2 == NULL){
		return 0;	
	}

	memset( key, 0, sizeof( key ) );
	memset( key1, 0, sizeof( key1 ) );
	sprintf( key, "%s%s", d1->componentType, d1->version );
	sprintf( key1, "%s%s", d2->componentType, d2->version );
	
	if (strcmp(key, key1) > 0){
		return  1;
	}
	else if (!strcmp(key, key1)){
		return 0;
	}
	else{
		return -1;
	}	
}
	
int constructorElementCompareFunction1( void *data1, void *data2 )
{
	Stg_ComponentRegisterElement *d1 = NULL;
	SearchCriteria *d2 = NULL;
	char key[1024];
	char key1[1024];

	d1 = (Stg_ComponentRegisterElement*)data1;
	d2 = (SearchCriteria*)data2;

	if (d1 == NULL || d2 == NULL){
		return 0;	
	}
	
	memset( key, 0, sizeof( key ) );
	memset( key1, 0, sizeof( key1 ) );
	sprintf( key, "%s%s", d1->componentType, d1->version );
	sprintf( key1, "%s%s", d2->type, d2->version );
	
	if (strcmp(key, key1) > 0){
		return  1;
	}
	else if (!strcmp(key, key1)){
		return 0;
	}
	else{
		return -1;
	}	
}

void constructorElementPrintFunction( void *nodeData, Stream *printStream )
{
	Stg_ComponentRegisterElement *element;
	
	assert( nodeData );

	element = ( Stg_ComponentRegisterElement* ) nodeData;
	Journal_Printf( printStream, "Constructor Information\n");
	Journal_Printf( printStream, "\tStg_ComponentType                : %s\n", element->componentType );
	Journal_Printf( printStream, "\tStg_Component Default Constructor: %p\n", element->defaultConstructor );
	Journal_Printf( printStream, "\tVersion                      : %s\n", element->version );
}

void constructorElementDeleteFunction( void *nodeData )
{
	Stg_ComponentRegisterElement *element = NULL;

	element = (Stg_ComponentRegisterElement*) nodeData;

	if( element ){
		Memory_Free( element->componentType );
		Memory_Free( element->version );
		Memory_Free( element );
	}
}

typedef struct {
	Type     type;
	float    percentageSimilar;
} Stg_ComponentRegister_SimilarityObject;

typedef struct {
	Name                                    nameToCompare;
	Index                                   currentIndex;
	Stg_ComponentRegister_SimilarityObject* array;
} Stg_ComponentRegister_SimilarityList;

int _Stg_ComponentRegister_SimilarityCompare( const void *ptr1, const void *ptr2 ) {
	Stg_ComponentRegister_SimilarityObject* similarityPtr1 = (Stg_ComponentRegister_SimilarityObject*) ptr1;
	Stg_ComponentRegister_SimilarityObject* similarityPtr2 = (Stg_ComponentRegister_SimilarityObject*) ptr2;

	if ( similarityPtr1->percentageSimilar > similarityPtr2->percentageSimilar )
		return -1;
	else 
		return 1;
}

void Stg_ComponentRegister_Similarity_Helper( void* nodeData, void* _list ) {
	Stg_ComponentRegisterElement*           element       = (Stg_ComponentRegisterElement*) nodeData;
	Stg_ComponentRegister_SimilarityList*   list          = (Stg_ComponentRegister_SimilarityList*) _list;
	Type                                    componentType = element->componentType;
	float                                   stringLength  = (float) strlen( list->nameToCompare );
	float                                   typeLength    = (float) strlen( componentType );
	Index                                   array_I       = list->currentIndex;
	unsigned int                            substringLength;

	substringLength = Stg_LongestMatchingSubsequenceLength( componentType, list->nameToCompare, False );
	list->array[ array_I ].type = componentType;
	list->array[ array_I ].percentageSimilar = (float) substringLength * 100.0 / MAX( typeLength, stringLength );

	list->currentIndex++;
}

void Stg_ComponentRegister_PrintSimilar( void* componentRegister, Name name, void* _stream, unsigned int number ) {
	Stg_ComponentRegister*                  self               = (Stg_ComponentRegister*) componentRegister;
	Stream*                                 stream             = (Stream*)         _stream;
	Stg_ComponentRegister_SimilarityList    similarityList;
	Stg_ComponentRegister_SimilarityObject* similarityObject;
	Index                                   type_I;
	Index                                   count              = self->constructors->nodeCount;

	similarityList.nameToCompare = name;
	similarityList.currentIndex = 0;
	similarityList.array = Memory_Alloc_Array( Stg_ComponentRegister_SimilarityObject, count, "similarityArray");

	/* First parse the tree, calculating how similar the 'name' is to the types registered */
	BTree_ParseTree( self->constructors, Stg_ComponentRegister_Similarity_Helper, &similarityList );

	/* Sort list from most similar to least similar */
	qsort( similarityList.array, (size_t)count, sizeof( Stg_ComponentRegister_SimilarityObject ), _Stg_ComponentRegister_SimilarityCompare );
	
	/* Print out the first 'number' in list */
	if ( number > count )
		number = count;
	for ( type_I = 0 ; type_I < number ; type_I++ ) {
		similarityObject = &similarityList.array[ type_I ];
		Journal_Printf( stream, "%s (%.2f%% similar)\n", similarityObject->type, similarityObject->percentageSimilar );
	}

	Memory_Free( similarityList.array );
}


void constructorElementPrintTypeFunction( void* nodeData, void* printStream ) {
	Stg_ComponentRegisterElement* element = ( Stg_ComponentRegisterElement* ) nodeData;
	Stream*                       stream  = (Stream*) printStream;
	
	assert( nodeData );
	assert( stream );

	Journal_Printf( stream, "%s\n", element->componentType );
}

void Stg_ComponentRegister_PrintAllTypes( void* componentRegister, void* stream ) {
	Stg_ComponentRegister*                  self               = (Stg_ComponentRegister*) componentRegister;

	/* Parse the tree, printing all the names */
	BTree_ParseTree( self->constructors, constructorElementPrintTypeFunction, stream );
}

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
** $Id: Map.c 2192 2004-10-15 02:45:38Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"

#include "types.h"
#include "BTreeNode.h"
#include "BTree.h"
#include "Set.h"
#include "Map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


/* Textual name of this class */
const Type Map_Type = "Map";


/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

Map* Map_New_all(
		Dictionary*					dictionary, 
		SizeT						keySize, 
		SizeT						valueSize, 
		BTree_compareFunction*				compareFunc, 
		BTree_dataCopyFunction*				dataCopyFunc, 
		BTree_dataDeleteFunction*			dataDeleteFunc )
{
	return _Map_New( 
		sizeof(Map), 
		Map_Type, 
		_Map_Delete, 
		_Map_Print, 
		NULL, 
		_Map_Union, 
		_Map_Intersection, 
		_Map_Subtraction, 
		dictionary, 
		keySize, 
		valueSize, 
		compareFunc, 
		dataCopyFunc, 
		dataDeleteFunc );
}


void Map_Init(
		Map*						self,
		Dictionary*					dictionary, 
		SizeT						keySize, 
		SizeT						valueSize, 
		BTree_compareFunction*				compareFunc, 
		BTree_dataCopyFunction*				dataCopyFunc, 
		BTree_dataDeleteFunction*			dataDeleteFunc )
{
	/* General info */
	self->type = Map_Type;
	self->_sizeOfSelf = sizeof(Map);
	self->_deleteSelf = False;
	self->dictionary = dictionary;
	
	/* Virtual info */
	self->_delete = _Map_Delete;
	self->_print = _Map_Print;
	self->_copy = NULL;
	self->_unionFunc = _Map_Union;
	self->_intersectionFunc = _Map_Intersection;
	self->_subtractionFunc = _Map_Subtraction;
	_Stg_Class_Init( (Stg_Class*)self );
	
	/* Map info */
	_Map_Init( self, keySize, valueSize, compareFunc, dataCopyFunc, dataDeleteFunc );
}


Map* _Map_New(
		SizeT						_sizeOfSelf, 
		Type						type,
		Stg_Class_DeleteFunction*				_delete,
		Stg_Class_PrintFunction*				_print, 
		Stg_Class_CopyFunction*				_copy, 
		Map_UnionFunc*					_unionFunc, 
		Map_IntersectionFunc*				_intersectionFunc, 
		Map_SubtractionFunc*				_subtractionFunc, 
		Dictionary*					dictionary, 
		SizeT						keySize, 
		SizeT						valueSize, 
		BTree_compareFunction*				compareFunc, 
		BTree_dataCopyFunction*				dataCopyFunc, 
		BTree_dataDeleteFunction*			dataDeleteFunc )
{
	Map*	self;
	
	/* allocate memory */
	assert( _sizeOfSelf >= sizeof(Map) );
	self = (Map*)_Stg_Class_New(
		_sizeOfSelf,
		type,
		_delete,
		_print, 
		_copy );
	
	/* general info */
	self->dictionary = dictionary;
	
	/* virtual info */
	self->_unionFunc = _unionFunc;
	self->_intersectionFunc = _intersectionFunc;
	self->_subtractionFunc = _subtractionFunc;
	
	/* Map info */
	_Map_Init( self, keySize, valueSize, compareFunc, dataCopyFunc, dataDeleteFunc );
	
	return self;
}


void _Map_Init(
		Map*						self, 
		SizeT						keySize, 
		SizeT						valueSize, 
		BTree_compareFunction*				compareFunc, 
		BTree_dataCopyFunction*				dataCopyFunc, 
		BTree_dataDeleteFunction*			dataDeleteFunc )
{
	/* TODO: convert to journal */
	assert( compareFunc && dataDeleteFunc );

	self->_keySize = keySize;
	self->_valSize = valueSize;
	self->_btree = BTree_New( compareFunc, dataCopyFunc, dataDeleteFunc, NULL, BTREE_ALLOW_DUPLICATES );
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _Map_Delete( void* map ) {
	Map*	self = (Map*)map;
	
	/* delete the class itself */
	Stg_Class_Delete( self->_btree );
	
	/* delete parent */
	_Stg_Class_Delete( self );
}


void _Map_Print( void* map, Stream* stream ) {
	Map*		self = (Map*)map;
	Stream*		myStream = Journal_Register( InfoStream_Type, "MapStream" );

	/* print parent */
	_Stg_Class_Print( self, stream );
	
	/* general info */
	Journal_Printf( myStream, "Map (ptr): (%p)\n", self );
	Journal_Printf( myStream, "\tdictionary (ptr): %p\n", self->dictionary );
	
	/* virtual info */
	
	/* Map info */
}


void* _Map_Union( void* map, void* operand ) {
	Map*	self = (Map*)map;
	Map*	results;

	results = Map_New_all( self->dictionary, 
			       self->_keySize, 
			       self->_valSize, 
			       self->_btree->compareFunction, 
			       self->_btree->dataCopyFunction, 
			       self->_btree->dataDeleteFunction );

	BTree_ParseTree( self->_btree, _Map_BTreeUnion, results );
	BTree_ParseTree( ((Map*)operand)->_btree, _Map_BTreeUnion, results );

	return results;
}


void* _Map_Intersection( void* map, void* operand ) {
	Map*	self = (Map*)map;
	Map*	results;
	void*	pack[2];

	results = Map_New_all( self->dictionary, 
			       self->_keySize, 
			       self->_valSize, 
			       self->_btree->compareFunction, 
			       self->_btree->dataCopyFunction, 
			       self->_btree->dataDeleteFunction );

	pack[0] = operand;
	pack[1] = results;
	BTree_ParseTree( self->_btree, _Map_BTreeIntersection, pack );

	return results;
}


void* _Map_Subtraction( void* map, void* operand ) {
	Map*	self = (Map*)map;
	Map*	results;
	void*	pack[2];

	results = Map_New_all( self->dictionary, 
			       self->_keySize, 
			       self->_valSize, 
			       self->_btree->compareFunction, 
			       self->_btree->dataCopyFunction, 
			       self->_btree->dataDeleteFunction );

	pack[0] = operand;
	pack[1] = results;
	BTree_ParseTree( self->_btree, _Map_BTreeSubtraction, pack );

	return results;
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

Bool Map_Insert( Map* self, void* keyData, void* valueData ) {
	MapTuple	tuple;

	tuple.keyData = keyData;
	tuple.valueData = valueData;
	if( BTree_FindNode( self->_btree, &tuple ) != NULL ) {
		return False;
	}

	BTree_InsertNode( self->_btree, &tuple, sizeof(MapTuple) );

	return True;
}


void Map_Traverse( Map* self, BTree_parseFunction* func, void* args ) {
	BTree_ParseTree( self->_btree, func, args );
}


void* Map_Find( Map* self, void* keyData ) {
	MapTuple	tuple;
	BTreeNode*	node;

	tuple.keyData = keyData;
	tuple.valueData = NULL;
	if( (node = BTree_FindNode( self->_btree, &tuple )) != NULL ) {
		return ((MapTuple*)node->data)->valueData;
	}
	else {
		return NULL;
	}
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

void _Map_BTreeUnion( void* tuple, void* map ) {
	Map_Insert( (Map*)map, ((MapTuple*)tuple)->keyData, ((MapTuple*)tuple)->valueData );
}


void _Map_BTreeIntersection( void* tuple, void* pack ) {
	if( BTree_FindNode( ((Map**)pack)[0]->_btree, tuple ) != NULL ) {
		Map_Insert( ((Map**)pack)[1], ((MapTuple*)tuple)->keyData, ((MapTuple*)tuple)->valueData );
	}
}


void _Map_BTreeSubtraction( void* tuple, void* pack ) {
	if( BTree_FindNode( ((Map**)pack)[0]->_btree, tuple ) == NULL ) {
		Map_Insert( ((Map**)pack)[1], ((MapTuple*)tuple)->keyData, ((MapTuple*)tuple)->valueData );
	}
}

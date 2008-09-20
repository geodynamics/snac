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
** $Id: IndexMap.c 2192 2004-10-15 02:45:38Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"

#include "units.h"
#include "types.h"
#include "IndexMap.h"
#include "PtrMap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>


/* Textual name of this class */
const Type IndexMap_Type = "IndexMap";


/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

IndexMap* IndexMap_New_Param(
		unsigned					delta )
{
	return _IndexMap_New( 
		sizeof(IndexMap), 
		IndexMap_Type, 
		_IndexMap_Delete, 
		_IndexMap_Print, 
		_IndexMap_Copy, 
		delta );
}


void IndexMap_Init(
		IndexMap*					self,
		unsigned					delta )
{
	/* General info */
	self->type = IndexMap_Type;
	self->_sizeOfSelf = sizeof(IndexMap);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _IndexMap_Delete;
	self->_print = _IndexMap_Print;
	self->_copy = NULL;
	_Stg_Class_Init( (Stg_Class*)self );
	
	/* IndexMap info */
	_IndexMap_Init( self, delta );
}


IndexMap* _IndexMap_New(
		SizeT						_sizeOfSelf, 
		Type						type,
		Stg_Class_DeleteFunction*				_delete,
		Stg_Class_PrintFunction*				_print, 
		Stg_Class_CopyFunction*				_copy, 
		unsigned					delta )
{
	IndexMap*			self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(IndexMap) );
	self = (IndexMap*)_Stg_Class_New(
		_sizeOfSelf,
		type,
		_delete,
		_print,
		_copy );
	
	/* General info */
	
	/* Virtual info */
	
	/* IndexMap info */
	_IndexMap_Init( self, delta );
	
	return self;
}


void _IndexMap_Init(
		IndexMap*				self,
		unsigned				delta )
{
	/* General and Virtual info should already be set */
	
	/* IndexMap info */
	assert( self );
	
	self->delta = delta;
	self->maxTuples = self->delta;
	self->tupleTbl = Memory_Alloc_Array( IndexMapTuple, self->maxTuples, "IndexMap->tupleTbl" );
	assert( self->tupleTbl ); /* TODO change this to a firewall, or something */
	self->tupleCnt = 0;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _IndexMap_Delete( void* indexMap ) {
	IndexMap*			self = (IndexMap*)indexMap;
	
	/* Stg_Class_Delete the class itself */
	assert( self );
	
	if( self->tupleTbl ) {
		Memory_Free( self->tupleTbl );
	}
	
	/* Stg_Class_Delete parent */
	_Stg_Class_Delete( self );
}


void _IndexMap_Print( void* indexMap, Stream* stream ) {
	IndexMap*		self = (IndexMap*)indexMap;
	unsigned		tuple_I;
	
	/* Set the Journal for printing informations */
	Stream*			myStream;
	myStream = Journal_Register( InfoStream_Type, "IndexMapStream" );

	/* Print parent */
	assert( self );
	_Stg_Class_Print( self, stream );
	
	/* General info */
	Journal_Printf( myStream, "IndexMap (ptr): (%p)\n", self );
	
	/* Virtual info */
	
	/* IndexMap info */
	for( tuple_I = 0; tuple_I < self->tupleCnt; tuple_I++ ) {
		Journal_Printf( myStream, "\ttuple[%d]: %d -> %d\n", tuple_I, self->tupleTbl[tuple_I].key, self->tupleTbl[tuple_I].idx );
	}
}


void* _IndexMap_Copy( void* indexMap, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap ) {
	IndexMap*	self = (IndexMap*)indexMap;
	IndexMap*	newIndexMap;
	PtrMap*		map = ptrMap;
	Bool		ownMap = False;
	
	if( !map ) {
		map = PtrMap_New( 10 );
		ownMap = True;
	}
	
	newIndexMap = _Stg_Class_Copy( self, dest, deep, nameExt, map );
	
	newIndexMap->dictionary = self->dictionary;
	newIndexMap->delta = self->delta;
	newIndexMap->maxTuples = self->maxTuples;
	newIndexMap->tupleCnt = self->tupleCnt;
	
	if( deep ) {
		if( (newIndexMap->tupleTbl = PtrMap_Find( map, self->tupleTbl )) == NULL && self->tupleTbl ) {
			newIndexMap->tupleTbl = Memory_Alloc_Array( IndexMapTuple, self->maxTuples, "IndexMap->tupleTbl" );
			memcpy( newIndexMap->tupleTbl, self->tupleTbl, sizeof(IndexMapTuple) * self->maxTuples );
			PtrMap_Append( map, self->tupleTbl, newIndexMap->tupleTbl );
		}
	}
	else {
		newIndexMap->tupleTbl = self->tupleTbl;
	}
	
	if( ownMap ) {
		Stg_Class_Delete( map );
	}
	
	return (void*)newIndexMap;
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void IndexMap_Append( void* indexMap, Index key, Index idx ) {
	IndexMap*		self = (IndexMap*)indexMap;
	unsigned		newTupleCnt;
	
	assert( self && key != -1 && idx != -1 );
	
	if( IndexMap_Find( self, key ) != -1 ) {
		return;
	}
	
	newTupleCnt = self->tupleCnt + 1;
	if( newTupleCnt >= self->maxTuples ) {
		unsigned		factor;
		IndexMapTuple*		newTuples;
		
		factor = ceil( (float)(newTupleCnt - self->maxTuples) / (float)self->delta );
		self->maxTuples += factor * self->delta;
		
		newTuples = Memory_Alloc_Array( IndexMapTuple, self->maxTuples, "IndexMap->tupleTbl" );
		assert( newTuples ); /* TODO change this */
		if( self->tupleTbl ) {
			memcpy( newTuples, self->tupleTbl, sizeof(IndexMapTuple) * self->tupleCnt );
			Memory_Free( self->tupleTbl );
		}
		self->tupleTbl = newTuples;
	}
	
	self->tupleTbl[self->tupleCnt].key = key;
	self->tupleTbl[self->tupleCnt].idx = idx;
	self->tupleCnt = newTupleCnt;
}


Index IndexMap_Find( void* indexMap, Index key ) {
	IndexMap*		self = (IndexMap*)indexMap;
	unsigned		tuple_I;
	
	assert( self );
	
	if( key != -1 ) {
		for( tuple_I = 0; tuple_I < self->tupleCnt; tuple_I++ ) {
			if( self->tupleTbl[tuple_I].key == key )
				return self->tupleTbl[tuple_I].idx;
		}
	}
	
	return -1;
}


void IndexMap_Remap( void* indexMap, void* mapThrough ) {
	IndexMap*	self = (IndexMap*)indexMap;
	Index		tuple_I;
	
	for( tuple_I = 0; tuple_I < self->tupleCnt; tuple_I++ ) {
		self->tupleTbl[tuple_I].idx = IndexMap_Find( mapThrough, self->tupleTbl[tuple_I].idx );
	}
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/


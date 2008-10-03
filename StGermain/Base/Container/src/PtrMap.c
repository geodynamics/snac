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
** $Id: PtrMap.c 2192 2004-10-15 02:45:38Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"

#include "units.h"
#include "types.h"
#include "PtrMap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>


/* Textual name of this class */
const Type PtrMap_Type = "PtrMap";


/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

PtrMap* PtrMap_New(
		unsigned					delta )
{
	return _PtrMap_New( 
		sizeof(PtrMap), 
		PtrMap_Type, 
		_PtrMap_Delete, 
		_PtrMap_Print, 
		NULL, 
		delta );
}


void PtrMap_Init(
		PtrMap*						self,
		unsigned					delta )
{
	/* General info */
	self->type = PtrMap_Type;
	self->_sizeOfSelf = sizeof(PtrMap);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _PtrMap_Delete;
	self->_print = _PtrMap_Print;
	self->_copy = NULL;
	_Stg_Class_Init( (Stg_Class*)self );
	
	/* PtrMap info */
	_PtrMap_Init( self, delta );
}


PtrMap* _PtrMap_New(
		SizeT						_sizeOfSelf, 
		Type						type,
		Stg_Class_DeleteFunction*				_delete,
		Stg_Class_PrintFunction*				_print, 
		Stg_Class_CopyFunction*				_copy, 
		unsigned					delta )
{
	PtrMap*			self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(PtrMap) );
	self = (PtrMap*)_Stg_Class_New(
		_sizeOfSelf,
		type,
		_delete,
		_print,
		_copy );
	
	/* General info */
	
	/* Virtual info */
	
	/* PtrMap info */
	_PtrMap_Init( self, delta );
	
	return self;
}


void _PtrMap_Init(
		PtrMap*					self,
		unsigned				delta )
{
	/* General and Virtual info should already be set */
	
	/* PtrMap info */
	assert( self );
	
	self->delta = delta;
	self->maxTuples = self->delta;
	self->tupleTbl = Memory_Alloc_Array_Unnamed( PtrMapTuple, self->maxTuples );
	assert( self->tupleTbl ); /* TODO change this to a firewall, or something */

	self->tupleCnt = 0;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _PtrMap_Delete( void* ptrMap ) {
	PtrMap*			self = (PtrMap*)ptrMap;
	
	/* Stg_Class_Delete the class itself */
	assert( self );
	
	if( self->tupleTbl )
		Memory_Free( self->tupleTbl );
	
	/* Stg_Class_Delete parent */
	_Stg_Class_Delete( self );
}


void _PtrMap_Print( void* ptrMap, Stream* stream ) {
	PtrMap*			self = (PtrMap*)ptrMap;
	
	/* Set the Journal for printing informations */
	Stream*			myStream;
	myStream = Journal_Register( InfoStream_Type, "PtrMapStream" );

	/* Print parent */
	assert( self );
	_Stg_Class_Print( self, stream );
	
	/* General info */
	Journal_Printf( myStream, "PtrMap (ptr): (%p)\n", self );
	
	/* Virtual info */
	
	/* PtrMap info */
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void PtrMap_Append( void* ptrMap, void* key, void* ptr ) {
	PtrMap*			self = (PtrMap*)ptrMap;
	unsigned		newTupleCnt;
	
	assert( self && key && ptr );
	
	if( PtrMap_Find( self, key ) != NULL ) {
		return;
	}
	
	newTupleCnt = self->tupleCnt + 1;
	if( newTupleCnt >= self->maxTuples ) {
		unsigned		factor;
		PtrMapTuple*		newTuples;
		
		factor = ceil( (float)(newTupleCnt - self->maxTuples) / (float)self->delta );
		self->maxTuples += factor * self->delta;
		
		newTuples = Memory_Alloc_Array_Unnamed( PtrMapTuple, self->maxTuples );
		assert( newTuples ); /* TODO change this */
		if( self->tupleTbl ) {
			memcpy( newTuples, self->tupleTbl, sizeof(PtrMapTuple) * self->tupleCnt );
			Memory_Free( self->tupleTbl );
		}
		self->tupleTbl = newTuples;
	}
	
	self->tupleTbl[self->tupleCnt].key = key;
	self->tupleTbl[self->tupleCnt].ptr = ptr;
	self->tupleCnt = newTupleCnt;
}


void* PtrMap_Find( void* ptrMap, void* key ) {
	PtrMap*			self = (PtrMap*)ptrMap;
	unsigned		tuple_I;
	
	assert( self );
	
	if( key ) {
		for( tuple_I = 0; tuple_I < self->tupleCnt; tuple_I++ ) {
			if( self->tupleTbl[tuple_I].key == key )
				return self->tupleTbl[tuple_I].ptr;
		}
	}
	
	return NULL;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

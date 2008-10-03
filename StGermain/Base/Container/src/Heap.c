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
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/** \file
** <b>Role:</b>
**	Binary Tree class for objects.
**
** <b>Assumptions:</b>
**	None
**
** <b>Comments:</b>
**	None
**
** $Id: Heap.c 2087 2005-2-25 02:28:44Z RaquibulHassan $
**
**/

#include <Base/Foundation/Foundation.h>
#include <Base/IO/IO.h>

#include "types.h"
#include "Heap.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

const Type Heap_Type = "Heap";

_Heap* _Heap_New(
				SizeT							_sizeOfSelf,
				Type							type,
				Stg_Class_DeleteFunction*			_delete,
				Stg_Class_PrintFunction*			_print,
				Stg_Class_CopyFunction*				_copy,
				Heap_HeapifyFunction				*heapify,
				Heap_ExtractFunction				*extract,
				Heap_InsertFunction					*insert,
				Heap_KeySwapFunction*				keySwapFunction,
				Heap_CompareFunction				*compareFunction,
				Heap_ExtendArrayFunc				*extendArray,
				void						**keys,
				int							keyLength,
				int							numArrayElements )
{
	_Heap *self = NULL;

	self = (_Heap*)_Stg_Class_New(
							_sizeOfSelf,
							type,
							_delete,
							_print,
							_copy);

	assert( heapify );
	assert( extract );
	assert( insert );
	assert( compareFunction );
	assert( keySwapFunction );
	assert( keys );
	assert( extendArray );
	
	self->heapify = heapify;
	self->extract = extract;
	self->insert = insert;
	self->keySwap = keySwapFunction;
	self->compare = compareFunction;
	self->extendArray = extendArray;
	self->numArrayElements = numArrayElements;
	self->keys = keys;
	self->keyLength = keyLength;
	
	_Heap_Init( self );
	
	return self;
}

void _Heap_Init( _Heap* self )
{
	assert( self );
	
	self->numHeapElements = self->numArrayElements;

	_Heap_BuildHeap( self );
}

void _Heap_BuildHeap( _Heap *heap )
{
	int i = 0;
	
	assert( heap );
	assert( heap->heapify );
	
	for( i=heap->numArrayElements/2+1; i>=1; i--  ){
		heap->heapify( heap, i );
	}
}

void _Heap_Heapify( _Heap *heap, int index )
{
	
}

void* _Heap_Extract( _Heap *heap )
{
	return NULL;
}

void _Heap_Insert( _Heap *heap, void* key )
{
	
}
	
void _Heap_DeleteFunc ( void *heap )
{
	_Heap *self = ( _Heap* ) heap;

	assert( self );

	Memory_Free( self->keys );
	_Stg_Class_Delete( self );
}
	
void _Heap_PrintFunc ( void *heap, Stream* stream )
{
	_Heap *self = (_Heap*)heap;
	assert( self );

	/* print parent */
	_Stg_Class_Print( (void*) self, stream );

	/* general info */
	Journal_Printf( stream, "Heap (ptr): (%p)\n", self );
	
	Journal_Printf( stream, "Heap Elements\t%d\n", self->numHeapElements );
	Journal_Printf( stream, "Array Elements\t%d\n", self->numArrayElements );
	Journal_Printf( stream, "Key Length    \t%d\n", self->keyLength );
}
	
/*Private functions*/

int _Heap_Parent( _Heap *heap, int i )
{
	return i/2;
}

int _Heap_Left( _Heap *heap, int i )
{
	return 2*i;
}

int _Heap_Right( _Heap *heap, int i )
{
	return 2*i+1;
}

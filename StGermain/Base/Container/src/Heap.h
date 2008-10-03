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
** This class creates a heap data structure from a given set of keys. It receives
** an array of pointer to the keys and few function pointers to achieve the task.
** The class assumes ownership of the array of the keys, alters the order of the
** keys as the heap is built, and deletes the keys array when the destructor is
** called on the class.
** The _Heap_Insert function is useful for inserting new items into the heap after
** it has been built, which entails the extension of the initial array. The user
** supplied function 'extendArray' is invoked on such occasions for extending the
** initial array.
** 
** $Id: Heap.h 2087 2005-2-25 02:28:44Z RaquibulHassan $
**
**/

#ifndef __Heap_h__
#define __Heap_h__

	/** Textual name for List class. */
	extern const Type Heap_Type;
	
	/** Virtual Function Pointers */
	
	typedef void (Heap_HeapifyFunction) ( struct _Heap*, int );
	typedef void* (Heap_ExtractFunction) ( struct _Heap* );
	typedef void (Heap_InsertFunction) ( struct _Heap*, void* );
	typedef void (Heap_KeySwapFunction) ( void**, void** );
	typedef int (Heap_CompareFunction) ( void*, void* );
	typedef void** (Heap_ExtendArrayFunc) ( int, void *** );
	
	/** \def __List See __List */
	#define __Heap \
		/* General info */ \
		__Stg_Class \
		\
		/* Virtual info */ \
		\
		/* Heap info */ \
		Heap_HeapifyFunction*		heapify; \
		Heap_ExtractFunction*		extract; \
		Heap_InsertFunction*		insert; \
		Heap_KeySwapFunction*		keySwap; \
		Heap_CompareFunction*		compare; \
		Heap_ExtendArrayFunc*		extendArray; \
		int							numHeapElements; \
		void						**keys; \
		int							keyLength; \
		int							numArrayElements;
	struct _Heap { __Heap };

	_Heap* _Heap_New(
				SizeT							_sizeOfSelf,
				Type							type,
				Stg_Class_DeleteFunction*			_delete,
				Stg_Class_PrintFunction*			_print,
				Stg_Class_CopyFunction*				_copy,
				Heap_HeapifyFunction				*heapify,
				Heap_ExtractFunction				*extract,
				Heap_InsertFunction					*insert,
				Heap_KeySwapFunction*				KeySwapFunction,
				Heap_CompareFunction				*compareFunction,
				Heap_ExtendArrayFunc				*extendArray,
				void						**keys,
				int							keyLength,
				int							numArrayElements
				);

	
	void _Heap_Init( _Heap* self );
	
	void _Heap_BuildHeap( _Heap *heap );

	void _Heap_Heapify( _Heap *heap, int index );

	void* _Heap_Extract( _Heap *heap );

	void _Heap_Insert( _Heap *heap, void* key );
	
	/** Stg_Class_Delete interface. */
		/** Stg_Class delete function */
	void _Heap_DeleteFunc ( void *heap );
	
	/** Print interface. */
		/** Stg_Class print function */
	void _Heap_PrintFunc ( void *heap, Stream* stream );

	/* Private functions */
	int _Heap_Parent( _Heap *heap, int i );
	
	int _Heap_Left( _Heap *heap, int i );
	
	int _Heap_Right( _Heap *heap, int i );

#endif /* __Heap_h__ */


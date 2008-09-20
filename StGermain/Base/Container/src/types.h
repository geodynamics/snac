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
**	Basic framework types.
**
** Assumptions:
**	None as yet.
**
** Comments:
**	None as yet.
**
** $Id: types.h 3789 2006-09-11 03:26:19Z RaquibulHassan $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Base_Container_types_h__
#define __Base_Container_types_h__
	
	/* IndexSet types */
	typedef Index					IndexSet_Index;
	
	/* classes */
	typedef struct IndexSet				IndexSet;
	typedef struct PtrMap				PtrMap;
	typedef struct IndexMap				IndexMap;
	typedef struct List				List;
	typedef struct Array				Array;
	typedef struct LinkedListNode			LinkedListNode;
	typedef struct LinkedList			LinkedList;
	typedef struct LinkedListIterator	LinkedListIterator;
	typedef struct HashTable_Entry		HashTable_Entry;
	typedef struct HashTable_Index		HashTable_Index;
	typedef struct HashTable			HashTable;
	typedef struct UniqueList			UniqueList;
	typedef struct Set				Set;
	typedef struct PtrSet				PtrSet;
	typedef struct Map				Map;
	typedef struct _Heap			_Heap;
	typedef struct MaxHeap			MaxHeap;
	typedef struct UIntMap			UIntMap;
	typedef struct MemoryPool		MemoryPool;
	typedef struct RangeSet			RangeSet;
	typedef unsigned char		Stg_Byte;
	typedef char					BitField;
	
	typedef struct BTreeNode			BTreeNode;
	typedef struct BTree				BTree;
	typedef struct BTreeIterator			BTreeIterator;

	typedef struct MapTuple				MapTuple;

	typedef enum Color_t{
		BTREE_NODE_RED,
		BTREE_NODE_BLACK
	}
	Color;

	typedef enum BTreeProperty_t{
		BTREE_ALLOW_DUPLICATES,
		BTREE_NO_DUPLICATES
	}
	BTreeProperty;

	typedef enum Order_t{
		LINKEDLIST_SORTED,
		LINKEDLIST_UNSORTED
	}
	Order;

	typedef enum KeyType_t{
		HASHTABLE_STRING_KEY,
		HASHTABLE_INTEGER_KEY,
		HASHTABLE_POINTER_KEY
	}
	KeyType;

#define LinkedListIterator_First( it ) \
	(it==NULL)?NULL:(it->list == NULL)?NULL:((it->curr = it->list->head)==NULL)?NULL:it->curr->data

#define LinkedListIterator_Next( it ) \
	(it==NULL)?NULL:(it->curr == NULL)?NULL:((it->curr = it->curr->next)==NULL)?NULL:it->curr->data

#endif /* __Base_Container_types_h__ */


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
**	HashTableIterator class for objects.
**
** <b>Assumptions:</b>
**	None
**
** <b>Comments:</b>
**	None
**
** $Id: HashTableIterator.h 2087 2005-4-16 02:28:44Z RaquibulHassan $
**
**/

#ifndef __Base_Foundation__HashTableIterator_h__
#define __Base_Foundation__HashTableIterator_h__

	/** Textual name for HashTableIterator class. */
	extern const Type HashTableIterator_Type;
	
	/** Virtual Function Pointers */
	
	/** \def __HashTableIterator_Iterator See __HashTable_Iterator */
	#define __HashTableIterator \
		__Stg_Class \
		HashTable		*ht; \
		HashTable_Entry	*curr; \
		int				index;
	
	struct HashTableIterator { __HashTableIterator };
	

	/** Constructor interface. */
	HashTableIterator* HashTableIterator_New( HashTable *ht );
	
	HashTableIterator *_HashTableIterator_New(
			SizeT					_sizeOfSelf,
			Type					type,
			Stg_Class_DeleteFunction*	_delete,
			Stg_Class_PrintFunction*	_print,
			Stg_Class_CopyFunction*		_copy
			);
	
	/** Init interface. */
	void _HashTableIterator_Init( HashTableIterator *self );
	
	/** Stg_Class_Delete Interface */
	void _HashTableIterator_DeleteFunc( void *self );

	/** Print Interface */
	void _HashTableIterator_PrintFunc( void *, Stream * );

	HashTable_Entry *HashTableIterator_First( HashTableIterator *hi );
	
	HashTable_Entry *HashTableIterator_Next( HashTableIterator *hi );
	
#endif /* __Base_Foundation__HashTableIterator_h__ */

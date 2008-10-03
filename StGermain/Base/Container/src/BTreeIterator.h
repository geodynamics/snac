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
** $Id: BTreeIterator.h 2087 2005-4-16 02:28:44Z RaquibulHassan $
**
**/

#ifndef __Base_Foundation__BTreeIterator_h__
#define __Base_Foundation__BTreeIterator_h__

#define MAX_DEPTH 100

	/** Textual name for BTreeIterator class. */
	extern const Type BTreeIterator_Type;
	
	/** Virtual Function Pointers */
	
	/** \def __BTreeIterator_Iterator See __BTree_Iterator */
	#define __BTreeIterator \
		__Stg_Class \
		BTree						*tree; \
		int							depth; \
		BTreeNode					*stack[MAX_DEPTH];
	
	struct BTreeIterator { __BTreeIterator };
	

	/** Constructor interface. */
	BTreeIterator* BTreeIterator_New( BTree *tree );
	
	BTreeIterator *_BTreeIterator_New(
			SizeT					_sizeOfSelf,
			Type					type,
			Stg_Class_DeleteFunction*	_delete,
			Stg_Class_PrintFunction*	_print,
			Stg_Class_CopyFunction*		_copy
			);
	
	/** Init interface. */
	void _BTreeIterator_Init( BTreeIterator *self );
	
	void BTreeIterator_Init( BTreeIterator *self );

	/** Stg_Class_Delete Interface */
	void _BTreeIterator_DeleteFunc( void *self );

	/** Print Interface */
	void _BTreeIterator_PrintFunc( void *, Stream * );
	
	/** Public Functions **/
		/* Returns the data stored inside the Tree's first node ( will be used for iterating through the tree ) */
	void *BTreeIterator_First( BTreeIterator *self );
		
	/* Returns the data stored inside the Tree's next node ( will be used for iterating through the tree ) */
	void *BTreeIterator_Next( BTreeIterator *self );
	
#endif /* __Base_Foundation__BTreeIterator_h__ */

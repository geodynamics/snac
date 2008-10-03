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
**	LinkedListIterator class for objects.
**
** <b>Assumptions:</b>
**	None
**
** <b>Comments:</b>
**	None
**
** $Id: LinkedListIterator.h 2087 2005-4-16 02:28:44Z RaquibulHassan $
**
**/

#ifndef __Base_Foundation__LinkedListIterator_h__
#define __Base_Foundation__LinkedListIterator_h__

	/** Textual name for LinkedListIterator class. */
	extern const Type LinkedListIterator_Type;
	
	/** Virtual Function Pointers */
	
	/** \def __LinkedListIterator_Iterator See __LinkedList_Iterator */
	#define __LinkedListIterator \
		__Stg_Class \
		LinkedList		*list; \
		LinkedListNode	*curr;
	
	struct LinkedListIterator { __LinkedListIterator };
	

	/** Constructor interface. */
	LinkedListIterator* LinkedListIterator_New( LinkedList *list );
	
	LinkedListIterator *_LinkedListIterator_New(
			SizeT					_sizeOfSelf,
			Type					type,
			Stg_Class_DeleteFunction*	_delete,
			Stg_Class_PrintFunction*	_print,
			Stg_Class_CopyFunction*		_copy
			);
	
	/** Init interface. */
	void _LinkedListIterator_Init( LinkedListIterator *self );
	
	/** Stg_Class_Delete Interface */
	void _LinkedListIterator_DeleteFunc( void *self );

	/** Print Interface */
	void _LinkedListIterator_PrintFunc( void *, Stream * );
	
#endif /* __Base_Foundation__LinkedListIterator_h__ */

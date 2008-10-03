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
**	Memory-efficient class faciliting a set of Boolean values for a
**	group of indices.
**
** Assumptions:
**
** Comments:
**	The Boolean values are stored using single bits in a bit field.
**	The total count of values in the set is currently fixed.
**	IndexSet_GetMembers() and IndexSet_GetVacancies() is potentially a CPU-hog
**	if called very frequently on a large set. If this becomes a problem, the
**	client code's algorithm may need to be improved with a lookup or hash
**	table: see MeshDecomp .
**
**
** $Id: IndexSet.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Base_Container_IndexSet_h__
#define __Base_Container_IndexSet_h__
	

	/** Virtual function template */
	typedef		void	(IndexSet_DuplicateFunction)		( void* indexSet, void* newIndexSet );
	
	
	/** Textual name of this class */
	extern const Type IndexSet_Type;
	
	
	/** IndexSet define (see IndexSet::) */
	#define __IndexSet \
		/* General info */ \
		__Stg_Class \
		\
		/* Virtual info */ \
		IndexSet_DuplicateFunction*	_duplicate; \
		\
		/* IndexSet info */ \
		Stream*				error; \
		/** Number of items in the set. Currently fixed. */ \
		IndexSet_Index			size; \
		/** Total size of the container in bytes */ \
		SizeT				_containerSize; \
		/** The set of boolean-repesenting bytes */ \
		char*				_container; \
		/** Counter of 'Trues' in set, is reset to unsigned -1 when Add or Remove is called. */ \
		IndexSet_Index			membersCount;					

	/**	Memory-efficient class faciliting a set of Boolean values for a
	**	group of indices. See IndexSet.h for documentation of member functions & the module. */
	struct IndexSet { __IndexSet };
	
	/** Macro to test if a returned index represents invalid */
	#define IndexSet_Invalid( self ) \
		(self)->size
	
	/** Create a IndexSet:: of the given size. All values are
	initially False.*/
	IndexSet* IndexSet_New( IndexSet_Index size );
	
	/** IndexSet_New() for when user created the class already. */
	void IndexSet_Init( IndexSet* self, IndexSet_Index size );
	
	/** IndexSet_New() implementation. */
	IndexSet* _IndexSet_New( 
		SizeT				_sizeOfSelf, 
		Type				type,
		Stg_Class_DeleteFunction*		_delete,
		Stg_Class_PrintFunction*		_print, 
		Stg_Class_CopyFunction*		_copy, 
		IndexSet_DuplicateFunction*	_duplicate,
		IndexSet_Index			size );
	
	/** IndexSet_Init() implementation. */
	void _IndexSet_Init( IndexSet* self, IndexSet_Index size );
	
	/** Stg_Class_Delete() implementation.  */
	void _IndexSet_Delete( void* indexIndexSet );
	
	/** Stg_Class_Print() implementation. */
	void _IndexSet_Print( void* indexIndexSet, Stream* stream );
	
	/* Copy */
	#define IndexSet_Copy( self ) \
		(IndexSet*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define IndexSet_DeepCopy( self ) \
		(IndexSet*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _IndexSet_Copy( void* indexSet, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/** "Add" an index - i.e., set a particular index into the set's value as
	True. If true already, value is unchanged. */
	void IndexSet_Add( void* indexSet, Index index );
	
	/** "Remove" an index - i.e., set a particular index into the set's value
	as False. If false already, value is unchanged. */
	void IndexSet_Remove( void* indexSet, Index index );
	
	/** Is the index a member of this set? (is its index in the set currently
	True?) */
	Bool IndexSet_IsMember( void* indexSet, Index index );
	
	/** Gets the current number of index members */
	IndexSet_Index IndexSet_UpdateMembersCount( void* indexSet );
	
	/** Returns the index of the Nth member in the set. Returns IndexSet_Invalid if there
	are less members than requested. Note that first member's index is found by passing
	0 for nthMember. */
	IndexSet_Index IndexSet_GetIndexOfNthMember( void* indexSet, const Index nthMember );
	
	/** Updates an array, already correctly allocated to the size returned by 
	IndexSet_UpdateMemebersCount(), to contain the array of all indices that are 
	currently True in the set. Use in preference to IndexSet_GetMembers() if memory
	tracing is important. */
	void IndexSet_GetMembers2( void* indexSet, Index* const array );

	/** Return a dynamically allocated array of all indices in the set that
	are currently True. Note that the time is proportional to the number
	of elements in the set, so use with some caution for large sets. */
	void IndexSet_GetMembers( void* indexSet, IndexSet_Index* countPtr, Index** arrayPtr );
	
	/** Return a dynamically allocated array of all indices in the set that
	are currently False. The same speed issue as applies for
	IndexSet_GetMembers() applies for this function. */
	void IndexSet_GetVacancies( void* indexSet, IndexSet_Index* countPtr, Index** arrayPtr );
	
	/** Do a binary OR on each pair of corresponding members of 2 given sets,
	and save the result to the first set. Note that if the sets are different
	lengths, the OR operation is only applied up to the shorter of the 2
	lengths. Thus if the second set is shorter, there will be some uncompared
	values at the end of the first set.
	*/
	void IndexSet_Merge_OR( void* indexSet, void* merger );
	
	/** Do a binary AND on each pair of corresponding members of 2 given sets,
	and save the result to the first set. Note that if the sets are different
	lengths, the AND operation is only applied up to the shorter of the 2
	lengths. Thus if the second set is shorter, there will be some uncompared
	values at the end of the first set.
	*/
	void IndexSet_Merge_AND( void* indexSet, void* merger );
	
	/** Sets every index in the set as True. */
	void IndexSet_AddAll( void* indexSet );
	
	/** Sets every index in the set as False. */
	void IndexSet_RemoveAll( void* indexSet );
	
	/** Returns a pointer to a dynamically allocated copy of the current set.*/
	IndexSet* IndexSet_Duplicate( void* indexSet );
	
	/** IndexSet_Duplicate() implementation. */
	void _IndexSet_Duplicate( void* indexSet, void* newIndexSet );
	
	/** Prints message and exits properly when bad access/write detected. */
	void _IndexSet_Abort( IndexSet* indexSet, const char* const funcName, IndexSet_Index index );

#endif /* __Base_Container_IndexSet_h__ */

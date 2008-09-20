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
** $Id: IndexSet.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdarg.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"

#include "units.h"
#include "types.h"
#include "IndexSet.h"

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>


/* Textual name of this class */
const Type IndexSet_Type = "IndexSet";

/** Macro to calculate container membership */
#define IS_MEMBER( indexSet, index ) \
	((indexSet)->_container[(index) / (sizeof(char) * 8 )] & (1 << ((index) % (sizeof(char) * 8))))

IndexSet* IndexSet_New( IndexSet_Index size ) {
	return _IndexSet_New( sizeof(IndexSet), IndexSet_Type, _IndexSet_Delete, _IndexSet_Print, _IndexSet_Copy, _IndexSet_Duplicate, size );
}

void IndexSet_Init( IndexSet* self, IndexSet_Index size ) {
	/* General info */
	self->type = IndexSet_Type;
	self->_sizeOfSelf = sizeof(IndexSet);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _IndexSet_Delete;
	self->_print = _IndexSet_Print;
	self->_copy = _IndexSet_Copy;
	self->_duplicate = _IndexSet_Duplicate;
	_Stg_Class_Init( (Stg_Class*)self );
	
	/* IndexSet info */
	_IndexSet_Init( self, size );
}


IndexSet* _IndexSet_New( 
		SizeT				sizeOfSelf, 
		Type				type,
		Stg_Class_DeleteFunction*		_delete,
		Stg_Class_PrintFunction*		_print, 
		Stg_Class_CopyFunction*		_copy, 
		IndexSet_DuplicateFunction*	_duplicate,
		IndexSet_Index			size )
{
	IndexSet* self;
	
	/* Allocate memory */
	assert( sizeOfSelf >= sizeof(IndexSet) );
	self = (IndexSet*)_Stg_Class_New( sizeOfSelf, type, _delete, _print, _copy );
	
	/* General info */
	
	/* Virtual functions */
	self->_duplicate = _duplicate;
	
	/* IndexSet info */
	_IndexSet_Init( self, size );
	
	return self;
}


void _IndexSet_Init( IndexSet* self, IndexSet_Index size ) {
	/* General and Virtual info should already be set */
	
	/* IndexSet info */
	self->size = size;
	self->_containerSize = self->size / (sizeof(char) * 8) + (self->size % (sizeof(char) * 8) ? 1 : 0);
	self->_container = Memory_Alloc_Array( BitField, self->_containerSize, "IndexSet->_container");
	memset( self->_container, 0, sizeof(char) * self->_containerSize );
	self->membersCount = 0;
	self->error = Journal_Register( ErrorStream_Type, self->type );
}


void _IndexSet_Delete( void* indexSet ) {
	IndexSet* self = (IndexSet*)indexSet;
	
	if( self->_container ) {
		Memory_Free( self->_container );
	}
	
	/* Stg_Class_Delete parent class */
	_Stg_Class_Delete( self );
}


void _IndexSet_Print( void* indexSet, Stream* stream ) {
	IndexSet* self = (IndexSet*)indexSet;
	
	/* Set the Journal for printing informations */
	Stream* indexSetStream = Journal_Register( InfoStream_Type, "IndexSetStream");
	
	/* General info */
	Journal_Printf( indexSetStream, "IndexSet (%p):\n", self );
	
	/* Virtual info */
	
	/* IndexSet */
	Journal_Printf( indexSetStream, "\tsize: %u\n", self->size );
	Journal_Printf( indexSetStream, "\t_containerSize: %lu\n", self->_containerSize );
	Journal_Printf( indexSetStream, "\t_container: %p\n", self->_container );

	if( self->_container ) {
		IndexSet_Index		index_I;
		
		for( index_I = 0; index_I < self->size; index_I++ ) {
			Index			containerIndex;
			char			indexMask;
			
			containerIndex = index_I / (sizeof(char) * 8);
			indexMask = 1 << (index_I % (sizeof(char) * 8));
			
			if( self->_container[containerIndex] & indexMask ) {
				Journal_Printf( indexSetStream, "\t\tindex %u: In set.\n", index_I );
			}
			else {
				Journal_Printf( indexSetStream, "\t\tindex %u: Not in set.\n", index_I );
			}
		}
	}
	
	/* Print parent class */
	_Stg_Class_Print( self, stream );
}


void* _IndexSet_Copy( void* indexSet, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	IndexSet*	self = (IndexSet*)indexSet;
	IndexSet*	newIndexSet;
	
	newIndexSet = (IndexSet*)_Stg_Class_Copy( self, dest, deep, nameExt, ptrMap );
	
	/* Virtual methods */
	newIndexSet->_duplicate = self->_duplicate;
	
	newIndexSet->size = self->size;
	newIndexSet->_containerSize = self->_containerSize;
	newIndexSet->_container = Memory_Alloc_Array( char, newIndexSet->_containerSize, "IndexSet->_container" );
	memcpy( newIndexSet->_container, self->_container, sizeof(char) * newIndexSet->_containerSize );
	newIndexSet->membersCount = self->membersCount;
	newIndexSet->error = self->error;
	
	return (void*)newIndexSet;
}


void IndexSet_Add( void* indexSet, Index index ) {
	IndexSet* self = (IndexSet*)indexSet;

	#ifdef CAUTIOUS
		Journal_Firewall( index < self->size, self->error, "Error- %s: index %u outside current size %d. Aborting.\n",
			__func__, index, self->size);
	#endif
	
	self->_container[index / (sizeof(char) * 8)] |= (1 << (index % (sizeof(char) * 8)));
	self->membersCount = (unsigned)-1;
}


void IndexSet_Remove( void* indexSet, Index index ) {
	IndexSet* self = (IndexSet*)indexSet;

	#ifdef CAUTIOUS
		Journal_Firewall( index < self->size, self->error, "Error- %s: index %u outside current size %d. Aborting.\n",
			__func__, index, self->size);
	#endif
	
	self->_container[index / (sizeof(char) * 8)] &= ~(1 << (index % (sizeof(char) * 8)));
	self->membersCount = (unsigned)-1;
}

Bool IndexSet_IsMember( void* indexSet, Index index ) {
	IndexSet* self = (IndexSet*)indexSet;

	#ifdef CAUTIOUS
		Journal_Firewall( index < self->size, self->error, "Error- %s: index %u outside current size %d. Aborting.\n",
			__func__, index, self->size);
	#endif
	
	return IS_MEMBER( self, index );
}


IndexSet_Index IndexSet_GetIndexOfNthMember( void* indexSet, const Index nthMember ) {
	IndexSet*		self = (IndexSet*)indexSet;
	int		membersSoFar = -1;
	IndexSet_Index	index_I;
	
	for( index_I = 0; index_I < self->size; index_I++ ) {
		if( IS_MEMBER( self, index_I ) ) {
			membersSoFar++;
			if ( nthMember == membersSoFar ) {
				return index_I;
			}
		}
	}

	return IndexSet_Invalid( self );
}


IndexSet_Index IndexSet_UpdateMembersCount( void* indexSet ) {
	IndexSet*		self = (IndexSet*)indexSet;
	IndexSet_Index		index_I;

	if (self->membersCount == (unsigned)-1 ) {
		self->membersCount = 0;
		for( index_I = 0; index_I < self->size; index_I++ ) {
			if( IS_MEMBER( self, index_I ) ) {
				self->membersCount++;
			}
		}
	}

	return self->membersCount;
}


void IndexSet_GetMembers( void* indexSet, IndexSet_Index* countPtr, Index** arrayPtr ) {
	IndexSet*		self = (IndexSet*)indexSet;
	
	*countPtr = IndexSet_UpdateMembersCount( self );
	if( *countPtr ) {
		*arrayPtr = Memory_Alloc_Array( Index, (*countPtr), "IndexSet members" );
		IndexSet_GetMembers2( self, *arrayPtr );
	}
	else {
		*arrayPtr = NULL;
	}
}	


void IndexSet_GetMembers2( void* indexSet, Index* const array ) {
	IndexSet*		self = (IndexSet*)indexSet;
	IndexSet_Index		index_I;
	unsigned int		array_I;

	for( array_I = 0, index_I = 0; index_I < self->size; index_I++ ) {
		if( IS_MEMBER( self, index_I ) ) {
			array[array_I] = index_I;
			array_I++;
		}
	}
}


void IndexSet_GetVacancies( void* indexSet, IndexSet_Index* countPtr, Index** arrayPtr ) {
	IndexSet*		self = (IndexSet*)indexSet;
	IndexSet_Index		index_I;
	unsigned int		array_I;

	IndexSet_UpdateMembersCount( self );
	*countPtr = self->size - self->membersCount;
	
	*arrayPtr = Memory_Alloc_Array( Index, (*countPtr), "IndexSet vacancies" );
	for( array_I = 0, index_I = 0; index_I < self->size; index_I++ ) {
		if( !IS_MEMBER( self, index_I) ) {
			(*arrayPtr)[array_I] = index_I;
			array_I++;
		}
	}
}


void IndexSet_Merge_OR(void* indexSet, void* indexSetTwo )
{
	IndexSet*	self = (IndexSet*)indexSet;
	IndexSet*	secondSet = (IndexSet*)indexSetTwo;
	Index		size;
	Index		i;
	
	size = self->size <= secondSet->size ? self->_containerSize : secondSet->_containerSize;
	
	for (i = 0; i < size; i++)
		self->_container[i] |= secondSet->_container[i];
	
	self->membersCount = (unsigned int)-1;
}


void IndexSet_Merge_AND(void* indexSet, void* indexSetTwo )
{
	IndexSet*	self = (IndexSet*)indexSet;
	IndexSet*	secondSet = (IndexSet*)indexSetTwo;
	Index		size;
	Index		i;
	
	size = self->size <= secondSet->size ? self->_containerSize : secondSet->_containerSize;
	
	for (i = 0; i < size; i++)
		self->_container[i] &= secondSet->_container[i];
	
	self->membersCount = (unsigned int)-1;
}


void IndexSet_AddAll( void* indexSet )
{
	IndexSet*	self = (IndexSet*)indexSet;
	Index		i;
	
	for( i = 0; i < self->_containerSize; i++)
		self->_container[i] |= 0xFF;

	self->membersCount = self->size;
}


void IndexSet_RemoveAll( void* indexSet )
{
	IndexSet*	self = (IndexSet*)indexSet;
	Index		i;
	
	for( i = 0; i < self->_containerSize; i++)
		self->_container[i] &= 0x00;

	self->membersCount = 0;
}


IndexSet* IndexSet_Duplicate( void* indexSet ) {
	IndexSet*	self = (IndexSet*)indexSet;
	IndexSet*	newSelf = 0;
	
	if( self  ) {
		newSelf = _IndexSet_New( self->_sizeOfSelf, self->type, self->_delete, self->_print, NULL, self->_duplicate, self->size );
		self->_duplicate( self, newSelf );
	}
	return newSelf;
}


void _IndexSet_Duplicate( void* indexSet, void* newIndexSet ){
	/* self->containerSize and self->container are set by _IndexSet_Init */
	IndexSet*	self = (IndexSet*)indexSet;
	IndexSet*	newSet = (IndexSet*)newIndexSet;
	memcpy( ((IndexSet*)newIndexSet)->_container, ((IndexSet*)indexSet)->_container, sizeof(char)*((IndexSet*)indexSet)->_containerSize );
	newSet->membersCount = self->membersCount;
}

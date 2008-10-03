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
** $Id: RangeSet.c 3584 2006-05-16 11:11:07Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <mpi.h>

#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"

#include "types.h"
#include "BTreeNode.h"
#include "BTree.h"
#include "RangeSet.h"


/* Textual name of this class */
const Type RangeSet_Type = "RangeSet";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

RangeSet* RangeSet_New() {
	return _RangeSet_New( sizeof(RangeSet), 
			      RangeSet_Type, 
			      _RangeSet_Delete, 
			      _RangeSet_Print, 
			      _RangeSet_Copy );
}

RangeSet* _RangeSet_New( RANGESET_DEFARGS ) {
	RangeSet* self;

	/* Allocate memory */
	assert( sizeOfSelf >= sizeof(RangeSet) );
	self = (RangeSet*)_Stg_Class_New( STG_CLASS_PASSARGS );

	/* Virtual info */

	/* RangeSet info */
	_RangeSet_Init( self );

	return self;
}

void _RangeSet_Init( RangeSet* self ) {
	assert( self );

	self->nInds = 0;
	self->nRanges = 0;
	self->ranges = NULL;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _RangeSet_Delete( void* rangeSet ) {
	RangeSet*	self = (RangeSet*)rangeSet;

	assert( self );

	RangeSet_Destruct( self );

	/* Delete the parent. */
	_Stg_Class_Delete( self );
}

void _RangeSet_Print( void* rangeSet, Stream* stream ) {
	RangeSet*	self = (RangeSet*)rangeSet;
	
	/* Set the Journal for printing informations */
	Stream* rangeSetStream;
	rangeSetStream = Journal_Register( InfoStream_Type, "RangeSetStream" );

	/* Print parent */
	Journal_Printf( stream, "RangeSet (ptr): (%p)\n", self );
	_Stg_Class_Print( self, stream );
}

void* _RangeSet_Copy( void* rangeSet, void* destProc_I, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	RangeSet*	self = (RangeSet*)rangeSet;
	RangeSet*	newRangeSet;

	newRangeSet = RangeSet_New();
	newRangeSet->nInds = self->nInds;
	newRangeSet->nRanges = self->nRanges;
	if( self->nRanges ) {
		newRangeSet->ranges = Memory_Alloc_Array( RangeSet_Range, self->nRanges, "RangeSet::ranges" );
		memcpy( newRangeSet->ranges, self->ranges, self->nRanges * sizeof(RangeSet_Range) );
	}

	return (void*)newRangeSet;
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void RangeSet_SetIndices( void* rangeSet, unsigned nInds, unsigned* inds ) {
	RangeSet*	self = (RangeSet*)rangeSet;
	unsigned*	tmpInds;
	unsigned	curInd = 0;

	assert( self );
	assert( !nInds || inds );

	RangeSet_Destruct( self );
	if( !nInds ) return;

	self->nInds = nInds;
	tmpInds = Memory_Alloc_Array_Unnamed( unsigned, nInds );
	memcpy( tmpInds, inds, nInds * sizeof(unsigned) );
	qsort( tmpInds, nInds, sizeof(unsigned), RangeSet_SortCmp );

	while( curInd < nInds ) {
		RangeSet_Range*	range;

		if( !self->ranges ) {
			self->ranges = Memory_Alloc_Array( RangeSet_Range, ++self->nRanges, "RangeSet::ranges" );
		}
		else {
			self->ranges = Memory_Realloc_Array( self->ranges, RangeSet_Range, ++self->nRanges );
		}

		range = self->ranges + self->nRanges - 1;

		range->begin = tmpInds[curInd++];
		while( curInd < nInds && tmpInds[curInd] == tmpInds[curInd - 1] ) {
			curInd++;
			self->nInds--;
		}

		if( curInd == nInds ) {
			range->end = range->begin + 1;
			range->step = 1;
			break;
		}

		range->end = tmpInds[curInd++];
		while( curInd < nInds && tmpInds[curInd] == range->end ) {
			curInd++;
			self->nInds--;
		}

		range->step = range->end - range->begin;

		while( curInd < nInds && tmpInds[curInd] - range->end == range->step ) {
			range->end = tmpInds[curInd++];
			while( curInd < nInds && tmpInds[curInd] == range->end ) {
				curInd++;
				self->nInds--;
			}
		}

		range->end++;
	}

	FreeArray( tmpInds );
}


void RangeSet_Clear( void* rangeSet ) {
	RangeSet*	self = (RangeSet*)rangeSet;

	assert( self );

	RangeSet_Destruct( self );
}


void RangeSet_Union( void* rangeSet, RangeSet* rSet ) {
	RangeSet*	self = (RangeSet*)rangeSet;
	unsigned	maxInds;
	unsigned	nInds = 0;
	unsigned*	inds;
	unsigned	r_i;

	assert( self );
	assert( rSet );

	maxInds = rSet->nInds + self->nInds;
	inds = Memory_Alloc_Array_Unnamed( unsigned, maxInds );
	for( r_i = 0; r_i < self->nRanges; r_i++ ) {
		RangeSet_Range*	range = self->ranges + r_i;
		unsigned	ind_i;

		for( ind_i = range->begin; ind_i < range->end; ind_i += range->step )
			inds[nInds++] = ind_i;
	}
	for( r_i = 0; r_i < rSet->nRanges; r_i++ ) {
		RangeSet_Range*	range = rSet->ranges + r_i;
		unsigned	ind_i;

		for( ind_i = range->begin; ind_i < range->end; ind_i += range->step )
			inds[nInds++] = ind_i;
	}

	RangeSet_SetIndices( self, nInds, inds );
	FreeArray( inds );
}


void RangeSet_Intersection( void* rangeSet, RangeSet* rSet ) {
	RangeSet*	self = (RangeSet*)rangeSet;
	unsigned	maxInds;
	unsigned	nInds = 0;
	unsigned*	inds;
	unsigned	r_i;

	assert( self );
	assert( rSet );

	maxInds = (rSet->nInds > self->nInds) ? rSet->nInds : self->nInds;
	inds = Memory_Alloc_Array_Unnamed( unsigned, maxInds );

	for( r_i = 0; r_i < self->nRanges; r_i++ ) {
		RangeSet_Range*	range = self->ranges + r_i;
		unsigned	ind_i;

		for( ind_i = range->begin; ind_i < range->end; ind_i += range->step ) {
			if( RangeSet_HasIndex( rSet, ind_i ) )
				inds[nInds++] = ind_i;
		}
	}

	RangeSet_SetIndices( self, nInds, inds );
	FreeArray( inds );
}


void RangeSet_Subtraction( void* rangeSet, RangeSet* rSet ) {
	RangeSet*	self = (RangeSet*)rangeSet;
	unsigned	maxInds;
	unsigned	nInds = 0;
	unsigned*	inds;
	unsigned	r_i;

	assert( self );
	assert( rSet );

	maxInds = self->nInds;
	inds = Memory_Alloc_Array_Unnamed( unsigned, maxInds );

	for( r_i = 0; r_i < self->nRanges; r_i++ ) {
		RangeSet_Range*	range = self->ranges + r_i;
		unsigned	ind_i;

		for( ind_i = range->begin; ind_i < range->end; ind_i += range->step ) {
			if( !RangeSet_HasIndex( rSet, ind_i ) )
				inds[nInds++] = ind_i;
		}
	}

	RangeSet_SetIndices( self, nInds, inds );
	FreeArray( inds );
}


void RangeSet_Unpickle( void* rangeSet, unsigned nBytes, Stg_Byte* bytes ) {
	RangeSet*	self = (RangeSet*)rangeSet;

	assert( self );
	assert( (nBytes - sizeof(unsigned)) % sizeof(RangeSet_Range) == 0 );
	assert( !nBytes || bytes );

	RangeSet_Destruct( self );

	if( nBytes ) {
		self->nInds = ((unsigned*)bytes)[0];
		if( self->nInds ) {
			self->nRanges = (nBytes - sizeof(unsigned)) / sizeof(RangeSet_Range);
			self->ranges = Memory_Alloc_Array( RangeSet_Range, self->nRanges, "RangeSet::ranges" );
			memcpy( self->ranges, bytes + sizeof(unsigned), nBytes - sizeof(unsigned) );
		}
	}
}


void RangeSet_GetIndices( void* rangeSet, unsigned* nInds, unsigned** inds ) {
	RangeSet*	self = (RangeSet*)rangeSet;
	unsigned	r_i;

	assert( self );

	*nInds = 0;
	if( self->nInds )
		*inds = Memory_Alloc_Array_Unnamed( unsigned, self->nInds );
	else
		*inds = NULL;

	for( r_i = 0; r_i < self->nRanges; r_i++ ) {
		RangeSet_Range*	range = self->ranges + r_i;
		unsigned	ind_i;

		for( ind_i = range->begin; ind_i < range->end; ind_i += range->step )
			(*inds)[(*nInds)++] = ind_i;
	}

	/* Sanity check. */
	assert( *nInds == self->nInds );
}


Bool RangeSet_HasIndex( void* rangeSet, unsigned ind ) {
	RangeSet*	self = (RangeSet*)rangeSet;
	unsigned	r_i;

	assert( self );

	for( r_i = 0; r_i < self->nRanges; r_i++ ) {
		RangeSet_Range*	range = self->ranges + r_i;

		if( ind >= range->begin && ind < range->end ) {
			if( !((ind - range->begin) % range->step) )
				return True;
		}
		else if( ind < range->begin )
			break;
	}

	return False;
}


unsigned RangeSet_GetNIndices( void* rangeSet ) {
	RangeSet*	self = (RangeSet*)rangeSet;

	assert( self );

	return self->nInds;
}


unsigned RangeSet_GetNRanges( void* rangeSet ) {
	RangeSet*	self = (RangeSet*)rangeSet;

	assert( self );

	return self->nRanges;
}


void RangeSet_GetRange( void* rangeSet, unsigned ind, RangeSet_Range* range ) {
	RangeSet*	self = (RangeSet*)rangeSet;

	assert( self );
	assert( ind < self->nRanges );
	assert( range );

	memcpy( range, self->ranges + ind, sizeof(RangeSet_Range) );
}


void RangeSet_Pickle( void* rangeSet, unsigned* nBytes, Stg_Byte** bytes ) {
	RangeSet*	self = (RangeSet*)rangeSet;

	assert( self );
	assert( nBytes );
	assert( bytes );

	if( self->nInds ) {
		*nBytes = sizeof(unsigned) + self->nRanges * sizeof(RangeSet_Range);
		*bytes = Memory_Alloc_Array_Unnamed( Stg_Byte, *nBytes );
		((unsigned*)*bytes)[0] = self->nInds;
		if( self->nRanges )
			memcpy( *bytes + sizeof(unsigned), self->ranges, *nBytes - sizeof(unsigned) );
	}
	else {
		*nBytes = 0;
		*bytes = NULL;
	}
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

int RangeSet_SortCmp( const void* itema, const void* itemb ) {
	assert( itema && itemb );
	return *((unsigned*)itema) - *((unsigned*)itemb);
}


void RangeSet_Destruct( RangeSet* self ) {
	assert( self );

	KillArray( self->ranges );
	self->nRanges = 0;
	self->nInds = 0;
}

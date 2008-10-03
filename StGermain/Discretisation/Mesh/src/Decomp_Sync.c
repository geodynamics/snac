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
** $Id: Decomp_Sync.c 3584 2006-05-16 11:11:07Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <mpi.h>

#include "Base/Base.h"

#include "types.h"
#include "shortcuts.h"
#include "CommTopology.h"
#include "Decomp.h"
#include "Decomp_Sync_Claim.h"
#include "Decomp_Sync_Negotiate.h"
#include "Decomp_Sync.h"


/* Textual name of this class */
const Type Decomp_Sync_Type = "Decomp_Sync";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

Decomp_Sync* Decomp_Sync_New( Name name ) {
	return _Decomp_Sync_New( sizeof(Decomp_Sync), 
				 Decomp_Sync_Type, 
				 _Decomp_Sync_Delete, 
				 _Decomp_Sync_Print, 
				 _Decomp_Sync_Copy, 
				 (void* (*)(Name))_Decomp_Sync_New, 
				 _Decomp_Sync_Construct, 
				 _Decomp_Sync_Build, 
				 _Decomp_Sync_Initialise, 
				 _Decomp_Sync_Execute, 
				 _Decomp_Sync_Destroy, 
				 name, 
				 NON_GLOBAL );
}

Decomp_Sync* _Decomp_Sync_New( DECOMP_SYNC_DEFARGS ) {
	Decomp_Sync* self;
	
	/* Allocate memory */
	assert( sizeOfSelf >= sizeof(Decomp_Sync) );
	self = (Decomp_Sync*)_Stg_Component_New( STG_COMPONENT_PASSARGS );

	/* Virtual info */

	/* Decomp_Sync info */
	_Decomp_Sync_Init( self );

	return self;
}

void _Decomp_Sync_Init( Decomp_Sync* self ) {
	self->decomp = NULL;
	self->commTopo = NULL;

	self->claim = Decomp_Sync_Claim_New( "" );
	self->negotiate = Decomp_Sync_Negotiate_New( "" );

	self->nRemotes = 0;
	self->remotes = NULL;
	self->nShared = 0;
	self->shared = NULL;

	self->netSrcs = 0;
	self->nSrcs = NULL;
	self->srcs = NULL;
	self->netSnks = 0;
	self->nSnks = NULL;
	self->snks = NULL;

	self->nArrays = 0;
	self->arrays = NULL;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _Decomp_Sync_Delete( void* sync ) {
	Decomp_Sync*	self = (Decomp_Sync*)sync;

	Decomp_Sync_Destruct( self );
	FreeObject( self->claim );
	FreeObject( self->negotiate );

	/* Delete the parent. */
	_Stg_Component_Delete( self );
}

void _Decomp_Sync_Print( void* sync, Stream* stream ) {
	Decomp_Sync*	self = (Decomp_Sync*)sync;
	
	/* Set the Journal for printing informations */
	Stream* syncStream;
	syncStream = Journal_Register( InfoStream_Type, "Decomp_SyncStream" );

	/* Print parent */
	Journal_Printf( stream, "Decomp_Sync (ptr): (%p)\n", self );
	_Stg_Component_Print( self, stream );
}

void* _Decomp_Sync_Copy( void* sync, void* destProc_I, Bool deep, Name nameExt, PtrMap* ptrMap ) {
#if 0
	Decomp_Sync*	self = (Decomp_Sync*)sync;
	Decomp_Sync*	newDecomp_Sync;
	PtrMap*	map = ptrMap;
	Bool	ownMap = False;

	/* Damn me for making copying so difficult... what was I thinking? */
	
	/* We need to create a map if it doesn't already exist. */
	if( !map ) {
		map = PtrMap_New( 10 );
		ownMap = True;
	}
	
	newDecomp_Sync = (Decomp_Sync*)_Mesh_Copy( self, destProc_I, deep, nameExt, map );
	
	/* Copy the virtual methods here. */

	/* Deep or shallow? */
	if( deep ) {
	}
	else {
	}
	
	/* If we own the map, get rid of it here. */
	if( ownMap ) Stg_Class_Delete( map );
	
	return (void*)newDecomp_Sync;
#endif

	return NULL;
}

void _Decomp_Sync_Construct( void* sync, Stg_ComponentFactory* cf, void* data ) {
}

void _Decomp_Sync_Build( void* sync, void* data ) {
}

void _Decomp_Sync_Initialise( void* sync, void* data ) {
}

void _Decomp_Sync_Execute( void* sync, void* data ) {
}

void _Decomp_Sync_Destroy( void* sync, void* data ) {
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void Decomp_Sync_SetDecomp( void* sync, Decomp* decomp ) {
	Decomp_Sync*	self = (Decomp_Sync*)sync;

	assert( self );

	Decomp_Sync_Destruct( self );

	self->decomp = decomp;
	if( decomp )
		Decomp_AddSync( decomp, self );
}

void Decomp_Sync_SetClaim( void* sync, Decomp_Sync_Claim* claim ) {
	Decomp_Sync*	self = (Decomp_Sync*)sync;

	assert( self );

	Decomp_Sync_DestructRemotes( self );
	FreeObject( self->claim );

	if( claim )
		self->claim = claim;
	else
		self->claim = Decomp_Sync_Claim_New( "" );
}

void Decomp_Sync_SetNegotiate( void* sync, Decomp_Sync_Negotiate* negotiate ) {
	Decomp_Sync*	self = (Decomp_Sync*)sync;

	assert( self );

	Decomp_Sync_DestructRemotes( self );
	FreeObject( self->negotiate );

	if( negotiate )
		self->negotiate = negotiate;
	else
		self->negotiate = Decomp_Sync_Negotiate_New( "" );
}

void Decomp_Sync_SetRemotes( void* sync, unsigned nRemotes, unsigned* remotes ) {
	Decomp_Sync*	self = (Decomp_Sync*)sync;
	RangeSet*	lSet;
	RangeSet*	rSet;
	unsigned	nInds;
	unsigned*	inds;

	assert( self );
	assert( self->decomp );
	assert( !nRemotes || remotes );
#ifndef NDEBUG
	{
		lSet = RangeSet_New();
		rSet = RangeSet_New();
		RangeSet_SetIndices( lSet, self->decomp->nLocals, self->decomp->locals );
		RangeSet_SetIndices( rSet, nRemotes, remotes );
		RangeSet_Intersection( rSet, lSet );
		assert( !RangeSet_GetNIndices( rSet ) );
		FreeObject( lSet );
		FreeObject( rSet );
	}
#endif

	Decomp_Sync_DestructRemotes( self );

	/* Store remote values. */
	self->nRemotes = nRemotes;
	if( nRemotes ) {
		self->remotes = Memory_Alloc_Array( unsigned, nRemotes, "Decomp_Sync::remotes" );
		memcpy( self->remotes, remotes, nRemotes * sizeof(unsigned) );
	}
	else
		self->remotes = NULL;

	/* Build a communication topology and intersections. */
	lSet = RangeSet_New();
	rSet = RangeSet_New();
	RangeSet_SetIndices( lSet, self->decomp->nLocals, self->decomp->locals );
	RangeSet_SetIndices( rSet, nRemotes, remotes );
	RangeSet_Union( lSet, rSet );
	FreeObject( rSet );
	RangeSet_GetIndices( lSet, &nInds, &inds );
	FreeObject( lSet );
	Decomp_Sync_BuildIntersections( self, nInds, inds );
	FreeArray( inds );

	/* Negotiate sources and sinks. */
	Decomp_Sync_Negotiate_Select( self->negotiate, self );

	/* Build the global to remote map. */
	Decomp_Sync_BuildGRMap( self );

	/* Build shared information. */
	Decomp_Sync_BuildShared( self );
}

void Decomp_Sync_Decompose( void* sync, unsigned nRequired, unsigned* required ) {
	Decomp_Sync*	self = (Decomp_Sync*)sync;
	Decomp*		decomp;
	unsigned	nLocals;
	unsigned*	locals;

	assert( self );
	assert( self->decomp );
	assert( !nRequired || required );

	/* Destroy self. */
	decomp = self->decomp;
	Decomp_Sync_Destruct( self );
	self->decomp = decomp;

	/* Build index intersections and communication topology. */
	Decomp_Sync_BuildIntersections( self, nRequired, required );

	/* Claim ownership. */
	Decomp_Sync_Claim_Select( self->claim, self, nRequired, required, 
				  &nLocals, &locals );

	/* Reset the decomposition. */
	Decomp_SetLocals( decomp, nLocals, locals );
	self->decomp = decomp;
	Decomp_AddSync( decomp, self );
	Decomp_Sync_Negotiate_Select( self->negotiate, self );
	Decomp_Sync_BuildGRMap( self );
	Decomp_Sync_BuildShared( self );
}

unsigned Decomp_Sync_GetDomainSize( void* sync ) {
	Decomp_Sync*	self = (Decomp_Sync*)sync;

	assert( self );
	assert( self->decomp );

	return self->decomp->nLocals + self->nRemotes;
}

Bool Decomp_Sync_IsDomain( void* sync, unsigned global ) {
	Decomp_Sync*	self = (Decomp_Sync*)sync;

	assert( self );
	assert( self->decomp );
	assert( global < self->decomp->nGlobals );
	assert( self->decomp->glMap );
	assert( self->grMap );

	return Decomp_IsLocal( self->decomp, global ) || UIntMap_HasKey( self->grMap, global );
}

Bool Decomp_Sync_IsRemote( void* sync, unsigned domain ) {
	Decomp_Sync*	self = (Decomp_Sync*)sync;

	assert( self );
	assert( self->decomp );
	assert( domain < self->decomp->nLocals + self->nRemotes );

	return domain >= self->decomp->nLocals;
}

unsigned Decomp_Sync_GlobalToDomain( void* sync, unsigned global ) {
	Decomp_Sync*	self = (Decomp_Sync*)sync;

	assert( self );
	assert( self->decomp );
	assert( global < self->decomp->nGlobals );
	assert( self->decomp->glMap );
	assert( self->grMap );

	if( Decomp_IsLocal( self->decomp, global ) )
		return Decomp_GlobalToLocal( self->decomp, global );
	else
		return self->decomp->nLocals + UIntMap_Map( self->grMap, global );
}

unsigned Decomp_Sync_DomainToGlobal( void* sync, unsigned domain ) {
	Decomp_Sync*	self = (Decomp_Sync*)sync;

	assert( self );
	assert( self->decomp );
	assert( domain < self->decomp->nLocals + self->nRemotes );

	if( domain < self->decomp->nLocals )
		return Decomp_LocalToGlobal( self->decomp, domain );
	else
		return self->remotes[domain - self->decomp->nLocals];
}

unsigned Decomp_Sync_DomainToShared( void* sync, unsigned domain ) {
	Decomp_Sync*	self = (Decomp_Sync*)sync;

	assert( self );
	assert( self->decomp );
	assert( domain < self->decomp->nLocals + self->nRemotes );
	assert( self->dsMap );

	return UIntMap_Map( self->dsMap, domain );
}

unsigned Decomp_Sync_SharedToDomain( void* sync, unsigned shared ) {
	Decomp_Sync*	self = (Decomp_Sync*)sync;

	assert( self );
	assert( shared < self->nShared );
	assert( self->shared );

	return self->shared[shared];
}

Decomp_Sync_Array* Decomp_Sync_AddArray( void* sync, void* localArray, void* remoteArray, 
					 size_t localStride, size_t remoteStride, size_t itemSize )
{
	Decomp_Sync*		self = (Decomp_Sync*)sync;
	Decomp_Sync_Array*	array;

	/* Sanity checks. */
	assert( self );
	assert( self->decomp );
	assert( !self->decomp->nLocals || localArray );
	assert( !self->nRemotes || remoteArray );
	assert( itemSize );

	/* Resize the array array (?). */
	if( self->nArrays ) {
		self->arrays = Memory_Realloc_Array( self->arrays, Decomp_Sync_Array*, ++self->nArrays );
	}
	else {
		self->arrays = Memory_Alloc_Array( Decomp_Sync_Array*, ++self->nArrays, "Decomp_Sync::Arrays" );
	}
	self->arrays[self->nArrays - 1] = Memory_Alloc_Array_Unnamed( Decomp_Sync_Array, 1 );
	array = self->arrays[self->nArrays - 1];

	/* Store information. */
	array->snkArray = localArray;
	array->snkStride = localStride;
	array->srcArray = remoteArray;
	array->srcStride = remoteStride;
	array->itemSize = itemSize;

	/* Build this array. */
	Decomp_Sync_BuildArray( self, array );

	return array;
}

void Decomp_Sync_RemoveArray( void* sync, Decomp_Sync_Array* array ) {
	Decomp_Sync*	self = (Decomp_Sync*)sync;
	unsigned	a_i;

	assert( self );

	a_i = 0;
	while( self->arrays[a_i++] != array && a_i < self->nArrays );
	assert( a_i <= self->nArrays );
	for( ; a_i < self->nArrays; a_i++ )
		self->arrays[a_i - 1] = self->arrays[a_i];

	if( --self->nArrays == 0 ) {
		KillArray( self->arrays );
	}
	else
		self->arrays = Memory_Realloc_Array( self->arrays, Decomp_Sync_Array*, self->nArrays );

	Decomp_Sync_DestructArray( array );
}

void Decomp_Sync_Sync( void* sync ) {
	Decomp_Sync*	self = (Decomp_Sync*)sync;
	unsigned	a_i;

	/* Sanity checks. */
	assert( self );
	assert( !self->nArrays || self->arrays );

	for( a_i = 0; a_i < self->nArrays; a_i++ )
		Decomp_Sync_SyncArray( self, self->arrays[a_i] );
}

void Decomp_Sync_SyncArray( void* sync, Decomp_Sync_Array* array ) {
	Decomp_Sync*	self = (Decomp_Sync*)sync;
	CommTopology*	commTopo;
	unsigned	rank;
	unsigned	nInc;
	unsigned*	inc;
	Stg_Byte*	snkArray;
	Stg_Byte*	srcArray;
	unsigned	p_i;

	/* Sanity checks. */
	assert( self );
	assert( self->commTopo );
	assert( array );

	/* Shortcuts. */
	commTopo = self->commTopo;
	MPI_Comm_rank( commTopo->comm, (int*)&rank );

	/* Pack from locals to a contiguous array. */
	if( self->netSnks > 0 ) {
		unsigned	snk_i;

		snkArray = Memory_Alloc_Array_Unnamed( Stg_Byte, self->netSnks * array->itemSize );
		for( snk_i = 0; snk_i < self->netSnks; snk_i++ ) {
			memcpy( snkArray + snk_i * array->itemSize, 
				(Stg_Byte*)array->snkArray + array->snkOffs[snk_i], 
				array->itemSize );
		}
	}
	else
		snkArray = NULL;

	/* Allocate for sources. */
	if( self->netSrcs > 0 )
		srcArray = Memory_Alloc_Array_Unnamed( Stg_Byte, self->netSrcs * array->itemSize );
	else
		srcArray = NULL;

	/* Get incidence. */
	CommTopology_GetIncidence( commTopo, rank, &nInc, &inc );

	/* Transfer. */
	for( p_i = 0; p_i < nInc; p_i++ ) {
		int		snkSize = array->snkSizes[p_i];
		int		snkDisp = array->snkDisps[p_i];
		int		srcSize = array->srcSizes[p_i];
		int		srcDisp = array->srcDisps[p_i];
		MPI_Status	status;
		unsigned	tag = 6669;

		MPI_Sendrecv( snkArray + snkDisp, snkSize, MPI_BYTE, inc[p_i], tag, 
			      srcArray + srcDisp, srcSize, MPI_BYTE, inc[p_i], tag, 
			      commTopo->comm, &status );
	}

	/* Return incidence. */
	CommTopology_ReturnIncidence( commTopo, rank, &nInc, &inc );

	/* Free the sink array. */
	FreeArray( snkArray );

	/* Unpack sources. */
	if( self->netSnks > 0 ) {
		unsigned	src_i;

		for( src_i = 0; src_i < self->netSrcs; src_i++ ) {
			memcpy( (Stg_Byte*)array->srcArray + array->srcOffs[src_i], 
				srcArray + src_i * array->itemSize, 
				array->itemSize );
		}
	}

	/* Free source array. */
	FreeArray( srcArray );
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

void Decomp_Sync_BuildIntersections( Decomp_Sync* self, unsigned nIndices, unsigned* indices ) {
	unsigned	rank, nProcs;
	RangeSet*	lSet;
	MPI_Group	worldGroup;
	unsigned*	worldRanks;
	unsigned*	subRanks;
	unsigned	nCommInc;
	unsigned*	commInc;
	RangeSet**	iSets;
	unsigned	nInc;
	unsigned*	inc;
	unsigned	p_i;

	assert( self );
	assert( !nIndices || indices );

	/* Get basic MPI info. */
	MPI_Comm_rank( self->decomp->comm, (int*)&rank );
	MPI_Comm_size( self->decomp->comm, (int*)&nProcs );

	/* We'll need to modify the world group. */
	MPI_Comm_group( self->decomp->comm, &worldGroup );
	worldRanks = Memory_Alloc_Array_Unnamed( unsigned, nProcs );
	subRanks = Memory_Alloc_Array_Unnamed( unsigned, nProcs );

	/* We need space to store index intersections. */
	iSets = Memory_Alloc_Array_Unnamed( RangeSet*, nProcs );
	memset( iSets, 0, nProcs * sizeof(RangeSet*) );

	/* Create a local set of required indices. */
	lSet = RangeSet_New();
	RangeSet_SetIndices( lSet, nIndices, indices );

	/* Tackle each processor one at a time. */
	for( p_i = 0; p_i < nProcs - 1; p_i++ ) {
		int		groupRange[3];
		MPI_Group	subGroup;
		MPI_Comm	subComm;
		unsigned	p_j;

		/* Set the processor range. */
		groupRange[0] = p_i;
		groupRange[1] = nProcs - 1;
		groupRange[2] = 1;

		/* We'll need a new group, as we only want to communicate using a triangular scheme. */
		MPI_Group_range_incl( worldGroup, 1, &groupRange, &subGroup );
		MPI_Comm_create( self->decomp->comm, subGroup, &subComm );

		/* Only continue if we're part of the sub-communicator. */
		if( rank >= p_i ) {
			unsigned	nBytes;
			Stg_Byte*	bytes;
			unsigned*	nFounds;
			Stg_Byte**	founds;

			/* Create a mapping between ranks. */
			for( p_j = p_i; p_j < nProcs; p_j++ )
				subRanks[p_j] = p_j - p_i;
			MPI_Group_translate_ranks( subGroup, nProcs - p_i, (int*)(subRanks + p_i), 
						   worldGroup, (int*)worldRanks );

			if( p_i == rank )
				RangeSet_Pickle( lSet, &nBytes, &bytes );

			MPIArray_Bcast( &nBytes, (void**)&bytes, sizeof(Stg_Byte), subRanks[p_i], subComm );

			if( p_i != rank ) {
				/* Create the intersection. */
				iSets[p_i] = RangeSet_New();
				RangeSet_Unpickle( iSets[p_i], nBytes, bytes );
				RangeSet_Intersection( iSets[p_i], lSet );

				/* Pickle the intersection to send back. */
				FreeArray( bytes );
				RangeSet_Pickle( iSets[p_i], &nBytes, &bytes );
			}
			else {
				KillArray( bytes );
				nBytes = 0;
			}

			/* Retrieve the results and unpickle each of them. */
			MPIArray_Gather( nBytes, bytes, &nFounds, (void***)&founds, sizeof(Stg_Byte), 
					 subRanks[p_i], subComm );
			if( p_i == rank ) {
				for( p_j = 0; p_j < nProcs - p_i; p_j++ ) {
					if( !nFounds[p_j] )
						continue;

					iSets[worldRanks[p_j]] = RangeSet_New();
					RangeSet_Unpickle( iSets[worldRanks[p_j]], nFounds[p_j], founds[p_j] );
				}

				/* Free the found arrays. */
				FreeArray( nFounds );
				FreeArray( founds );
			}
			else {
				/* Free pickled range set. */
				FreeArray( bytes );
			}

			/* Destroy the sub-communicator. */
			MPI_Comm_free( &subComm );
		}

		/* Destroy the sub-group. */
		MPI_Group_free( &subGroup );
	}

	/* Free rank translation arrays and local range set. */
	FreeArray( worldRanks );
	FreeArray( subRanks );

	/* Build a set of communication incidence. */
	nCommInc = 0;
	commInc = Memory_Alloc_Array_Unnamed( unsigned, nProcs );
	for( p_i = 0; p_i < nProcs; p_i++ ) {
		if( iSets[p_i] && iSets[p_i]->nInds )
			commInc[nCommInc++] = p_i;
	}

	/* Create the communication topology. */
	self->commTopo = CommTopology_New( "Decomp_Sync::commTopo" );
	CommTopology_SetComm( self->commTopo, self->decomp->comm );
	CommTopology_SetIncidence( self->commTopo, nCommInc, commInc );
	FreeArray( commInc );

	/* Build final intersections. */
	CommTopology_GetIncidence( self->commTopo, rank, &nInc, &inc );
	if( nInc ) {
		unsigned	inc_i;

		self->isects = Memory_Alloc_Array_Unnamed( RangeSet*, nInc );
		for( inc_i = 0; inc_i < nInc; inc_i++ )
			self->isects[inc_i] = iSets[inc[inc_i]];
	}
	else
		self->isects = NULL;
	CommTopology_ReturnIncidence( self->commTopo, rank, &nInc, &inc );

	/* Free intersection array. */
	FreeArray( iSets );
}

void Decomp_Sync_BuildShared( Decomp_Sync* self ) {
	unsigned	rank;
	RangeSet*	sharedSet;
	unsigned	nInc;
	unsigned*	inc;
	unsigned**	sharers;
	unsigned	s_i, p_i;

	assert( self );
	assert( self->commTopo );

	/* Get basic MPI info. */
	MPI_Comm_rank( self->decomp->comm, (int*)&rank );

	/* Get incidence. */
	CommTopology_GetIncidence( self->commTopo, rank, &nInc, &inc );

	if( nInc ) {
		assert( self->isects );

		/* Take the union of all intersections to determine how many shared indices we have. */
		sharedSet = RangeSet_New();
		for( p_i = 0; p_i < nInc; p_i++ )
			RangeSet_Union( sharedSet, self->isects[p_i] );

		/* Build table and map. */
		RangeSet_GetIndices( sharedSet, &self->nShared, &self->shared );
		self->dsMap = UIntMap_New();
		for( s_i = 0; s_i < self->nShared; s_i++ ) {
			self->shared[s_i] = Decomp_Sync_GlobalToDomain( self, self->shared[s_i] );
			UIntMap_Insert( self->dsMap, self->shared[s_i], s_i );
		}
		FreeObject( sharedSet );

		/* Allocate enough space to hold maximum results. */
		self->nSharers = Memory_Alloc_Array( unsigned, self->nShared, "Decomp_Sync::nSharers" );
		sharers = Memory_Alloc_2DArray_Unnamed( unsigned, self->nShared, nInc );
		memset( self->nSharers, 0, self->nShared * sizeof(unsigned) );

		/* Collect processors. */
		for( p_i = 0; p_i < nInc; p_i++ ) {
			unsigned	nInds;
			unsigned*	inds;
			unsigned	ind_i;

			RangeSet_GetIndices( self->isects[p_i], &nInds, &inds );
			for( ind_i = 0; ind_i < nInds; ind_i++ ) {
				unsigned	dInd = Decomp_Sync_GlobalToDomain( self, inds[ind_i] );
				unsigned       	sInd = Decomp_Sync_DomainToShared( self, dInd );

				sharers[sInd][self->nSharers[sInd]++] = p_i;
			}
			FreeArray( inds );
		}

		/* Transfer results to self. */
		self->sharers = Memory_Alloc_2DComplex( unsigned, self->nShared, self->nSharers, "Decomp_Sync::sharers" );
		for( s_i = 0; s_i < self->nShared; s_i++ )
			memcpy( self->sharers[s_i], sharers[s_i], self->nSharers[s_i] * sizeof(unsigned) );

		/* Cleanup. */
		FreeArray( sharers );
	}

	/* Return incidence. */
	CommTopology_ReturnIncidence( self->commTopo, rank, &nInc, &inc );
}

void Decomp_Sync_BuildGRMap( Decomp_Sync* self ) {
	UIntMap*	map;
	unsigned	r_i;

	FreeObject( self->grMap );

	map = UIntMap_New();
	for( r_i = 0; r_i < self->nRemotes; r_i++ )
		UIntMap_Insert( map, self->remotes[r_i], r_i );
	self->grMap = map;
}

void Decomp_Sync_BuildArray( Decomp_Sync* self, Decomp_Sync_Array* array ) {
	CommTopology*	commTopo;
	Decomp*		decomp;
	unsigned	rank;
	unsigned	nInc;
	unsigned*	inc;

	assert( self );
	assert( self->commTopo );

	/* Shortcuts. */
	decomp = self->decomp;
	commTopo = self->commTopo;
	MPI_Comm_rank( commTopo->comm, (int*)&rank );

	/* Extract incidence. */
	CommTopology_GetIncidence( commTopo, rank, &nInc, &inc );

	if( nInc ) {
		/* Determine sink (local) information. */
		if( self->netSnks > 0 ) {
			unsigned*	snkOffs;
			unsigned*	snkSizes;
			unsigned*	snkDisps;
			unsigned	snkInd = 0;
			unsigned	p_i;

			/* Allocate/reallocate memory. */
			snkDisps = Memory_Alloc_Array( unsigned, nInc, "Decomp_Sync_Array::snkDisps" );
			snkSizes = Memory_Alloc_Array( unsigned, nInc, "Decomp_Sync_Array::snkSizes" );
			snkOffs = Memory_Alloc_Array( unsigned, self->netSnks, "Decomp_Sync_Array::snkOffs" );

			/* Calculate offsets and sizes. */
			for( p_i = 0; p_i < nInc; p_i++ ) {
				unsigned	snk_i;

				snkSizes[p_i] = 0;
				for( snk_i = 0; snk_i < self->nSnks[p_i]; snk_i++ ) {
					unsigned	dInd = Decomp_Sync_GlobalToDomain( self, self->snks[p_i][snk_i] );

					snkOffs[snkInd] = dInd * array->snkStride;
					snkSizes[p_i] += array->itemSize;
					snkInd++;
				}
			}

			/* Calculate the displacements. */
			snkDisps[0] = 0;
			for( p_i = 1; p_i < nInc; p_i++ )
				snkDisps[p_i] = snkDisps[p_i - 1] + snkSizes[p_i - 1];

			/* Store arrays. */
			array->snkOffs = snkOffs;
			array->snkDisps = snkDisps;
			array->snkSizes = snkSizes;
		}
		else {
			/* Store null information. */
			array->snkOffs = NULL;
			array->snkDisps = Memory_Alloc_Array( unsigned, nInc, "Decomp_Sync_Array::snkDisps" );
			array->snkSizes = Memory_Alloc_Array( unsigned, nInc, "Decomp_Sync_Array::snkSizes" );
			memset( array->snkDisps, 0, nInc * sizeof(unsigned) );
			memset( array->snkSizes, 0, nInc * sizeof(unsigned) );
		}

		/* Determine source (shadow) information. */
		if( self->netSrcs > 0 ) {
			unsigned*	srcOffs;
			unsigned*	srcSizes;
			unsigned*	srcDisps;
			unsigned	srcInd = 0;
			unsigned	p_i;

			/* Allocate/reallocate memory. */
			srcDisps = Memory_Alloc_Array( unsigned, nInc, "Decomp_Sync_Array::srcDisps" );
			srcSizes = Memory_Alloc_Array( unsigned, nInc, "Decomp_Sync_Array::srcSizes" );
			srcOffs = Memory_Alloc_Array( unsigned, self->netSrcs, "Decomp_Sync_Array::srcOffs" );

			/* Calculate offsets and sizes. */
			for( p_i = 0; p_i < nInc; p_i++ ) {
				unsigned	src_i;

				srcSizes[p_i] = 0;
				for( src_i = 0; src_i < self->nSrcs[p_i]; src_i++ ) {
					unsigned	sInd = Decomp_Sync_GlobalToDomain( self, self->srcs[p_i][src_i] );

					assert( sInd >= decomp->nLocals );
					sInd -= decomp->nLocals;
					srcOffs[srcInd] = sInd * array->srcStride;
					srcSizes[p_i] += array->itemSize;
					srcInd++;
				}
			}

			/* Calculate the displacements. */
			srcDisps[0] = 0;
			for( p_i = 1; p_i < nInc; p_i++ )
				srcDisps[p_i] = srcDisps[p_i - 1] + srcSizes[p_i - 1];

			/* Store arrays. */
			array->srcOffs = srcOffs;
			array->srcDisps = srcDisps;
			array->srcSizes = srcSizes;
		}
		else {
			/* Store null information. */
			array->srcOffs = NULL;
			array->srcDisps = Memory_Alloc_Array( unsigned, nInc, "Decomp_Sync_Array::srcDisps" );
			array->srcSizes = Memory_Alloc_Array( unsigned, nInc, "Decomp_Sync_Array::srcSizes" );
			memset( array->srcDisps, 0, nInc * sizeof(unsigned) );
			memset( array->srcSizes, 0, nInc * sizeof(unsigned) );
		}
	}
	else {
		array->snkOffs = NULL;
		array->snkDisps = NULL;
		array->snkSizes = NULL;
		array->srcOffs = NULL;
		array->srcDisps = NULL;
		array->srcSizes = NULL;
	}

	/* Return incidence. */
	CommTopology_ReturnIncidence( commTopo, rank, &nInc, &inc );
}

void Decomp_Sync_Destruct( Decomp_Sync* self ) {
	Decomp_Sync_DestructRemotes( self );

	if( self->decomp ) {
		Decomp_RemoveSync( self->decomp, self );
		self->decomp = NULL;
	}
}

void Decomp_Sync_DestructRemotes( Decomp_Sync* self ) {
	Decomp_Sync_DestructArrays( self );

	if( self->commTopo ) {
		unsigned	p_i;

		for( p_i = 0; p_i < self->commTopo->nInc; p_i++ )
			FreeObject( self->isects[p_i] );
	}
	KillArray( self->isects );
	KillObject( self->commTopo );

	KillArray( self->remotes );
	self->nRemotes = 0;
	KillArray( self->shared );
	self->nShared = 0;
	KillArray( self->nSharers );
	KillArray( self->sharers );

	KillObject( self->grMap );
	KillObject( self->dsMap );

	self->netSrcs = 0;
	KillArray( self->nSrcs );
	KillArray( self->srcs );
	self->netSnks = 0;
	KillArray( self->nSnks );
	KillArray( self->snks );
}

void Decomp_Sync_DestructArrays( Decomp_Sync* self ) {
	unsigned	a_i;

	for( a_i = 0; a_i < self->nArrays; a_i++ )
		Decomp_Sync_DestructArray( self->arrays[a_i] );
	KillArray( self->arrays );
	self->nArrays = 0;
}

void Decomp_Sync_DestructArray( Decomp_Sync_Array* array ) {
	FreeArray( array->snkDisps );
	FreeArray( array->snkSizes );
	FreeArray( array->snkOffs );
	FreeArray( array->srcDisps );
	FreeArray( array->srcSizes );
	FreeArray( array->srcOffs );
	FreeArray( array );
}

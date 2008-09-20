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
** $Id: DecompTransfer.c 3584 2006-05-16 11:11:07Z PatrickSunter $
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
#include "DecompTransfer.h"


/* Textual name of this class */
const Type DecompTransfer_Type = "DecompTransfer";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

DecompTransfer* DecompTransfer_New( Name name ) {
	return _DecompTransfer_New( sizeof(DecompTransfer), 
				    DecompTransfer_Type, 
				    _DecompTransfer_Delete, 
				    _DecompTransfer_Print, 
				    NULL, 
				    (void* (*)(Name))_DecompTransfer_New, 
				    _DecompTransfer_Construct, 
				    _DecompTransfer_Build, 
				    _DecompTransfer_Initialise, 
				    _DecompTransfer_Execute, 
				    _DecompTransfer_Destroy, 
				    name, 
				    NON_GLOBAL );
}

DecompTransfer* _DecompTransfer_New( DECOMPTRANSFER_DEFARGS ) {
	DecompTransfer* self;
	
	/* Allocate memory */
	assert( sizeOfSelf >= sizeof(DecompTransfer) );
	self = (DecompTransfer*)_Stg_Component_New( STG_COMPONENT_PASSARGS );

	/* Virtual info */

	/* DecompTransfer info */
	_DecompTransfer_Init( self );

	return self;
}

void _DecompTransfer_Init( DecompTransfer* self ) {
	self->decomps[0] = NULL;
	self->decomps[1] = NULL;
	self->nIncIndices = 0;
	self->incIndices[0] = NULL;
	self->incIndices[1] = NULL;
	self->commTopo = NULL;

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

void _DecompTransfer_Delete( void* decompTransfer ) {
	DecompTransfer*	self = (DecompTransfer*)decompTransfer;

	DecompTransfer_Destruct( self );

	/* Delete the parent. */
	_Stg_Component_Delete( self );
}

void _DecompTransfer_Print( void* decompTransfer, Stream* stream ) {
	DecompTransfer*	self = (DecompTransfer*)decompTransfer;
	
	/* Set the Journal for printing informations */
	Stream* decompTransferStream;
	decompTransferStream = Journal_Register( InfoStream_Type, "DecompTransferStream" );

	/* Print parent */
	Journal_Printf( stream, "DecompTransfer (ptr): (%p)\n", self );
	_Stg_Component_Print( self, stream );
}

void _DecompTransfer_Construct( void* decompTransfer, Stg_ComponentFactory* cf, void* data ) {
}

void _DecompTransfer_Build( void* decompTransfer, void* data ) {
}

void _DecompTransfer_Initialise( void* decompTransfer, void* data ) {
}

void _DecompTransfer_Execute( void* decompTransfer, void* data ) {
}

void _DecompTransfer_Destroy( void* decompTransfer, void* data ) {
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void DecompTransfer_SetDecomps( void* decompTransfer, Decomp* fromDecomp, Decomp* toDecomp, UIntMap* mapping ) {
	DecompTransfer*	self = (DecompTransfer*)decompTransfer;

	assert( self );

	DecompTransfer_Destruct( self );

	self->decomps[0] = fromDecomp;
	self->decomps[1] = toDecomp;
	UIntMap_GetItems( mapping, &self->nIncIndices, &self->incIndices[0], &self->incIndices[1] );

	DecompTransfer_BuildTables( self );
}

DecompTransfer_Array* DecompTransfer_AddArray( void* decompTransfer, void* localArray, void* remoteArray, 
					       size_t localStride, size_t remoteStride, size_t itemSize )
{
	DecompTransfer*		self = (DecompTransfer*)decompTransfer;
	DecompTransfer_Array*	array;

	/* Sanity checks. */
	assert( self );
	assert( self->decomps[0] );
	assert( self->decomps[1] );
	assert( !self->decomps[0]->nLocals || localArray );
	assert( !self->decomps[1]->nLocals || remoteArray );
	assert( itemSize );

	/* Resize the array array (?). */
	if( self->nArrays ) {
		self->arrays = Memory_Realloc_Array( self->arrays, DecompTransfer_Array*, ++self->nArrays );
	}
	else {
		self->arrays = Memory_Alloc_Array( DecompTransfer_Array*, ++self->nArrays, "DecompTransfer::Arrays" );
	}
	self->arrays[self->nArrays - 1] = Memory_Alloc_Array_Unnamed( DecompTransfer_Array, 1 );
	array = self->arrays[self->nArrays - 1];

	/* Store information. */
	array->snkArray = localArray;
	array->snkStride = localStride;
	array->srcArray = remoteArray;
	array->srcStride = remoteStride;
	array->itemSize = itemSize;

	/* Build this array. */
	DecompTransfer_BuildArray( self, array );

	return array;
}

void DecompTransfer_RemoveArray( void* decompTransfer, DecompTransfer_Array* array ) {
	DecompTransfer*	self = (DecompTransfer*)decompTransfer;
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
		self->arrays = Memory_Realloc_Array( self->arrays, DecompTransfer_Array*, self->nArrays );

	DecompTransfer_DestructArray( array );
}

void DecompTransfer_Transfer( void* decompTransfer ) {
	DecompTransfer*	self = (DecompTransfer*)decompTransfer;
	unsigned	a_i;

	/* Sanity checks. */
	assert( self );
	assert( !self->nArrays || self->arrays );

	for( a_i = 0; a_i < self->nArrays; a_i++ )
		DecompTransfer_TransferArray( self, self->arrays[a_i] );
}

void DecompTransfer_TransferArray( void* decompTransfer, DecompTransfer_Array* array ) {
	DecompTransfer*	self = (DecompTransfer*)decompTransfer;
	CommTopology*	commTopo;
	unsigned	rank;
	unsigned	nInc;
	unsigned*	inc;
	Stg_Byte*	snkArray;
	Stg_Byte*	srcArray;
	unsigned	p_i, ind_i;

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

	/*
	** Copy local information.
	*/

	for( ind_i = 0; ind_i < self->nLocalInds; ind_i++ ) {
		memcpy( (Stg_Byte*)array->srcArray + self->dstLocalInds[ind_i] * array->srcStride, 
			(Stg_Byte*)array->snkArray + self->srcLocalInds[ind_i] * array->snkStride, 
			array->itemSize );
	}
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

void DecompTransfer_BuildTables( DecompTransfer* self ) {
	MPI_Comm	comm;
	unsigned	nProcs, rank;
	UIntMap*	invMap;
	RangeSet*	lSet;
	unsigned	nRemoteInds;
	unsigned*	srcRemoteInds;
	unsigned*	dstRemoteInds;
	RangeSet*	rSet;
	unsigned	nSrcBytes;
	Stg_Byte*	srcBytes;
	unsigned*	nDstBytes;
	Stg_Byte**	dstBytes;
	RangeSet**	fndSets;
	unsigned*	nFndBytes;
	Stg_Byte**	fndBytes;
	RangeSet**	retSets;
	unsigned*	nRetBytes;
	Stg_Byte**	retBytes;
	unsigned	nIncProcs;
	unsigned*	incProcs;
	unsigned	ind_i, p_i;

	assert( self );

	/*
	** Shortcuts.
	*/

	comm = self->decomps[0]->comm;
	MPI_Comm_size( comm, (int*)&nProcs );
	MPI_Comm_rank( comm, (int*)&rank );

	/*
	** Build an inverse inter-decomp mapping.
	*/

	invMap = UIntMap_New();
	for( ind_i = 0; ind_i < self->nIncIndices; ind_i++ )
		UIntMap_Insert( invMap, self->incIndices[1][ind_i], self->incIndices[0][ind_i] );

	/*
	** Separate the inter-mesh incidence into local and remote sets.
	*/

	self->nLocalInds = 0;
	nRemoteInds = 0;
	self->srcLocalInds = Memory_Alloc_Array( unsigned, self->nIncIndices, "DecompTransfer::srcLocalInds" );
	self->dstLocalInds = Memory_Alloc_Array( unsigned, self->nIncIndices, "DecompTransfer::dstLocalInds" );
	srcRemoteInds = Memory_Alloc_Array_Unnamed( unsigned, self->nIncIndices );
	dstRemoteInds = Memory_Alloc_Array_Unnamed( unsigned, self->nIncIndices );
	for( ind_i = 0; ind_i < self->nIncIndices; ind_i++ ) {
		unsigned	mappedInd;

		if( Decomp_IsLocal( self->decomps[1], self->incIndices[1][ind_i] ) ) {
			mappedInd = Decomp_GlobalToLocal( self->decomps[1], self->incIndices[1][ind_i] );
			self->srcLocalInds[self->nLocalInds] = self->incIndices[0][ind_i];
			self->dstLocalInds[self->nLocalInds++] = mappedInd;
		}
		else {
			srcRemoteInds[nRemoteInds] = self->incIndices[0][ind_i];
			dstRemoteInds[nRemoteInds++] = self->incIndices[1][ind_i];
		}
	}
	self->srcLocalInds = Memory_Realloc_Array( self->srcLocalInds, unsigned, self->nLocalInds );
	self->dstLocalInds = Memory_Realloc_Array( self->dstLocalInds, unsigned, self->nLocalInds );
	srcRemoteInds = Memory_Realloc_Array( srcRemoteInds, unsigned, nRemoteInds );
	dstRemoteInds = Memory_Realloc_Array( dstRemoteInds, unsigned, nRemoteInds );

	/*
	** Package up the remote indices and send them off to our neighbours.
	*/

	rSet = RangeSet_New();
	RangeSet_SetIndices( rSet, nRemoteInds, dstRemoteInds );
	FreeArray( dstRemoteInds );
	RangeSet_Pickle( rSet, &nSrcBytes, &srcBytes );
	MPIArray_Allgather( nSrcBytes, srcBytes, &nDstBytes, (void***)&dstBytes, sizeof(unsigned), comm );
	FreeArray( srcBytes );

	/*
	** Determine which of the other processors' remote indices we need.
	*/

	lSet = RangeSet_New();
	RangeSet_SetIndices( lSet, self->decomps[1]->nLocals, self->decomps[1]->locals );
	fndSets = Memory_Alloc_Array_Unnamed( RangeSet*, nProcs );
	nFndBytes = Memory_Alloc_Array_Unnamed( unsigned, nProcs );
	fndBytes = Memory_Alloc_Array_Unnamed( Stg_Byte*, nProcs );
	for( p_i = 0; p_i < nProcs; p_i++ ) {
		if( p_i == rank || !nDstBytes[p_i] ) {
			fndSets[p_i] = NULL;
			nFndBytes[p_i] = 0;
			fndBytes[p_i] = NULL;
			continue;
		}

		fndSets[p_i] = RangeSet_New();
		RangeSet_Unpickle( rSet, nDstBytes[p_i], dstBytes[p_i] );
		RangeSet_Union( rSet, lSet );

		if( !fndSets[p_i]->nInds ) {
			KillObject( fndSets[p_i] );
			nFndBytes[p_i] = 0;
			fndBytes[p_i] = NULL;
			continue;
		}

		RangeSet_Pickle( rSet, nFndBytes + p_i, fndBytes + p_i );
	}
	FreeArray( dstBytes );
	FreeArray( nDstBytes );
	FreeObject( lSet );
	FreeObject( rSet );

	/*
	** Send our findings back to the appropriate processors.
	*/

	retSets = Memory_Alloc_Array_Unnamed( RangeSet*, nProcs );
	MPIArray2D_Alltoall( nFndBytes, (void**)fndBytes, &nRetBytes, (void***)&retBytes, sizeof(unsigned), comm );
	for( p_i = 0; p_i < nProcs; p_i++ ) {
		if( p_i == rank || !nRetBytes[p_i] ) {
			retSets[p_i] = NULL;
			continue;
		}

		retSets[p_i] = RangeSet_New();
		RangeSet_Unpickle( retSets[p_i], nRetBytes[p_i], retBytes[p_i] );
		FreeArray( fndBytes[p_i] );
	}
	FreeArray( nFndBytes );
	FreeArray( fndBytes );
	FreeArray( nRetBytes );
	FreeArray( retBytes );

	/*
	** Build our communication topology.
	*/

	nIncProcs = 0;
	incProcs = Memory_Alloc_Array_Unnamed( unsigned, nProcs );
	for( p_i = 0; p_i < nProcs; p_i++ ) {
		if( fndSets[p_i] || retSets[p_i] )
			incProcs[nIncProcs++] = p_i;
	}
	self->commTopo = CommTopology_New( "" );
	CommTopology_SetComm( self->commTopo, comm );
	CommTopology_SetIncidence( self->commTopo, nIncProcs, incProcs );
	FreeArray( incProcs );

	/*
	** Dump range sets into source and sink arrays.
	*/

	if( self->commTopo->nInc ) {
	self->nSnks = Memory_Alloc_Array( unsigned, self->commTopo->nInc, "DecompTransfer::nSnks" );
	self->nSrcs = Memory_Alloc_Array( unsigned, self->commTopo->nInc, "DecompTransfer::nSrcs" );
	self->snks = Memory_Alloc_Array( unsigned*, self->commTopo->nInc, "DecompTransfer::nSnks" );
	self->srcs = Memory_Alloc_Array( unsigned*, self->commTopo->nInc, "DecompTransfer::nSrcs" );
	for( p_i = 0; p_i < nProcs; p_i++ ) {
		unsigned	incRank;

		incRank = self->commTopo->inc[p_i];
		if( retSets[incRank] )
			RangeSet_GetIndices( retSets[incRank], self->nSnks + p_i, self->snks + p_i );
		else {
			self->nSnks[p_i] = 0;
			self->snks[p_i] = NULL;
		}

		if( fndSets[incRank] )
			RangeSet_GetIndices( fndSets[incRank], self->nSrcs + p_i, self->srcs + p_i );
		else {
			self->nSrcs[p_i] = 0;
			self->srcs[p_i] = NULL;
		}

		FreeObject( retSets[incRank] );
		FreeObject( fndSets[incRank] );
	}
	FreeArray( retSets );
	FreeArray( fndSets );
	}
	else {
		self->nSnks = NULL;
		self->nSrcs = NULL;
		self->snks = NULL;
		self->srcs = NULL;
	}

	/*
	** Map the dumped indices to the source decomp and count totals.
	*/

	self->netSnks = 0;
	self->netSrcs = 0;
	for( p_i = 0; p_i < self->commTopo->nInc; p_i++ ) {
		self->netSnks += self->nSnks[p_i];
		for( ind_i = 0; ind_i < self->nSnks[p_i]; ind_i++ ) {
			self->snks[p_i][ind_i] = UIntMap_Map( invMap, self->snks[p_i][ind_i] );
			self->snks[p_i][ind_i] = Decomp_GlobalToLocal( self->decomps[0], self->snks[p_i][ind_i] );
		}

		self->netSrcs += self->nSrcs[p_i];
		for( ind_i = 0; ind_i < self->nSrcs[p_i]; ind_i++ )
			self->srcs[p_i][ind_i] = Decomp_GlobalToLocal( self->decomps[1], self->srcs[p_i][ind_i] );
	}
	FreeObject( invMap );
}

void DecompTransfer_BuildArray( DecompTransfer* self, DecompTransfer_Array* array ) {
	CommTopology*	commTopo;
	unsigned	rank;
	unsigned	nInc;
	unsigned*	inc;

	assert( self );
	assert( self->commTopo );

	/* Shortcuts. */
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
			snkDisps = Memory_Alloc_Array( unsigned, nInc, "DecompTransfer_Array::snkDisps" );
			snkSizes = Memory_Alloc_Array( unsigned, nInc, "DecompTransfer_Array::snkSizes" );
			snkOffs = Memory_Alloc_Array( unsigned, self->netSnks, "DecompTransfer_Array::snkOffs" );

			/* Calculate offsets and sizes. */
			for( p_i = 0; p_i < nInc; p_i++ ) {
				unsigned	snk_i;

				snkSizes[p_i] = 0;
				for( snk_i = 0; snk_i < self->nSnks[p_i]; snk_i++ ) {
					snkOffs[snkInd] = self->snks[p_i][snk_i] * array->snkStride;
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
			array->snkDisps = Memory_Alloc_Array( unsigned, nInc, "DecompTransfer_Array::snkDisps" );
			array->snkSizes = Memory_Alloc_Array( unsigned, nInc, "DecompTransfer_Array::snkSizes" );
			memset( array->snkDisps, 0, nInc * sizeof(unsigned) );
			memset( array->snkSizes, 0, nInc * sizeof(unsigned) );
		}

		/* Determine source information. */
		if( self->netSrcs > 0 ) {
			unsigned*	srcOffs;
			unsigned*	srcSizes;
			unsigned*	srcDisps;
			unsigned	srcInd = 0;
			unsigned	p_i;

			/* Allocate/reallocate memory. */
			srcDisps = Memory_Alloc_Array( unsigned, nInc, "DecompTransfer_Array::srcDisps" );
			srcSizes = Memory_Alloc_Array( unsigned, nInc, "DecompTransfer_Array::srcSizes" );
			srcOffs = Memory_Alloc_Array( unsigned, self->netSrcs, "DecompTransfer_Array::srcOffs" );

			/* Calculate offsets and sizes. */
			for( p_i = 0; p_i < nInc; p_i++ ) {
				unsigned	src_i;

				srcSizes[p_i] = 0;
				for( src_i = 0; src_i < self->nSrcs[p_i]; src_i++ ) {
					srcOffs[srcInd] = self->srcs[p_i][src_i] * array->srcStride;
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
			array->srcDisps = Memory_Alloc_Array( unsigned, nInc, "DecompTransfer_Array::srcDisps" );
			array->srcSizes = Memory_Alloc_Array( unsigned, nInc, "DecompTransfer_Array::srcSizes" );
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
}

void DecompTransfer_Destruct( DecompTransfer* self ) {
	self->netSrcs = 0;
	KillArray( self->nSrcs );
	KillArray( self->srcs );
	self->netSnks = 0;
	KillArray( self->nSnks );
	KillArray( self->snks );
}

void DecompTransfer_DestructArrays( DecompTransfer* self ) {
	unsigned	a_i;

	for( a_i = 0; a_i < self->nArrays; a_i++ )
		DecompTransfer_DestructArray( self->arrays[a_i] );
	KillArray( self->arrays );
	self->nArrays = 0;
}

void DecompTransfer_DestructArray( DecompTransfer_Array* array ) {
	FreeArray( array->snkDisps );
	FreeArray( array->snkSizes );
	FreeArray( array->snkOffs );
	FreeArray( array->srcDisps );
	FreeArray( array->srcSizes );
	FreeArray( array->srcOffs );
	FreeArray( array );
}

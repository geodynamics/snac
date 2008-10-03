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
** $Id: MPIRoutines.c 2276 2004-11-04 02:01:18Z AlanLo $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <mpi.h>

#include "Base/Foundation/Foundation.h"

#include "types.h"
#include "MPIRoutines.h"


void MPIArray_Bcast( unsigned* arraySize, void** array, size_t itemSize, 
		     unsigned root, MPI_Comm comm )
{
	unsigned	nProcs;
	unsigned	rank;

	/* Sanity check. */
	assert( !arraySize || array );

	/* Get basic MPI info. */
	MPI_Comm_size( comm, (int*)&nProcs );
	MPI_Comm_rank( comm, (int*)&rank );

	/* Send array sizes and allocate. */
	MPI_Bcast( arraySize, 1, MPI_UNSIGNED, root, comm );
	if( rank != root )
		*array = Memory_Alloc_Array_Bytes_Unnamed( itemSize, *arraySize, "unknown" );

	/* Send array. */
	MPI_Bcast( *array, itemSize * (*arraySize), MPI_BYTE, root, comm );
}


void MPIArray_Gather( unsigned arraySize, void* array, 
		      unsigned** dstSizes, void*** dstArrays, 
		      size_t itemSize, unsigned root, MPI_Comm comm )
{
	unsigned	nProcs;
	unsigned	rank;
	unsigned*	tmpSizes;
	unsigned*	disps;
	unsigned	netSize;
	Stg_Byte*	tmpArray;
	unsigned	p_i;

	/* Get basic MPI info. */
	MPI_Comm_size( comm, (int*)&nProcs );
	MPI_Comm_rank( comm, (int*)&rank );

	/*
	** Send a 1D array of arbitrary length to root process in supplied communicator.  This means we also
	** need to receive arrays of arbitrary length from all others.  As the array from this proc is already stored
	** elsewhere we will remove it from the received array, setting its length to zero.
	*/
	if( rank == root )
		*dstSizes = Memory_Alloc_Array_Unnamed( unsigned, nProcs );
	else
		*dstSizes = NULL;
	MPI_Gather( &arraySize, 1, MPI_UNSIGNED, *dstSizes, 1, MPI_UNSIGNED, root, comm );

	/* Factor in 'itemSize'. */
	if( rank == root ) {
		tmpSizes = Memory_Alloc_Array_Unnamed( unsigned, nProcs );
		for( p_i = 0; p_i < nProcs; p_i++ )
			tmpSizes[p_i] = (*dstSizes)[p_i] * itemSize;

		/* Allocate space for the coming arrays and build a displacement list. */
		disps = Memory_Alloc_Array_Unnamed( unsigned, nProcs );
		disps[0] = 0;
		netSize = (*dstSizes)[0];
		for( p_i = 1; p_i < nProcs; p_i++ ) {
			disps[p_i] = disps[p_i - 1] + tmpSizes[p_i - 1];
			netSize += (*dstSizes)[p_i];
		}

		if( netSize )
			tmpArray = Memory_Alloc_Array_Bytes_Unnamed( itemSize, netSize, "unknown" );
		else
			tmpArray = NULL;
	}
	else {
		tmpArray = NULL;
		tmpSizes = NULL;
		disps = NULL;
	}

	/* Send/receive array/s. */
	MPI_Gatherv( array, arraySize * itemSize, MPI_BYTE, 
		     tmpArray, (int*)tmpSizes, (int*)disps, MPI_BYTE, 
		     root, comm );

	/* Free the displacements and temporary-sizes. */
	FreeArray( disps );
	FreeArray( tmpSizes );

	/* Convert result to 2D array. */
	if( rank == root )
		Array_1DTo2D( nProcs, *dstSizes, tmpArray, dstArrays, itemSize );
}

void MPIArray_Allgather( unsigned arraySize, void* array, 
			 unsigned** dstSizes, void*** dstArrays, 
			 size_t itemSize, MPI_Comm comm )
{
	unsigned	nProcs;
	unsigned	rank;
	unsigned*	tmpSizes;
	void*		tmpArray1D;
	unsigned*	disps;
	unsigned	netSize;
	unsigned	p_i;

	/* Get basic MPI info. */
	MPI_Comm_size( comm, (int*)&nProcs );
	MPI_Comm_rank( comm, (int*)&rank );

	/*
	** Send a 1D array of arbitrary length to all other processes in the supplied communicator.  This means we also
	** need to receive arrays of arbitrary length from all others.  As the array from this proc is already stored
	** elsewhere we will remove it from the received array, setting its length to zero.
	*/
	*dstSizes = Memory_Alloc_Array_Unnamed( unsigned, nProcs );
	MPI_Allgather( &arraySize, 1, MPI_UNSIGNED, *dstSizes, 1, MPI_UNSIGNED, comm );

	/* Factor in 'itemSize'. */
	tmpSizes = Memory_Alloc_Array_Unnamed( unsigned, nProcs );
	for( p_i = 0; p_i < nProcs; p_i++ )
		tmpSizes[p_i] = (*dstSizes)[p_i] * itemSize;
		
	/* Allocate space for the coming arrays and build a displacement list. */
	disps = Memory_Alloc_Array_Unnamed( unsigned, nProcs );
	disps[0] = 0;
	netSize = (*dstSizes)[0];
	for( p_i = 1; p_i < nProcs; p_i++ ) {
		disps[p_i] = disps[p_i - 1] + tmpSizes[p_i - 1];
		netSize += (*dstSizes)[p_i];
	}

	if( netSize )
		tmpArray1D = Memory_Alloc_Array_Bytes_Unnamed( itemSize, netSize, "unknown" );
	else
		tmpArray1D = NULL;

	/* Send/receive array/s. */
	MPI_Allgatherv( array, arraySize * itemSize, MPI_BYTE, 
			tmpArray1D, (int*)tmpSizes, (int*)disps, 
			MPI_BYTE, comm );

	/* Free the displacements and temp-sizes. */
	FreeArray( disps );
	FreeArray( tmpSizes );

	/* Unpack the 1D array into the 2D destination. */
	Array_1DTo2D( nProcs, *dstSizes, tmpArray1D, dstArrays, itemSize );

	/* Free resources. */
	FreeArray( tmpArray1D );
}

void MPIArray2D_Alltoall( unsigned* arraySizes, void** arrays, 
			  unsigned** dstSizes, void*** dstArrays, 
			  size_t itemSize, MPI_Comm comm )
{
	unsigned	nProcs;
	unsigned	rank;
	unsigned*	tmpDstSizes;
	unsigned*	tmpDstArray1D;
	unsigned*	dstDisps;
	unsigned	netSize;
	unsigned*	tmpSizes;
	void*		tmpSrcArray1D;
	unsigned*	disps;
	unsigned	p_i;

	/* Get basic MPI info. */
	MPI_Comm_size( comm, (int*)&nProcs );
	MPI_Comm_rank( comm, (int*)&rank );

	/*
	** Blah, blah, sick of comments.
	*/
	*dstSizes = Memory_Alloc_Array_Unnamed( unsigned, nProcs );
	MPI_Alltoall( arraySizes, 1, MPI_UNSIGNED, *dstSizes, 1, MPI_UNSIGNED, comm );

	/* Copy sizes into a new array and modify to include 'itemSize'. */
	tmpDstSizes = Memory_Alloc_Array_Unnamed( unsigned, nProcs );
	for( p_i = 0; p_i < nProcs; p_i++ )
		tmpDstSizes[p_i] = (*dstSizes)[p_i] * itemSize;

	/* Allocate space for the coming arrays and build a displacement list. */
	dstDisps = Memory_Alloc_Array_Unnamed( unsigned, nProcs );
	dstDisps[0] = 0;
	netSize = (*dstSizes)[0];
	for( p_i = 1; p_i < nProcs; p_i++ ) {
		dstDisps[p_i] = dstDisps[p_i - 1] + tmpDstSizes[p_i - 1];
		netSize += (*dstSizes)[p_i];
	}

	if( netSize )
		tmpDstArray1D = Memory_Alloc_Array_Bytes_Unnamed( itemSize, netSize, "unknown" );
	else
		tmpDstArray1D = NULL;

	/* Pack the supplied 2D array into a 1D array and send/receive. */
	Array_2DTo1D( nProcs, arraySizes, arrays, &tmpSrcArray1D, sizeof(unsigned), &disps );

	/* Generate a temporary set of sizes to include 'itemSize'. Modify 'dists' while we're at it. */
	tmpSizes = Memory_Alloc_Array_Unnamed( unsigned, nProcs );
	for( p_i = 0; p_i < nProcs; p_i++ ) {
		disps[p_i] *= itemSize;
		tmpSizes[p_i] = arraySizes[p_i] * itemSize;
	}

	/* Send/recv. */
	MPI_Alltoallv( tmpSrcArray1D, (int*)tmpSizes, (int*)disps, MPI_BYTE, 
		       tmpDstArray1D, (int*)tmpDstSizes, (int*)dstDisps, MPI_BYTE, 
		       comm );

	/* Free memory. */
	FreeArray( tmpSizes );
	FreeArray( disps );
	FreeArray( dstDisps );
	FreeArray( tmpDstSizes );
	FreeArray( tmpSrcArray1D );

	/* Unpack the 1D array into the 2D destination. */
	Array_1DTo2D( nProcs, *dstSizes, tmpDstArray1D, dstArrays, itemSize );

	/* Free resources. */
	FreeArray( tmpDstArray1D );
}

void Array_1DTo2D( unsigned nBlocks, unsigned* sizes, void* srcArray, 
		   void*** dstArrays, size_t itemSize )
{
	Stg_Byte**		tmp;
	unsigned*	tmpSizes;
	unsigned	curPos = 0;
	unsigned	b_i;

	if( nBlocks == 0 ) {
		*dstArrays = NULL;
		return;
	}

	/* Calculate sizes. */
	tmpSizes = Memory_Alloc_Array_Unnamed( unsigned, nBlocks );
	for( b_i = 0; b_i < nBlocks; b_i++ )
		tmpSizes[b_i] = sizes[b_i] * (unsigned)itemSize;

	/* Allocate. */
	tmp = Memory_Alloc_2DComplex_Unnamed( Stg_Byte, nBlocks, tmpSizes );
	for( b_i = 0; b_i < nBlocks; b_i++ ) {
		if( sizes[b_i] == 0 )
			continue;

		memcpy( tmp[b_i], (Stg_Byte*)srcArray + curPos, tmpSizes[b_i] );
		curPos += tmpSizes[b_i];
	}

	/* Free memory. */
	FreeArray( tmpSizes );

	*dstArrays = (void**)tmp;
}

void Array_2DTo1D( unsigned nBlocks, unsigned* sizes, void** srcArrays, 
		   void** dstArray, size_t itemSize, unsigned** disps )
{
	if( nBlocks == 0 ) {
		dstArray = NULL;
		disps = NULL;
		return;
	}
	
	/*
	** Dump a 2D array into a 1D array and build a displacement array to accompany it.
	*/
	
	{
		unsigned	netSize;
		unsigned	b_i;
		unsigned*	tmpDisps;
		
		tmpDisps = Memory_Alloc_Array_Unnamed( unsigned, nBlocks );
		
		tmpDisps[0] = 0;
		netSize = sizes[0];
		for( b_i = 1; b_i < nBlocks; b_i++ ) {
			tmpDisps[b_i] = tmpDisps[b_i - 1] + sizes[b_i - 1];
			netSize += sizes[b_i];
		}
		
		if( netSize > 0 ) {
			void*	tmpArray;
			char*	dest;
			
			tmpArray = Memory_Alloc_Array_Bytes_Unnamed( itemSize, netSize, "unknown" );

			for( b_i = 0; b_i < nBlocks; b_i++ ) {
				dest = tmpArray;
				dest += (tmpDisps[b_i] * itemSize);
				memcpy( dest, srcArrays[b_i], itemSize * sizes[b_i] );
			}
			
			*dstArray = tmpArray;
		}
		else {
			*dstArray = NULL;
		}
		
		*disps = tmpDisps;
	}
}

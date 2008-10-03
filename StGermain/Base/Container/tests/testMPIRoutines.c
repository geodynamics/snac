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
** $Id: testMPIRoutines.c 2136 2004-09-30 02:47:13Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"


Bool testBcast( unsigned rank, unsigned nProcs, unsigned watch );
Bool testArrayConv( unsigned rank, unsigned nProcs, unsigned watch );
Bool testGather( unsigned rank, unsigned nProcs, unsigned watch );
Bool testAllgather( unsigned rank, unsigned nProcs, unsigned watch );
Bool testAlltoall( unsigned rank, unsigned nProcs, unsigned watch );

int main( int argc, char* argv[] ) {
	unsigned	rank;
	unsigned	nProcs;
	unsigned	watch;
	Bool		result;

	/* Initialise MPI, get world info. */
	MPI_Init( &argc, &argv );
	MPI_Comm_size( MPI_COMM_WORLD, (int*)&nProcs );
	MPI_Comm_rank( MPI_COMM_WORLD, (int*)&rank );

	/* Initialise StGermain. */
	BaseFoundation_Init( &argc, &argv );
	BaseIO_Init( &argc, &argv );
	BaseContainer_Init( &argc, &argv );

	/* Watching a particular processor? */
	watch = (argc >= 2) ? atoi( argv[1] ) : 0;

	/* Run some tests. */
	result = testBcast( rank, nProcs, watch );
	if( rank == watch )
		printf( "Testing array broadcast... %s\n", result ? "passed" : "failed" );

	result = testArrayConv( rank, nProcs, watch );
	if( rank == watch )
		printf( "Testing array conversion... %s\n", result ? "passed" : "failed" );

	result = testGather( rank, nProcs, watch );
	if( rank == watch )
		printf( "Testing array gather... %s\n", result ? "passed" : "failed" );

	result = testAllgather( rank, nProcs, watch );
	if( rank == watch )
		printf( "Testing array gather to all... %s\n", result ? "passed" : "failed" );

	result = testAlltoall( rank, nProcs, watch );
	if( rank == watch )
		printf( "Testing array all to all... %s\n", result ? "passed" : "failed" );

	/* Finalise StGermain. */
	BaseContainer_Finalise();
	BaseIO_Finalise();
	BaseFoundation_Finalise();

	/* Close off MPI */
	MPI_Finalize();

	return MPI_SUCCESS;
}

Bool testArrayConv( unsigned rank, unsigned nProcs, unsigned watch ) {
	if( rank == watch ) {
		unsigned	nBlocks = 3;
		unsigned	sizes[3] = {2, 4, 1};
		unsigned	src[7] = {0, 1, 2, 3, 4, 5, 6};
		unsigned**	dst2D;
		unsigned*	dst1D;
		unsigned*	disps;

		Array_1DTo2D( nBlocks, sizes, src, (void***)&dst2D, sizeof(unsigned) );
		if( dst2D[0][0] != 0 || dst2D[0][1] != 1 || 
		    dst2D[1][0] != 2 || dst2D[1][1] != 3 || dst2D[1][2] != 4 || dst2D[1][3] != 5 || 
		    dst2D[2][0] != 6 )
		{
			FreeArray( dst2D );
			return False;
		}

		Array_2DTo1D( nBlocks, sizes, (void**)dst2D, (void*)&dst1D, sizeof(unsigned), &disps );
		if( dst1D[0] != 0 || dst1D[1] != 1 || dst1D[2] != 2 || dst1D[3] != 3 || 
		    dst1D[4] != 4 || dst1D[5] != 5 || dst1D[6] != 6 || 
		    disps[0] != 0 || disps[1] != 2 || disps[2] != 6 )
		{
			FreeArray( dst2D );
			FreeArray( dst1D );
			FreeArray( disps );
			return False;
		}

		FreeArray( dst2D );
		FreeArray( dst1D );
		FreeArray( disps );

		sizes[1] = 0;
		Array_1DTo2D( nBlocks, sizes, src, (void***)&dst2D, sizeof(unsigned) );
		if( dst2D[0][0] != 0 || dst2D[0][1] != 1 || 
		    dst2D[2][0] != 2 )
		{
			FreeArray( dst2D );
			return False;
		}

		Array_2DTo1D( nBlocks, sizes, (void**)dst2D, (void*)&dst1D, sizeof(unsigned), &disps );
		if( dst1D[0] != 0 || dst1D[1] != 1 || dst1D[2] != 2 || 
		    disps[0] != 0 || disps[1] != 2 || disps[2] != 2 )
		{
			FreeArray( dst2D );
			FreeArray( dst1D );
			FreeArray( disps );
			return False;
		}

		FreeArray( dst2D );
		FreeArray( dst1D );
		FreeArray( disps );
	}

	return True;
}

void fillArray( unsigned* array, unsigned size, unsigned rank ) {
	unsigned	i;

	for( i = 0; i < size; i++ )
		array[i] = rank * size + i;
}

Bool testBcast( unsigned rank, unsigned nProcs, unsigned watch ) {
	unsigned	size = 200;
	unsigned*	src;

	if( rank == 0 ) {
		src = Memory_Alloc_Array_Unnamed( unsigned, size );
		fillArray( src, size, rank );
	}

	MPIArray_Bcast( &size, (void**)&src, sizeof(unsigned), 
			0, MPI_COMM_WORLD );

	if( rank == watch ) {
		unsigned	i;

		for( i = 0; i < size; i++ )
			if( src[i] != i ) break;
		if( i < size ) {
			if( rank != 0 )
				FreeArray( src );
			return False;
		}
	}

	if( rank != 0 )
		FreeArray( src );

	return True;
}

Bool testGather( unsigned rank, unsigned nProcs, unsigned watch ) {
	unsigned	size = 200;
	unsigned	src[200];
	unsigned*	dstSizes;
	unsigned**	dstArrays;

	fillArray( src, size, rank );
	MPIArray_Gather( size, src, &dstSizes, (void***)&dstArrays, sizeof(unsigned), watch, MPI_COMM_WORLD );
	if( rank == watch ) {
		unsigned	 i;

		for( i = 0; i < nProcs; i++ )
			if( dstSizes[i] != size ) break;
		if( i < nProcs ) {
			FreeArray( dstSizes );
			FreeArray( dstArrays );
			return False;
		}

		for( i = 0; i < nProcs; i++ ) {
			unsigned	j;

			for( j = 0; j < size; j++ )
				if( dstArrays[i][j] != i * size + j ) break;
			if( j < size ) break;
		}
		if( i < nProcs ) {
			FreeArray( dstSizes );
			FreeArray( dstArrays );
			return False;
		}

		FreeArray( dstSizes );
		FreeArray( dstArrays );
	}

	return True;
}

Bool testAllgather( unsigned rank, unsigned nProcs, unsigned watch ) {
	unsigned	size = 200;
	unsigned	src[200];
	unsigned*	dstSizes;
	unsigned**	dstArrays;

	fillArray( src, size, rank );
	MPIArray_Allgather( size, src, &dstSizes, (void***)&dstArrays, sizeof(unsigned), MPI_COMM_WORLD );
	if( rank == watch ) {
		unsigned	 i;

		for( i = 0; i < nProcs; i++ )
			if( dstSizes[i] != size ) break;
		if( i < nProcs ) {
			FreeArray( dstSizes );
			FreeArray( dstArrays );
			return False;
		}

		for( i = 0; i < nProcs; i++ ) {
			unsigned	j;

			if( !dstArrays[i] ) break;
			for( j = 0; j < size; j++ )
				if( dstArrays[i][j] != size * i + j ) break;
			if( j < size ) break;
		}
		if( i < nProcs ) {
			FreeArray( dstSizes );
			FreeArray( dstArrays );
			return False;
		}
	}

	FreeArray( dstSizes );
	FreeArray( dstArrays );

	return True;
}

Bool testAlltoall( unsigned rank, unsigned nProcs, unsigned watch ) {
	unsigned*	srcSizes;
	unsigned**	srcArrays;
	unsigned*	dstSizes;
	unsigned**	dstArrays;
	unsigned	i;

	srcSizes = Memory_Alloc_Array_Unnamed( unsigned, nProcs );
	srcArrays = Memory_Alloc_2DArray_Unnamed( unsigned, nProcs, 1 );
	for( i = 0; i < nProcs; i++ ) {
		srcSizes[i] = 1;
		srcArrays[i][0] = i;
	}

	MPIArray2D_Alltoall( srcSizes, (void**)srcArrays, &dstSizes, (void***)&dstArrays, sizeof(unsigned), MPI_COMM_WORLD );
	FreeArray( srcSizes );
	FreeArray( srcArrays );
	if( rank == watch ) {
		for( i = 0; i < nProcs; i++ )
			if( dstSizes[i] != 1 || dstArrays[i][0] != rank ) break;
		if( i < nProcs ) {
			FreeArray( dstSizes );
			FreeArray( dstArrays );
			return False;
		}
	}

	FreeArray( dstSizes );
	FreeArray( dstArrays );

	return True;
}

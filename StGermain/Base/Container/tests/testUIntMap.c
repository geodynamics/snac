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

Bool testInsert( unsigned rank, unsigned nProcs, unsigned watch );
Bool testMap( unsigned rank, unsigned nProcs, unsigned watch );
Bool testMemory( unsigned rank, unsigned nProcs, unsigned watch );

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
	result = testInsert( rank, nProcs, watch );
	if( rank == watch )
		printf( "Testing array conversion... %s\n", result ? "passed" : "failed" );

	result = testMap( rank, nProcs, watch );
	if( rank == watch )
		printf( "Testing array gather... %s\n", result ? "passed" : "failed" );

	result = testMemory( rank, nProcs, watch );
	if( rank == watch )
		printf( "Testing memory... %s\n", result ? "passed" : "failed" );

	/* Finalise StGermain. */
	BaseContainer_Finalise();
	BaseIO_Finalise();
	BaseFoundation_Finalise();

	/* Close off MPI */
	MPI_Finalize();

	return MPI_SUCCESS;
}

void fillMap( UIntMap* map, unsigned size ) {
	unsigned	i;

	for( i = 0; i < size; i++ )
		UIntMap_Insert( map, i, size + i );
}

Bool testInsert( unsigned rank, unsigned nProcs, unsigned watch ) {
	if( rank == watch ) {
		unsigned	size = 5;
		UIntMap*	map = UIntMap_New();

		fillMap( map, size );
		if( map->size != size ) {
			FreeObject( map );
			return False;
		}

		FreeObject( map );
	}

	return True;
}

Bool testMap( unsigned rank, unsigned nProcs, unsigned watch ) {
	if( rank == watch ) {
		unsigned	size = 5;
		UIntMap*	map = UIntMap_New();
		unsigned	i;

		fillMap( map, size );
		for( i = 0; i < size; i++ )
			if( UIntMap_Map( map, i ) != size + i ) break;
		if( i < size ) {
			FreeObject( map );
			return False;
		}

		FreeObject( map );
	}

	return True;
}

Bool testMemory( unsigned rank, unsigned nProcs, unsigned watch ) {
	if( rank == watch ) {
		unsigned	size = 5;
		unsigned	nReps = 10;
		UIntMap*	map;
		unsigned	r_i;

		for( r_i = 0; r_i < nReps; r_i++ ) {
			unsigned	i;

			map = UIntMap_New();
			fillMap( map, size );
			for( i = 0; i < size; i++ )
				if( UIntMap_Map( map, i ) != size + i ) break;
			if( i < size ) {
				FreeObject( map );
				return False;
			}
			FreeObject( map );
		}
	}

	return True;
}

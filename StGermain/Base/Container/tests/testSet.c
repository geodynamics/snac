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
** $Id: testSet.c 2136 2004-09-30 02:47:13Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


int compareFunc( void* left, void* right ) {
	if( *(int*)left < *(int*)right ) {
		return -1;
	}
	else if( *(int*)left > *(int*)right ) {
		return 1;
	}
	else {
		return 0;
	}
}


void copyFunc( void** newData, void* data, SizeT size ) {
	*newData = Memory_Alloc_Bytes_Unnamed( size, char );
	/* TODO: convert to journal */
	assert( *newData );

	*(int*)(*newData) = *(int*)data;
}


void deleteFunc( void* data ) {
	/* TODO: convert to journal */
	assert( data );
	Memory_Free( data );
}


void printSet( void* data, void* args ) {
	printf( "%d\n", *(int*)data );
}


int main( int argc, char* argv[] ) {
	MPI_Comm	CommWorld;
	int		rank;
	int		numProcessors;
	int		procToWatch;
	
	
	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size( CommWorld, &numProcessors );
	MPI_Comm_rank( CommWorld, &rank );

	BaseFoundation_Init( &argc, &argv );
	BaseIO_Init( &argc, &argv );
	BaseContainer_Init( &argc, &argv );

	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}
	
	if( rank == procToWatch ) {
		Set*		setA;
		Set*		setB;
		Set*		setC;
		unsigned	int_I;

		setA = Set_New( NULL, int, compareFunc, copyFunc, deleteFunc );
		setB = Set_New( NULL, int, compareFunc, copyFunc, deleteFunc );

		/* I expect the set to be able to insert 100,000 items in the worst case
		   scenario in a reasonable amount of time. */
		for( int_I = 0; int_I < 100000; int_I++ ) {
			Set_Insert( setA, &int_I );
		}

		for( int_I = 50000; int_I < 150000; int_I++ ) {
			Set_Insert( setB, &int_I );
		}

		setC = Set_Subtraction( setA, setB );
		Set_Traverse( setC, printSet, NULL );

		Stg_Class_Delete( setA );
		Stg_Class_Delete( setB );
		Stg_Class_Delete( setC );
	}
	
	BaseContainer_Finalise();
	BaseIO_Finalise();
	BaseFoundation_Finalise();
	
	MPI_Finalize();
	
	return EXIT_SUCCESS;
}

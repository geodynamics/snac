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
** $Id: testMap.c 2136 2004-09-30 02:47:13Z PatrickSunter $
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


int mapCompareFunc( void* left, void* right ) {
	return strcmp( ((MapTuple*)left)->keyData, ((MapTuple*)right)->keyData );
}


void mapCopyFunc( void** newData, void* data, SizeT size ) {
	*newData = Memory_Alloc_Bytes_Unnamed( size , char);
	/* TODO: convert to journal */
	assert( *newData );

	(*(MapTuple**)newData)->keyData = Memory_Alloc_Bytes_Unnamed( strlen( ((MapTuple*)data)->keyData ) + 1 , char);
	strcpy( (*(MapTuple**)newData)->keyData, ((MapTuple*)data)->keyData );

	(*(MapTuple**)newData)->valueData = Memory_Alloc( int, "MapTuple_newData->valueData" );
	*(int*)(*(MapTuple**)newData)->valueData = *(int*)((MapTuple*)data)->valueData;
}


void mapDeleteFunc( void* data ) {
	/* TODO: convert to journal */
	assert( data );
	Memory_Free( ((MapTuple*)data)->keyData );
	Memory_Free( ((MapTuple*)data)->valueData );
	Memory_Free( data );
}


void printMap( void* data, void* args ) {
	printf( "(%s, %d)\n", (char*)((MapTuple*)data)->keyData, *(int*)((MapTuple*)data)->valueData );
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
		Map*		mapA;
		Map*		mapB;
		Map*		mapC;
		unsigned	int_I;
		unsigned*	valData;
		char*		months[] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "Decemeber" };

		mapA = Map_New( NULL, char*, int, mapCompareFunc, mapCopyFunc, mapDeleteFunc );
		mapB = Map_New( NULL, char*, int, mapCompareFunc, mapCopyFunc, mapDeleteFunc );

		for( int_I = 0; int_I < 7; int_I++ ) {
			Map_Insert( mapA, months[int_I], &int_I );
		}

		for( int_I = 5; int_I < 12; int_I++ ) {
			Map_Insert( mapB, months[int_I], &int_I );
		}

		mapC = Map_Union( mapA, mapB );
		Map_Traverse( mapC, printMap, NULL );

		valData = Map_Find( mapC, months[0] );
		printf( "%d\n", *valData );
		valData = Map_Find( mapC, months[3] );
		printf( "%d\n", *valData );

		Stg_Class_Delete( mapA );
		Stg_Class_Delete( mapB );
		Stg_Class_Delete( mapC );
	}
	
	BaseContainer_Finalise();
	BaseIO_Finalise();
	BaseFoundation_Finalise();
	
	MPI_Finalize();
	
	return EXIT_SUCCESS;
}

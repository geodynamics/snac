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
** Role:
**	Tests reallocation of objects, 1D arrays and the byte versions.
**
** $Id: testMemoryRealloc.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "Base/Foundation/Foundation.h"
#include "JournalWrappers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

int main( int argc, char* argv[] )
{	
	MPI_Comm CommWorld;
	int rank;
	int numProcessors;
	int procToWatch;
	
	Stream* stream;
	
	int i;

	void* bytesObj;
	void* bytesArray;

	double*	array1d;

	double** array2d;
	double*** array3d;

	void* fromReallocObj;
	char* fromReallocArray;
	char** fromRealloc2DArray;
	char*** fromRealloc3DArray;
	char* fromRealloc2DAs1D;
	char* fromRealloc3DAs1D;

	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size( CommWorld, &numProcessors );
	MPI_Comm_rank( CommWorld, &rank );
	
	BaseFoundation_Init( &argc, &argv );
	
	stream = Journal_Register ( "info", "MyInfo" );
	
	if( argc >= 2 )
	{
		procToWatch = atoi( argv[1] );
	}
	else
	{
		procToWatch = 0;
	}
	if( rank == procToWatch ) 
	{
		Journal_Printf( stream,  "Watching rank: %i\n", rank );
	}


	/*
	 * 1 - Create allocs
	 */
	bytesObj = Memory_Alloc_Bytes( 6, "Bytes", "Alloced" );
	bytesArray = Memory_Alloc_Array_Bytes( 2, 10, "Bytes", "Alloced" );	/* Array of three 10 byte objects */

	array1d = Memory_Alloc_Array( double, 3, "Alloced" );
	array2d = Memory_Alloc_2DArray( double, 1, 3, "Alloced" );		/* Test Realloc safe */
	array3d = Memory_Alloc_3DArray( double, 1, 2, 3, "Alloced" );		/* Test Realloc safe */
	
	/*
	 * 2 - Write values
	 */
	strcpy( (char*)bytesObj, "hello" );
	
	for ( i = 0; i < 3; ++i )
	{
		array1d[i] = (double)i;
	}
	
	
	if ( rank == procToWatch )
	{
		Journal_Printf( stream, "Allocation results.\n" );
		Memory_Print();
	}
	
	
	/*
	 * 3 - Realloc
	 */
	bytesObj = Memory_Realloc( bytesObj, 10 );				/* enlarged object. */
	bytesArray = Memory_Realloc_Array_Bytes( bytesArray, 5, 10 );		/* enlarged array. */
	
	array1d = Memory_Realloc_Array( array1d, double, 5 );			/* enlarged array. */
	array2d = Memory_Realloc_2DArraySafe( array2d, double, 1, 3, 3, 3 );
	array3d = Memory_Realloc_3DArraySafe( array3d, double, 1, 2, 3, 4, 4, 4 );
	
	/*
	 * 4 - Read to make sure they are still there.
	 */
	Journal_Printf( stream, "%s\n", (char*)bytesObj );
	
	for ( i = 0; i < 3; ++i )
	{
		Journal_Printf( stream, "%lf ", array1d[i] );
	}
	Journal_Printf( stream, "\n" );

	
	/*
	 * 5 - Making new allocs from Realloc() NULL
	 */
	fromReallocObj = Memory_Realloc( NULL, 50 );
	fromReallocArray = Memory_Realloc_Array( NULL, char, 50 );
	fromRealloc2DArray = Memory_Realloc_2DArray( NULL, char, 4, 4 );
	fromRealloc3DArray = Memory_Realloc_3DArray( NULL, char, 3, 3, 3 );
	fromRealloc2DAs1D = Memory_Realloc_2DArrayAs1D( NULL, char, 0, 0, 4, 4 );
	fromRealloc3DAs1D = Memory_Realloc_3DArrayAs1D( NULL, char, 0, 0, 0, 3, 3, 3 );


	if( rank == procToWatch )
	{
		Journal_Printf( stream,  "\nReallocaiton results.\n" );
		Memory_Print();

		Journal_Printf( stream, "\nNew pointers from realloc:\n" );
		Memory_Print_Pointer( fromReallocObj );
		Memory_Print_Pointer( fromReallocArray );
		Memory_Print_Pointer( fromRealloc2DArray );
		Memory_Print_Pointer( fromRealloc3DArray );
		Memory_Print_Pointer( fromRealloc2DAs1D );
		Memory_Print_Pointer( fromRealloc3DAs1D );	
	}

	Memory_Free( bytesObj );
	Memory_Free( bytesArray );
	Memory_Free( array1d );
	Memory_Free( array2d );
	Memory_Free( array3d );
	Memory_Free( fromReallocObj );
	Memory_Free( fromReallocArray );
	Memory_Free( fromRealloc2DArray );
	Memory_Free( fromRealloc3DArray );
	Memory_Free( fromRealloc2DAs1D );
	Memory_Free( fromRealloc3DAs1D );


	if( rank == procToWatch )
	{
		Journal_Printf( stream,  "\nFree results\n" );
		Memory_Print();
		Memory_Print_Leak();
	}


	BaseFoundation_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();

	return 0; /* success */
}

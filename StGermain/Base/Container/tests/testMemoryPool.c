
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
** $Id: testLinkedList.c 2136 2005-02-28 02:47:13Z RaquibulHassan $
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

typedef struct Plane_t{
	double normal[3];
	double k;
}Plane;

#define CACHE_SIZE 2000

int main( int argc, char* argv[] ) {
	MPI_Comm			CommWorld;
	int				rank;
	int				numProcessors;
	int				procToWatch;
	Plane			*planeRefs[CACHE_SIZE];
	
	Stream *myStream = NULL;
	
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
		MemoryPool *pool = NULL;
		Plane *p = NULL;
		int i = 0, passed = 0;

		pool = MemoryPool_New( Plane, CACHE_SIZE );
		myStream = Journal_Register( InfoStream_Type, "MemoryPoolStream" );

		passed = 1;
		Journal_Printf( myStream, "Testing memory allocation from the Memory Pool.. " );
		for( i=0; i<CACHE_SIZE; i++ ){
			p = NULL;
			p = MemoryPool_NewObject( Plane, pool );
			if( !p ){
				passed = 0;
			}
			else{
				planeRefs[i] = p;
			}
		}
		Journal_Printf( myStream, "%s\n", passed?"Passed\n":"Failed\n" );
		
		passed = 1;
		Journal_Printf( myStream, "Testing out of memory.. " );
		for( i=0; i<10; i++ ){
			p = MemoryPool_NewObject( Plane, pool );
			if( p ){
				passed = 0;
				break;
			}
		}
		Journal_Printf( myStream, "%s\n", passed?"Passed\n":"Failed\n" );
		
		passed = 1;
		Journal_Printf( myStream, "Testing memory deallocations.. " );
		for( i=0; i<CACHE_SIZE; i++ ){
			if(!MemoryPool_DeleteObject( pool, planeRefs[i] )){
				passed = 0;
				break;
			}
		}
		Journal_Printf( myStream, "%s\n", passed?"Passed\n":"Failed\n" );
		
		passed = 1;
		Journal_Printf( myStream, "Testing illegal memory deallocations.. " );
		{
			int *junkRefs[CACHE_SIZE];
			for( i=0; i<CACHE_SIZE/4; i++ ){
				junkRefs[i] = (int*)(junkRefs+i+1);
				
				if(MemoryPool_DeleteObject( pool, junkRefs[i] )){
					passed = 0;
					break;
				}
			}
		}
		Journal_Printf( myStream, "%s\n", passed?"Passed\n":"Failed\n" );
	}

	BaseContainer_Finalise();
	BaseIO_Finalise();
	BaseFoundation_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0; /* success */
}


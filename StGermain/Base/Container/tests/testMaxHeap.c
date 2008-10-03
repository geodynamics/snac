
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
#include <time.h>

#define NUM_DATA 100
#define NUM_INITIAL_DATA 50

int *data;

int compareFunction(void *data1, void *data2)
{
	int *d1 = NULL, *d2 = NULL;

	d1 = (int*)data1;
	d2 = (int*)data2;

	if (d1 == NULL || d2 == NULL){
		return 0;	
	}
	
	if (*d1 > *d2){
		return  1;
	}
	else if (*d1 == *d2){
		return 0;
	}
	else{
		return -1;
	}
}

void** extendArray( int newCount, void ***array )
{
	assert( array );
	//TODO Check if this is executed by creating memory
	if( (*(int***)array = (int**)realloc(*array, newCount * sizeof( int* ))) == NULL ){
		Journal_Firewall( 0, Journal_Register(ErrorStream_Type, "testMaxHeap"), "Memory allocation failed in '%s'!!\n Aborting..!!\n", __func__ );
	
	}
	else{
		return *array;
	}

	return NULL;
}

void keySwap( void **a, void **b )
{
	int *temp;

	temp = (*((int**)a));

	(*((int**)a)) = (*((int**)b));
	(*((int**)b)) = temp;
}
	
int main( int argc, char* argv[] ) {
	MPI_Comm			CommWorld;
	int				rank;
	int				numProcessors;
	int				procToWatch;
	int **keys;
	
	MaxHeap *heap;
	int i = 0;
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
		
		myStream = Journal_Register( InfoStream_Type, "LinkedListStream" );
		data = Memory_Alloc_Array_Unnamed( int, NUM_DATA );
		keys = Memory_Alloc_Array_Unnamed( int*, NUM_INITIAL_DATA );
		
		Journal_Printf( myStream, "\nCreating the Heap\n" );
		for(i=0; i<NUM_INITIAL_DATA; i++){
			data[i] = i;
			keys[i] = &(data[i]);
		}
		
		heap = MaxHeap_New(
					(void**)(keys), sizeof(int),
					NUM_INITIAL_DATA,
					keySwap,
					compareFunction,
					extendArray );

		Journal_Printf( myStream, "\nPrinting the Heap\n" );
		Stg_Class_Print( heap, myStream );
		
		Journal_Printf( myStream, "\nInserting more entries into the Heap\n" );
		for( i=50; i<NUM_DATA; i++ ){
			data[i] = i;
			MaxHeap_Insert( heap, &(data[i]) );
		}
		
		Journal_Printf( myStream, "\nPrinting the Heap\n" );
		Stg_Class_Print( heap, myStream );
		
		Journal_Printf( myStream, "\nExtracting all the entries in the Heap\n" );
		for( i=0; i<NUM_DATA; i++ ){
			printf( "Heap Max %d\n", *(int*)MaxHeap_Extract( (_Heap*)heap ) );
		}
		
		Journal_Printf( myStream, "\nPrinting the Heap\n" );
		Stg_Class_Print( heap, myStream );
	
		Journal_Printf( myStream, "\nDeleting the Heap\n" );
		Stg_Class_Delete( (void*)heap );
		
		Memory_Free( data );
	}
	
	BaseContainer_Finalise();
	BaseIO_Finalise();
	BaseFoundation_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0; /* success */
}


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
** $Id: testBTree.c 2136 2005-04-05 02:47:13Z RaquibulHassan $
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
#include <math.h>

#define NUM_DATA 100
#define NUM_DATA_LARGE 10000

typedef struct praseFunctionArguments_t{
	double result;
}parseFunctionArguments;

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

void dataPrintFunction( void *nodeData, Stream *printStream )
{
	assert (nodeData);

	Journal_Printf( printStream, "\t%d\n", *(int*)nodeData );
}

void dataCopyFunction( void **nodeData, void *newData, SizeT dataSize)
{
	*nodeData = malloc( dataSize );
	memset(*nodeData, 0, dataSize);

	memcpy(*nodeData, newData, dataSize);
}

void dataDeleteFunction( void *nodeData )
{
	if( nodeData != NULL ){
		free( nodeData );
	}
}

void treeParseFunction( void *nodeData, void *args )
{
	parseFunctionArguments *arguments = NULL;

	assert( nodeData );
	arguments = (parseFunctionArguments*) args;
	assert( arguments );

	arguments->result += *(int*)nodeData;
}

int main( int argc, char* argv[] ) {
	MPI_Comm			CommWorld;
	int				rank;
	int				numProcessors;
	int				procToWatch;
	
	BTree *numList = NULL;
	BTreeNode *node = NULL;
	int i = 0;
	int *array[NUM_DATA] = {0};
	float randomNum, randomMax;
	Stream *myStream = NULL;
	int data = 0;
	void *treeData = NULL;
	double average = 0, parsedAverage = 0;
	parseFunctionArguments parseArguments;
	int idealComplexity = 0, hops = 0;
	BTreeIterator *iterator = NULL;
	double epsilon  = 1e-5;
		
	
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
		
		myStream = Journal_Register( InfoStream_Type, "BTreeStream" );
		numList = BTree_New(
					compareFunction,
					dataCopyFunction,
					dataDeleteFunction,
					dataPrintFunction,
					BTREE_ALLOW_DUPLICATES);
		
		Journal_Printf( myStream, "Testing basic Insertion, Deletion and Query functions\n" );
		Journal_Printf( myStream, "Inserting data into the Tree\n");
		
		for(i=0; i<NUM_DATA; i++){
			array[i] = malloc(sizeof(int));
			randomNum = rand();
			randomMax = RAND_MAX;
			*array[i] = NUM_DATA - i;
			BTree_InsertNode(numList, (void*)array[i], sizeof(int));
		}
	
		Journal_Printf(myStream, "\nPrinting the tree\n");
		Print( (void*)numList, myStream );
		
		Journal_Printf(myStream, "\nDeleting half the nodes previously inserted into the tree\n");
		for(i=0; i<NUM_DATA/2; i++){
			node = BTree_FindNode( numList, (void*)array[i] );
			
			if( node ){
				BTree_DeleteNode( numList, node );
			}
		}
		
		Journal_Printf(myStream, "\nPrinting the tree\n");
		Print( (void*)numList, myStream );
		
		Journal_Printf( myStream, "\nSearching for Node data in the tree\n" );
		for(i=1; i<NUM_DATA/4; i++){
			node = BTree_FindNode( numList, array[NUM_DATA - i] );
			if ( node != NULL ){
				printf( "Looking for - %d, Found..\n", *(int*)(node->data) );
			}
			else{
				printf( "Failed to find.. %d\n", *array[i] );
			}
		}

		iterator = BTreeIterator_New( numList );
		Journal_Printf( myStream, "\nPrinting the iterator\n" );
		Print( (void*)iterator, myStream );
		Journal_Printf( myStream, "\nIterating over all the nodes and printing them\n" );
		for( treeData = BTreeIterator_First( iterator ); treeData != NULL; treeData = BTreeIterator_Next( iterator ) ){
			Journal_Printf( myStream, "%d\n", *(int*)treeData );
		}
	
		Journal_Printf(myStream, "\nDeleting all the nodes previously inserted into the tree\n");
		for(i=0; i<NUM_DATA; i++){
			node = BTree_FindNode( numList, (void*)array[i] );
			
			if( node ){
				BTree_DeleteNode( numList, node );
			}
		}

		average = 0;
		srand( (unsigned)time( NULL ) );
		for( i=0; i<NUM_DATA_LARGE; i++ ){
			data = (int) NUM_DATA_LARGE * (rand() / (RAND_MAX + 1.0));
			BTree_InsertNode(numList, (void*)&data, sizeof(int));
			
			idealComplexity = (int)ceil((log( (double)numList->nodeCount ) / log(2.0)));
			
			/* 
			 * 			n >= 2^(bh) - 1
			 *  where bh is the black height of the tree 
			 *  but bh ~ h/2 
			 *  
			 *  log( n+1 ) >= (bh/2)
			 *	bh <= 2log(n+1)
			 *
			 *  */	
			
			if( (node = BTree_FindNodeAndHops( numList, (void*)&data, &hops )) != NULL ){
				if( idealComplexity * 2 < hops ){
					Journal_Printf( myStream, "Complexity exceeds log(n)..\n" );
					exit( 0 );
				}
			}
			average += data;
		}

		Journal_Printf( myStream, "\nCalculating Complexity\n" );
		Journal_Printf( myStream, "BTree_FindNode has a complexity of O(log n)\nThis implies similar complexity for Insertion and Deletion\n" );
		
		Journal_Printf( myStream, "\nCalculating the average of all the numbers stored in the tree\n" );
		average /= NUM_DATA_LARGE;
		
		/* Using the parseTreeFunction to parse the entire tree and calculate the average of all the numnbers stored in the tree.
		 * The parseTreeFunction returns the sum of all the numbers in the tree via parseArguments.result, where parseArguments
		 * is a user specified struct. */

		parseArguments.result = 0;
		BTree_ParseTree( numList, treeParseFunction, (void*) &parseArguments );
		parsedAverage = parseArguments.result / NUM_DATA_LARGE;
	
		if( fabs(average - parsedAverage) < epsilon ){
			Journal_Printf( myStream, "Average calculated corrrectly\n" );
		}
		else{
			Journal_Printf( myStream, "Error calculating average..\n" );
			Journal_Printf( myStream, "Calculated value %lf, actual %lf\n", parsedAverage, average );
			exit( 0 );
		}
		
		
		
		Journal_Printf( myStream, "\nDeleting the Tree\n" );
		Stg_Class_Delete( (void*)numList );
		Stg_Class_Delete( (void*)iterator );

		for(i=0; i<NUM_DATA; i++){
			free( array[i] );
		}
	}
	
	BaseContainer_Finalise();
	BaseIO_Finalise();
	BaseFoundation_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0; /* success */
}

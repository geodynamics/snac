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
** $Id: testHashTable.c 2136 2005-02-28 02:47:13Z RaquibulHassan $
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

void deleteFunction( void *data ){
	assert( data );

	Memory_Free( data );
}

void printFunction( void *data, void *stream ){
	
	assert( data );
	assert( stream );

	Journal_Printf( (Stream*)stream, "\t\t%s", (void*)data );
}

void ptrPrintFunction( void *data, void *stream ){
	
	assert( data );
	assert( stream );

	Journal_Printf( (Stream*)stream, "\t\t(ptr)%p\n", (void*)data );
}

int main( int argc, char* argv[] ) {
	MPI_Comm			CommWorld;
	int				rank;
	int				numProcessors;
	int				procToWatch;
	HashTable		*table;
	HashTable		*ptrTable;
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
		FILE *dataFile = NULL;
		char buffer[512];
		int result = 0;
		int sum = 0;
		int count = 0;
		char *data = NULL;
		char *searchResult = NULL;
		void *dataRefs[200];
		
		myStream = Journal_Register( InfoStream_Type, "hashTableStream" );

		table = HashTable_New( NULL, printFunction, NULL, HASHTABLE_STRING_KEY );
		
		Journal_Printf( myStream, "Testing hash table with string keys..\n\n" );
		Journal_Printf( myStream, "Inserting data into the hash table\n\n" );
		
		dataFile = fopen( "dictionary.txt", "r+" );
		memset( buffer, 0, sizeof( buffer ) );
		while( (fgets( buffer, sizeof(buffer), dataFile ) != NULL) ){
			data = StG_Strdup(buffer);
			result = HashTable_InsertEntry( table, (void*)data, strlen(buffer), (void*)data, strlen(buffer));
			if( result > 1 ){
				Journal_Printf( myStream, "Incurred %d collisions while inserting %s", result, buffer );
			}
			sum += result;
			memset( buffer, 0, sizeof( buffer ) );
		}
		fclose( dataFile );
		
		Journal_Printf( myStream, "Successfully inserted %d items into the table\n\n", table->count );

		if( sum == table->count ){
			Journal_Printf( myStream, "0 collisions\n\n" );
		}
		
		Journal_Printf( myStream, "Printing the hash table\n\n" );
		Print( table, myStream );

		Journal_Printf( myStream, "Searching for items in the table:\n\n" );
		
		dataFile = fopen( "dictionary.txt", "r+" );
		memset( buffer, 0, sizeof( buffer ) );
		count = 0;
		while( (fgets( buffer, sizeof(buffer), dataFile ) != NULL) ){
			searchResult = (char*)HashTable_FindEntry( table, (void*)buffer, strlen(buffer), char* );
			assert( searchResult );
			if( !strcmp( buffer, searchResult ) ){
				searchResult[strlen(searchResult)-1] = '\0';
				Journal_Printf( myStream, "Looking for %s, ...found\n", searchResult );
			}
			else{
				Journal_Printf( myStream, "item not found..!!\n" );
			}
			memset( buffer, 0, sizeof( buffer ) );
			count++;
		}
		fclose( dataFile );
	
		Journal_Printf( myStream, "\nDeleting the Table\n\n" );
		Stg_Class_Delete( table );
		
		ptrTable = HashTable_New( NULL, ptrPrintFunction, NULL, HASHTABLE_POINTER_KEY );
		
		Journal_Printf( myStream, "Testing hash table with pointer keys..\n\n" );
		Journal_Printf( myStream, "Inserting data into the hash table\n\n" );
		
		dataFile = fopen( "dictionary.txt", "r+" );
		memset( buffer, 0, sizeof( buffer ) );
		count = 0;
		while( (fgets( buffer, sizeof(buffer), dataFile ) != NULL) ){
			data = StG_Strdup(buffer);
			result = HashTable_InsertEntry( ptrTable, (void*)data, sizeof(void*), (void*)data, strlen(buffer));
			if( result > 1 ){
				Journal_Printf( myStream, "Incurred %d collisions while inserting (ptr)%p\n", result, data );
			}
			sum += result;
			memset( buffer, 0, sizeof( buffer ) );
			dataRefs[count++] = data;
		}
		fclose( dataFile );
		
		Journal_Printf( myStream, "Successfully inserted %d items into the table\n\n", ptrTable->count );

		if( sum == ptrTable->count ){
			Journal_Printf( myStream, "0 collisions\n\n" );
		}
		
		Journal_Printf( myStream, "Printing the hash table\n\n" );
		Print( ptrTable, myStream );

		Journal_Printf( myStream, "Searching for items in the table:\n\n" );
		
		dataFile = fopen( "dictionary.txt", "r+" );
		memset( buffer, 0, sizeof( buffer ) );
		count = 0;
		while( (fgets( buffer, sizeof(buffer), dataFile ) != NULL) ){
			searchResult = (char*)HashTable_FindEntry( ptrTable, (void*)dataRefs[count], strlen(buffer), char* );
			assert( searchResult );
			if( !strcmp( buffer, searchResult ) ){
				searchResult[strlen(searchResult)-1] = '\0';
				Journal_Printf( myStream, "Looking for (ptr)%p, ...found\n", searchResult );
			}
			else{
				Journal_Printf( myStream, "item not found..!!\n" );
			}
			memset( buffer, 0, sizeof( buffer ) );
			count++;
		}
		fclose( dataFile );
	
		Journal_Printf( myStream, "\nDeleting the Table\n" );
		Stg_Class_Delete( ptrTable );
	}

	BaseContainer_Finalise();
	BaseIO_Finalise();
	BaseFoundation_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0; /* success */
}

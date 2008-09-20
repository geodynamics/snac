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
** $Id: testIndexSet.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void Print_Container( IndexSet* is ) {
	unsigned int i;

	printf( "is->_container[0-%lu]: ", is->_containerSize );
	for( i = 0; i < is->_containerSize; i++ ) {
		printf( "%u%u%u%u%u%u%u%u, ", is->_container[i] & 0x01 ? 1 : 0, is->_container[i] & 0x02 ? 1 : 0, is->_container[i] & 0x04 ? 1 : 0, is->_container[i] & 0x08 ? 1 : 0, is->_container[i] & 0x10 ? 1 : 0, is->_container[i] & 0x20 ? 1 : 0, is->_container[i] & 0x40 ? 1 : 0, is->_container[i] & 0x80 ? 1 : 0 );
	}
	printf( "\n" );
}	


void Print_Member_List( Index* setArray, unsigned int setArraySize ) {
	int j;

	printf( "setArray[0-%u]: ", setArraySize );
	for( j = 0; j < setArraySize; j++ ) {
		printf( "%u, ", setArray[j] );
	}
	printf( "\n" );
}			


int main( int argc, char* argv[] ) {
	MPI_Comm			CommWorld;
	int				rank;
	int				numProcessors;
	int				procToWatch;
	
	
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
		IndexSet*			is;
		IndexSet*			is2;
		unsigned int			i;
		IndexSet_Index 			index;
		Index*				setArray;
		unsigned int			setArraySize;
		
		printf( "Watching rank: %i\n", rank );
		
		printf( "* Test Construction *\n" );
		is = IndexSet_New( 24 );
		
		printf( "* Test Insertion(Add) *\n" );
		IndexSet_Add( is, 0 );
		IndexSet_Add( is, 7 );
		IndexSet_Add( is, 8 );
		IndexSet_Add( is, 22 );
		IndexSet_Add( is, 23 );
		Print_Container( is );
		
		printf( "* Test removal *\n" );
		IndexSet_Remove( is, 1 );
		IndexSet_Remove( is, 22 );
		Print_Container( is );
		
		printf( "* Test IsMember *\n" );
		for( i = 0; i < is->size; i++ ){
			if( IndexSet_IsMember( is, i ) ) {
				printf( "Index: %u, Is member: yes\n", i );
			}
			else {
				printf( "Index: %u, Is member: no\n", i );
			}
		}
		
		printf( "* Test UpdateMembersCount *\n" );
		IndexSet_UpdateMembersCount( is );
		printf( "Members count is now %d\n", is->membersCount );

		for( i = 0; i <= is->membersCount; i++ ){
			printf( "* Test GetIndexOfNthMember %d*\n", i );
			index = IndexSet_GetIndexOfNthMember( is, i );
			printf( "Index of member %d=%d", i, index );
			if ( IndexSet_Invalid(is) == index ) {
				printf(" (invalid)");
			}
			printf("\n");
		}

		for (i=1; i <= 2; i++ ) {	
			printf( "* Test GetMembers %d*\n", i );
			IndexSet_GetMembers( is, &setArraySize, &setArray );
			Print_Member_List( setArray, setArraySize );
			Memory_Free( setArray );
		}	
		
		for (i=1; i <= 2; i++ ) {	
			printf( "* Test GetVacancies %d*\n", i );
			IndexSet_GetVacancies( is, &setArraySize, &setArray );
			Print_Member_List( setArray, setArraySize );
			Memory_Free( setArray );
		}	

		printf( "* Test AddAll *\n" );
		IndexSet_AddAll( is );
		Print_Container( is );
		printf( "* Test RemoveAll *\n" );
		IndexSet_RemoveAll( is );
		Print_Container( is );
		printf( "* Test Duplicate *\n" );
		is2 = IndexSet_Duplicate( is );
		printf( "* Test Merge_OR *\n" );
		IndexSet_Add( is, 0 );
		IndexSet_Add( is, 7 );
		IndexSet_Add( is2, 7 );
		IndexSet_Add( is2, 8 );
		IndexSet_Add( is2, 22 );
		printf( "Pre_Merge\n" );
		Print_Container( is );
		Print_Container( is2 );
		IndexSet_Merge_OR( is, is2 );
		printf( "Post_Merge\n" );
		Print_Container( is );
		Print_Container( is2 );
		printf( "Test GetMembers again\n" );
		IndexSet_GetMembers( is, &setArraySize, &setArray );
		Print_Member_List( setArray, setArraySize );
		Memory_Free( setArray );
		printf( "Final remove then GetMembers test\n" );
		IndexSet_Remove( is, 7 );
		IndexSet_GetMembers( is, &setArraySize, &setArray );
		Print_Member_List( setArray, setArraySize );
		Memory_Free( setArray );

		Stg_Class_Delete( is );
		Stg_Class_Delete( is2 );
	}
	
	BaseContainer_Finalise();
	BaseIO_Finalise();
	BaseFoundation_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0; /* success */
}



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
**	Tests the dictionary functionality
**
** $Id: testDictionary.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main( int argc, char* argv[] ) {
	MPI_Comm CommWorld;
	int rank;
	int numProcessors;
	int procToWatch;
	Stream* stream;
	
	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size( CommWorld, &numProcessors );
	MPI_Comm_rank( CommWorld, &rank );
	
	BaseFoundation_Init( &argc, &argv );
	BaseIO_Init( &argc, &argv );
	stream = Journal_Register( InfoStream_Type, Dictionary_Type );
	
	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}
	if( rank == procToWatch ) {
		Dictionary*			dictionary = Dictionary_New();
		Dictionary_Entry_Value*		testStruct; 
		Dictionary_Entry_Value*		testStruct2; 
		Dictionary_Entry_Value*		testList; 
		Dictionary_Entry_Value*		yValue;
		Dictionary_Entry_Value*		currValue;
		Dictionary_Index		index;
		
		Dictionary_Add( dictionary, "test_cstring",
			Dictionary_Entry_Value_FromString( "hello" ) );
		Dictionary_Add( dictionary, "test_double",
			Dictionary_Entry_Value_FromDouble( 45.567 ) );
		Dictionary_Add( dictionary, "test_uint",
			Dictionary_Entry_Value_FromUnsignedInt( 5 ) );
		Dictionary_Add( dictionary, "test_int",
			Dictionary_Entry_Value_FromInt( -5 ) );
		Dictionary_Add( dictionary, "test_unsignedlong",
			Dictionary_Entry_Value_FromUnsignedLong( 52342423 ) );
		Dictionary_Add( dictionary, "test_bool",
			Dictionary_Entry_Value_FromUnsignedInt( 1 ) );

		/* Adding members to a struct */
		testStruct = Dictionary_Entry_Value_NewStruct();
		Dictionary_Entry_Value_AddMember( testStruct, "height",
			Dictionary_Entry_Value_FromDouble( 37 ) );
		Dictionary_Entry_Value_AddMember( testStruct, "anisotropic",
			Dictionary_Entry_Value_FromBool( True ) );
		Dictionary_Entry_Value_AddMember( testStruct, "person",
			Dictionary_Entry_Value_FromString( "Patrick" ) );
		Dictionary_Add( dictionary, "test_struct", testStruct );
		/* note the Dictionary_Add could be done before setting up the struct elements,
		 * but this would be less efficient.
		 */

		/* adding a list */
		testList = Dictionary_Entry_Value_NewList();
		Dictionary_Add( dictionary, "test_list", testList );
		Dictionary_Entry_Value_AddElement( testList, Dictionary_Entry_Value_FromDouble(37) );
		Dictionary_Entry_Value_AddElement( testList, Dictionary_Entry_Value_FromDouble(65) );
		Dictionary_Entry_Value_AddElement( testList, Dictionary_Entry_Value_FromDouble(32) );
		Dictionary_Entry_Value_AddElement( testList, Dictionary_Entry_Value_FromDouble(42) );
		Dictionary_Entry_Value_AddElement( testList, Dictionary_Entry_Value_FromDouble(92) );


		/* Adding a struct within a struct */
		testStruct2 = Dictionary_Entry_Value_NewStruct();
		Dictionary_Entry_Value_AddMember( testStruct, "geom",
			testStruct2 );

		Dictionary_Entry_Value_AddMember( testStruct2, "startx",
			Dictionary_Entry_Value_FromUnsignedInt( 45 ) );
		Dictionary_Entry_Value_AddMember( testStruct2, "starty",
			Dictionary_Entry_Value_FromUnsignedInt( 60 ) );
		Dictionary_Entry_Value_AddMember( testStruct2, "startz",
			Dictionary_Entry_Value_FromUnsignedInt( 70 ) );

		/* print dict contents to check */

		printf( "Dictionary:\n" );
		printf( "\tsize: %u\n", dictionary->size );
		printf( "\tdelta: %u\n", dictionary->delta );
		printf( "\tcount: %u\n", dictionary->count );
		printf( "\tentryPtr[0-%u]: {\n", dictionary->count );
		for( index = 0; index < dictionary->count; index++ ) {
			
			printf( "\t\t" );
			Dictionary_Entry_Print( dictionary->entryPtr[index], stream ); 
			printf( "\n" );
		}
		printf( "\t}\n" );

		printf( "getting out dictionary from a struct:\nyvalue = ");
		yValue = Dictionary_Entry_Value_GetMember(
			Dictionary_Entry_Value_GetMember(testStruct, "geom"), "starty");
		Dictionary_Entry_Value_Print( yValue, stream );
		printf("\n");

		printf("changing first 2 list elements:\n");
		/* getting dictionary out of a list */
		currValue = Dictionary_Entry_Value_GetFirstElement( testList );
		/* do something to this value */
		Dictionary_Entry_Value_SetFromDouble( currValue, 34.3 );
		currValue = currValue->next;
		/* do something to this value */
		Dictionary_Entry_Value_SetFromDouble( currValue, 38.9 );
		printf("list now contains:\n");
		Dictionary_Entry_Value_Print( testList, stream );
		printf("\n");

		printf("turning the starty value into a list using add element:\n");
		Dictionary_Entry_Value_AddElement( yValue, Dictionary_Entry_Value_FromDouble(-45) );

		printf("test_struct now contains:");
		Dictionary_Entry_Value_Print( testStruct, stream );
		printf("\n");

		Stg_Class_Delete( dictionary );
	}
	
	BaseIO_Finalise();
	BaseFoundation_Finalise();

	/* Close off MPI */
	MPI_Finalize();
	
	return EXIT_SUCCESS;
}

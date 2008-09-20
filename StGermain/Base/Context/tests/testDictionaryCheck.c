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


#include <stdarg.h>
#include <mpi.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"
#include "Base/Automation/Automation.h"
#include "Base/Extensibility/Extensibility.h"
#include "Base/Context/Context.h"
#include "Base/Context/DictionaryCheck.h"

#include "stdio.h"
#include "stdlib.h"

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
	

	stream = Journal_Register( Info_Type, Dictionary_Type );
	stJournal->firewallProducesAssert = False;
	Stream_RedirectFile(Journal_Register( Error_Type, "DictionaryCheck"), "DictionaryCheck.txt");
	Stream_RedirectFile(stream, "DictionaryCheck.txt");

	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}
	if( rank == procToWatch ) {
		Dictionary*			dictionary = Dictionary_New();
		Dictionary*			dictionary2 = Dictionary_New();
		
		Dictionary_Index	index;
		char*			errMessage = "Component dictionary must have unique names\n";
		
		/* Create a set of Dictionary entries */
		/* For dictionary */
		Dictionary_Add( dictionary, "test_dict_string",
			Dictionary_Entry_Value_FromString( "hello" ) );
		Dictionary_Add( dictionary, "test_dict_double",
			Dictionary_Entry_Value_FromDouble( 45.567 ) );
		Dictionary_Add( dictionary, "test_dict_string",
			Dictionary_Entry_Value_FromString( "goodbye" ) );	
		Dictionary_Add( dictionary, "test_dict_string",
			Dictionary_Entry_Value_FromString( "hello" ) );
		Dictionary_Add( dictionary, "test_dict_string2",
			Dictionary_Entry_Value_FromString( "hello" ) );
		
		/* For dictionary2 */
		Dictionary_Add( dictionary2, "test_dict_string",
			Dictionary_Entry_Value_FromString( "hello" ) );
		Dictionary_Add( dictionary2, "test_dict_double",
			Dictionary_Entry_Value_FromDouble( 45.567 ) );
		Dictionary_Add( dictionary2, "test_dict_stuff",
			Dictionary_Entry_Value_FromString( "hello") );

		/* Print out dictionary stats */
		Journal_Printf(stream, "Dictionary:\n" );
		Journal_Printf(stream, "\tsize: %u\n", dictionary->size );
		Journal_Printf(stream, "\tdelta: %u\n", dictionary->delta );
		Journal_Printf(stream, "\tcount: %u\n", dictionary->count );
		Journal_Printf(stream, "\tentryPtr[0-%u]: {\n", dictionary->count );
		for( index = 0; index < dictionary->count; index++ ) {
			
			Journal_Printf(stream,"\t\t" );
			Dictionary_Entry_Print( dictionary->entryPtr[index], stream ); 
			Journal_Printf(stream, "\n" );
		}
		Journal_Printf(stream, "\t}\n" );
		
		Journal_Printf(stream, "Dictionary 2:\n" );
		Journal_Printf(stream, "\tsize: %u\n", dictionary2->size );
		Journal_Printf(stream, "\tdelta: %u\n", dictionary2->delta );
		Journal_Printf(stream, "\tcount: %u\n", dictionary2->count );
		Journal_Printf(stream, "\tentryPtr[0-%u]: {\n", dictionary2->count );
		for( index = 0; index < dictionary2->count; index++ ) {
			
			Journal_Printf(stream, "\t\t" );
			Dictionary_Entry_Print( dictionary2->entryPtr[index], stream ); 
			Journal_Printf(stream, "\n" );
		}
		Journal_Printf(stream, "\t}\n" );


		/* Call DictionaryCheck function */
		CheckDictionaryKeys(dictionary2, errMessage);
		CheckDictionaryKeys(dictionary,  errMessage);
		
		Stg_Class_Delete( dictionary );
		Stg_Class_Delete( dictionary2 );
	}
	
	BaseIO_Finalise();
	BaseFoundation_Finalise();

	/* Close off MPI */
	MPI_Finalize();
	
	return EXIT_SUCCESS;
}

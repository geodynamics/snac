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
** $Id: testDictionary-shortcuts.c 3462 2006-02-19 06:53:24Z WalterLandry $
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
	
	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size( CommWorld, &numProcessors );
	MPI_Comm_rank( CommWorld, &rank );
	
	BaseFoundation_Init( &argc, &argv );
	BaseIO_Init( &argc, &argv );
	
	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}
	if( rank == procToWatch ) {
		Dictionary*			dictionary = Dictionary_New();
		Stream*             outputStream = Journal_Register( Info_Type, "OutputStream" );

		Stream_RedirectFile( outputStream, "output/output.dat" );

		Dictionary_Add( dictionary, "test_double",
			Dictionary_Entry_Value_FromDouble( 45.567 ) );
		Dictionary_Add( dictionary, "test_float",
			Dictionary_Entry_Value_FromDouble( 5.5 ) );
		Dictionary_Add( dictionary, "test_uint",
			Dictionary_Entry_Value_FromUnsignedInt( 5 ) );
		Dictionary_Add( dictionary, "test_int",
			Dictionary_Entry_Value_FromInt( -5 ) );
		Dictionary_Add( dictionary, "test_bool",
			Dictionary_Entry_Value_FromUnsignedInt( True ) );
		Dictionary_Add( dictionary, "test_string",
			Dictionary_Entry_Value_FromString( "String" ) );
		
		Stg_Class_Print( dictionary, outputStream );

		Journal_PrintValue( outputStream, Dictionary_GetDouble_WithDefault( dictionary, "test_double", 2.8 ));
		Journal_PrintValue( outputStream, Dictionary_GetDouble_WithDefault( dictionary, "test_doubleDefault", 2.8 ));
		
		Journal_PrintValue( outputStream, Dictionary_GetDouble_WithDefault( dictionary, "test_float", 2.2 ));
		Journal_PrintValue( outputStream, Dictionary_GetDouble_WithDefault( dictionary, "test_floatDefault", 2.2 ));
		
		Journal_PrintValue( outputStream, Dictionary_GetUnsignedInt_WithDefault( dictionary, "test_uint", 8 ));
		Journal_PrintValue( outputStream, Dictionary_GetUnsignedInt_WithDefault( dictionary, "test_uintDefault", 8 ));
		
		Journal_PrintValue( outputStream, Dictionary_GetInt_WithDefault( dictionary, "test_int", -8 ));
		Journal_PrintValue( outputStream, Dictionary_GetInt_WithDefault( dictionary, "test_intDefault", -8 ));
		
		Journal_PrintBool( outputStream, Dictionary_GetBool_WithDefault( dictionary, "test_bool", False ));
		Journal_PrintBool( outputStream, Dictionary_GetBool_WithDefault( dictionary, "test_boolDefault", False ));
		
		Journal_PrintString( outputStream, Dictionary_GetString_WithDefault( dictionary, "test_string", "StringDefault" ));
		Journal_PrintString( outputStream, Dictionary_GetString_WithDefault( dictionary, "test_stringDefault", "StringDefault" ));
		
		Journal_Printf( outputStream, "Dictionary_GetString_WithPrintfDefault( dictionary, \"test_stringDefault\", \"StringDefault%%d\", 2 ) = %s\n", Dictionary_GetString_WithPrintfDefault( dictionary, "test_stringDefault", "StringDefault%d", 2 ));
		Journal_Printf( outputStream, "Dictionary_GetString_WithPrintfDefault( dictionary, \"test_stringDefault2\", \"StringDefault%%d\", 2 ) = %s\n", Dictionary_GetString_WithPrintfDefault( dictionary, "test_stringDefault2", "StringDefault%d", 2 ));

		Stg_Class_Print( dictionary, outputStream );
		Stg_Class_Delete( dictionary );
	}
	
	BaseIO_Finalise();
	BaseFoundation_Finalise();

	/* Close off MPI */
	MPI_Finalize();
	
	return EXIT_SUCCESS;
}

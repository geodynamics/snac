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
**	Tests the journal functionality
**
** $Id: testJournal-Dictionary.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
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
	if( rank == procToWatch )
	{
		Stream* infoTest1;
		Stream* infoTest2;
		Stream* debugTest1;
		Stream* debugTest2;
		Stream* dumpTest1;
		Stream* dumpTest2;
		Stream* newTest1;
		Stream* newTest2;
		
		Stream* fileTest1;
		Stream* fileTest2;
		
		Stream* propTest1;
		Stream* propTest2;
		
		Dictionary* dictionary = Dictionary_New();
		XML_IO_Handler* io_handler = XML_IO_Handler_New();

		infoTest1 = Journal_Register( Info_Type, "test1" );
		infoTest2 = Journal_Register( Info_Type, "test2" );
		debugTest1 = Journal_Register( Debug_Type, "test1" );
		debugTest2 = Journal_Register( Debug_Type, "test2" );
		dumpTest1 = Journal_Register( Dump_Type, "test1" );
		dumpTest2 = Journal_Register( Dump_Type, "test2" );
		
		IO_Handler_ReadAllFromFile( io_handler, "data/journal.xml", dictionary ); 

		Journal_ReadFromDictionary( dictionary );

		newTest1 = Journal_Register( Info_Type, "test1.new1" );
		newTest2 = Journal_Register( Info_Type, "test1.new2" );
		
		Journal_Printf( infoTest1, "infoTest1\n" );
		Journal_Printf( infoTest2, "infoTest2\n" );
		Journal_Printf( debugTest1, "debugTest1\n" );
		Journal_Printf( debugTest2, "debugTest2\n" );
		Journal_Printf( dumpTest1, "dumpTest1\n" );
		Journal_Printf( dumpTest2, "dumpTest2\n" );		
		Journal_PrintfL( newTest1, 3, "newTest1\n" );
		Journal_PrintfL( newTest1, 4, "newTest1\n" );
		Journal_Printf( newTest2, "newTest2\n" );		
		
		fileTest1 = Journal_Register( "newtype", "hello" );
		fileTest2 = Journal_Register( "newtype", "other" );
		
		Journal_Printf( fileTest1, "yay!" );
		Journal_Printf( fileTest2, "double yay!" );

		propTest1 = Journal_Register( Info_Type, "propertiestest1" );
		propTest2 = Journal_Register( Info_Type, "propertiestest2" );

		Print( propTest1, infoTest1 );
		Print( propTest2, infoTest1 );
		
		Stg_Class_Delete( io_handler );
		Stg_Class_Delete( dictionary );
	}

	BaseIO_Finalise();
	BaseFoundation_Finalise();

	/* Close off MPI */
	MPI_Finalize();
	
	return EXIT_SUCCESS;
}

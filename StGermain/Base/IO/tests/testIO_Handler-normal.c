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
**	Tests the IO handler
**
** $Id: testIO_Handler-normal.c 3743 2006-08-03 03:14:38Z KentHumphries $
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
		XML_IO_Handler*		io_handler = XML_IO_Handler_New();
		Dictionary*		dictionary = Dictionary_New();
		Dictionary_Index	index;
		Stream*			stream = Journal_Register( InfoStream_Type, XML_IO_Handler_Type );
		
		/* use the base class functions */
		printf( "\ntest of reading normal xml file:\n");
		IO_Handler_ReadAllFromFile( io_handler, "data/normal.xml", dictionary ); 

		printf( "\ndictionary now contains:\n" );
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

		printf( "\ntest of writing normal xml file:\n");
		IO_Handler_WriteAllToFile( io_handler, "data/newnormal.xml", dictionary );
		XML_IO_Handler_SetWriteExplicitTypes( io_handler, True );
		IO_Handler_WriteAllToFile( io_handler, "data/newnormalexplicittypes.xml", dictionary );

		printf( "\ntest of writing single entry:\n");
		XML_IO_Handler_WriteEntryToFile( io_handler, "data/newgeom.xml", "geom", Dictionary_Get( dictionary, "geom" ), Dictionary_GetSource( dictionary, "geom" ) );

		Stg_Class_Delete( io_handler );
		Stg_Class_Delete( dictionary );
	}
	
	BaseIO_Finalise();
	BaseFoundation_Finalise();

	/* Close off MPI */
	MPI_Finalize();
	
	return EXIT_SUCCESS;
}

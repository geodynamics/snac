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
** $Id: testJournal-Dictionary.c 2745 2005-03-05 08:12:18Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

int main( int argc, char* argv[] ) {
	MPI_Comm CommWorld;
	int rank;
	int numProcessors;
	
	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size( CommWorld, &numProcessors );
	MPI_Comm_rank( CommWorld, &rank );
	
	BaseFoundation_Init( &argc, &argv );
	BaseIO_Init( &argc, &argv );

	{
		char data[13];
		int i;
	
		Dictionary* dictionary = Dictionary_New();
		XML_IO_Handler* io_handler = XML_IO_Handler_New();

		Stream* stream1;
		Stream* stream2;
		
		IO_Handler_ReadAllFromFile( io_handler, "data/mpi-stream.xml", dictionary ); 

		Journal_ReadFromDictionary( dictionary );

		stream1 = Journal_Register( MPIStream_Type, "one" );
		/* Write half the alphabet to each process */
		for ( i = 0; i < 13; ++i ) {
			data[i] = 'a' + i + 13 * rank;
		}
		
		/* Print the alphabet */
		MPIStream_WriteAllProcessors( stream1, data, sizeof(char), 13, MPI_COMM_WORLD );
		
		stream2 = Journal_Register( MPIStream_Type, "two" );
		switch ( rank ) {
			case 0:
				MPIStream_SetOffset( stream2, 8, MPI_COMM_WORLD );
				Journal_Printf( stream2, "Hello %d", 10 );
				break;
			case 1:
				MPIStream_SetOffset( stream2, 12, MPI_COMM_WORLD );
				Journal_Printf( stream2, "world %.3f\n", 0.123f );
				break;
		}
		
		Stg_Class_Delete( io_handler );
		Stg_Class_Delete( dictionary );
	}

	BaseIO_Finalise();
	BaseFoundation_Finalise();

	/* Close off MPI */
	MPI_Finalize();
	
	return EXIT_SUCCESS;
}

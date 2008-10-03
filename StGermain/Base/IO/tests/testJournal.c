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
** $Id: testJournal.c 3462 2006-02-19 06:53:24Z WalterLandry $
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
	
		/* At the moment, only the BaseIO_Init do some real work(Initialise the
		singleton of the 3 stream type). The rest of the Init are not doing 
		anything yet. But, it might in the future and hence it is better 
		to code it in all the test files */
	BaseIO_Init( &argc, &argv );
	
	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}
	if( rank == procToWatch ) {
		Stream* myStream;
		Stream* errStream;
		Stream* debugStream;

		Stream* childStream1;
		Stream* childStream2;

		Stream* fileStream;

		Stream* register2Stream;
		Stream* register2Test;
		
		int integer=99;
		float floatValue = 88.11f;
		
		/* Create a CStream & ErrorStream */
		myStream = Journal_Register( InfoStream_Type, "myComponent");
		errStream = Journal_Register( ErrorStream_Type, "myError" );
		debugStream = Journal_Register( DebugStream_Type, "myDebug" );

		/* use the stream */

		Journal_Printf( myStream, "Blah Blah Blah\n" );
		Journal_Printf( myStream, "Integer:%d\n", integer );
		Journal_Printf( myStream, "Float:%f\n", floatValue );
		Journal_Printf( myStream, "Char:%c\n", 's' );
		Journal_Printf( myStream, "String:%s\n", "myString" );
		Journal_Printf( myStream, "Double:%lf\n", 0.88 );
		/* Don't print to error until check system handles error checks */
		/*Journal_Printf( errStream, "Error Error Error\n" );*/
		Journal_Printf( debugStream, "Debugging the test program\n");
		printf("testing journal.\n" );
		
		Journal_Enable_TypedStream( Debug_Type, True );
		Stream_Enable( debugStream, True );
		
		Journal_Printf( debugStream, "1 " );
		Stream_Indent( debugStream );
		Journal_Printf( debugStream, "\n2 " );
		Journal_Printf( debugStream, "3 \n" );
		Stream_UnIndent( debugStream );
		Journal_Printf( debugStream, "4" );		
		Journal_Printf( debugStream, "\n\n\n\n\n" );
		Journal_Printf( debugStream, "G'day\n" );
	
		Journal_PrintfL( myStream, 1, "Hello\n" );
		Journal_PrintfL( myStream, 2, "Hello\n" );

		childStream1 = Stream_RegisterChild( myStream, "child1" );
		childStream2 = Stream_RegisterChild( childStream1, "child2" );

		Journal_Printf( myStream, "0 no indent\n" );
		Stream_IndentBranch( myStream );
		Journal_Printf( childStream1, "1 with 1 indent\n" );
		Stream_IndentBranch( myStream );
		Journal_Printf( childStream2, "2 with 2 indent\n" );
		Stream_UnIndentBranch( myStream );
		Journal_Printf( childStream2, "2 with 1 indent\n" );
		Stream_UnIndentBranch( myStream );
		Journal_Printf( childStream1, "1 with no indent\n" );
		Journal_Printf( childStream2, "2 with no indent\n" );

		fileStream = Journal_Register( Info_Type, "File test" );
		Stream_RedirectFile( fileStream, "data/testRedirect.txt" );
		Journal_Printf( fileStream, "Hello world\n" );

		register2Stream = Journal_Register2( Info_Type, "Component", "Instance" );
		register2Test   = Journal_Register(  Info_Type, "Component.Instance"    );

		/* using printf to test - because we are testing the journal for sanity here */
		if ( register2Stream == register2Test ) {
			printf( "Journal_Register2 works\n" );
		}
		else {
			printf( "Journal_Register2 failed\n" );
		}
		
		Journal_PrintConcise();
	}

	
	BaseIO_Finalise();
	BaseFoundation_Finalise();

	/* Close off MPI */
	MPI_Finalize();
	
	return EXIT_SUCCESS;
}

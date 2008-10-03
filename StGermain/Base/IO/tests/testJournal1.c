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
** $Id: testJournal1.c 3462 2006-02-19 06:53:24Z WalterLandry $
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
		Stream* myInfo;
		Stream* myDebug;		
		Stream* myDump;
		Stream* myError;
		
		Stream* allNew;

		myInfo = Journal_Register( Info_Type, "MyInfo" );
		myDebug = Journal_Register( Debug_Type, "MyDebug" );
		myDump = Journal_Register( Dump_Type, "MyDump" );
		myError = Journal_Register( Error_Type, "MyError" );
		
		allNew = Journal_Register( "My own stream", "allNew" );
		
		printf( "TEST: \"HELLO\" should appear\n" );
		Journal_Printf( myInfo, "%s\n", "HELLO" );
		printf( "TEST: \"WORLD\" should NOT appear\n" );
		Journal_Printf( myDebug, "%s\n", "HELLO" );
		printf( "TEST: \"HELLO\" should NOT appear\n" );
		Journal_Printf( myDump, "%s\n", "HELLO" );
		printf( "TEST: \"WORLD\" should NOT appear\n" );
		Journal_Printf( myError, "%s\n", "HELLO" );

		printf( "Turning off myInfo\n" );
		Journal_Enable_NamedStream( Info_Type, "MyInfo" , False );
		
		printf( "TEST: \"HELLO\" should NOT appear\n" );
		Journal_Printf( myInfo, "%s\n", "HELLO" );

		printf( "Turning on Dump\n"  );
		Journal_Enable_TypedStream( Dump_Type, True );
		Journal_Enable_NamedStream( Dump_Type, "MyDump", True );
		
		printf( "TEST: \"HELLO\" should appear\n" );
		Journal_Printf( myDump, "%s\n", "HELLO" );
		
		printf( "Turning off Journal\n" );
		stJournal->enable = False;

		printf( "TEST: \"HELLO\" should NOT appear\n" );		
		Journal_Printf( myDump, "%s\n", "HELLO" );
		
		stJournal->enable = True;
		
		Journal_Enable_NamedStream( Info_Type, "MyInfo", True );
		
		printf( "TEST: DPrintf\n" );
		Journal_DPrintf( myInfo, "DPrintf\n" );	
	}

	Memory_Print();
	
	BaseIO_Finalise();

	BaseFoundation_Finalise();

	/* Close off MPI */
	MPI_Finalize();
	
	return EXIT_SUCCESS;
}



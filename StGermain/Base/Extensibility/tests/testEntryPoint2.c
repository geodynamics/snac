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
** $Id: testEntryPoint2.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"
#include "Base/Automation/Automation.h"
#include "Base/Extensibility/Extensibility.h"

#include "stdio.h"
#include "stdlib.h"
#include "mpi.h"

Stream* stream;

void Test0( void ) {
	Journal_Printf( (void*) stream, "This is Test0\n" );
}

void Test1( void ) {
	Journal_Printf( (void*) stream, "This is Test1\n" );
}

void Test2( void ) {
	Journal_Printf( (void*) stream, "This is Test2\n" );
}

void Test3( void ) {
	Journal_Printf( (void*) stream, "This is Test3\n" );
}

void Test4( void ) {
	Journal_Printf( (void*) stream, "This is Test4\n" );
}

void Test5( void ) {
	Journal_Printf( (void*) stream, "This is Test5\n" );
}

void Test6( void ) {
	Journal_Printf( (void*) stream, "This is Test6\n" );
}

void Test7( void ) {
	Journal_Printf( (void*) stream, "This is Test7\n" );
}

void Test8( void ) {
	Journal_Printf( (void*) stream, "This is Test8\n" );
}

void Test9( void ) {
	Journal_Printf( (void*) stream, "This is Test9\n" );
}

const Name testEpName = "test";

int main( int argc, char* argv[] ) {
	MPI_Comm CommWorld;
	int rank;
	int numProcessors;
	int procToWatch;
	EntryPoint* entryPoint;
	
	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size( CommWorld, &numProcessors );
	MPI_Comm_rank( CommWorld, &rank );
	
	BaseFoundation_Init( &argc, &argv );
	BaseIO_Init( &argc, &argv );
	BaseContainer_Init( &argc, &argv );
	BaseAutomation_Init( &argc, &argv );
	BaseExtensibility_Init( &argc, &argv );

	/* creating a stream */
	stream =  Journal_Register( InfoStream_Type, "myStream" );
	
	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}
	if( rank == procToWatch ) Journal_Printf( (void*) stream, "Watching rank: %i\n", rank );
	
	/* Read input */

	/* Build the entrypoint */
	entryPoint = EntryPoint_New( testEpName, EntryPoint_0_CastType );
	if( rank == procToWatch ) {
		Journal_Printf( (void*) stream, "entryPoint->hooks->_size: %u\n", entryPoint->hooks->_size );
		Journal_Printf( (void*) stream, "entryPoint->hooks->count: %u\n", entryPoint->hooks->count );
	}
	EntryPoint_Append( entryPoint, "Test0", (void*)Test0, "testCode" );
	EntryPoint_Append( entryPoint, "Test1", (void*)Test1, "testCode" );
	EntryPoint_Append( entryPoint, "Test2", (void*)Test2, "testCode" );
	EntryPoint_Append( entryPoint, "Test3", (void*)Test3, "testCode" );
	EntryPoint_Append( entryPoint, "Test4", (void*)Test4, "testCode" );
	EntryPoint_Append( entryPoint, "Test5", (void*)Test5, "testCode" );
	EntryPoint_Append( entryPoint, "Test6", (void*)Test6, "testCode" );
	EntryPoint_Append( entryPoint, "Test7", (void*)Test7, "testCode" );
	EntryPoint_Append( entryPoint, "Test8", (void*)Test8, "testCode" );
	EntryPoint_Append( entryPoint, "Test9", (void*)Test9, "testCode" );
	if( rank == procToWatch ) {
		Hook_Index hookIndex;
		Journal_Printf( (void*) stream, "entryPoint->hooks->_size: %u\n", entryPoint->hooks->_size );
		Journal_Printf( (void*) stream, "entryPoint->hooks->count: %u\n", entryPoint->hooks->count );

		for (hookIndex = 0; hookIndex < entryPoint->hooks->count; hookIndex++ ) {
			Journal_Printf( (void*) stream, "entryPoint->hooks->data[%d]->name: %s\n", hookIndex,
				entryPoint->hooks->data[hookIndex]->name  );
		}
	}

	/* Run the entry point */
	if( rank == procToWatch ) {
		((EntryPoint_0_CallCast*) entryPoint->run)( entryPoint );
	}

	/* Stg_Class_Delete stuff */
	Stg_Class_Delete( entryPoint );

	BaseExtensibility_Finalise();
	BaseAutomation_Finalise();
	BaseContainer_Finalise();
	BaseIO_Finalise();
	BaseFoundation_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();

	return 0; /* success */
}

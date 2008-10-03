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
** $Id: testEntryPoint0.c 2933 2005-05-12 06:13:10Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"
#include "Base/Automation/Automation.h"
#include "Base/Extensibility/Extensibility.h"

#include "Regresstor/libRegresstor/Regresstor.h"

#include "stdio.h"
#include "stdlib.h"
#include "mpi.h"

#define __Listener \
	__Stg_Class \
	int x;
struct Listener { __Listener };
typedef struct Listener Listener;

Listener* Listener_New( int x ) {
	Listener* result = (Listener*)_Stg_Class_New(
		sizeof( Listener ),
		"Listener",
		_Stg_Class_Delete,
		NULL,
		NULL );
	result->x = x;
	return result;
}

void Listener_0_Func( void* ref ) {
	Listener* self = (Listener*) ref;
	Journal_Printf(
		Journal_Register( Info_Type, __FILE__ ),
		"%s printing %d\n",
		__func__,
		self->x );
}
void Listener_VoidPtr_Func( void* ref, void* data0 ) {
	Listener* self = (Listener*) ref;
	int* data = (int*)data0;
	
	Journal_Printf(
		Journal_Register( Info_Type, __FILE__ ),
		"%s printing %d and %d \n",
		__func__,
		self->x,
		*data );
}


int main( int argc, char* argv[] ) {
	MPI_Comm CommWorld;
	int rank;
	int numProcessors;
	int procToWatch;
	EntryPoint* class0;
	EntryPoint* classVoidPtr;

	#define NUM_LISTENERS 3
	Listener* listeners[NUM_LISTENERS];
	char buf[100];
	int ii;
	int data = 5;

	Stream* stream;

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
	
	RegressionTest_Init( "Base/Extensibility/EntryPoint" );
	
	/* creating a stream */
	stream =  Journal_Register( InfoStream_Type, __FILE__ );

	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}
	if( rank == procToWatch ) Journal_Printf( (void*) stream, "Watching rank: %i\n", rank );
	
	/* Read input */

	/* Build the entry point */
	class0 = EntryPoint_New( "Class0", EntryPoint_Class_0_CastType );
	classVoidPtr = EntryPoint_New( "Class_VoidPtr", EntryPoint_Class_VoidPtr_CastType );

	RegressionTest_Check(
		class0 != NULL &&
		classVoidPtr != NULL,
		stream,
		"EntryPoint creation with ClassHook types",
		"Can we create an EntryPoint with ClassHook run casts" );

	for ( ii = 0; ii < NUM_LISTENERS; ++ii ) {
		listeners[ii] = Listener_New( ii );
		sprintf( buf, "%d", ii );
		EntryPoint_AppendClassHook( class0, buf, (void*)Listener_0_Func, __FILE__, listeners[ii] );
		EntryPoint_AppendClassHook( classVoidPtr, buf, (void*)Listener_VoidPtr_Func, __FILE__, listeners[ii] );
	}

	RegressionTest_Check(
		class0->hooks->count == NUM_LISTENERS &&
		classVoidPtr->hooks->count ==  NUM_LISTENERS,
		stream,
		"Adding ClassHooks",
		"Can we add class hooks onto EntryPoints" );

	/* Run the entry point */
	((EntryPoint_Class_0_CallCast*) class0->run)( class0);
	
	RegressionTest_Check(
		1,
		stream,
		"Run ClassHook with 0 arguments",
		"Can we run ClassHook with 0 arguments" );

	((EntryPoint_Class_VoidPtr_CallCast*) classVoidPtr->run)( classVoidPtr, &data );

	RegressionTest_Check(
		1,
		stream,
		"Run ClassHook with 1 argument",
		"Can we run ClassHook with 1 argument" );

	for ( ii = 0; ii < NUM_LISTENERS; ++ii ) {
		Stg_Class_Delete( listeners[ii] );
	}

	RegressionTest_Check(
		1,
		stream,
		"Destruction of ClassHooks",
		"Can we delete the ClassHooks" );

	/* Stg_Class_Delete stuff */
	Stg_Class_Delete( class0 );
	Stg_Class_Delete( classVoidPtr );

	RegressionTest_Finalise();

	BaseExtensibility_Finalise();
	BaseAutomation_Finalise();
	BaseContainer_Finalise();
	BaseIO_Finalise();
	BaseFoundation_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();

	return 0; /* success */
}

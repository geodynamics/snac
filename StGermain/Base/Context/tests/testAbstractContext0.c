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
**	Tests AbstractContext in the most basic sence... creation on default values, run with single entries added, and delete.
**
** $Id: testAbstractContext0.c 3903 2006-12-12 00:46:24Z JulianGiordani $
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

#include "stdio.h"
#include "stdlib.h"

/* need to allocate memory for this stream */
Stream* stream;

void MyDelete( void* abstractContext ) {
	_AbstractContext_Delete( abstractContext );
}

void MyPrint( void* abstractContext, Stream* stream ) {
	_AbstractContext_Print( abstractContext, stream );
}

void MyBuild( void* context ) {
	Journal_Printf( (void*) stream, "This is Build\n" );
}

void MyInitialConditions( void* context ) {
	Journal_Printf( (void*) stream, "This is InitialConditions\n" );
}

double dt = 2.0f;

double MyDt( void* context ) {
	Journal_Printf( (void*) stream, "This is Dt, returning %g\n", dt );
	return dt;
}

void MySolve( void* context ) {
	Journal_Printf( (void*) stream, "This is Solve\n" );
}

void MySync( void* context ) {
	Journal_Printf( (void*) stream, "This is Sync\n" );
}

void MyOutput( void* context ) {
	Journal_Printf( (void*) stream, "This is Output\n" );
}

void MySetDt( void* context, double _dt ) {
	Journal_Printf( (void*) stream, "This is SetDt... setting to %g\n", _dt );
	dt = _dt;
}

int main( int argc, char* argv[] ) {
	MPI_Comm CommWorld;
	int rank;
	int numProcessors;
	int procToWatch;
	Dictionary* dictionary;
	AbstractContext* abstractContext;
	
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
	BaseContext_Init( &argc, &argv );
	
	/* Initialize stream */
	stream =  Journal_Register( InfoStream_Type, "myStream" );

	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}
	if( rank == procToWatch ) Journal_Printf( (void*) stream, "Watching rank: %i\n", rank );

	/* Read input:
		Added an entry to the dictionary to pass a logic check, in function _AbstractContext_New.
       		The logic check fails if the dictionary is empty, thus I've added one here. JulianGiordani 12-Dec-2006
	*/
	dictionary = Dictionary_New();
	Dictionary_Add( dictionary, "test_string_For_AbstractContext_Test",
			Dictionary_Entry_Value_FromString( "This is Abstract" ) );
	
	/* Build the context */
	abstractContext = _AbstractContext_New( 
		sizeof(AbstractContext), 
		"TestContext", 
		MyDelete, 
		MyPrint, 
		NULL, 
		NULL,
		NULL,
		_AbstractContext_Build, 
		_AbstractContext_Initialise, 
		_AbstractContext_Execute, 
		_AbstractContext_Destroy, 
		"context",
		True,
		MySetDt, 
		0, 
		10, 
		CommWorld, 
		dictionary );

	/* add hooks to existing entry points */
	ContextEP_ReplaceAll( abstractContext, AbstractContext_EP_Build, MyBuild );
	ContextEP_ReplaceAll( abstractContext, AbstractContext_EP_Initialise, MyInitialConditions );
	ContextEP_ReplaceAll( abstractContext, AbstractContext_EP_Solve, MySolve ); 
	ContextEP_ReplaceAll( abstractContext, AbstractContext_EP_Dt, MyDt ); 

	if( rank == procToWatch ) {
		stream = Journal_Register( InfoStream_Type, AbstractContext_Type );
		AbstractContext_PrintConcise( abstractContext, stream );
		
		Journal_Printf( 
			stream, 
			"abstractContext->entryPointList->_size: %lu\n", 
			abstractContext->entryPoint_Register->_size );
		Journal_Printf( 
			stream,
			"abstractContext->entryPointList->count: %u\n", 
			abstractContext->entryPoint_Register->count );
	}
	
	/* Run the context */
	if( rank == procToWatch ) {
		Stg_Component_Build( abstractContext, 0 /* dummy */, False );
		Stg_Component_Initialise( abstractContext, 0 /* dummy */, False );
		Stg_Component_Execute( abstractContext, 0 /* dummy */, False );
		Stg_Component_Destroy( abstractContext, 0 /* dummy */, False );
	}
	
	/* Stg_Class_Delete stuff */
	Stg_Class_Delete( abstractContext );
	Stg_Class_Delete( dictionary );
	
	BaseContext_Finalise();
	BaseExtensibility_Finalise();
	BaseAutomation_Finalise();
	BaseContainer_Finalise();
	BaseIO_Finalise();
	BaseFoundation_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0; /* success */
}

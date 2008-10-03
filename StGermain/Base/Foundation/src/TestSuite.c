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
** $Id: TestSuite.c 3584 2006-05-16 11:11:07Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <mpi.h>

#include "types.h"
#include "shortcuts.h"
#include "forwardDecl.h"
#include "MemoryTag.h"
#include "Memory.h"
#include "Class.h"
#include "TestSuite.h"


/* Textual name of this class */
const Type TestSuite_Type = "TestSuite";


/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

TestSuite* TestSuite_New() {
	return _TestSuite_New( sizeof(TestSuite), 
			      TestSuite_Type, 
			      _TestSuite_Delete, 
			      _TestSuite_Print, 
			      _TestSuite_Copy );
}

TestSuite* _TestSuite_New( TESTSUITE_DEFARGS ) {
	TestSuite* self;

	/* Allocate memory */
	assert( sizeOfSelf >= sizeof(TestSuite) );
	self = (TestSuite*)_Stg_Class_New( STG_CLASS_PASSARGS );

	/* Virtual info */

	/* TestSuite info */
	_TestSuite_Init( self );

	return self;
}

void _TestSuite_Init( TestSuite* self ) {
	assert( self );

	self->nTests = 0;
	self->tests = NULL;
	self->watch = 0;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _TestSuite_Delete( void* testSuite ) {
	TestSuite*	self = (TestSuite*)testSuite;

	assert( self );

	TestSuite_Destruct( self );

	/* Delete the parent. */
	_Stg_Class_Delete( self );
}

void _TestSuite_Print( void* testSuite, struct Stream* stream ) {
	TestSuite*	self = (TestSuite*)testSuite;

	/* Print parent */
	Journal_Printf( stream, "TestSuite (ptr): (%p)\n", self );
	_Stg_Class_Print( self, stream );
}

void* _TestSuite_Copy( void* testSuite, void* destProc_I, Bool deep, Name nameExt, struct PtrMap* ptrMap ) {
#if 0
	TestSuite*	self = (TestSuite*)testSuite;
	TestSuite*	newTestSuite;
	PtrMap*	map = ptrMap;
	Bool	ownMap = False;

	/* Damn me for making copying so difficult... what was I thinking? */
	
	/* We need to create a map if it doesn't already exist. */
	if( !map ) {
		map = PtrMap_New( 10 );
		ownMap = True;
	}
	
	newTestSuite = (TestSuite*)_Mesh_Copy( self, destProc_I, deep, nameExt, map );
	
	/* Copy the virtual methods here. */

	/* Deep or shallow? */
	if( deep ) {
	}
	else {
	}
	
	/* If we own the map, get rid of it here. */
	if( ownMap ) Stg_Class_Delete( map );
	
	return (void*)newTestSuite;
#endif

	return NULL;
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void TestSuite_SetTests( void* testSuite, unsigned nTests, TestSuite_Test* tests ) {
	TestSuite*	self = (TestSuite*)testSuite;

	assert( self );
	assert( !nTests || tests );

	TestSuite_Destruct( self );

	self->nTests = nTests;
	if( nTests ) {
		self->tests = Memory_Alloc_Array( TestSuite_Test, nTests, "TestSuite::tests" );
		memcpy( self->tests, tests, nTests * sizeof(TestSuite_Test) );
	}
}

void TestSuite_SetProcToWatch( void* testSuite, unsigned watch ) {
	TestSuite*	self = (TestSuite*)testSuite;

	assert( self );
#ifndef NDEBUG
	{
		unsigned	nProcs;

		MPI_Comm_size( MPI_COMM_WORLD, (int*)&nProcs );
		assert( watch < nProcs );
	}
#endif

	self->watch = watch;
}

void TestSuite_Run( void* testSuite ) {
	TestSuite*	self = (TestSuite*)testSuite;
	unsigned	nProcs;
	unsigned	rank;
	unsigned	t_i;

	assert( self );

	MPI_Comm_size( MPI_COMM_WORLD, (int*)&nProcs );
	MPI_Comm_rank( MPI_COMM_WORLD, (int*)&rank );

	for( t_i = 0; t_i < self->nTests; t_i++ ) {
		TestSuite_Test*	test = self->tests + t_i;
		unsigned	success = 0;
		unsigned	failure = 1;
		unsigned	r_i;

		assert( test );
		assert( test->name );
		assert( test->func );

		if( rank == self->watch )
			printf( "   Running test '%s'... ", test->name );

		for( r_i = 0; r_i < test->nReps; r_i++ ) {
			Bool	result = test->func( rank, nProcs, self->watch );

			if( rank == self->watch ) {
				if( result )
					MPI_Bcast( &success, 1, MPI_UNSIGNED, self->watch, MPI_COMM_WORLD );
				else {
					MPI_Bcast( &failure, 1, MPI_UNSIGNED, self->watch, MPI_COMM_WORLD );
					break;
				}
			}
			else {
				unsigned	status;

				MPI_Bcast( &status, 1, MPI_UNSIGNED, self->watch, MPI_COMM_WORLD );
				if( status == failure ) break;
			}
		}

		if( rank == self->watch )
			printf( "%s\n", (r_i == test->nReps) ? "passed" : "failed" );
	}
}

/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

void TestSuite_Destruct( TestSuite* self ) {
	assert( self );

	self->nTests = 0;
	KillArray( self->tests );
}

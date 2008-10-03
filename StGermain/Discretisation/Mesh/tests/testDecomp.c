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
** $Id: testRangeSet.c 2136 2004-09-30 02:47:13Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <mpi.h>

#include "Base/Base.h"
#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Mesh/Mesh.h"


#define nTests	2


Bool testLocals( unsigned rank, unsigned nProcs, unsigned watch ) {
	Decomp*		decomp = Decomp_New( "" );
	unsigned	nLocals = 100;
	unsigned	nGlobals = nProcs * nLocals;
	unsigned*	locals;
	unsigned	l_i;

	locals = Memory_Alloc_Array_Unnamed( unsigned, nLocals );
	for( l_i = 0; l_i < nLocals; l_i++ )
		locals[l_i] = rank * nLocals + l_i;

	Decomp_SetLocals( decomp, nLocals, locals );

	if( decomp->nGlobals != nGlobals ) {
		FreeArray( locals );
		FreeObject( decomp );
		return False;
	}

	for( l_i = 0; l_i < nLocals; l_i++ ) {
		if( decomp->locals[l_i] != locals[l_i] ) {
			FreeArray( locals );
			FreeObject( decomp );
			return False;
		}
	}

	FreeArray( locals );
	FreeObject( decomp );

	return True;
}

Bool testMaps( unsigned rank, unsigned nProcs, unsigned watch ) {
	Decomp*		decomp = Decomp_New( "" );
	unsigned	nLocals = 100;
	unsigned	nGlobals = nProcs * nLocals;
	unsigned*	locals;
	unsigned	l_i, g_i;

	locals = Memory_Alloc_Array_Unnamed( unsigned, nLocals );
	for( l_i = 0; l_i < nLocals; l_i++ )
		locals[l_i] = rank * nLocals + l_i;

	Decomp_SetLocals( decomp, nLocals, locals );

	for( l_i = 0; l_i < nLocals; l_i++ ) {
		if( !Decomp_IsLocal( decomp, locals[l_i] ) || 
		    Decomp_LocalToGlobal( decomp, l_i ) != locals[l_i] )
		{
			FreeObject( decomp );
			return False;
		}
	}

	FreeArray( locals );

	for( g_i = 0; g_i < nGlobals; g_i++ ) {
		if( !Decomp_IsLocal( decomp, g_i ) )
			continue;

		if( Decomp_GlobalToLocal( decomp, g_i ) != g_i % nLocals ) {
			FreeObject( decomp );
			return False;
		}
	}

	FreeObject( decomp );

	return True;
}


int main( int argc, char* argv[] ) {
	TestSuite*	suite;
	TestSuite_Test	tests[nTests] = {{"set locals", testLocals}, 
					 {"mappings", testMaps}};

	/* Initialise MPI, get world info. */
	MPI_Init( &argc, &argv );

	/* Initialise StGermain. */
	BaseFoundation_Init( &argc, &argv );
	BaseIO_Init( &argc, &argv );
	BaseContainer_Init( &argc, &argv );

	/* Create the test suite. */
	suite = TestSuite_New();
	TestSuite_SetProcToWatch( suite, (argc >= 2) ? atoi( argv[1] ) : 0 );
	TestSuite_SetTests( suite, nTests, tests );

	/* Run the tests. */
	TestSuite_Run( suite );

	/* Destroy test suites. */
	FreeObject( suite );

	/* Finalise StGermain. */
	BaseContainer_Finalise();
	BaseIO_Finalise();
	BaseFoundation_Finalise();

	/* Close off MPI */
	MPI_Finalize();

	return MPI_SUCCESS;
}

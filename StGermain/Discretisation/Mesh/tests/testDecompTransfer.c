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
** $Id: testDecompTransfer.c 2136 2004-09-30 02:47:13Z PatrickSunter $
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


Bool testAll( unsigned rank, unsigned nProcs, unsigned watch ) {
	Bool		result = True;
	DecompTransfer*	transfer;
	Decomp*		decomps[2];
	UIntMap*	map;
	unsigned	nLocals[2];
	unsigned*	locals[2];
	unsigned	l_i, ind_i;

	nLocals[0] = 10;
	locals[0] = Memory_Alloc_Array_Unnamed( unsigned, nLocals[0] );
	for( l_i = 0; l_i < nLocals[0]; l_i++ )
		locals[0][l_i] = rank * nLocals[0] + l_i;

	decomps[0] = Decomp_New( "" );
	Decomp_SetLocals( decomps[0], nLocals[0], locals[0] );

	nLocals[1] = 10;
	locals[1] = Memory_Alloc_Array_Unnamed( unsigned, nLocals[1] );
	for( l_i = 0; l_i < nLocals[0]; l_i++ )
		locals[1][l_i] = (nProcs - rank - 1) * nLocals[1] + l_i;

	decomps[1] = Decomp_New( "" );
	Decomp_SetLocals( decomps[1], nLocals[1], locals[1] );

	map = UIntMap_New();
	for( ind_i = 0; ind_i < nLocals[0]; ind_i++ )
		UIntMap_Insert( map, Decomp_GlobalToLocal( decomps[0], locals[0][ind_i] ), locals[1][ind_i] );

	transfer = DecompTransfer_New( "" );
	DecompTransfer_SetDecomps( transfer, decomps[0], decomps[1], map );

	if( rank == watch ) {
	}

done:
	FreeObject( transfer );

	return result;
}


#define nTests	1

TestSuite_Test	tests[nTests] = {{"everything", testAll, 10}};


int main( int argc, char* argv[] ) {
	TestSuite*	suite;

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

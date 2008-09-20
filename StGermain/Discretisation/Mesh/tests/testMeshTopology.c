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
** $Id: testMeshTopology.c 2136 2004-09-30 02:47:13Z PatrickSunter $
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


Bool testSetElements( unsigned rank, unsigned nProcs, unsigned watch );
void buildElements( unsigned rank, unsigned nProcs, 
		    unsigned* nEls, unsigned** els );


#define nTests	1

TestSuite_Test	tests[nTests] = {{"set elements", testSetElements}};


Bool testSetElements( unsigned rank, unsigned nProcs, unsigned watch ) {
	MeshTopology*	topo;
	unsigned	nDims = 3;
	unsigned	nEls = 0;
	unsigned*	els = NULL;

	buildElements( rank, nProcs, &nEls, &els );
	topo = MeshTopology_New( "" );
	MeshTopology_SetNDims( topo, nDims );
	MeshTopology_SetElements( topo, MT_VERTEX, nEls, els );

	if( rank == watch ) {
		if( topo->nDims != 3 || topo->nTDims != 4 || 
		    !topo->domains || !topo->domains[MT_VERTEX] || !topo->domains[MT_VERTEX]->decomp )
		{
			FreeArray( els );
			FreeObject( topo );
			return False;
		}
	}

	FreeArray( els );
	FreeObject( topo );

	return True;
}

void buildElements( unsigned rank, unsigned nProcs, 
		    unsigned* nEls, unsigned** els )
{
	unsigned	start;
	unsigned	e_i;

	*nEls = 100;
	*els = Memory_Alloc_Array_Unnamed( unsigned, *nEls );
	start = rank * (*nEls - 10);

	for( e_i = 0; e_i < *nEls; e_i++ )
		(*els)[e_i] = start + e_i;
}


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

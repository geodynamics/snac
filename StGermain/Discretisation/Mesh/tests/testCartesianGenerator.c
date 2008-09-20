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


Bool testSetParams( unsigned rank, unsigned nProcs, unsigned watch ) {
	CartesianGenerator*	gen;
	unsigned		sizes[3];

	sizes[0] = 5;
	sizes[1] = 2;
	sizes[2] = 2;

	gen = CartesianGenerator_New( "" );
	CartesianGenerator_SetTopologyParams( gen, 3, sizes, 0, NULL, NULL );

	FreeObject( gen );

	return True;
}

Bool testGen( unsigned rank, unsigned nProcs, unsigned watch ) {
	CartesianGenerator*	gen;
	Mesh*			mesh;
	unsigned		sizes[3];
	double			min[3] = {0, 0, 0};
	double			max[3] = {1, 1, 1};

	sizes[0] = 12;
	sizes[1] = 12;
	sizes[2] = 12;

	gen = CartesianGenerator_New( "" );
	mesh = Mesh_DefaultNew( "" );
	mesh->topo = MeshTopology_New( "" );
	mesh->info = ExtensionManager_New_OfExistingObject( "mesh_info", mesh );
	CartesianGenerator_SetTopologyParams( gen, 3, sizes, 0, NULL, NULL );
	CartesianGenerator_SetGeometryParams( gen, min, max );
	CartesianGenerator_Generate( gen, mesh );

	FreeObject( gen );

	return True;
}


#define nTests	2

TestSuite_Test	tests[nTests] = {{"set parameters", testSetParams, 0}, 
				 {"generate mesh", testGen, 1}};


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

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
** $Id: testDimensionMacros.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"

#include <stdio.h>

void testDimensionMacros_DoOneTest( IJK coord, IJK meshSize );

int main( int argc, char* argv[] ) {
	IJK coord;
	IJK meshSize;
	Index i, j, k;
	MPI_Comm CommWorld;
	int rank;
	int numProcessors;
	
	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size( CommWorld, &numProcessors );
	MPI_Comm_rank( CommWorld, &rank );
	
	Base_Init( &argc, &argv );
	DiscretisationGeometry_Init( &argc, &argv );
	MPI_Barrier( CommWorld ); /* Ensures copyright info always come first in output */
	
	printf( "+++ 1D Tests +++\n\n" );
	coord[I_AXIS] = 3; coord[J_AXIS] = 0; coord[K_AXIS] = 0;
	meshSize[I_AXIS] = 8; meshSize[J_AXIS] = 0; meshSize[K_AXIS] = 0;
	testDimensionMacros_DoOneTest( coord, meshSize );
	
	coord[I_AXIS] = 0; coord[J_AXIS] = 3; coord[K_AXIS] = 0;
	meshSize[I_AXIS] = 0; meshSize[J_AXIS] = 8; meshSize[K_AXIS] = 0;
	testDimensionMacros_DoOneTest( coord, meshSize );
	
	coord[I_AXIS] = 0; coord[J_AXIS] = 0; coord[K_AXIS] = 3;
	meshSize[I_AXIS] = 0; meshSize[J_AXIS] = 0; meshSize[K_AXIS] = 8;
	testDimensionMacros_DoOneTest( coord, meshSize );

	printf( "\n+++ 2D Tests +++\n\n" );
	coord[I_AXIS] = 3; coord[J_AXIS] = 4; coord[K_AXIS] = 0;
	meshSize[I_AXIS] = 8; meshSize[J_AXIS] = 8; meshSize[K_AXIS] = 0;
	testDimensionMacros_DoOneTest( coord, meshSize );
	
	coord[I_AXIS] = 3; coord[J_AXIS] = 0; coord[K_AXIS] = 4;
	meshSize[I_AXIS] = 8; meshSize[J_AXIS] = 0; meshSize[K_AXIS] = 8;
	testDimensionMacros_DoOneTest( coord, meshSize );
	
	coord[I_AXIS] = 0; coord[J_AXIS] = 3; coord[K_AXIS] = 4;
	meshSize[I_AXIS] = 0; meshSize[J_AXIS] = 8; meshSize[K_AXIS] = 8;
	testDimensionMacros_DoOneTest( coord, meshSize );
	
	printf( "\n+++ 3D Tests +++\n\n" );
	meshSize[I_AXIS] = 3; meshSize[J_AXIS] = 4; meshSize[K_AXIS] = 5;
	for ( k=0; k < meshSize[K_AXIS]; k++ ) {
		for ( j=0; j < meshSize[J_AXIS]; j++ ) {
			for ( i=0; i < meshSize[I_AXIS]; i++ ) {
				coord[I_AXIS] = i; coord[J_AXIS] = j; coord[K_AXIS] = k;
				testDimensionMacros_DoOneTest( coord, meshSize );
			}
		}
	}	
	
	DiscretisationGeometry_Finalise();
	Base_Finalise();
	MPI_Finalize();
	return 0;
}

void testDimensionMacros_DoOneTest( IJK coord, IJK meshSize ) {
	IJK newCoord;
	Index element_I;
	
	printf( "(%d,%d,%d) in mesh sized %d*%d*%d -> ",
		coord[0], coord[1], coord[2],
		meshSize[0], meshSize[1], meshSize[2] );
	
	Dimension_3DTo1D( coord, meshSize, &element_I );
	printf( "index %d\n", element_I );
	Dimension_1DTo3D( element_I, meshSize, newCoord );
	printf( "which maps back to (%d,%d,%d)\n", 
		newCoord[0], newCoord[1], newCoord[2] );
}

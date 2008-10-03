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
** $Id: testRMatrix.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"

#include <stdio.h>


void RMatrix_Print( RMatrix mat );


int main( int argc, char* argv[] ) {
	RMatrix		matrix;
	Coord		xz = { 1.0, 0.0, 1.0 };
	MPI_Comm	CommWorld;
	int		rank;
	int		numProcessors;
	
	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size( CommWorld, &numProcessors );
	MPI_Comm_rank( CommWorld, &rank );
	
	Base_Init( &argc, &argv );
	
	DiscretisationGeometry_Init( &argc, &argv );
	MPI_Barrier( CommWorld ); /* Ensures copyright info always come first in output */
	
	RMatrix_LoadIdentity( matrix );
	printf( "\n" ); RMatrix_Print( matrix );
	RMatrix_ApplyRotationX( matrix, M_PI / 2.0 );
	printf( "\n" ); RMatrix_Print( matrix );
	RMatrix_VectorMult( xz, matrix, xz );
	printf( "\n\t%g, %g, %g\n", xz[0], xz[1], xz[2] );
	
	RMatrix_LoadIdentity( matrix );
	RMatrix_ApplyRotationY( matrix, M_PI / 2.0 );
	printf( "\n" ); RMatrix_Print( matrix );
	RMatrix_VectorMult( xz, matrix, xz );
	printf( "\n\t%g, %g, %g\n", xz[0], xz[1], xz[2] );
	
	RMatrix_LoadIdentity( matrix );
	RMatrix_ApplyRotationZ( matrix, M_PI / 2.0 );
	printf( "\n" ); RMatrix_Print( matrix );
	RMatrix_VectorMult( xz, matrix, xz );
	printf( "\n\t%g, %g, %g\n", xz[0], xz[1], xz[2] );
	
	RMatrix_LoadIdentity( matrix );
	RMatrix_ApplyRotationX( matrix, M_PI / 2.0 );
	RMatrix_ApplyRotationY( matrix, M_PI / 2.0 );
	RMatrix_ApplyRotationZ( matrix, M_PI / 2.0 );
	printf( "\n" ); RMatrix_Print( matrix );
	RMatrix_VectorMult( xz, matrix, xz );
	printf( "\n\t%g, %g, %g\n", xz[0], xz[1], xz[2] );
	
	DiscretisationGeometry_Finalise();
	
	Base_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return EXIT_SUCCESS;
}


void RMatrix_Print( RMatrix mat ) {
	unsigned		i, j;
	
	for( j = 0; j < 3; j++ ) {
		for( i = 0; i < 3; i++ )
			printf( "\t%g", mat[j * 3 + i] );
		printf( "\n" );
	}
}

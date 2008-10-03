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
**	Robert B. Turnbull, Monash Cluster Computing. (Robert.Turnbull@sci.monash.edu.au)
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
** $Id: testTrigMath.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"

#include <stdio.h>



int main( int argc, char* argv[] ) {
	MPI_Comm CommWorld;
	int rank;
	int numProcessors;
	int procToWatch;
	
	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size( CommWorld, &numProcessors );
	MPI_Comm_rank( CommWorld, &rank );
	
	Base_Init( &argc, &argv );
	
	DiscretisationGeometry_Init( &argc, &argv );
	MPI_Barrier( CommWorld ); /* Ensures copyright info always come first in output */
	
	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}

	if( rank == procToWatch ) {
		double angle;
		double rectOriginal[] = {2.4,5,-10};
		double spherical[3];
		double rectangular[3];
		Index dim;
		Stream* stream = Journal_Register( Info_Type, __FILE__ );


		printf("\n****************************\n");
		printf("Test angle conversion macros\n");
		printf("39 degrees in radians = %2.3f\n", StGermain_DegreeToRadian( 37.0 + 3 ) );
		printf("2.468 radians in degrees = %2.3f\n", StGermain_RadianToDegree( 1.234 * 2 ) );

		printf("\n****************************\n");
		printf("Test domain finding function\n");
		angle = 1.5;
		printf("Angle %lf is equivalent to %lf\n", angle, StGermain_TrigDomain(angle) );
		printf("Sine test: %lf = %lf\n", sin(angle), sin( StGermain_TrigDomain(angle) ) );
		angle = -1.2;
		printf("Angle %lf is equivalent to %lf\n", angle, StGermain_TrigDomain(angle) );
		printf("Sine test: %lf = %lf\n", sin(angle), sin( StGermain_TrigDomain(angle) ) );
		angle = 20.0;
		printf("Angle %lf is equivalent to %lf\n", angle, StGermain_TrigDomain(angle) );
		printf("Sine test: %lf = %lf\n", sin(angle), sin( StGermain_TrigDomain(angle) ) );

		printf("\n****************************\n");
		printf("Test quadrant finding function\n");
		angle = 45;
		printf("Angle %lf degrees is in quadrant %d\n", angle, StGermain_TrigQuadrant( StGermain_DegreeToRadian(angle) ));
		angle = 120;
		printf("Angle %lf degrees is in quadrant %d\n", angle, StGermain_TrigQuadrant( StGermain_DegreeToRadian(angle) ));
		angle = 195;
		printf("Angle %lf degrees is in quadrant %d\n", angle, StGermain_TrigQuadrant( StGermain_DegreeToRadian(angle) ));
		angle = 340;
		printf("Angle %lf degrees is in quadrant %d\n", angle, StGermain_TrigQuadrant( StGermain_DegreeToRadian(angle) ));
		angle = 730;
		printf("Angle %lf degrees is in quadrant %d\n", angle, StGermain_TrigQuadrant( StGermain_DegreeToRadian(angle) ));
		angle = -135;
		printf("Angle %lf degrees is in quadrant %d\n", angle, StGermain_TrigQuadrant( StGermain_DegreeToRadian(angle) ));
		
		printf("\n****************************\n");
		printf("Test coordinate conversion functions 2D\n");
		dim = 2;
		StGermain_PrintNamedVector( stream, rectOriginal, dim );
		StGermain_RectangularToSpherical( spherical, rectOriginal, dim );
		StGermain_PrintNamedVector( stream, spherical, dim );
		StGermain_SphericalToRectangular( rectangular, spherical, dim );
		StGermain_PrintNamedVector( stream, rectangular, dim );

		printf("\n****************************\n");
		printf("Test coordinate conversion functions 3D\n");
		dim = 3;
		StGermain_PrintNamedVector( stream, rectOriginal, dim );
		StGermain_RectangularToSpherical( spherical, rectOriginal, dim );
		StGermain_PrintNamedVector( stream, spherical, dim );
		StGermain_SphericalToRectangular( rectangular, spherical, dim );
		StGermain_PrintNamedVector( stream, rectangular, dim );
	}
	
	printf("\n");
	DiscretisationGeometry_Finalise();
	
	Base_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0;
}

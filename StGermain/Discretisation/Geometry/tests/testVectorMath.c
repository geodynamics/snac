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
** $Id: testVectorMath.c 3926 2007-01-02 04:53:16Z KathleenHumble $
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
		Coord	a, b, c;
		Coord	d = { 1.0, 1.0, 1.0 };
		Coord	e = { 1.0, 2.0, -3.0 };
		Index	i;

		printf( "Basic tests:\n" );
		printf( "d = { %g, %g, %g }\n", d[0], d[1], d[2] );
		Vector_SetScalar( d, 2.0, 0.5, 1.5 );
		printf( "d = { %g, %g, %g }\n", d[0], d[1], d[2] );

		Vector_Set( c, d );
		printf( "c = d = { %g, %g, %g }\n", c[0], c[1], c[2] );

		Vector_Add( b, c, d );
		printf( "b = c + d = {%g, %g, %g}\n", b[0], b[1], b[2] );

		Vector_Sub( a, d, b );
		printf( "a = d - b = {%g, %g, %g}\n", a[0], a[1], a[2] );

		Vector_Cross( d, a, e );
		printf( "d = a x e = {%g, %g, %g}\n", d[0], d[1], d[2] );

		printf( "a . c = %g\n", Vector_Dot( a, c ) );

		Vector_Mult( b, b, 2.0 );
		printf( "b = 2b = { %g, %g, %g }\n", b[0], b[1], b[2] );

		Vector_Norm( b, b );
		printf( "b^ = { %g, %g, %g }\n", b[0], b[1], b[2] );

		printf( "Complete test:\n" );
		for( i = 1; i <= 10; i++ )
		{
			Index	j;

			a[0] = 10.0 / (double)i;
			b[0] = 30.0 / (double)i;

			for( j = 1; j <= 10; j++ )
			{
				Index	k;

				a[1] = 10.0 / (double)j;
				b[1] = 30.0 / (double)j;

				for( k = 1; k <= 10; k++ )
				{
					a[2] = 10.0 / (double)k;
					b[2] = 30.0 / (double)k;

					Vector_Proj( c, a, b );

					printf( "{ %g, %g, %g }\n", c[0], c[1], c[2] );
				}
			}
		}
		Vector_SetScalar2D( a, 5.0, 9.0);
		printf( "2D: { %g, %g }\n", a[0], a[1]);
		

	}
	if( rank == procToWatch ) {
		double i[] = {1.0,0.0,0.0};
		double j[] = {0.0,1.0,0.0};
		double k[] = {0.0,0.0,1.0};
		double A[] = { 7.4, 2  ,   5  ,  1,  3 ,   -42};
		double B[] = { 4  , 2.3,   5.8,  6, -12, 39289};
		double C[] = {23  , 5  , -14  , 32, -21,    78};
		double D[] = {23  , 5  , -14  , 32, -21,    78};
		double angle;
		double **matrix;
		double vector[6];
		double *coordList[4];
		int d;
		Stream* stream = Journal_Register( Info_Type, "VectorMath" );
		
		coordList[0] = A;
		coordList[1] = B;
		coordList[2] = C;
		coordList[3] = D;

		/* Check Rotation functions */
		printf("\n****************************\n");
		angle = 1.0;
		StGermain_RotateCoordinateAxis( vector, k, I_AXIS, angle );
		printf("K Rotated %2.3f degrees around I axis -  %2.3f %2.3f %2.3f\n", angle, vector[0], vector[1], vector[2] );
		StGermain_RotateVector( vector, k, i, angle );
		printf("K Rotated %2.3f degrees around I axis -  %2.3f %2.3f %2.3f\n", angle, vector[0], vector[1], vector[2] );
		printf("Angle between vectors = %2.3f\n", StGermain_AngleBetweenVectors( vector, k, 3 ));

		angle = 2.3;
		StGermain_RotateCoordinateAxis( vector, i, J_AXIS, angle );
		printf("I Rotated %2.3f degrees around J axis -  %2.3f %2.3f %2.3f\n", angle, vector[0], vector[1], vector[2] );
		StGermain_RotateVector( vector, i, j, angle );
		printf("I Rotated %2.3f degrees around J axis -  %2.3f %2.3f %2.3f\n", angle, vector[0], vector[1], vector[2] );
		printf("Angle between vectors = %2.3f\n", StGermain_AngleBetweenVectors( vector, i, 3 ));

		angle = 1.8;
		StGermain_RotateCoordinateAxis( vector, j, K_AXIS, ( angle ) );
		printf("J Rotated %2.3f degrees around K axis -  %2.3f %2.3f %2.3f\n", angle, vector[0], vector[1], vector[2] );
		StGermain_RotateVector( vector, j, k, ( angle ) );
		printf("J Rotated %2.3f degrees around K axis -  %2.3f %2.3f %2.3f\n", angle, vector[0], vector[1], vector[2] );
		printf("Angle between vectors = %2.3f\n", StGermain_AngleBetweenVectors( vector, j, 3 ));
		
		/* Check addition function */
		printf("\n****************************\n");
		printf("vector = A + B\n");
		for ( d = 0 ; d <= 6 ; d++ ) {
			StGermain_VectorAddition( vector, A, B, d );
			StGermain_PrintNamedVector( stream, vector, d );
		}

		/* Check subtraction function */
		printf("\n****************************\n");
		printf("vector = A - B\n");
		for ( d = 0 ; d <= 6 ; d++ ) {
			StGermain_VectorSubtraction( vector, A, B, d );
			StGermain_PrintNamedVector( stream, vector, d );
		}
	
		/* Check Magnitude Function */
		printf("\n****************************\n");
		printf("Check Magnitude Function\n");
		for ( d = 0 ; d <= 6 ; d++ ) {
			printf("dim = %d magnitude = %2.3f\n", d, StGermain_VectorMagnitude( A, d ) );
			printf("dim = %d magnitude = %2.3f\n", d, StGermain_VectorMagnitude( B, d ) );
		}

		/* Check Dot Product */
		printf("\n****************************\n");
		printf("Check Dot Product Function\n");
		for ( d = 0 ; d <= 6 ; d++ ) 
			printf("dim = %d dot product = %2.3f\n", d, StGermain_VectorDotProduct( A, B, d ) );

		/* Check Cross Product */
		/* Tested against http://www.engplanet.com/redirect.html?3859 */
		printf("\n****************************\n");
		printf("Check Cross Product Function\n");
		StGermain_VectorCrossProduct( vector, A, B );
		StGermain_PrintNamedVector( stream, vector, 3 );

		/* Checking distance between points function */
		printf("\n****************************\n");
		printf("Checking distance between points Function\n");
		for ( d = 0 ; d <= 6 ; d++ ) 
			printf("dim = %d distance = %2.3f\n", d, StGermain_DistanceBetweenPoints( A, B, d ) );
		
		/* Checking normal to plane function */
		printf("\n****************************\n");
		printf("Checking normal to plane function function\n");
		vector[0] = vector[1] = vector[2] = vector[3] = vector[4] = vector[5] = 0.0;
		StGermain_NormalToPlane( vector, A, B, C );
		StGermain_PrintNamedVector( stream, vector, 3 );

		/* Checking centroid function */
		printf("\n****************************\n");
		printf("Checking centroid function\n");
		for ( d = 0 ; d <= 6 ; d++ ) {
			vector[0] = vector[1] = vector[2] = vector[3] = vector[4] = vector[5] = 0.0;
			StGermain_TriangleCentroid( vector, A, B, C, d );
			StGermain_PrintNamedVector( stream, vector, d );
		}

		/* Check Triangle Area */
		printf("\n****************************\n");
		printf("Check Triangle Area Function\n");
		for ( d = 0 ; d <= 6 ; d++ ) 
			printf("dim = %d Triangle Area = %2.3f\n", d, StGermain_TriangleArea( A, B, C, d ) );

		/* Check Normalisation Function */
		printf("\n****************************\n");
		printf("Check Normalisation Function\n");
		d = 2;
		StGermain_VectorNormalise( A, d );
		StGermain_PrintNamedVector( stream, A, d);
		printf("mag = %2.3f\n", StGermain_VectorMagnitude( A, d ) );

		d = 3;
		StGermain_VectorNormalise( B, d );
		StGermain_PrintNamedVector( stream, B, d);
		printf("mag = %2.3f\n", StGermain_VectorMagnitude( B, d ) );

		d = 5;
		StGermain_VectorNormalise( C, d );
		StGermain_PrintNamedVector( stream, C, d);
		printf("mag = %2.3f\n", StGermain_VectorMagnitude( C, d ) );

		printf("\n****************************\n");
		printf("Check StGermain_VectorCrossProductMagnitude\n");
		A[0] = 1.0; A[1] = 2.0 ; A[2] = 3.0;
		B[0] = 4.0; B[1] = 5.0 ; B[2] = 6.0;
		StGermain_PrintNamedVector( stream, A, 3);
		StGermain_PrintNamedVector( stream, B, 3);
		printf("mag = %2.3g (2D)\n", StGermain_VectorCrossProductMagnitude( A, B, 2 ) );
		printf("mag = %2.3g (3D)\n", StGermain_VectorCrossProductMagnitude( A, B, 3 ) );

		printf("\n****************************\n");
		printf("Check StGermain_ConvexQuadrilateralArea\n");
		A[0] = 0.0; A[1] = 0.0 ; A[2] = 0.0;
		B[0] = 0.0; B[1] = 4.0 ; B[2] = 2.3;
		C[0] = 1.2; C[1] = 4.0 ; C[2] = 2.3;
		D[0] = 1.2; D[1] = 0.0 ; D[2] = 0.0;
		printf("area = %2.3g (2D)\n", StGermain_ConvexQuadrilateralArea( A, B, C, D, 2 ) );
		printf("area = %2.3g (3D)\n", StGermain_ConvexQuadrilateralArea( A, B, C, D, 3 ) );
		
		printf("\n****************************\n");
		printf("Check StGermain_ScalarTripleProduct \n");
		matrix = Memory_Alloc_2DArray( double, 3, 3, "matrix" );
		matrix[0][0] = 1.0; matrix[0][1] = 2.0 ; matrix[0][2] = 3.0;
		matrix[1][0] = 4.0; matrix[1][1] = 5.0 ; matrix[1][2] = 6.0;
		matrix[2][0] = 7.0; matrix[2][1] = 8.0 ; matrix[2][2] = 11.0;
		printf("scalar triple product = %2.3g\n", StGermain_ScalarTripleProduct( matrix[0], matrix[1], matrix[2] ));
		printf("scalar triple product = %2.3g\n", StGermain_ScalarTripleProduct( matrix[2], matrix[0], matrix[1] ));
		printf("scalar triple product = %2.3g\n", StGermain_ScalarTripleProduct( matrix[1], matrix[2], matrix[0] ));
		printf("scalar triple product = %2.3g\n", StGermain_MatrixDeterminant( matrix , 3 ));
		Memory_Free( matrix );
		
		printf("\n****************************\n");
		printf("Check StGermain_ParallelepipedVolume \n");
		A[0] = 0.0; A[1] = 0.0 ; A[2] = 0.0;
		B[0] = 1.1; B[1] = 0.0 ; B[2] = 0.0;
		C[0] = 0.2; C[1] = 1.7 ; C[2] = 0.0;
		D[0] = 0.0; D[1] = 0.0 ; D[2] = 1.3;
		printf("volume = %2.3g\n", StGermain_ParallelepipedVolume( A, B, C, D ));

		printf("\n****************************\n");
		printf("Check StGermain_AverageCoord \n");
		A[0] = 1.0;  A[1] = 2.2 ; A[2] = 3.2;
		B[0] = 41.0; B[1] = 5.0 ; B[2] = 6.9;
		C[0] = -7.0; C[1] = 8.7 ; C[2] = 11.0;

		StGermain_AverageCoord( D, coordList, 3, 3 );
		StGermain_PrintNamedVector( stream, D, 3);



	}
	
	printf("\n");
	DiscretisationGeometry_Finalise();
	
	Base_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0;
}

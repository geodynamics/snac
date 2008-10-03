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
**	Kathleen M. Humble, Computational Scientist, VPAC. (khumble@vpac.org)
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
** $Id: testVectorMath.c 3462 2006-02-19 06:53:24Z WalterLandry $
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
	Stream* stream;
	
	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size( CommWorld, &numProcessors );
	MPI_Comm_rank( CommWorld, &rank );
	
	Base_Init( &argc, &argv );
	
	DiscretisationGeometry_Init( &argc, &argv );
	MPI_Barrier( CommWorld ); /* Ensures copyright info always come first in output */
	
	stream = Journal_Register( Info_Type, "ComplexVectorMath" );
	
	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}
	if( rank == procToWatch ) {
		CoordC	a, b, c;
		CoordC	d = { {1.0, 1.0}, {1.0, 0.0}, {0.0, 1.0} };
		CoordC	e = { {1.0, 0.0}, {2.0, 2.0}, {-3.0, -1.0} };
		Cmplx value = {1.0, 1.0}; 
		Cmplx c1 = {1.0, 0.0};
		Cmplx c2 = {2.0, 1.0};
		Cmplx c3 = {1.5, 1.0};
		
		Journal_Printf( stream,  "Basic tests:\n" );
		Journal_Printf( stream, "d = \n");
		Journal_PrintCmplx( stream, d[0]);
		Journal_PrintCmplx( stream, d[1]);
		Journal_PrintCmplx( stream, d[2]);
		 
		Journal_Printf( stream, "Set Complex Scalar\n");
		ComplexVector_SetScalar( c1, c2, c3, d );
		Journal_Printf( stream, "d = \n");
		Journal_PrintCmplx( stream, d[0]);
		Journal_PrintCmplx( stream, d[1]);
		Journal_PrintCmplx( stream, d[2]);

		Journal_Printf( stream, "Set c = d\n");
		ComplexVector_Set( d, c );
		Journal_Printf( stream, "c = \n");
		Journal_PrintCmplx( stream, c[0]);
		Journal_PrintCmplx( stream, c[1]);
		Journal_PrintCmplx( stream, c[2]);

		ComplexVector_Add(c, d, b );
		Journal_Printf( stream,  "b = c + d \n");
		Journal_Printf( stream, "b = \n");
		Journal_PrintCmplx( stream, b[0]);
		Journal_PrintCmplx( stream, b[1]);
		Journal_PrintCmplx( stream, b[2]);

		ComplexVector_Sub( b, d, a );
		Journal_Printf( stream,  "a = b - d \n");
		Journal_Printf( stream, "a = \n");
		Journal_PrintCmplx( stream, a[0]);
		Journal_PrintCmplx( stream, a[1]);
		Journal_PrintCmplx( stream, a[2]);

		ComplexVector_Cross( a, e, d );		
		Journal_Printf( stream,  "d = a x e \n");
		Journal_Printf( stream, "e = \n");
		Journal_PrintCmplx( stream, e[0]);
		Journal_PrintCmplx( stream, e[1]);
		Journal_PrintCmplx( stream, e[2]);		
		Journal_Printf( stream, "d = \n");
		Journal_PrintCmplx( stream, d[0]);
		Journal_PrintCmplx( stream, d[1]);
		Journal_PrintCmplx( stream, d[2]);
		
		ComplexVector_Dot( a, e, value );
		Journal_Printf( stream,  "value = a . e \n");
		Journal_PrintCmplx( stream, value);
		
		value[REAL_PART] = 2.0;
		value[IMAG_PART] = 1.0;
		Journal_Printf( stream, "b = \n");
		Journal_PrintCmplx( stream, b[0]);
		Journal_PrintCmplx( stream, b[1]);
		Journal_PrintCmplx( stream, b[2]);		
		ComplexVector_Mult( b, value, b);
		Journal_Printf( stream,  "b = (2 + i) * b \n");
		Journal_Printf( stream, "b = \n");
		Journal_PrintCmplx( stream, b[0]);
		Journal_PrintCmplx( stream, b[1]);
		Journal_PrintCmplx( stream, b[2]);
		
		ComplexVector_MultReal(b, 3.0, b);
		Journal_Printf( stream,  "b = 3 * b \n");
		Journal_Printf( stream, "b = \n");
		Journal_PrintCmplx( stream, b[0]);
		Journal_PrintCmplx( stream, b[1]);
		Journal_PrintCmplx( stream, b[2]);	

		b[0][REAL_PART] = 0.0; b[0][IMAG_PART] = 1.0; 
		b[1][REAL_PART] = 1.0; b[1][IMAG_PART] = 1.0;
		b[2][REAL_PART] = 0.0; b[2][IMAG_PART] = 1.0;
		
		Journal_Printf( stream,  "b = \n");
		Journal_PrintCmplx( stream, b[0]);
		Journal_PrintCmplx( stream, b[1]);
		Journal_PrintCmplx( stream, b[2]);

		Journal_Printf( stream,  "|b| = %g\n", ComplexVector_Mag(b));
				
		ComplexVector_Proj(a, b, d);
		Journal_Printf( stream,  "d = proj a onto b \n");
		Journal_Printf( stream, "d = \n");
		Journal_PrintCmplx( stream, d[0]);
		Journal_PrintCmplx( stream, d[1]);
		Journal_PrintCmplx( stream, d[2]);			
		
		ComplexVector_Div(a, value, e);
		Journal_Printf( stream,  "e = a / value \n");
		Journal_PrintCmplx( stream, value);
		Journal_Printf( stream, "e = \n");
		Journal_PrintCmplx( stream, e[0]);
		Journal_PrintCmplx( stream, e[1]);
		Journal_PrintCmplx( stream, e[2]);

		Journal_Printf( stream, "b = \n");
		Journal_PrintCmplx( stream, b[0]);
		Journal_PrintCmplx( stream, b[1]);
		Journal_PrintCmplx( stream, b[2]);
		ComplexVector_Norm( b, b );
		Journal_Printf( stream,  "Norm(b) = \n");
		Journal_PrintCmplx( stream, b[0]);
		Journal_PrintCmplx( stream, b[1]);
		Journal_PrintCmplx( stream, b[2]);
	

		Journal_Printf( stream,  "|b| = %g\n", ComplexVector_Mag(b));
		
		Journal_Printf( stream,  "a  = \n");
		Journal_PrintCmplx( stream, a[0]);
		Journal_PrintCmplx( stream, a[1]);
		Journal_PrintCmplx( stream, a[2]);
		
		ComplexVector_Swizzle(a, K_AXIS, I_AXIS, J_AXIS, a);
		Journal_Printf( stream,  "swizzle(a)(k, i, j) = \n");
		Journal_PrintCmplx( stream, a[0]);
		Journal_PrintCmplx( stream, a[1]);
		Journal_PrintCmplx( stream, a[2]);
		

	}
	if( rank == procToWatch ) {
		#define STG_COMPLEXVECTOR_TOL 1e-16;
		
		Cmplx i[] = {{1.00000000, 0.000000000},{0.00000000, 0.000000000},{0.000000000, 0.00000000}};
		Cmplx j[] = {{0.00000000, 0.00000000},{1.0000000, 0.00000000},{0.00000000, 0.0000000}};
		Cmplx k[] = {{0.00000000, 0.000000000},{0.00000000, 0.000000000},{1.000000000, 0.000000000}};
		Cmplx A[] = {{7.4, 1.0}, {  2, 0.0}, {  5, 1.0}, { 1, 0.0}, {  3, 2.0}, {  -42, 0.0}};
		Cmplx B[] = {{  4, 2.0}, {2.3, 0.0}, {5.8, 0.0}, { 6, 0.0}, {-12, 0.0}, {39289, 0.0}};
		Cmplx C[] = {{23, 0.0}, {  5, 0.0}, {-14, 0.0}, {32, 0.0}, {-21, 1.0}, {	78, 0.0}};
		Cmplx D[] = {{23, 0.0}, {  5, 0.0}, {-14, 0.0}, {32, 0.0}, {-21, 0.0}, {   78, 0.0}};
		double angle;
		Cmplx **matrix;
		Cmplx vector[6], differenceVector[6];
		Cmplx *coordList[4];
		int d;
		double realVector[3], tolerance;
		Cmplx dotProductResult;
		Cmplx value;
		
		tolerance = STG_COMPLEXVECTOR_TOL;
		
		coordList[0] = A;
		coordList[1] = B;
		coordList[2] = C;
		coordList[3] = D;
		Journal_Printf( stream, "\n****************************\n");
		Journal_Printf(stream, "Vectors - A, B, C, and D\n");
		
		StGermain_PrintNamedComplexVector( stream, A, 6 );
		StGermain_PrintNamedComplexVector( stream, B, 6 );
		StGermain_PrintNamedComplexVector( stream, C, 6 );
		StGermain_PrintNamedComplexVector( stream, D, 6 );

		/* Check Rotation functions */
		Journal_Printf( stream, "\n****************************\n");

		StGermain_PrintNamedComplexVector( stream, i, 3 );
		StGermain_PrintNamedComplexVector( stream, j, 3 );
		StGermain_PrintNamedComplexVector( stream, k, 3 );

		
		angle = M_PI / 2.0;
	
		
		Journal_Printf(stream, "Axis Rotation\n");				
		StGermain_RotateCoordinateAxisComplex( k, I_AXIS, angle, vector ) ;
		Journal_Printf( stream, "K Rotated %g radians around I axis - \n", angle);
		Cmplx_Subtract(vector[0], j[0], differenceVector[0]);
		Cmplx_Subtract(vector[1], j[1], differenceVector[1]);
		Cmplx_Subtract(vector[2], j[2], differenceVector[2]);
		
		if ( (Cmplx_Modulus(differenceVector[0]) < tolerance) && 
			 (Cmplx_Modulus(differenceVector[1]) < tolerance) &&
			 (Cmplx_Modulus(differenceVector[2]) < tolerance) ) {
			Journal_Printf( stream, "Answer within tolerance %g of expected result: ", tolerance);
			StGermain_PrintNamedComplexVector( stream, j, 3);
		}
		else {
			Journal_Printf( stream, "Answer not within tolerance %g of expected result: ", tolerance);
			StGermain_PrintNamedComplexVector( stream, j, 3);
		}
		
		Journal_Printf(stream, "Angle Rotation\n");		
		StGermain_RotateComplexVector(k, angle,0.0, 0.0, vector);
		Journal_Printf( stream, "K Rotated %g radians around I axis - \n", angle); 
		Cmplx_Subtract(vector[0], j[0], differenceVector[0]);
		Cmplx_Subtract(vector[1], j[1], differenceVector[1]);
		Cmplx_Subtract(vector[2], j[2], differenceVector[2]);
		
		if ( (Cmplx_Modulus(differenceVector[0]) < tolerance) && 
			 (Cmplx_Modulus(differenceVector[1]) < tolerance) &&
			 (Cmplx_Modulus(differenceVector[2]) < tolerance) ) {
			Journal_Printf( stream, "Answer within tolerance %g of expected result: ", tolerance);
			StGermain_PrintNamedComplexVector( stream, j, 3);
		}
		else {
			Journal_Printf( stream, "Answer not within tolerance %g of expected result: ", tolerance);
			StGermain_PrintNamedComplexVector( stream, j, 3);
		}
		
		Journal_Printf(stream, "Axis Rotation\n");		
		StGermain_RotateCoordinateAxisComplex( i, J_AXIS, angle, vector );
		Journal_Printf( stream, "I Rotated %g radians around J axis - \n", angle); 
		Cmplx_Subtract(vector[0], k[0], differenceVector[0]);
		Cmplx_Subtract(vector[1], k[1], differenceVector[1]);
		Cmplx_Subtract(vector[2], k[2], differenceVector[2]);
		
		if ( (Cmplx_Modulus(differenceVector[0]) < tolerance) && 
			 (Cmplx_Modulus(differenceVector[1]) < tolerance) &&
			 (Cmplx_Modulus(differenceVector[2]) < tolerance) ) {
			Journal_Printf( stream, "Answer within tolerance %g of expected result: ", tolerance);
			StGermain_PrintNamedComplexVector( stream, k, 3);
		}
		else {
			Journal_Printf( stream, "Answer not within tolerance %g of expected result: ", tolerance);
			StGermain_PrintNamedComplexVector( stream, k, 3);
		}

		
		Journal_Printf(stream, "Angle Rotation\n");
		StGermain_RotateComplexVector(i, 0.0, angle, 0.0, vector );
		Journal_Printf( stream, "I Rotated %g radians around J axis - \n", angle); 
		Cmplx_Subtract(vector[0], k[0], differenceVector[0]);
		Cmplx_Subtract(vector[1], k[1], differenceVector[1]);
		Cmplx_Subtract(vector[2], k[2], differenceVector[2]);
		
		if ( (Cmplx_Modulus(differenceVector[0]) < tolerance) && 
			 (Cmplx_Modulus(differenceVector[1]) < tolerance) &&
			 (Cmplx_Modulus(differenceVector[2]) < tolerance) ) {
			Journal_Printf( stream, "Answer within tolerance %g of expected result: ", tolerance);
			StGermain_PrintNamedComplexVector( stream, k, 3);
		}
		else {
			Journal_Printf( stream, "Answer not within tolerance %g of expected result: ", tolerance);
			StGermain_PrintNamedComplexVector( stream, k, 3);
		}
		
		Journal_Printf(stream, "Axis Rotation\n");		
		StGermain_RotateCoordinateAxisComplex( j, K_AXIS, angle, vector );
		Journal_Printf( stream, "J Rotated %g radians around K axis - \n", angle); 
		Cmplx_Subtract(vector[0], i[0], differenceVector[0]);
		Cmplx_Subtract(vector[1], i[1], differenceVector[1]);
		Cmplx_Subtract(vector[2], i[2], differenceVector[2]);
		
		if ( (Cmplx_Modulus(differenceVector[0]) < tolerance) && 
			 (Cmplx_Modulus(differenceVector[1]) < tolerance) &&
			 (Cmplx_Modulus(differenceVector[2]) < tolerance) ) {
			Journal_Printf( stream, "Answer within tolerance %g of expected result: ", tolerance);
			StGermain_PrintNamedComplexVector( stream, i, 3);
		}
		else {
			Journal_Printf( stream, "Answer not within tolerance %g of expected result: ", tolerance);
			StGermain_PrintNamedComplexVector( stream, i, 3);
		}

		
		Journal_Printf(stream, "Angle Rotation\n");
		StGermain_RotateComplexVector( j, 0.0, 0.0, angle, vector );
		Journal_Printf( stream, "J Rotated %g radians around K axis - \n", angle); 
		Cmplx_Subtract(vector[0], i[0], differenceVector[0]);
		Cmplx_Subtract(vector[1], i[1], differenceVector[1]);
		Cmplx_Subtract(vector[2], i[2], differenceVector[2]);
		
		if ( (Cmplx_Modulus(differenceVector[0]) < tolerance) && 
			 (Cmplx_Modulus(differenceVector[1]) < tolerance) &&
			 (Cmplx_Modulus(differenceVector[2]) < tolerance) ) {
			Journal_Printf( stream, "Answer within tolerance %g of expected result: ", tolerance);
			StGermain_PrintNamedComplexVector( stream, i, 3);
		}
		else {
			Journal_Printf( stream, "Answer not within tolerance %g of expected result: ", tolerance);
			StGermain_PrintNamedComplexVector( stream, i, 3);
		}

	
		angle = M_PI / 4.0;

		StGermain_RotateComplexVector(i, 0.0, angle, angle, vector );
		Journal_Printf( stream, "I Rotated %g radians around J axis "
		"and %2g radians around K axis: \n", angle, angle);
		StGermain_PrintNamedComplexVector( stream, vector, 3 );

		StGermain_RotateComplexVector(j, angle, 0.0, angle, vector );
		Journal_Printf( stream, "J Rotated %g radians around I axis "
		"and %g radians around K axis: \n", angle, angle); 
		StGermain_PrintNamedComplexVector( stream, vector, 3 );

		StGermain_RotateComplexVector(k, angle, angle, 0.0, vector );
		Journal_Printf( stream, "K Rotated %g radians around I axis "
		"and %g radians around J axis: \n", angle, angle); 
		StGermain_PrintNamedComplexVector( stream, vector, 3 );


		/* Check addition function */
		Journal_Printf( stream, "\n****************************\n");
		Journal_Printf( stream, "vector = A + B\n");
		for ( d = 0 ; d <= 6 ; d++ ) {
			StGermain_ComplexVectorAddition( vector, A, B, d );
			StGermain_PrintNamedComplexVector( stream, vector, d );
		}

		/* Check subtraction function */
		Journal_Printf( stream, "\n****************************\n");
		Journal_Printf( stream, "vector = A - B\n");
		for ( d = 0 ; d <= 6 ; d++ ) {
			StGermain_ComplexVectorSubtraction( vector, A, B, d );
			StGermain_PrintNamedComplexVector( stream, vector, d );
		}
	
		/* Check Magnitude Function */
		Journal_Printf( stream, "\n****************************\n");
		Journal_Printf( stream, "Check Magnitude Function\n");
		for ( d = 0 ; d <= 6 ; d++ ) {
			Journal_Printf( stream, "dim = %d magnitude A = %2.3f\n", d, StGermain_ComplexVectorMagnitude( A, d ) );
			Journal_Printf( stream, "dim = %d magnitude B = %2.3f\n", d, StGermain_ComplexVectorMagnitude( B, d ) );
		}

		/* Check Dot Product */
		Journal_Printf( stream, "\n****************************\n");
		Journal_Printf( stream, "Check Dot Product Function\n");
		Journal_Printf( stream, "value = A . B \n");
		
		
		for (d = 0; d <=6; d++) {
		StGermain_ComplexVectorDotProduct(A, B, d, dotProductResult);			
		Journal_Printf( stream, "dim = %d dot product = %2.3f + %2.3f i\n",
			d, dotProductResult[0], dotProductResult[1] );
		}
		

		/* Check Cross Product */
		/* Tested against http://www.engplanet.com/redirect.html?3859 */
		Journal_Printf( stream, "\n****************************\n");
		Journal_Printf( stream, "Check Cross Product Function\n");
		Journal_Printf( stream, " A x B in 3-D\n");
		StGermain_ComplexVectorCrossProduct( vector, A, B );
		StGermain_PrintNamedComplexVector( stream, vector, 3 );


		/* Checking centroid function */
		Journal_Printf( stream, "\n****************************\n");
		Journal_Printf( stream, "Checking centroid function\n");
		for ( d = 0 ; d <= 6 ; d++ ) {
			StGermain_ComplexTriangleCentroid( vector, A, B, C, d );
			StGermain_PrintNamedComplexVector( stream, vector, d );
		}


		/* Check Normalisation Function */
		Journal_Printf( stream, "\n****************************\n");
		Journal_Printf( stream, "Check Normalisation Function\n");
		
		Journal_Printf( stream, "2-D\n\n");
		d = 2;
		StGermain_PrintNamedComplexVector( stream, A, d );
		StGermain_ComplexVectorNormalise( A, d );
		StGermain_PrintNamedComplexVector( stream, A, d);
		Journal_Printf( stream, "mag = %2.3f\n", StGermain_ComplexVectorMagnitude( A, d ) );

		Journal_Printf( stream, "3-D\n\n");
		d = 3;
		StGermain_PrintNamedComplexVector( stream, B, d );
		StGermain_ComplexVectorNormalise( B, d );
		StGermain_PrintNamedComplexVector( stream, B, d);
		Journal_Printf( stream, "mag = %2.3f\n", StGermain_ComplexVectorMagnitude( B, d ) );

		Journal_Printf( stream, "5-D\n\n");
		d = 5;
		StGermain_PrintNamedComplexVector( stream, C, d );
		StGermain_ComplexVectorNormalise( C, d );
		StGermain_PrintNamedComplexVector( stream, C, d);
		Journal_Printf( stream, "mag = %2.3f\n", StGermain_ComplexVectorMagnitude( C, d ) );

		Journal_Printf( stream, "\n****************************\n");
		Journal_Printf( stream, "Check StGermain_ComplexVectorCrossProductMagnitude\n");
		A[0][REAL_PART] = 1.0; A[0][IMAG_PART] = 1.0; 
		A[1][REAL_PART] = 2.0; A[1][IMAG_PART] = 0.0;
		A[2][REAL_PART] = 3.0; A[2][IMAG_PART] = 0.0;
		B[0][REAL_PART] = 4.0; B[0][IMAG_PART] = 0.0;
		B[1][REAL_PART] = 5.0; B[1][IMAG_PART] = 0.0;
		B[2][REAL_PART] = 6.0; B[2][IMAG_PART] = 3.0;
		StGermain_PrintNamedComplexVector( stream, A, 3);
		StGermain_PrintNamedComplexVector( stream, B, 3);
		
		StGermain_ComplexVectorCrossProductMagnitude(A, B, 2, value ) ;
		Journal_Printf( stream, "mag = %2.3g + %2.3g i (2D)\n", value[REAL_PART], value[IMAG_PART] );
		
		StGermain_ComplexVectorCrossProductMagnitude(A, B, 3, value ) ;
		Journal_Printf( stream, "mag = %2.3g + %2.3g i (3D)\n", value[REAL_PART], value[IMAG_PART] );


		Journal_Printf( stream, "\n****************************\n");
		Journal_Printf( stream, "Check StGermain_ComplexScalarTripleProduct \n");
		
		matrix = Memory_Alloc_2DArray( Cmplx, 3, 3, "matrix" );
		
		matrix[0][0][REAL_PART] = 1.0; 	matrix[0][0][IMAG_PART] = 1.0;
		matrix[0][1][REAL_PART] = 2.0; 	matrix[0][1][IMAG_PART] = 0.0; 
		matrix[0][2][REAL_PART] = 3.0; 	matrix[0][2][IMAG_PART] = 2.0;
		matrix[1][0][REAL_PART] = 4.0; 	matrix[1][0][IMAG_PART] = 0.0;
		matrix[1][1][REAL_PART] = 5.0; 	matrix[1][1][IMAG_PART] = 3.0; 
		matrix[1][2][REAL_PART] = 6.0; 	matrix[1][2][IMAG_PART] = 0.0;
		matrix[2][0][REAL_PART] = 7.0; 	matrix[2][0][IMAG_PART] = 1.0;
		matrix[2][1][REAL_PART] = 8.0; 	matrix[2][1][IMAG_PART] = 0.0;
		matrix[2][2][REAL_PART] = 11.0; matrix[2][2][IMAG_PART] = 1.0;
		StGermain_PrintNamedComplexVector( stream, matrix[0], 3);
		StGermain_PrintNamedComplexVector( stream, matrix[1], 3);
		StGermain_PrintNamedComplexVector( stream, matrix[2], 3);

		StGermain_ComplexScalarTripleProduct( matrix[0], matrix[1], matrix[2], value );
		Journal_Printf( stream, "scalar triple product: ");
		Journal_PrintCmplx( stream, value );
		
		StGermain_ComplexScalarTripleProduct( matrix[2], matrix[0], matrix[1], value );
		Journal_Printf( stream, "scalar triple product: ");
		Journal_PrintCmplx( stream, value );
		StGermain_ComplexScalarTripleProduct( matrix[1], matrix[2], matrix[0], value );
		Journal_Printf( stream, "scalar triple product: ");
		Journal_PrintCmplx( stream, value );
		
		Journal_Printf( stream, "\n****************************\n");
		Journal_Printf( stream, "Check Vector_ToComplexVector function \n");
		
		realVector[0] = 1.0; realVector[1] = 2.0; realVector[2] = 3.0;
		
		StGermain_PrintNamedVector(stream, realVector, 3);
		Vector_ToComplexVector(realVector, 3, matrix[0]) ;
		StGermain_PrintNamedComplexVector( stream, matrix[0], 3);

		Journal_Printf( stream, "\n****************************\n");
		Journal_Printf( stream, "Check ComplexVector_ToVector function \n");	
		
		matrix[0][0][REAL_PART] = 5.0; 	matrix[0][0][IMAG_PART] = 0.0;
		matrix[0][1][REAL_PART] = 6.0; 	matrix[0][1][IMAG_PART] = 0.0; 
		matrix[0][2][REAL_PART] = 7.0; 	matrix[0][2][IMAG_PART] = 0.0;
		
		StGermain_PrintNamedComplexVector( stream, matrix[0], 3);
		ComplexVector_ToVector(matrix[0], 3, realVector) ;
		StGermain_PrintNamedVector(stream, realVector, 3);

		
		Memory_Free( matrix );


	}
	
	Journal_Printf( stream, "\n");
	DiscretisationGeometry_Finalise();
	
	Base_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0;
}

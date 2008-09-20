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
** $Id: testTensorMath.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"

#include <stdio.h>

#define TENSORMULTMATH_TEST_ERROR 1e-05

int main( int argc, char* argv[] ) {
	MPI_Comm CommWorld;
	int rank;
	int numProcessors;
	int procToWatch;
	Stream*  stream;
	
	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size( CommWorld, &numProcessors );
	MPI_Comm_rank( CommWorld, &rank );
	
	Base_Init( &argc, &argv );
	
	DiscretisationGeometry_Init( &argc, &argv );
	MPI_Barrier( CommWorld ); /* Ensures copyright info always come first in output */
	
	
	stream = Journal_Register( InfoStream_Type, "TensorMultMath" );
	/* stout -> file redirect code */
	//stJournal->firewallProducesAssert = False;
	//Stream_RedirectFile(Journal_Register( Error_Type, "TensorMultMath"), "TensorMultMath.txt");
	//Stream_RedirectFile(stream, "TensorMultMath.txt");
	
	
	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}

	if( rank == procToWatch ) {
		/* Put in tests here */
		SymmetricTensor symmTensor, symmTensor2, symmTensorResult;
		TensorArray     tensorArray, tensorArray2, tensorResult, tensorCompare, tensorDiff;
		XYZ 	vector, vectorResult;
		double result, errorValue;
		Dimension_Index row, col;
		Bool tensorMultMathTest_Flag;
		double  **nonSquareMatrixA, **nonSquareMatrixB;
		double	**nonSquareMatrixResult, **nonSquareMatrixAT;
		double solutionVector[4], solutionVectorCompare[4];
		Bool error_flag;
		#define STG_TENSORMULTMATHTEST_ERROR 1.0e-14;
		
		errorValue = STG_TENSORMULTMATHTEST_ERROR;
		Journal_Printf( stream, "%g", errorValue);
		Journal_Printf(stream, "\n/*******************    Test 1   ************************/\n");
		Journal_Printf( stream, "Test TensorArray Identity\n\n");
		
		Journal_Printf( stream, "2-D\n");
		TensorArray_Identity(2, tensorArray );
		Journal_PrintTensorArray( stream, tensorArray, 2);
		
		Journal_Printf( stream, "3-D\n");
		TensorArray_Identity(3, tensorArray );
		Journal_PrintTensorArray( stream, tensorArray, 3);
		
		Journal_Printf(stream, "\n/*******************    Test 2   ************************/\n");
		Journal_Printf( stream, "Test SymmetricTensor Identity\n\n");
				
		Journal_Printf( stream, "2-D\n");
		SymmetricTensor_Identity(2, symmTensor );
		Journal_PrintSymmetricTensor( stream, symmTensor, 2);
		
		Journal_Printf( stream, "3-D\n");
		SymmetricTensor_Identity(3, symmTensor );
		Journal_PrintSymmetricTensor( stream, symmTensor, 3);
		
		Journal_Printf(stream, "\n/*******************    Test 3   ************************/\n");
		Journal_Printf( stream, "Test function TensorArray_Transpose \n\n");
		Journal_Printf( stream, "2-D\n");		
		tensorArray[FT2D_00] = 0.7;
		tensorArray[FT2D_01] = 1;
		tensorArray[FT2D_10] = 2;
		tensorArray[FT2D_11] = 3;
		Journal_PrintTensorArray( stream, tensorArray, 2);
		TensorArray_Transpose(tensorArray, 2, tensorResult);
		Journal_PrintTensorArray( stream, tensorResult, 2);

		Journal_Printf( stream, "3-D\n");		
		tensorArray[FT3D_00] = 0.5;
		tensorArray[FT3D_01] = 10;
		tensorArray[FT3D_02] = 20;
		tensorArray[FT3D_10] = 30;
		tensorArray[FT3D_11] = 40;
		tensorArray[FT3D_12] = 50;
		tensorArray[FT3D_20] = 60;
		tensorArray[FT3D_21] = 70;
		tensorArray[FT3D_22] = 80;
		
		Journal_PrintTensorArray( stream, tensorArray, 3);
		TensorArray_Transpose(tensorArray, 3, tensorResult);
		Journal_PrintTensorArray( stream, tensorResult, 3);
		
		Journal_Printf(stream, "\n/*******************    Test 4   ************************/\n");
		Journal_Printf( stream, "Test function TensorArray_Add \n\n");
		
		Journal_Printf( stream, "2-D\n");
		tensorArray2[FT2D_00] = 5;
		tensorArray2[FT2D_01] = 6;
		tensorArray2[FT2D_10] = 7;
		tensorArray2[FT2D_11] = 8;
				
		Journal_PrintTensorArray( stream, tensorArray, 2);
		Journal_PrintTensorArray( stream, tensorArray2, 2);
		TensorArray_Add(tensorArray, tensorArray2, 2, tensorResult);
		Journal_PrintTensorArray( stream, tensorResult, 2);
		
		Journal_Printf( stream, "3-D\n");
		tensorArray2[FT3D_00] = 5;
		tensorArray2[FT3D_01] = 1;
		tensorArray2[FT3D_02] = 2;
		tensorArray2[FT3D_10] = 3;
		tensorArray2[FT3D_11] = 4;
		tensorArray2[FT3D_12] = 5;
		tensorArray2[FT3D_20] = 6;
		tensorArray2[FT3D_21] = 7;
		tensorArray2[FT3D_22] = 8;
		
		Journal_PrintTensorArray( stream, tensorArray, 3);
		Journal_PrintTensorArray( stream, tensorArray2, 3);
		TensorArray_Add(tensorArray, tensorArray2, 3, tensorResult);	
		Journal_PrintTensorArray( stream, tensorResult, 3);
		
		Journal_Printf(stream, "\n/*******************    Test 5   ************************/\n");
		Journal_Printf( stream, "Test function TensorArray_Subtract \n\n");
		
		Journal_Printf( stream, "2-D\n");
		tensorArray2[FT2D_00] = 5;
		tensorArray2[FT2D_01] = 6;
		tensorArray2[FT2D_10] = 7;
		tensorArray2[FT2D_11] = 8;
				
		Journal_PrintTensorArray( stream, tensorArray, 2);
		Journal_PrintTensorArray( stream, tensorArray2, 2);
		TensorArray_Subtract(tensorArray, tensorArray2, 2, tensorResult);
		Journal_PrintTensorArray( stream, tensorResult, 2);
		
		Journal_Printf( stream, "3-D\n");
		tensorArray2[FT3D_00] = 50;
		tensorArray2[FT3D_01] = 1;
		tensorArray2[FT3D_02] = 2;
		tensorArray2[FT3D_10] = 3;
		tensorArray2[FT3D_11] = 4;
		tensorArray2[FT3D_12] = 5;
		tensorArray2[FT3D_20] = 6;
		tensorArray2[FT3D_21] = 7;
		tensorArray2[FT3D_22] = 8;
		
		Journal_PrintTensorArray( stream, tensorArray, 3);
		Journal_PrintTensorArray( stream, tensorArray2, 3);
		TensorArray_Subtract(tensorArray, tensorArray2, 3, tensorResult);	
		Journal_PrintTensorArray( stream, tensorResult, 3);		
		
		Journal_Printf(stream, "\n/*******************    Test 6   ************************/\n");
		Journal_Printf( stream, "Test function TensorArray_MultiplyByTensorArray \n\n");
		Journal_Printf( stream, "Solutions tested against: http://www.uni-bonn.de/~manfear/matrixcalc.php");
		Journal_Printf( stream, "2-D\n");


		tensorArray[FT2D_00] = 1;
		tensorArray[FT2D_01] = 2;
		tensorArray[FT2D_10] = 3;
		tensorArray[FT2D_11] = 4;								
		
		tensorArray2[FT2D_00] = 5;
		tensorArray2[FT2D_01] = 6;
		tensorArray2[FT2D_10] = 7;
		tensorArray2[FT2D_11] = 8;
		
		Journal_PrintTensorArray( stream, tensorArray, 2);
		Journal_PrintTensorArray( stream, tensorArray2, 2);
		TensorArray_MultiplyByTensorArray(tensorArray, tensorArray2, 2, tensorResult);
		Journal_PrintTensorArray( stream, tensorResult, 2);
		
		Journal_Printf( stream, "3-D\n");
		tensorArray[FT3D_00] = 1;
		tensorArray[FT3D_01] = 2;
		tensorArray[FT3D_02] = 3;
		tensorArray[FT3D_10] = 4;
		tensorArray[FT3D_11] = 5;
		tensorArray[FT3D_12] = 6;
		tensorArray[FT3D_20] = 7;
		tensorArray[FT3D_21] = 8;
		tensorArray[FT3D_22] = 9;
		
		tensorArray2[FT3D_00] = 10;
		tensorArray2[FT3D_01] = 11;
		tensorArray2[FT3D_02] = 12;
		tensorArray2[FT3D_10] = 13;
		tensorArray2[FT3D_11] = 14;
		tensorArray2[FT3D_12] = 15;
		tensorArray2[FT3D_20] = 16;
		tensorArray2[FT3D_21] = 17;
		tensorArray2[FT3D_22] = 18;		
		
		Journal_PrintTensorArray( stream, tensorArray, 3);
		Journal_PrintTensorArray( stream, tensorArray2, 3);
		TensorArray_MultiplyByTensorArray(tensorArray, tensorArray2, 3, tensorResult);
		Journal_PrintTensorArray( stream, tensorResult, 3);	
		
		Journal_Printf(stream, "\n/*******************    Test 7   ************************/\n");
		Journal_Printf( stream, "Test function TensorArray_MultiplyByRightTranspose \n\n");
		Journal_Printf( stream, "Solutions tested against: http://www.uni-bonn.de/~manfear/matrixcalc.php");
		
		Journal_Printf( stream, "2-D\n");
		tensorArray[FT2D_00] = 1;
		tensorArray[FT2D_01] = 2;
		tensorArray[FT2D_10] = 3;
		tensorArray[FT2D_11] = 4;
		
		Journal_PrintTensorArray( stream, tensorArray, 2);
		TensorArray_MultiplyByRightTranspose(tensorArray, 2, symmTensorResult);
		Journal_Printf( stream, "The answer, A * A^T = \n");
		Journal_PrintSymmetricTensor( stream, symmTensorResult, 2);

		Journal_Printf( stream, "3-D\n");
		tensorArray[FT3D_00] = 1;
		tensorArray[FT3D_01] = 2;
		tensorArray[FT3D_02] = 3;
		tensorArray[FT3D_10] = 4;
		tensorArray[FT3D_11] = 5;
		tensorArray[FT3D_12] = 6;
		tensorArray[FT3D_20] = 7;
		tensorArray[FT3D_21] = 8;
		tensorArray[FT3D_22] = 9;
		
		Journal_PrintTensorArray( stream, tensorArray, 3);
		TensorArray_MultiplyByRightTranspose(tensorArray, 3, symmTensorResult);
		Journal_Printf( stream, "The answer, A * A^T = \n");
		Journal_PrintSymmetricTensor( stream, symmTensorResult, 3);
		
		Journal_Printf(stream, "\n/*******************    Test 8   ************************/\n");
		Journal_Printf( stream, "Test function TensorArray_MultiplyByLeftTranspose \n\n");
		Journal_Printf( stream, "Solutions tested against: http://www.uni-bonn.de/~manfear/matrixcalc.php");
		
		Journal_Printf( stream, "2-D\n");
		tensorArray[FT2D_00] = 1;
		tensorArray[FT2D_01] = 2;
		tensorArray[FT2D_10] = 3;
		tensorArray[FT2D_11] = 4;
		
		Journal_PrintTensorArray( stream, tensorArray, 2);
		TensorArray_MultiplyByLeftTranspose(tensorArray, 2, symmTensorResult);
		Journal_Printf( stream, "The answer, A^T * A = \n");
		Journal_PrintSymmetricTensor( stream, symmTensorResult, 2);

		Journal_Printf( stream, "3-D\n");
		tensorArray[FT3D_00] = 1;
		tensorArray[FT3D_01] = 2;
		tensorArray[FT3D_02] = 3;
		tensorArray[FT3D_10] = 4;
		tensorArray[FT3D_11] = 5;
		tensorArray[FT3D_12] = 6;
		tensorArray[FT3D_20] = 7;
		tensorArray[FT3D_21] = 8;
		tensorArray[FT3D_22] = 9;
		
		Journal_PrintTensorArray( stream, tensorArray, 3);
		TensorArray_MultiplyByLeftTranspose(tensorArray, 3, symmTensorResult);
		Journal_Printf( stream, "The answer, A^T * A = \n");
		Journal_PrintSymmetricTensor( stream, symmTensorResult, 3);		
		
		
		Journal_Printf(stream, "\n/*******************    Test 9   ************************/\n");
		Journal_Printf( stream, "Test function TensorArray_MultiplyBySymmetricTensor \n\n");
		Journal_Printf( stream, "Solutions tested against: http://www.uni-bonn.de/~manfear/matrixcalc.php");
		
		Journal_Printf( stream, "2-D\n");
		tensorArray[FT2D_00] = 1;
		tensorArray[FT2D_01] = 2;
		tensorArray[FT2D_10] = 3;
		tensorArray[FT2D_11] = 4;
		
		symmTensor[ST2D_00] = 5;		
		symmTensor[ST2D_01] = 6;		
		symmTensor[ST2D_11] = 7;		
		
		Journal_PrintTensorArray( stream, tensorArray, 2);
		Journal_PrintSymmetricTensor( stream, symmTensor, 2);
		TensorArray_MultiplyBySymmetricTensor(tensorArray, symmTensor, 2, tensorResult);
		Journal_PrintTensorArray( stream, tensorResult, 2);

		Journal_Printf( stream, "3-D\n");
		tensorArray[FT3D_00] = 1;
		tensorArray[FT3D_01] = 2;
		tensorArray[FT3D_02] = 3;
		tensorArray[FT3D_10] = 4;
		tensorArray[FT3D_11] = 5;
		tensorArray[FT3D_12] = 6;
		tensorArray[FT3D_20] = 7;
		tensorArray[FT3D_21] = 8;
		tensorArray[FT3D_22] = 9;
		
		symmTensor[ST3D_00] = 10;
		symmTensor[ST3D_01] = 11;
		symmTensor[ST3D_02] = 12;
		symmTensor[ST3D_11] = 13;
		symmTensor[ST3D_12] = 14;
		symmTensor[ST3D_22] = 15;
		
		Journal_PrintTensorArray( stream, tensorArray, 3);
		Journal_PrintSymmetricTensor( stream, symmTensor, 3);
		TensorArray_MultiplyBySymmetricTensor(tensorArray, symmTensor, 3, tensorResult);
		Journal_PrintTensorArray( stream, tensorResult, 3);

		Journal_Printf(stream, "\n/*******************    Test 10   ************************/\n");
		Journal_Printf( stream, "Test function TensorArray_MultiplyByLeftVector \n\n");
		Journal_Printf( stream, "Solutions tested against: http://www.uni-bonn.de/~manfear/matrixcalc.php");		
		
		Journal_Printf( stream, "2-D\n");
		tensorArray[FT2D_00] = 1;
		tensorArray[FT2D_01] = 2;
		tensorArray[FT2D_10] = 3;
		tensorArray[FT2D_11] = 4;
		
		vector[0] = 5;
		vector[1] = 6;
		
		Journal_PrintTensorArray( stream, tensorArray, 2);
		StGermain_PrintNamedVector( stream, vector, 2);
		TensorArray_MultiplyByLeftVector(tensorArray, vector, 2, vectorResult);
		StGermain_PrintNamedVector( stream, vectorResult, 2);
		
		Journal_Printf( stream, "3-D\n");
		tensorArray[FT3D_00] = 1;
		tensorArray[FT3D_01] = 2;
		tensorArray[FT3D_02] = 3;
		tensorArray[FT3D_10] = 4;
		tensorArray[FT3D_11] = 5;
		tensorArray[FT3D_12] = 6;
		tensorArray[FT3D_20] = 7;
		tensorArray[FT3D_21] = 8;
		tensorArray[FT3D_22] = 9;

		vector[0] = 10;
		vector[1] = 11;
		vector[2] = 12;

		Journal_PrintTensorArray( stream, tensorArray, 3);
		StGermain_PrintNamedVector( stream, vector, 3);
		TensorArray_MultiplyByLeftVector(tensorArray, vector, 3, vectorResult);
		StGermain_PrintNamedVector( stream, vectorResult, 3);
		
		Journal_Printf(stream, "\n/*******************    Test 11   ************************/\n");
		Journal_Printf( stream, "Test function TensorArray_MultiplyByRightVector \n\n");
		Journal_Printf( stream, "Solutions tested against: http://www.uni-bonn.de/~manfear/matrixcalc.php");		

		Journal_Printf( stream, "2-D\n");
		tensorArray[FT2D_00] = 1;
		tensorArray[FT2D_01] = 2;
		tensorArray[FT2D_10] = 3;
		tensorArray[FT2D_11] = 4;
		
		vector[0] = 5;
		vector[1] = 6;
		
		Journal_PrintTensorArray( stream, tensorArray, 2);
		StGermain_PrintNamedVector( stream, vector, 2);
		TensorArray_MultiplyByRightVector(tensorArray, vector, 2, vectorResult);
		StGermain_PrintNamedVector( stream, vectorResult, 2);
		
		Journal_Printf( stream, "3-D\n");
		tensorArray[FT3D_00] = 1;
		tensorArray[FT3D_01] = 2;
		tensorArray[FT3D_02] = 3;
		tensorArray[FT3D_10] = 4;
		tensorArray[FT3D_11] = 5;
		tensorArray[FT3D_12] = 6;
		tensorArray[FT3D_20] = 7;
		tensorArray[FT3D_21] = 8;
		tensorArray[FT3D_22] = 9;

		vector[0] = 10;
		vector[1] = 11;
		vector[2] = 12;

		Journal_PrintTensorArray( stream, tensorArray, 3);
		StGermain_PrintNamedVector( stream, vector, 3);
		TensorArray_MultiplyByRightVector(tensorArray, vector, 3, vectorResult);
		StGermain_PrintNamedVector( stream, vectorResult, 3);

		Journal_Printf(stream, "\n/*******************    Test 12   ************************/\n");
		Journal_Printf( stream, "Test function TensorArray_CalcDeterminant \n\n");
		Journal_Printf( stream, "Solutions tested against: http://www.bluebit.gr/matrix-calculator/");		

		Journal_Printf( stream, "2-D\n");
		tensorArray[FT2D_00] = 1;
		tensorArray[FT2D_01] = 2;
		tensorArray[FT2D_10] = 3;
		tensorArray[FT2D_11] = 4;
		
		Journal_PrintTensorArray( stream, tensorArray, 2);		
		result = TensorArray_CalcDeterminant(tensorArray, 2);
		Journal_Printf( stream, "Determinant = \n");
		Journal_PrintValue( stream, result);

		Journal_Printf( stream, "3-D\n");
		tensorArray[FT3D_00] = 1;
		tensorArray[FT3D_01] = 2;
		tensorArray[FT3D_02] = 3;
		tensorArray[FT3D_10] = 30;
		tensorArray[FT3D_11] = 22;
		tensorArray[FT3D_12] = 4;
		tensorArray[FT3D_20] = 5;
		tensorArray[FT3D_21] = 7;
		tensorArray[FT3D_22] = 9;
		
		Journal_PrintTensorArray( stream, tensorArray, 3);		
		result = TensorArray_CalcDeterminant(tensorArray, 3);
		Journal_Printf( stream, "Determinant = \n");
		Journal_PrintValue( stream, result);

		Journal_Printf(stream, "\n/*******************    Test 13   ************************/\n");
		Journal_Printf( stream, "Test function TensorArray_CalcInverseWithDeterminant \n\n");
				
		Journal_Printf( stream, "2-D\n");
		
		tensorMultMathTest_Flag = True;

		tensorArray[FT2D_00] = 1;
		tensorArray[FT2D_01] = 2;
		tensorArray[FT2D_10] = 3;
		tensorArray[FT2D_11] = 4;
		
		tensorCompare[FT2D_00] = -2;
		tensorCompare[FT2D_01] =  1;
		tensorCompare[FT2D_10] =  1.5;
		tensorCompare[FT2D_11] = -0.5;
		
		Journal_PrintTensorArray( stream, tensorArray, 2);
		result = TensorArray_CalcDeterminant(tensorArray, 2);		
		TensorArray_CalcInverseWithDeterminant(tensorArray, result, 2, tensorResult);
		
		TensorArray_Subtract(tensorResult, tensorCompare, 2, tensorDiff);
		
		for (row = 0; row < 2; row++ ){
			for ( col = 0; col < 2; col++ ){
				if (fabs(tensorDiff[TensorArray_TensorMap(row, col, 2)]) > errorValue ) {
					Journal_Printf( stream, "Value, %g at index ( %d, %d) not within tolerance, %g\n", 
							tensorResult[TensorArray_TensorMap(row, col, 2)],row, col, errorValue);
					tensorMultMathTest_Flag = False;
				}
			}
		}
		if (tensorMultMathTest_Flag == True) {
			Journal_Printf( stream, "Answer calculated within tolerance %g, for solution:\n", errorValue);
		}
		else {
			Journal_Printf( stream, "Answer calculated not within tolerance %g, for solution:\n", errorValue);			
		}
		
		Journal_Printf( stream, "Inverse of tensor:\n");
		Journal_PrintTensorArray( stream, tensorCompare, 2);

		Journal_Printf( stream, "3-D\n");
		
		tensorMultMathTest_Flag = True;
		
		tensorArray[FT3D_00] = 1;
		tensorArray[FT3D_01] = 2;
		tensorArray[FT3D_02] = 3;
		tensorArray[FT3D_10] = 30;
		tensorArray[FT3D_11] = 22;
		tensorArray[FT3D_12] = 4;
		tensorArray[FT3D_20] = 5;
		tensorArray[FT3D_21] = 7;
		tensorArray[FT3D_22] = 9;

		tensorCompare[FT3D_00] = -5.666666666666667;
		tensorCompare[FT3D_01] = -0.100000000000000;
		tensorCompare[FT3D_02] =  1.933333333333333;
		tensorCompare[FT3D_10] =  8.333333333333333;
		tensorCompare[FT3D_11] =  0.200000000000000;
		tensorCompare[FT3D_12] = -2.866666666666667;
		tensorCompare[FT3D_20] = -3.333333333333333;
		tensorCompare[FT3D_21] = -0.100000000000000;
		tensorCompare[FT3D_22] =  1.266666666666667;
		
		Journal_PrintTensorArray( stream, tensorArray, 3);
		result = TensorArray_CalcDeterminant(tensorArray, 3);		
		TensorArray_CalcInverseWithDeterminant(tensorArray,result, 3, tensorResult);
		
		TensorArray_Subtract(tensorResult, tensorCompare, 3, tensorDiff);
		
		for (row = 0; row < 3; row++ ){
			for ( col = 0; col < 3; col++ ){
				if (fabs(tensorDiff[TensorArray_TensorMap(row, col, 3)]) > errorValue ) {
					Journal_Printf( stream, "Value, %g at index ( %d, %d) not within tolerance, %g\n", 
							tensorResult[TensorArray_TensorMap(row, col, 3)],row, col, errorValue);
					tensorMultMathTest_Flag = False;
				}
			}
		}
		if (tensorMultMathTest_Flag == True) {
			Journal_Printf( stream, "Answer calculated within tolerance %g, for solution:\n", errorValue);
		}
		else {
			Journal_Printf( stream, "Answer calculated not within tolerance %g, for solution:\n", errorValue);			
		}
		
		
		Journal_Printf( stream, "Inverse of tensor:\n");
		Journal_PrintTensorArray( stream, tensorCompare, 3);

		Journal_Printf(stream, "\n/*******************    Test 14   ************************/\n");
		Journal_Printf( stream, "Test function TensorArray_CalcInverse \n\n");
		Journal_Printf( stream, "Solutions tested against: http://www.bluebit.gr/matrix-calculator/");		

		Journal_Printf( stream, "2-D\n");

		tensorMultMathTest_Flag = True;		
		
		tensorArray[FT2D_00] = 1;
		tensorArray[FT2D_01] = 2;
		tensorArray[FT2D_10] = 3;
		tensorArray[FT2D_11] = 4;

		tensorCompare[FT2D_00] = -2;
		tensorCompare[FT2D_01] =  1;
		tensorCompare[FT2D_10] =  1.5;
		tensorCompare[FT2D_11] = -0.5;
		
		Journal_PrintTensorArray( stream, tensorArray, 2);	
		TensorArray_CalcInverse(tensorArray, 2, tensorResult);
		
		TensorArray_Subtract(tensorResult, tensorCompare, 2, tensorDiff);
		
		for (row = 0; row < 2; row++ ){
			for ( col = 0; col < 2; col++ ){
				if (fabs(tensorDiff[TensorArray_TensorMap(row, col, 2)]) > errorValue ) {
					Journal_Printf( stream, "Value, %g at index ( %d, %d) not within tolerance, %g\n", 
							tensorResult[TensorArray_TensorMap(row, col, 2)],row, col, errorValue);
					tensorMultMathTest_Flag = False;
				}
			}
		}
		if (tensorMultMathTest_Flag == True) {
			Journal_Printf( stream, "Answer calculated within tolerance %g, for solution:\n", errorValue);
		}
		else {
			Journal_Printf( stream, "Answer calculated not within tolerance %g, for solution:\n", errorValue);			
		}
		
		
		Journal_Printf( stream, "Inverse of tensor:\n");
		Journal_PrintTensorArray( stream, tensorCompare, 2);
		
		Journal_Printf( stream, "3-D\n");

		tensorMultMathTest_Flag = True;

		tensorArray[FT3D_00] = 1;
		tensorArray[FT3D_01] = 2;
		tensorArray[FT3D_02] = 3;
		tensorArray[FT3D_10] = 30;
		tensorArray[FT3D_11] = 22;
		tensorArray[FT3D_12] = 4;
		tensorArray[FT3D_20] = 5;
		tensorArray[FT3D_21] = 7;
		tensorArray[FT3D_22] = 9;

		tensorCompare[FT3D_00] = -5.666666666666667;
		tensorCompare[FT3D_01] = -0.1;
		tensorCompare[FT3D_02] =  1.933333333333333;
		tensorCompare[FT3D_10] =  8.333333333333333;
		tensorCompare[FT3D_11] =  0.200000000000;
		tensorCompare[FT3D_12] = -2.866666666666667;
		tensorCompare[FT3D_20] = -3.333333333333333;
		tensorCompare[FT3D_21] = -0.1;
		tensorCompare[FT3D_22] =  1.266666666666667;
		
		
		Journal_PrintTensorArray( stream, tensorArray, 3);	
		TensorArray_CalcInverse(tensorArray, 3, tensorResult);
		
		TensorArray_Subtract(tensorResult, tensorCompare, 3, tensorDiff);
		
		for (row = 0; row < 3; row++ ){
			for ( col = 0; col < 3; col++ ){
				if (fabs(tensorDiff[TensorArray_TensorMap(row, col, 3)]) > errorValue ) {
					Journal_Printf( stream, "Value, %g at index ( %d, %d) not within tolerance, %g\n", 
							tensorResult[TensorArray_TensorMap(row, col, 3)],row, col, errorValue);
					tensorMultMathTest_Flag = False;
				}
			}
		}
		
		if (tensorMultMathTest_Flag == True) {
			Journal_Printf( stream, "Answer calculated within tolerance %g, for solution:\n", errorValue);
		}
		else {
			Journal_Printf( stream, "Answer calculated not within tolerance %g, for solution:\n", errorValue);			
		}
		
		
		Journal_Printf( stream, "Inverse of tensor:\n");
		Journal_PrintTensorArray( stream, tensorCompare, 3);




		Journal_Printf(stream, "\n/*******************    Test 15   ************************/\n");
		Journal_Printf( stream, "Test function TensorArray_DoubleContraction \n\n");
		Journal_Printf( stream, "Hand verified\n");

		Journal_Printf( stream, "2-D\n");
		tensorArray[FT2D_00] = 1;
		tensorArray[FT2D_01] = 2;
		tensorArray[FT2D_10] = 3;
		tensorArray[FT2D_11] = 4;

		tensorArray2[FT2D_00] = 5;
		tensorArray2[FT2D_01] = 6;
		tensorArray2[FT2D_10] = 7;
		tensorArray2[FT2D_11] = 8;
		
		Journal_PrintTensorArray( stream, tensorArray, 2);	
		Journal_PrintTensorArray( stream, tensorArray2, 2);	
		result = TensorArray_DoubleContraction(tensorArray, tensorArray2, 2);
		Journal_Printf( stream, "Double Contraction = \n");
		Journal_PrintValue( stream, result);
		
		Journal_Printf( stream, "3-D\n");
		tensorArray[FT3D_00] = 1;
		tensorArray[FT3D_01] = 2;
		tensorArray[FT3D_02] = 3;
		tensorArray[FT3D_10] = 4;
		tensorArray[FT3D_11] = 5;
		tensorArray[FT3D_12] = 6;
		tensorArray[FT3D_20] = 7;
		tensorArray[FT3D_21] = 8;
		tensorArray[FT3D_22] = 9;
		
		tensorArray2[FT3D_00] = 11;
		tensorArray2[FT3D_01] = 12;
		tensorArray2[FT3D_02] = 13;
		tensorArray2[FT3D_10] = 14;
		tensorArray2[FT3D_11] = 15;
		tensorArray2[FT3D_12] = 16;
		tensorArray2[FT3D_20] = 17;
		tensorArray2[FT3D_21] = 18;
		tensorArray2[FT3D_22] = 19;
		
		Journal_PrintTensorArray( stream, tensorArray, 3);	
		Journal_PrintTensorArray( stream, tensorArray2, 3);	
		result = TensorArray_DoubleContraction(tensorArray, tensorArray2, 3);
		Journal_Printf( stream, "Double Contraction = \n");
		Journal_PrintValue( stream, result);
		
		Journal_Printf(stream, "\n/*******************    Test 16   ************************/\n");
		Journal_Printf( stream, "Test function SymmetricTensor_DoubleContraction \n\n");
		Journal_Printf( stream, "Hand verified\n");
		Journal_Printf( stream, "2-D\n");		
		symmTensor[ST2D_00] = 1;
		symmTensor[ST2D_01] = 2;
		symmTensor[ST2D_11] = 4;

		symmTensor2[ST2D_00] = 10;
		symmTensor2[ST2D_01] = 20;
		symmTensor2[ST2D_11] = 40;

		Journal_PrintSymmetricTensor( stream, symmTensor, 2);
		Journal_PrintSymmetricTensor( stream, symmTensor2, 2);
		result = SymmetricTensor_DoubleContraction(symmTensor,symmTensor2, 2);
		Journal_Printf( stream, "Double Contraction = \n");
		Journal_PrintValue( stream, result);

		Journal_Printf( stream, "3-D\n");
		symmTensor[ST3D_00] = 1;
		symmTensor[ST3D_01] = 2;
		symmTensor[ST3D_02] = 3;
		symmTensor[ST3D_11] = 4;
		symmTensor[ST3D_12] = 5;
		symmTensor[ST3D_22] = 6;

		symmTensor2[ST3D_00] = 10;
		symmTensor2[ST3D_01] = 20;
		symmTensor2[ST3D_02] = 30;
		symmTensor2[ST3D_11] = 40;
		symmTensor2[ST3D_12] = 50;
		symmTensor2[ST3D_22] = 60;
		
		Journal_PrintSymmetricTensor( stream, symmTensor, 3);
		Journal_PrintSymmetricTensor( stream, symmTensor2, 3);
		result = SymmetricTensor_DoubleContraction(symmTensor,symmTensor2, 3);
		Journal_Printf( stream, "Double Contraction = \n");
		Journal_PrintValue( stream, result);

		Journal_Printf(stream, "\n/*******************    Test 17   ************************/\n");
		Journal_Printf( stream, "Test function Journal_PrintNonSquareMatrix \n\n");


		nonSquareMatrixA = Memory_Alloc_2DArray(double, 3, 4, "NonSquareMatrixA" );
		nonSquareMatrixB = Memory_Alloc_2DArray(double, 4, 2, "NonSquareMatrixB" );
		nonSquareMatrixAT = Memory_Alloc_2DArray(double, 4, 3, "NonSquareMatrixAT" );
		nonSquareMatrixResult = Memory_Alloc_2DArray(double, 3, 2, "NonSquareMatrixResult" );
		//Matrix A
		nonSquareMatrixA[0][0] = 0; 	nonSquareMatrixA[1][0] =  4;
		nonSquareMatrixA[0][1] = 1;		nonSquareMatrixA[1][1] =  5;
		nonSquareMatrixA[0][2] = 2;		nonSquareMatrixA[1][2] =  6;		
		nonSquareMatrixA[0][3] = 3;		nonSquareMatrixA[1][3] =  7;
		
		nonSquareMatrixA[2][0] = 8;		
		nonSquareMatrixA[2][1] = 9;
		nonSquareMatrixA[2][2] = 10;
		nonSquareMatrixA[2][3] = 11;
		//Matrix B
		nonSquareMatrixB[0][0] = 0; 	nonSquareMatrixB[1][0] =  2;
		nonSquareMatrixB[0][1] = 1;		nonSquareMatrixB[1][1] =  3;
		
		nonSquareMatrixB[2][0] = 4;		nonSquareMatrixB[3][0] =  6;
		nonSquareMatrixB[2][1] = 5;		nonSquareMatrixB[3][1] =  7;
		
		
		Journal_PrintNonSquareMatrix(stream, nonSquareMatrixA, 3, 4);
		Journal_PrintNonSquareMatrix(stream, nonSquareMatrixB, 4, 2);

		Journal_Printf(stream, "\n/*******************    Test 18   ************************/\n");
		Journal_Printf( stream, "Test function NonSquareMatrix_Transpose \n\n");

		Journal_PrintNonSquareMatrix(stream, nonSquareMatrixA, 3, 4);
		
		NonSquareMatrix_Transpose( nonSquareMatrixA, 3, 4, nonSquareMatrixAT);
		
		Journal_PrintNonSquareMatrix(stream, nonSquareMatrixAT, 4, 3);

		Journal_Printf(stream, "\n/*******************    Test 19   ************************/\n");
		Journal_Printf( stream, "Test function NonSquareMatrix_MultiplicationByNonSquareMatrix \n\n");
		
		Journal_PrintNonSquareMatrix(stream, nonSquareMatrixA, 3, 4);
		Journal_PrintNonSquareMatrix(stream, nonSquareMatrixB, 4, 2);

		NonSquareMatrix_MultiplicationByNonSquareMatrix(nonSquareMatrixA, 3, 4, 
			nonSquareMatrixB, 4,2, nonSquareMatrixResult );

		Journal_PrintNonSquareMatrix(stream, nonSquareMatrixResult, 3, 2);

		Journal_Printf(stream, "\n/*******************    Test 20   ************************/\n");
		Journal_Printf( stream, "Test function NonSquareMatrix_MatrixVectorMultiplication \n\n");
		Journal_Printf( stream, "Tested against solutions at http://www.uni-bonn.de/~manfear/solve_lineq.php\n\n");
		
		vector[0] = 1; vector[1] = 2; vector[2] = 3;
		solutionVectorCompare[0] = 32; solutionVectorCompare[1] = 38;
		solutionVectorCompare[2] = 44; solutionVectorCompare[3] = 50;

		Journal_PrintNonSquareMatrix(stream, nonSquareMatrixAT, 4, 3);
		StGermain_PrintNamedVector(stream, vector, 3);
		
		NonSquareMatrix_MatrixVectorMultiplication( nonSquareMatrixAT, 4, 3, vector, 3, solutionVector );
		error_flag = False;
		for ( row = 0; row < 4; row++ ) {
			if (fabs(solutionVector[row] - solutionVectorCompare[row]) > TENSORMULTMATH_TEST_ERROR ) {
				error_flag = True;
				Journal_Printf(stream, "solutionVector[%d]=%f not within tolerance of solution, %f\n", row, solutionVector[row], solutionVectorCompare[row]);
			}
		}
		if (error_flag == False) {
			Journal_Printf(stream, "SolutionVector within tolerance %g of solution:\n", TENSORMULTMATH_TEST_ERROR);
			StGermain_PrintNamedVector(stream, solutionVectorCompare, 4);
		}
		else {
			Journal_Printf(stream, "SolutionVector not within tolerance %g of solution:\n", TENSORMULTMATH_TEST_ERROR);
			StGermain_PrintNamedVector(stream, solutionVectorCompare, 4);
		}
		
		Memory_Free(nonSquareMatrixA);
		Memory_Free(nonSquareMatrixB);
		Memory_Free(nonSquareMatrixAT);
		Memory_Free(nonSquareMatrixResult);



	}
	
	DiscretisationGeometry_Finalise();
	
	Base_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0;
}

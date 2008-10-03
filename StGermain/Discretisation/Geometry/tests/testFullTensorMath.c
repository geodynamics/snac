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
	
	stream = Journal_Register( InfoStream_Type, "FullTensorMath" );

	stJournal->firewallProducesAssert = False;
	Stream_RedirectFile(Journal_Register( Error_Type, "FullTensorMath"), "FullTensorMath.txt");
	Stream_RedirectFile(stream, "FullTensorMath.txt");
	
	
	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}

	if( rank == procToWatch ) {
		double **tensor = Memory_Alloc_2DArray( double, 5, 5, "Tensor" );
		TensorArray     tensorArray;
		ComplexTensorArray complexTensorArray;
		ComplexEigenvector eigenvectorList[3], eigenvectorListCompare[3], eigenvectorListDiffs[3];
		Cmplx **complexMatrix;
		double errorTolerance;
		Cmplx dummy1, dummy2, dummy3;
		int eigenCount;
		#define STG_FULLTENSORTEST_ERROR 1.0e-6;
		
		errorTolerance = STG_FULLTENSORTEST_ERROR;
		complexMatrix = Memory_Alloc_2DArray(Cmplx, 3, 3, "complexMatrix" );
		
		tensor[0][0] = 3;  tensor[0][1] = 21;   tensor[0][2] = -1; tensor[0][3] = -99; tensor[0][4] = 9;
		tensor[1][0] = 6;  tensor[1][1] = 5.8;  tensor[1][2] = 32; tensor[1][3] = 3  ; tensor[1][4] = -2.5;
		tensor[2][0] = 2;  tensor[2][1] = 2;    tensor[2][2] = -7; tensor[2][3] = 2  ; tensor[2][4] = 3.1;
		tensor[3][0] = -4; tensor[3][1] = 9;    tensor[3][2] = 3 ; tensor[3][3] = 8  ; tensor[3][4] = 6;
		tensor[4][0] = 3;  tensor[4][1] = 1;    tensor[4][2] = 9 ; tensor[4][3] = 2  ; tensor[4][4] = 12;
		
		Journal_Printf(stream, "\n/*******************    Test Eigenvector 1   ************************/\n");
		Journal_Printf( stream, "Symmetric 2D Case from Kresig, p. 371f\n\n");
		tensorArray[FT2D_00] = -5;
		tensorArray[FT2D_11] = -2;
		tensorArray[FT2D_01] =  2;
		tensorArray[FT2D_10] =  2;

		Journal_Printf( stream, "Matrix to solve for eigenvectors is:\n");
		Journal_PrintTensorArray( stream, tensorArray, 2 );

		TensorArray_CalcAllEigenvectors( tensorArray, 2, eigenvectorList );
		
		/* Set comparison eigenvecorList with correct solution */
		eigenvectorListCompare[0].eigenvalue[REAL_PART] = -1;
		eigenvectorListCompare[0].eigenvalue[IMAG_PART] = 0;
		
		eigenvectorListCompare[1].eigenvalue[REAL_PART] = -6;
		eigenvectorListCompare[1].eigenvalue[IMAG_PART] = 0;

		eigenvectorListCompare[0].vector[0][REAL_PART] = -0.447214;
		eigenvectorListCompare[0].vector[0][IMAG_PART] = 0;

		eigenvectorListCompare[0].vector[1][REAL_PART] = -0.894427;
		eigenvectorListCompare[0].vector[1][IMAG_PART] = 0;

		eigenvectorListCompare[1].vector[0][REAL_PART] = -0.894427;
		eigenvectorListCompare[1].vector[0][IMAG_PART] = 0;

		eigenvectorListCompare[1].vector[1][REAL_PART] = 0.447214;
		eigenvectorListCompare[1].vector[1][IMAG_PART] = 0;
		
		/*Check if eigenvalue solution is within tolerance */
		Cmplx_Subtract(eigenvectorListCompare[0].eigenvalue, eigenvectorList[0].eigenvalue, 
						eigenvectorListDiffs[0].eigenvalue);
		Cmplx_Subtract(eigenvectorListCompare[1].eigenvalue, eigenvectorList[1].eigenvalue, 
						eigenvectorListDiffs[1].eigenvalue);

		if ( ( Cmplx_Modulus(eigenvectorListDiffs[0].eigenvalue) <= errorTolerance ) && 
			(Cmplx_Modulus(eigenvectorListDiffs[1].eigenvalue) <= errorTolerance) ) 
		{		
			Journal_Printf(stream, "\nEigenvalues within tolerance, %g of:\n", errorTolerance); 
		}
		else {
			Journal_Printf(stream, "\nEigenvalues not within tolerance, %g of:\n", errorTolerance); 
		}
		
		/* Print correct eigenvalues */
		Journal_PrintCmplx(stream,eigenvectorListCompare[0].eigenvalue );
		Journal_PrintCmplx(stream,eigenvectorListCompare[1].eigenvalue );
				/* Check if eigenvector solutions are within tolerance */
		StGermain_ComplexVectorSubtraction(	eigenvectorListDiffs[0].vector,
				eigenvectorListCompare[0].vector,
				eigenvectorList[0].vector, 2);
		StGermain_ComplexVectorSubtraction(	eigenvectorListDiffs[1].vector, 
				eigenvectorListCompare[1].vector,
				eigenvectorList[1].vector, 2);		
		
		if ( (StGermain_ComplexVectorMagnitude(eigenvectorListDiffs[0].vector, 2) <=errorTolerance) &&
			(StGermain_ComplexVectorMagnitude(eigenvectorListDiffs[1].vector, 2) <=errorTolerance) )
		{ 
				
			Journal_Printf(stream, "\nEigenvectors within tolerance, %g of:\n", errorTolerance); 
		}
		else {
			Journal_Printf(stream, "\nEigenvectors not within tolerance, %g of:\n", errorTolerance); 
		}
		/* Print out correct eigenvectors */		
		StGermain_PrintNamedComplexVector( stream,eigenvectorListCompare[0].vector ,2);
		StGermain_PrintNamedComplexVector( stream,eigenvectorListCompare[1].vector ,2);

		Journal_Printf( stream, "\n/*******************    Test Eigenvector 2   ************************/\n");
		Journal_Printf( stream, "Symmetric 3D Case - tested against:\n");
		Journal_Printf( stream, "http://www.arndt-bruenner.de/mathe/scripts/engl_eigenwert.htm - 3/11/04.\n");
		tensorArray[FT3D_00] = 2;
		tensorArray[FT3D_11] = 3;
		tensorArray[FT3D_22] = 5;
		tensorArray[FT3D_01] = 7;
		tensorArray[FT3D_02] = 11;
		tensorArray[FT3D_12] = 13;
		tensorArray[FT3D_10] = 7;
		tensorArray[FT3D_20] = 11;
		tensorArray[FT3D_21] = 13;		

		Journal_Printf( stream, "Matrix to solve for eigenvectors is:\n");
		Journal_PrintTensorArray( stream, tensorArray, 3 );

		TensorArray_CalcAllEigenvectors( tensorArray, 3, eigenvectorList );

	/* Set comparison eigenvecorList with correct solution */
		eigenvectorListCompare[0].eigenvalue[REAL_PART] = -4.45970173;
		eigenvectorListCompare[0].eigenvalue[IMAG_PART] = 0;
		
		eigenvectorListCompare[1].eigenvalue[REAL_PART] = -9.96854402;
		eigenvectorListCompare[1].eigenvalue[IMAG_PART] = 0;
		
		eigenvectorListCompare[2].eigenvalue[REAL_PART] = 24.42824575;
		eigenvectorListCompare[2].eigenvalue[IMAG_PART] = 0;
	
		dummy1[IMAG_PART] = 0;
		dummy2[IMAG_PART] = 0;
		dummy3[IMAG_PART] = 0;

		dummy1[REAL_PART] = -0.786471; 
		dummy2[REAL_PART] =  0.61348; 	
		dummy3[REAL_PART] =  0.0714557; 			
		ComplexVector_SetScalar( dummy1, dummy2, dummy3, 
								eigenvectorListCompare[0].vector );
		dummy1[REAL_PART] = -0.364102; 
		dummy2[REAL_PART] = -0.553977; 
		dummy3[REAL_PART] =  0.748692; 
		ComplexVector_SetScalar( dummy1, dummy2, dummy3, 
								eigenvectorListCompare[1].vector );
		dummy1[REAL_PART] = -0.498892; 
		dummy2[REAL_PART] = -0.562807; 
		dummy3[REAL_PART] = -0.659056; 								
		ComplexVector_SetScalar( dummy1, dummy2, dummy3, 
								eigenvectorListCompare[2].vector );
		
		/*Check if eigenvalue solution is within tolerance */
		Cmplx_Subtract(eigenvectorListCompare[0].eigenvalue, eigenvectorList[0].eigenvalue, 
						eigenvectorListDiffs[0].eigenvalue);
		Cmplx_Subtract(eigenvectorListCompare[1].eigenvalue, eigenvectorList[1].eigenvalue, 
						eigenvectorListDiffs[1].eigenvalue);
		Cmplx_Subtract(eigenvectorListCompare[2].eigenvalue, eigenvectorList[2].eigenvalue, 
						eigenvectorListDiffs[2].eigenvalue);

		if ( ( Cmplx_Modulus(eigenvectorListDiffs[0].eigenvalue) <= errorTolerance ) && 
			(Cmplx_Modulus(eigenvectorListDiffs[1].eigenvalue) <= errorTolerance) && 
			(Cmplx_Modulus(eigenvectorListDiffs[2].eigenvalue) <= errorTolerance) ) {		
			Journal_Printf(stream, "\nEigenvalues within tolerance, %g of:\n", errorTolerance); 
		}
		else {
			Journal_Printf(stream, "\nEigenvalues not within tolerance, %g of:\n", errorTolerance); 
		}
		
		/* Print correct eigenvalues */
		Journal_PrintCmplx(stream,eigenvectorListCompare[0].eigenvalue );
		Journal_PrintCmplx(stream,eigenvectorListCompare[1].eigenvalue );
		Journal_PrintCmplx(stream,eigenvectorListCompare[2].eigenvalue );
		
		/* Check if eigenvector solutions are within tolerance */
		StGermain_ComplexVectorSubtraction(	eigenvectorListDiffs[0].vector,
				eigenvectorListCompare[0].vector,
				eigenvectorList[0].vector, 3);
		StGermain_ComplexVectorSubtraction(	eigenvectorListDiffs[1].vector, 
				eigenvectorListCompare[1].vector,
				eigenvectorList[1].vector, 3);		
		StGermain_ComplexVectorSubtraction(	eigenvectorListDiffs[2].vector, 
				eigenvectorListCompare[2].vector,
				eigenvectorList[2].vector, 3);
		
		
		if ( (StGermain_ComplexVectorMagnitude(eigenvectorListDiffs[0].vector, 3) <=errorTolerance) &&
			(StGermain_ComplexVectorMagnitude(eigenvectorListDiffs[1].vector, 3) <=errorTolerance) &&
			(StGermain_ComplexVectorMagnitude(eigenvectorListDiffs[2].vector, 3) <=errorTolerance) )
		{ 
				
			Journal_Printf(stream, "\nEigenvectors within tolerance, %g of:\n", errorTolerance); 
		}
		else {
			Journal_Printf(stream, "\nEigenvectors not within tolerance, %g of:\n", errorTolerance); 
		}
		/* Print out correct eigenvectors */		
		StGermain_PrintNamedComplexVector( stream,eigenvectorListCompare[0].vector ,3);
		StGermain_PrintNamedComplexVector( stream,eigenvectorListCompare[1].vector ,3);
		StGermain_PrintNamedComplexVector( stream,eigenvectorListCompare[2].vector ,3);
		
		Journal_Printf( stream, "\n/*******************    Test Eigenvector 3   ************************/\n");
		Journal_Printf( stream, "Non-Symmetric test for 2-D\n\n");
		
		tensorArray[FT2D_00] = 4;
		tensorArray[FT2D_01] = 4;
		tensorArray[FT2D_10] = 3;
		tensorArray[FT2D_11] = 5;
							
		Journal_PrintTensorArray(stream, tensorArray, 2);
		TensorArray_CalcAllEigenvectors( tensorArray, 2, eigenvectorList );
		
		/* Set comparison eigenvecorList with correct solution */
		eigenvectorListCompare[0].eigenvalue[REAL_PART] = 1;
		eigenvectorListCompare[0].eigenvalue[IMAG_PART] = 0;
		
		eigenvectorListCompare[1].eigenvalue[REAL_PART] = 8;
		eigenvectorListCompare[1].eigenvalue[IMAG_PART] = 0;

		eigenvectorListCompare[0].vector[0][REAL_PART] = -0.8 ;
		eigenvectorListCompare[0].vector[0][IMAG_PART] = 0;

		eigenvectorListCompare[0].vector[1][REAL_PART] = 0.6;
		eigenvectorListCompare[0].vector[1][IMAG_PART] = 0;

		eigenvectorListCompare[1].vector[0][REAL_PART] = -0.707107 ;
		eigenvectorListCompare[1].vector[0][IMAG_PART] = 0;

		eigenvectorListCompare[1].vector[1][REAL_PART] = -0.707107;
		eigenvectorListCompare[1].vector[1][IMAG_PART] = 0;
		
		/*Check if eigenvalue solution is within tolerance */
		Cmplx_Subtract(eigenvectorListCompare[0].eigenvalue, eigenvectorList[0].eigenvalue, 
						eigenvectorListDiffs[0].eigenvalue);
		Cmplx_Subtract(eigenvectorListCompare[1].eigenvalue, eigenvectorList[1].eigenvalue, 
						eigenvectorListDiffs[1].eigenvalue);

		if ( ( Cmplx_Modulus(eigenvectorListDiffs[0].eigenvalue) <= errorTolerance ) && 
			(Cmplx_Modulus(eigenvectorListDiffs[1].eigenvalue) <= errorTolerance) ) 
		{		
			Journal_Printf(stream, "\nEigenvalues within tolerance, %g of:\n", errorTolerance); 
		}
		else {
			Journal_Printf(stream, "\nEigenvalues not within tolerance, %g of:\n", errorTolerance); 
		}
		
		/* Print correct eigenvalues */
		Journal_PrintCmplx(stream,eigenvectorListCompare[0].eigenvalue );
		Journal_PrintCmplx(stream,eigenvectorListCompare[1].eigenvalue );
				/* Check if eigenvector solutions are within tolerance */
		StGermain_ComplexVectorSubtraction(	eigenvectorListDiffs[0].vector,
				eigenvectorListCompare[0].vector,
				eigenvectorList[0].vector, 2);
		StGermain_ComplexVectorSubtraction(	eigenvectorListDiffs[1].vector, 
				eigenvectorListCompare[1].vector,
				eigenvectorList[1].vector, 2);		
		
		if ( (StGermain_ComplexVectorMagnitude(eigenvectorListDiffs[0].vector, 2) <=errorTolerance) &&
			(StGermain_ComplexVectorMagnitude(eigenvectorListDiffs[1].vector, 2) <=errorTolerance) )
		{ 
				
			Journal_Printf(stream, "\nEigenvectors within tolerance, %g of:\n", errorTolerance); 
		}
		else {
			Journal_Printf(stream, "\nEigenvectors not within tolerance, %g of:\n", errorTolerance); 
		}
		/* Print out correct eigenvectors */		
		StGermain_PrintNamedComplexVector( stream,eigenvectorListCompare[0].vector ,2);
		StGermain_PrintNamedComplexVector( stream,eigenvectorListCompare[1].vector ,2);

		Journal_Printf( stream, "\n");

		Journal_Printf( stream, "\n/*******************    Test Eigenvector 4   ************************/\n");
		Journal_Printf( stream, "Non-Symmetric Test for 3-D\n\n");
		
		tensorArray[FT3D_00] = 4;
		tensorArray[FT3D_01] = 0;
		tensorArray[FT3D_02] = 3;
		tensorArray[FT3D_10] = 0;
		tensorArray[FT3D_11] = 5;
		tensorArray[FT3D_12] = 0;
		tensorArray[FT3D_20] = 2;		
		tensorArray[FT3D_21] = 5;
		tensorArray[FT3D_22] = 6;						

		Journal_PrintTensorArray(stream, tensorArray, 3);
		TensorArray_CalcAllEigenvectors( tensorArray, 3, eigenvectorList );

		/* Set comparison eigenvecorList with correct solution */
		eigenvectorListCompare[0].eigenvalue[REAL_PART] = 2.35424868893;
		eigenvectorListCompare[0].eigenvalue[IMAG_PART] = 0;
		
		eigenvectorListCompare[1].eigenvalue[REAL_PART] = 5;
		eigenvectorListCompare[1].eigenvalue[IMAG_PART] = 0;
		
		eigenvectorListCompare[2].eigenvalue[REAL_PART] = 7.645751311;
		eigenvectorListCompare[2].eigenvalue[IMAG_PART] = 0;
	
		dummy1[IMAG_PART] = 0;
		dummy2[IMAG_PART] = 0;
		dummy3[IMAG_PART] = 0;

		dummy1[REAL_PART] = -0.87674 ; 
		dummy2[REAL_PART] =  0; 	
		dummy3[REAL_PART] =  0.480965 ; 			
		ComplexVector_SetScalar( dummy1, dummy2, dummy3, 
								eigenvectorListCompare[0].vector );
		dummy1[REAL_PART] = -0.867472 ; 
		dummy2[REAL_PART] = 0.40482; 
		dummy3[REAL_PART] =  -0.289157 ; 
		ComplexVector_SetScalar( dummy1, dummy2, dummy3, 
								eigenvectorListCompare[1].vector );
		dummy1[REAL_PART] = -0.635406; 
		dummy2[REAL_PART] = 0; 
		dummy3[REAL_PART] = -0.772178; 								
		ComplexVector_SetScalar( dummy1, dummy2, dummy3, 
								eigenvectorListCompare[2].vector );
		
		/*Check if eigenvalue solution is within tolerance */
		Cmplx_Subtract(eigenvectorListCompare[0].eigenvalue, eigenvectorList[0].eigenvalue, 
						eigenvectorListDiffs[0].eigenvalue);
		Cmplx_Subtract(eigenvectorListCompare[1].eigenvalue, eigenvectorList[1].eigenvalue, 
						eigenvectorListDiffs[1].eigenvalue);
		Cmplx_Subtract(eigenvectorListCompare[2].eigenvalue, eigenvectorList[2].eigenvalue, 
						eigenvectorListDiffs[2].eigenvalue);

		if ( ( Cmplx_Modulus(eigenvectorListDiffs[0].eigenvalue) <= errorTolerance ) && 
			(Cmplx_Modulus(eigenvectorListDiffs[1].eigenvalue) <= errorTolerance) && 
			(Cmplx_Modulus(eigenvectorListDiffs[2].eigenvalue) <= errorTolerance) ) {		
			Journal_Printf(stream, "\nEigenvalues within tolerance, %g of:\n", errorTolerance); 
		}
		else {
			Journal_Printf(stream, "\nEigenvalues not within tolerance, %g of:\n", errorTolerance); 
		}
		
		/* Print correct eigenvalues */
		Journal_PrintCmplx(stream,eigenvectorListCompare[0].eigenvalue );
		Journal_PrintCmplx(stream,eigenvectorListCompare[1].eigenvalue );
		Journal_PrintCmplx(stream,eigenvectorListCompare[2].eigenvalue );
		
		/* Check if eigenvector solutions are within tolerance */
		StGermain_ComplexVectorSubtraction(	eigenvectorListDiffs[0].vector,
				eigenvectorListCompare[0].vector,
				eigenvectorList[0].vector, 3);
		StGermain_ComplexVectorSubtraction(	eigenvectorListDiffs[1].vector, 
				eigenvectorListCompare[1].vector,
				eigenvectorList[1].vector, 3);		
		StGermain_ComplexVectorSubtraction(	eigenvectorListDiffs[2].vector, 
				eigenvectorListCompare[2].vector,
				eigenvectorList[2].vector, 3);
		
		
		if ( (StGermain_ComplexVectorMagnitude(eigenvectorListDiffs[0].vector, 3) <=errorTolerance) &&
			(StGermain_ComplexVectorMagnitude(eigenvectorListDiffs[1].vector, 3) <=errorTolerance) &&
			(StGermain_ComplexVectorMagnitude(eigenvectorListDiffs[2].vector, 3) <=errorTolerance) )
		{ 
				
			Journal_Printf(stream, "\nEigenvectors within tolerance, %g of:\n", errorTolerance); 
		}
		else {
			Journal_Printf(stream, "\nEigenvectors not within tolerance, %g of:\n", errorTolerance); 
		}
		/* Print out correct eigenvectors */		
		StGermain_PrintNamedComplexVector( stream,eigenvectorListCompare[0].vector ,3);
		StGermain_PrintNamedComplexVector( stream,eigenvectorListCompare[1].vector ,3);
		StGermain_PrintNamedComplexVector( stream,eigenvectorListCompare[2].vector ,3);		
			
		Journal_Printf( stream, "\n");
	
		Journal_Printf( stream, "\n/*******************    Test Eigenvector 5   ************************/\n");
		Journal_Printf( stream, "Non-Symmetric test with zero entries for 2-D\n\n");
		
		tensorArray[FT2D_00] = 4;
		tensorArray[FT2D_01] = 0;
		tensorArray[FT2D_10] = 1;
		tensorArray[FT2D_11] = 5;
							
		Journal_PrintTensorArray(stream, tensorArray, 2);
		TensorArray_CalcAllEigenvectors( tensorArray, 2, eigenvectorList );

		/* Set comparison eigenvecorList with correct solution */
		eigenvectorListCompare[0].eigenvalue[REAL_PART] = 4;
		eigenvectorListCompare[0].eigenvalue[IMAG_PART] = 0;
		
		eigenvectorListCompare[1].eigenvalue[REAL_PART] = 5;
		eigenvectorListCompare[1].eigenvalue[IMAG_PART] = 0;

		eigenvectorListCompare[0].vector[0][REAL_PART] = 0.707107;
		eigenvectorListCompare[0].vector[0][IMAG_PART] = 0;

		eigenvectorListCompare[0].vector[1][REAL_PART] = -0.707107;
		eigenvectorListCompare[0].vector[1][IMAG_PART] = 0;

		eigenvectorListCompare[1].vector[0][REAL_PART] = 0;
		eigenvectorListCompare[1].vector[0][IMAG_PART] = 0;

		eigenvectorListCompare[1].vector[1][REAL_PART] = 1;
		eigenvectorListCompare[1].vector[1][IMAG_PART] = 0;
		
		/*Check if eigenvalue solution is within tolerance */
		Cmplx_Subtract(eigenvectorListCompare[0].eigenvalue, eigenvectorList[0].eigenvalue, 
						eigenvectorListDiffs[0].eigenvalue);
		Cmplx_Subtract(eigenvectorListCompare[1].eigenvalue, eigenvectorList[1].eigenvalue, 
						eigenvectorListDiffs[1].eigenvalue);

		if ( ( Cmplx_Modulus(eigenvectorListDiffs[0].eigenvalue) <= errorTolerance ) && 
			(Cmplx_Modulus(eigenvectorListDiffs[1].eigenvalue) <= errorTolerance) ) 
		{		
			Journal_Printf(stream, "\nEigenvalues within tolerance, %g of:\n", errorTolerance); 
		}
		else {
			Journal_Printf(stream, "\nEigenvalues not within tolerance, %g of:\n", errorTolerance); 
		}
		
		/* Print correct eigenvalues */
		Journal_PrintCmplx(stream,eigenvectorListCompare[0].eigenvalue );
		Journal_PrintCmplx(stream,eigenvectorListCompare[1].eigenvalue );
				
		/* Check if eigenvector solutions are within tolerance */
		StGermain_ComplexVectorSubtraction(	eigenvectorListDiffs[0].vector,
				eigenvectorListCompare[0].vector,
				eigenvectorList[0].vector, 2);
		StGermain_ComplexVectorSubtraction(	eigenvectorListDiffs[1].vector, 
				eigenvectorListCompare[1].vector,
				eigenvectorList[1].vector, 2);		
		
		if ( (StGermain_ComplexVectorMagnitude(eigenvectorListDiffs[0].vector, 2) <=errorTolerance) &&
			(StGermain_ComplexVectorMagnitude(eigenvectorListDiffs[1].vector, 2) <=errorTolerance) )
		{ 
				
			Journal_Printf(stream, "\nEigenvectors within tolerance, %g of:\n", errorTolerance); 
		}
		else {
			Journal_Printf(stream, "\nEigenvectors not within tolerance, %g of:\n", errorTolerance); 
		}
		/* Print out correct eigenvectors */		
		StGermain_PrintNamedComplexVector( stream,eigenvectorListCompare[0].vector ,2);
		StGermain_PrintNamedComplexVector( stream,eigenvectorListCompare[1].vector ,2);

		Journal_Printf( stream, "\n");
		
		/* Test for complex answers */
		Journal_Printf( stream, "\n/*******************    Test Eigenvector 6   ************************/\n");
		Journal_Printf( stream, "Non-Symmetric test with complex eigenvalues for 2-D\n\n");
		Journal_Printf( stream, "Tested against Soluions in:\n");	
		Journal_Printf( stream, "Elementary Differential Equations and Boundary Value Problems, 6th Ed\n");
		Journal_Printf( stream, "By William E. Boyce and Richard C. DiPrima\n");	
		Journal_Printf( stream, "Problem: ch 7.3, question 16\n\n");
		tensorArray[FT2D_00] =  3;
		tensorArray[FT2D_01] = -2;
		tensorArray[FT2D_10] =  4;
		tensorArray[FT2D_11] = -1;
		
		Journal_PrintTensorArray(stream, tensorArray, 2);
		TensorArray_CalcAllEigenvectors( tensorArray, 2, eigenvectorList );
		
		/* Set comparison eigenvecorList with correct solution */
		eigenvectorListCompare[0].eigenvalue[REAL_PART] = 1;
		eigenvectorListCompare[0].eigenvalue[IMAG_PART] = 2;
		
		eigenvectorListCompare[1].eigenvalue[REAL_PART] = 1;
		eigenvectorListCompare[1].eigenvalue[IMAG_PART] = -2;

		eigenvectorListCompare[0].vector[0][REAL_PART] = 0.408248;
		eigenvectorListCompare[0].vector[0][IMAG_PART] = 0.408248;

		eigenvectorListCompare[0].vector[1][REAL_PART] = 0.816497 ;
		eigenvectorListCompare[0].vector[1][IMAG_PART] = 0;

		eigenvectorListCompare[1].vector[0][REAL_PART] = 0.408248;
		eigenvectorListCompare[1].vector[0][IMAG_PART] = -0.408248;

		eigenvectorListCompare[1].vector[1][REAL_PART] = 0.816497;
		eigenvectorListCompare[1].vector[1][IMAG_PART] = 0;
		
		/*Check if eigenvalue solution is within tolerance */
		Cmplx_Subtract(eigenvectorListCompare[0].eigenvalue, eigenvectorList[0].eigenvalue, 
						eigenvectorListDiffs[0].eigenvalue);
		Cmplx_Subtract(eigenvectorListCompare[1].eigenvalue, eigenvectorList[1].eigenvalue, 
						eigenvectorListDiffs[1].eigenvalue);

		if ( ( Cmplx_Modulus(eigenvectorListDiffs[0].eigenvalue) <= errorTolerance ) && 
			(Cmplx_Modulus(eigenvectorListDiffs[1].eigenvalue) <= errorTolerance) ) 
		{		
			Journal_Printf(stream, "\nEigenvalues within tolerance, %g of:\n", errorTolerance); 
		}
		else {
			Journal_Printf(stream, "\nEigenvalues not within tolerance, %g of:\n", errorTolerance); 
		}
		
		/* Print correct eigenvalues */
		Journal_PrintCmplx(stream,eigenvectorListCompare[0].eigenvalue );
		Journal_PrintCmplx(stream,eigenvectorListCompare[1].eigenvalue );
				/* Check if eigenvector solutions are within tolerance */
		StGermain_ComplexVectorSubtraction(	eigenvectorListDiffs[0].vector,
				eigenvectorListCompare[0].vector,
				eigenvectorList[0].vector, 2);
		StGermain_ComplexVectorSubtraction(	eigenvectorListDiffs[1].vector, 
				eigenvectorListCompare[1].vector,
				eigenvectorList[1].vector, 2);		
		
		if ( (StGermain_ComplexVectorMagnitude(eigenvectorListDiffs[0].vector, 2) <=errorTolerance) &&
			(StGermain_ComplexVectorMagnitude(eigenvectorListDiffs[1].vector, 2) <=errorTolerance) )
		{ 
				
			Journal_Printf(stream, "\nEigenvectors within tolerance, %g of:\n", errorTolerance); 
		}
		else {
			Journal_Printf(stream, "\nEigenvectors not within tolerance, %g of:\n", errorTolerance); 
		}
		/* Print out correct eigenvectors */		
		StGermain_PrintNamedComplexVector( stream,eigenvectorListCompare[0].vector ,2);
		StGermain_PrintNamedComplexVector( stream,eigenvectorListCompare[1].vector ,2);
	
		Journal_Printf( stream, "\n/*******************    Test Eigenvector 7   ************************/\n");
		Journal_Printf( stream, "Non-Symmetric test with complex eigenvalues for 3-D\n");
		Journal_Printf( stream, "Tested against Soluions in:\n");	
		Journal_Printf( stream, "Elementary Differential Equations and Boundary Value Problems, 6th Ed\n");
		Journal_Printf( stream, "By William E. Boyce and Richard C. DiPrima\n");	
		Journal_Printf( stream, "Problem: ch 7.3, question 21\n\n");		
		tensorArray[FT3D_00] = 1;
		tensorArray[FT3D_01] = 0;
		tensorArray[FT3D_02] = 0;
		tensorArray[FT3D_10] = 2;
		tensorArray[FT3D_11] = 1;
		tensorArray[FT3D_12] = -2;
		tensorArray[FT3D_20] = 3;		
		tensorArray[FT3D_21] = 2;
		tensorArray[FT3D_22] = 1;
		
		Journal_PrintTensorArray(stream, tensorArray, 3);
		TensorArray_CalcAllEigenvectors( tensorArray, 3, eigenvectorList );
		/* Set comparison eigenvecorList with correct solution */
		eigenvectorListCompare[0].eigenvalue[REAL_PART] = 1;
		eigenvectorListCompare[0].eigenvalue[IMAG_PART] = 0;
		
		eigenvectorListCompare[1].eigenvalue[REAL_PART] = 1;
		eigenvectorListCompare[1].eigenvalue[IMAG_PART] = 2;
		
		eigenvectorListCompare[2].eigenvalue[REAL_PART] = 1;
		eigenvectorListCompare[2].eigenvalue[IMAG_PART] = -2;
		
		dummy1[REAL_PART] =  0.485071 ; 	dummy1[IMAG_PART] = 0;
		dummy2[REAL_PART] = -0.727607; 		dummy2[IMAG_PART] = 0;
		dummy3[REAL_PART] =  0.485071; 		dummy3[IMAG_PART] = 0;

		ComplexVector_SetScalar( dummy1, dummy2, dummy3, 
								eigenvectorListCompare[0].vector );

		dummy1[REAL_PART] = 0; 				dummy1[IMAG_PART] = 0;
		dummy2[REAL_PART] = 0.707107; 		dummy2[IMAG_PART] = 0;
		dummy3[REAL_PART] = 0;				dummy3[IMAG_PART] = -0.707107; 

		ComplexVector_SetScalar( dummy1, dummy2, dummy3, 
								eigenvectorListCompare[1].vector );

		dummy1[REAL_PART] = 0; 				dummy1[IMAG_PART] = 0;
		dummy2[REAL_PART] = 0.707107; 		dummy2[IMAG_PART] = 0;
		dummy3[REAL_PART] = 0;				dummy3[IMAG_PART] = 0.707107;

		ComplexVector_SetScalar( dummy1, dummy2, dummy3, 
								eigenvectorListCompare[2].vector );
		
		/*Check if eigenvalue solution is within tolerance */
		Cmplx_Subtract(eigenvectorListCompare[0].eigenvalue, eigenvectorList[0].eigenvalue, 
						eigenvectorListDiffs[0].eigenvalue);
		Cmplx_Subtract(eigenvectorListCompare[1].eigenvalue, eigenvectorList[1].eigenvalue, 
						eigenvectorListDiffs[1].eigenvalue);
		Cmplx_Subtract(eigenvectorListCompare[2].eigenvalue, eigenvectorList[2].eigenvalue, 
						eigenvectorListDiffs[2].eigenvalue);

		if ( ( Cmplx_Modulus(eigenvectorListDiffs[0].eigenvalue) <= errorTolerance ) && 
			(Cmplx_Modulus(eigenvectorListDiffs[1].eigenvalue) <= errorTolerance) && 
			(Cmplx_Modulus(eigenvectorListDiffs[2].eigenvalue) <= errorTolerance) ) {		
			Journal_Printf(stream, "\nEigenvalues within tolerance, %g of:\n", errorTolerance); 
		}
		else {
			Journal_Printf(stream, "\nEigenvalues not within tolerance, %g of:\n", errorTolerance); 
		}
		
		/* Print correct eigenvalues */
		Journal_PrintCmplx(stream,eigenvectorListCompare[0].eigenvalue );
		Journal_PrintCmplx(stream,eigenvectorListCompare[1].eigenvalue );
		Journal_PrintCmplx(stream,eigenvectorListCompare[2].eigenvalue );
		
		/* Check if eigenvector solutions are within tolerance */
		StGermain_ComplexVectorSubtraction(	eigenvectorListDiffs[0].vector,
				eigenvectorListCompare[0].vector,
				eigenvectorList[0].vector, 3);
		StGermain_ComplexVectorSubtraction(	eigenvectorListDiffs[1].vector, 
				eigenvectorListCompare[1].vector,
				eigenvectorList[1].vector, 3);		
		StGermain_ComplexVectorSubtraction(	eigenvectorListDiffs[2].vector, 
				eigenvectorListCompare[2].vector,
				eigenvectorList[2].vector, 3);
		
		
		if ( (StGermain_ComplexVectorMagnitude(eigenvectorListDiffs[0].vector, 3) <=errorTolerance) &&
			(StGermain_ComplexVectorMagnitude(eigenvectorListDiffs[1].vector, 3) <=errorTolerance) &&
			(StGermain_ComplexVectorMagnitude(eigenvectorListDiffs[2].vector, 3) <=errorTolerance) )
		{ 
				
			Journal_Printf(stream, "\nEigenvectors within tolerance, %g of:\n", errorTolerance); 
		}
		else {
			Journal_Printf(stream, "\nEigenvectors not within tolerance, %g of:\n", errorTolerance); 
		}
		/* Print out correct eigenvectors */		
		StGermain_PrintNamedComplexVector( stream,eigenvectorListCompare[0].vector ,3);
		StGermain_PrintNamedComplexVector( stream,eigenvectorListCompare[1].vector ,3);
		StGermain_PrintNamedComplexVector( stream,eigenvectorListCompare[2].vector ,3);		

		Journal_Printf( stream, "\n");
		Journal_Printf( stream, "\n/*******************    Eigenvector Test  8  ************************/\n");
		Journal_Printf( stream, "Test Calc eigenvectors function with repeated roots\n\n");		
		Journal_Printf( stream, "Tested against Solutions in:\n");	
		Journal_Printf( stream, "Elementary Differential Equations and Boundary Value Problems, 6th Ed\n");
		Journal_Printf( stream, "By William E. Boyce and Richard C. DiPrima\n");	
		Journal_Printf( stream, "Problem: ch 7.3, question 24\n\n");
		tensorArray[FT3D_00] = 3;
		tensorArray[FT3D_01] = 2;
		tensorArray[FT3D_02] = 4;
		tensorArray[FT3D_10] = 2;
		tensorArray[FT3D_11] = 0;
		tensorArray[FT3D_12] = 2;
		tensorArray[FT3D_20] = 4;		
		tensorArray[FT3D_21] = 2;
		tensorArray[FT3D_22] = 3;
		
		Journal_PrintTensorArray(stream, tensorArray, 3);
		TensorArray_CalcAllEigenvectors( tensorArray, 3, eigenvectorList );

		/* Set comparison eigenvecorList with correct solution */
		eigenvectorListCompare[0].eigenvalue[REAL_PART] = -1;
		eigenvectorListCompare[0].eigenvalue[IMAG_PART] = 0;
		
		eigenvectorListCompare[1].eigenvalue[REAL_PART] = -1;
		eigenvectorListCompare[1].eigenvalue[IMAG_PART] = 0;
		
		eigenvectorListCompare[2].eigenvalue[REAL_PART] = 8;
		eigenvectorListCompare[2].eigenvalue[IMAG_PART] = 0;

		dummy1[IMAG_PART] = 0;
		dummy2[IMAG_PART] = 0;
		dummy3[IMAG_PART] = 0;

		dummy1[REAL_PART] = 0; 
		dummy2[REAL_PART] = -0.894427; 
		dummy3[REAL_PART] = 0.447214;
		
		ComplexVector_SetScalar( dummy1, dummy2, dummy3, 
								eigenvectorListCompare[0].vector );

		dummy1[REAL_PART] = -0.719905 ; 
		dummy2[REAL_PART] = 0.519697; 
		dummy3[REAL_PART] = 0.460056; 								
		
		ComplexVector_SetScalar( dummy1, dummy2, dummy3, 
								eigenvectorListCompare[1].vector );
		dummy1[REAL_PART] = 0.666667; 
		dummy2[REAL_PART] = 0.333333; 
		dummy3[REAL_PART] = 0.666667; 								
		
		ComplexVector_SetScalar( dummy1, dummy2, dummy3, 
								eigenvectorListCompare[2].vector );
		
		/*Check if eigenvalue solution is within tolerance */
		Cmplx_Subtract(eigenvectorListCompare[0].eigenvalue, eigenvectorList[0].eigenvalue, 
						eigenvectorListDiffs[0].eigenvalue);
		Cmplx_Subtract(eigenvectorListCompare[1].eigenvalue, eigenvectorList[1].eigenvalue, 
						eigenvectorListDiffs[1].eigenvalue);
		Cmplx_Subtract(eigenvectorListCompare[2].eigenvalue, eigenvectorList[2].eigenvalue, 
						eigenvectorListDiffs[2].eigenvalue);

		if ( ( Cmplx_Modulus(eigenvectorListDiffs[0].eigenvalue) <= errorTolerance ) && 
			(Cmplx_Modulus(eigenvectorListDiffs[1].eigenvalue) <= errorTolerance) && 
			(Cmplx_Modulus(eigenvectorListDiffs[2].eigenvalue) <= errorTolerance) ) {		
			Journal_Printf(stream, "\nEigenvalues within tolerance, %g of:\n", errorTolerance); 
		}
		else {
			Journal_Printf(stream, "\nEigenvalues not within tolerance, %g of:\n", errorTolerance); 
		}
		
		/* Print correct eigenvalues */
		Journal_PrintCmplx(stream,eigenvectorListCompare[0].eigenvalue );
		Journal_PrintCmplx(stream,eigenvectorListCompare[1].eigenvalue );
		Journal_PrintCmplx(stream,eigenvectorListCompare[2].eigenvalue );
		
		/* Check if non-repeated eigenvector solution is within tolerance */

		StGermain_ComplexVectorSubtraction(	eigenvectorListDiffs[2].vector, 
				eigenvectorListCompare[2].vector,
				eigenvectorList[2].vector, 3);
		
		/* Check if repeated eigenvectors follow correct Pattern */
		for (eigenCount = 0; eigenCount < 2; eigenCount++) {
			Cmplx_RealMultiply(eigenvectorList[eigenCount].vector[0], 2.0, dummy1 );
			Cmplx_RealMultiply(eigenvectorList[eigenCount].vector[2], 2.0, dummy2 );
			Cmplx_Add( eigenvectorList[eigenCount].vector[1], dummy1, dummy3 ) ;
			Cmplx_Add( dummy3, dummy2, eigenvectorListDiffs[eigenCount].vector[0]);
		}
		
		if ( ((Cmplx_Modulus(eigenvectorListDiffs[0].vector[0])) <= errorTolerance ) &&
			((Cmplx_Modulus(eigenvectorListDiffs[1].vector[0])) <= errorTolerance ) &&
			(StGermain_ComplexVectorMagnitude(eigenvectorListDiffs[2].vector, 3) <=errorTolerance) )
		{ 
				
			Journal_Printf(stream, "\nNon-repeated Eigenvector within tolerance, %g,\n", errorTolerance);
			Journal_Printf(stream, "And Repeated Eigenvectors follow pattern:\n");
			Journal_Printf(stream, "2 * x_1 + x_2 + 2 * x_3 = 0 to within tolerance, %g of:\n", errorTolerance);
					
		}
		else {
			Journal_Printf(stream, "\nEither Non-repeated Eigenvector not within tolerance, %g,\n", errorTolerance);
			Journal_Printf(stream, "Or Repeated Eigenvectors do not follow pattern:\n");
			Journal_Printf(stream, "2 * x_1 + x_2 + 2 * x_3 = 0 to within tolerance, %g.\n", errorTolerance);
			Journal_Printf(stream, "Calculated solution:\n");
			StGermain_PrintNamedComplexVector( stream,eigenvectorList[0].vector ,3);
			StGermain_PrintNamedComplexVector( stream,eigenvectorList[1].vector ,3);
			StGermain_PrintNamedComplexVector( stream,eigenvectorList[2].vector ,3);				
		}
		/* Print out correct eigenvectors */
		Journal_Printf(stream, "One set of correct eigenvectors: \n");		
		StGermain_PrintNamedComplexVector( stream,eigenvectorListCompare[0].vector ,3);
		StGermain_PrintNamedComplexVector( stream,eigenvectorListCompare[1].vector ,3);
		StGermain_PrintNamedComplexVector( stream,eigenvectorListCompare[2].vector ,3);		
			
		Journal_Printf(stream, "\nFor repeated eigenvalues with non-degenerate eigenvectors, \n");
		Journal_Printf(stream, "eigenvectors have to be in same plane as solution vectors \n");
		Journal_Printf(stream, "rather than exactly the same, as eigenvectors can be arbitrarily set\n");
		Journal_Printf(stream, "based on the specific tensor array equation.\n");
		Journal_Printf(stream, "For this problem, solution for repeated eigenvectors collapses to:\n");
		Journal_Printf(stream, "2 * x_1 + x_2 + 2 * x_3 = 0\n");
		Journal_Printf(stream, "Eigenvectors are then set based on what x_1, x_2 and x_3 are set to equal.\n");
	
		
		
		/* Test solve complex system functions */
		Journal_Printf( stream, "\n/*******************    Test  10  ************************/\n");
		Journal_Printf( stream, "Test print ComplexTensorArray function\n\n");		
		Journal_Printf( stream, "2-D\n");
		complexTensorArray[FT2D_00][REAL_PART] = 4;		complexTensorArray[FT2D_00][IMAG_PART] = 0;
		complexTensorArray[FT2D_01][REAL_PART] = 4;		complexTensorArray[FT2D_01][IMAG_PART] = 1;
		complexTensorArray[FT2D_10][REAL_PART] = 3;		complexTensorArray[FT2D_10][IMAG_PART] = 0.33;
		complexTensorArray[FT2D_11][REAL_PART] = 5;		complexTensorArray[FT2D_11][IMAG_PART] = 100;
		
		Journal_PrintComplexTensorArray(stream, complexTensorArray, 2);
		
		Journal_Printf( stream, "3-D\n");
		complexTensorArray[FT3D_00][REAL_PART] = 1;		complexTensorArray[FT3D_00][IMAG_PART] = 0.5;
		complexTensorArray[FT3D_01][REAL_PART] = 2;		complexTensorArray[FT3D_01][IMAG_PART] = 0;
		complexTensorArray[FT3D_02][REAL_PART] = 3;		complexTensorArray[FT3D_02][IMAG_PART] = 0;
		complexTensorArray[FT3D_10][REAL_PART] = 4;		complexTensorArray[FT3D_10][IMAG_PART] = 0;
		complexTensorArray[FT3D_11][REAL_PART] = 5;		complexTensorArray[FT3D_11][IMAG_PART] = 1;
		complexTensorArray[FT3D_12][REAL_PART] = 6;		complexTensorArray[FT3D_12][IMAG_PART] = 2;
		complexTensorArray[FT3D_20][REAL_PART] = 7;		complexTensorArray[FT3D_20][IMAG_PART] = 0;
		complexTensorArray[FT3D_21][REAL_PART] = 8;		complexTensorArray[FT3D_21][IMAG_PART] = 0;
		complexTensorArray[FT3D_22][REAL_PART] = 9;		complexTensorArray[FT3D_22][IMAG_PART] = 30;	
		
		Journal_PrintComplexTensorArray(stream, complexTensorArray, 3);		

		Journal_Printf( stream, "\n/*******************    Test  11  ************************/\n");
		Journal_Printf( stream, "Test print ComplexMatrix function\n\n");		
		Journal_Printf( stream, "2-D\n");
		complexMatrix[0][0][REAL_PART] = 1;		complexMatrix[0][0][IMAG_PART] = 0;
		complexMatrix[0][1][REAL_PART] = 2;		complexMatrix[0][1][IMAG_PART] = 1;
		complexMatrix[0][2][REAL_PART] = 4;		complexMatrix[0][2][IMAG_PART] = 1;
		
		complexMatrix[1][0][REAL_PART] = 3;		complexMatrix[1][0][IMAG_PART] = 0.33;
		complexMatrix[1][1][REAL_PART] = 5;		complexMatrix[1][1][IMAG_PART] = 100;
		complexMatrix[1][2][REAL_PART] = 5;		complexMatrix[1][2][IMAG_PART] = 10.5;
		
		complexMatrix[2][0][REAL_PART] = 30;	complexMatrix[2][0][IMAG_PART] = 0.33;
		complexMatrix[2][1][REAL_PART] = 0.5;	complexMatrix[2][1][IMAG_PART] = 100;
		complexMatrix[2][2][REAL_PART] = 5.5;	complexMatrix[2][2][IMAG_PART] = 10.5;		
		
		Journal_PrintComplexMatrix(stream, complexMatrix, 2);
		
		Journal_Printf( stream, "3-D\n");
		Journal_PrintComplexMatrix(stream, complexMatrix, 3);

		Journal_Printf( stream, "\n/*******************    Test  12  ************************/\n");
		Journal_Printf( stream, "Test TensorArray to ComplexTensorArray conversion function\n\n");
		tensorArray[0] = 1 ; 	tensorArray[3] = 5 ;	tensorArray[6] = 9 ; 	
		tensorArray[1] = 3 ;	tensorArray[4] = 7 ;	tensorArray[7] = 11 ;	
		tensorArray[2] = 4 ;	tensorArray[5] = 8 ;	tensorArray[8] = 12 ;	
		
		Journal_Printf(stream, "2-D conversion\n");
		Journal_PrintTensorArray(stream, tensorArray, 2);
		TensorArray_ToComplexTensorArray(tensorArray, complexTensorArray, 2);
		Journal_PrintComplexTensorArray(stream, complexTensorArray, 2);		

		Journal_Printf(stream, "3-D conversion\n");
		Journal_PrintTensorArray(stream, tensorArray, 3);
		TensorArray_ToComplexTensorArray(tensorArray, complexTensorArray, 3);
		Journal_PrintComplexTensorArray(stream, complexTensorArray, 3);

		Journal_Printf( stream, "\n/*******************    Test  13  ************************/\n");
		Journal_Printf( stream, "Test ComplexTensorArray to ComplexMatrix conversion function\n\n");
		complexTensorArray[FT3D_00][REAL_PART] = 1;		complexTensorArray[FT3D_00][IMAG_PART] = 0.5;
		complexTensorArray[FT3D_01][REAL_PART] = 2;		complexTensorArray[FT3D_01][IMAG_PART] = 0;
		complexTensorArray[FT3D_02][REAL_PART] = 3;		complexTensorArray[FT3D_02][IMAG_PART] = 0;
		complexTensorArray[FT3D_10][REAL_PART] = 4;		complexTensorArray[FT3D_10][IMAG_PART] = 0;
		complexTensorArray[FT3D_11][REAL_PART] = 5;		complexTensorArray[FT3D_11][IMAG_PART] = 1;
		complexTensorArray[FT3D_12][REAL_PART] = 6;		complexTensorArray[FT3D_12][IMAG_PART] = 2;
		complexTensorArray[FT3D_20][REAL_PART] = 7;		complexTensorArray[FT3D_20][IMAG_PART] = 0;
		complexTensorArray[FT3D_21][REAL_PART] = 8;		complexTensorArray[FT3D_21][IMAG_PART] = 0;
		complexTensorArray[FT3D_22][REAL_PART] = 9;		complexTensorArray[FT3D_22][IMAG_PART] = 30;	
		
		Journal_Printf(stream, "2-D conversion\n");
		Journal_PrintComplexTensorArray(stream, complexTensorArray, 2);
		ComplexTensorArray_ToComplexMatrix(complexTensorArray, 2, complexMatrix ) ;
		Journal_PrintComplexMatrix(stream, complexMatrix, 2);		

		Journal_Printf(stream, "3-D conversion\n");
		Journal_PrintComplexTensorArray(stream, complexTensorArray, 3);
		ComplexTensorArray_ToComplexMatrix(complexTensorArray, 3, complexMatrix ) ;
		Journal_PrintComplexMatrix(stream, complexMatrix, 3);	
		
		Memory_Free(complexMatrix);

		Journal_Printf( stream, "\n/*******************    Test  14  ************************/\n");
		Journal_Printf( stream, "Test ComplexTensorArray to TensorArray conversion function\n\n");
		complexTensorArray[ 0][REAL_PART]  = 1;		complexTensorArray[ 0][IMAG_PART] = 0;	
		complexTensorArray[ 1][REAL_PART]  = 2;		complexTensorArray[ 1][IMAG_PART] = 0;	
		complexTensorArray[ 2][REAL_PART]  = 3;		complexTensorArray[ 2][IMAG_PART] = 0;	
		complexTensorArray[ 3][REAL_PART]  = 4;		complexTensorArray[ 3][IMAG_PART] = 0;	
		complexTensorArray[ 4][REAL_PART]  = 5;		complexTensorArray[ 4][IMAG_PART] = 0;	
		complexTensorArray[ 5][REAL_PART]  = 6;		complexTensorArray[ 5][IMAG_PART] = 0;	
		complexTensorArray[ 6][REAL_PART]  = 7;		complexTensorArray[ 6][IMAG_PART] = 0;	
		complexTensorArray[ 7][REAL_PART]  = 88;	complexTensorArray[ 7][IMAG_PART] = 0;	
		complexTensorArray[ 8][REAL_PART]  = 9.5;	complexTensorArray[ 8][IMAG_PART] = 0;	
		
		Journal_Printf(stream, "2-D conversion\n");
		Journal_PrintComplexTensorArray(stream, complexTensorArray, 2);		
		ComplexTensorArray_ToTensorArray(complexTensorArray, tensorArray, 2);
		Journal_PrintTensorArray(stream, tensorArray, 2);

		Journal_Printf(stream, "3-D conversion\n");
		Journal_PrintComplexTensorArray(stream, complexTensorArray, 3);		
		ComplexTensorArray_ToTensorArray(complexTensorArray, tensorArray, 3);
		Journal_PrintTensorArray(stream, tensorArray, 3);
		

		/*Note: This has to be the last test! As it will have an error and cleanly
		exit the test program.*/
		Journal_Printf(stream, "Failing conversion\n");
		complexTensorArray[0][IMAG_PART] = 1;		
		Journal_PrintComplexTensorArray(stream, complexTensorArray, 3);		
		ComplexTensorArray_ToTensorArray(complexTensorArray, tensorArray, 3);
		
		
		Memory_Free( tensor );
	}
	
	DiscretisationGeometry_Finalise();
	
	Base_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0;
}

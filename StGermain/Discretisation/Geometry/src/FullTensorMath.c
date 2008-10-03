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
** $Id: FullTensorMath.c  $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "units.h"
#include "types.h"
#include "TensorMath.h"
#include "VectorMath.h"
#include "FullTensorMath.h"
#include "ComplexVectorMath.h"
#include "ComplexMath.h"
#include "stg_lapack.h"
#include "petscksp.h"


#include <math.h>
#include <string.h>

#define STG_TENSOR_ERROR 1.0e-05;




/** This function converts TensorArray's to ComplexTensorArray's */
void TensorArray_ToComplexTensorArray(TensorArray tensorArray, ComplexTensorArray complexTensorArray, Dimension_Index dim) {
	Dimension_Index index_I;

	if (dim !=2 ) {
		if (dim != 3) {
			Journal_Firewall( False, Journal_Register( Error_Type, "FullTensorMath" ),
				"In func '%s' don't understand dim = %u\n", __func__, dim );
		}
	}	
			
	for (index_I = 0; index_I < dim * dim; index_I++) {
		complexTensorArray[index_I][REAL_PART] = tensorArray[index_I];
		complexTensorArray[index_I][IMAG_PART] = 0.0;
	}
	return;			
}
/** This function converts ComplexTensorArrays back into TensorArrays.
If there are any non-zero entries for complex components,
this function will JournalFirewall and exit */
void ComplexTensorArray_ToTensorArray(ComplexTensorArray complexTensorArray, TensorArray tensorArray, Dimension_Index dim) {
	Dimension_Index index_I;
	Stream* error = Journal_Register( ErrorStream_Type, "FullTensorMath" );

	if (dim !=2 ) {
		if (dim != 3) {
			Journal_Firewall( False, Journal_Register( Error_Type, "FullTensorMath" ),
				"In func '%s' don't understand dim = %u\n", __func__, dim );
		}
	}
		
	for (index_I = 0; index_I < dim * dim; index_I++) {
		if (complexTensorArray[index_I][IMAG_PART] == 0) {
			
			tensorArray[index_I] = complexTensorArray[index_I][REAL_PART];
		}
	
		else {
			Journal_Printf(error, "Cannot convert to real matrix:\n");			
			Journal_Printf(error, "Indicee %d in complexTensorArray is complex value.\n", index_I);
			Journal_PrintComplexTensorArray(error, complexTensorArray, dim);
			Journal_Firewall( False, error, "In func '%s'. Cannot convert ComplexTensor to Real Tensor \n", __func__ );
		}
	}
	
	return;		
}
/** This function converts ComplexTensorArrays to Complex Square Matrices */
void ComplexTensorArray_ToComplexMatrix(ComplexTensorArray complexTensor, Dimension_Index dim, Cmplx** complexMatrix ) {
	if (dim == 2) {
		Cmplx_Copy(complexTensor[FT2D_00], complexMatrix[0][0]); 			
		Cmplx_Copy(complexTensor[FT2D_01], complexMatrix[0][1]);	
		Cmplx_Copy(complexTensor[FT2D_10], complexMatrix[1][0]);		
		Cmplx_Copy(complexTensor[FT2D_11], complexMatrix[1][1]);	
	}
	else if (dim == 3) {
		Cmplx_Copy(complexTensor[FT3D_00], complexMatrix[0][0]);	
		Cmplx_Copy(complexTensor[FT3D_01], complexMatrix[0][1]);	
		Cmplx_Copy(complexTensor[FT3D_02], complexMatrix[0][2]);
		
		Cmplx_Copy(complexTensor[FT3D_10], complexMatrix[1][0]);		
		Cmplx_Copy(complexTensor[FT3D_11], complexMatrix[1][1]);	
		Cmplx_Copy(complexTensor[FT3D_12], complexMatrix[1][2]);
		
		Cmplx_Copy(complexTensor[FT3D_20], complexMatrix[2][0]);	
		Cmplx_Copy(complexTensor[FT3D_21], complexMatrix[2][1]);			
		Cmplx_Copy(complexTensor[FT3D_22], complexMatrix[2][2]);	
	}
	else {
		Journal_Firewall( False, Journal_Register( Error_Type, "FullTensorMath" ),
				"In func '%s' don't understand dim = %u\n", __func__, dim );
	}
}

/** This function calculates only the eigenvalues of a given TensorArray */
void TensorArray_CalcAllEigenvalues( TensorArray tensor, Dimension_Index dim, ComplexEigenvector* eigenvectorList ) {
	/* False flag means Eigenvectors are not written to eigenvectorList */
	TensorArray_CalcAllEigenFunctions(tensor, dim, False, eigenvectorList);
	
}

/** This function calculates only the eigenvalues of a given 2D TensorArray */
void TensorArray_CalcAllEigenvalues2D( TensorArray tensor, ComplexEigenvector* eigenvectorList ) {
	/* False flag means Eigenvectors are not written to eigenvectorList */
	TensorArray_CalcAllEigenFunctions(tensor, 2, False, eigenvectorList);
	
}

/** This function calculates only the eigenvalues of a given 3D TensorArray */
void TensorArray_CalcAllEigenvalues3D( TensorArray tensor, ComplexEigenvector* eigenvectorList ) {
	/* False flag means Eigenvectors are not written to eigenvectorList */
	TensorArray_CalcAllEigenFunctions(tensor, 3, False, eigenvectorList);
	
}

/** This function is a wrapper to calculate all eigenvalues and vectors for 2 or 3D TensorArray's */
void TensorArray_CalcAllEigenvectors(TensorArray tensor, Dimension_Index dim, ComplexEigenvector* eigenvectorList){
	/* True flag means eigenvalues and vectors are calculated */
	TensorArray_CalcAllEigenFunctions(tensor, dim, True, eigenvectorList);

	ComplexEigenvectorList_Sort( eigenvectorList, dim );
}

/** This function is a wrapper to calculate all eigenvalues and vectors for 2D TensorArray's */
void TensorArray_CalcAllEigenvectors2D( TensorArray tensor, ComplexEigenvector* eigenvectorList ) {
	
	/* True flag means eigenvalues and vectors are calculated */

	TensorArray_CalcAllEigenFunctions(tensor, 2, True, eigenvectorList);

	ComplexEigenvectorList_Sort( eigenvectorList, 2 );
	

}
/** This function is a wrapper to calculate all eigenvalues and vectors for 3D TensorArray's */
void TensorArray_CalcAllEigenvectors3D( TensorArray tensor, ComplexEigenvector* eigenvectorList ) {
	
	/* True flag means eigenvalues and vectors are calculated */

	TensorArray_CalcAllEigenFunctions(tensor, 3, True, eigenvectorList);

	ComplexEigenvectorList_Sort( eigenvectorList, 3 );
	

}

/** This function will call the blas-lapack library and calculate the eigenvalues and eigenvectors
For a given tensorArray and return the answers in a ComplexEigenvector structure.*/
void TensorArray_CalcAllEigenFunctions(TensorArray tensor, Dimension_Index dim, Bool eigenFlag, ComplexEigenvector* eigenvectorList) {
/**This function will call the blas-lapack library and calculate the eigenvalues and eigenvectors */
	/* Define functions needed to pass to blaslapack library function */
	char jobVecLeft='V';
	char jobVecRight='N';
	
	double* arrayA;
	int	leadDimVL, leadDimVR, dimWorkSpace, INFO;
	double errorValue;
    double* workSpace;
    double* outputReal;
    double* outputImag;
    double* leftEigenVec;
    double* rightEigenVec;
	
	int row_I, col_I; 
	//char* 	errorStringValues;
	Stream* errorStream = Journal_Register( ErrorStream_Type, "FullTensorMath" );
	
	/* Set size of workspace to pass to function */
	dimWorkSpace = 10*dim;

	/* define array size */
	arrayA = Memory_Alloc_Array( double, dim * dim, "ArrayA" );				

	/* define output eigenvalue matrices */
	outputReal = Memory_Alloc_Array( double, dim, "OutputReal" );				
	outputImag = Memory_Alloc_Array( double, dim, "OutputImag" );
	for (row_I = 0; row_I < dim; row_I++) {
		outputReal[row_I] = 0;
		outputImag[row_I] = 0;
	}
	/* Define workspace */
	workSpace = Memory_Alloc_Array( double, dimWorkSpace, "DimWorkSpace" );
	
	/* Transpose array so that it is in Fortran-style indexing */
	for( row_I = 0 ; row_I < dim ; row_I++ ) {
		 for( col_I = 0 ; col_I < dim ; col_I++ ) {
			arrayA[ ( row_I * dim ) + col_I ] = tensor[TensorArray_TensorMap(row_I, col_I, dim)];
		 }
	}
	 /* Turn off eigenvector calculations if eigenvector flag is not set */
	if (eigenFlag == False) {
		 jobVecLeft = 'N';
	}
	/* Set sizes for eigenvectors */
	if (jobVecLeft=='V') {
		/* times size by 2 to account for complex eigenvectors */
		leadDimVL = 2*dim;
	}
	else {
		leadDimVL = 1;
	}
	/* Set sizes for alternate eigenvectors
	This is currently always turned off since calculating right eigenvectors
	as well is redundant */
	if (jobVecRight=='V') {
		/* times 2 to account for complex eigenvectors */
		leadDimVR = 2*dim;
	}
	else {
		leadDimVR = 1;
	}
	
	/* set size of eigenvector arrays */
	leftEigenVec = Memory_Alloc_Array( double, leadDimVL * dim, "LeftEigenVec" );				
	rightEigenVec = Memory_Alloc_Array( double, leadDimVR * dim, "RightEigenVec" );
	for (row_I = 0; row_I < leadDimVL * dim; row_I++) {
		leftEigenVec[row_I] = 0;
	}
	for (row_I = 0; row_I < leadDimVR * dim; row_I++) {
		rightEigenVec[row_I] = 0;
	}
	
	/* Definitions of lapack call inputs (from dgeev man page):

		JOBVL   (input) CHARACTER*1
				  = 'N': left eigenvectors of A are not computed;
				  = 'V': left eigenvectors of A are computed.
		JOBVR   (input) CHARACTER*1
				 = 'N': right eigenvectors of A are not computed;
				 = 'V': right eigenvectors of A are computed
		N       (input) INTEGER
				 The order of the matrix A. N >= 0.
		A       (input/output) DOUBLE PRECISION array, dimension (LDA,N)
				 On entry, the N-by-N matrix A.
				 On exit, A has been overwritten.
		LDA     (input) INTEGER
				 The leading dimension of the array A.  LDA >= max(1,N).
		WR      (output) DOUBLE PRECISION array, dimension (N)
		WI      (output) DOUBLE PRECISION array, dimension (N)
				 WR and WI contain the real and imaginary parts,
				 respectively, of the computed eigenvalues.  Complex
				 conjugate pairs of eigenvalues appear consecutively
				 with the eigenvalue having the positive imaginary part
				 first.
		VL      (output) DOUBLE PRECISION array, dimension (LDVL,N)
				 If JOBVL = 'V', the left eigenvectors u(j) are stored one
				 after another in the columns of VL, in the same order
				 as their eigenvalues.
				 If JOBVL = 'N', VL is not referenced.
				 If the j-th eigenvalue is real, then u(j) = VL(:,j),
				 the j-th column of VL.
				 If the j-th and (j+1)-st eigenvalues form a complex
				 conjugate pair, then u(j) = VL(:,j) + i*VL(:,j+1) and
				 u(j+1) = VL(:,j) - i*VL(:,j+1).
		LDVL    (input) INTEGER
				 The leading dimension of the array VL.  LDVL >= 1; if
				 JOBVL = 'V', LDVL >= N.
		VR      (output) DOUBLE PRECISION array, dimension (LDVR,N)
				 If JOBVR = 'V', the right eigenvectors v(j) are stored one
				 after another in the columns of VR, in the same order
				 as their eigenvalues.
				 If JOBVR = 'N', VR is not referenced.
				 If the j-th eigenvalue is real, then v(j) = VR(:,j),
				 the j-th column of VR.
				 If the j-th and (j+1)-st eigenvalues form a complex
				 conjugate pair, then v(j) = VR(:,j) + i*VR(:,j+1) and
				 v(j+1) = VR(:,j) - i*VR(:,j+1).
		LDVR    (input) INTEGER
				 The leading dimension of the array VR.  LDVR >= 1; if
				 JOBVR = 'V', LDVR >= N.
		WORK    (workspace/output) DOUBLE PRECISION array, dimension (LWORK)
				 On exit, if INFO = 0, WORK(1) returns the optimal LWORK.
		
		LWORK   (input) INTEGER
				 The dimension of the array WORK.  LWORK >= max(1,3*N), and
				 if JOBVL = 'V' or JOBVR = 'V', LWORK >= 4*N.  For good
				 performance, LWORK must generally be larger.
				 If LWORK = -1, a workspace query is assumed.  The optimal
				 size for the WORK array is calculated and stored in WORK(1),
				 and no other work except argument checking is performed.
		INFO    (output) INTEGER
				 = 0:  successful exit
				 < 0:  if INFO = -i, the i-th argument had an illegal value.
				 > 0:  if INFO = i, the QR algorithm failed to compute all the
					   eigenvalues, and no eigenvectors have been computed;
					   elements i+1:N of WR and WI contain eigenvalues which
					   have converged.	 
	*/	 


	/** Passes into blaslapack function dgeev:
		 From Man page:
		 	1.  JOBVL			2.	JOBVR 			3.	N 
			4.	A 				5.	LDA 			6.	WR 
			7.	WI	 			8. 	VL	 			9. 	LDVL 
			10.	VR 				11.	LDVR 			12.	WORK 
			13.	LWORK 			14. INFO 
		 
		 In this code:
		 	1.  &jobVecLeft		2.  &jobVecRight 	3. &dimOrderN 
		 	4.  arrayA 			5.  &dim 	 		6. outputReal
			7.  outputImag		8.  leftEigenVec 	9. &dimOrderN
			10. rightEigenVec	11. &dimOrderN		12. workSpace
			13. &dimWorkSpace	14. &INFO		 
		 */
		 
	/** Calls blas-lapack function, dgeev through stg_lapack header file substitution
	to take account of different Fortran compilers	*/	 
	stg_dgeev( &jobVecLeft, &jobVecRight, &dim, arrayA, &dim, 
	 		outputReal, outputImag, leftEigenVec, &leadDimVL, 
	 		rightEigenVec, &leadDimVR, workSpace, &dimWorkSpace, &INFO );


	/* Check flag for succesful calculation */

	if (INFO < 0) {
		Journal_Printf( errorStream, "Error in %s, Blas-Lapack failed at %f-th argument for tensor:", 
		__func__, fabs(INFO));
		Journal_PrintTensorArray( errorStream, tensor, dim );
		Journal_Firewall(INFO , errorStream, "Error.\n" );

	}
	else if (INFO > 0) {
		Journal_Printf( errorStream, "Error in %s, Blas-Lapack function failed for tensor:", __func__ );
		Journal_PrintTensorArray( errorStream, tensor, dim );
		Journal_Firewall(INFO, errorStream, "Error.\n" );		
	}
	

/*Pass values back */
	errorValue = STG_TENSOR_ERROR;	
	/* Assign eigenvalues */
	for (col_I=0; col_I < dim; col_I++) {
		
		eigenvectorList[col_I].eigenvalue[REAL_PART] = outputReal[col_I];
		eigenvectorList[col_I].eigenvalue[IMAG_PART] = outputImag[col_I];
		if (fabs(eigenvectorList[col_I].eigenvalue[REAL_PART]) < errorValue) {
			eigenvectorList[col_I].eigenvalue[REAL_PART] = 0;
		}
		if (fabs(eigenvectorList[col_I].eigenvalue[IMAG_PART]) < errorValue) {
			eigenvectorList[col_I].eigenvalue[IMAG_PART] = 0;
		}	
	}
	
	/* If eigenvectors have been calculated */
	if (eigenFlag == True ) {
		int index_K;
		int numSign;
		
		/* Assign eigenvectors - see format for VL in comments for lapack pass above*/
		for (col_I=0; col_I < dim; col_I++) {
			
			if (outputImag[col_I] == 0.0) {
				for (row_I = 0; row_I < dim; row_I++) {
					eigenvectorList[col_I].vector[row_I][REAL_PART] = leftEigenVec[col_I * leadDimVL + row_I];
					eigenvectorList[col_I].vector[row_I][IMAG_PART] = 0;
				}
			}
			else {
				for (index_K = col_I; index_K <= col_I + 1; index_K++) {
					
					/* set sign of complex vector components */
					if (index_K == col_I) {
						numSign = -1;
					}
					else {
						numSign = 1;
					}	
					for (row_I = 0; row_I < dim; row_I++) {
					
						/* u(col, row) = v(row, col) 
											     \+- i * v(row, col + 1) */
						eigenvectorList[index_K].vector[row_I][REAL_PART] = 
							leftEigenVec[col_I * leadDimVL + row_I];
			
						eigenvectorList[index_K].vector[row_I][IMAG_PART] = 
							numSign * leftEigenVec[(col_I + 1) * leadDimVL + row_I];
					

					}
				}
				col_I++;
			}
		}
	}
	/* Round up values that are less than the error bar */
	for (row_I = 0; row_I < dim; row_I++) {
		for (col_I = 0; col_I <dim; col_I++) {
			
			if (fabs(eigenvectorList[row_I].vector[col_I][REAL_PART]) < errorValue) {
						eigenvectorList[row_I].vector[col_I][REAL_PART] = 0.0;
				}
			if (fabs(eigenvectorList[row_I].vector[col_I][IMAG_PART]) < errorValue) {
						eigenvectorList[row_I].vector[col_I][IMAG_PART] = 0.0;
				} 	
		}
	}
	
	
				
	/* Free memory and exit function */
	Memory_Free( arrayA );
	Memory_Free( outputReal );
	Memory_Free( outputImag );
	Memory_Free( leftEigenVec );
	Memory_Free( rightEigenVec );
	Memory_Free( workSpace );	
}
/** This function checks eigenvalues to find which of
the two has the biggest modulus*/
int _QsortComplexEigenvalue( const void* _a, const void* _b ) {
	ComplexEigenvector* a = (ComplexEigenvector*) _a;
	ComplexEigenvector* b = (ComplexEigenvector*) _b;
	double tmp_a, tmp_b;
	tmp_a = Cmplx_Modulus(a->eigenvalue);
	tmp_b = Cmplx_Modulus(b->eigenvalue);
	if ( tmp_a > tmp_b )
		return 1;
	else
		return -1;
}

/** Sorts the eigenvectors and eigenvalues according to the value of the modulus of the 
eigenvalue - from smallest to greatest */
void ComplexEigenvectorList_Sort( ComplexEigenvector* eigenvectorList, Index count ) {
	qsort( eigenvectorList, count, sizeof( ComplexEigenvector ), _QsortComplexEigenvalue );

}
/** This function prints a ComplexTensorArray 
At present, it only shows %7.5g as the format.
TODO: It would be nice in future to make this variable.*/
void Journal_PrintComplexTensorArray_Unnamed( Stream* stream, ComplexTensorArray tensor, Dimension_Index dim ) {
	Dimension_Index row_I, col_I;

	/* For efficency - Check if stream is enabled */
	if (!Stream_IsEnable(stream)) return;

	for ( row_I = 0 ; row_I < dim ; row_I++ ) {
		for ( col_I = 0 ; col_I < dim ; col_I++ ) {
			Journal_Printf( stream, "%7.5g + %7.5g i", 
				tensor[ MAP_TENSOR( row_I, col_I, dim ) ][REAL_PART], 
				tensor[ MAP_TENSOR( row_I, col_I, dim ) ][IMAG_PART]);
		}
		Journal_Printf( stream, "\n" );
	}
}

/** This function prints a Complex Square matrix */
void Journal_PrintComplexMatrix_Unnamed( Stream* stream, Cmplx** complexMatrix, Dimension_Index dim ) {
	Dimension_Index row_I, col_I;

	/* For efficency - Check if stream is enabled */
	if (!Stream_IsEnable(stream)) return;

	for ( row_I = 0 ; row_I < dim ; row_I++ ) {
		for ( col_I = 0 ; col_I < dim ; col_I++ ) {
			Journal_Printf( stream, "%7.5g + %7.5g i", 
				complexMatrix[row_I][col_I][REAL_PART], 
				complexMatrix[row_I][col_I][IMAG_PART]);
		}
		Journal_Printf( stream, "\n" );
	}
}

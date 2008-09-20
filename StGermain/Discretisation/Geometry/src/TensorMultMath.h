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
**	Alireza Asgari, Researcher, Deakin University.
**  Julian Giordani, Researcher, Monash Cluster Computing, Monash University
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
** Role:
**    Provides basic tensor and vector multiplication operations for 2D and 3D.
**
** Assumptions:
** Comments:
**
** $Id: TensorMultMath.h  $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Geometry_TensorMultMath_h__
#define __Discretisation_Geometry_TensorMultMath_h__

/* Added to enhance Tensor math for Solid Mechanics applications */

/* TODO These should be in TensorMath  */
void TensorArray_Identity(Dimension_Index dim, TensorArray tensorArray );
void SymmetricTensor_Identity(Dimension_Index dim, SymmetricTensor symmetricTensor) ;

/* Basic operations */
void TensorArray_Transpose(TensorArray tensorArray, Dimension_Index dim, TensorArray result);
void TensorArray_Add(	TensorArray tensorArrayA, TensorArray tensorArrayB, 
						Dimension_Index dim, TensorArray result);
void TensorArray_Subtract(	TensorArray tensorArrayA, TensorArray tensorArrayB, Dimension_Index dim,
							TensorArray result);

/* Multiplication Operations */

void TensorArray_MultiplyByTensorArray(	TensorArray tensorArrayA, TensorArray tensorArrayB, 
										Dimension_Index dim, TensorArray result);
void TensorArray_MultiplyByRightTranspose(TensorArray tensorArray, Dimension_Index dim, SymmetricTensor result);
void TensorArray_MultiplyByLeftTranspose(TensorArray tensorArray, Dimension_Index dim, SymmetricTensor result);
void TensorArray_MultiplyBySymmetricTensor(	TensorArray tensorArray, SymmetricTensor symmetricTensor,
											Dimension_Index dim, TensorArray result);
void TensorArray_MultiplyByLeftVector(	TensorArray tensorArray, double* vector, 
										Dimension_Index dim, double* result);
void TensorArray_MultiplyByRightVector(	TensorArray tensorArray, double* vector, 
										Dimension_Index dim, double* result); 

/* Other useful operations */
double TensorArray_CalcDeterminant(TensorArray tensorArray, Dimension_Index dim);
void TensorArray_CalcInverse( TensorArray tensorA, Dimension_Index dim, TensorArray result);
void TensorArray_CalcInverseWithDeterminant(TensorArray tensor, double determinant, Dimension_Index dim, TensorArray result);


/* Useful operations in Solid Mechanics */
double TensorArray_DoubleContraction(TensorArray tensorArrayA,TensorArray tensorArrayB, Dimension_Index dim);
double SymmetricTensor_DoubleContraction(SymmetricTensor tensorA, SymmetricTensor tensorB, Dimension_Index dim);

/* Non-square matrix operations */
void NonSquareMatrix_Transpose( double** originalMatrix, Dimension_Index rowDimOrig, 
	Dimension_Index colDimOrig, double** newMatrix );

void NonSquareMatrix_MultiplicationByNonSquareMatrix( double **AMatrix, int rowsInA, int colsInA,
					 double **BMatrix, int rowsInB, int colsInB, 
					 double** resultMatrix );	

void NonSquareMatrix_CumulativeMultiplicationByNonSquareMatrix( double **AMatrix, int rowDimA, int colDimA,
					 double **BMatrix, int rowDimB, int colDimB, 
					 double** resultMatrix );

void NonSquareMatrix_MatrixVectorMultiplication( double** AMatrix, int rowsInA, int colsInA,
		                               double* BVec, int rowsInB, double* resultVector );
void NonSquareMatrix_CumulativeMatrixVectorMultiplication( double** AMatrix, int rowsInA, int colsInA,
		                               double* BVec, int rowsInB, double* resultVector ); 						   
/** Print a named NonSquareMatrix */
#define Journal_PrintNonSquareMatrix(stream, matrix, rowDim, colDim) \
	do {	\
		Journal_Printf( stream, #matrix " - \n" ); \
		Journal_PrintNonSquareMatrix_Unnamed( stream, matrix, rowDim, colDim ); \
	} while(0) 
	
void Journal_PrintNonSquareMatrix_Unnamed( Stream* stream, double** NonSquareMatrix, 
	Dimension_Index rowDim, Dimension_Index colDim ) ;




#endif /* __Discretisation_Geometry_TensorMultMath_h__ */

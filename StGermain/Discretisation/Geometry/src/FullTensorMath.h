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
** Role:
**    Provides basic full tensor operations and conversions.
**
** Assumptions:
** Comments:
**
** $Id: FullTensorMath.h  $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Geometry_FullTensorMath_h__
#define __Discretisation_Geometry_FullTensorMath_h__

#include "ComplexMath.h"
#include "ComplexVectorMath.h"
#include "TensorMath.h"


/** Create complex eigenvalue and vector:
	This creates a Cmplx tuple to represent the vector
	And a Complex value to represent the eigenvalue.
	These can be referenced by:
	{eigenvectorName}.vector[{indexNumber}], or {eigenvectorName}.eigenvalue
	*/
typedef struct {
	XYZC    vector;
	Cmplx eigenvalue;
} ComplexEigenvector;

/** TensorArray - Tensor (t_{ij}) here is defined in 2D as
	 * t_{00} = tensor[0] t_{01} = tensor[1]
	 * t_{10} = tensor[2] t_{11} = tensor[3] 
	 *
	 * and in 3D as
	 * t_{00} = tensor[0] t_{01} = tensor[1] t_{02} = tensor[2]
	 * t_{10} = tensor[3] t_{11} = tensor[4] t_{12} = tensor[5]
	 * t_{20} = tensor[6] t_{21} = tensor[7] t_{22} = tensor[8]
	 *
	 * */

	/** SymmetricTensor - stores only unique components 
	 * in 2D
	 * tensor[0] = u_{00}
	 * tensor[1] = u_{11}
	 * tensor[2] = u_{12} = u_{21}
	 *
	 * in 3D
	 * tensor[0] = u_{00}
	 * tensor[1] = u_{11}
	 * tensor[2] = u_{22}
	 * tensor[3] = u_{01} = u_{10}
	 * tensor[4] = u_{02} = u_{20}
	 * tensor[5] = u_{12} = u_{21}
	 */


/* Define ComplexTensor conversion function */ 	
void TensorArray_ToComplexTensorArray(TensorArray tensorArray, ComplexTensorArray complexTensorArray, Dimension_Index dim);

void ComplexTensorArray_ToTensorArray(ComplexTensorArray complexTensorArray, TensorArray tensorArray, Dimension_Index dim);

void ComplexTensorArray_ToComplexMatrix(ComplexTensorArray complexTensor, Dimension_Index dim, Cmplx** complexMatrix ) ;


/* Define all Eigenvalue and Eigenvector functions for TensorArray's */

void TensorArray_CalcAllEigenvalues( TensorArray tensor, Dimension_Index dim, ComplexEigenvector* eigenvectorList ) ;

void TensorArray_CalcAllEigenvalues2D( TensorArray tensor, ComplexEigenvector* eigenvectorList ) ;
void TensorArray_CalcAllEigenvalues3D( TensorArray tensor, ComplexEigenvector* eigenvectorList ) ;


void TensorArray_CalcAllEigenvectors( TensorArray tensor, Dimension_Index dim, ComplexEigenvector* eigenvectorList );

void TensorArray_CalcAllEigenvectors2D( TensorArray tensor, ComplexEigenvector* eigenvectorList ) ;
void TensorArray_CalcAllEigenvectors3D( TensorArray tensor, ComplexEigenvector* eigenvectorList ) ;

void TensorArray_CalcAllEigenFunctions(TensorArray tensor, Dimension_Index dim, Bool EigenFlag, ComplexEigenvector* eigenvectorList);

/* Sorts the eigenvectors according to the value of the eigenvalue - from smallest to greatest */

void ComplexEigenvectorList_Sort( ComplexEigenvector* eigenvectorList, Index count );


/* Define print statements */

/** Print a named ComplexTensorArray */
#define Journal_PrintComplexTensorArray(stream, tensor, dim) \
	do {	\
		Journal_Printf( stream, #tensor " - \n" ); \
		Journal_PrintComplexTensorArray_Unnamed( stream, tensor, dim ); \
	} while(0) 

void Journal_PrintComplexTensorArray_Unnamed( Stream* stream, ComplexTensorArray tensor, Dimension_Index dim ); 

	/** Print a named ComplexMatrix */
#define Journal_PrintComplexMatrix(stream, matrix, dim) \
	do {	\
		Journal_Printf( stream, #matrix " - \n" ); \
		Journal_PrintComplexMatrix_Unnamed( stream, matrix, dim ); \
	} while(0) 
	
void Journal_PrintComplexMatrix_Unnamed( Stream* stream, Cmplx** complexMatrix, Dimension_Index dim ) ;

	
	
#endif /* __Discretisation_Geometry_FullTensorMath_h__ */

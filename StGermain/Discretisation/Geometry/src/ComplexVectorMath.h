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
**    Provides basic complex vector operations.
**
** Assumptions:
**    - CoordC is an array of 3 Cmplx. 
**
** Comments:
**    In any operation that involves two or more input vectors, those vectors 
**    may be one and the same; it may be assumed that such an occurence will be
**    handled.
**
** $Id: ComplexVectorMath.h 3677 2006-07-14 11:03:24Z KathleenHumble $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Geometry_ComplexVectorMath_h__
#define __Discretisation_Geometry_ComplexVectorMath_h__
		
#include "ComplexMath.h"

typedef Cmplx ComplexVector[3];

void ComplexVector_Set(CoordC set, CoordC dest);
void ComplexVector_SetScalar( Cmplx a, Cmplx b, Cmplx c, CoordC dest );
void ComplexVector_Add( CoordC a, CoordC b, CoordC dest );
void ComplexVector_Sub( CoordC a, CoordC b, CoordC dest );		
void ComplexVector_Dot(CoordC a, CoordC b, Cmplx dest );
void ComplexVector_Mult(CoordC a, Cmplx s, CoordC dest );
void ComplexVector_MultReal(CoordC a, double valueReal, CoordC dest );
double ComplexVector_Mag(CoordC a );								
void ComplexVector_Proj(CoordC a, CoordC b, CoordC  dest );					
	

void ComplexVector_Cross( CoordC a, CoordC b, CoordC dest );
void ComplexVector_Div( CoordC a, Cmplx s, CoordC dest );
void ComplexVector_Norm( CoordC a, CoordC dest);
void ComplexVector_Swizzle( CoordC src, unsigned char iInd, 
		unsigned char jInd, unsigned char kInd, CoordC dst );


void StGermain_RotateComplexVector(Cmplx* vector, double phi, double theta, 
			double eta, Cmplx* rotatedVector);
void StGermain_RotateCoordinateAxisComplex( Cmplx* vector, Index axis, double theta, Cmplx* rotatedVector ) ;
void StGermain_ComplexVectorSubtraction(Cmplx* destination, Cmplx* vector1, Cmplx* vector2, Index dim) ;
void StGermain_ComplexVectorAddition(Cmplx* destination, Cmplx* vector1, Cmplx* vector2, Index dim) ;

double StGermain_ComplexVectorMagnitude(Cmplx* vector, Index dim) ;
void StGermain_ComplexVectorDotProduct(Cmplx* vector1, Cmplx* vector2, Dimension_Index dim, Cmplx dotProduct) ;
void StGermain_ComplexVectorCrossProduct(Cmplx* destination, Cmplx* vector1, Cmplx* vector2) ;
void StGermain_ComplexVectorCrossProductMagnitude( Cmplx* vector1, Cmplx* vector2, Dimension_Index dim, Cmplx dest ) ;
void StGermain_ComplexScalarTripleProduct( Cmplx* vectorA, Cmplx* vectorB, Cmplx* vectorC, Cmplx dest );

void StGermain_ComplexVectorNormalise(Cmplx* vector, Index dim ) ;

void StGermain_ComplexTriangleCentroid( Cmplx* centroid, Cmplx* pos0, Cmplx* pos1, Cmplx* pos2, Index dim) ;

void ComplexVector_ToVector(CoordC complexVector, Dimension_Index dim, Coord vector) ;
void Vector_ToComplexVector(Coord vector, Dimension_Index dim, CoordC complexVector) ;


void StGermain_PrintComplexVector( Stream* stream, Cmplx* vector, Index dim ) ;

/** Prints a named ComplexVector */
#define StGermain_PrintNamedComplexVector(stream, vector, dim) \
		do {	\
			Journal_Printf( stream, #vector " - " ); \
			StGermain_PrintComplexVector( stream, vector, dim );	\
		} while(0)
		

		
#endif /* __Discretisation_Geometry_ComplexVectorMath_h__ */

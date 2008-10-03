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
** $Id: ComplexVectorMath.c 3677 2006-07-14 11:03:24Z KathleenHumble $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


#include <mpi.h>
#include "Base/Base.h"


#include "units.h"
#include "types.h"
#include "ComplexMath.h"
#include "VectorMath.h"
#include "ComplexVectorMath.h"
#include "TrigMath.h"

#include <math.h>
#include <assert.h>

/**(Assumes <=3D),  Set one ComplexVector to another */
void ComplexVector_Set(CoordC set, CoordC dest) {
	dest[0][REAL_PART] = set[0][REAL_PART];
	dest[0][IMAG_PART] = set[0][IMAG_PART];
	dest[1][REAL_PART] = set[1][REAL_PART];
	dest[1][IMAG_PART] = set[1][IMAG_PART];
	dest[2][REAL_PART] = set[2][REAL_PART];
	dest[2][IMAG_PART] = set[2][IMAG_PART];
}

/**(Assumes 3D),  Set complex numbers to entries of function.
One drawback, scalars must already be defined as complex numbers */
void ComplexVector_SetScalar( Cmplx a, Cmplx b, Cmplx c, CoordC dest ) {
	dest[0][REAL_PART] = a[REAL_PART];
	dest[0][IMAG_PART] = a[IMAG_PART];
	dest[1][REAL_PART] = b[REAL_PART];
	dest[1][IMAG_PART] = b[IMAG_PART];
	dest[2][REAL_PART] = c[REAL_PART];
	dest[2][IMAG_PART] = c[IMAG_PART];

}


/** (Assumes 3D), Add two complex vectors:  dest = a + b */
void ComplexVector_Add( CoordC a, CoordC b, CoordC dest ) {
	Cmplx_Add( a[0], b[0], dest[0] );
	Cmplx_Add( a[1], b[1], dest[1] );
	Cmplx_Add( a[2], b[2], dest[2] );
}	
		
/** (Assumes 3D), Subtract two complex vectors: dest = a - b */
void ComplexVector_Sub( CoordC a, CoordC b, CoordC dest ) {
	Cmplx_Subtract( a[0], b[0], dest[0] );
	Cmplx_Subtract( a[1], b[1], dest[1] );
	Cmplx_Subtract( a[2], b[2], dest[2] );
}
			
/** (Assumes 3D), Returns the dot product of two complex vectors, a and b */
void ComplexVector_Dot(CoordC a, CoordC b, Cmplx destSum ) {
	CoordC dest;

	Cmplx_Multiply(a[0], b[0], dest[0]);
	Cmplx_Multiply(a[1], b[1], dest[1]);
	Cmplx_Multiply(a[2], b[2], dest[2]);
	Cmplx_Add(dest[0], dest[1], destSum);
	Cmplx_Add(destSum, dest[2], destSum);
	
}	
	
/** (Assumes 3D), Multiply two complex vectors: dest = a * s */
void ComplexVector_Mult(CoordC a, Cmplx s, CoordC dest ) {
	Cmplx_Multiply(a[0], s, dest[0]);
	Cmplx_Multiply(a[1], s, dest[1]);
	Cmplx_Multiply(a[2], s, dest[2]);
}

/** (Assumes 3D), Multiply complex vector by real number */ 
void ComplexVector_MultReal(CoordC a, double valueReal, CoordC dest ) {
	Cmplx value;
	value[REAL_PART] = valueReal;
	value[IMAG_PART] = 0.0;
	Cmplx_Multiply(a[0], value, dest[0]);
	Cmplx_Multiply(a[1], value, dest[1]);
	Cmplx_Multiply(a[2], value, dest[2]);
}

/** (Assumes 3D), Returns the magnitude of complex vector a */
double ComplexVector_Mag(CoordC a ) {
	double a_0, a_1, a_2;
	a_0 = Cmplx_Modulus(a[0]);
	a_1 = Cmplx_Modulus(a[1]);
	a_2 = Cmplx_Modulus(a[2]);
	return sqrt(a_0*a_0 + a_1*a_1 + a_2*a_2); 	
}

/** (Assumes 3D), complexVector projection of a onto b, store result in dest */
void ComplexVector_Proj(CoordC a, CoordC b, CoordC dest ) {
		/* Calculate norm of b */
		Cmplx tmp;
		ComplexVector_Norm( b, dest);
		/* Calculate proj of a onto b */
		ComplexVector_Dot( a, b, tmp );
		ComplexVector_Mult( dest, tmp, dest );
}
	
/** (Assumes 3D), Calculates cross product of two complex vectors */	
void ComplexVector_Cross( CoordC a, CoordC b, CoordC dest ) {

	Cmplx 	ans1, ans2;
	 
	Cmplx_Multiply(a[1], b[2], ans1);
	Cmplx_Multiply(a[2], b[1], ans2);
	Cmplx_Subtract(ans1, ans2, dest[0]);
	
	Cmplx_Multiply(a[2], b[0], ans1);
	Cmplx_Multiply(a[0], b[2], ans2);
	Cmplx_Subtract(ans1, ans2, dest[1]);
	
	Cmplx_Multiply(a[0], b[1], ans1);
	Cmplx_Multiply(a[1], b[0], ans2);
	Cmplx_Subtract(ans1, ans2, dest[2]);

}

/** (Assumes 3D), Divide complex vector by complex number */
void ComplexVector_Div( CoordC a, Cmplx s, CoordC dest )
{
	Cmplx	inv, one;
	one[REAL_PART] = 1.0;
	one[IMAG_PART] = 0.0;
	
	Cmplx_Division( one, s, inv );

	
	Cmplx_Multiply(a[0], inv, dest[0]);
	Cmplx_Multiply(a[1], inv, dest[1]);
	Cmplx_Multiply(a[2], inv, dest[2]);
}

/** (Assumes 3D), Normalises complex vector */
void ComplexVector_Norm(CoordC a, CoordC dest) {
	double invMag;
	
	 
	invMag = 1.0 / ComplexVector_Mag( a );
	ComplexVector_MultReal(a, invMag, dest);

}

/** (Assumes 3D), Swaps coords based on i,j,k input */
void ComplexVector_Swizzle( CoordC src, unsigned char iInd, 
		unsigned char jInd, unsigned char kInd, CoordC dst ) {
	CoordC dummy;
	assert( iInd < 3 && jInd < 3 && kInd < 3 );
	dummy[0][REAL_PART] = src[iInd][REAL_PART];
	dummy[0][IMAG_PART] = src[iInd][IMAG_PART];
	dummy[1][REAL_PART] = src[jInd][REAL_PART];
	dummy[1][IMAG_PART] = src[jInd][IMAG_PART];
	dummy[2][REAL_PART] = src[kInd][REAL_PART];
	dummy[2][IMAG_PART] = src[kInd][IMAG_PART];
		
	dst[0][REAL_PART] = dummy[0][REAL_PART];
	dst[0][IMAG_PART] = dummy[0][IMAG_PART];	
	dst[1][REAL_PART] = dummy[1][REAL_PART];
	dst[1][IMAG_PART] = dummy[1][IMAG_PART];
	dst[2][REAL_PART] = dummy[2][REAL_PART];
	dst[2][IMAG_PART] = dummy[2][IMAG_PART];
}


/** StGermain_ComplexRotateVector takes an argument 'vectorToRotate', and rotates it through 
three angles for the x, y and z coordinates.(\alpha, \beta, \gama) respectively I believe.
The angles should be reals, and in radians.
This function cannot use Rodrigues' Rotation Formula because that is only defined for reals.
See: 
http://mathworld.wolfram.com/EulerAngles.html
http://mathworld.wolfram.com/EulerParameters.html */
void StGermain_RotateComplexVector(Cmplx* vector, double alpha, double beta, 
			double gama, Cmplx* rotatedVector) {
	double rotationMatrix[3][3]; 	/* Indicies [Column][Row][Real or Imag] */
	//double e0, e1, e2, e3;
				
	Cmplx r_0, r_1, r_2, tmp ;

	rotationMatrix[0][0] =   cos(beta) * cos(gama);
	rotationMatrix[0][1] =   cos(beta) * sin(gama); 
	rotationMatrix[0][2] =  -sin(beta);
				
	rotationMatrix[1][0] = -cos(alpha) * sin(gama) + sin(alpha) * sin(beta) * cos(gama);
	rotationMatrix[1][1] =  cos(alpha) * cos(gama) + sin(alpha) * sin(beta) * sin(gama);
	rotationMatrix[1][2] =  sin(alpha) * cos(beta);
	
	rotationMatrix[2][0] =  sin(alpha) * sin(gama) + cos(alpha) * sin(beta) * cos(gama);
	rotationMatrix[2][1] = -sin(alpha) * cos(gama) + cos(alpha) * sin(beta) * sin(gama);
	rotationMatrix[2][2] = 	cos(alpha) * cos(beta);		
				

	// x direction
	Cmplx_RealMultiply(vector[0], rotationMatrix[0][0], r_0);
	Cmplx_RealMultiply(vector[1], rotationMatrix[0][1], r_1);
	Cmplx_RealMultiply(vector[2], rotationMatrix[0][2], r_2);

	Cmplx_Add(r_0, r_1, tmp);
	Cmplx_Add(tmp, r_2, rotatedVector[0]);

	
	// y direction
	Cmplx_RealMultiply(vector[0], rotationMatrix[1][0], r_0);
	Cmplx_RealMultiply(vector[1], rotationMatrix[1][1], r_1);
	Cmplx_RealMultiply(vector[2], rotationMatrix[1][2], r_2);

	Cmplx_Add(r_0, r_1, tmp);
	Cmplx_Add(tmp, r_2, rotatedVector[1]);
	
	// z direction
	Cmplx_RealMultiply(vector[0], rotationMatrix[2][0], r_0);
	Cmplx_RealMultiply(vector[1], rotationMatrix[2][1], r_1);
	Cmplx_RealMultiply(vector[2], rotationMatrix[2][2], r_2);

	Cmplx_Add(r_0, r_1, tmp);
	Cmplx_Add(tmp, r_2, rotatedVector[2]);
}		

/** StGermain_RotateCoordinateAxisComplex multiplies a vector with a Rotation Matrix to rotate it around a co-ordinate axis -
Is a simpler function than StGermain_RotateComplexVector for more specific cases where the vector is to be rotated around one of the axes of the co-ordinate system. The arguments are the same except the the 'axis' argument is of type 'Index' which could be either I_AXIS, J_AXIS or K_AXIS. Vectors have to be the size of 3 doubles.
See, Eric W. Weisstein. "Rotation Matrix." 
From MathWorld--A Wolfram Web Resource. http://mathworld.wolfram.com/RotationMatrix.htm */
void StGermain_RotateCoordinateAxisComplex( Cmplx* vector, 
			Index axis, double theta, Cmplx* rotatedVector ) {
	
	Cmplx r_1, r_2;
	/* Rotation around one axis will always leave the component on that axis alone */
	rotatedVector[axis][REAL_PART] = vector[axis][REAL_PART];
	rotatedVector[axis][IMAG_PART] = vector[axis][IMAG_PART];
	//printf("axis %d, %2.3f", axis, vector[axis][REAL_PART]); 				
	switch (axis) {
		case K_AXIS: /* Rotate around Z axis */
			Cmplx_RealMultiply(vector[0], cos(theta), r_1);
			Cmplx_RealMultiply(vector[1], sin(theta), r_2);
			Cmplx_Add(r_1, r_2, rotatedVector[0]);
		
			Cmplx_RealMultiply(vector[0], sin(theta), r_1);
			Cmplx_RealMultiply(vector[1], cos(theta), r_2);
			Cmplx_Subtract(r_2, r_1, rotatedVector[1]);
			return;
		case I_AXIS:  /* Rotate around X axis */
			Cmplx_RealMultiply(vector[1], cos(theta), r_1);
			Cmplx_RealMultiply(vector[2], sin(theta), r_2);
			Cmplx_Add(r_1, r_2, rotatedVector[1]);
		
			Cmplx_RealMultiply(vector[1], sin(theta), r_1);
			Cmplx_RealMultiply(vector[2], cos(theta), r_2);
			Cmplx_Subtract(r_2, r_1, rotatedVector[2] );
			return;
		case J_AXIS: /* Rotate around Y axis */
			Cmplx_RealMultiply(vector[0], cos(theta), r_1);
			Cmplx_RealMultiply(vector[2], sin(theta), r_2);
			Cmplx_Subtract(r_1, r_2, rotatedVector[0] );
		
			Cmplx_RealMultiply(vector[0], sin(theta), r_1);
			Cmplx_RealMultiply(vector[2], cos(theta), r_2);
			Cmplx_Add(r_1, r_2, rotatedVector[2]) ;
			return;
		default: {
			Stream* error = Journal_Register( ErrorStream_Type, "ComplexVectorMath" );
			Journal_Printf( error, "Impossible axis to rotate around in %s.", __func__);
			Journal_Firewall( False, Journal_Register( Error_Type, "ComplexVectorMath" ),
				"Error in '%s':Impossible axis to rotate around. \n", __func__);
		}
	}
}

/** Subtracts one complex vector from another - 
destination = vector1 - vector2
Destination vector may be the same as either of the other two input vectors 
Function is optimised for 1-3 dimensions but will work for any dimension */
void StGermain_ComplexVectorSubtraction(Cmplx* destination, Cmplx* vector1, Cmplx* vector2, Index dim) {

	switch (dim) {
		case 3:
			Cmplx_Subtract(vector1[2], vector2[2], destination[2]);			
			
		case 2:
			Cmplx_Subtract(vector1[1], vector2[1], destination[1]);
		case 1: 
			Cmplx_Subtract(vector1[0], vector2[0], destination[0]);
			return;
		default: {
			Index d;
			for ( d = 0 ; d < dim ; d++ ) 
				Cmplx_Subtract(vector1[d], vector2[d], destination[d]);
			return;
		}
	}	
}

/** Adds two vectors - 
destination = vector1 + vector2
Destination vector may be the same as either of the other two input vectors 
Function is optimised for 1-3 dimensions but will work for any dimension */
void StGermain_ComplexVectorAddition(Cmplx* destination, Cmplx* vector1, Cmplx* vector2, Index dim) {
	switch (dim) {
		case 3: 
			Cmplx_Add(vector1[2], vector2[2], destination[2]);	
		case 2:
			Cmplx_Add(vector1[1], vector2[1], destination[1]);	
		case 1: 
			Cmplx_Add(vector1[0], vector2[0], destination[0]);	
			return;
		default: {
			Index d;
			for ( d = 0 ; d < dim ; d++ ) 
				Cmplx_Add(vector1[d], vector2[d], destination[d]);	
				//printf("%f, %f", destination[d][REAL_PART], destination[d][IMAG_PART]);
			return;
		}
	}	
}

/** StGermain_ComplexVectorMagnitude calculates the magnitude of a vector
|v| = \sqrt{ v . v } 
This function uses function StGermain_ComplexVectorDotProduct to calculate v . v. 
Vector has to be of size dim doubles */
double StGermain_ComplexVectorMagnitude(Cmplx* vector, Index dim) {
	Cmplx dotProduct;

	StGermain_ComplexVectorDotProduct(vector, vector, dim, dotProduct);
	return sqrt(Cmplx_Modulus(dotProduct));
}

/** StGermain_ComplexVectorDotProduct calculates the complex valued dot product of two
complex vectors
*/
void StGermain_ComplexVectorDotProduct(Cmplx* vector1, Cmplx* vector2, Dimension_Index dim, Cmplx dotProduct) {
	Cmplx tmp;
	dotProduct[REAL_PART] = 0.0;
	dotProduct[IMAG_PART] = 0.0;

	switch (dim) {
		case 3: {
			Cmplx_Multiply(vector1[2], vector2[2], tmp);
			Cmplx_Add(dotProduct, tmp, dotProduct);
		}
		case 2: {
			Cmplx_Multiply(vector1[1], vector2[1], tmp);
			Cmplx_Add(dotProduct, tmp, dotProduct);
		}
		case 1: {
			Cmplx_Multiply(vector1[0], vector2[0], tmp);
			Cmplx_Add(dotProduct, tmp, dotProduct);
			break;
		}
		default: {
			Dimension_Index d;
			for ( d = 0 ; d < dim ; d++ ) {
				Cmplx_Multiply(vector1[d], vector2[d], tmp);
				Cmplx_Add(dotProduct, tmp, dotProduct);
			}
			break;
		}
	}
	
	
}

/** See Eric W. Weisstein. "Cross Product." 
From MathWorld--A Wolfram Web Resource. http://mathworld.wolfram.com/CrossProduct.html 
Tested against http://www.engplanet.com/redirect.html?3859 */
void StGermain_ComplexVectorCrossProduct(Cmplx* destination, Cmplx* vector1, Cmplx* vector2) {
	Cmplx c_1, c_2;
	//x direction
	Cmplx_Multiply(vector1[1], vector2[2], c_1);
	Cmplx_Multiply(vector1[2], vector2[1], c_2);
	Cmplx_Subtract(c_1, c_2, destination[0]);

	//y direction
	Cmplx_Multiply(vector1[2], vector2[0], c_1);
	Cmplx_Multiply(vector1[0], vector2[2], c_2);
	Cmplx_Subtract(c_1, c_2, destination[1]);
	//z direction
	Cmplx_Multiply(vector1[0], vector2[1], c_1);
	Cmplx_Multiply(vector1[1], vector2[0], c_2);
	Cmplx_Subtract(c_1, c_2, destination[2]);
}

/** StGermain_VectorCrossProductMagnitude - See Eric W. Weisstein. "Cross Product." 
From MathWorld--A Wolfram Web Resource. http://mathworld.wolfram.com/CrossProduct.html 
|a \times b| = |a||b|\sqrt{ 1 - (\hat a . \hat b)^2}
*/
void StGermain_ComplexVectorCrossProductMagnitude( Cmplx* vector1, Cmplx* vector2, Dimension_Index dim, Cmplx tmp ) {
	double mag1       = StGermain_ComplexVectorMagnitude( vector1, dim );
	double mag2       = StGermain_ComplexVectorMagnitude( vector2, dim );
	Cmplx dotProduct, dotSquared;
	StGermain_ComplexVectorDotProduct( vector1, vector2, dim, dotProduct );
	
	Cmplx_Multiply(dotProduct, dotProduct, dotSquared);
	Cmplx_RealMultiply(dotSquared, 1.0/(mag1 * mag1 * mag2 * mag2), tmp);
	Cmplx_RealMinusCmplx(tmp, 1.0, tmp); 
	Cmplx_RealPower(tmp, 0.5, tmp);
	Cmplx_RealMultiply(tmp, (mag1 * mag2), tmp);
	
}


/** StGermain_ComplexScalarTripleProduct - Calculates the scalar vector product of three vectors -
 * see Eric W. Weisstein. "Scalar Triple Product." From MathWorld--A Wolfram Web Resource. 
	http://mathworld.wolfram.com/ScalarTripleProduct.html
 * Assumes 3 Dimensions */
void StGermain_ComplexScalarTripleProduct( Cmplx* vectorA, Cmplx* vectorB, Cmplx* vectorC, Cmplx tripleProduct ) {
	XYZC crossProduct;
	
	StGermain_ComplexVectorCrossProduct( crossProduct, vectorB, vectorC );
	StGermain_ComplexVectorDotProduct( vectorA, crossProduct, 3, tripleProduct );
	 
}


/** StGermain_ComplexVectorNormalise calculates the magnitude of a vector
\hat v = frac{v} / {|v|}
This function uses function StGermain_VectorDotProduct to calculate v . v. 
Vector has to be of size dim Cmplx */
void StGermain_ComplexVectorNormalise(Cmplx* vector, Index dim) {
	double mag;

	mag = StGermain_ComplexVectorMagnitude( vector , dim );
	switch (dim) {
		case 3: 
			Cmplx_RealMultiply(vector[2], 1.0/mag, vector[2]);
		
		case 2:
			Cmplx_RealMultiply(vector[1], 1.0/mag, vector[1]);

		case 1: 
			Cmplx_RealMultiply(vector[0], 1.0/mag, vector[0]);			

			break;
		default: {
			Index d;
			for ( d = 0 ; d < dim ; d++ )
				Cmplx_RealMultiply(vector[d], 1.0/mag, vector[d]);
			break;
		}
	}	
}



#define STGERMAIN_COMPLEXVECTOR_ONE_THIRD 0.3333333333333333333

/** StGermain_ComplexTriangleCentroid Calculates the position vector to the centroid of a triangle whose verticies are given by position vectors 
Position vectors have to be of size dim Cmplx */
void StGermain_ComplexTriangleCentroid( Cmplx* centroid, Cmplx* pos0, Cmplx* pos1, Cmplx* pos2, Index dim) {
	Cmplx tmp;
	switch (dim) {
		case 3:
			Cmplx_Add(pos0[2], pos1[2], tmp);
			Cmplx_Add(pos2[2], tmp, tmp);
			Cmplx_RealMultiply(tmp, STGERMAIN_COMPLEXVECTOR_ONE_THIRD, centroid[2]);
			
		case 2: 
			Cmplx_Add(pos0[1], pos1[1], tmp);
			Cmplx_Add(pos2[1], tmp, tmp);
			Cmplx_RealMultiply(tmp, STGERMAIN_COMPLEXVECTOR_ONE_THIRD, centroid[1]);
		case 1:
			Cmplx_Add(pos0[0], pos1[0], tmp);
			Cmplx_Add(pos2[0], tmp, tmp);
			Cmplx_RealMultiply(tmp, STGERMAIN_COMPLEXVECTOR_ONE_THIRD, centroid[0]);
			return;
		default: {
			Index d;
			for ( d = 0 ; d < dim ; d++ ) {
				Cmplx_Add(pos0[d], pos1[d], tmp);
				Cmplx_Add(pos2[d], tmp, tmp);
				Cmplx_RealMultiply(tmp, STGERMAIN_COMPLEXVECTOR_ONE_THIRD, centroid[d]);
			}
			return;
		}
	}
}

/** Prints complex Vector using %g on all entries.
TODO: would like this to be specified from function call
with automatic default value
*/
void StGermain_PrintComplexVector( Stream* stream, Cmplx* vector, Index dim ) {
	Index d;

	if ( dim <= 0 ) {
		Journal_Printf( stream, "{<NON_POSITIVE DIMENSION %d>}\n", dim );
		return;
	}

	Journal_Printf( stream, "{");
	for ( d = 0 ; d < dim - 1 ; d++ ) 
		Journal_Printf( stream, "%g + i %g, ", vector[d][REAL_PART], vector[d][IMAG_PART] );
	
	Journal_Printf( stream, "%g + i %g}\n", vector[d][REAL_PART], vector[d][IMAG_PART] );
}

/** Converts ComplexVector into a vector,
but only if there are no non-zero imaginary values. */
void ComplexVector_ToVector(CoordC complexVector, Dimension_Index dim, Coord vector) {
	Dimension_Index index;
	for (index = 0; index < dim; index++) {
		if (complexVector[index][IMAG_PART] != 0.0) {
			Journal_Firewall( False, Journal_Register( Error_Type, "ComplexVectorMath" ),
				"Error in '%s': Complex value in complex vector at index '%s' \n", __func__, index );
		}
		else {
			vector[index] = complexVector[index][REAL_PART];
		}
	}
}

/** Converts vector into Complex vector, seting all imaginary 
parts to zero */
void Vector_ToComplexVector(Coord vector, Dimension_Index dim, CoordC complexVector) {
	Dimension_Index index;
	for (index = 0; index < dim; index++) {
		complexVector[index][REAL_PART] = vector[index];
		complexVector[index][IMAG_PART] = 0.0;
	}		
}

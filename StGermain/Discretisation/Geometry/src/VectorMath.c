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
** $Id: VectorMath.c 3735 2006-08-01 08:05:14Z KathleenHumble $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"


#include "units.h"
#include "types.h"
#include "VectorMath.h"

#include <math.h>
#include <assert.h>

/*--------------------------------------------------------------------------------------------------------------------------
** Macros
*/


/*--------------------------------------------------------------------------------------------------------------------------
** Functions
*/

/*
** Base operations.
*/

/** (Assumes 3D) Define a cross product of 2 vectors */
void Vector_Cross( Coord dst, Coord a, Coord b ) {
	Coord	tmp;
	
	tmp[0] = a[1] * b[2] - a[2] * b[1];
	tmp[1] = a[2] * b[0] - a[0] * b[2];
	tmp[2] = a[0] * b[1] - a[1] * b[0];
	
	Vector_Set( dst, tmp );
}

/** (Assumes 3D) Divide a vector by a real */
void Vector_Div( Coord dest, Coord a, double s )
{
	double	inv = 1.0 / s;
	
	dest[0] = a[0] * inv;
	dest[1] = a[1] * inv;
	dest[2] = a[2] * inv;
}

/** Calculate the normal of the vector. (ie length = 1 )*/
void Vector_Norm( Coord dest, Coord a )
{
	double	invMag = 1.0 / sqrt( a[0] * a[0] + a[1] * a[1] + a[2] * a[2] );
	
	dest[0] = a[0] * invMag;
	dest[1] = a[1] * invMag;
	dest[2] = a[2] * invMag;
}

/** Swap coordinates according to i,j, k index */
void Vector_Swizzle( Coord dst, Coord src, unsigned char iInd, unsigned char jInd, unsigned char kInd ) {
	assert( iInd < 3 && jInd < 3 && kInd < 3 );
	
	dst[0] = src[iInd];
	dst[1] = src[jInd];
	dst[2] = src[kInd];
}


/** StGermain_RotateVector takes an argument 'vectorToRotate', to rotate around a second vector 'axis' through an angle 
'theta' radians. The rotated vector is stored in vector from the first argument. Each vector: 'rotatedVector', 
'vectorToRotate' and 'axis' must be the size of 3 doubles. The function uses Rodrigues' Rotation Formula 
which was taken from: 
Eric W. Weisstein et al. "Rodrigues' Rotation Formula." From MathWorld--A Wolfram Web Resource. 
http://mathworld.wolfram.com/RodriguesRotationFormula.html. */
void StGermain_RotateVector(double* rotatedVector, double* vector, double* w, double theta) {
	double rotationMatrix[3][3]; 	/* Indicies [Column][Row] */
	double cosTheta = cos(theta);
	double sinTheta = sin(theta);
	
	rotationMatrix[0][0] = cosTheta + w[0] * w[0] * (1.0 - cosTheta);
	rotationMatrix[0][1] = w[0] * w[1] * (1.0 - cosTheta) - w[2] * sinTheta;
	rotationMatrix[0][2] = w[1] * sinTheta + w[0] * w[2] * (1.0 - cosTheta);

	rotationMatrix[1][0] = w[2] * sinTheta + w[0] * w[1] * (1.0 - cosTheta);
	rotationMatrix[1][1] = cosTheta + w[1] * w[1] * (1.0 - cosTheta);
	rotationMatrix[1][2] = -w[0] * sinTheta + w[1] * w[2] * (1.0 - cosTheta);
	
	rotationMatrix[2][0] = -w[1] * sinTheta + w[0] * w[2] * (1.0 - cosTheta);
	rotationMatrix[2][1] = w[0] * sinTheta + w[1] * w[2] * (1.0 - cosTheta);
	rotationMatrix[2][2] = cosTheta + w[2] * w[2] * (1.0 - cosTheta);

	rotatedVector[0] = rotationMatrix[0][0]*vector[0] + rotationMatrix[0][1]*vector[1] + rotationMatrix[0][2]*vector[2];
	rotatedVector[1] = rotationMatrix[1][0]*vector[0] + rotationMatrix[1][1]*vector[1] + rotationMatrix[1][2]*vector[2];
	rotatedVector[2] = rotationMatrix[2][0]*vector[0] + rotationMatrix[2][1]*vector[1] + rotationMatrix[2][2]*vector[2];
}		

/** StGermain_RotateCoordinateAxis multiplies a vector with a Rotation Matrix to rotate it around a co-ordinate axis -
Is a simpler function than StGermain_RotateVector for more specific cases where the vector is to be rotated around one of the axes of the co-ordinate system. The arguments are the same except the the 'axis' argument is of type 'Index' which could be either I_AXIS, J_AXIS or K_AXIS. Vectors have to be the size of 3 doubles.
See, Eric W. Weisstein. "Rotation Matrix." 
From MathWorld--A Wolfram Web Resource. http://mathworld.wolfram.com/RotationMatrix.htm */
void StGermain_RotateCoordinateAxis( double* rotatedVector, double* vector, Index axis, double theta ) {
	
	/* Rotation around one axis will always leave the component on that axis alone */
	rotatedVector[axis] = vector[axis];
	
	switch (axis) {
		case K_AXIS: /* Rotate around Z axis */
			rotatedVector[0] =  vector[0]*cos(theta) - vector[1]*sin(theta);
			rotatedVector[1] =  vector[0]*sin(theta) + vector[1]*cos(theta);
			return;
		case I_AXIS:  /* Rotate around X axis */
			rotatedVector[1] =  vector[1]*cos(theta) - vector[2]*sin(theta);
			rotatedVector[2] =  vector[1]*sin(theta) + vector[2]*cos(theta);
			return;
		case J_AXIS: /* Rotate around Y axis */
			rotatedVector[0] =  vector[0]*cos(theta) + vector[2]*sin(theta);
			rotatedVector[2] = -vector[0]*sin(theta) + vector[2]*cos(theta);
			return;
		default: {
			Stream* error = Journal_Register( ErrorStream_Type, "VectorMath" );
			Journal_Printf( error, "Impossible axis to rotate around in %s.", __func__);
			exit(EXIT_FAILURE);
		}
	}
}

/** Subtracts one vector from another - 
destination = vector1 - vector2
Destination vector may be the same as either of the other two input vectors 
Function is optimised for 1-3 dimensions but will work for any dimension */
void StGermain_VectorSubtraction(double* destination, double* vector1, double* vector2, Index dim) {
	switch (dim) {
		case 3: 
			destination[2] = vector1[2] - vector2[2];
		case 2:
			destination[1] = vector1[1] - vector2[1];
		case 1: 
			destination[0] = vector1[0] - vector2[0];
			return;
		default: {
			Index d;
			for ( d = 0 ; d < dim ; d++ ) 
				destination[d] = vector1[d] - vector2[d];
			return;
		}
	}	
}

/** Adds two vectors - 
destination = vector1 + vector2
Destination vector may be the same as either of the other two input vectors 
Function is optimised for 1-3 dimensions but will work for any dimension */
void StGermain_VectorAddition(double* destination, double* vector1, double* vector2, Index dim) {
	switch (dim) {
		case 3: 
			destination[2] = vector1[2] + vector2[2];
		case 2:
			destination[1] = vector1[1] + vector2[1];
		case 1: 
			destination[0] = vector1[0] + vector2[0];
			return;
		default: {
			Index d;
			for ( d = 0 ; d < dim ; d++ ) 
				destination[d] = vector1[d] + vector2[d];
			return;
		}
	}	
}

/** StGermain_VectorMagnitude calculates the magnitude of a vector
|v| = \sqrt{ v . v } 
This function uses function StGermain_VectorDotProduct to calculate v . v. 
Vector has to be of size dim doubles */
double StGermain_VectorMagnitude(double* vector, Index dim) {
	return sqrt(StGermain_VectorDotProduct(vector,vector,dim));
}

/** StGermain_VectorDotProduct calculates the magnitude of a vector
|v| = \sqrt{ v . v } 
This function uses function StGermain_VectorDotProduct to calculate v . v. 
Vectors have to be of size dim doubles
*/
double StGermain_VectorDotProduct(double* vector1, double* vector2, Index dim) {
	double dotProduct = 0.0;

	switch (dim) {
		case 3:	
			dotProduct = dotProduct + vector1[2] * vector2[2];
		case 2:
			dotProduct = dotProduct + vector1[1] * vector2[1];
		case 1:
			dotProduct = dotProduct + vector1[0] * vector2[0];
			break;
		default: {
			Index d;
			for ( d = 0 ; d < dim ; d++ ) 
				dotProduct = dotProduct + vector1[d] * vector2[d];
			break;
		}
	}
	
	return dotProduct;
}

/** See Eric W. Weisstein. "Cross Product." 
From MathWorld--A Wolfram Web Resource. http://mathworld.wolfram.com/CrossProduct.html 
Tested against http://www.engplanet.com/redirect.html?3859 */
void StGermain_VectorCrossProduct(double* destination, double* vector1, double* vector2) {
	destination[0] = vector1[1]*vector2[2] - vector1[2]*vector2[1];
	destination[1] = vector1[2]*vector2[0] - vector1[0]*vector2[2];
	destination[2] = vector1[0]*vector2[1] - vector1[1]*vector2[0];
}

/** StGermain_VectorCrossProductMagnitude - See Eric W. Weisstein. "Cross Product." 
From MathWorld--A Wolfram Web Resource. http://mathworld.wolfram.com/CrossProduct.html 
|a \times b| = |a||b|\sqrt{ 1 - (\hat a . \hat b)^2}
*/
double StGermain_VectorCrossProductMagnitude( double* vector1, double* vector2, Dimension_Index dim ) {
	double mag1       = StGermain_VectorMagnitude( vector1, dim );
	double mag2       = StGermain_VectorMagnitude( vector2, dim );
	double dotProduct = StGermain_VectorDotProduct( vector1, vector2, dim );
	
	return mag1 * mag2 * sqrt( 1.0 - dotProduct * dotProduct / (mag1 * mag1 * mag2 * mag2) );
}


/** StGermain_ScalarTripleProduct - Calculates the scalar vector product of three vectors -
 * see Eric W. Weisstein. "Scalar Triple Product." From MathWorld--A Wolfram Web Resource. http://mathworld.wolfram.com/ScalarTripleProduct.html
 * Assumes 3 Dimensions */
double StGermain_ScalarTripleProduct( double* vectorA, double* vectorB, double* vectorC ) {
	double crossProduct[3];
	
	StGermain_VectorCrossProduct( crossProduct, vectorB, vectorC );
	return StGermain_VectorDotProduct( vectorA, crossProduct, 3 );
}


/** StGermain_VectorNormalise calculates the magnitude of a vector
\hat v = frac{v} / {|v|}
This function uses function StGermain_VectorDotProduct to calculate v . v. 
Vector has to be of size dim doubles */
void StGermain_VectorNormalise(double* vector, Index dim) {
	double mag;

	mag = StGermain_VectorMagnitude( vector , dim );
	switch (dim) {
		case 3: 
			vector[2] = vector[2]/mag;
		case 2:
			vector[1] = vector[1]/mag;
		case 1: 
			vector[0] = vector[0]/mag;
			break;
		default: {
			Index d;
			for ( d = 0 ; d < dim ; d++ ) 
				vector[d] = vector[d]/mag;
			break;
		}
	}	
}

/** Finds the angle between two vectors 
\cos{\theta} = \frac{a.b}{|a||b|}
Uses StGermain_AngleBetweenVectors and StGermain_VectorMagnitude
Vectors has to be of size dim doubles */
double StGermain_AngleBetweenVectors( double* vectorA, double* vectorB, Index dim ) {
	double dotProduct = StGermain_VectorDotProduct(vectorA, vectorB, dim);
	double magA = StGermain_VectorMagnitude( vectorA, dim );
	double magB = StGermain_VectorMagnitude( vectorB, dim );

	double value = dotProduct/ (magA * magB);

	if (value >= 1.0)
		return 0.0;
	if (value <= -1.0)
		return M_PI;

	return acos( value );
}

/** Calculates the distance between two points - given by their position vectors relative to the origin
distance = |a - b|
Uses StGermain_VectorSubtraction and StGermain_VectorMagnitude
Position vectors have to be of size dim doubles */
double StGermain_DistanceBetweenPoints( double* pos1, double* pos2, Index dim) {
	double mag;
	double *vector;
	
	/* Allocate memory */
	vector = Memory_Alloc_Array( double, dim, "Vector" );
	
	/* vector between points */
	StGermain_VectorSubtraction( vector, pos1, pos2, dim ) ;
	mag = StGermain_VectorMagnitude( vector, dim );

	Memory_Free( vector );
	return mag;
}

/** Given three points which define a plane, StGermain_NormalToPlane will give the unit vector which is normal to that plane
Uses StGermain_VectorSubtraction, StGermain_VectorCrossProduct and StGermain_VectorNormalise
Position vectors and normal have to be of size 3 doubles */
void StGermain_NormalToPlane( double* normal, double* pos0, double* pos1, double* pos2) {
	double vector1[3], vector2[3];

	StGermain_VectorSubtraction( vector1, pos1, pos0, 3) ;
	StGermain_VectorSubtraction( vector2, pos2, pos0, 3) ;
	
	StGermain_VectorCrossProduct( normal, vector1, vector2 );

	StGermain_VectorNormalise( normal, 3 );
}

#define ONE_THIRD 0.3333333333333333333
/** Calculates the position vector to the centroid of a triangle whose verticies are given by position vectors 
Position vectors have to be of size dim doubles */
void StGermain_TriangleCentroid( double* centroid, double* pos0, double* pos1, double* pos2, Index dim) {

	switch (dim) {
		case 3:
			centroid[2] = ONE_THIRD * (pos0[2] + pos1[2] + pos2[2]);
		case 2: 
			centroid[1] = ONE_THIRD * (pos0[1] + pos1[1] + pos2[1]);
		case 1:
			centroid[0] = ONE_THIRD * (pos0[0] + pos1[0] + pos2[0]);
			return;
		default: {
			Index d;
			for ( d = 0 ; d < dim ; d++ ) 
				centroid[d] = ONE_THIRD * (pos0[d] + pos1[d] + pos2[d]);
			return;
		}
	}
}

/** Calculates the area of a triangle whose verticies are given by position vectors 
Position vectors have to be of size dim doubles 
Only works for dim == 2 or dim == 3
*/
double StGermain_TriangleArea( double* pos0, double* pos1, double* pos2, Index dim ) {
	double normal[3];
	double vector1[3], vector2[3];
	double area;

	if (dim != 2 && dim != 3) return 0.0;

	vector1[2] = vector2[2] = 0.0;

	StGermain_VectorSubtraction( vector1, pos1, pos0, dim) ;
	StGermain_VectorSubtraction( vector2, pos2, pos0, dim) ;

	StGermain_VectorCrossProduct( normal, vector1, vector2 );
	
	area = 0.5 * StGermain_VectorMagnitude(normal, dim);
	return area;
}

/** StGermain_ConvexQuadrilateralArea - See Eric W. Weisstein. "Quadrilateral." 
 * From MathWorld--A Wolfram Web Resource. http://mathworld.wolfram.com/Quadrilateral.html
 * Area = \frac{1}{2}| p \times q | where p and q are diagonals of a convex polygon
 * This function will not work for dim > 3 and verticies have to be given in a winding direction
*/
double StGermain_ConvexQuadrilateralArea( double* vertexCoord1, double* vertexCoord2, double* vertexCoord3, double* vertexCoord4, Dimension_Index dim ) {
	Coord diagonal1;	
	Coord diagonal2;

	/* Calculate Diagonals */
	StGermain_VectorSubtraction( diagonal1, vertexCoord3, vertexCoord1, dim );
	StGermain_VectorSubtraction( diagonal2, vertexCoord4, vertexCoord2, dim );

	return 0.5 * StGermain_VectorCrossProductMagnitude( diagonal1, diagonal2, dim );
}

/** StGermain_ParallelepipedVolume - Calculates the volume of parallelepiped -
 * see Eric W. Weisstein et al. "Parallelepiped." From MathWorld--A Wolfram Web Resource. http://mathworld.wolfram.com/Parallelepiped.html
 * Assumes 3 Dimensions */
double StGermain_ParallelepipedVolume( 
		double* coordLeftBottomFront, 
		double* coordRightBottomFront, 
		double* coordLeftTopFront, 
		double* coordLeftBottomBack ) {

	double vectorA[3], vectorB[3], vectorC[3];

	/* Find vectors which define Parallelepiped */
	StGermain_VectorSubtraction( vectorA, coordRightBottomFront, coordLeftBottomFront, 3 );
	StGermain_VectorSubtraction( vectorB, coordLeftTopFront,     coordLeftBottomFront, 3 );
	StGermain_VectorSubtraction( vectorC, coordLeftBottomBack,   coordLeftBottomFront, 3 );

	return fabs( StGermain_ScalarTripleProduct( vectorA, vectorB, vectorC ) );
}	

/** StGermain_ParallelepipedVolumeFromCoordList - 
 * shortcut for StGermain_ParallelepipedVolume - assuming nodes laid out in order of
 * 	Left Bottom Front        
 * 	Right Bottom Front        
 * 	Right Top Front
 * 	Left Top_Front             
 * 	Left Bottom Back       
 * 	Right Bottom Back      
 * 	Right Top Back 
 * 	Left Top Back   */
double StGermain_ParallelepipedVolumeFromCoordList( Coord_List list ) {
	return StGermain_ParallelepipedVolume( list[ 0 ], list[ 1 ], list[ 3 ], list[ 4 ] );
}

/** StGermain_AverageCoord - Does simple average of coordinates */
void StGermain_AverageCoord( double* coord, double** coordList, Index count, Dimension_Index dim ) {
	double factor = 1.0/(double)count;
	Index  coord_I;
	
	/* Initialise */
	coord[ I_AXIS ] = coord[ J_AXIS ] = coord[ K_AXIS ] = 0.0;

	/* Sum verticies */
	for ( coord_I = 0 ; coord_I < count ; coord_I++ ) {
		coord[ I_AXIS ] += coordList[ coord_I ][ I_AXIS ];
		coord[ J_AXIS ] += coordList[ coord_I ][ J_AXIS ];
		if (dim == 3)
			coord[ K_AXIS ] += coordList[ coord_I ][ K_AXIS ];
	}

	/* Find Average */
	coord[ I_AXIS ] *= factor;
	coord[ J_AXIS ] *= factor;
	if (dim == 3)
		coord[ K_AXIS ] *= factor;
}
/** Prints a vector of any non-zero positive length 
Uses %lf print statement*/
void StGermain_PrintVector( Stream* stream, double* vector, Index dim ) {
	Index d;

	if ( dim <= 0 ) {
		Journal_Printf( stream, "{<NON_POSITIVE DIMENSION %d>}\n", dim );
		return;
	}

	Journal_Printf( stream, "{");
	for ( d = 0 ; d < dim - 1 ; d++ ) 
		Journal_Printf( stream, "%lf, ", vector[d] );
	
	Journal_Printf( stream, "%lf}\n", vector[d] );
}

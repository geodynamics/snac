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
** Role:
**    Provides basic vector operations.
**
** Assumptions:
**    - Coord is an array of 3 doubles.
**
** Comments:
**    In any operation that involves two or more input vectors, those vectors 
**    may be one and the same; it may be assumed that such an occurence will be
**    handled.
**
** $Id: VectorMath.h 3926 2007-01-02 04:53:16Z KathleenHumble $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Geometry_VectorMath_h__
#define __Discretisation_Geometry_VectorMath_h__
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Macros
	*/
	
	/*
	** Base operations.
	*/
	
	/** copy src onto dest */
	#define Vector_Set( dest, src )		\
		(dest)[0] = (src)[0];		\
		(dest)[1] = (src)[1];		\
		(dest)[2] = (src)[2]
	
	
	/** set dest's components to src */
	#define Vector_SetScalar( dest, x, y, z )		\
		(dest)[0] = x;					\
		(dest)[1] = y;					\
		(dest)[2] = z
	/** set dest's components to src vector in 2d */
	#define Vector_SetScalar2D( dest, x, y )                \
		(dest)[0] = x;                                  \
		(dest)[1] = y	
	
	/** dest = a + b */
	#define Vector_Add( dest, a, b )		\
		(dest)[0] = (a)[0] + (b)[0];		\
		(dest)[1] = (a)[1] + (b)[1];		\
		(dest)[2] = (a)[2] + (b)[2]
		
	
	/** dest = a - b */
	#define Vector_Sub( dest, a, b )		\
		(dest)[0] = (a)[0] - (b)[0];		\
		(dest)[1] = (a)[1] - (b)[1];		\
		(dest)[2] = (a)[2] - (b)[2]
		
	
	/** returns the dot product of a and b */
	#define Vector_Dot( a, b )						\
		((a)[0] * (b)[0] + (a)[1] * (b)[1] + (a)[2] * (b)[2])
		
	
	/** dest = a * s */
	#define Vector_Mult( dest, a, s )		\
		(dest)[0] = (a)[0] * (s);		\
		(dest)[1] = (a)[1] * (s);		\
		(dest)[2] = (a)[2] * (s)
		
	
	/** returns the magnitude of a */
	#define Vector_Mag( a )								\
		sqrt( (a)[0] * (a)[0] + (a)[1] * (a)[1] + (a)[2] * (a)[2] )
		
		
	/*
	** Combinations of base operations.
	*/
	
	/** vector projection of a onto b, store result in dest */
	#define Vector_Proj( dest, a, b )					\
		Vector_Norm( (dest), (b) );					\
		Vector_Mult( (dest), (dest), Vector_Dot( a, b ) )


	/*--------------------------------------------------------------------------------------------------------------------------
	** Functions
	*/
	
	/*
	** Base operations.
	*/
	
	/* Initialise a Vecotor with zeros */
	void ZeroVector( double* vector, Index length );
	/* Vector cross-product. */
	void Vector_Cross( Coord dst, Coord a, Coord b );
	
	/* dest = a / s */
	void Vector_Div( Coord dest, Coord a, double s );
	
	/* normalise a, store in dest */
	void Vector_Norm( Coord dest, Coord a );

	void StGermain_RotateVector(double* rotatedVector, double* vector, double* w, double theta) ;
	void StGermain_RotateCoordinateAxis( double* rotatedVector, double* vector, Index axis, double theta ) ;
	void StGermain_VectorSubtraction(double* destination, double* vector1, double* vector2, Index dim) ;
	void StGermain_VectorAddition(double* destination, double* vector1, double* vector2, Index dim) ;
	double StGermain_VectorMagnitude(double* vector, Index dim) ;
	double StGermain_VectorDotProduct(double* vector1, double* vector2, Index dim) ;
	void StGermain_VectorCrossProduct(double* destination, double* vector1, double* vector2) ;
	double StGermain_VectorCrossProductMagnitude( double* vector1, double* vector2, Dimension_Index dim ) ;
	double StGermain_ScalarTripleProduct( double* vectorA, double* vectorB, double* vectorC ) ;

	void StGermain_VectorNormalise(double* vector, Index dim) ;
	double StGermain_AngleBetweenVectors( double* vectorA, double* vectorB, Index dim ) ;
	double StGermain_DistanceBetweenPoints( double* pos1, double* pos2, Index dim) ;
	void StGermain_NormalToPlane( double* normal, double* pos0, double* pos1, double* pos2) ;

	void StGermain_TriangleCentroid( double* centroid, double* pos0, double* pos1, double* pos2, Index dim) ;
	double StGermain_TriangleArea( double* pos0, double* pos1, double* pos2, Index dim ) ;
	double StGermain_ConvexQuadrilateralArea( double* vertexCoord1, double* vertexCoord2, double* vertexCoord3, double* vertexCoord4, Dimension_Index dim ) ;
	double StGermain_ParallelepipedVolume( 
		double* coordLeftBottomFront, 
		double* coordRightBottomFront, 
		double* coordLeftTopFront, 
		double* coordLeftBottomBack );
	double StGermain_ParallelepipedVolumeFromCoordList( Coord_List list ) ;
	
	void StGermain_AverageCoord( double* coord, double** coordList, Index count, Dimension_Index dim ) ;
	void StGermain_PrintVector( Stream* stream, double* vector, Index dim ) ;
	/** Print a named vector. Name comes from vector variable in file*/
	#define StGermain_PrintNamedVector(stream, vector, dim) \
		do {	\
			Journal_Printf( stream, #vector " - " ); \
			StGermain_PrintVector( stream, vector, dim );	\
		} while(0)

#endif /* __Discretisation_Geometry_VectorMath_h__ */

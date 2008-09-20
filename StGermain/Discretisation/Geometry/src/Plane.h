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
**    Provides operations for use on infinite planes.
**
** Assumptions:
**    - Coord is an array of 3 doubles.
**
** Comments:
**
** $Id: Plane.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Geometry_Plane_h__
#define __Discretisation_Geometry_Plane_h__
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Macros
	*/
	
	/* calculate a plane equation from two non-parallel vectors and a point */
	#define Plane_CalcFromVec( dest, axisA, axisB, pointOnPlane )		\
		Vector_Cross( dest, axisA, axisB );				\
		Vector_Norm( dest, dest );					\
		dest[3] = Vector_Dot( dest, pointOnPlane )
		
	
	/* return the plane's normal in dest */
	#define Plane_Normal( dest, plane )		\
		Vector_Set( dest, plane )
		
	
	/* calculate the shortest distance from plane to point */
	#define Plane_DistanceToPoint( plane, point )		\
		(Vector_Dot( plane, point ) - plane[3])
	
	
	/* determines if point is in front of the plane based on the direction of the plane's normal */
	#define Plane_PointIsInFront( plane, point )					\
		(Plane_DistanceToPoint( plane, point ) > 0.0 ? True : False)
	
	
	/* determines if the point is situated on the plane */
	#define Plane_PointIsOnPlane( plane, point )				\
		(Vector_Dot( plane, point ) == plane[3] ? True : False)
		

	/*--------------------------------------------------------------------------------------------------------------------------
	** Functions
	*/
	
	void Plane_LineIntersect( Plane plane, Line3 line, Coord point );

	
#endif /* __Discretisation_Geometry_Plane_h__ */

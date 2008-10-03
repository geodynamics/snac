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
**
** Comments:
**
** $Id: TrigMath.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Geometry_TrigMath_h__
#define __Discretisation_Geometry_TrigMath_h__

	#define UPPER_RIGHT_QUADRANT 	0
	#define UPPER_LEFT_QUADRANT 	1
	#define LOWER_LEFT_QUADRANT 	2
	#define LOWER_RIGHT_QUADRANT 	3
	
	#ifndef M_PI
		#define M_PI   3.14159265358979323846
	#endif
	#ifndef M_1_PI
		#define M_1_PI 0.31830988618379067154
	#endif
	#ifndef M_2_PI
		#define M_2_PI 0.63661977236758134308
	#endif

	#define StGermain_RadianToDegree(RAD) (180.0 * M_1_PI * (RAD))
	#define StGermain_DegreeToRadian(DEG) (M_PI / 180.0 * (DEG))

	void StGermain_RectangularToSpherical(double* sphericalCoords, double* rectangularCoords, Index dim) ;
	void StGermain_SphericalToRectangular(double* rectangularCoords, double* sphericalCoords, Index dim) ;

	double StGermain_TrigDomain(double angle) ;
	char StGermain_TrigQuadrant(double angle) ;

#endif /* __Discretisation_Geometry_TrigMath_h__ */

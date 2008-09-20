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
** $Id: RMatrix.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Geometry_TMatrix_h__
#define __Discretisation_Geometry_TMatrix_h__
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Functions
	*/
	
	#define ArcTan( o, a ) \
		(o < 0.0 ? atan( o / a ) + M_PI : o < 0.0 ? atan( o / a ) + 2.0 * M_PI : atan( o / a ))
	
	void RMatrix_BuildOrientation( RMatrix dest, Coord zAxis, Coord xAxis );
	
	void RMatrix_ApplyRotationX( RMatrix dest, double angle );
	
	void RMatrix_ApplyRotationY( RMatrix dest, double angle );
	
	void RMatrix_ApplyRotationZ( RMatrix dest, double angle );
	
	void RMatrix_Mult( RMatrix dest, RMatrix a, RMatrix b );
	
	void RMatrix_VectorMult( Coord dest, RMatrix mat, Coord vec );
	
	#define RMatrix_LoadIdentity( dest ) \
		(dest)[0] = 1.0; (dest)[1] = 0.0; (dest)[2] = 0.0; \
		(dest)[3] = 0.0; (dest)[4] = 1.0; (dest)[5] = 0.0; \
		(dest)[6] = 0.0; (dest)[7] = 0.0; (dest)[8] = 1.0
	
	
#endif /* __Discretisation_Geometry_TMatrix_h__ */

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
** $Id: RMatrix.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"


#include "units.h"
#include "types.h"
#include "VectorMath.h"
#include "RMatrix.h"

#include <math.h>
#include <assert.h>
#include <string.h>


/*--------------------------------------------------------------------------------------------------------------------------
** Functions
*/

void RMatrix_BuildOrientation( RMatrix dest, Coord zAxis, Coord xAxis ) {
	Coord		nxa;
	
	RMatrix_LoadIdentity( dest );
	RMatrix_ApplyRotationY( dest, ArcTan( zAxis[0], zAxis[2] ) );
	RMatrix_ApplyRotationX( dest, asin( zAxis[1] / Vector_Mag( zAxis ) ) );
	
	RMatrix_VectorMult( nxa, dest, xAxis );
	RMatrix_ApplyRotationZ( dest, asin( nxa[1] / Vector_Mag( nxa ) ) );
}


void RMatrix_ApplyRotationY( RMatrix dest, double angle ) {
	RMatrix		tmp;
	
	tmp[0] = cos( angle );  tmp[1] = 0.0;   tmp[2] = -sin( angle );
	tmp[3] = 0.0;		tmp[4] = 1.0;   tmp[5] = 0.0;
	tmp[6] = sin( angle );  tmp[7] = 0.0;   tmp[8] = cos( angle );
	
	RMatrix_Mult( dest, dest, tmp );
}


void RMatrix_ApplyRotationZ( RMatrix dest, double angle ) {
	RMatrix		tmp;
	
	tmp[0] = cos( angle );  tmp[1] = sin( angle );  tmp[2] = 0.0;
	tmp[3] = -sin( angle);  tmp[4] = cos( angle );  tmp[5] = 0.0;
	tmp[6] = 0.0;		tmp[7] = 0.0;		tmp[8] = 1.0;
	
	RMatrix_Mult( dest, dest, tmp );
}


void RMatrix_ApplyRotationX( RMatrix dest, double angle ) {
	RMatrix		tmp;
	
	tmp[0] = 1.0;   tmp[1] = 0.0;		tmp[2] = 0.0;
	tmp[3] = 0.0;   tmp[4] = cos( angle );  tmp[5] = sin( angle );
	tmp[6] = 0.0;   tmp[7] = -sin( angle);  tmp[8] = cos( angle );
	
	RMatrix_Mult( dest, dest, tmp );
}


void RMatrix_Mult( RMatrix dest, RMatrix a, RMatrix b ) {
	RMatrix		tmp;
	
	tmp[0] = a[0] * b[0] + a[1] * b[3] + a[2] * b[6];
	tmp[1] = a[0] * b[1] + a[1] * b[4] + a[2] * b[7];
	tmp[2] = a[0] * b[2] + a[1] * b[5] + a[2] * b[8];
	tmp[3] = a[3] * b[0] + a[4] * b[3] + a[5] * b[6];
	tmp[4] = a[3] * b[1] + a[4] * b[4] + a[5] * b[7];
	tmp[5] = a[3] * b[2] + a[4] * b[5] + a[5] * b[8];
	tmp[6] = a[6] * b[0] + a[7] * b[3] + a[8] * b[6];
	tmp[7] = a[6] * b[1] + a[7] * b[4] + a[8] * b[7];
	tmp[8] = a[6] * b[2] + a[7] * b[5] + a[8] * b[8];
	
	memcpy( dest, tmp, sizeof(RMatrix) );
}


void RMatrix_VectorMult( Coord dest, RMatrix mat, Coord vec ) {
	Coord		tmp;
	
	tmp[0] = mat[0] * vec[0] + mat[1] * vec[1] + mat[2] * vec[2];
	tmp[1] = mat[3] * vec[0] + mat[4] * vec[1] + mat[5] * vec[2];
	tmp[2] = mat[6] * vec[0] + mat[7] * vec[1] + mat[8] * vec[2];
	
	Vector_Set( dest, tmp );
}

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
**	Kathleen M. Humble, Computational Scientist, VPAC. (khumble@vpac.org)
**
**  This library is free software; you can redistribute it and/or
**  modify it under the terms of the GNU Lesser General Public
**  License as publishdify it under the terms of the GNU Lesser General Public
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
*/
/** \file
**  Role:
**
** Assumptions:
**
** Comments:
**
** $Id: units.h 3735 2006-08-01 08:05:14Z KathleenHumble $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Geometry_units_h__
#define __Discretisation_Geometry_units_h__

	#define MAX_SYMMETRIC_TENSOR_COMPONENTS 6
	#define MAX_TENSOR_COMPONENTS           9
	
	/** Defines a complex number as 2 doubles */
	typedef double Cmplx[2];
	/** Defines a set of 3 doubles */
	typedef double XYZ[3];
	typedef XYZ Coord;
	/**Defines a set of 3 floats */
	typedef float XYZF[3];
	typedef XYZF CoordF;
	/** Defines a set of 3 integers that can be used to index
	into vectors */
	typedef int XYZI[3];
	typedef XYZI CoordI;
	/** Defines a set of 3 Cmplx */
	typedef Cmplx XYZC[3];
	typedef XYZC CoordC;

	typedef double SymmetricTensor[ MAX_SYMMETRIC_TENSOR_COMPONENTS ];
	typedef double TensorArray[ MAX_TENSOR_COMPONENTS ];
	typedef Cmplx ComplexTensorArray[MAX_TENSOR_COMPONENTS]; 

#endif /* __Discretisation_Geometry_units_h__ */

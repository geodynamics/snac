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
** $Id: Init.c 3599 2006-05-22 00:25:46Z AlanLo $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <Base/Base.h>
#include <Discretisation/Geometry/Geometry.h>


#include "Shape.h"

#include <stdio.h>

Bool DiscretisationShape_Init( int* argc, char** argv[] ) {
	Stg_ComponentRegister* componentRegister = Stg_ComponentRegister_Get_ComponentRegister();

	Journal_Printf( Journal_Register( DebugStream_Type, "Context" ), "In: %s\n", __func__ ); /* DO NOT CHANGE OR REMOVE */

	Stg_ComponentRegister_Add( componentRegister, Superellipsoid_Type, "0", _Superellipsoid_DefaultNew );
	Stg_ComponentRegister_Add( componentRegister, Box_Type,            "0", _Box_DefaultNew );
	Stg_ComponentRegister_Add( componentRegister, Everywhere_Type,     "0", _Everywhere_DefaultNew );
	Stg_ComponentRegister_Add( componentRegister, ConvexHull_Type,     "0", _ConvexHull_DefaultNew );
	Stg_ComponentRegister_Add( componentRegister, Sphere_Type,         "0", _Sphere_DefaultNew );
	Stg_ComponentRegister_Add( componentRegister, PolygonShape_Type,        "0", _PolygonShape_DefaultNew );
	Stg_ComponentRegister_Add( componentRegister, Union_Type,          "0", _Union_DefaultNew );
	Stg_ComponentRegister_Add( componentRegister, Intersection_Type,   "0", _Intersection_DefaultNew );
	Stg_ComponentRegister_Add( componentRegister, Cylinder_Type,       "0", _Cylinder_DefaultNew );
	
	Stg_ComponentRegister_Add( componentRegister, BelowPlane_Type,     "0", _BelowPlane_DefaultNew );
	Stg_ComponentRegister_Add( componentRegister, BelowCosinePlane_Type,     "0", _BelowCosinePlane_DefaultNew );
	
	#ifdef HAVE_PYTHON
		Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), PythonShape_Type, "0", _PythonShape_DefaultNew );
		RegisterParent( PythonShape_Type, Stg_Shape_Type);
	#endif

	RegisterParent( Stg_Shape_Type,                 Stg_Component_Type );
	RegisterParent( Superellipsoid_Type,            Stg_Shape_Type );
	RegisterParent( Box_Type,                       Stg_Shape_Type );
	RegisterParent( Everywhere_Type,                Stg_Shape_Type );
	RegisterParent( ConvexHull_Type,                Stg_Shape_Type );
	RegisterParent( Sphere_Type,                    Stg_Shape_Type );
	RegisterParent( PolygonShape_Type,                   Stg_Shape_Type );
	RegisterParent( Union_Type,                     Stg_Shape_Type );
	RegisterParent( Intersection_Type,              Stg_Shape_Type );
	RegisterParent( Cylinder_Type,                  Stg_Shape_Type );
	
	RegisterParent( BelowPlane_Type,                  Stg_Shape_Type );
	RegisterParent( BelowCosinePlane_Type,            BelowPlane_Type );
	
	return True;
}

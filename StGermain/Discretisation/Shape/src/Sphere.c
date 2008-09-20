/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003, Victorian Partnership for Advanced Computing (VPAC) Ltd, 110 Victoria Street, Melbourne, 3053, Australia.
**
** Authors:
**	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
**	Patrick D. Sunter, Software Engineer, VPAC. (pds@vpac.org)
**	Luke J. Hodkinson, Computational Engineer, VPAC. (lhodkins@vpac.org)
**	Siew-Ching Tan, Software Engineer, VPAC. (siew@vpac.org) ) 
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
** $Id: Sphere.c 3869 2006-10-16 13:42:59Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <Base/Base.h>
#include <Discretisation/Geometry/Geometry.h>

#include "types.h"
#include "ShapeClass.h"
#include "Sphere.h"

#include <assert.h>
#include <string.h>
#include <math.h>


/* Textual name of this class */
const Type Sphere_Type = "Sphere";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

Sphere* Sphere_New(
		Name                                  name,
		Dimension_Index                       dim,
		XYZ                                   centre, 
		double                                alpha,
		double                                beta,
		double                                gamma,
		double                                radius )
{
	Sphere* self = (Sphere*) _Sphere_DefaultNew( name );

	Sphere_InitAll( 
		self, 
		dim,
		centre,
		alpha,
		beta,
		gamma,
		radius) ;
	return self;
}

Sphere* _Sphere_New(
		SizeT                                 _sizeOfSelf, 
		Type                                  type,
		Stg_Class_DeleteFunction*             _delete,
		Stg_Class_PrintFunction*              _print,
		Stg_Class_CopyFunction*               _copy, 
		Stg_Component_DefaultConstructorFunction* _defaultConstructor,
		Stg_Component_ConstructFunction*      _construct,
		Stg_Component_BuildFunction*          _build,
		Stg_Component_InitialiseFunction*     _initialise,
		Stg_Component_ExecuteFunction*        _execute,
		Stg_Component_DestroyFunction*        _destroy,		
		Stg_Shape_IsCoordInsideFunction*      _isCoordInside,
		Stg_Shape_CalculateVolumeFunction*    _calculateVolume,
		Name                                  name )
{
	Sphere* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(Sphere) );
	self = (Sphere*)_Stg_Shape_New( 
			_sizeOfSelf,
			type,
			_delete,
			_print,
			_copy,
			_defaultConstructor,
			_construct,
			_build,
			_initialise,
			_execute,
			_destroy,		
			_isCoordInside ,
			_calculateVolume,
			name );
	
	/* General info */

	/* Virtual Info */
	self->_isCoordInside = _isCoordInside;
	
	return self;
}

void _Sphere_Init( void* sphere, double radius ) {
	Sphere* self = (Sphere*)sphere;
	self->radius        = radius;
	self->radiusSquared = radius * radius;
}


void Sphere_InitAll( 
		void*                                 sphere, 
		Dimension_Index                       dim, 
		Coord                                 centre,
		double                                alpha,
		double                                beta,
		double                                gamma,
		double                                radius)
{
	Sphere* self = (Sphere*)sphere;

	Stg_Shape_InitAll( self, dim, centre, alpha, beta, gamma );
	_Sphere_Init( self, radius );
}
	

/*------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _Sphere_Delete( void* sphere ) {
	Sphere* self = (Sphere*)sphere;
	
	/* Delete parent */
	_Stg_Shape_Delete( self );
}


void _Sphere_Print( void* sphere, Stream* stream ) {
	Sphere* self = (Sphere*)sphere;
	
	/* Print parent */
	_Stg_Shape_Print( self, stream );
}



void* _Sphere_Copy( void* sphere, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	Sphere*	self = (Sphere*)sphere;
	Sphere*	newSphere;
	
	newSphere = (Sphere*)_Stg_Shape_Copy( self, dest, deep, nameExt, ptrMap );
	
	newSphere->radiusSquared = self->radiusSquared;
	
	return (void*)newSphere;
}

void* _Sphere_DefaultNew( Name name ) {
	return (void*) _Sphere_New(
			sizeof(Sphere),
			Sphere_Type,
			_Sphere_Delete,
			_Sphere_Print,
			_Sphere_Copy,
			_Sphere_DefaultNew,
			_Sphere_Construct,
			_Sphere_Build,
			_Sphere_Initialise,
			_Sphere_Execute,
			_Sphere_Destroy,
			_Sphere_IsCoordInside,
			_Sphere_CalculateVolume,
			name );
}


void _Sphere_Construct( void* sphere, Stg_ComponentFactory* cf, void* data ) {
	Sphere*	self      = (Sphere*) sphere;
	double             radius;

	_Stg_Shape_Construct( self, cf, data );

	radius = Stg_ComponentFactory_GetDouble( cf, self->name, "radius", 0.0 );


	_Sphere_Init( self, radius );
}

void _Sphere_Build( void* sphere, void* data ) {
	Sphere*	self = (Sphere*)sphere;

	_Stg_Shape_Build( self, data );
}
void _Sphere_Initialise( void* sphere, void* data ) {
	Sphere*	self = (Sphere*)sphere;
	
	_Stg_Shape_Initialise( self, data );
}
void _Sphere_Execute( void* sphere, void* data ) {
	Sphere*	self = (Sphere*)sphere;
	
	_Stg_Shape_Execute( self, data );
}
void _Sphere_Destroy( void* sphere, void* data ) {
	Sphere*	self = (Sphere*)sphere;
	
	_Stg_Shape_Destroy( self, data );
}

/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/
	
/*---------------------------------------------------------------------------------------------------------------------
** Private Member functions
*/

Bool _Sphere_IsCoordInside( void* sphere, Coord coord ) {
	Sphere*         self              = (Sphere*)sphere;
	Coord           newCoord;
	double          insideOutsideValue;
	double          x, y, z;

	/* Transform coordinate into canonical reference frame */
	Stg_Shape_TransformCoord( self, coord, newCoord );

	x = newCoord[ I_AXIS ];
	y = newCoord[ J_AXIS ];
	if(self->dim == 2)
		insideOutsideValue = x*x + y*y;
	else {
		z = newCoord[ K_AXIS ];
		insideOutsideValue = x*x + y*y + z*z;
	}

	return (insideOutsideValue <= self->radiusSquared);
	
}

double _Sphere_CalculateVolume( void* sphere ) {
	Sphere* self = sphere;
	if ( self->dim == 2 ) {
		return M_PI * self->radiusSquared;
	}
	else {
		return (4.0/3.0) * M_PI * self->radiusSquared * self->radius;
	}
}
	

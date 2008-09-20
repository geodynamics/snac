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
** $Id: Superellipsoid.c 3869 2006-10-16 13:42:59Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <Base/Base.h>
#include <Discretisation/Geometry/Geometry.h>

#include "types.h"
#include "ShapeClass.h"
#include "Superellipsoid.h"

#include <assert.h>
#include <string.h>
#include <math.h>


/* Textual name of this class */
const Type Superellipsoid_Type = "Superellipsoid";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

Superellipsoid* Superellipsoid_New(
		Name                                  name,
		Dimension_Index                       dim,
		XYZ                                   centre, 
		double                                alpha,
		double                                beta,
		double                                gamma,
		double                                epsilon1,
		double                                epsilon2,   
		XYZ                                   radius )
{
	Superellipsoid* self = (Superellipsoid*) _Superellipsoid_DefaultNew( name );

	Superellipsoid_InitAll( 
		self, 
		dim,
		centre,
		alpha,
		beta,
		gamma,
		epsilon1, 
		epsilon2,
		radius) ;
	return self;
}

Superellipsoid* _Superellipsoid_New(
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
	Superellipsoid* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(Superellipsoid) );
	self = (Superellipsoid*)_Stg_Shape_New( 
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
			_isCoordInside,
			_calculateVolume,
			name );
	
	/* General info */

	/* Virtual Info */
	self->_isCoordInside = _isCoordInside;
	
	return self;
}

void _Superellipsoid_Init( void* superellipsoid, double epsilon1, double epsilon2, XYZ radius ) {
	Superellipsoid* self = (Superellipsoid*)superellipsoid;
	
	self->epsilon1 = epsilon1;
	self->epsilon2 = epsilon2;

	memcpy( self->radius, radius, sizeof(XYZ));
}


void Superellipsoid_InitAll( 
		void*                                 superellipsoid, 
		Dimension_Index                       dim, 
		Coord                                 centre,
		double                                alpha,
		double                                beta,
		double                                gamma,
		double                                epsilon1, 
		double                                epsilon2,
		XYZ                                   radius)
{
	Superellipsoid* self = (Superellipsoid*)superellipsoid;

	Stg_Shape_InitAll( self, dim, centre, alpha, beta, gamma );
	_Superellipsoid_Init( self, epsilon1, epsilon2, radius );
}
	

/*------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _Superellipsoid_Delete( void* superellipsoid ) {
	Superellipsoid* self = (Superellipsoid*)superellipsoid;
	
	/* Delete parent */
	_Stg_Shape_Delete( self );
}


void _Superellipsoid_Print( void* superellipsoid, Stream* stream ) {
	Superellipsoid* self = (Superellipsoid*)superellipsoid;
	
	/* Print parent */
	_Stg_Shape_Print( self, stream );
}



void* _Superellipsoid_Copy( void* superellipsoid, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	Superellipsoid*	self = (Superellipsoid*)superellipsoid;
	Superellipsoid*	newSuperellipsoid;
	
	newSuperellipsoid = (Superellipsoid*)_Stg_Shape_Copy( self, dest, deep, nameExt, ptrMap );
	
	newSuperellipsoid->epsilon1 = self->epsilon1;
	newSuperellipsoid->epsilon2 = self->epsilon2;
	
	memcpy( newSuperellipsoid->radius, self->radius, sizeof(XYZ));
	
	return (void*)newSuperellipsoid;
}

void* _Superellipsoid_DefaultNew( Name name ) {
	return (void*) _Superellipsoid_New(
			sizeof(Superellipsoid),
			Superellipsoid_Type,
			_Superellipsoid_Delete,
			_Superellipsoid_Print,
			_Superellipsoid_Copy,
			_Superellipsoid_DefaultNew,
			_Superellipsoid_Construct,
			_Superellipsoid_Build,
			_Superellipsoid_Initialise,
			_Superellipsoid_Execute,
			_Superellipsoid_Destroy,
			_Superellipsoid_IsCoordInside,
			_Superellipsoid_CalculateVolume,
			name );
}


void _Superellipsoid_Construct( void* superellipsoid, Stg_ComponentFactory* cf, void* data ) {
	Superellipsoid*	self      = (Superellipsoid*) superellipsoid;
	XYZ             radius;
	double          epsilon1;
	double          epsilon2;

	_Stg_Shape_Construct( self, cf, data );

	radius[ I_AXIS ] = Stg_ComponentFactory_GetDouble( cf, self->name, "radiusX", 1.0 );
	radius[ J_AXIS ] = Stg_ComponentFactory_GetDouble( cf, self->name, "radiusY", 1.0 );
	radius[ K_AXIS ] = Stg_ComponentFactory_GetDouble( cf, self->name, "radiusZ", 1.0 );

	epsilon1 = Stg_ComponentFactory_GetDouble( cf, self->name, "epsilon1", 1.0 );
	epsilon2 = Stg_ComponentFactory_GetDouble( cf, self->name, "epsilon2", 1.0 );

	_Superellipsoid_Init( self, epsilon1, epsilon2, radius );
}

void _Superellipsoid_Build( void* superellipsoid, void* data ) {
	Superellipsoid*	self = (Superellipsoid*)superellipsoid;

	_Stg_Shape_Build( self, data );
}
void _Superellipsoid_Initialise( void* superellipsoid, void* data ) {
	Superellipsoid*	self = (Superellipsoid*)superellipsoid;
	
	_Stg_Shape_Initialise( self, data );
}
void _Superellipsoid_Execute( void* superellipsoid, void* data ) {
	Superellipsoid*	self = (Superellipsoid*)superellipsoid;
	
	_Stg_Shape_Execute( self, data );
}
void _Superellipsoid_Destroy( void* superellipsoid, void* data ) {
	Superellipsoid*	self = (Superellipsoid*)superellipsoid;
	
	_Stg_Shape_Destroy( self, data );
}

/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/
	
/*---------------------------------------------------------------------------------------------------------------------
** Private Member functions
*/

Bool _Superellipsoid_IsCoordInside( void* superellipsoid, Coord coord ) {
	Superellipsoid* self              = (Superellipsoid*)superellipsoid;
	Coord           newCoord;
	double          insideOutsideValue;
	double          epsilon1          = self->epsilon1;
	double          epsilon2          = self->epsilon2;
	double          x, y, z;

	/* Transform coordinate into canonical reference frame */
	Stg_Shape_TransformCoord( self, coord, newCoord );
	
	x = newCoord[ I_AXIS ]/self->radius[ I_AXIS ];
	y = newCoord[ J_AXIS ]/self->radius[ J_AXIS ];
	z = newCoord[ K_AXIS ]/self->radius[ K_AXIS ];

	/* Evaluate Inside-Outside Function */
	if (self->dim == 2)
		insideOutsideValue = pow( x*x, 1.0/epsilon1 ) + pow( y*y, 1.0/epsilon1 );
	else 
		insideOutsideValue = pow( pow( x*x, 1.0/epsilon2 ) + pow( y*y, 1.0/epsilon2 ) , epsilon2/epsilon1 )
			+ pow( z*z, 1.0/epsilon1 );

	/* Return True if coord is inside and False otherwise */
	return ( insideOutsideValue <= 1.0 );
}

double _Superellipsoid_CalculateVolume( void* superellipsoid ) {
	assert( 0  );
	return 0.0;
}
	

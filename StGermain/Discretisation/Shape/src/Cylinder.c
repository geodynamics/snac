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
** $Id: Cylinder.c 3869 2006-10-16 13:42:59Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <Base/Base.h>
#include <Discretisation/Geometry/Geometry.h>

#include "types.h"
#include "ShapeClass.h"
#include "Cylinder.h"

#include <assert.h>
#include <string.h>
#include <math.h>


/* Textual name of this class */
const Type Cylinder_Type = "Cylinder";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/
Cylinder* Cylinder_New(
		Name                                  name,
		Dimension_Index                       dim,
		XYZ                                   centre, 
		double                                alpha,
		double                                beta,
		double                                gamma,
		double                                radius, 
		XYZ                                   start, 
		XYZ                                   end, 
		Axis                                  perpendicularAxis )
{
	Cylinder* self = (Cylinder*) _Cylinder_DefaultNew( name );

	Cylinder_InitAll( 
		self, 
		dim,
		centre,
		alpha,
		beta,
		gamma,
		radius,
		start,
		end,
		perpendicularAxis ) ;
	return self;
}

Cylinder* _Cylinder_New(
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
	Cylinder* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(Cylinder) );
	self = (Cylinder*)_Stg_Shape_New( 
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
	
	return self;
}

void _Cylinder_Init( Cylinder* self, double radius, XYZ start, XYZ end, Axis perpendicularAxis ) {
	memcpy( self->start, start, sizeof(XYZ));
	memcpy( self->end, end, sizeof(XYZ));
	self->perpendicularAxis = perpendicularAxis;
	self->radius = radius;
}


void Cylinder_InitAll( 
		void*                                 cylinder, 
		Dimension_Index                       dim, 
		Coord                                 centre,
		double                                alpha,
		double                                beta,
		double                                gamma,
		double                                radius, 
		XYZ                                   start, 
		XYZ                                   end, 
		Axis                                  perpendicularAxis )
{
	Cylinder* self = (Cylinder*)cylinder;

	Stg_Shape_InitAll( self, dim, centre, alpha, beta, gamma );
	_Cylinder_Init( self, radius, start, end, perpendicularAxis );
}
	

/*------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _Cylinder_Delete( void* cylinder ) {
	Cylinder* self = (Cylinder*)cylinder;
	
	/* Delete parent */
	_Stg_Shape_Delete( self );
}


void _Cylinder_Print( void* cylinder, Stream* stream ) {
	Cylinder* self = (Cylinder*)cylinder;
	
	/* Print parent */
	_Stg_Shape_Print( self, stream );
}



void* _Cylinder_Copy( void* cylinder, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	Cylinder*	self = (Cylinder*)cylinder;
	Cylinder*	newCylinder;
	
	newCylinder = (Cylinder*)_Stg_Shape_Copy( self, dest, deep, nameExt, ptrMap );
	
	memcpy( newCylinder->start, self->start, sizeof(XYZ));
	memcpy( newCylinder->end, self->end, sizeof(XYZ));

	newCylinder->radius = self->radius;
	newCylinder->perpendicularAxis = self->perpendicularAxis;
	
	return (void*)newCylinder;
}

void* _Cylinder_DefaultNew( Name name ) {
	return (void*) _Cylinder_New(
			sizeof(Cylinder),
			Cylinder_Type,
			_Cylinder_Delete,
			_Cylinder_Print,
			_Cylinder_Copy,
			_Cylinder_DefaultNew,
			_Cylinder_Construct,
			_Cylinder_Build,
			_Cylinder_Initialise,
			_Cylinder_Execute,
			_Cylinder_Destroy,
			_Cylinder_IsCoordInside,
			_Cylinder_CalculateVolume,
			name );
}

#define BIG 1.0e99

void _Cylinder_Construct( void* cylinder, Stg_ComponentFactory* cf, void* data ) {
	Cylinder*            self                     = (Cylinder*) cylinder;
	XYZ                  start                    = { -BIG, -BIG, -BIG }; 
	XYZ                  end                      = {  BIG,  BIG,  BIG };
	double               radius                   = 0.0;
	Axis                 perpendicularAxis        = I_AXIS;
	char*                perpendicularAxisName    = NULL;

	_Stg_Shape_Construct( self, cf, data );
	
	radius = Stg_ComponentFactory_GetDouble( cf, self->name, "radius", 0.0 );

	start[ I_AXIS ] = Stg_ComponentFactory_GetDouble( cf, self->name, "startX", -BIG );
	start[ J_AXIS ] = Stg_ComponentFactory_GetDouble( cf, self->name, "startY", -BIG );
	start[ K_AXIS ] = Stg_ComponentFactory_GetDouble( cf, self->name, "startZ", -BIG );
	end[ I_AXIS ] = Stg_ComponentFactory_GetDouble( cf, self->name, "endX", BIG );
	end[ J_AXIS ] = Stg_ComponentFactory_GetDouble( cf, self->name, "endY", BIG );
	end[ K_AXIS ] = Stg_ComponentFactory_GetDouble( cf, self->name, "endZ", BIG );

	perpendicularAxisName = Stg_ComponentFactory_GetString( cf, self->name, "perpendicularAxis", "x" );
	switch ( perpendicularAxisName[0] ) {
		case 'x': case 'X': case 'i': case 'I': case '0':
			perpendicularAxis = I_AXIS; break;
		case 'y': case 'Y': case 'j': case 'J': case '1':
			perpendicularAxis = J_AXIS; break;
		case 'z': case 'Z': case 'k': case 'K': case '2':
			perpendicularAxis = K_AXIS; break;
		default:
			Journal_Firewall( False, Journal_Register( Error_Type, self->type ),
					"Cannot understand perpendicularAxis '%s'\n", perpendicularAxisName );
	}
	
	_Cylinder_Init( self, radius, start, end, perpendicularAxis );
}

void _Cylinder_Build( void* cylinder, void* data ) {
	Cylinder*	self = (Cylinder*)cylinder;

	_Stg_Shape_Build( self, data );
}
void _Cylinder_Initialise( void* cylinder, void* data ) {
	Cylinder*	self = (Cylinder*)cylinder;
	
	_Stg_Shape_Initialise( self, data );
}
void _Cylinder_Execute( void* cylinder, void* data ) {
	Cylinder*	self = (Cylinder*)cylinder;
	
	_Stg_Shape_Execute( self, data );
}
void _Cylinder_Destroy( void* cylinder, void* data ) {
	Cylinder*	self = (Cylinder*)cylinder;
	
	_Stg_Shape_Destroy( self, data );
}

/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/
	
/*---------------------------------------------------------------------------------------------------------------------
** Private Member functions
*/

Bool _Cylinder_IsCoordInside( void* cylinder, Coord coord ) {
	Cylinder*       self       = (Cylinder*)cylinder;
	Coord           newCoord;
	double          insideOutsideValue;
	double          x, y, z;
	Axis            axis_I;

	/* Check whether coord is within min and max values */
	for ( axis_I = 0 ; axis_I < self->dim ; axis_I++ ) {
		if ( coord[ axis_I ] < self->start[ axis_I ] || coord[ axis_I ] > self->end[ axis_I ] )
			return False;
	}
	
	/* Transform coordinate into canonical reference frame */
	Stg_Shape_TransformCoord( self, coord, newCoord );
	
	newCoord[ self->perpendicularAxis ] = 0.0;

	/* Check if coord is within radius */
	x = newCoord[ I_AXIS ];
	y = newCoord[ J_AXIS ];
	if(self->dim == 2)
		insideOutsideValue = x*x + y*y;
	else {
		z = newCoord[ K_AXIS ];
		insideOutsideValue = x*x + y*y + z*z;
	}
	if ( insideOutsideValue > (self->radius * self->radius) )
		return False;


	return True;
}

double _Cylinder_CalculateVolume( void* cylinder ) {
	assert( 0 /* unsure how this cylinder is setup...but shouldn't be hard to implement -- Alan */ );
	return 0.0;
}


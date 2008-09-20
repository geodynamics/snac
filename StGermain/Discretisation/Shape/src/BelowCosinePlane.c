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
** $Id: BelowCosinePlane.c 3523 2006-04-11 06:42:09Z AlanLo $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <Base/Base.h>
#include <Discretisation/Geometry/Geometry.h>

#include "types.h"
#include "ShapeClass.h"
#include "BelowPlane.h"
#include "BelowCosinePlane.h"

#include <assert.h>
#include <string.h>
#include <math.h>


/* Textual name of this class */
const Type BelowCosinePlane_Type = "BelowCosinePlane";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/
BelowCosinePlane* BelowCosinePlane_New(
		Name                                  name,
		Dimension_Index                       dim,
		XYZ                                   centre, 
		double                                alpha,
		double                                beta,
		double                                gamma,
		double                                offset,
		XYZ                                   width,
		double                                amplitude,
		double                                period,
		double                                phase )
{
	BelowCosinePlane* self = (BelowCosinePlane*) _BelowCosinePlane_DefaultNew( name );

	BelowCosinePlane_InitAll( 
		self, 
		dim,
		centre,
		alpha,
		beta,
		gamma,
		offset,
		width,
		amplitude,
		period,
		phase ) ;
	return self;
}

BelowCosinePlane* _BelowCosinePlane_New(
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
	BelowCosinePlane* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(BelowCosinePlane) );
	self = (BelowCosinePlane*)_Stg_Shape_New( 
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

void _BelowCosinePlane_Init( void* belowPlane, double offset, XYZ width, double amplitude, double period, double phase ) {
	BelowCosinePlane* self = (BelowCosinePlane*)belowPlane;

	_BelowPlane_Init( self, offset, width );	
	self->amplitude = amplitude;
	self->period = period;
	self->phase = phase;
}


void BelowCosinePlane_InitAll( 
		void*                                 belowPlane, 
		Dimension_Index                       dim, 
		Coord                                 centre,
		double                                alpha,
		double                                beta,
		double                                gamma,
		double                                offset, 
		XYZ                                   width,
		double                                amplitude,
		double                                period,
		double                                phase )
{
	BelowCosinePlane* self = (BelowCosinePlane*)belowPlane;

	BelowPlane_InitAll( self, dim, centre, alpha, beta, gamma, offset, width );
	_BelowCosinePlane_Init( self, offset, width, amplitude, period, phase );
}
	

/*------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _BelowCosinePlane_Delete( void* belowPlane ) {
	BelowCosinePlane* self = (BelowCosinePlane*)belowPlane;
	
	/* Delete parent */
	_Stg_Shape_Delete( self );
}


void _BelowCosinePlane_Print( void* belowPlane, Stream* stream ) {
	BelowCosinePlane* self = (BelowCosinePlane*)belowPlane;
	
	/* Print parent */
	_Stg_Shape_Print( self, stream );
}



void* _BelowCosinePlane_Copy( void* belowPlane, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	BelowCosinePlane*	self = (BelowCosinePlane*)belowPlane;
	BelowCosinePlane*	newBelowCosinePlane;
	
	newBelowCosinePlane = (BelowCosinePlane*)_BelowPlane_Copy( self, dest, deep, nameExt, ptrMap );

	newBelowCosinePlane->amplitude = self->amplitude;
	newBelowCosinePlane->period = self->period;
	newBelowCosinePlane->phase = self->phase;
	
	return (void*)newBelowCosinePlane;
}

void* _BelowCosinePlane_DefaultNew( Name name ) {
	return (void*) _BelowCosinePlane_New(
			sizeof(BelowCosinePlane),
			BelowCosinePlane_Type,
			_BelowCosinePlane_Delete,
			_BelowCosinePlane_Print,
			_BelowCosinePlane_Copy,
			_BelowCosinePlane_DefaultNew,
			_BelowCosinePlane_Construct,
			_BelowCosinePlane_Build,
			_BelowCosinePlane_Initialise,
			_BelowCosinePlane_Execute,
			_BelowCosinePlane_Destroy,
			_BelowCosinePlane_IsCoordInside,
			_BelowCosinePlane_CalculateVolume,
			name );
}


void _BelowCosinePlane_Construct( void* belowPlane, Stg_ComponentFactory* cf, void* data ) {
	BelowCosinePlane*            self          = (BelowCosinePlane*) belowPlane;
	double                       amplitude;
	double                       period;
	double                       phase;

	_BelowPlane_Construct( self, cf, data );

	amplitude = Stg_ComponentFactory_GetDouble( cf, self->name, "amplitude", 0.1 );
	period = Stg_ComponentFactory_GetDouble( cf, self->name, "period", 1.0 );
	phase = Stg_ComponentFactory_GetDouble( cf, self->name, "phase", 0.0 );

	_BelowCosinePlane_Init( self, self->offset, self->width, amplitude, period, phase );
}

void _BelowCosinePlane_Build( void* belowPlane, void* data ) {
	BelowCosinePlane*	self = (BelowCosinePlane*)belowPlane;

	_Stg_Shape_Build( self, data );
}
void _BelowCosinePlane_Initialise( void* belowPlane, void* data ) {
	BelowCosinePlane*	self = (BelowCosinePlane*)belowPlane;
	
	_Stg_Shape_Initialise( self, data );
}
void _BelowCosinePlane_Execute( void* belowPlane, void* data ) {
	BelowCosinePlane*	self = (BelowCosinePlane*)belowPlane;
	
	_Stg_Shape_Execute( self, data );
}
void _BelowCosinePlane_Destroy( void* belowPlane, void* data ) {
	BelowCosinePlane*	self = (BelowCosinePlane*)belowPlane;
	
	_Stg_Shape_Destroy( self, data );
}

/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/
	
/*---------------------------------------------------------------------------------------------------------------------
** Private Member functions
*/

Bool _BelowCosinePlane_IsCoordInside( void* belowPlane, Coord coord ) {
	BelowCosinePlane*            self       = (BelowCosinePlane*)belowPlane;
	Coord                        newCoord;

	double                       x, y;

	/* Transform coordinate into canonical reference frame */
	Stg_Shape_TransformCoord( self, coord, newCoord );

	x = newCoord[ I_AXIS ];

	y =  self->offset + self->amplitude * cos( (self->period * M_PI * x) + self->phase );

	if ( fabs( newCoord[ J_AXIS ] < y) ) {
		return True;
	}
	return False;
}

double _BelowCosinePlane_CalculateVolume( void* belowPlane ) {
	BelowCosinePlane* self = (BelowCosinePlane*)belowPlane;
	double volume;
	double dx = self->width[ I_AXIS ];

	volume = self->offset*dx + ( self->amplitude/self->period )* sin( dx*self->period + self->phase );

	if ( self->dim == 3 ) 
		volume = self->width[ K_AXIS ] * volume;
	
	return volume;
}


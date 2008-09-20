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
** $Id: BelowPlane.c 3523 2006-04-11 06:42:09Z AlanLo $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <Base/Base.h>
#include <Discretisation/Geometry/Geometry.h>

#include "types.h"
#include "ShapeClass.h"
#include "BelowPlane.h"

#include <assert.h>
#include <string.h>
#include <math.h>


/* Textual name of this class */
const Type BelowPlane_Type = "BelowPlane";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/
BelowPlane* BelowPlane_New(
		Name                                  name,
		Dimension_Index                       dim,
		XYZ                                   centre, 
		double                                alpha,
		double                                beta,
		double                                gamma,
		double                                offset,
		XYZ                                   width )
{
	BelowPlane* self = (BelowPlane*) _BelowPlane_DefaultNew( name );

	BelowPlane_InitAll( 
		self, 
		dim,
		centre,
		alpha,
		beta,
		gamma,
		offset,
		width ) ;
	return self;
}

BelowPlane* _BelowPlane_New(
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
	BelowPlane* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(BelowPlane) );
	self = (BelowPlane*)_Stg_Shape_New( 
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

void _BelowPlane_Init( void* belowPlane, double offset, XYZ width ) {
	BelowPlane* self = (BelowPlane*)belowPlane;

	self->offset = offset;

	memcpy( self->width, width, sizeof(XYZ) );
}


void BelowPlane_InitAll( 
		void*                                 belowPlane, 
		Dimension_Index                       dim, 
		Coord                                 centre,
		double                                alpha,
		double                                beta,
		double                                gamma,
		double                                offest,
		XYZ                                   width )
{
	BelowPlane* self = (BelowPlane*)belowPlane;

	Stg_Shape_InitAll( self, dim, centre, alpha, beta, gamma );
	_BelowPlane_Init( self, offest, width );
}
	

/*------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _BelowPlane_Delete( void* belowPlane ) {
	BelowPlane* self = (BelowPlane*)belowPlane;
	
	/* Delete parent */
	_Stg_Shape_Delete( self );
}


void _BelowPlane_Print( void* belowPlane, Stream* stream ) {
	BelowPlane* self = (BelowPlane*)belowPlane;
	
	/* Print parent */
	_Stg_Shape_Print( self, stream );
}



void* _BelowPlane_Copy( void* belowPlane, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	BelowPlane*	self = (BelowPlane*)belowPlane;
	BelowPlane*	newBelowPlane;
	
	newBelowPlane = (BelowPlane*)_Stg_Shape_Copy( self, dest, deep, nameExt, ptrMap );

	newBelowPlane->offset = self->offset;
	
	return (void*)newBelowPlane;
}

void* _BelowPlane_DefaultNew( Name name ) {
	return (void*) _BelowPlane_New(
			sizeof(BelowPlane),
			BelowPlane_Type,
			_BelowPlane_Delete,
			_BelowPlane_Print,
			_BelowPlane_Copy,
			_BelowPlane_DefaultNew,
			_BelowPlane_Construct,
			_BelowPlane_Build,
			_BelowPlane_Initialise,
			_BelowPlane_Execute,
			_BelowPlane_Destroy,
			_BelowPlane_IsCoordInside,
			_BelowPlane_CalculateVolume,
			name );
}


void _BelowPlane_Construct( void* belowPlane, Stg_ComponentFactory* cf, void* data ) {
	BelowPlane*	             self          = (BelowPlane*) belowPlane;
	double                       offset;
	XYZ                          minValue;
	XYZ                          maxValue;
	XYZ                          width;

	_Stg_Shape_Construct( self, cf, data );

	offset = Stg_ComponentFactory_GetDouble( cf, self->name, "offset", 0.5 );

	minValue[ I_AXIS ] = Stg_ComponentFactory_GetRootDictDouble( cf, "minX", 0.0 );
	minValue[ J_AXIS ] = Stg_ComponentFactory_GetRootDictDouble( cf, "minY", 0.0 );
	minValue[ K_AXIS ] = Stg_ComponentFactory_GetRootDictDouble( cf, "minZ", 0.0 );

	maxValue[ I_AXIS ] = Stg_ComponentFactory_GetRootDictDouble( cf, "maxX", 1.0 );
	maxValue[ J_AXIS ] = Stg_ComponentFactory_GetRootDictDouble( cf, "maxY", 1.0 );
	maxValue[ K_AXIS ] = Stg_ComponentFactory_GetRootDictDouble( cf, "maxZ", 1.0 );

	width[ I_AXIS ] = maxValue[ I_AXIS ] - minValue[ I_AXIS ] ;
	width[ J_AXIS ] = maxValue[ J_AXIS ] - minValue[ J_AXIS ] ;
	width[ K_AXIS ] = maxValue[ K_AXIS ] - minValue[ K_AXIS ] ;

	_BelowPlane_Init( self, offset, width );
}

void _BelowPlane_Build( void* belowPlane, void* data ) {
	BelowPlane*	self = (BelowPlane*)belowPlane;

	_Stg_Shape_Build( self, data );
}
void _BelowPlane_Initialise( void* belowPlane, void* data ) {
	BelowPlane*	self = (BelowPlane*)belowPlane;
	
	_Stg_Shape_Initialise( self, data );
}
void _BelowPlane_Execute( void* belowPlane, void* data ) {
	BelowPlane*	self = (BelowPlane*)belowPlane;
	
	_Stg_Shape_Execute( self, data );
}
void _BelowPlane_Destroy( void* belowPlane, void* data ) {
	BelowPlane*	self = (BelowPlane*)belowPlane;
	
	_Stg_Shape_Destroy( self, data );
}

/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/
	
/*---------------------------------------------------------------------------------------------------------------------
** Private Member functions
*/

Bool _BelowPlane_IsCoordInside( void* belowPlane, Coord coord ) {
	BelowPlane*            self       = (BelowPlane*)belowPlane;
	Coord           newCoord;

	/* Transform coordinate into canonical reference frame */
	Stg_Shape_TransformCoord( self, coord, newCoord );

	if ( fabs( newCoord[ J_AXIS ] < self->offset ) ) {
		return True;
	}
	return False;
}

double _BelowPlane_CalculateVolume( void* belowPlane ) {
	BelowPlane* self = (BelowPlane*)belowPlane;
	double volume;

	if ( self->dim == 2 ) {
		volume = self->width[ I_AXIS ] * self->offset;
	}
	else {
		volume = self->width[ I_AXIS ] * self->width[ K_AXIS ] * self->offset;
	}

	return volume;
}


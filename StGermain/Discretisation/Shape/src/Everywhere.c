/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003, Victorian Partnership for Advanced Computing (VPAC) Ltd, 110 Victoria Street, Melbourne, 3053, Australia.
**
** Authors:
**	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
**	Patrick D. Sunter, Software Engineer, VPAC. (pds@vpac.org)
**	Luke J. Hodkinson, Computational Engineer, VPAC. (lhodkins@vpac.org)
**	Siew-Ching Tan, Software Engineer, VPAC. (siew@vpac.org) ) {
	IrregTopology* self = (IrregTopology*)ir
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
** $Id: Everywhere.c 3869 2006-10-16 13:42:59Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <Base/Base.h>
#include <Discretisation/Geometry/Geometry.h>

#include "types.h"
#include "ShapeClass.h"
#include "Everywhere.h"

#include <assert.h>
#include <string.h>
#include <math.h>

/* Textual name of this class */
const Type Everywhere_Type = "Everywhere";

/*-------------------------------------------------------------------------------------------------------------------------
** Constructors
*/
Everywhere* Everywhere_New(
		Name                                  name,
		Dimension_Index                       dim )
{
	Everywhere* self = (Everywhere*) _Everywhere_DefaultNew( name );
	XYZ         centre = { 0.0,0.0,0.0 };

	Everywhere_InitAll( 
		self, 
		dim,
		centre,
		0.0,
		0.0,
		0.0 );
	return self;
}

Everywhere* _Everywhere_New(
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
	Everywhere* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(Everywhere) );
	self = (Everywhere*)_Stg_Shape_New( 
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

void _Everywhere_Init( void* everywhere ) {
}


void Everywhere_InitAll( 
		void*                                 everywhere, 
		Dimension_Index                       dim, 
		Coord                                 centre,
		double                                alpha,
		double                                beta,
		double                                gamma )
{
	Everywhere* self = (Everywhere*)everywhere;

	Stg_Shape_InitAll( self, dim, centre, alpha, beta, gamma );
	_Everywhere_Init( self );
}
	

/*------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _Everywhere_Delete( void* everywhere ) {
	Everywhere* self = (Everywhere*)everywhere;
	
	/* Delete parent */
	_Stg_Shape_Delete( self );
}

void _Everywhere_Print( void* everywhere, Stream* stream ) {
	Everywhere* self = (Everywhere*)everywhere;
	
	/* Print parent */
	_Stg_Shape_Print( self, stream );
}

void* _Everywhere_Copy( void* everywhere, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	Everywhere*	self = (Everywhere*)everywhere;
	Everywhere*	newEverywhere;
	
	newEverywhere = (Everywhere*)_Stg_Shape_Copy( self, dest, deep, nameExt, ptrMap );
	
	return (void*)newEverywhere;
}

void* _Everywhere_DefaultNew( Name name ) {
	return (void*) _Everywhere_New(
			sizeof(Everywhere),
			Everywhere_Type,
			_Everywhere_Delete,
			_Everywhere_Print,
			_Everywhere_Copy,
			_Everywhere_DefaultNew,
			_Everywhere_Construct,
			_Everywhere_Build,
			_Everywhere_Initialise,
			_Everywhere_Execute,
			_Everywhere_Destroy,
			_Everywhere_IsCoordInside,
			_Everywhere_CalculateVolume,
			name );
}


void _Everywhere_Construct( void* everywhere, Stg_ComponentFactory* cf, void* data ) {
	Everywhere*	self          = (Everywhere*) everywhere;

	_Stg_Shape_Construct( self, cf, data );
	_Everywhere_Init( self );
}

void _Everywhere_Build( void* everywhere, void* data ) {
	Everywhere*	self = (Everywhere*)everywhere;
	_Stg_Shape_Build( self, data );
}
void _Everywhere_Initialise( void* everywhere, void* data ) {
	Everywhere*	self = (Everywhere*)everywhere;
	_Stg_Shape_Initialise( self, data );
}
void _Everywhere_Execute( void* everywhere, void* data ) {
	Everywhere*	self = (Everywhere*)everywhere;
	_Stg_Shape_Execute( self, data );
}
void _Everywhere_Destroy( void* everywhere, void* data ) {
	Everywhere*	self = (Everywhere*)everywhere;
	_Stg_Shape_Destroy( self, data );
}

/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/
	
/*---------------------------------------------------------------------------------------------------------------------
** Private Member functions
*/

Bool _Everywhere_IsCoordInside( void* everywhere, Coord coord ) {
	return True;
}

double _Everywhere_CalculateVolume( void* everywhere ) {
	return 1.0;
}	

/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/


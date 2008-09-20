/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003, Victorian Partnership for Advanced Computing (VPAC) Ltd, 110 Victoria Street, Melbourne, 3053, Australia.
**
** Authors:
**	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
**	Patrick D. Sunter, Software Engineer, VPAC. (pds@vpac.org)
**	Luke J. Hodkinson, Computational Engineer, VPAC. (lhodkins@vpac.org)
**	Siew-Ching Tan, Software Engineer, VPAC. (siew@vpac.org) ) {
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
** $Id: ShapeAdvector.c 212 2005-11-08 23:50:02Z RobertTurnbull $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"
#include "Discretisation/Mesh/Mesh.h"

#include "types.h"
#include "ShapeAdvector.h"
#include "TimeIntegrator.h"
#include "TimeIntegratee.h"
#include "FieldVariable.h"

#include <assert.h>
#include <string.h>
#include <math.h>


/* Textual name of this class */
const Type ShapeAdvector_Type = "ShapeAdvector";

/*-------------------------------------------------------------------------------------------------------------------------
** Constructors
*/
ShapeAdvector* ShapeAdvector_New(
		Name                                       name,
		TimeIntegrator*                            timeIntegrator,
		FieldVariable*                             velocityField,
		Stg_Shape*                                 shape,
		Bool                                       allowFallbackToFirstOrder )
{
	ShapeAdvector* self = (ShapeAdvector*) _ShapeAdvector_DefaultNew( name );

	/* 	ShapeAdvector_InitAll */
	_ShapeAdvector_Init( self, timeIntegrator, velocityField, shape, allowFallbackToFirstOrder );

	return self;
}

ShapeAdvector* _ShapeAdvector_New(
		SizeT                                      _sizeOfSelf, 
		Type                                       type,
		Stg_Class_DeleteFunction*                  _delete,
		Stg_Class_PrintFunction*                   _print,
		Stg_Class_CopyFunction*                    _copy, 
		Stg_Component_DefaultConstructorFunction*  _defaultConstructor,
		Stg_Component_ConstructFunction*           _construct,
		Stg_Component_BuildFunction*               _build,
		Stg_Component_InitialiseFunction*          _initialise,
		Stg_Component_ExecuteFunction*             _execute,
		Stg_Component_DestroyFunction*             _destroy,		
		Name                                       name )
{
	ShapeAdvector* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(ShapeAdvector) );
	self = (ShapeAdvector*)_Stg_Component_New( 
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
			name,
			NON_GLOBAL );
	
	/* General info */

	/* Virtual Info */
	
	return self;
}

void _ShapeAdvector_Init( 
		ShapeAdvector*                             self,
		TimeIntegrator*                            timeIntegrator,
		FieldVariable*                             velocityField,
		Stg_Shape*                                 shape,
		Bool                                       allowFallbackToFirstOrder )
{
	self->velocityField = velocityField;
	self->shape = shape;
	self->shapeCount = 1;
	self->shapeCentrePtr = shape->centre;

	self->shapeCentreVariable = 
		Variable_NewVector( "shapeCentreVariable", Variable_DataType_Double, shape->dim, &self->shapeCount, &self->shapeCentrePtr, NULL );
	self->timeIntegratee = 
		TimeIntegratee_New( "shapeTimeIntegratee", timeIntegrator, self->shapeCentreVariable, 1,
		(Stg_Component**) &velocityField, allowFallbackToFirstOrder );
}


/*------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _ShapeAdvector_Delete( void* shapeAdvector ) {
	ShapeAdvector* self = (ShapeAdvector*)shapeAdvector;

	Stg_Class_Delete( self->shapeCentreVariable );
	Stg_Class_Delete( self->timeIntegratee );

	/* Delete parent */
	_Stg_Component_Delete( self );
}


void _ShapeAdvector_Print( void* shapeAdvector, Stream* stream ) {
	ShapeAdvector* self = (ShapeAdvector*)shapeAdvector;
	
	/* Print parent */
	_Stg_Component_Print( self, stream );
}


void* _ShapeAdvector_Copy( void* shapeAdvector, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	ShapeAdvector*	self = (ShapeAdvector*)shapeAdvector;
	ShapeAdvector*	newShapeAdvector;
	
	newShapeAdvector = (ShapeAdvector*)_Stg_Component_Copy( self, dest, deep, nameExt, ptrMap );

	newShapeAdvector->velocityField = self->velocityField;
	newShapeAdvector->shape         = self->shape;
	
	return (void*)newShapeAdvector;
}

void* _ShapeAdvector_DefaultNew( Name name ) {
	return (void*) _ShapeAdvector_New(
			sizeof(ShapeAdvector),
			ShapeAdvector_Type,
			_ShapeAdvector_Delete,
			_ShapeAdvector_Print,
			_ShapeAdvector_Copy,
			_ShapeAdvector_DefaultNew,
			_ShapeAdvector_Construct,
			_ShapeAdvector_Build,
			_ShapeAdvector_Initialise,
			_ShapeAdvector_Execute,
			_ShapeAdvector_Destroy,
			name );
}


void _ShapeAdvector_Construct( void* shapeAdvector, Stg_ComponentFactory* cf, void* data ) {
	ShapeAdvector*	            self          = (ShapeAdvector*) shapeAdvector;
	FieldVariable*              velocityField;
	Stg_Shape*                  shape;
	TimeIntegrator*             timeIntegrator;
	Bool                        allowFallbackToFirstOrder = False;

	timeIntegrator = Stg_ComponentFactory_ConstructByKey( cf, self->name, "TimeIntegrator", TimeIntegrator, True, data  ) ;
	velocityField  = Stg_ComponentFactory_ConstructByKey( cf, self->name, "VelocityField", FieldVariable, True, data  ) ;
	shape          = Stg_ComponentFactory_ConstructByKey( cf, self->name, "Shape", Stg_Shape, True, data ) ;
	allowFallbackToFirstOrder = Stg_ComponentFactory_GetBool( cf, self->name, "allowFallbackToFirstOrder", False );

	_ShapeAdvector_Init( self, timeIntegrator, velocityField, shape, allowFallbackToFirstOrder );
}

void _ShapeAdvector_Build( void* shapeAdvector, void* data ) {
}
void _ShapeAdvector_Initialise( void* shapeAdvector, void* data ) {
}
void _ShapeAdvector_Execute( void* shapeAdvector, void* data ) {
}
void _ShapeAdvector_Destroy( void* shapeAdvector, void* data ) {
}



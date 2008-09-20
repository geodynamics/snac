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
** $Id: PythonShape.c 3869 2006-10-16 13:42:59Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifdef HAVE_PYTHON
#include <Python.h>

#include <mpi.h>
#include <Base/Base.h>
#include <Discretisation/Geometry/Geometry.h>


#include "types.h"
#include "ShapeClass.h"
#include "PythonShape.h"

#include <assert.h>
#include <string.h>
#include <math.h>


/* Textual name of this class */
const Type PythonShape_Type = "PythonShape";

char* PYTHONSHAPE_TEST_CONDITION_NAME = "testCondition";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/
PythonShape* PythonShape_New(
		Name                                  name,
		Dimension_Index                       dim,
		XYZ                                   centre, 
		double                                alpha,
		double                                beta,
		double                                gamma,
		char*                                 conditionFunction )
{
	PythonShape* self = (PythonShape*) _PythonShape_DefaultNew( name );

	PythonShape_InitAll( 
		self, 
		dim,
		centre,
		alpha,
		beta,
		gamma,
		conditionFunction );
	return self;
}

PythonShape* _PythonShape_New(
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
	PythonShape* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(PythonShape) );
	self = (PythonShape*)_Stg_Shape_New( 
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
	
	return self;
}

void _PythonShape_Init( void* pythonShape, char* conditionFunction ) {
	PythonShape* self = (PythonShape*)pythonShape;
	
	Stg_asprintf( &self->testCondition, "%s = %s", PYTHONSHAPE_TEST_CONDITION_NAME, conditionFunction );
	
	/* Python Stuff */
	self->pythonDictionary = PyModule_GetDict(PyImport_AddModule("__main__"));
	PyRun_SimpleString( "from math import *" );
}


void PythonShape_InitAll( 
		void*                                 pythonShape, 
		Dimension_Index                       dim, 
		Coord                                 centre,
		double                                alpha,
		double                                beta,
		double                                gamma,
		char*                                 conditionFunction )
{
	PythonShape* self = (PythonShape*)pythonShape;

	Stg_Shape_InitAll( self, dim, centre, alpha, beta, gamma );
	_PythonShape_Init( self, conditionFunction );
}
	

/*------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _PythonShape_Delete( void* pythonShape ) {
	PythonShape* self = (PythonShape*)pythonShape;
	
	/* Delete parent */
	_Stg_Shape_Delete( self );

	Memory_Free( self->testCondition );
}


void _PythonShape_Print( void* pythonShape, Stream* stream ) {
	PythonShape* self = (PythonShape*)pythonShape;
	
	/* Print parent */
	_Stg_Shape_Print( self, stream );
}



void* _PythonShape_Copy( void* pythonShape, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	PythonShape*	self = (PythonShape*)pythonShape;
	PythonShape*	newPythonShape;
	
	newPythonShape = (PythonShape*)_Stg_Shape_Copy( self, dest, deep, nameExt, ptrMap );
	abort();
	
	return (void*)newPythonShape;
}

void* _PythonShape_DefaultNew( Name name ) {
	return (void*) _PythonShape_New(
			sizeof(PythonShape),
			PythonShape_Type,
			_PythonShape_Delete,
			_PythonShape_Print,
			_PythonShape_Copy,
			_PythonShape_DefaultNew,
			_PythonShape_Construct,
			_PythonShape_Build,
			_PythonShape_Initialise,
			_PythonShape_Execute,
			_PythonShape_Destroy,
			_PythonShape_IsCoordInside,
			_PythonShape_CalculateVolume,
			name );
}


void _PythonShape_Construct( void* pythonShape, Stg_ComponentFactory* cf, void* data ) {
	PythonShape*	self      = (PythonShape*) pythonShape;
	char*           conditionFunction;

	_Stg_Shape_Construct( self, cf, data );

	conditionFunction = Stg_ComponentFactory_GetString( cf, self->name, "Function", "0" );

	_PythonShape_Init( self, conditionFunction );
}

void _PythonShape_Build( void* pythonShape, void* data ) {
	PythonShape*	self = (PythonShape*)pythonShape;

	_Stg_Shape_Build( self, data );
}
void _PythonShape_Initialise( void* pythonShape, void* data ) {
	PythonShape*	self = (PythonShape*)pythonShape;
	
	_Stg_Shape_Initialise( self, data );
}
void _PythonShape_Execute( void* pythonShape, void* data ) {
	PythonShape*	self = (PythonShape*)pythonShape;
	
	_Stg_Shape_Execute( self, data );
}
void _PythonShape_Destroy( void* pythonShape, void* data ) {
	PythonShape*	self = (PythonShape*)pythonShape;
	
	_Stg_Shape_Destroy( self, data );
}

/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/
	
/*---------------------------------------------------------------------------------------------------------------------
** Private Member functions
*/

Bool _PythonShape_IsCoordInside( void* pythonShape, Coord coord ) {
	PythonShape*          self                   = (PythonShape*)pythonShape;
	Coord                 newCoord;
	PyObject*             xCoord;
	PyObject*             yCoord;
	PyObject*             zCoord;
	PyObject*             testConditionValue;
    PyObject*             pythonDictionary       = (PyObject*) self->pythonDictionary;

	/* Transform coordinate into canonical reference frame */
	Stg_Shape_TransformCoord( self, coord, newCoord );
	
	/* Add values to dictionary */
	xCoord = PyFloat_FromDouble( newCoord[ I_AXIS ] );
	PyMapping_SetItemString( self->pythonDictionary, "x", xCoord );
	yCoord = PyFloat_FromDouble( newCoord[ J_AXIS ] );
	PyMapping_SetItemString( self->pythonDictionary, "y", yCoord );
	zCoord = PyFloat_FromDouble( newCoord[ K_AXIS ] );
	PyMapping_SetItemString( self->pythonDictionary, "z", zCoord );

	/* Evaluate Test Condition Using Python */
	PyRun_SimpleString(self->testCondition);

	/* Get Value Out of Python */
	testConditionValue = PyMapping_GetItemString(pythonDictionary, PYTHONSHAPE_TEST_CONDITION_NAME);
	Journal_Firewall( 
			testConditionValue != NULL, 
			Journal_Register( Error_Type, self->type), 
			"In func %s: Cannot Evalutate Function '%s'.\n", __func__, self->testCondition );

	return PyObject_IsTrue( testConditionValue );
}

double _PythonShape_CalculateVolume( void* pythonShape ) {
	assert( 0 );
	return 0.0;
}

#endif /* HAVE_PYTHON */

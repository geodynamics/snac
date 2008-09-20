/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003, Victorian Partnership for Advanced Computing (VPAC) Ltd, 110 Victoria Street, Melbourne, 3053, Australia.
**
** Authors:
**	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
**	David May, PhD Student Monash University, VPAC. (davidm@vpac.org)
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
** $Id: TimeIntegratee.c 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"
#include "Discretisation/Mesh/Mesh.h"

#include "types.h"
#include "TimeIntegrator.h"
#include "TimeIntegratee.h"
#include "FieldVariable.h"

#include <assert.h>
#include <string.h>

/* Textual name of this class */
const Type TimeIntegratee_Type = "TimeIntegratee";

TimeIntegratee* TimeIntegratee_New( 
		Name                                       name,
		TimeIntegrator*                            timeIntegrator, 
		Variable*                                  variable,
		Index                                      dataCount, 
		Stg_Component**                            data,
		Bool                                       allowFallbackToFirstOrder )
{
	TimeIntegratee*	self;

	self = (TimeIntegratee*) _TimeIntegratee_DefaultNew( name );
	_TimeIntegratee_Init( self, timeIntegrator, variable, dataCount, data, allowFallbackToFirstOrder );
	return self;
}

TimeIntegratee* _TimeIntegratee_New( 
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
		TimeIntegratee_CalculateTimeDerivFunction* _calculateTimeDeriv,
		TimeIntegratee_IntermediateFunction*       _intermediate,
		Name                                       name )
{
	TimeIntegratee*	self;
	
	assert( _sizeOfSelf >= sizeof(TimeIntegratee) );
	
	/* General info */
	self = (TimeIntegratee*)_Stg_Component_New( 
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

	/* virtual info */
	self->_calculateTimeDeriv = _calculateTimeDeriv;
	self->_intermediate = _intermediate;
	
	return self;
}

void _TimeIntegratee_Init( 
		void*                                      timeIntegratee, 
		TimeIntegrator*                            timeIntegrator, 
		Variable*                                  variable, 
		Index                                      dataCount, 
		Stg_Component**                            data,
		Bool                                       allowFallbackToFirstOrder )
{
	TimeIntegratee* self = (TimeIntegratee*)timeIntegratee;
	
	self->debug          = Journal_Register( Debug_Type, self->type );
	self->variable       = variable;
	self->dataCount      = dataCount;
	self->timeIntegrator = timeIntegrator;
	self->data           = Memory_Alloc_Array( Stg_Component*, dataCount, "data" );
	self->allowFallbackToFirstOrder = allowFallbackToFirstOrder;
	memcpy( self->data, data, dataCount * sizeof(Stg_Component*) );

	TimeIntegrator_Add( timeIntegrator, self );
}

void _TimeIntegratee_Delete( void* timeIntegratee ) {
	TimeIntegratee* self = (TimeIntegratee*)timeIntegratee;
	
	Journal_DPrintf( self->debug, "In %s for %s '%s'\n", __func__, self->type, self->name );
	
	Memory_Free( self->data );

	/* Stg_Class_Delete parent*/
	_Stg_Component_Delete( self );
}

void _TimeIntegratee_Print( void* timeIntegratee, Stream* stream ) {
	TimeIntegratee* self = (TimeIntegratee*)timeIntegratee;

	/* General info */
	Journal_DPrintf( self->debug, "TimeIntegratee - '%s'\n", self->name );
	Journal_PrintPointer( stream, self );
	Stream_Indent( stream );
	
	/* Print parent */
	_Stg_Component_Print( self, stream );
	
	/* Virtual info */

	/* Regular Info */
	
	Stream_UnIndent( stream );
}

void* _TimeIntegratee_Copy( void* timeIntegratee, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	TimeIntegratee*	self = (TimeIntegratee*)timeIntegratee;
	TimeIntegratee*	newTimeIntegratee;
	
	Journal_DPrintf( self->debug, "In %s for %s '%s'\n", __func__, self->type, self->name );
	/* TODO */ abort();

	return (void*)newTimeIntegratee;
}

void* _TimeIntegratee_DefaultNew( Name name ) {
	return (void*) _TimeIntegratee_New( 
		sizeof(TimeIntegratee), 
		TimeIntegratee_Type, 
		_TimeIntegratee_Delete,
		_TimeIntegratee_Print, 
		_TimeIntegratee_Copy, 
		_TimeIntegratee_DefaultNew, 
		_TimeIntegratee_Construct, 
		_TimeIntegratee_Build, 
		_TimeIntegratee_Initialise,
		_TimeIntegratee_Execute,
		_TimeIntegratee_Destroy, 
		_TimeIntegratee_AdvectionTimeDeriv,
		_TimeIntegratee_Intermediate,
		name );
}

void _TimeIntegratee_Construct( void* timeIntegratee, Stg_ComponentFactory* cf, void* data ) {
	TimeIntegratee*         self                    = (TimeIntegratee*)timeIntegratee;
	Index                   dataCount               = 0;
	Stg_Component**         initData                = NULL;
	Variable*               variable                = NULL;
	TimeIntegrator*         timeIntegrator          = NULL;
	Bool                    allowFallbackToFirstOrder = False;
	
	variable       =  Stg_ComponentFactory_ConstructByKey( cf, self->name, Variable_Type,       Variable,       False, data ) ;
	timeIntegrator =  Stg_ComponentFactory_ConstructByKey( cf, self->name, TimeIntegrator_Type, TimeIntegrator, True, data ) ;
	initData = Stg_ComponentFactory_ConstructByList( 
		cf, 
		self->name, 
		"data", 
		Stg_ComponentFactory_Unlimited, 
		Stg_Component, 
		False, 
		&dataCount, 
		data );
	allowFallbackToFirstOrder = Stg_ComponentFactory_GetBool( cf, self->name, "allowFallbackToFirstOrder", False );	

	_TimeIntegratee_Init( self, timeIntegrator, variable, dataCount, initData, allowFallbackToFirstOrder );

	if( initData != NULL )
		Memory_Free( initData );
}

void _TimeIntegratee_Build( void* timeIntegratee, void* data ) {
	TimeIntegratee* self = (TimeIntegratee*)timeIntegratee;

	Journal_DPrintf( self->debug, "In %s for %s '%s'\n", __func__, self->type, self->name );

	Build( self->variable, NULL, False );
}

void _TimeIntegratee_Initialise( void* timeIntegratee, void* data ) {
	TimeIntegratee* self = (TimeIntegratee*)timeIntegratee;
	
	Journal_DPrintf( self->debug, "In %s for %s '%s'\n", __func__, self->type, self->name );
	Initialise( self->variable, NULL, False );
}

void _TimeIntegratee_Execute( void* timeIntegratee, void* data ) {
	TimeIntegratee*	self = (TimeIntegratee*)timeIntegratee;

	Journal_DPrintf( self->debug, "In %s for %s '%s'\n", __func__, self->type, self->name );
}
	
void _TimeIntegratee_Destroy( void* timeIntegratee, void* data ) {
}

/* +++ Virtual Functions +++ */
void TimeIntegratee_FirstOrder( void* timeIntegratee, Variable* startValue, double dt ) {
	TimeIntegratee*	self           = (TimeIntegratee*)timeIntegratee;
	Variable*       variable       = self->variable;
	double*         arrayDataPtr;
	double*         startDataPtr;
	double**        timeDeriv;
	Index           component_I; 
	Index           componentCount = *variable->dataTypeCounts;
	Index           array_I; 
	Index           arrayCount;
	Bool            successFlag = False;
	Stream*         errorStream = Journal_Register( Error_Type, self->type );

	Journal_DPrintf( self->debug, "In func %s for %s '%s'\n", __func__, self->type, self->name );

	/* Update Variables */
	Variable_Update( variable );
	Variable_Update( startValue );
	arrayCount     = variable->arraySize;

	timeDeriv = Memory_Alloc_2DArray( double, arrayCount, componentCount, "Time Deriv" );
	for( array_I = 0; array_I < arrayCount; array_I++ ) {
		successFlag = TimeIntegratee_CalculateTimeDeriv( self, array_I, timeDeriv[array_I] );
		Journal_Firewall( True == successFlag, errorStream,
			"Error - in %s(), for TimeIntegratee \"%s\" of type %s: When trying to find time "
			"deriv for item %u in step %u, *failed*.\n",
			__func__, self->name, self->type, array_I, 1 );
	}

	for ( array_I = 0 ; array_I < arrayCount ; array_I++ ) {
		arrayDataPtr = Variable_GetPtrDouble( variable, array_I );
		startDataPtr = Variable_GetPtrDouble( startValue, array_I );
		
		for ( component_I = 0 ; component_I < componentCount ; component_I++ ) {
			arrayDataPtr[ component_I ] = startDataPtr[ component_I ] + dt * timeDeriv[array_I][ component_I ];
		}
	
		TimeIntegratee_Intermediate( self, array_I );
	}

	Memory_Free( timeDeriv );
}

void TimeIntegratee_SecondOrder( void* timeIntegratee, Variable* startValue, double dt ) {
	TimeIntegratee*	self           = (TimeIntegratee*)timeIntegratee;
	Variable*       variable       = self->variable;
	double*         arrayDataPtr;
	double*         startDataPtr;
	double*         timeDeriv;
	double*         startData;
	Index           component_I; 
	Index           componentCount = *variable->dataTypeCounts;
	Index           array_I; 
	Index           arrayCount;
	double          startTime      = TimeIntegrator_GetTime( self->timeIntegrator );
	Bool            successFlag = False;
	Stream*         errorStream = Journal_Register( Error_Type, self->type );

	timeDeriv = Memory_Alloc_Array( double, componentCount, "Time Deriv" );
	startData = Memory_Alloc_Array( double, componentCount, "StartData" );
	memset( timeDeriv, 0, componentCount * sizeof( double ) );
	memset( startData, 0, componentCount * sizeof( double ) );
	
	/* Update Variables */
	Variable_Update( variable );
	Variable_Update( startValue );
	arrayCount     = variable->arraySize;
	
	for ( array_I = 0 ; array_I < arrayCount ; array_I++ ) {
		arrayDataPtr = Variable_GetPtrDouble( variable, array_I );
		startDataPtr = Variable_GetPtrDouble( startValue, array_I );

		TimeIntegrator_SetTime( self->timeIntegrator, startTime );

		/* Store Original Value in case startValue == self->variable */
		memcpy( startData, startDataPtr, sizeof( double ) * componentCount );

		/* Do Predictor Step */
		successFlag = TimeIntegratee_CalculateTimeDeriv( self, array_I, timeDeriv );
		Journal_Firewall( True == successFlag, errorStream,
			"Error - in %s(), for TimeIntegratee \"%s\" of type %s: When trying to find time "
			"deriv for item %u in step %u, *failed*.\n",
			__func__, self->name, self->type, array_I, 1 );
		
		for ( component_I = 0 ; component_I < componentCount ; component_I++ ) 
			arrayDataPtr[ component_I ] = startData[ component_I ] + 0.5 * dt * timeDeriv[ component_I ];
		TimeIntegratee_Intermediate( self, array_I );

		TimeIntegrator_SetTime( self->timeIntegrator, startTime + 0.5 * dt );

		/* Do Corrector Step */
		successFlag = TimeIntegratee_CalculateTimeDeriv( self, array_I, timeDeriv );

		if ( True == successFlag ) {
			for ( component_I = 0 ; component_I < componentCount ; component_I++ ) 
				arrayDataPtr[ component_I ] = startData[ component_I ] + dt * timeDeriv[ component_I ];
			
			TimeIntegratee_Intermediate( self, array_I );
		}
		else {
			Journal_Firewall( True == self->allowFallbackToFirstOrder, errorStream,
				"Error - in %s(), for TimeIntegratee \"%s\" of type %s: When trying to find time "
				"deriv for item %u in step %u, *failed*, and self->allowFallbackToFirstOrder "
				"not enabled.\n", __func__, self->name, self->type, array_I, 2 );
				
			_TimeIntegratee_RewindToStartAndApplyFirstOrderUpdate( self,
				arrayDataPtr, startData, startTime, dt,
				timeDeriv, array_I );
		}
	}

	Memory_Free( timeDeriv );
	Memory_Free( startData );
}

void TimeIntegratee_FourthOrder( void* timeIntegratee, Variable* startValue, double dt ) {
	TimeIntegratee*	self           = (TimeIntegratee*)timeIntegratee;
	Variable*       variable       = self->variable;
	double*         arrayDataPtr;
	double*         startDataPtr;
	double*         timeDeriv;
	double*         finalTimeDeriv;
	double*         startData;
	Index           component_I; 
	Index           componentCount = *variable->dataTypeCounts;
	Index           array_I; 
	Index           arrayCount;
	double          startTime      = TimeIntegrator_GetTime( self->timeIntegrator );
	Bool            successFlag = False;
	Stream*         errorStream = Journal_Register( Error_Type, self->type );

	timeDeriv      = Memory_Alloc_Array( double, componentCount, "Time Deriv" );
	startData      = Memory_Alloc_Array( double, componentCount, "StartData" );
	finalTimeDeriv = Memory_Alloc_Array( double, componentCount, "StartData" );
	memset( timeDeriv,      0, componentCount * sizeof( double ) );
	memset( startData,      0, componentCount * sizeof( double ) );
	memset( finalTimeDeriv, 0, componentCount * sizeof( double ) );
	
	/* Update Variables */
	Variable_Update( variable );
	Variable_Update( startValue );
	arrayCount     = variable->arraySize;
	
	for ( array_I = 0 ; array_I < arrayCount ; array_I++ ) {
		arrayDataPtr = Variable_GetPtrDouble( variable, array_I );
		startDataPtr = Variable_GetPtrDouble( startValue, array_I );
		
		TimeIntegrator_SetTime( self->timeIntegrator, startTime );

		/* Store Original Value in case startValue == self->variable */
		memcpy( startData, startDataPtr, sizeof( double ) * componentCount );

		/* Do first Step - store K1 in finalTimeDeriv and update for next step */
		successFlag = TimeIntegratee_CalculateTimeDeriv( self, array_I, finalTimeDeriv );
		Journal_Firewall( True == successFlag, errorStream,
			"Error - in %s(), for TimeIntegratee \"%s\" of type %s: When trying to find time "
			"deriv for item %u in step %u, *failed*.\n",
			__func__, self->name, self->type, array_I, 1 );

		for ( component_I = 0 ; component_I < componentCount ; component_I++ ) 
			arrayDataPtr[ component_I ] = startData[ component_I ] + 0.5 * dt * finalTimeDeriv[ component_I ];
		TimeIntegratee_Intermediate( self, array_I );

		TimeIntegrator_SetTime( self->timeIntegrator, startTime + 0.5 * dt );

		/* Do Second Step - add 2xK2 value to finalTimeDeriv and update for next step */
		successFlag = TimeIntegratee_CalculateTimeDeriv( self, array_I, timeDeriv );
		if ( True == successFlag ) {
			for ( component_I = 0 ; component_I < componentCount ; component_I++ ) {
				arrayDataPtr[ component_I ] = startData[ component_I ] + 0.5 * dt * timeDeriv[ component_I ];
				finalTimeDeriv[ component_I ] += 2.0 * timeDeriv[ component_I ];
			}
			TimeIntegratee_Intermediate( self, array_I );
		}
		else {
			Journal_Firewall( True == self->allowFallbackToFirstOrder, errorStream,
				"Error - in %s(), for TimeIntegratee \"%s\" of type %s: When trying to find time "
				"deriv for item %u in step %u, *failed*, and self->allowFallbackToFirstOrder "
				"not enabled.\n", __func__, self->name, self->type, array_I, 2 );
				
			_TimeIntegratee_RewindToStartAndApplyFirstOrderUpdate( self,
				arrayDataPtr, startData, startTime, dt,
				timeDeriv, array_I );
		}

		/* Do Third Step - add 2xK3 value to finalTimeDeriv and update for next step */
		successFlag = TimeIntegratee_CalculateTimeDeriv( self, array_I, timeDeriv );
		if ( True == successFlag ) {
			for ( component_I = 0 ; component_I < componentCount ; component_I++ ) {
				arrayDataPtr[ component_I ] = startData[ component_I ] + dt * timeDeriv[ component_I ];
				finalTimeDeriv[ component_I ] += 2.0 * timeDeriv[ component_I ];
			}
			TimeIntegratee_Intermediate( self, array_I );
		}
		else {
			Journal_Firewall( True == self->allowFallbackToFirstOrder, errorStream,
				"Error - in %s(), for TimeIntegratee \"%s\" of type %s: When trying to find time "
				"deriv for item %u in step %u, *failed*, and self->allowFallbackToFirstOrder "
				"not enabled.\n", __func__, self->name, self->type, array_I, 3 );
				
			_TimeIntegratee_RewindToStartAndApplyFirstOrderUpdate( self,
				arrayDataPtr, startData, startTime, dt,
				timeDeriv, array_I );
		}

		TimeIntegrator_SetTime( self->timeIntegrator, startTime + dt );

		/* Do Fourth Step - 'K1 + 2K2 + 2K3' and K4 finalTimeDeriv to find final value */
		successFlag = TimeIntegratee_CalculateTimeDeriv( self, array_I, timeDeriv );
		if ( True == successFlag ) {
			for ( component_I = 0 ; component_I < componentCount ; component_I++ ) {
				arrayDataPtr[ component_I ] = startData[ component_I ] + 
					dt/6.0 * (timeDeriv[ component_I ] + finalTimeDeriv[ component_I ] );
			}		
			TimeIntegratee_Intermediate( self, array_I );
		}
		else {
			Journal_Firewall( True == self->allowFallbackToFirstOrder, errorStream,
				"Error - in %s(), for TimeIntegratee \"%s\" of type %s: When trying to find time "
				"deriv for item %u in step %u, *failed*, and self->allowFallbackToFirstOrder "
				"not enabled.\n", __func__, self->name, self->type, array_I, 4 );
				
			_TimeIntegratee_RewindToStartAndApplyFirstOrderUpdate( self,
				arrayDataPtr, startData, startTime, dt,
				timeDeriv, array_I );
		}
	}

	Memory_Free( timeDeriv );
	Memory_Free( startData );
	Memory_Free( finalTimeDeriv );
}


/* Note : this function is used to apply to just one item/particle - see the array_I parameter */
void _TimeIntegratee_RewindToStartAndApplyFirstOrderUpdate( 
		TimeIntegratee* self,
		double*         arrayDataPtr,
		double*         startData,
		double          startTime,
		double          dt,
		double*         timeDeriv,
		Index           array_I )
{
	Variable*       variable       = self->variable;
	Index           component_I; 
	Index           componentCount = *variable->dataTypeCounts;
	Bool            successFlag = False;

	/* First, go back to initial positions, so we can re-calculate the time derivative there */
	for ( component_I = 0 ; component_I < componentCount ; component_I++ ) {
		arrayDataPtr[ component_I ] = startData[ component_I ];
	}	
	TimeIntegratee_Intermediate( self, array_I );

	/* Now recalculate time deriv at start positions, then do a full dt first order update from
	 * there */
	TimeIntegrator_SetTime( self->timeIntegrator, startTime );
	successFlag = TimeIntegratee_CalculateTimeDeriv( self, array_I, timeDeriv );
	for ( component_I = 0 ; component_I < componentCount ; component_I++ ) 
		arrayDataPtr[ component_I ] = startData[ component_I ] + dt * timeDeriv[ component_I ];
	TimeIntegratee_Intermediate( self, array_I );
}


/** +++ Sample Time Deriv Functions +++ **/


/** This function assumes that
 *            the ODE that we are solving is \dot \phi = u(x,y) 
 *            the velocity Field Variable is stored in data[0]
 *            the variable being updated is the global coordinate of the object */
Bool _TimeIntegratee_AdvectionTimeDeriv( void* timeIntegratee, Index array_I, double* timeDeriv ) {
	TimeIntegratee*	     self          = (TimeIntegratee*) timeIntegratee;
	FieldVariable*       velocityField = (FieldVariable*)  self->data[0];
	double*              coord;
	InterpolationResult  result;

	/* Get Coordinate of Object using Variable */
	coord = Variable_GetPtrDouble( self->variable, array_I );

	result = FieldVariable_InterpolateValueAt( velocityField, coord, timeDeriv );

	if ( result == OTHER_PROC || result == OUTSIDE_GLOBAL || isinf(timeDeriv[0]) || isinf(timeDeriv[1]) || 
			( velocityField->dim == 3 && isinf(timeDeriv[2]) ) ) 
	{
		#if 0
		Journal_Printf( Journal_Register( Error_Type, self->type ),
			"Error in func '%s' for particle with index %u.\n\tPosition (%g, %g, %g)\n\tVelocity here is (%g, %g, %g)."
			"\n\tInterpolation result is %s.\n",
			__func__, array_I, coord[0], coord[1], coord[2], 
			timeDeriv[0], timeDeriv[1], ( velocityField->dim == 3 ? timeDeriv[2] : 0.0 ),
			InterpolationResultToStringMap[result]  );
		return False;	
		#endif
	}

	return True;
}


void _TimeIntegratee_Intermediate( void* timeIntegratee, Index array_I ) {}

/* +++ Public Functions +++ */
void TimeIntegratee_StoreTimeDeriv( void* timeIntegratee, Variable* timeDeriv ) {
	TimeIntegratee*	self           = (TimeIntegratee*)timeIntegratee;
	double*         arrayDataPtr;
	Index           array_I; 
	Index           arrayCount;

	/* Update Variable */
	Variable_Update( timeDeriv );
	arrayCount = timeDeriv->arraySize;

	for ( array_I = 0 ; array_I < arrayCount ; array_I++ ) {
		arrayDataPtr = Variable_GetPtrDouble( timeDeriv, array_I );
		TimeIntegratee_CalculateTimeDeriv( self, array_I, arrayDataPtr );
	}
}

void TimeIntegratee_Add2TimesTimeDeriv( void* timeIntegratee, Variable* timeDerivVariable ) {
	TimeIntegratee*	self           = (TimeIntegratee*)timeIntegratee;
	Variable*       variable       = self->variable;
	double*         timeDerivPtr;
	double*         timeDeriv;
	Index           component_I; 
	Index           componentCount = *variable->dataTypeCounts;
	Index           array_I; 
	Index           arrayCount;

	timeDeriv = Memory_Alloc_Array( double, componentCount, "Time Deriv" );
	memset( timeDeriv,      0, componentCount * sizeof( double ) );
	
	/* Update Variables */
	Variable_Update( variable );
	Variable_Update( timeDerivVariable );
	arrayCount = variable->arraySize;
	
	for ( array_I = 0 ; array_I < arrayCount ; array_I++ ) {
		TimeIntegratee_CalculateTimeDeriv( self, array_I, timeDeriv );
		timeDerivPtr = Variable_GetPtrDouble( timeDerivVariable, array_I );
		
		for ( component_I = 0 ; component_I < componentCount ; component_I++ ) {
			timeDerivPtr[ component_I ] += 2.0 * timeDeriv[ component_I ];
		}
	}

	Memory_Free( timeDeriv );
}
	

void TimeIntegratee_FourthOrderFinalStep( void* timeIntegratee, Variable* startData, Variable* timeDerivVariable, double dt ) {
	TimeIntegratee*	self           = (TimeIntegratee*)timeIntegratee;
	Variable*       variable       = self->variable;
	double*         k4;
	double*         k1_2k2_2k3;
	double*         startPtr;
	double*         arrayPtr;
	Index           component_I; 
	Index           componentCount = *variable->dataTypeCounts;
	Index           array_I; 
	Index           arrayCount;

	k4 = Memory_Alloc_Array( double, componentCount, "k4 Time Deriv" );
	memset( k4, 0, componentCount * sizeof( double ) );
	
	/* Update Variables */
	Variable_Update( variable );
	Variable_Update( startData );
	Variable_Update( timeDerivVariable );
	arrayCount     = variable->arraySize;
	
	for ( array_I = 0 ; array_I < arrayCount ; array_I++ ) {
		TimeIntegratee_CalculateTimeDeriv( self, array_I, k4 );

		k1_2k2_2k3 = Variable_GetPtrDouble( timeDerivVariable, array_I );
		arrayPtr = Variable_GetPtrDouble( variable, array_I );
		startPtr = Variable_GetPtrDouble( startData, array_I );
		
		for ( component_I = 0 ; component_I < componentCount ; component_I++ ) {
			arrayPtr[ component_I ] = startPtr[ component_I ] + dt/6.0 * ( k4[ component_I ] + k1_2k2_2k3[ component_I ] );
		}
		TimeIntegratee_Intermediate( self, array_I );
	}

	Memory_Free( k4 );
}



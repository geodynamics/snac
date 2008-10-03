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
** $Id: SwarmVariable.c 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"
#include "Discretisation/Mesh/Mesh.h"
#include "Discretisation/Utils/Utils.h"

#include "types.h"
#include "SwarmClass.h"
#include "SwarmVariable_Register.h"
#include "SwarmVariable.h"

#include <assert.h>
#include <string.h>

const Type SwarmVariable_Type = "SwarmVariable";

SwarmVariable* SwarmVariable_New(		
		Name                                               name,
		Swarm*                                             swarm,
		Variable*                                          variable,
		Index                                              dofCount )
{
	SwarmVariable* self = (SwarmVariable*) _SwarmVariable_DefaultNew( name );

	SwarmVariable_InitAll( self, swarm, variable, dofCount );

	return self;
}

SwarmVariable* _SwarmVariable_New(
		SizeT                                              _sizeOfSelf, 
		Type                                               type,
		Stg_Class_DeleteFunction*	                       _delete,
		Stg_Class_PrintFunction*	                       _print, 
		Stg_Class_CopyFunction*	                           _copy, 
		Stg_Component_DefaultConstructorFunction*          _defaultConstructor,
		Stg_Component_ConstructFunction                    _construct,
		Stg_Component_BuildFunction*                       _build,
		Stg_Component_InitialiseFunction*                  _initialise,
		Stg_Component_ExecuteFunction*                     _execute,
		Stg_Component_DestroyFunction*                     _destroy,
		SwarmVariable_ValueAtFunction*                     _valueAt,
		SwarmVariable_GetGlobalValueFunction*              _getMinGlobalMagnitude,
		SwarmVariable_GetGlobalValueFunction*              _getMaxGlobalMagnitude,		
		Name                                               name )
{
	SwarmVariable*		self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(SwarmVariable) );
	self = (SwarmVariable*)_Stg_Component_New( 
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
	
	/* Virtual functions */
	self->_valueAt                    = _valueAt;
	self->_getMinGlobalMagnitude      = _getMinGlobalMagnitude;
	self->_getMaxGlobalMagnitude      = _getMaxGlobalMagnitude;

	return self;
}

void _SwarmVariable_Init( SwarmVariable* self, Swarm* swarm, Variable* variable, Index dofCount ) {
	/* Add ourselves to the register for later retrieval by clients */
	self->isConstructed = True;

	self->swarm                  = swarm;
	self->variable               = variable;
	self->dofCount               = dofCount;
	self->swarmVariable_Register = swarm->swarmVariable_Register;
	self->dim                    = swarm->dim;
	
	if ( self->swarmVariable_Register != NULL )	
		SwarmVariable_Register_Add( self->swarmVariable_Register, self );
}

void SwarmVariable_InitAll( void* swarmVariable, Swarm* swarm, Variable* variable, Index dofCount ) {
	SwarmVariable* self = (SwarmVariable*) swarmVariable;

	/* Should be calling InitAll of parent here */

	/* Call my Init function */
	_SwarmVariable_Init( self, swarm, variable, dofCount );
}

void _SwarmVariable_Delete( void* swarmVariable ) {
	SwarmVariable* self = (SwarmVariable*) swarmVariable;
	_Stg_Component_Delete( self );
}

void _SwarmVariable_Print( void* _swarmVariable, Stream* stream ) {
	SwarmVariable* self = (SwarmVariable*) _swarmVariable;

	Journal_Printf( stream, "SwarmVariable - '%s'\n", self->name );
	Stream_Indent( stream );
	_Stg_Component_Print( self, stream );

	Journal_PrintPointer( stream, self->_valueAt );
	Journal_PrintPointer( stream, self->_getMinGlobalMagnitude );
	Journal_PrintPointer( stream, self->_getMaxGlobalMagnitude );

	Journal_Printf( stream, "Swarm - '%s'\n", self->swarm->name );
	if ( self->variable != NULL )
		Journal_Printf( stream, "Variable - '%s'\n", self->variable->name );

	Journal_PrintValue( stream, self->dofCount );
	Journal_PrintPointer( stream, self->swarmVariable_Register );
	Stream_UnIndent( stream );
}

void* _SwarmVariable_Copy( void* swarmVariable, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	SwarmVariable*	self = (SwarmVariable*)swarmVariable;
	SwarmVariable*	newSwarmVariable;
	PtrMap*			map = ptrMap;
	Bool			ownMap = False;
	
	if( !map ) {
		map = PtrMap_New( 10 );
		ownMap = True;
	}
	
	newSwarmVariable = _Stg_Component_Copy( self, dest, deep, nameExt, map );
	
	newSwarmVariable->_valueAt                   = self->_valueAt;
	newSwarmVariable->_getMinGlobalMagnitude     = self->_getMinGlobalMagnitude  ;
	newSwarmVariable->_getMaxGlobalMagnitude     = self->_getMaxGlobalMagnitude;

	newSwarmVariable->swarm                      = self->swarm;
	newSwarmVariable->variable                   = self->variable;
	newSwarmVariable->dofCount                   = self->dofCount;
	newSwarmVariable->swarmVariable_Register     = self->swarmVariable_Register;

	if( ownMap ) {
		Stg_Class_Delete( map );
	}
				
	return (void*)newSwarmVariable;
}


void* _SwarmVariable_DefaultNew( Name name ) {
		return (void*) _SwarmVariable_New( 
			sizeof(SwarmVariable), 
			SwarmVariable_Type, 
			_SwarmVariable_Delete, 
			_SwarmVariable_Print,
			_SwarmVariable_Copy, 
			_SwarmVariable_DefaultNew,
			_SwarmVariable_Construct,
			_SwarmVariable_Build, 
			_SwarmVariable_Initialise, 
			_SwarmVariable_Execute, 
			_SwarmVariable_Destroy, 
			_SwarmVariable_ValueAt,
			_SwarmVariable_GetMinGlobalMagnitude,
			_SwarmVariable_GetMaxGlobalMagnitude,
			name );
}

void _SwarmVariable_Construct( void* swarmVariable, Stg_ComponentFactory* cf, void* data ) {
	SwarmVariable*	        self         = (SwarmVariable*)swarmVariable;
	Swarm*                  swarm;
	Variable*               variable;
	Index                   dofCount;

	swarm    =  Stg_ComponentFactory_ConstructByKey(  cf,  self->name,  "Swarm", Swarm, True, data  ) ;
	variable =  Stg_ComponentFactory_ConstructByKey(  cf,  self->name,  "Variable", Variable,  False, data  ) ;
	dofCount = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, "dofCount", 0 );

	_SwarmVariable_Init( self, swarm, variable, dofCount );
	
}

void _SwarmVariable_Build( void* swarmVariable, void* data ) {
	SwarmVariable*	        self         = (SwarmVariable*)swarmVariable;

	if ( self->variable )
		Stg_Component_Build( self->variable, data, False );
}

void _SwarmVariable_Initialise( void* swarmVariable, void* data ) {
	SwarmVariable*	        self         = (SwarmVariable*)swarmVariable;

	if ( self->variable ) {
		Variable_Update( self->variable );
		Stg_Component_Initialise( self->variable, data, False );
	}
}

void _SwarmVariable_Execute( void* swarmVariable, void* data ) {
}

void _SwarmVariable_Destroy( void* swarmVariable, void* data ) {
}

double SwarmVariable_GetMinGlobalMagnitude( void* swarmVariable ) {
	SwarmVariable*	self = (SwarmVariable*)swarmVariable;
	return self->_getMinGlobalMagnitude( self );
}

double SwarmVariable_GetMaxGlobalMagnitude( void* swarmVariable ) {
	SwarmVariable*	self = (SwarmVariable*)swarmVariable;
	return self->_getMaxGlobalMagnitude( self );
}

/*** Default Implementations ***/

void _SwarmVariable_ValueAt( void* swarmVariable, Particle_Index lParticle_I, double* value ) {
	SwarmVariable*	self = (SwarmVariable*)swarmVariable;
	Variable*       variable = self->variable;

	switch( variable->dataTypes[0] ) {
		case Variable_DataType_Double:
			self->_valueAt = _SwarmVariable_ValueAtDouble;
			break;
		case Variable_DataType_Int:
			self->_valueAt = _SwarmVariable_ValueAtInt;
			break;
		case Variable_DataType_Float:
			self->_valueAt = _SwarmVariable_ValueAtFloat;
			break;
		case Variable_DataType_Char:
			self->_valueAt = _SwarmVariable_ValueAtChar;
			break;
		case Variable_DataType_Short:
			self->_valueAt = _SwarmVariable_ValueAtShort;
			break;
		default:
			assert(0);
	}
	SwarmVariable_ValueAt( self, lParticle_I, value );
}


	
void _SwarmVariable_ValueAtDouble( void* swarmVariable, Particle_Index lParticle_I, double* value ) {
	SwarmVariable*	self = (SwarmVariable*)swarmVariable;
	double*         dataPtr;

	dataPtr = Variable_GetPtrDouble( self->variable, lParticle_I );
	memcpy( value, dataPtr, sizeof(double) * self->dofCount );
}
void _SwarmVariable_ValueAtInt( void* swarmVariable, Particle_Index lParticle_I, double* value ) {
	SwarmVariable*	self     = (SwarmVariable*)swarmVariable;
	Variable*       variable = self->variable;
	Dof_Index       dofCount = self->dofCount;
	Dof_Index       dof_I;

	for ( dof_I = 0 ; dof_I < dofCount ; dof_I++ ) {
		value[ dof_I ] = (double) Variable_GetValueAtInt( variable, lParticle_I, dof_I );
	}
}
void _SwarmVariable_ValueAtFloat( void* swarmVariable, Particle_Index lParticle_I, double* value ) {
	SwarmVariable*	self     = (SwarmVariable*)swarmVariable;
	Variable*       variable = self->variable;
	Dof_Index       dofCount = self->dofCount;
	Dof_Index       dof_I;

	for ( dof_I = 0 ; dof_I < dofCount ; dof_I++ ) {
		value[ dof_I ] = (double) Variable_GetValueAtFloat( variable, lParticle_I, dof_I );
	}
}
void _SwarmVariable_ValueAtChar( void* swarmVariable, Particle_Index lParticle_I, double* value ) {
	SwarmVariable*	self     = (SwarmVariable*)swarmVariable;
	Variable*       variable = self->variable;
	Dof_Index       dofCount = self->dofCount;
	Dof_Index       dof_I;

	for ( dof_I = 0 ; dof_I < dofCount ; dof_I++ ) {
		value[ dof_I ] = (double) Variable_GetValueAtChar( variable, lParticle_I, dof_I );
	}
}
void _SwarmVariable_ValueAtShort( void* swarmVariable, Particle_Index lParticle_I, double* value ) {
	SwarmVariable*	self     = (SwarmVariable*)swarmVariable;
	Variable*       variable = self->variable;
	Dof_Index       dofCount = self->dofCount;
	Dof_Index       dof_I;

	for ( dof_I = 0 ; dof_I < dofCount ; dof_I++ ) {
		value[ dof_I ] = (double) Variable_GetValueAtShort( variable, lParticle_I, dof_I );
	}
}
	

double _SwarmVariable_GetMinGlobalMagnitude( void* swarmVariable ) {
	SwarmVariable*	self               = (SwarmVariable*)swarmVariable;
	double*         value;
	Swarm*          swarm              = self->swarm;
	Particle_Index  particleLocalCount = swarm->particleLocalCount;
	Particle_Index  lParticle_I;
	double          localMin           = HUGE_VAL;
	double          globalMin;
	Index           dofCount           = self->dofCount;
	double          magnitude;

	value = Memory_Alloc_Array( double, dofCount, "value" );

	/* Search through all local particles and find smallest value of variable */
	for ( lParticle_I = 0 ; lParticle_I < particleLocalCount ; lParticle_I++ ) {
		SwarmVariable_ValueAt( self, lParticle_I, value );

		if ( dofCount == 1 )
			magnitude = value[0];
		else 
			assert(0); //TODO

		if ( localMin > magnitude )
			localMin = magnitude;
	}

	Memory_Free( value );
	MPI_Allreduce( &localMin, &globalMin, dofCount, MPI_DOUBLE, MPI_MIN, swarm->comm );

	return globalMin;
}


double _SwarmVariable_GetMaxGlobalMagnitude( void* swarmVariable ) {
	SwarmVariable*	self               = (SwarmVariable*)swarmVariable;
	double*         value;
	Swarm*          swarm              = self->swarm;
	Particle_Index  particleLocalCount = swarm->particleLocalCount;
	Particle_Index  lParticle_I;
	double          localMax           = -HUGE_VAL;
	double          globalMax;
	Index           dofCount           = self->dofCount;
	double          magnitude;

	value = Memory_Alloc_Array( double, dofCount, "value" );

	/* Search through all local particles and find smallest value of variable */
	for ( lParticle_I = 0 ; lParticle_I < particleLocalCount ; lParticle_I++ ) {
		SwarmVariable_ValueAt( self, lParticle_I, value );

		if ( dofCount == 1 )
			magnitude = value[0];
		else 
			assert(0); //TODO

		if ( localMax < magnitude )
			localMax = magnitude;
	}

	Memory_Free( value );
	MPI_Allreduce( &localMax, &globalMax, dofCount, MPI_DOUBLE, MPI_MAX, swarm->comm );
	
	return globalMax;
}

	





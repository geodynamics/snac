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
** $Id: OperatorSwarmVariable.c 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <string.h>
#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"
#include "Discretisation/Mesh/Mesh.h"
#include "Discretisation/Utils/Utils.h"

#include "types.h"
#include "SwarmVariable_Register.h"
#include "SwarmVariable.h"
#include "OperatorSwarmVariable.h"
#include "SwarmClass.h"

#include <assert.h>

const Type OperatorSwarmVariable_Type = "OperatorSwarmVariable";
const Name defaultOperatorSwarmVariableName = "defaultOperatorSwarmVariableName";

OperatorSwarmVariable* OperatorSwarmVariable_NewUnary( 
		Name                                               name,
		void*                                              _swarmVariable,
		Name                                               operatorName )
{
	SwarmVariable*          swarmVariable = (SwarmVariable*) _swarmVariable;
	OperatorSwarmVariable*  operatorSwarmVariable;
       	
	operatorSwarmVariable = OperatorSwarmVariable_New( 
			name,
			_OperatorSwarmVariable_UnaryValueAt, 
			operatorName,
			1,
			&swarmVariable );

	return operatorSwarmVariable;
}

OperatorSwarmVariable* OperatorSwarmVariable_NewBinary( 
		Name                                               name,
		void*                                              _swarmVariable1,
		void*                                              _swarmVariable2,
		Name                                               operatorName )
{
	SwarmVariable* swarmVariableList[2];
       
	swarmVariableList[0] = (SwarmVariable*) _swarmVariable1;
	swarmVariableList[1] = (SwarmVariable*) _swarmVariable2;
	
	return OperatorSwarmVariable_New( 
			name,
			_OperatorSwarmVariable_BinaryValueAt, 
			operatorName,
			2, 
			swarmVariableList );
}

void* _OperatorSwarmVariable_DefaultNew( Name name )
{
		return (void*) _OperatorSwarmVariable_New( 
			sizeof(OperatorSwarmVariable), 
			OperatorSwarmVariable_Type, 
			_SwarmVariable_Delete, 
			_OperatorSwarmVariable_Print,
			_OperatorSwarmVariable_Copy, 
			_OperatorSwarmVariable_DefaultNew,
			_OperatorSwarmVariable_Construct,
			_OperatorSwarmVariable_Build, 
			_OperatorSwarmVariable_Initialise, 
			_OperatorSwarmVariable_Execute,
			_OperatorSwarmVariable_Destroy,
			_OperatorSwarmVariable_ValueAt,
			_OperatorSwarmVariable_GetMinGlobalMagnitude,
			_OperatorSwarmVariable_GetMaxGlobalMagnitude, 
			name );
}

OperatorSwarmVariable* OperatorSwarmVariable_New( 
		Name                                               name,
		SwarmVariable_ValueAtFunction*                     _valueAt,
		Name                                               operatorName,
		Index                                              swarmVariableCount,
		SwarmVariable**                                    swarmVariableList )
{
	OperatorSwarmVariable*  operatorSwarmVariable;

	operatorSwarmVariable = _OperatorSwarmVariable_New( 
			sizeof(OperatorSwarmVariable), 
			OperatorSwarmVariable_Type, 
			_SwarmVariable_Delete, 
			_OperatorSwarmVariable_Print,
			_OperatorSwarmVariable_Copy, 
			_OperatorSwarmVariable_DefaultNew,
			_OperatorSwarmVariable_Construct,
			_OperatorSwarmVariable_Build, 
			_OperatorSwarmVariable_Initialise, 
			_OperatorSwarmVariable_Execute,
			_OperatorSwarmVariable_Destroy,
			_OperatorSwarmVariable_ValueAt,
			_OperatorSwarmVariable_GetMinGlobalMagnitude,
			_OperatorSwarmVariable_GetMaxGlobalMagnitude, 
			name );

	/* Following Rob's inheritance approach for this file, until we do the big fix-up */
	/* Can pass in 0 for the dofs since this will be updated in the next func */
	SwarmVariable_InitAll( operatorSwarmVariable, swarmVariableList[0]->swarm, NULL, 0 );
	_OperatorSwarmVariable_Init( operatorSwarmVariable, operatorName, swarmVariableCount, swarmVariableList );

	return operatorSwarmVariable;
}

OperatorSwarmVariable* _OperatorSwarmVariable_New(
			SizeT                                              _sizeOfSelf, 
			Type                                               type,
			Stg_Class_DeleteFunction*                          _delete,
			Stg_Class_PrintFunction*                           _print, 
			Stg_Class_CopyFunction*                            _copy, 
			Stg_Component_DefaultConstructorFunction*          _defaultConstructor,
			Stg_Component_ConstructFunction*                   _construct,
			Stg_Component_BuildFunction*                       _build,
			Stg_Component_InitialiseFunction*                  _initialise,
			Stg_Component_ExecuteFunction*                     _execute,
			Stg_Component_DestroyFunction*                     _destroy,
			SwarmVariable_ValueAtFunction*                     _valueAt,
			SwarmVariable_GetGlobalValueFunction*              _getMinGlobalMagnitude,
			SwarmVariable_GetGlobalValueFunction*              _getMaxGlobalMagnitude,
			Name                                               name )
{
	OperatorSwarmVariable*		self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(OperatorSwarmVariable) );
	self = (OperatorSwarmVariable*) _SwarmVariable_New( 
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
			_valueAt,
			_getMinGlobalMagnitude,
			_getMaxGlobalMagnitude,
			name );
	
	return self;
}

void _OperatorSwarmVariable_Delete( void* _swarmVariable ) {
	OperatorSwarmVariable* self = (OperatorSwarmVariable*) _swarmVariable;

	Memory_Free( self->swarmVariableList );

	_SwarmVariable_Delete( self );
}

void _OperatorSwarmVariable_Print( void* _swarmVariable, Stream* stream ) {
	OperatorSwarmVariable* self = (OperatorSwarmVariable*) _swarmVariable;
	Index                  swarmVariable_I;

	_SwarmVariable_Print( self, stream );

	Journal_PrintValue( stream, self->swarmVariableCount );
	for ( swarmVariable_I = 0 ; swarmVariable_I < self->swarmVariableCount ; swarmVariable_I++ ) 
		Journal_Printf( stream, "\tSwarmVariable %u - '%s'\n", swarmVariable_I, self->swarmVariableList[ swarmVariable_I ]->name );

}

void _OperatorSwarmVariable_Init( void* ofv, Name operatorName, Index swarmVariableCount, SwarmVariable** swarmVariableList ) {
	OperatorSwarmVariable*	self = (OperatorSwarmVariable*)ofv;
	SwarmVariable*              swarmVariable;
	Index                       swarmVariable_I;
	Stream*                     errorStream       = Journal_Register( Error_Type, self->type );

	self->isConstructed = True;

	self->_operator              = Operator_NewFromName( operatorName, swarmVariableList[0]->dofCount, self->dim );
	self->dofCount               = self->_operator->resultDofs; /* reset value */
	self->swarmVariableCount     = swarmVariableCount;

	/* Copy swarm variable list */
	self->swarmVariableList      = Memory_Alloc_Array( SwarmVariable*, swarmVariableCount, "Array of Swarm Variables" );
	memcpy( self->swarmVariableList, swarmVariableList, swarmVariableCount * sizeof( SwarmVariable* ) );

	for ( swarmVariable_I = 0 ; swarmVariable_I < swarmVariableCount ; swarmVariable_I++ ) {
		swarmVariable = swarmVariableList[ swarmVariable_I ];
		Journal_Firewall( swarmVariable != NULL, errorStream, 
				"In func %s: SwarmVariable %u in list is NULL\n", __func__, swarmVariable_I );
		Journal_Firewall( swarmVariable->dofCount <= MAX_DOF, errorStream, 
			"In func %s: Swarm Variable '%s' has too many components.\n", __func__, swarmVariable->name );
	}
}


void* _OperatorSwarmVariable_Copy( void* swarmVariable, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	OperatorSwarmVariable*	self = (OperatorSwarmVariable*)swarmVariable;
	OperatorSwarmVariable*	newOperatorSwarmVariable;
	
	newOperatorSwarmVariable = _SwarmVariable_Copy( self, dest, deep, nameExt, ptrMap );
	
	newOperatorSwarmVariable->_operator              = self->_operator;
	newOperatorSwarmVariable->swarmVariableCount     = self->swarmVariableCount;
	
	if (deep) {
		newOperatorSwarmVariable->swarmVariableList = Memory_Alloc_Array( SwarmVariable*, self->swarmVariableCount, 
				"Array of Swarm Variables" );
		memcpy( newOperatorSwarmVariable->swarmVariableList, self->swarmVariableList, 
				self->swarmVariableCount * sizeof( SwarmVariable* ) );
	}
	else 
		newOperatorSwarmVariable->swarmVariableList = self->swarmVariableList;
	
	return (void*)newOperatorSwarmVariable;
}

void _OperatorSwarmVariable_Construct( void* swarmVariable, Stg_ComponentFactory* cf, void* data ) {
	OperatorSwarmVariable*  self       = (OperatorSwarmVariable*) swarmVariable;
	Dictionary*             dictionary = Dictionary_GetDictionary( cf->componentDict, self->name );
	Dictionary_Entry_Value* list;
	Index                   swarmVariableCount = 0;
	Index                   swarmVariable_I;
	Name                    swarmVariableName;
	Name                    operatorName;
	SwarmVariable**         swarmVariableList;
	SwarmVariable_Register* swarmVariable_Register;

	/* Call parent's construct function */
	_SwarmVariable_Construct( self, cf, data );
	swarmVariable_Register = self->swarm->swarmVariable_Register;

	operatorName = Stg_ComponentFactory_GetString( cf, self->name, "Operator", "" );

	list = Dictionary_Get( dictionary, "SwarmVariables" );

	swarmVariableCount = ( list ? Dictionary_Entry_Value_GetCount(list) : 1 );
	swarmVariableList = Memory_Alloc_Array( SwarmVariable*, swarmVariableCount, "SwarmVars" );

	for ( swarmVariable_I = 0 ; swarmVariable_I < swarmVariableCount ; swarmVariable_I++ ) {
		swarmVariableName = (list ? 
				Dictionary_Entry_Value_AsString( Dictionary_Entry_Value_GetElement( list, swarmVariable_I ) ) :
				Dictionary_GetString( dictionary, "SwarmVariable" ) );

		/* Check in swarmVariable_Register first before assuming in LiveComponentRegister */
		Journal_PrintfL( cf->infoStream, 2, "Looking for SwarmVariable '%s' in swarmVariable_Register.\n",
				swarmVariableName );
		swarmVariableList[swarmVariable_I] = (SwarmVariable*) 
			SwarmVariable_Register_GetByName( swarmVariable_Register, swarmVariableName );
		
		if ( !swarmVariableList[swarmVariable_I] )
			swarmVariableList[swarmVariable_I] = 
				Stg_ComponentFactory_ConstructByName( cf, swarmVariableName, SwarmVariable, True, data );
	}

	_SwarmVariable_Construct( self, cf, data );
	_OperatorSwarmVariable_Init( self, operatorName, swarmVariableCount, swarmVariableList );

	Memory_Free( swarmVariableList );
}

void _OperatorSwarmVariable_Build( void* swarmVariable, void* data ) {
	OperatorSwarmVariable* self = (OperatorSwarmVariable*) swarmVariable;
	Index                  swarmVariable_I;

	for ( swarmVariable_I = 0 ; swarmVariable_I < self->swarmVariableCount ; swarmVariable_I++ ) 
		Build( self->swarmVariableList[ swarmVariable_I ] , data, False );
}

void _OperatorSwarmVariable_Execute( void* swarmVariable, void* data ) {}

void _OperatorSwarmVariable_Destroy( void* swarmVariable, void* data ) {}

void _OperatorSwarmVariable_Initialise( void* swarmVariable, void* data ) {
	OperatorSwarmVariable* self = (OperatorSwarmVariable*) swarmVariable;
	Index                  swarmVariable_I;

	for ( swarmVariable_I = 0 ; swarmVariable_I < self->swarmVariableCount ; swarmVariable_I++ ) 
		Initialise( self->swarmVariableList[ swarmVariable_I ] , data, False );
}

double _OperatorSwarmVariable_GetMinGlobalMagnitude( void* swarmVariable ) { 
	/* Just use particle function */
	return _SwarmVariable_GetMinGlobalMagnitude( swarmVariable); 
}
double _OperatorSwarmVariable_GetMaxGlobalMagnitude( void* swarmVariable ) {
	/* Just use particle function */
	return _SwarmVariable_GetMaxGlobalMagnitude( swarmVariable); 
}

void _OperatorSwarmVariable_ValueAt( void* swarmVariable, Particle_Index lParticle_I, double* value ) {
	OperatorSwarmVariable* self            = (OperatorSwarmVariable*) swarmVariable;

	switch ( self->swarmVariableCount ) {
		case 1:
			self->_valueAt = _OperatorSwarmVariable_UnaryValueAt; break;
		case 2:
			self->_valueAt = _OperatorSwarmVariable_BinaryValueAt; break;
		default:
			Journal_Firewall( False, Journal_Register( Error_Type, self->type ),
					"Can't use func '%s' with swarmVariableCount = %d\n", __func__, self->swarmVariableCount );
	}

	SwarmVariable_ValueAt( self, lParticle_I, value );
}

void _OperatorSwarmVariable_UnaryValueAt( void* swarmVariable, Particle_Index lParticle_I, double* value ) {
	OperatorSwarmVariable* self            = (OperatorSwarmVariable*) swarmVariable;
	SwarmVariable*         swarm0          = self->swarmVariableList[0];
	double                 swarmValue[ MAX_DOF ]; 

	SwarmVariable_ValueAt( swarm0, lParticle_I, swarmValue );
	Operator_CarryOutUnaryOperation( self->_operator, swarmValue, value );
}

void _OperatorSwarmVariable_BinaryValueAt( void* swarmVariable, Particle_Index lParticle_I, double* value ) {
	OperatorSwarmVariable* self            = (OperatorSwarmVariable*) swarmVariable;
	SwarmVariable*         swarm0          = self->swarmVariableList[0];
	SwarmVariable*         swarm1          = self->swarmVariableList[1];
	double                 swarmValue0[ MAX_DOF ]; 
	double                 swarmValue1[ MAX_DOF ]; 

	SwarmVariable_ValueAt( swarm0, lParticle_I, swarmValue0 );
	SwarmVariable_ValueAt( swarm1, lParticle_I, swarmValue1 );

	Operator_CarryOutBinaryOperation( self->_operator, swarmValue0, swarmValue1, value ); 
}

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
** $Id: Operator.c 3819 2006-09-28 06:03:10Z JulianGiordani $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


#include <string.h>
#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"
#include "Discretisation/Mesh/Mesh.h"

#include "types.h"
#include "Operator.h"

const Type Operator_Type = "Operator";

Operator* Operator_New(		
		Name                                       name, 		
		Func_Ptr                                   _carryOut,
		Index                                      numberOfOperands,
		Dof_Index                                  operandDofs,
		Dof_Index                                  resultDofs,
		Dimension_Index                            dim ) 
{
	return _Operator_New( 
			sizeof( Operator ),
			Operator_Type,
			_Operator_Delete,
			_Operator_Print,
			_Operator_Copy,
			name,
			_carryOut,
			numberOfOperands,
			operandDofs,
			resultDofs,
			dim );
}

Operator* _Operator_New(		
		SizeT                                      _sizeOfSelf,
		Type                                       type,
		Stg_Class_DeleteFunction*                  _delete,
		Stg_Class_PrintFunction*                   _print, 
		Stg_Class_CopyFunction*                    _copy,
		Name                                       name,
		Func_Ptr                                   _carryOut,
		Index                                      numberOfOperands,
		Dof_Index                                  operandDofs,
		Dof_Index                                  resultDofs,
		Dimension_Index                            dim )
{
	Operator* self = (Operator*) _Stg_Object_New(_sizeOfSelf, type, _delete, _print, _copy, name, NON_GLOBAL);

	/* Set values */
	self->_carryOut        = _carryOut;
	self->numberOfOperands = numberOfOperands;
	self->operandDofs      = operandDofs;
	self->resultDofs       = resultDofs;
	self->dim              = dim;

	/* Create Stream */
	self->errorStream = Journal_MyStream( Error_Type, self );

	return self;
}


void _Operator_Delete(void* operator) {
	Operator* self = (Operator*) operator;
	
	/* Stg_Class_Delete parent class */
	_Stg_Class_Delete( self );
}

void _Operator_Print(void* operator, Stream* stream) {
	//Operator* self = (Operator*) operator;

	abort();
}

void* _Operator_Copy( void* operator, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	//Operator* self = (Operator*) operator;
	abort();
}


/* Carry Out Operators */
void Operator_CarryOutUnaryOperation( void* operator, double* operand0, double* result ) {
	Operator* self            = (Operator*) operator;

	Operator_FirewallUnary( self );
			
	(*(Operator_CarryOutUnaryOperationFunction*) (self->_carryOut))( self, operand0, result );
}

void Operator_CarryOutBinaryOperation( void* operator, double* operand0, double* operand1, double* result ) {
	Operator* self            = (Operator*) operator;

	Operator_FirewallBinary( self );

	(*(Operator_CarryOutBinaryOperationFunction*) (self->_carryOut))( self, operand0, operand1, result );
}

/** Operator Functions */
#define SQUARE_VECTOR2D( vector ) \
	( vector[0] * vector[0] + vector[1] * vector[1] )
#define SQUARE_VECTOR3D( vector ) \
	( vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2] )

void Operator_VectorSquare( void* operator, double* operand0, double* result ) {
	Operator* self            = (Operator*) operator;

	Operator_FirewallUnary( self );
	Operator_FirewallResultDofs( self, 1 );

	switch ( self->operandDofs ) {
		case 2:
			*result = SQUARE_VECTOR2D( operand0 ); break;
		case 3:
			*result = SQUARE_VECTOR3D( operand0 ); break;
		default:
			Journal_Printf( self->errorStream, 
					"Error in %s for %s '%s': Incorrect number of degrees of freedom for what you are operating on.\n",
					__func__, self->type, self->name );
			abort();
	}
}

void Operator_Magnitude( void* operator, double* operand0, double* result ) {
	Operator* self            = (Operator*) operator;

	Operator_FirewallUnary( self );
	Operator_FirewallResultDofs( self, 1 );

	switch ( self->operandDofs ) {
		case 2:
			*result = sqrt( SQUARE_VECTOR2D( operand0 ) ); break;
		case 3:
			*result = sqrt( SQUARE_VECTOR3D( operand0 ) ); break;
		case 1:
			*result = fabs( *operand0 ); break;
		default:
			Journal_Printf( self->errorStream, 
					"Error in %s for %s '%s': Incorrect number of degrees of freedom for what you are operating on.\n",
					__func__, self->type, self->name );
			abort();			
	}
}

void Operator_TensorSymmetricPart( void* operator, double* operand0, double* result ) {
	Operator* self = (Operator*) operator;

	Operator_FirewallUnary( self );
	
	TensorArray_GetSymmetricPart( operand0, self->dim, result );
}

void Operator_SymmetricTensor_Invariant( void* operator, double* operand0, double* result ) {
	Operator* self = (Operator*) operator;

	Operator_FirewallUnary( self );
	Operator_FirewallResultDofs( self, 1 );
	
	*result = SymmetricTensor_2ndInvariant( operand0, self->dim );
}

void Operator_TensorAntisymmetricPart( void* operator, double* operand0, double* result ) {
	Operator* self = (Operator*) operator;

	Operator_FirewallUnary( self );
		
	TensorArray_GetAntisymmetricPart( operand0, self->dim, result );
}

void Operator_SymmetricTensor_GetNegAverageTrace( void* operator, double* operand0, double* result ) {
	Operator* self = (Operator*) operator;
	
	Operator_FirewallUnary( self );
	Operator_FirewallResultDofs( self, 1 );
	
	SymmetricTensor_GetTrace( operand0, self->dim, result ); 

	*result = -1.0 * (*result) / self->dim ;
}

void Operator_Tensor_GetNegAverageTrace( void* operator, double* operand0, double* result ) {
	Operator* self = (Operator*) operator;
	
	Operator_FirewallUnary( self );
	Operator_FirewallResultDofs( self, 1 );
	
	TensorArray_GetTrace( operand0, self->dim, result ); 

	*result = -1.0 * (*result) / self->dim ;
}
	

void Operator_TakeFirstComponent( void* operator, double* operand0, double* result ) {
	Operator* self = (Operator*) operator;

	Operator_FirewallUnary( self );
	Operator_FirewallResultDofs( self, 1 );
	Operator_FirewallMinimumOperandDofs( self, 1 );

	*result = operand0[0];
}
void Operator_TakeSecondComponent( void* operator, double* operand0, double* result ) {
	Operator* self = (Operator*) operator;

	Operator_FirewallUnary( self );
	Operator_FirewallResultDofs( self, 1 );
	Operator_FirewallMinimumOperandDofs( self, 2 );

	*result = operand0[1];
}
void Operator_TakeThirdComponent( void* operator, double* operand0, double* result ) {
	Operator* self = (Operator*) operator;

	Operator_FirewallUnary( self );
	Operator_FirewallResultDofs( self, 1 );
	Operator_FirewallMinimumOperandDofs( self, 3 );

	*result = operand0[2];
}
void Operator_TakeFourthComponent( void* operator, double* operand0, double* result ) {
	Operator* self = (Operator*) operator;
	
	Operator_FirewallUnary( self );
	Operator_FirewallResultDofs( self, 1 );
	Operator_FirewallMinimumOperandDofs( self, 4 );

	*result = operand0[3];
}
void Operator_TakeFifthComponent( void* operator, double* operand0, double* result ) {
	Operator* self = (Operator*) operator;

	Operator_FirewallUnary( self );
	Operator_FirewallResultDofs( self, 1 );
	Operator_FirewallMinimumOperandDofs( self, 5 );

	*result = operand0[4];
}

/** Binary Operator Functions */
void Operator_Addition( void* operator, double* operand0, double* operand1, double* result){
	Operator* self = (Operator*) operator;

	Operator_FirewallBinary( self );
	Operator_FirewallEqualOperandAndResultDofs( self );
	
	switch (self->operandDofs) {
		case 3:
			result[2] = operand0[2] + operand1[2];
		case 2:
			result[1] = operand0[1] + operand1[1];
		case 1:
			result[0] = operand0[0] + operand1[0]; break;
		default:
			Journal_Printf( self->errorStream, 
					"Error in %s for %s '%s': Incorrect number of degrees of freedom for what you are operating on.\n",
					__func__, self->type, self->name );
			abort();				
	}
}

void Operator_Subtraction( void* operator, double* operand0, double* operand1, double* result){
	Operator* self = (Operator*) operator;
	
	Operator_FirewallBinary( self );
	Operator_FirewallEqualOperandAndResultDofs( self );
	
	switch (self->operandDofs) {
		case 3:
			result[2] = operand0[2] - operand1[2];
		case 2:
			result[1] = operand0[1] - operand1[1];
		case 1:
			result[0] = operand0[0] - operand1[0]; break;
		default:
			Journal_Printf( self->errorStream, 
					"Error in %s for %s '%s': Incorrect number of degrees of freedom for what you are operating on.\n",
					__func__, self->type, self->name );
			abort();				
	}
}
	
void Operator_ScalarMultiplication( void* operator, double* operand0, double* operand1, double* result ) {
	Operator* self = (Operator*) operator;
	
	Operator_FirewallBinary( self );
	Operator_FirewallResultDofs( self, 1 );
	Operator_FirewallEqualOperandAndResultDofs( self );

	*result = (*operand0) * (*operand1);
}


void Operator_ScalarDivision( void* operator, double* operand0, double* operand1, double* result ) {
	Operator* self = (Operator*) operator;

	Operator_FirewallBinary( self );
	Operator_FirewallResultDofs( self, 1 );
	Operator_FirewallEqualOperandAndResultDofs( self );

	*result = (*operand0) / (*operand1);
}

void Operator_VectorScale( void* operator, double* operand0, double* operand1, double* result){
	Operator* self = (Operator*) operator;
	
	Operator_FirewallBinary( self );

	switch (self->operandDofs) {
		case 3:
			result[2] = (*operand0) * operand1[2];
		case 2:
			result[1] = (*operand0) * operand1[1];
		case 1:
			result[0] = (*operand0) * operand1[0]; break;
		default:
			Journal_Printf( self->errorStream, 
					"Error in %s for %s '%s': Incorrect number of degrees of freedom for what you are operating on.\n",
					__func__, self->type, self->name );
			abort();				
	}
}

void Operator_Rounding( void* operator, double* operand0, double* result ) {
	Operator*     self = (Operator*) operator;
	// TODO: read this in from somewhere instead of just hard-coding it....
	unsigned int  nSigFigsToRoundTo = 6;
	Index         val_I;

	Operator_FirewallUnary( self );

	for( val_I = 0; val_I < self->operandDofs ; val_I++ ) {	
		result[val_I] = StG_RoundDoubleToNSigFigs( operand0[val_I], nSigFigsToRoundTo );
	}
}


void Operator_HorizontalDivergence( void* operator, double* velocityGradient, double* result ) {
	Operator* self = (Operator*) operator;

	Operator_FirewallUnary( self );
	Operator_FirewallResultDofs( self, 1 );

	*result = velocityGradient[0] + velocityGradient[8];
}

void Operator_VerticalVorticity( void* operator, double* velocityGradient, double* result ) {
	Operator* self = (Operator*) operator;

	Operator_FirewallUnary( self );
	Operator_FirewallResultDofs( self, 1 );	

	*result = velocityGradient[2] - velocityGradient[6];
}

void Operator_Divergence( void* operator, double* velocityGradient, double* result ) {
	Operator* self = (Operator*) operator;
	
	Operator_FirewallUnary( self );

	TensorArray_GetTrace( velocityGradient, self->dim, result );
}

void Operator_TensorInnerProduct( void* operator, double* operand0, double* operand1, double* result ) {
	Operator* self = (Operator*)operator;
	
	Operator_FirewallBinary( self );

	if( self->dim == 2 ) {
		*result = operand0[0] * operand1[0] + operand0[1] * operand1[1] + ( 2 * operand0[2] * operand1[2] );
	} else {
		*result = operand0[0] * operand1[0]
		       	+ operand0[1] * operand1[1] 
			+ operand0[2] * operand1[2] 
			+ 2.0 * (operand0[3] * operand1[3] + operand0[4] * operand1[4] + operand0[5] * operand1[5] );
	}
}



/* HACK - This function isn't extensible */
Operator* Operator_NewFromName( 
		Name                                       name, 		
		Dof_Index                                  operandDofs,
		Dimension_Index                            dim ) 
{
	Func_Ptr                                   _carryOut;
	Dof_Index                                  resultDofs;
	Index                                      numberOfOperands;

	if ( ! strcasecmp( name, "VectorSquare" ) ) {
		resultDofs = 1;
		numberOfOperands = 1;
		_carryOut = Operator_VectorSquare;
	}
	else if ( ! strcasecmp( name, "Magnitude" ) ) {
		resultDofs = 1;
		numberOfOperands = 1;
		_carryOut = Operator_Magnitude;
	}
	else if ( ! strcasecmp( name, "TensorSymmetricPart" ) ) {
		resultDofs = StGermain_nSymmetricTensorVectorComponents( dim );
		numberOfOperands = 1;
		_carryOut = Operator_TensorSymmetricPart;
	}
	else if ( ! strcasecmp( name, "TensorAntisymmetricPart" ) ) { 
		resultDofs = dim * dim;
		numberOfOperands = 1;
		_carryOut = Operator_TensorAntisymmetricPart;
	}
	else if ( ! strcasecmp( name, "SymmetricTensor_Invariant" ) ){ 
		resultDofs = 1;
		numberOfOperands = 1;
		_carryOut = Operator_SymmetricTensor_Invariant;
	}
	else if ( ! strcasecmp( name, "SymmetricTensor_NegAverageTrace" ) ){ 
		resultDofs = 1;
		numberOfOperands = 1;
		_carryOut = Operator_SymmetricTensor_GetNegAverageTrace;
	}
	else if ( ! strcasecmp( name, "Tensor_NegAverageTrace" ) ){ 
		resultDofs = 1;
		numberOfOperands = 1;
		_carryOut = Operator_Tensor_GetNegAverageTrace;
	}
	else if ( ! strcasecmp( name, "TakeFirstComponent" ) ) {
		resultDofs = 1;
		numberOfOperands = 1;
		_carryOut = Operator_TakeFirstComponent;
	}
	else if ( ! strcasecmp( name, "TakeSecondComponent" ) ){ 
		resultDofs = 1;
		numberOfOperands = 1;
		_carryOut = Operator_TakeSecondComponent;
	}
	else if ( ! strcasecmp( name, "TakeThirdComponent" ) ){ 
		resultDofs = 1;
		numberOfOperands = 1;
		_carryOut = Operator_TakeThirdComponent;
	}
	else if ( ! strcasecmp( name, "Addition" ) ) {
		resultDofs = operandDofs;
		numberOfOperands = 2;
		_carryOut = Operator_Addition;
	}
	else if ( ! strcasecmp( name, "Subtraction" ) ){ 
		resultDofs = operandDofs;
		numberOfOperands = 2;
		_carryOut = Operator_Subtraction;
	}
	else if ( ! strcasecmp( name, "ScalarMultiplication" ) ){ 
		resultDofs = 1;
		numberOfOperands = 2;
		_carryOut = Operator_ScalarMultiplication;
	}
	else if ( ! strcasecmp( name, "ScalarDivision" ) ){ 
		resultDofs = 1;
		numberOfOperands = 2;
		_carryOut = Operator_ScalarDivision;
	}
	else if ( ! strcasecmp( name, "VectorScale" ) ){ 
		resultDofs = dim;
		numberOfOperands = 2;
		_carryOut = Operator_VectorScale;
	} 
	else if ( ! strcasecmp( name, "Rounding" ) ){ 
		resultDofs = operandDofs;
		numberOfOperands = 1;
		_carryOut = Operator_Rounding;
	} 
	else if ( ! strcasecmp( name, "TensorInnerProduct" ) ){ 
		resultDofs = 1;
		numberOfOperands = 2;
		_carryOut = Operator_TensorInnerProduct;
	}
	else if ( ! strcasecmp( name, "HorizontalDivergence" ) ){ 
		resultDofs = 1;
		numberOfOperands = 1;
		_carryOut = Operator_HorizontalDivergence;
	}
	else if ( ! strcasecmp( name, "VerticalVorticity" ) ){ 
		resultDofs = 1;
		numberOfOperands = 1;
		_carryOut = Operator_VerticalVorticity;
	}
	else if ( ! strcasecmp( name, "Divergence" ) ){ 
		resultDofs = 1;
		numberOfOperands = 1;
		_carryOut = Operator_Divergence;
	}
	else {
		resultDofs = 0;
		numberOfOperands = 0;
		_carryOut = NULL;
		
		Journal_Printf( Journal_Register( Error_Type, Operator_Type ), 
				"In func %s: Error trying to create operator from name '%s'.\n",
				__func__, name );
		abort();
	}

	return Operator_New( name, _carryOut, numberOfOperands, operandDofs, resultDofs, dim );
}


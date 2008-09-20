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
** $Id: Operator.h 3896 2006-11-20 04:59:16Z JulianGiordani $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


#ifndef __Discretisation_Utils_Operator_h__
#define __Discretisation_Utils_Operator_h__

typedef void (Operator_CarryOutUnaryOperationFunction)  ( void* operatorObject, double* operand0, double* result );
typedef void (Operator_CarryOutBinaryOperationFunction) ( void* operatorObject, double* operand0, double* operand1, double* result );

	/** Class contents */
	#define __Operator \
		/* Parent info */ \
		__Stg_Object \
		/* Virtual Functions */ \
		Func_Ptr                                   _carryOut;                            \
		/* Other information */ \
		Index                                      numberOfOperands;                     \
		Dof_Index                                  operandDofs;                          \
		Dof_Index                                  resultDofs;                           \
		Dimension_Index                            dim;                                  \
		Stream*                                    errorStream;                          \


	struct Operator { __Operator };

	extern const Type Operator_Type;

	Operator* Operator_New(		
		Name                                       name,
		Func_Ptr                                   _carryOut,
		Index                                      numberOfOperands,
		Dof_Index                                  operandDofs,
		Dof_Index                                  resultDofs,
		Dimension_Index                            dim ) ;

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
		Dimension_Index                            dim );

	void _Operator_Delete(void* operatorObject) ;
	void _Operator_Print(void* operatorObject, Stream* stream) ;
	void* _Operator_Copy( void* operatorObject, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) ;

	/* Carry Out Wrapper functions */
	void Operator_CarryOutUnaryOperation( void* operatorObject, double* operand0, double* result ) ;
	void Operator_CarryOutBinaryOperation( void* operatorObject, double* operand0, double* operand1, double* result ) ;

	/** Unary Operator Functions */
	void Operator_VectorSquare( void* operatorObject, double* operand0, double* result ) ;
	void Operator_Magnitude( void* operatorObject, double* operand0, double* result ) ;
	void Operator_TensorSymmetricPart( void* operatorObject, double* operand0, double* result );
	void Operator_SymmetricTensor_Invariant( void* operatorObject, double* operand0, double* result );
	void Operator_TensorAntisymmetricPart( void* operatorObject, double* operand0, double* result ) ;
	void Operator_Tensor_GetNegAverageTrace( void* operatorObject, double* operand0, double* result );
	void Operator_SymmetricTensor_GetNegAverageTrace( void* operatorObject, double* operand0, double* result );
	void Operator_TakeFirstComponent( void* operatorObject, double* operand0, double* result ) ;
	void Operator_TakeSecondComponent( void* operatorObject, double* operand0, double* result ) ;
	void Operator_TakeThirdComponent( void* operatorObject, double* operand0, double* result );
	void Operator_TakeFourthComponent( void* operatorObject, double* operand0, double* result );
	void Operator_TakeFifthComponent( void* operatorObject, double* operand0, double* result );

	/** Binary Operator Functions */
	void Operator_Addition( void* operatorObject, double* operand0, double* operand1, double* result);	
	void Operator_Subtraction( void* operatorObject, double* operand0, double* operand1, double* result);
	void Operator_ScalarMultiplication( void* operatorObject, double* operand0, double* operand1, double* result ) ;
	void Operator_ScalarDivision( void* operatorObject, double* operand0, double* operand1, double* result ) ;
	void Operator_VectorScale( void* operatorObject, double* operand0, double* operand1, double* result);
	void Operator_Rounding( void* operatorObject, double* operand0, double* result );
	void Operator_HorizontalDivergence( void* operatorObject, double* velocityGradient, double* result ) ;
	void Operator_VerticalVorticity( void* operatorObject, double* velocityGradient, double* result ) ;
	void Operator_Divergence( void* operatorObject, double* velocityGradient, double* result ) ;
	void Operator_TensorInnerProduct( void* operatorObject, double* operand0, double* operand1, double* result );

	Operator* Operator_NewFromName( 
		Name                                       name, 		
		Dof_Index                                  operandDofs,
		Dimension_Index                            dim ) ;

/* Error Checking Macros */
#define Operator_FirewallUnary( self ) \
	Journal_DFirewall(                                                                  \
			self->numberOfOperands == 1,                                                \
			self->errorStream,                                                          \
			"Error in %s for %s '%s':\n"                                                \
			"\tOperator expects %u operands but is trying to use a unary function.\n",  \
			__func__, self->type, self->name, self->numberOfOperands );

#define Operator_FirewallBinary( self ) \
	Journal_DFirewall(                                                                  \
			self->numberOfOperands == 2,                                                \
			self->errorStream,                                                          \
			"Error in %s for %s '%s':\n"                                                \
			"\tOperator expects %u operands but is trying to use a binary function.\n", \
			__func__, self->type, self->name, self->numberOfOperands );

#define Operator_FirewallResultDofs( self, expectedResultDofs )                                 \
	Journal_DFirewall(                                                                  \
			self->resultDofs == expectedResultDofs,                                             \
			self->errorStream,                                                          \
			"Error in %s for %s '%s':\n"                                                \
			"\tResult for Operator expects %u degrees of freedom but is using a function expects %u.\n", \
			__func__, self->type, self->name, self->resultDofs, expectedResultDofs );

#define Operator_FirewallMinimumOperandDofs( self, minimumOperandDofs )                                 \
	Journal_DFirewall(                                                                  \
			self->operandDofs >= minimumOperandDofs,                                             \
			self->errorStream,                                                          \
			"Error in %s for %s '%s':\n"                                                \
			"\tOperand has %u degrees of freedom but this function needs at least %u.\n", \
			__func__, self->type, self->name, self->operandDofs, minimumOperandDofs );


#define Operator_FirewallEqualOperandAndResultDofs( self )                                 \
	Journal_DFirewall(                                                                  \
			self->operandDofs == self->resultDofs,                                             \
			self->errorStream,                                                          \
			"Error in %s for %s '%s':\n"                                                \
			"\tFunction expects operands and result to have the same number of degrees of freedom.\n", \
			__func__, self->type, self->name );
	
#endif

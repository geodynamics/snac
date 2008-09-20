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
*/
/** \file
**  Role:
**	Represents a Variable that is a discretisation of a swarm-like physical property over a spatial domain.
**
** Assumptions:
**	The function interfaces assume the spatially disc. variable is stored as a double
**	(it can't be an int because its an approximation to a continuous variable right?)
**
** Comments:
**	Abstract class that defines an interface to use when accessing spatially discretised
**	swarm variables.
**
**	This means that e.g. visualisation code can be written to use this class,
**	and doesn't have to worry exactly how the variable is discretised - that will be
**	done by the back-end implementation of this class.
**
**	The name comes from the definition of "swarm" in the physics domain: A region of space
**	characterized by a physical property, such as gravitational or electromagnetic force or
**	fluid pressure, having a determinable value at every point in the region.
**
**	TODO: should it have a ptr to the Variable its based on at this level?
**	doesn't make sense at the moment as the FeVariable used a \
**	doflayout rather than a variable -> but may in future... 
**
**	$Id: OperatorSwarmVariable.h 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Utils_OperatorSwarmVariable_h__
#define __Discretisation_Utils_OperatorSwarmVariable_h__

	#define MAX_DOF 9

	/** Textual name of this class */
	extern const Type OperatorSwarmVariable_Type;

	typedef void (OperatorSwarmVariable_UnaryOperatorFunction)  ( void* swarmVariable, double* value0, double* result );
	typedef void (OperatorSwarmVariable_BinaryOperatorFunction) ( void* swarmVariable, double* value0, double* value1, double* result );
	
	/** OperatorSwarmVariable contents */
	#define __OperatorSwarmVariable \
		/* Parent info */ \
		__SwarmVariable \
		\
		/* Other info */ \
		Operator*                                                   _operator;           \
		Index                                                       swarmVariableCount;  \
		SwarmVariable**                                             swarmVariableList;   \


	struct OperatorSwarmVariable { __OperatorSwarmVariable };	

	/** Shortcut constructors */
	OperatorSwarmVariable* OperatorSwarmVariable_NewUnary(
		Name                                               name,
		void*                                              _swarmVariable,
		Name                                               operatorName );

	OperatorSwarmVariable* OperatorSwarmVariable_NewBinary(
		Name                                               name,
		void*                                              _swarmVariable1,
		void*                                              _swarmVariable2,
		Name                                               operatorName );
	
	/* Public Constructor */
	void* _OperatorSwarmVariable_DefaultNew( Name name );

	OperatorSwarmVariable* OperatorSwarmVariable_New( 
		Name                                               name,
		SwarmVariable_ValueAtFunction*                     valueAt,
		Name                                               operatorName,
		Index                                              swarmVariableCount,
		SwarmVariable**                                    swarmVariableList );

	/** Private Constructor */
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
			Name                                               name );

	void _OperatorSwarmVariable_Init( void* swarmVariable, Name operatorName, Index swarmVariableCount, SwarmVariable** swarmVariableList ) ;

	void _OperatorSwarmVariable_Delete( void* variable ) ;
	void _OperatorSwarmVariable_Print( void* _swarmVariable, Stream* stream ) ;


	void* _OperatorSwarmVariable_Copy( void* swarmVariable, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) ;
	void _OperatorSwarmVariable_Construct( void* swarmVariable, Stg_ComponentFactory* cf, void* data ) ;
	void _OperatorSwarmVariable_Build( void* swarmVariable, void* data ) ;
	void _OperatorSwarmVariable_Execute( void* variable, void* data ) ;
	void _OperatorSwarmVariable_Destroy( void* variable, void* data ) ;
	void _OperatorSwarmVariable_Initialise( void* variable, void* data ) ;

	void _OperatorSwarmVariable_ValueAt( void* swarmVariable, Particle_Index lParticle_I, double* value ) ;

	double _OperatorSwarmVariable_GetMinGlobalMagnitude( void* swarmVariable ) ;
	double _OperatorSwarmVariable_GetMaxGlobalMagnitude( void* swarmVariable ) ;

	void _OperatorSwarmVariable_UnaryValueAt( void* swarmVariable, Particle_Index lParticle_I, double* value ) ;
	void _OperatorSwarmVariable_BinaryValueAt( void* swarmVariable, Particle_Index lParticle_I, double* value ) ;

	void OperatorSwarmVariable_UnaryOperator( void* swarmVariable, double* swarmValue0, double* value ) ;
	void OperatorSwarmVariable_BinaryOperator( void* swarmVariable, double* swarmValue0, double* swarmValue1, double* value ) ;

	/** Operator Functions */
	void OperatorSwarmVariable_VectorSquare( void* swarmVariable, double* swarmValue0, double* value ) ;

	void OperatorSwarmVariable_VectorMag( void* swarmVariable, double* swarmValue0, double* value ) ;
	void OperatorSwarmVariable_Abs( void* swarmVariable, double* swarmValue0, double* value ) ;

	/** Symmetric Part of tensor Operators */
	void OperatorSwarmVariable_TensorSymmetricPart( void* swarmVariable, double* swarmValue0, double* value ) ;
	void OperatorSwarmVariable_SymmetricTensor_Invariant( void* swarmVariable, double* swarmValue0, double* value ) ;
	void OperatorSwarmVariable_TensorAntisymmetricPart( void* swarmVariable, double* swarmValue0, double* value ) ;

	/** Operators to take specific componets of a vector */
	void OperatorSwarmVariable_TakeFirstComponent( void* swarmVariable, double* swarmValue0, double* value ) ;
	void OperatorSwarmVariable_TakeSecondComponent( void* swarmVariable, double* swarmValue0, double* value ) ;
	void OperatorSwarmVariable_TakeThirdComponent( void* swarmVariable, double* swarmValue0, double* value ) ;

	/** Binary Operator Functions */
	void OperatorSwarmVariable_ScalarAddition( void* swarmVariable, double* swarmValue0, double* swarmValue1, double* value ) ;
	void OperatorSwarmVariable_VectorAddition( void* swarmVariable, double* swarmValue0, double* swarmValue1, double* value) ;
	void OperatorSwarmVariable_ScalarSubtraction( void* swarmVariable, double* swarmValue0, double* swarmValue1, double* value ) ;
	void OperatorSwarmVariable_VectorSubtraction( void* swarmVariable, double* swarmValue0, double* swarmValue1, double* value) ;
	void OperatorSwarmVariable_ScalarMultiplication( void* swarmVariable, double* swarmValue0, double* swarmValue1, double* value ) ;
	void OperatorSwarmVariable_ScalarDivision( void* swarmVariable, double* swarmValue0, double* swarmValue1, double* value ) ;

	void OperatorSwarmVariable_VectorScale( void* swarmVariable, double* swarmValue0, double* swarmValue1, double* value);

	/** TODO HACK - This function will be obselete when function registers come to town */
	Func_Ptr OperatorSwarmVariable_GetOperatorByName( void* swarmVariable, Name name ) ;
#endif /* __Discretisation_Utils_OperatorSwarmVariable_h__ */

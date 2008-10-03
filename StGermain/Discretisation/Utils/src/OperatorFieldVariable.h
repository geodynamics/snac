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
**	Represents a Variable that is a discretisation of a field-like physical property over a spatial domain.
**
** Assumptions:
**	The function interfaces assume the spatially disc. variable is stored as a double
**	(it can't be an int because its an approximation to a continuous variable right?)
**
** Comments:
**	Abstract class that defines an interface to use when accessing spatially discretised
**	field variables.
**
**	This means that e.g. visualisation code can be written to use this class,
**	and doesn't have to worry exactly how the variable is discretised - that will be
**	done by the back-end implementation of this class.
**
**	The name comes from the definition of "field" in the physics domain: A region of space
**	characterized by a physical property, such as gravitational or electromagnetic force or
**	fluid pressure, having a determinable value at every point in the region.
**
**	TODO: should it have a ptr to the Variable its based on at this level?
**	doesn't make sense at the moment as the FeVariable used a \
**	doflayout rather than a variable -> but may in future... 
**
**	$Id: OperatorFieldVariable.h 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Utils_OperatorFieldVariable_h__
#define __Discretisation_Utils_OperatorFieldVariable_h__

	#define MAX_FIELD_COMPONENTS 9

	/** Textual name of this class */
	extern const Type OperatorFieldVariable_Type;

	typedef void (OperatorFieldVariable_UnaryOperatorFunction)  ( void* fieldVariable, double* value0, double* result );
	typedef void (OperatorFieldVariable_BinaryOperatorFunction) ( void* fieldVariable, double* value0, double* value1, double* result );
	
	/** OperatorFieldVariable contents */
	#define __OperatorFieldVariable \
		/* General info */ \
		__FieldVariable \
		\
		/* Virtual info */ \
		Operator*                                                   _operator;           \
		Index                                                       fieldVariableCount;  \
		FieldVariable**                                             fieldVariableList;   \


	struct OperatorFieldVariable { __OperatorFieldVariable };	

	/** Shortcut constructors */
	OperatorFieldVariable* OperatorFieldVariable_NewUnary(
		Name                                               name,
		void*                                              _fieldVariable,
		Name                                               operatorName );

	OperatorFieldVariable* OperatorFieldVariable_NewBinary(
		Name                                               name,
		void*                                              _fieldVariable1,
		void*                                              _fieldVariable2,
		Name                                               operatorName );
	
	/* Public Constructor */
	OperatorFieldVariable* OperatorFieldVariable_DefaultNew();

	OperatorFieldVariable* OperatorFieldVariable_New( 
		Name                                               name,
		FieldVariable_InterpolateValueAtFunction*          interpolateValueAt,
		Name                                               operatorName,
		Index                                              fieldVariableCount,
		FieldVariable**                                    fieldVariableList,
		Dimension_Index                                    dim,
		Bool                                               isCheckpointedAndReloaded,
		MPI_Comm                                           communicator,
		FieldVariable_Register*                            fV_Register ) ;

	/** Private Constructor */
	OperatorFieldVariable* _OperatorFieldVariable_New(
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
			Name                                               name,
			Bool												initFlag,
			FieldVariable_GetValueFunction*	                   _getMinGlobalFieldMagnitude,
			FieldVariable_GetValueFunction*                    _getMaxGlobalFieldMagnitude,
			FieldVariable_GetCoordFunction*                    _getMinAndMaxLocalCoords,
			FieldVariable_GetCoordFunction*                    _getMinAndMaxGlobalCoords,
			FieldVariable_InterpolateValueAtFunction*          interpolateValueAt,
			Name                                               operatorName,
			Index                                              fieldVariableCount,
			FieldVariable**                                    fieldVariableList,
			Dimension_Index                                    dim,
			Bool                                               isCheckpointedAndReloaded,
			MPI_Comm                                           communicator,
			FieldVariable_Register*                            fV_Register );

	void _OperatorFieldVariable_Init( void* fieldVariable, Name operatorName, Index fieldVariableCount, FieldVariable** fieldVariableList ) ;

	void _OperatorFieldVariable_Delete( void* variable ) ;
	void _OperatorFieldVariable_Print( void* _fieldVariable, Stream* stream ) ;


	void* _OperatorFieldVariable_Copy( void* fieldVariable, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) ;
	void _OperatorFieldVariable_Construct( void* fieldVariable, Stg_ComponentFactory* cf, void* data ) ;
	void _OperatorFieldVariable_Build( void* fieldVariable, void* data ) ;
	void _OperatorFieldVariable_Execute( void* variable, void* data ) ;
	void _OperatorFieldVariable_Destroy( void* variable, void* data ) ;
	void _OperatorFieldVariable_Initialise( void* variable, void* data ) ;

	InterpolationResult _OperatorFieldVariable_InterpolateValueAt( void* fieldVariable, Coord coord, double* value ) ;

	double _OperatorFieldVariable_GetMinLocalFieldMagnitude( void* fieldVariable ) ;
	double _OperatorFieldVariable_GetMaxLocalFieldMagnitude( void* fieldVariable ) ;
	void  _OperatorFieldVariable_GetMinAndMaxLocalCoords( void* fieldVariable, Coord min, Coord max ) ;
	void  _OperatorFieldVariable_GetMinAndMaxGlobalCoords( void* fieldVariable, Coord min, Coord max ) ;

	InterpolationResult _OperatorFieldVariable_InterpolateValueAt( void* fieldVariable, Coord coord, double* value ) ;
	
	InterpolationResult OperatorFieldVariable_UnaryInterpolationFunc( void* fieldVariable, Coord coord, double* value ) ;
	InterpolationResult OperatorFieldVariable_BinaryInterpolationFunc( void* fieldVariable, Coord coord, double* value ) ;

	void OperatorFieldVariable_UnaryOperator( void* fieldVariable, double* fieldValue0, double* value ) ;
	void OperatorFieldVariable_BinaryOperator( void* fieldVariable, double* fieldValue0, double* fieldValue1, double* value ) ;

#endif /* __Discretisation_Utils_OperatorFieldVariable_h__ */

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003, Victorian Partnership for Advanced Computing (VPAC) Ltd, 110 Victoria Street, Melbourne, 3053, Australia.
**
** Authors:
**	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
**
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2, or (at your option) any
** later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**
*/
/** \file
**  Role:
**
** Assumptions:
**
** Comments:
**
** $Id: ExchangerVC.h 1113 2004-03-30 23:58:28Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __SnacExchangerForceBC_ExchangerVC_h__
#define __SnacExchangerForceBC_ExchangerVC_h__
	
	
	extern const Type ExchangerVC_Type;
	
	
	#define __ExchangerVC \
		/* General info */ \
		__VariableCondition \
		\
		/* Virtual info */ \
		\
		/* Stg_Class info */ \
		Name				varName; \
		IndexSet*			valuesSet; \
		IndexSet_Index			valuesSize; \
		VariableCondition_Value*	values; \
		Index*				valuesIndices;
	struct _ExchangerVC { __ExchangerVC };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructor
	*/
	
	VariableCondition*		ExchangerVC_Factory(
						Variable_Register*				variable_Register, 
						ConditionFunction_Register*			conFunc_Register, 
						Dictionary*					dictionary,
						void*						data );
	
	ExchangerVC*			ExchangerVC_DefaultNew( Name name );

	ExchangerVC*			ExchangerVC_New( 
						Variable_Register*				variable_Register, 
						ConditionFunction_Register*			conFunc_Register,
						Dictionary*					dictionary,
						Name						varName );
	
	void				ExchangerVC_Init(
						ExchangerVC*					self,
						Variable_Register*				variable_Register, 
						ConditionFunction_Register*			conFunc_Register,
						Dictionary*					dictionary,
						Name						varName );
	
	ExchangerVC*			_ExchangerVC_New( 
						SizeT						_sizeOfSelf, 
						Type						type,
						Stg_Class_DeleteFunction*				_delete,
						Stg_Class_PrintFunction*				_print,
						Stg_Class_CopyFunction*				_copy, 
						Stg_Component_DefaultConstructorFunction*		_defaultConstructor,
						Stg_Component_ConstructFunction*			_construct,
						Stg_Component_BuildFunction*			_build,
						Stg_Component_InitialiseFunction*			_initialise,
						Stg_Component_ExecuteFunction*			_execute,
						Stg_Component_ExecuteFunction*			_destroy,
						Name						varName,
						Bool						initFlag,
						VariableCondition_BuildSelfFunc*		_buildSelf, 
						VariableCondition_PrintConciseFunc*		_printConcise,
						VariableCondition_ReadDictionaryFunc*		_readDictionary,
						VariableCondition_GetSetFunc*			_getSet,
						VariableCondition_GetVariableCountFunc*		_getVariableCount,
						VariableCondition_GetVariableIndexFunc*		_getVariableIndex,
						VariableCondition_GetValueIndexFunc*		_getValueIndex,
						VariableCondition_GetValueCountFunc*		_getValueCount,
						VariableCondition_GetValueFunc*			_getValue,
						Variable_Register*				variable_Register, 
						ConditionFunction_Register*			conFunc_Register,
						Dictionary*					dictionary );
	
	void				_ExchangerVC_Init(
						void*						exchangerVC,
						Name						varName );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** General virtual functions
	*/
	
	void				_ExchangerVC_Delete( void* exchangerVC );
	
	void				_ExchangerVC_Print( void* exchangerVC, Stream* stream );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Macros
	*/
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	void				_ExchangerVC_ReadDictionary( void* variableCondition, void* dictionary );
	
	IndexSet*			_ExchangerVC_GetSet( void* variableCondition );
	
	VariableCondition_VariableIndex	_ExchangerVC_GetVariableCount( void* variableCondition, Index set_I );
	
	Variable_Index			_ExchangerVC_GetVariableIndex(
						void*				variableCondition,
						Index				set_I, 
						VariableCondition_VariableIndex	var_I);
						
	VariableCondition_ValueIndex	_ExchangerVC_GetValueIndex(
						void*				variableCondition, 
						Index				set_I, 
						VariableCondition_VariableIndex	var_I );
						
	VariableCondition_ValueIndex	_ExchangerVC_GetValueCount( void* variableCondition );
	
	VariableCondition_Value		_ExchangerVC_GetValue( void* variableCondition, VariableCondition_ValueIndex valIndex );
	
	void				_ExchangerVC_PrintConcise( void* variableCondition, Stream* stream );
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Build functions
	*/
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Functions
	*/

	
#endif /* __SnacExchangerForceBC_ExchangerVC_h__ */

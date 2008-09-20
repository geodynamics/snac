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
**
** Assumptions:
**
** Comments:
**
** $Id: InnerWallVC.h 3291 2005-10-18 00:05:33Z AlanLo $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Utils_InnerWallVC_h__
#define __Discretisation_Utils_InnerWallVC_h__
	

	extern const Type InnerWallVC_Type;
	
	extern const char* InnerWallVC_InnerWallEnumToStr[InnerWallVC_InnerWall_Size];
	
	#define __InnerWallVC_Entry \
		Name				varName; \
		VariableCondition_Value		value; \
		
	struct _InnerWallVC_Entry { __InnerWallVC_Entry };
	
	
	#define __InnerWallVC \
		/* General info */ \
		__VariableCondition \
		\
		/* Virtual info */ \
		\
		/* Stg_Class info */ \
		Name				_dictionaryEntryName; \
		InnerWallVC_InnerWall			_innerWall; \
		InnerWallVC_Entry_Index		_entryCount; \
		InnerWallVC_Entry*			_entryTbl; \
		Mesh*				_mesh;

	struct _InnerWallVC { __InnerWallVC };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructor
	*/
	
	VariableCondition*	InnerWallVC_Factory(
						Variable_Register*				variable_Register, 
						ConditionFunction_Register*			conFunc_Register, 
						Dictionary*					dictionary,
						void*						data );
	
	InnerWallVC*				InnerWallVC_DefaultNew( Name name );

	InnerWallVC*				InnerWallVC_New(
						Name						name,
						Name						_dictionaryEntryName, 
						Variable_Register*				variable_Register, 
						ConditionFunction_Register*			conFunc_Register, 
						Dictionary*					dictionary,
						void*						_mesh );
	
	void				InnerWallVC_Init(
						InnerWallVC*						self,
						Name						name,
						Name						_dictionaryEntryName, 
						Variable_Register*				variable_Register, 
						ConditionFunction_Register*			conFunc_Register, 
						Dictionary*					dictionary,
						void*						_mesh );
	
	InnerWallVC*				_InnerWallVC_New( 
						SizeT						_sizeOfSelf, 
						Type						type,
						Stg_Class_DeleteFunction*				_delete,
						Stg_Class_PrintFunction*				_print,
						Stg_Class_CopyFunction*				_copy,
						Stg_Component_DefaultConstructorFunction*	_defaultConstructor,
						Stg_Component_ConstructFunction*			_construct,
						Stg_Component_BuildFunction*			_build,
						Stg_Component_InitialiseFunction*			_initialise,
						Stg_Component_ExecuteFunction*			_execute,
						Stg_Component_DestroyFunction*			_destroy,
						Name						name, 
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
						Name						_dictionaryEntryName, 
						Variable_Register*				variable_Register, 
						ConditionFunction_Register*			conFunc_Register, 
						Dictionary*					dictionary,
						void*						_mesh );
	
	void				_InnerWallVC_Init(
						void*						innerWallVC, 
						Name						_dictionaryEntryName, 
						void*						_mesh );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** General virtual functions
	*/
	
	void				_InnerWallVC_Delete( void* innerWallVC );
	
	void				_InnerWallVC_Print( void* innerWallVC, Stream* stream );
	
	/* Copy */
	#define InnerWallVC_Copy( self ) \
		(VariableCondition*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define InnerWallVC_Copy( self ) \
		(VariableCondition*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	
	void* _InnerWallVC_Copy( void* innerWallVC, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap );
	
	void				_InnerWallVC_Build(  void* innerWallVC, void* data );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Macros
	*/
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	void				_InnerWallVC_Construct( void* innerWallVC, Stg_ComponentFactory* cf, void* data );
	
	void				_InnerWallVC_BuildSelf( void* innerWallVC, void* data );
	
	void				_InnerWallVC_ReadDictionary( void* variableCondition, void* dictionary );
	
	IndexSet*			_InnerWallVC_GetSet( void* variableCondition );
	
	VariableCondition_VariableIndex	_InnerWallVC_GetVariableCount( void* variableCondition, Index globalIndex );
	
	Variable_Index			_InnerWallVC_GetVariableIndex(
						void*				variableCondition,
						Index				globalIndex, 
						VariableCondition_VariableIndex	varIndex );
						
	VariableCondition_ValueIndex	_InnerWallVC_GetValueIndex(
						void*				variableCondition, 
						Index				globalIndex, 
						VariableCondition_VariableIndex	varIndex );
						
	VariableCondition_ValueIndex	_InnerWallVC_GetValueCount( void* variableCondition );
	
	VariableCondition_Value		_InnerWallVC_GetValue( void* variableCondition, VariableCondition_ValueIndex valIndex );
	
	void				_InnerWallVC_PrintConcise( void* variableCondition, Stream* stream );
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Build functions
	*/
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Functions
	*/

	
#endif /* __Discretisation_Utils_InnerWallVC_h__ */

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
** This file may be distributed under the terms of the VPAC Public License
** as defined by VPAC of Australia and appearing in the file
** LICENSE.VPL included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
*/
/** \file
**  Role:
**
** Assumptions:
**
** Comments:
**
** $Id: FrictionVC.h 3291 2005-10-18 00:05:33Z AlanLo $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* Modified 2006 Walter Landry to implement Friction VC's */

#ifndef __Gale_BoundaryConditions_FrictionVC_h__
#define __Gale_BoundaryConditions_FrictionVC_h__
	

	extern const Type FrictionVC_Type;
	
	extern const char* FrictionVC_WallEnumToStr[FrictionVC_Wall_Size];
	
	#define __FrictionVC_Entry \
		Name				varName; \
		VariableCondition_Value		value; \
		
	struct _FrictionVC_Entry { __FrictionVC_Entry };
	
	
	#define __FrictionVC \
		/* General info */ \
		__VariableCondition \
		\
		/* Virtual info */ \
		\
		/* Stg_Class info */ \
		Name				_dictionaryEntryName; \
		FrictionVC_Wall			_wall; \
		FrictionVC_Entry_Index		_entryCount; \
		FrictionVC_Entry*			_entryTbl; \
		Mesh*				_mesh;

	struct _FrictionVC { __FrictionVC };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructor
	*/
	
	VariableCondition*	FrictionVC_Factory(
						Variable_Register*				variable_Register, 
						ConditionFunction_Register*			conFunc_Register, 
						Dictionary*					dictionary,
						void*						data );
	
	FrictionVC*				FrictionVC_DefaultNew( Name name );

	FrictionVC*				FrictionVC_New(
						Name						name,
						Name						_dictionaryEntryName, 
						Variable_Register*				variable_Register, 
						ConditionFunction_Register*			conFunc_Register, 
						Dictionary*					dictionary,
						void*						_mesh );
	
	void				FrictionVC_Init(
						FrictionVC*						self,
						Name						name,
						Name						_dictionaryEntryName, 
						Variable_Register*				variable_Register, 
						ConditionFunction_Register*			conFunc_Register, 
						Dictionary*					dictionary,
						void*						_mesh );
	
	FrictionVC*				_FrictionVC_New( 
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
	
	void				_FrictionVC_Init(
						void*						wallVC, 
						Name						_dictionaryEntryName, 
						void*						_mesh );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** General virtual functions
	*/
	
	void				_FrictionVC_Delete( void* wallVC );
	
	void				_FrictionVC_Print( void* wallVC, Stream* stream );
	
	/* Copy */
	#define FrictionVC_Copy( self ) \
		(VariableCondition*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define FrictionVC_Copy( self ) \
		(VariableCondition*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	
	void* _FrictionVC_Copy( void* wallVC, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap );
	
	void				_FrictionVC_Build(  void* wallVC, void* data );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Macros
	*/
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	void				_FrictionVC_Construct( void* wallVC, Stg_ComponentFactory* cf, void* data );
	
	void				_FrictionVC_BuildSelf( void* wallVC, void* data );
	
	void				_FrictionVC_ReadDictionary( void* variableCondition, void* dictionary );
	
	IndexSet*			_FrictionVC_GetSet( void* variableCondition );
	
	VariableCondition_VariableIndex	_FrictionVC_GetVariableCount( void* variableCondition, Index globalIndex );
	
	Variable_Index			_FrictionVC_GetVariableIndex(
						void*				variableCondition,
						Index				globalIndex, 
						VariableCondition_VariableIndex	varIndex );
						
	VariableCondition_ValueIndex	_FrictionVC_GetValueIndex(
						void*				variableCondition, 
						Index				globalIndex, 
						VariableCondition_VariableIndex	varIndex );
						
	VariableCondition_ValueIndex	_FrictionVC_GetValueCount( void* variableCondition );
	
	VariableCondition_Value		_FrictionVC_GetValue( void* variableCondition, VariableCondition_ValueIndex valIndex );
	
	void				_FrictionVC_PrintConcise( void* variableCondition, Stream* stream );
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Build functions
	*/
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Functions
	*/

	
#endif /* __Discretisation_Utils_FrictionVC_h__ */

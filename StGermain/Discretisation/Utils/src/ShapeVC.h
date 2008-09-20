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
** $Id: ShapeVC.h 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Utils_ShapeVC_h__
#define __Discretisation_Utils_ShapeVC_h__
	
	extern const Type ShapeVC_Type;
	
	#define __ShapeVC_Entry \
		Name                                        varName; \
		VariableCondition_Value                     value; \
	
	typedef struct { __ShapeVC_Entry } ShapeVC_Entry;
	
	#define __ShapeVC \
		/* General info */ \
		__VariableCondition \
		\
		/* Virtual info */ \
		\
		/* Stg_Class info */ \
		Name                                        _dictionaryEntryName; \
		ShapeVC_Entry_Index                         _entryCount;          \
		ShapeVC_Entry*                              _entryTbl;            \
		Mesh*                                       _mesh;                \
		Stg_Shape*                                  _shape;               \
		Name                                        shapeName;

	struct ShapeVC { __ShapeVC };
	
	
	/*-----------------------------------------------------------------------------------------------------------------
	** Constructor
	*/
	
	VariableCondition* ShapeVC_Factory(
		Variable_Register*                          variable_Register, 
		ConditionFunction_Register*                 conFunc_Register, 
		Dictionary*                                 dictionary,
		void*                                       data );
	

	ShapeVC* ShapeVC_New(
		Name                                        name,
		Name                                        _dictionaryEntryName, 
		Variable_Register*                          variable_Register, 
		ConditionFunction_Register*                 conFunc_Register, 
		Dictionary*	                                dictionary,
		void*                                       _mesh );
	
	ShapeVC* _ShapeVC_New( 
		SizeT                                       _sizeOfSelf, 
		Type                                        type,
		Stg_Class_DeleteFunction*                   _delete,
		Stg_Class_PrintFunction*                    _print,
		Stg_Class_CopyFunction*                     _copy,
		Stg_Component_DefaultConstructorFunction*   _defaultConstructor,
		Stg_Component_ConstructFunction*            _construct,
		Stg_Component_BuildFunction*                _build,
		Stg_Component_InitialiseFunction*           _initialise,
		Stg_Component_ExecuteFunction*              _execute,
		Stg_Component_DestroyFunction*              _destroy,
		VariableCondition_BuildSelfFunc*            _buildSelf, 
		VariableCondition_PrintConciseFunc*         _printConcise,
		VariableCondition_ReadDictionaryFunc*       _readDictionary,
		VariableCondition_GetSetFunc*               _getSet,
		VariableCondition_GetVariableCountFunc*     _getVariableCount,
		VariableCondition_GetVariableIndexFunc*     _getVariableIndex,
		VariableCondition_GetValueIndexFunc*        _getValueIndex,
		VariableCondition_GetValueCountFunc*        _getValueCount,
		VariableCondition_GetValueFunc*             _getValue,
		Name                                        name );
	
	void _ShapeVC_Init(
		void*                                       shapeVC, 
		Name                                        _dictionaryEntryName, 
		void*                                       _mesh );
	
	/* Stg_Class Virtual Functions */
	void _ShapeVC_Delete( void* shapeVC );
	void _ShapeVC_Print( void* shapeVC, Stream* stream );
	void* _ShapeVC_Copy( void* shapeVC, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap );
	#define ShapeVC_Copy( self ) \
		(VariableCondition*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define ShapeVC_Copy( self ) \
		(VariableCondition*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	
	/* Stg_Component Virtual Functions */
	void* _ShapeVC_DefaultNew( Name name );
	void _ShapeVC_Build(  void* shapeVC, void* data );
	void _ShapeVC_Construct( void* shapeVC, Stg_ComponentFactory* cf, void* data );
	
	/* VariableCondition Virtual Functions */
	void _ShapeVC_BuildSelf( void* shapeVC, void* data );
	void _ShapeVC_PrintConcise( void* variableCondition, Stream* stream );
	void _ShapeVC_ReadDictionary( void* variableCondition, void* dictionary );
	IndexSet* _ShapeVC_GetSet( void* variableCondition );
	VariableCondition_VariableIndex	_ShapeVC_GetVariableCount( void* variableCondition, Index globalIndex );
	Variable_Index _ShapeVC_GetVariableIndex(
		void*                                       variableCondition,
		Index                                       globalIndex, 
		VariableCondition_VariableIndex             varIndex );
						
	VariableCondition_ValueIndex _ShapeVC_GetValueIndex(
		void*                                       variableCondition, 
		Index                                       globalIndex, 
		VariableCondition_VariableIndex             varIndex );
						
	VariableCondition_ValueIndex _ShapeVC_GetValueCount( void* variableCondition );
	
	VariableCondition_Value	_ShapeVC_GetValue( void* variableCondition, VariableCondition_ValueIndex valIndex );
	
	
#endif

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
**	Types for the Automation layer
**
** Assumptions:
**	None as yet.
**
** Comments:
**	None as yet.
**
** $Id: types.h 3614 2006-06-01 08:58:48Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Base_Automation_types_h__
#define __Base_Automation_types_h__

	/* types/classes */
	typedef struct Stg_Component			Stg_Component;
	typedef struct Stg_ComponentMeta                Stg_ComponentMeta;
	typedef struct Stg_ComponentMeta_Value          Stg_ComponentMeta_Value;
	typedef struct Stg_ComponentRegisterElement     Stg_ComponentRegisterElement;
	typedef struct Stg_ComponentRegister            Stg_ComponentRegister;
	typedef struct LiveComponentRegister            LiveComponentRegister;
	typedef struct Stg_ComponentFactory		Stg_ComponentFactory;
	typedef struct DocumentationComponentFactory    DocumentationComponentFactory;
	typedef struct HierarchyTable                   HierarchyTable;
	typedef struct Stg_CallGraph			Stg_CallGraph;
	typedef struct _Variable			Variable;
	typedef struct _Variable_Register		Variable_Register;
	typedef struct VariableDumpStream		VariableDumpStream;
	typedef struct _VariableCondition		VariableCondition;
	typedef struct _SetVC				SetVC;
	typedef struct _CompositeVC			CompositeVC;
	typedef struct _VariableCondition_Register	VariableCondition_Register;
	typedef struct _ConditionFunction		ConditionFunction;
	typedef struct _ConditionFunction_Register	ConditionFunction_Register;

	typedef struct _VariableAllVC_Entry		VariableAllVC_Entry;
	typedef struct _VariableAllVC			VariableAllVC;
	typedef Index					VariableAllVC_Entry_Index;

	/* Variable_Register types */
	typedef Index					Variable_Set_Index;
	typedef Index					Variable_Index;
	typedef Index					Dof_Index;
	
	/* VariableCondition_Register types */
	typedef struct _VariableCondition_Register_Entry VariableCondition_Register_Entry;
	
	/* VariableCondition types */
	typedef enum
	{
		VC_ValueType_Double = 1,
		VC_ValueType_Int,
		VC_ValueType_Short,
		VC_ValueType_Char,
		VC_ValueType_Ptr,
		VC_ValueType_DoubleArray,
		VC_ValueType_CFIndex
	} VariableCondition_ValueType;
	
	typedef struct SearchCriteria_t{
		Name type;
		Name version;	
	}SearchCriteria;

	typedef Index					VariableCondition_Index;
	typedef struct _VariableCondition_Value		VariableCondition_Value;
	typedef struct _VariableCondition_Tuple		VariableCondition_Tuple;
	typedef Index					VariableCondition_ValueIndex;
	typedef Index					ConditionFunction_Index;
	typedef Index					VariableCondition_VariableIndex;

	typedef struct _SetVC_Entry			SetVC_Entry;
	typedef Index					SetVC_Entry_Index;
	
	/* CompositeVC types */
	typedef Index					CompositeVC_ItemIndex;
	
	typedef void* (Stg_Component_Constructor) ( Name );
	typedef void (Stg_Component_LoadFromDict) ( void*, Dictionary*, Dictionary*, Stg_ObjectList* );

#endif /* __Base_Automation_types_h__ */

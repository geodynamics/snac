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
** $Id: Init.c 3881 2006-10-26 03:14:19Z KathleenHumble $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdarg.h>

#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"

#include "types.h"
#include "shortcuts.h"
#include "Init.h"
#include "VariableCondition_Register.h"
#include "ConditionFunction_Register.h"
#include "Variable_Register.h"
#include "Stg_Component.h"
#include "Stg_ComponentRegister.h"
#include "Stg_ComponentFactory.h"
#include "Variable.h"
#include "VariableCondition.h"
#include "SetVC.h"
#include "CompositeVC.h"
#include "ConditionFunction.h"
#include "VariableAllVC.h"
#include "HierarchyTable.h"
#include "CallGraph.h"

#include "VariableDumpStream.h"

#include <stdio.h>

Bool BaseAutomation_Init( int* argc, char** argv[] ) 
{
	Stream* typedStream;
	
	Journal_Printf( Journal_Register( DebugStream_Type, "Context" ), "In: %s\n", __func__ ); /* DO NOT CHANGE OR REMOVE */
	
	variableCondition_Register = VariableCondition_Register_New();
	VariableCondition_Register_Add( variableCondition_Register, SetVC_Type, SetVC_Factory );
	VariableCondition_Register_Add( variableCondition_Register, CompositeVC_Type, CompositeVC_Factory );
	VariableCondition_Register_Add( variableCondition_Register, VariableAllVC_Type, VariableAllVC_Factory );

	typedStream = VariableDumpStream_New( VariableDumpStream_Type );
	Stream_Enable( typedStream, True );
	Stream_SetLevel( typedStream, 1 );
	Stream_SetFile( typedStream, stJournal->stdOut );
	
	Journal_RegisterTypedStream( typedStream );
	
	/** Initializing the Component Register singleton */
	stgComponentRegister = Stg_ComponentRegister_New( );
	
	/** Initializing the Hierarchy Table singleton */
	stgHierarchyTable = HierarchyTable_New();

	/** Initializing the Call Graph singleton */
	stgCallGraph = Stg_CallGraph_New();

	/** Initializing the ComponentRegister singleton */
	
	/** Adding default constructors of various components to the Stg_ComponentRegister */
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), Variable_Type, "0", (Stg_Component_DefaultConstructorFunction*)Variable_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), CompositeVC_Type, "0", (Stg_Component_DefaultConstructorFunction*)CompositeVC_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), SetVC_Type, "0", (Stg_Component_DefaultConstructorFunction*)SetVC_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), VariableAllVC_Type, "0", (Stg_Component_DefaultConstructorFunction*)VariableAllVC_DefaultNew );

	/** Register Parents for All Classes */
	RegisterParent( Stg_ComponentFactory_Type,           Stg_Class_Type );
	RegisterParent( Stg_ComponentRegister_Type,          Stg_Class_Type );
	RegisterParent( Stg_Component_Type,                  Stg_Object_Type );
	RegisterParent( VariableCondition_Type,          Stg_Component_Type );
	RegisterParent( CompositeVC_Type,                VariableCondition_Type );
	RegisterParent( ConditionFunction_Type,          Stg_Class_Type );
	RegisterParent( ConditionFunction_Register_Type, Stg_Class_Type );
	RegisterParent( HierarchyTable_Type,             HashTable_Type );
	RegisterParent( Stg_CallGraph_Type,              Stg_Class_Type );
	RegisterParent( Variable_Type,                   Stg_Component_Type );
	RegisterParent( VariableAllVC_Type,              VariableCondition_Type );
	RegisterParent( VariableCondition_Register_Type, Stg_Class_Type );
	RegisterParent( VariableDumpStream_Type,         CStream_Type );
	RegisterParent( Variable_Register_Type,          Stg_Class_Type );

	return True;
}

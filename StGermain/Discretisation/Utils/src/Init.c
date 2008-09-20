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
** $Id: Init.c 3884 2006-10-26 05:26:19Z KathleenHumble $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"
#include "Discretisation/Mesh/Mesh.h"

#include "types.h"
#include "Init.h"
#include "AllElementsVC.h"
#include "AllNodesVC.h"
#include "DiscretisationContext.h"
#include "MeshCoarsener_Hexa.h"
#include "WallVC.h"
#include "CornerVC.h"
#include "InnerWallVC.h"
#include "ShapeVC.h"
#include "FrictionVC.h"
#include "SplitFrictionWallVC.h"
#include "Sync.h"
#include "DofLayout.h"
#include "Operator.h"
#include "FieldVariable.h"
#include "FieldVariable_Register.h"
#include "LinearRegression.h"
#include "OperatorFieldVariable.h"
#include "TimeIntegrator.h"
#include "TimeIntegratee.h"
#include "ShapeAdvector.h"
#include "Remesher.h"
#include "StripRemesher.h"
#include "CellRemesher.h"

#include <stdio.h>

Bool DiscretisationUtils_Init( int* argc, char** argv[] ) {
	Journal_Printf( Journal_Register( DebugStream_Type, "Context" ), "In: %s\n", __func__ ); /* DO NOT CHANGE OR REMOVE */
	
	VariableCondition_Register_Add( variableCondition_Register, AllElementsVC_Type, AllElementsVC_Factory );
	VariableCondition_Register_Add( variableCondition_Register, AllNodesVC_Type, AllNodesVC_Factory );
	VariableCondition_Register_Add( variableCondition_Register, WallVC_Type, WallVC_Factory );
	VariableCondition_Register_Add( variableCondition_Register, CornerVC_Type, CornerVC_Factory );
	VariableCondition_Register_Add( variableCondition_Register, InnerWallVC_Type, InnerWallVC_Factory );
	VariableCondition_Register_Add( variableCondition_Register, ShapeVC_Type, ShapeVC_Factory );
	VariableCondition_Register_Add( variableCondition_Register, FrictionVC_Type, FrictionVC_Factory );
	VariableCondition_Register_Add( variableCondition_Register, SplitFrictionWallVC_Type, SplitFrictionWallVC_Factory );
	
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), AllElementsVC_Type, "0", (Stg_Component_DefaultConstructorFunction*)AllElementsVC_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), AllNodesVC_Type, "0", (Stg_Component_DefaultConstructorFunction*)AllNodesVC_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), DofLayout_Type, "0", (Stg_Component_DefaultConstructorFunction*)DofLayout_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), FieldVariable_Type, "0", (Stg_Component_DefaultConstructorFunction*)FieldVariable_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), OperatorFieldVariable_Type, "0", (Stg_Component_DefaultConstructorFunction*)OperatorFieldVariable_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), WallVC_Type, "0", (Stg_Component_DefaultConstructorFunction*)WallVC_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), CornerVC_Type, "0", (Stg_Component_DefaultConstructorFunction*)CornerVC_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), InnerWallVC_Type, "0", (Stg_Component_DefaultConstructorFunction*)InnerWallVC_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), ShapeVC_Type, "0", _ShapeVC_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), FrictionVC_Type, "0", (Stg_Component_DefaultConstructorFunction*)FrictionVC_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), SplitFrictionWallVC_Type, "0", (Stg_Component_DefaultConstructorFunction*)SplitFrictionWallVC_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), Remesher_Type, "0", (Stg_Component_DefaultConstructorFunction*)_Remesher_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), StripRemesher_Type, "0", (Stg_Component_DefaultConstructorFunction*)_StripRemesher_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), CellRemesher_Type, "0", (Stg_Component_DefaultConstructorFunction*)_CellRemesher_DefaultNew );

	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), TimeIntegrator_Type,"0", _TimeIntegrator_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), TimeIntegratee_Type,"0", _TimeIntegratee_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), ShapeAdvector_Type,"0", _ShapeAdvector_DefaultNew );

	RegisterParent( DiscretisationContext_Type,    AbstractContext_Type );

	RegisterParent( Operator_Type,                 Stg_Object_Type );
	RegisterParent( AllElementsVC_Type,            VariableCondition_Type );
	RegisterParent( AllNodesVC_Type,               VariableCondition_Type );
	RegisterParent( WallVC_Type,                   VariableCondition_Type );
	RegisterParent( CornerVC_Type,		       VariableCondition_Type );
	RegisterParent( InnerWallVC_Type,	       VariableCondition_Type );
	RegisterParent( ShapeVC_Type,                  VariableCondition_Type );
	RegisterParent( FrictionVC_Type,               VariableCondition_Type );
	RegisterParent( SplitFrictionWallVC_Type,      VariableCondition_Type );
	RegisterParent( DofLayout_Type,                Stg_Component_Type );
	RegisterParent( MeshCoarsener_Hexa_Type,       Stg_Component_Type );
	RegisterParent( Sync_Type,                     Stg_Component_Type );
	RegisterParent( Remesher_Type,                 Stg_Component_Type );
	RegisterParent( StripRemesher_Type,            Remesher_Type );
	RegisterParent( CellRemesher_Type,            Remesher_Type );

	RegisterParent( FieldVariable_Type,            Stg_Component_Type );
	RegisterParent( OperatorFieldVariable_Type,    FieldVariable_Type );
	RegisterParent( FieldVariable_Register_Type,   NamedObject_Register_Type );

	RegisterParent( LinearRegression_Type,         Stg_Class_Type );
	
	RegisterParent( TimeIntegratee_Type,           Stg_Component_Type );
	RegisterParent( TimeIntegrator_Type,           Stg_Component_Type );
	RegisterParent( ShapeAdvector_Type,            Stg_Component_Type );
	
	return True;
}

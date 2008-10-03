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
** $Id: Init.c 3883 2006-10-26 05:00:23Z KathleenHumble $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"

#include "types.h"
#include "shortcuts.h"
#include "Init.h"

#include "MeshClass.h"
#include "MeshLayout.h"
#include "MeshDecomp.h"
#include "HexaMD.h"
#include "DummyMD.h"
#include "NodeLayout.h"
#include "BodyNL.h"
#include "CornerNL.h"
#include "ElementLayout.h"
#include "HexaEL.h"
#include "IrregEL.h"
#include "ParallelPipedHexaEL.h"
#include "MeshGeometry.h"
#include "Mesh.h"
#include <stdio.h>

Stream* Mesh_VerboseConfig = NULL;
Stream* Mesh_Debug = NULL;
Stream* Mesh_Warning = NULL;
Stream* Mesh_Error = NULL;

Bool DiscretisationMesh_Init( int* argc, char** argv[] ) {
	Mesh_VerboseConfig = Journal_Register( Info_Type, "Mesh_VerboseConfig" );
	Mesh_Debug = Journal_Register( Debug_Type, "Mesh" );
	Mesh_Warning = Journal_Register( Error_Type, "Mesh" );
	Mesh_Error = Journal_Register( Error_Type, "Mesh" );
	
	Journal_Printf( Journal_Register( DebugStream_Type, "Context" ), "In: %s\n", __func__ ); /* DO NOT CHANGE OR REMOVE */

	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), 
				   MeshTopology_Type, "0", (Stg_Component_DefaultConstructorFunction*)MeshTopology_New );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), 
				   CartesianGenerator_Type, "0", (Stg_Component_DefaultConstructorFunction*)CartesianGenerator_New );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), 
				   SurfaceAdaptor_Type, "0", (Stg_Component_DefaultConstructorFunction*)SurfaceAdaptor_New );
	
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), BodyNL_Type, "0", (Stg_Component_DefaultConstructorFunction*)BodyNL_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), CornerNL_Type, "0", (Stg_Component_DefaultConstructorFunction*)CornerNL_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), DummyMD_Type, "0", (Stg_Component_DefaultConstructorFunction*)DummyMD_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), HexaMD_Type, "0", (Stg_Component_DefaultConstructorFunction*)HexaMD_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), HexaEL_Type, "0", (Stg_Component_DefaultConstructorFunction*)HexaEL_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), IrregEL_Type, "0", (Stg_Component_DefaultConstructorFunction*)IrregEL_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), ParallelPipedHexaEL_Type, "0", (Stg_Component_DefaultConstructorFunction*)ParallelPipedHexaEL_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), MeshGeometry_Type, "0", (Stg_Component_DefaultConstructorFunction*)MeshGeometry_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), Mesh_Type, "0", (Stg_Component_DefaultConstructorFunction*)Mesh_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), MeshLayout_Type, "0", (Stg_Component_DefaultConstructorFunction*)MeshLayout_DefaultNew );

	RegisterParent( MeshTopology_Type, Stg_Component_Type );
	RegisterParent( CartesianGenerator_Type, MeshGenerator_Type );
	RegisterParent( SurfaceAdaptor_Type, MeshAdaptor_Type );

	RegisterParent( NodeLayout_Type,          Stg_Component_Type );
	RegisterParent( BodyNL_Type,              NodeLayout_Type );
	RegisterParent( CornerNL_Type,            NodeLayout_Type );
	
	RegisterParent( MeshDecomp_Type,          Stg_Component_Type );
	RegisterParent( DummyMD_Type,             MeshDecomp_Type );
	RegisterParent( HexaMD_Type,              MeshDecomp_Type );
	
	RegisterParent( ElementLayout_Type,       Stg_Component_Type );
	RegisterParent( IrregEL_Type,             ElementLayout_Type );
	RegisterParent( HexaEL_Type,              ElementLayout_Type );
	RegisterParent( ParallelPipedHexaEL_Type, HexaEL_Type );
	RegisterParent( MeshGeometry_Type,	  BlockGeometry_Type );
	
	RegisterParent( MeshLayout_Type,          Stg_Component_Type );
	RegisterParent( Mesh_Type,                Stg_Component_Type );
	
	return True;
}

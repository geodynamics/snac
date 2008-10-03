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
** $Id: Init.c 3882 2006-10-26 04:41:18Z KathleenHumble $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"


#include "units.h"
#include "types.h"
#include "Init.h"
#include "Topology.h"
#include "IJKTopology.h"
#include "IJK6Topology.h"
#include "IJK26Topology.h"
#include "IrregTopology.h"
#include "GeometryClass.h"
#include "BlockGeometry.h"
#include "RefinedRegionsGeometry.h"
#include "ShellGeometry.h"
#include "IrregGeometry.h"
#include "Delaunay.h"
#include "ParallelDelaunay.h"

#include <stdio.h>

Bool DiscretisationGeometry_Init( int* argc, char** argv[] ) {
	Journal_Printf( Journal_Register( DebugStream_Type, "Context" ), "In: %s\n", __func__ ); /* DO NOT CHANGE OR REMOVE */

		
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), IJK6Topology_Type, "0", (Stg_Component_DefaultConstructorFunction*)IJK6Topology_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), IJK26Topology_Type, "0", (Stg_Component_DefaultConstructorFunction*)IJK26Topology_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), IrregTopology_Type, "0", (Stg_Component_DefaultConstructorFunction*)IrregTopology_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), BlockGeometry_Type, "0", (Stg_Component_DefaultConstructorFunction*)BlockGeometry_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), RefinedRegionsGeometry_Type, "0", RefinedRegionsGeometry_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), ShellGeometry_Type, "0", (Stg_Component_DefaultConstructorFunction*)ShellGeometry_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), IrregGeometry_Type, "0", (Stg_Component_DefaultConstructorFunction*)IrregGeometry_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), Delaunay_Type, "0", (Stg_Component_DefaultConstructorFunction*)Delaunay_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), ParallelDelaunay_Type, "0", (Stg_Component_DefaultConstructorFunction*)ParallelDelaunay_DefaultNew );

	RegisterParent( Delaunay_Type,               Stg_Component_Type );
	RegisterParent( ParallelDelaunay_Type,       Delaunay_Type );
	RegisterParent( Geometry_Type,               Stg_Component_Type );
	RegisterParent( BlockGeometry_Type,          Geometry_Type );
	RegisterParent( RefinedRegionsGeometry_Type, Geometry_Type );
	RegisterParent( IrregGeometry_Type,          Geometry_Type );
	RegisterParent( ShellGeometry_Type,          Geometry_Type );
	
	RegisterParent( Topology_Type,      Stg_Component_Type );
	RegisterParent( IJKTopology_Type,   Topology_Type );
	RegisterParent( IrregTopology_Type, Topology_Type );
	RegisterParent( IJK26Topology_Type, IJKTopology_Type );
	RegisterParent( IJK6Topology_Type,  IJKTopology_Type );
	
	return True;
}

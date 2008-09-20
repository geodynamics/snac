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
** $Id: Init.c 3887 2006-11-06 05:19:51Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"
#include "Discretisation/Mesh/Mesh.h"
#include "Discretisation/Utils/Utils.h"

#include "types.h"
#include "shortcuts.h"
#include "Init.h"

#include "ParticleLayout.h"
#include "PerCellParticleLayout.h"
#include "GaussParticleLayout.h"
#include "TriGaussParticleLayout.h"
#include "RandomParticleLayout.h"
#include "GlobalParticleLayout.h"
#include "SpaceFillerParticleLayout.h"
#include "WithinShapeParticleLayout.h"
#include "ManualParticleLayout.h"
#include "LineParticleLayout.h"
#include "PlaneParticleLayout.h"
#include "FileParticleLayout.h"
#include "CellLayout.h"
#include "SingleCellLayout.h"
#include "ElementCellLayout.h"
#include "Swarm.h"
#include "Swarm_Register.h"
#include "SwarmVariable_Register.h"
#include "SwarmVariable.h"
#include "OperatorSwarmVariable.h"
#include "SwarmOutput.h"
#include "SwarmDump.h"

#include "StandardParticle.h"
#include "IntegrationPoint.h"

#include "Swarm.h"

#include <stdio.h>

Stream* Swarm_VerboseConfig = NULL;
Stream* Swarm_Debug = NULL;
Stream* Swarm_Warning = NULL;
Stream* Swarm_Error = NULL;

Bool DiscretisationSwarm_Init( int* argc, char** argv[] ) {
	Swarm_VerboseConfig = Journal_Register( Info_Type, "Swarm_VerboseConfig" );
	Swarm_Debug = Journal_Register( Debug_Type, "Swarm" );
	Swarm_Warning = Journal_Register( Error_Type, "Swarm" );
	Swarm_Error = Journal_Register( Error_Type, "Swarm" );
	
	Journal_Printf( Journal_Register( DebugStream_Type, "Context" ), "In: %s\n", __func__ ); /* DO NOT CHANGE OR REMOVE */
	
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), GaussParticleLayout_Type, "0", _GaussParticleLayout_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), TriGaussParticleLayout_Type, "0", _TriGaussParticleLayout_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), RandomParticleLayout_Type, "0", _RandomParticleLayout_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), MeshParticleLayout_Type, "0", _MeshParticleLayout_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), SpaceFillerParticleLayout_Type, "0", _SpaceFillerParticleLayout_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), UnionParticleLayout_Type, "0", _UnionParticleLayout_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), WithinShapeParticleLayout_Type, "0", _WithinShapeParticleLayout_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), PlaneParticleLayout_Type, "0", _PlaneParticleLayout_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), ManualParticleLayout_Type, "0", _ManualParticleLayout_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), LineParticleLayout_Type, "0", _LineParticleLayout_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), FileParticleLayout_Type, "0", _FileParticleLayout_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), SingleCellLayout_Type, "0", _SingleCellLayout_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), ElementCellLayout_Type, "0", _ElementCellLayout_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), TriSingleCellLayout_Type, "0", (Stg_Component_DefaultConstructorFunction*)TriSingleCellLayout_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), Swarm_Type, "0", (Stg_Component_DefaultConstructorFunction*)Swarm_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), SwarmVariable_Type, "0", _SwarmVariable_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), OperatorSwarmVariable_Type, "0", _OperatorSwarmVariable_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), SwarmOutput_Type, "0", _SwarmOutput_DefaultNew );
	Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), SwarmDump_Type, "0", _SwarmDump_DefaultNew );

	RegisterParent( CellLayout_Type, Stg_Component_Type );
	RegisterParent( ElementCellLayout_Type, CellLayout_Type );
	RegisterParent( SingleCellLayout_Type, CellLayout_Type );
	RegisterParent( TriSingleCellLayout_Type, CellLayout_Type );

	RegisterParent( ParticleLayout_Type, Stg_Component_Type );

	RegisterParent( GlobalParticleLayout_Type, ParticleLayout_Type );
	RegisterParent( ManualParticleLayout_Type, GlobalParticleLayout_Type );
	RegisterParent( LineParticleLayout_Type, GlobalParticleLayout_Type );
	RegisterParent( FileParticleLayout_Type, GlobalParticleLayout_Type );
	RegisterParent( SpaceFillerParticleLayout_Type, GlobalParticleLayout_Type );
	RegisterParent( UnionParticleLayout_Type, GlobalParticleLayout_Type );
	RegisterParent( WithinShapeParticleLayout_Type, SpaceFillerParticleLayout_Type );
	RegisterParent( PlaneParticleLayout_Type, SpaceFillerParticleLayout_Type );

	RegisterParent( PerCellParticleLayout_Type, ParticleLayout_Type );
	RegisterParent( GaussParticleLayout_Type,   PerCellParticleLayout_Type );
	RegisterParent( RandomParticleLayout_Type,   PerCellParticleLayout_Type );
	RegisterParent( MeshParticleLayout_Type,   PerCellParticleLayout_Type );
	RegisterParent( TriGaussParticleLayout_Type,   PerCellParticleLayout_Type );
	
	
	RegisterParent( SwarmVariable_Register_Type, NamedObject_Register_Type );
	RegisterParent( SwarmVariable_Type,         Stg_Component_Type );
	RegisterParent( OperatorSwarmVariable_Type, SwarmVariable_Type );
	RegisterParent( SwarmOutput_Type,           Stg_Component_Type );
	RegisterParent( SwarmDump_Type,             Stg_Component_Type );
	
	RegisterParent( ParticleCommHandler_Type, Stg_Component_Type );
	RegisterParent( Swarm_Type, Stg_Component_Type );
	RegisterParent( Swarm_Register_Type, Stg_Class_Type );

	/* Create the singletons */
	stgSwarm_Register = Swarm_Register_New(); 

	return True;
}

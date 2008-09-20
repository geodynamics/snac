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
** Role:
**	Test that the ManualParticleLayout initialises particle positions properly.
**
** Assumptions:
**	None as yet.
**
** Comments:
**	None as yet.
**
** $Id: testManualParticleLayout.c 3555 2006-05-10 07:05:46Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"
#include "Discretisation/Mesh/Mesh.h"
#include "Discretisation/Utils/Utils.h"
#include "Discretisation/Swarm/Swarm.h"

#include <stdio.h>
#include <stdlib.h>

struct _Node {
	Coord				coord;
};

struct _Element {
	Coord				coord;
};

struct _Particle {
	__IntegrationPoint
};

int main( int argc, char* argv[] ) {
	MPI_Comm                    CommWorld;
	int                         rank;
	int                         numProcessors;
	int                         procToWatch;
	Dictionary*                 dictionary;
	Topology*                   nTopology;
	ElementLayout*              eLayout;
	NodeLayout*                 nLayout;
	MeshDecomp*                 decomp;
	MeshLayout*                 layout;
	ExtensionManager_Register*  extensionMgr_Register;
	Mesh*                       mesh;
	ManualParticleLayout*       particleLayout;
	ElementCellLayout*          elementCellLayout;
	Swarm*                      swarm;
	Stream*                     stream;
	Dimension_Index             dim;
	Dictionary_Entry_Value*     particlePositionsList;
	Dictionary_Entry_Value*     particlePositionEntry;
	
	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size( CommWorld, &numProcessors );
	MPI_Comm_rank( CommWorld, &rank );
	
	Base_Init( &argc, &argv );
	
	DiscretisationGeometry_Init( &argc, &argv );
	DiscretisationShape_Init( &argc, &argv );
	DiscretisationMesh_Init( &argc, &argv );
	DiscretisationUtils_Init( &argc, &argv );
	DiscretisationSwarm_Init( &argc, &argv );
	MPI_Barrier( CommWorld ); /* Ensures copyright info always come first in output */

	stream = Journal_Register (Info_Type, "myStream");

	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}
	if( rank == procToWatch ) printf( "Watching rank: %i\n", rank );
	
	/* Read input */
	dictionary = Dictionary_New();
	Dictionary_Add( dictionary, "rank", Dictionary_Entry_Value_FromUnsignedInt( rank ) );
	Dictionary_Add( dictionary, "numProcessors", Dictionary_Entry_Value_FromUnsignedInt( numProcessors ) );
	Dictionary_Add( dictionary, "meshSizeI", Dictionary_Entry_Value_FromUnsignedInt( 5 ) );
	Dictionary_Add( dictionary, "meshSizeJ", Dictionary_Entry_Value_FromUnsignedInt( 3 ) );
	Dictionary_Add( dictionary, "meshSizeK", Dictionary_Entry_Value_FromUnsignedInt( 2 ) );
	Dictionary_Add( dictionary, "minX", Dictionary_Entry_Value_FromDouble( 0.0f ) );
	Dictionary_Add( dictionary, "minY", Dictionary_Entry_Value_FromDouble( 0.0f ) );
	Dictionary_Add( dictionary, "minZ", Dictionary_Entry_Value_FromDouble( 0.0f ) );
	Dictionary_Add( dictionary, "maxX", Dictionary_Entry_Value_FromDouble( 1.0f ) );
	Dictionary_Add( dictionary, "maxY", Dictionary_Entry_Value_FromDouble( 1.0f ) );
	Dictionary_Add( dictionary, "maxZ", Dictionary_Entry_Value_FromDouble( 1.0f ) );
	Dictionary_Add( dictionary, "dim", Dictionary_Entry_Value_FromUnsignedInt( 3 ) );
	particlePositionsList = Dictionary_Entry_Value_NewList();
	Dictionary_Add( dictionary, "manualParticlePositions", particlePositionsList );
	particlePositionEntry = Dictionary_Entry_Value_NewStruct();
	Dictionary_Entry_Value_AddElement( particlePositionsList, particlePositionEntry );
	Dictionary_Entry_Value_AddMember( particlePositionEntry, "x", Dictionary_Entry_Value_FromDouble( 0.4 ) );
	Dictionary_Entry_Value_AddMember( particlePositionEntry, "y", Dictionary_Entry_Value_FromDouble( 0.3 ) );
	Dictionary_Entry_Value_AddMember( particlePositionEntry, "z", Dictionary_Entry_Value_FromDouble( 0.2 ) );
	particlePositionEntry = Dictionary_Entry_Value_NewStruct();
	Dictionary_Entry_Value_AddElement( particlePositionsList, particlePositionEntry );
	Dictionary_Entry_Value_AddMember( particlePositionEntry, "x", Dictionary_Entry_Value_FromDouble( 0.7 ) );
	Dictionary_Entry_Value_AddMember( particlePositionEntry, "y", Dictionary_Entry_Value_FromDouble( 0.6 ) );
	Dictionary_Entry_Value_AddMember( particlePositionEntry, "z", Dictionary_Entry_Value_FromDouble( 0.5 ) );
	particlePositionEntry = Dictionary_Entry_Value_NewStruct();
	Dictionary_Entry_Value_AddElement( particlePositionsList, particlePositionEntry );
	Dictionary_Entry_Value_AddMember( particlePositionEntry, "x", Dictionary_Entry_Value_FromDouble( 0.8 ) );
	Dictionary_Entry_Value_AddMember( particlePositionEntry, "y", Dictionary_Entry_Value_FromDouble( 0.1 ) );
	Dictionary_Entry_Value_AddMember( particlePositionEntry, "z", Dictionary_Entry_Value_FromDouble( 0.3 ) );
	particlePositionEntry = Dictionary_Entry_Value_NewStruct();
	Dictionary_Entry_Value_AddElement( particlePositionsList, particlePositionEntry );
	Dictionary_Entry_Value_AddMember( particlePositionEntry, "x", Dictionary_Entry_Value_FromDouble( 0.9 ) );
	Dictionary_Entry_Value_AddMember( particlePositionEntry, "y", Dictionary_Entry_Value_FromDouble( 0.4 ) );
	Dictionary_Entry_Value_AddMember( particlePositionEntry, "z", Dictionary_Entry_Value_FromDouble( 0.1 ) );
	
	/* Run the mesher */
	dim       = Dictionary_GetUnsignedInt( dictionary, "dim" );

	nTopology = (Topology*)IJK6Topology_New( "IJK6Topology", dictionary );
	eLayout   = (ElementLayout*)ParallelPipedHexaEL_New( "PPHexaEL", 3, dictionary );
	nLayout   = (NodeLayout*)CornerNL_New( "CornerNL", dictionary, eLayout, nTopology );
	decomp    = (MeshDecomp*)HexaMD_New( "HexaMD", dictionary, MPI_COMM_WORLD, eLayout, nLayout );
	layout    = MeshLayout_New( "MeshLayout", eLayout, nLayout, decomp );
	
	/* Init mesh */
	extensionMgr_Register = ExtensionManager_Register_New();
	mesh = Mesh_New( "Mesh", layout, sizeof(Node), sizeof(Element), extensionMgr_Register, dictionary );
	
	/* Configure the element-cell-layout */
	elementCellLayout = ElementCellLayout_New( "elementCellLayout", mesh );
	
	/* Build the mesh */
	Build( mesh, 0, False );
	Initialise( mesh, 0, False );
	
	/* Configure the gauss-particle-layout */
	particleLayout = ManualParticleLayout_New( "manualParticleLayout", dictionary );
	
	/* Configure the swarm */
	swarm = Swarm_New(  "testSwarm", elementCellLayout, particleLayout, dim, sizeof(Particle),
		extensionMgr_Register, NULL, CommWorld );
	
	/* Build the swarm */
	Build( swarm, 0, False );
	Initialise( swarm, 0, False );
	
	if( rank == procToWatch ) {
		Stg_Class_Print( particleLayout, stream );
		/* Print out the particles on all cells */
		Swarm_PrintParticleCoords_ByCell( swarm, stream );
	}
	

	/* Destroy stuff */
	Stg_Class_Delete( particleLayout );
	Stg_Class_Delete( elementCellLayout );
	Stg_Class_Delete( swarm );
	Stg_Class_Delete( mesh );
	Stg_Class_Delete( extensionMgr_Register );
	Stg_Class_Delete( layout );
	Stg_Class_Delete( decomp );
	Stg_Class_Delete( nLayout );
	Stg_Class_Delete( eLayout );
	Stg_Class_Delete( nTopology );
	Stg_Class_Delete( dictionary );
	
	DiscretisationSwarm_Finalise();
	DiscretisationUtils_Finalise();
	DiscretisationMesh_Finalise();
	DiscretisationShape_Finalise();
	DiscretisationGeometry_Finalise();
	
	Base_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0; /* success */
}

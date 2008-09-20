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
**	Test that the ElementCellLayout has the same layout and geometry as the mesh's element layout.
**
** Assumptions:
**	None as yet.
**
** Comments:
**	None as yet.
**
** $Id: testSwarm.c 3986 2007-01-29 07:31:46Z PatrickSunter $
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
	__GlobalParticle
};

void TestParticleSearchFunc( Swarm* swarm, Coord coord, Stream* stream ) {
	double              distance;
	Particle_Index      closestParticle_I;
	Particle_Index      lParticle_I;
	GlobalParticle*     particle;

	Journal_Printf( stream, "Testing coord %g %g %g\n", coord[ I_AXIS ], coord[ J_AXIS ], coord[ K_AXIS ] );

	closestParticle_I = Swarm_FindClosestParticle( swarm, 3, coord, &distance );

	if ( closestParticle_I < swarm->particleLocalCount ) {
		Journal_Printf( stream, "Closest Particle is %u with distance %g\n", closestParticle_I, distance );

		for ( lParticle_I = 0 ; lParticle_I < swarm->particleLocalCount ; lParticle_I++ ) {
			/* don't bother with testing same particle */
			if (lParticle_I == closestParticle_I)
				continue;

			particle = (GlobalParticle*)Swarm_ParticleAt( swarm , lParticle_I );

			/* Test if particle is closer */
			if (distance > StGermain_DistanceBetweenPoints( coord, particle->coord, 3 ) )
				Journal_Printf( stream, "Particle %u is closer to coord and has distance %g.\n" , 
						lParticle_I, StGermain_DistanceBetweenPoints( coord, particle->coord, 3 ) );
		}
	}
	else
		Journal_Printf( stream, "Coord not found on this processor.\n" );
}

int main( int argc, char* argv[] ) {
	MPI_Comm			CommWorld;
	int				rank;
	int				numProcessors;
	int				procToWatch;
	Dictionary*			dictionary;
	Topology*			nTopology;
	ElementLayout*			eLayout;
	NodeLayout*			nLayout;
	MeshDecomp*			decomp;
	MeshLayout*			layout;
	ExtensionManager_Register*		extensionMgr_Register;
	Mesh*				mesh;
	ElementCellLayout*		elementCellLayout;
	RandomParticleLayout*		randomParticleLayout;
	Swarm*				swarm;
	Stream*				stream;
	Coord                           coord = { 120, 2, 210 };
	double                          minX, minY, minZ, maxX, maxY, maxZ; 

	minX = minY = minZ = 0.0;
	maxX = 300;
	maxY = 12;
	maxZ = 300;
	
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
	Dictionary_Add( dictionary, "meshSizeI", Dictionary_Entry_Value_FromUnsignedInt( 3 ) );
	Dictionary_Add( dictionary, "meshSizeJ", Dictionary_Entry_Value_FromUnsignedInt( 4 ) );
	Dictionary_Add( dictionary, "meshSizeK", Dictionary_Entry_Value_FromUnsignedInt( 3 ) );
	Dictionary_Add( dictionary, "allowUnbalancing", Dictionary_Entry_Value_FromBool( True ) );
	Dictionary_Add( dictionary, "minX", Dictionary_Entry_Value_FromDouble( minX ) );
	Dictionary_Add( dictionary, "minY", Dictionary_Entry_Value_FromDouble( minY ) );
	Dictionary_Add( dictionary, "minZ", Dictionary_Entry_Value_FromDouble( minZ ) );
	Dictionary_Add( dictionary, "maxX", Dictionary_Entry_Value_FromDouble( maxX ) );
	Dictionary_Add( dictionary, "maxY", Dictionary_Entry_Value_FromDouble( maxY ) );
	Dictionary_Add( dictionary, "maxZ", Dictionary_Entry_Value_FromDouble( maxZ ) );
	
	/* Run the mesher */
	nTopology = (Topology*)IJK6Topology_New( "IJK6Topology", dictionary );
	eLayout = (ElementLayout*)ParallelPipedHexaEL_New( "PPHexaEL", 3, dictionary );
	nLayout = (NodeLayout*)CornerNL_New( "CornerNL", dictionary, eLayout, nTopology );
	decomp = (MeshDecomp*)HexaMD_New( "HexaMD", dictionary, MPI_COMM_WORLD, eLayout, nLayout );
	layout = MeshLayout_New( "MeshLayout", eLayout, nLayout, decomp );
	
	/* Init mesh */
	extensionMgr_Register = ExtensionManager_Register_New();
	mesh = Mesh_New( "Mesh", layout, sizeof(Node), sizeof(Element), extensionMgr_Register, dictionary );
	
	/* Build the mesh */
	Build( mesh, 0, False );
	Initialise( mesh, 0, False );
	
	/* Configure the element-cell-layout */
	elementCellLayout = ElementCellLayout_New( "elementCellLayout", mesh );
	
	/* Configure the random-particle-layout */
	randomParticleLayout = RandomParticleLayout_New( "randomParticleCellLayout", 4, 13 );
	
	/* Configure the swarm */
	swarm = Swarm_New( "testSwarm", elementCellLayout, randomParticleLayout, 3, sizeof(Particle),
		extensionMgr_Register, NULL, CommWorld );
	
	/* Build the swarm */
	Build( swarm, 0, False );
	Initialise( swarm, 0, False );
	
	if( rank == procToWatch ) {
		Print( swarm, stream );

		Journal_Printf( stream, "\n" );
		Swarm_PrintParticleCoords( swarm, stream );
		Journal_Printf( stream, "\n" );
		Swarm_PrintParticleCoords_ByCell( swarm, stream );
	
		/* Test Particle Searching Function */
		if (numProcessors == 1) {
			coord[ I_AXIS ] = 0.6 * 300; coord[ J_AXIS ] = 0.2*12; coord[ K_AXIS ] = 0.820*300;
			TestParticleSearchFunc( swarm, coord, stream );

			/* Test New Coord */
			coord[ I_AXIS ] = 0.2*300; coord[ J_AXIS ] = 0.9*12; coord[ K_AXIS ] = 0.120*300;
			TestParticleSearchFunc( swarm, coord, stream );
		}

		/* Test some particle deletion ... remember that in parallel the particles per proc is lower, so delete low numbers */
		Journal_Printf( stream, "\nAbout to test deletion of particles (current local count is %u)\n",
			swarm->particleLocalCount );

		Journal_Printf( stream, "About to delete particle 0:\n" );
		Swarm_DeleteParticle( swarm, 0 );
		Journal_Printf( stream, "(local particle count is now %u)\n", swarm->particleLocalCount );

		Journal_Printf( stream, "About to delete particle %u:\n", swarm->particleLocalCount / 2 );
		Swarm_DeleteParticle( swarm, swarm->particleLocalCount / 2 );
		Journal_Printf( stream, "(local particle count is now %u)\n", swarm->particleLocalCount );

		Journal_Printf( stream, "About to delete particle %u (last particle):\n", swarm->particleLocalCount - 1 );
		Swarm_DeleteParticle( swarm, swarm->particleLocalCount - 1 );
		Journal_Printf( stream, "(local particle count is now %u)\n", swarm->particleLocalCount );
	
		Journal_Printf( stream, "\nAfter deletions, particles remaining:\n" );
		Swarm_PrintParticleCoords( swarm, stream );
		Journal_Printf( stream, "\n" );
		Swarm_PrintParticleCoords_ByCell( swarm, stream );

		{
			GlobalParticle   newParticles[3];
			Cell_Index       newParticle_Cells[3];
			Particle_Index   particle_I;

			minX = ((ParallelPipedHexaEL*)eLayout)->minLocalThisPartition[0];
			minY = ((ParallelPipedHexaEL*)eLayout)->minLocalThisPartition[1];
			minZ = ((ParallelPipedHexaEL*)eLayout)->minLocalThisPartition[2];
			maxX = ((ParallelPipedHexaEL*)eLayout)->maxLocalThisPartition[0];
			maxY = ((ParallelPipedHexaEL*)eLayout)->maxLocalThisPartition[1];
			maxZ = ((ParallelPipedHexaEL*)eLayout)->maxLocalThisPartition[2];

			for( particle_I = 0; particle_I < 3; particle_I++) {
				newParticles[particle_I].coord[0] = ( ( rand() / (double)RAND_MAX ) * (maxX - minX)) + minX;
				newParticles[particle_I].coord[1] = ( ( rand() / (double)RAND_MAX ) * (maxY - minY)) + minY;
				newParticles[particle_I].coord[2] = ( ( rand() / (double)RAND_MAX ) * (maxZ - minZ)) + minZ;
				newParticle_Cells[particle_I] = CellLayout_CellOf( swarm->cellLayout, &newParticles[particle_I] );
			}
			
			Journal_Printf( stream, "\nAbout to test deletion of particles, and replacing with new (current local count is %u)\n",
				swarm->particleLocalCount );

			Journal_Printf( stream, "About to delete particle 0 and replace with new at (%.2f,%.2f,%.2f) - cell %u:\n",
				newParticles[0].coord[0], newParticles[0].coord[1], newParticles[0].coord[2],
				newParticle_Cells[0] );
			Swarm_DeleteParticleAndReplaceWithNew( swarm, 0, &newParticles[0], newParticle_Cells[0] );

			Journal_Printf( stream, "About to delete particle %u and replace with new at (%.2f,%.2f,%.2f) - cell %u:\n",
				swarm->particleLocalCount / 2, newParticles[1].coord[0], newParticles[1].coord[1],
				newParticles[1].coord[2], newParticle_Cells[1] );
			Swarm_DeleteParticleAndReplaceWithNew( swarm, swarm->particleLocalCount / 2, &newParticles[1],
				newParticle_Cells[1] );

			Journal_Printf( stream, "About to delete particle %u (last particle) and replace with new at "
				"(%.2f,%.2f,%.2f) - cell %u:\n",
				swarm->particleLocalCount - 1, newParticles[2].coord[0], newParticles[2].coord[1],
				newParticles[2].coord[2], newParticle_Cells[2] );
			Swarm_DeleteParticleAndReplaceWithNew( swarm, swarm->particleLocalCount - 1, &newParticles[2],
				newParticle_Cells[2] );
		
			Journal_Printf( stream, "\nAfter deletions and replacing, particles are:\n" );
			Swarm_PrintParticleCoords( swarm, stream );
			Journal_Printf( stream, "\n" );
			Swarm_PrintParticleCoords_ByCell( swarm, stream );
		}
	}
	
	/* Destroy stuff */
	Stg_Class_Delete( swarm );
	Stg_Class_Delete( randomParticleLayout );
	Stg_Class_Delete( elementCellLayout );
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

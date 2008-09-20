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
**	Tests that particles can be saved to file, then re-loaded onto a new context with exactly
**	the same positions and values.
**
** Assumptions:
**	None as yet.
**
** Comments:
**	None as yet.
**
** $Id: testSwarmDumpAndLoad.c 3634 2006-06-13 09:51:29Z PatrickSunter $
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
#include <assert.h>


struct _Node {
	Coord				coord;
};

struct _Element {
	Coord				coord;
};

struct _Particle {
	__GlobalParticle
	Coord               xi;
	unsigned int        testValue;
};

void UpdateParticlePositionsTowardsAttractor( 
		Swarm* swarm,
		Coord attractorPoint,
		Processor_Index rank,
		Processor_Index procToWatch );

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
	ExtensionManager_Register*	extensionMgr_Register;
	Mesh*				mesh;
	ElementCellLayout*		elementCellLayout;
	RandomParticleLayout*		randomParticleLayout;
	Swarm*				swarm;
	Stream*				stream;
	Index				timeStep;
	BlockGeometry*			blockGeom;
	Coord				attractorPoint;
	Index				dim_I;
	AbstractContext*                context = NULL;
	SwarmDump*                      swarmDumper = NULL;
	char                            filename[1000];
	FileParticleLayout*             fileParticleLayout = NULL;
	Swarm*                          newSwarm = NULL;
	Swarm*                          swarmList[1];
	Particle_Index                  lParticle_I = 0;
	Index                           errorCount = 0;
	
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

	/* *** Journal stuff *** */
	Journal_Enable_TypedStream( DebugStream_Type, False );
	Stream_EnableBranch( Swarm_Debug, True );
	Stream_SetLevelBranch( Swarm_Debug, 3 );

	/* Turn off the particle comm info to guarantee stdout order */
	Stream_Enable( Journal_Register( Info_Type, ParticleCommHandler_Type ), False );


	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}
	if( rank == procToWatch ) printf( "Watching rank: %i\n", rank );
	
	Stream_Enable( Journal_Register( Info_Type, SwarmDump_Type ), False );
	
	/* Read input */
	dictionary = Dictionary_New();
	Dictionary_Add( dictionary, "rank", Dictionary_Entry_Value_FromUnsignedInt( rank ) );
	Dictionary_Add( dictionary, "numProcessors", Dictionary_Entry_Value_FromUnsignedInt( numProcessors ) );
	Dictionary_Add( dictionary, "dim", Dictionary_Entry_Value_FromUnsignedInt( 3 ) );
	Dictionary_Add( dictionary, "meshSizeI", Dictionary_Entry_Value_FromUnsignedInt( 33 ) );
	Dictionary_Add( dictionary, "meshSizeJ", Dictionary_Entry_Value_FromUnsignedInt( 33 ) );
	Dictionary_Add( dictionary, "meshSizeK", Dictionary_Entry_Value_FromUnsignedInt( 33 ) );
	Dictionary_Add( dictionary, "allowUnbalancing", Dictionary_Entry_Value_FromBool( True ) );
	Dictionary_Add( dictionary, "minX", Dictionary_Entry_Value_FromDouble( 0.0f ) );
	Dictionary_Add( dictionary, "minY", Dictionary_Entry_Value_FromDouble( 0.0f ) );
	Dictionary_Add( dictionary, "minZ", Dictionary_Entry_Value_FromDouble( 0.0f ) );
	Dictionary_Add( dictionary, "maxX", Dictionary_Entry_Value_FromDouble( 1.0f ) );
	Dictionary_Add( dictionary, "maxY", Dictionary_Entry_Value_FromDouble( 1.0f ) );
	Dictionary_Add( dictionary, "maxZ", Dictionary_Entry_Value_FromDouble( 1.0f ) );
	Dictionary_Add( dictionary, "particlesPerCell", Dictionary_Entry_Value_FromUnsignedInt( 1 ) );
	Dictionary_Add( dictionary, "seed", Dictionary_Entry_Value_FromUnsignedInt( 13 ) );
	Dictionary_Add( dictionary, "shadowDepth", Dictionary_Entry_Value_FromUnsignedInt( 1 ) );
	
	/* Run the mesher */
	nTopology = (Topology*)IJK6Topology_New( "IJK6Topology", dictionary );
	eLayout = (ElementLayout*)ParallelPipedHexaEL_New( "PPHexaEL", 3, dictionary );
	nLayout = (NodeLayout*)CornerNL_New( "CornerNL", dictionary, eLayout, nTopology );
	decomp = (MeshDecomp*)HexaMD_New( "HexaMD", dictionary, CommWorld, eLayout, nLayout );
	layout = MeshLayout_New( "Meshlayout", eLayout, nLayout, decomp );
	
	/* Init mesh */
	extensionMgr_Register = ExtensionManager_Register_New();
	mesh = Mesh_New( "Mesh", layout, sizeof(Node), sizeof(Element), extensionMgr_Register, dictionary );
	
	/* Configure the element-cell-layout */
	elementCellLayout = ElementCellLayout_New( "elementCellLayout", mesh );
	
	/* Configure the random-particle-layout */
	randomParticleLayout = RandomParticleLayout_New( "randomParticleLayout", 1, 13 );
	
	/* Configure the swarm */
	swarm = Swarm_New( "testSwarm", elementCellLayout, randomParticleLayout, 3, sizeof(Particle),
		extensionMgr_Register, NULL, CommWorld );
	
	/* +++ BUILD PHASE +++ */
	
	/* Build the mesh */
	Build( mesh, 0, False );
	/* Build the swarm */
	Build( swarm, 0, False );

	/* +++ INITIALISE PHASE +++ */

	Initialise( mesh, 0, False );
	Initialise( swarm, 0, False );
	
	blockGeom = (BlockGeometry*)eLayout->geometry;
	for ( dim_I=0; dim_I < 3; dim_I++ ) {
		attractorPoint[dim_I] = ( blockGeom->max[dim_I] - blockGeom->min[dim_I] ) / 3;
	}
	if( rank == procToWatch ) {
		printf("Calculated attractor point is at (%f,%f,%f):\n", attractorPoint[0], attractorPoint[1], attractorPoint[2] );
	}	

	for ( lParticle_I = 0; lParticle_I < swarm->particleLocalCount; lParticle_I++ ) {
		swarm->particles[lParticle_I].testValue = rand() % 1000;
	}

	/* +++ RUN PHASE +++ */

	/* Start a sample app, where each timestep we move the particles towards the attractor point */
	for ( timeStep=1; timeStep <= 2; timeStep++ ) {
		if( rank == procToWatch ) {
			printf("\nStarting timestep %d:\n", timeStep );
		}	

		UpdateParticlePositionsTowardsAttractor( swarm, attractorPoint, rank, procToWatch );
	
		Swarm_UpdateAllParticleOwners( swarm );
	}
	
	Dictionary_Add( dictionary, "outputPath", Dictionary_Entry_Value_FromString( "./output" ) );
	
	/* Now we dump the swarm values, then create a new swarm and load the dumped values onto it,
		and check to see that they're the same */
	context = _AbstractContext_New( 
		sizeof(AbstractContext),
		AbstractContext_Type,
		_AbstractContext_Delete,
		_AbstractContext_Print,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		"testContext",
		True,
		NULL,
		0,
		0,
		CommWorld,
		dictionary );

	swarmList[0] = swarm;
	swarmDumper = SwarmDump_New( "swarmDumper", context, swarmList, 1, True );
	SwarmDump_Execute( swarmDumper, context );

	sprintf( filename, "%s/%s.%05d.dat", context->outputPath, swarm->name, context->timeStep ); 
	fileParticleLayout = FileParticleLayout_New( "fileParticleLayout", filename );
	newSwarm = Swarm_New( "testSwarm2", elementCellLayout, fileParticleLayout, 3, sizeof(Particle),
		extensionMgr_Register, NULL, CommWorld );
	Build( newSwarm, 0, False );
	Initialise( newSwarm, 0, False );

	assert( newSwarm->particleLocalCount == swarm->particleLocalCount );

	if( rank == procToWatch ) {
		printf( "\nComparing the %d local Particles between old and new swarms:\n", swarm->particleLocalCount );
		for ( lParticle_I = 0; lParticle_I < swarm->particleLocalCount; lParticle_I++ ) {
			if ( ( swarm->particles[lParticle_I].coord[I_AXIS] != newSwarm->particles[lParticle_I].coord[I_AXIS] ) 
				|| ( swarm->particles[lParticle_I].coord[J_AXIS] != newSwarm->particles[lParticle_I].coord[J_AXIS] ) 
				|| ( swarm->particles[lParticle_I].coord[K_AXIS] != newSwarm->particles[lParticle_I].coord[K_AXIS] ) ) {
				printf( "Error: Co-ords at particle %d don't match between old and new swarms.\n", lParticle_I );
				errorCount++;
			}

			if ( swarm->particles[lParticle_I].owningCell != newSwarm->particles[lParticle_I].owningCell ) {
				printf( "Error: owningCell at particle %d doesn't match between old and new swarms.\n", lParticle_I );
				errorCount++;
			}
			
			if ( ( swarm->particles[lParticle_I].xi[I_AXIS] != newSwarm->particles[lParticle_I].xi[I_AXIS] ) 
				|| ( swarm->particles[lParticle_I].xi[J_AXIS] != newSwarm->particles[lParticle_I].xi[J_AXIS] ) 
				|| ( swarm->particles[lParticle_I].xi[K_AXIS] != newSwarm->particles[lParticle_I].xi[K_AXIS] ) ) {
				printf( "Error: Xi values at particle %d don't match between old and new swarms.\n", lParticle_I );
				errorCount++;
			}

			if ( swarm->particles[lParticle_I].testValue != newSwarm->particles[lParticle_I].testValue ) {
				printf( "Error: testValue at particle %d doesn't match between old and new swarms.\n", lParticle_I );
				errorCount++;
			}
		}

		if ( 0 == errorCount ) {
			printf( "\tPassed: swarms are identical.\n" );
		}
		else {
			printf( "\tFailed: %d differences detected.\n", errorCount );
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


void UpdateParticlePositionsTowardsAttractor( 
		Swarm* swarm,
		Coord attractorPoint,
		Processor_Index rank,
		Processor_Index procToWatch )
{
	Cell_LocalIndex			lCell_I;
	Particle_InCellIndex		cParticle_I;
	Particle*	 		currParticle;
	Index				dim_I;

	for ( lCell_I=0; lCell_I < swarm->cellLocalCount; lCell_I++ ) {
		if( rank == procToWatch ) {
			//printf("\tUpdating Particles positions in local cell %d:\n", lCell_I );
		}	
		for ( cParticle_I=0; cParticle_I < swarm->cellParticleCountTbl[lCell_I]; cParticle_I++ ) {
			Coord movementVector = {0,0,0};
			Coord newParticleCoord = {0,0,0};
			Coord* oldCoord;

			currParticle = (Particle*)Swarm_ParticleInCellAt( swarm, lCell_I, cParticle_I );
			oldCoord = &currParticle->coord;
			if( rank == procToWatch ) {
				//printf("\t\tUpdating particleInCell %d:\n", cParticle_I );
			}	

			for ( dim_I=0; dim_I < 3; dim_I++ ) {
				movementVector[dim_I] = ( attractorPoint[dim_I] - (*oldCoord)[dim_I] ) / 3;
				newParticleCoord[dim_I] = (*oldCoord)[dim_I] + movementVector[dim_I];
			}

			if( rank == procToWatch ) {
				/*printf("\t\tChanging its coords from (%f,%f,%f) to (%f,%f,%f):\n",
					(*oldCoord)[0], (*oldCoord)[1], (*oldCoord)[2],
					newParticleCoord[0], newParticleCoord[1], newParticleCoord[2] );*/
			}		

			for ( dim_I=0; dim_I < 3; dim_I++ ) {
				currParticle->coord[dim_I] = newParticleCoord[dim_I];
			}
		}
	}
}

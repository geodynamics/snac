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
** $Id: testGaussLayoutSingleCell.c 3555 2006-05-10 07:05:46Z PatrickSunter $
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
	MPI_Comm			CommWorld;
	int				rank;
	int				numProcessors;
	int				procToWatch;
	Dictionary*			dictionary;
	GaussParticleLayout*		gaussParticleLayout;
	SingleCellLayout*		singleCellLayout;
	ExtensionManager_Register*		extensionMgr_Register;
	Swarm*				swarm;
	Stream*				stream;
	double 				w;
	Cell_PointIndex			count;
	Dimension_Index         dim;
	Bool			dimExists[] = { True, True, True };
	Particle_InCellIndex particlesPerDim[3];
	double x,y,z;
	int p;
	
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
	Dictionary_Add( dictionary, "minX", Dictionary_Entry_Value_FromDouble( 0.0f ) );
	Dictionary_Add( dictionary, "minY", Dictionary_Entry_Value_FromDouble( 0.0f ) );
	Dictionary_Add( dictionary, "minZ", Dictionary_Entry_Value_FromDouble( 0.0f ) );
	Dictionary_Add( dictionary, "maxX", Dictionary_Entry_Value_FromDouble( 300.0f ) );
	Dictionary_Add( dictionary, "maxY", Dictionary_Entry_Value_FromDouble( 12.0f ) );
	Dictionary_Add( dictionary, "maxZ", Dictionary_Entry_Value_FromDouble( 300.0f ) );
	Dictionary_Add( dictionary, "gaussParticlesX", Dictionary_Entry_Value_FromUnsignedInt( 2 ) );
	Dictionary_Add( dictionary, "gaussParticlesY", Dictionary_Entry_Value_FromUnsignedInt( 1 ) );
	Dictionary_Add( dictionary, "gaussParticlesZ", Dictionary_Entry_Value_FromUnsignedInt( 3 ) );
	Dictionary_Add( dictionary, "dim", Dictionary_Entry_Value_FromUnsignedInt( 3 ) );
	
	/* Configure the element-cell-layout */
	singleCellLayout = SingleCellLayout_New( "singleCellLayout", dimExists, NULL, NULL );
	
	/* Configure the gauss-particle-layout */
	dim = Dictionary_GetUnsignedInt( dictionary, "dim" );
	particlesPerDim[0] = Dictionary_GetUnsignedInt( dictionary, "gaussParticlesX" );
	particlesPerDim[1] = Dictionary_GetUnsignedInt( dictionary, "gaussParticlesY" );
	particlesPerDim[2] = Dictionary_GetUnsignedInt( dictionary, "gaussParticlesZ" );
	gaussParticleLayout = GaussParticleLayout_New( "gaussParticleLayout", dim , particlesPerDim );
	
	/* Configure the swarm */
	extensionMgr_Register = ExtensionManager_Register_New();
	swarm = Swarm_New( "testGaussSwarmSingleCell", singleCellLayout, gaussParticleLayout, dim,
		sizeof(Particle), extensionMgr_Register, NULL, CommWorld );
	
	/* Build the swarm */
	Build( swarm, 0, False );
	Initialise( swarm, 0, False );
	
	if( rank == procToWatch ) {
		Print( gaussParticleLayout, stream );
	}
	
	/* Print out the particles on any cell (they should all be the same) */
	count = swarm->cellParticleCountTbl[0];
	printf("count = %d \n",count );
	
	for( p = 0; p < count; p++ ) {
		x = ((IntegrationPoint*)Swarm_ParticleInCellAt( swarm, 0, p ))->xi[0]; 
		y = ((IntegrationPoint*)Swarm_ParticleInCellAt( swarm, 0, p ))->xi[1]; 
		z = ((IntegrationPoint*)Swarm_ParticleInCellAt( swarm, 0, p ))->xi[2]; 	
		w = ((IntegrationPoint*)Swarm_ParticleInCellAt( swarm, 0, p ))->weight;
		printf("pId=%d : xi = { %f, %f, %f } \n",p,x,y,z );
		printf("pId=%d : weight = %f \n",p,w );
	}	
	
	/* Destroy stuff */
	Stg_Class_Delete( gaussParticleLayout );
	Stg_Class_Delete( singleCellLayout );	
	Stg_Class_Delete( swarm );
	Stg_Class_Delete( extensionMgr_Register );
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

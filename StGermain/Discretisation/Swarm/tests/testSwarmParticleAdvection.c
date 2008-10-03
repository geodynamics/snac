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
**	Tests that particles can be successfully moved between cells. The problem is set up with a 
**	"gravitational attractor" in the exact middle of the domain - all particles are sucked in
**	towards it each timestep.
**
** Assumptions:
**	None as yet.
**
** Comments:
**	None as yet.
**
** $Id: testSwarmParticleAdvection.c 3851 2006-10-12 08:57:22Z SteveQuenette $
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
#include <unistd.h>


struct _Node {
	Coord				coord;
};

struct _Element {
	Coord				coord;
};

struct _Particle {
	__GlobalParticle
	double  velocity[3];
	double  randomColour;
};

double Dt( void* context ) {
	return 2.0;
}

// TODO: should be removed once we get saving of swarms onto disc context...
void SaveSwarms( void* context );

/** Global so other funcs can use */
Index procToWatch = 0;

int main( int argc, char* argv[] ) {
	DiscretisationContext*          context;
	MPI_Comm			CommWorld;
	int				rank;
	int				numProcessors;
	Dictionary*			dictionary;
	Dictionary*                     componentDict;
	Stg_ComponentFactory*           cf;
	XML_IO_Handler*                 ioHandler;
	ExtensionManager_Register*      extensionMgr_Register;
	SwarmVariable_Register*         swarmVariable_Register;
	Stream*                         stream;
	Swarm*                          swarm = NULL;
	Particle                        particle;
	Particle*                       currParticle = NULL;
	Particle_Index                  lParticle_I = 0;
	Dimension_Index                 dim_I = 0;
	
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

	dictionary = Dictionary_New();
	ioHandler = XML_IO_Handler_New();
	IO_Handler_ReadAllFromCommandLine( ioHandler, argc, argv, dictionary );

	/* TODO: temporary hack until Al gets the journal read from file going again */
	if ( False == Dictionary_GetBool_WithDefault( dictionary, "particleCommInfo", True ) ) {
		Stream_Enable( Journal_Register( Info_Type, ParticleCommHandler_Type ), False );
	}

	Journal_ReadFromDictionary( dictionary );

	/* *** Journal stuff *** */
	Journal_Enable_TypedStream( DebugStream_Type, False );
	Stream_EnableBranch( Swarm_Debug, True );
	Stream_SetLevelBranch( Swarm_Debug, 3 );

	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}
	if( rank == procToWatch ) printf( "Watching rank: %i\n", rank );
	/* For plugins to read */
	Dictionary_Add( dictionary, "procToWatch", Dictionary_Entry_Value_FromUnsignedInt( procToWatch ) );
	
/* Construction phase -------------------------------------------------------------------------------------------*/
	
	/* Create the Context */
	context = DiscretisationContext_New(
			"context",
			0,
			0,
			MPI_COMM_WORLD,
			dictionary );

	componentDict = Dictionary_GetDictionary( dictionary, "components" );
	
	assert( componentDict );

	cf = context->CF = Stg_ComponentFactory_New( dictionary, componentDict, context->register_Register );

	LiveComponentRegister_Add( cf->LCRegister, (Stg_Component*) context );
	PluginsManager_Load( context->plugins, context, dictionary );

	extensionMgr_Register = ExtensionManager_Register_New();
	swarmVariable_Register = SwarmVariable_Register_New( NULL );
	Stg_ObjectList_ClassAppend( cf->registerRegister, (void*)extensionMgr_Register, "ExtensionManager_Register" );
	Stg_ObjectList_ClassAppend( cf->registerRegister, (void*)swarmVariable_Register, "SwarmVariable_Register" );

	Stg_ComponentFactory_CreateComponents( cf );
	Stg_ComponentFactory_ConstructComponents( cf, 0 /* dummy */ );
	PluginsManager_ConstructPlugins( context->plugins, context->CF, 0 /* dummy */ );

	KeyCall( context, context->constructExtensionsK, EntryPoint_VoidPtr_CallCast* )( KeyHandle(context,context->constructExtensionsK), context );

	swarm = (Swarm*) LiveComponentRegister_Get( context->CF->LCRegister, "swarm" );
	ExtensionManager_Add( swarm->particleExtensionMgr, "ParticleVelocity", sizeof(double[3]) );
	ExtensionManager_Add( swarm->particleExtensionMgr, "ParticleColour", sizeof(double) );

	Swarm_NewVectorVariable(
		swarm,
		"Velocity",
		(ArithPointer) &particle.velocity - (ArithPointer) &particle,
		Variable_DataType_Double,
		swarm->dim,
		"VelocityX",
		"VelocityY",
		"VelocityZ" );

	Swarm_NewScalarVariable(
		swarm,
		"RandomColour",
		(ArithPointer) &particle.randomColour - (ArithPointer) &particle,
		Variable_DataType_Double );

	LiveComponentRegister_BuildAll( cf->LCRegister, context );
	LiveComponentRegister_InitialiseAll( cf->LCRegister, context );

	/* for each particle, set a random colour */
	for ( lParticle_I=0; lParticle_I < swarm->particleLocalCount; lParticle_I++ ) {
		currParticle = (Particle*)Swarm_ParticleAt( swarm, lParticle_I );
		for ( dim_I=0; dim_I < 3; dim_I++ ) {
			currParticle->velocity[dim_I] = 0;
		}	
		currParticle->randomColour = ( (double)  rand() ) / RAND_MAX;
	}
	
	if( rank == procToWatch ) {
		//Print( swarm, stream );
	}	

	Stg_Component_Build( context, 0 /* dummy */, False );
	Stg_Component_Initialise( context, 0 /* dummy */, False );
	
	/* +++ RUN PHASE +++ */
	AbstractContext_Dump( context );

	ContextEP_ReplaceAll( context, AbstractContext_EP_Dt, Dt );

	/* TODO: this should actually live on the Disc. context - but we need to rearrange the directories a bit
	first for this to happen - Main.PatrickSunter - 10 Jun 2006 */
	ContextEP_Append( context, AbstractContext_EP_Save, SaveSwarms );
	Stg_Component_Execute( context, 0 /* dummy */, False );
	Stg_Component_Destroy( context, 0 /* dummy */, False );

	/* Delete stuff */
	/* Deleting the component factory automatically deletes all components in it */
	// TODO: should the component factory be renamed a comp. manager? Since it deletes
	//	components as well?
	Stg_Class_Delete( cf );
	/* Remaining registers etc that don't live on the context or anything */
	Stg_Class_Delete( extensionMgr_Register );
	Stg_Class_Delete( swarmVariable_Register );
	/* Input/Output stuff */
	Stg_Class_Delete( dictionary );
	Stg_Class_Delete( ioHandler );
	
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


void SaveSwarms( void* context ) {
	
	Swarm_Register_SaveAllRegisteredSwarms( 
		Swarm_Register_GetSwarm_Register(), context );
}


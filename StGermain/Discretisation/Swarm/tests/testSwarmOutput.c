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
** $Id: testSwarmOutput.c 3851 2006-10-12 08:57:22Z SteveQuenette $
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
	__IntegrationPoint
};

double Dt( void* context ) {
	return 2.0;
}

void _SetDt( void* context, double dt ) {
}

void MoveParticles( AbstractContext* context ) {
	Swarm*            swarm = (Swarm*) LiveComponentRegister_Get( context->CF->LCRegister, "swarm" );
	Particle_Index    lParticle_I;
	GlobalParticle* particle;
	double            x,y;

	for ( lParticle_I = 0 ; lParticle_I < swarm->particleLocalCount; lParticle_I++ ) {
		particle = (GlobalParticle*)Swarm_ParticleAt( swarm, lParticle_I );

		x = particle->coord[ I_AXIS ];
		y = particle->coord[ J_AXIS ];
		particle->coord[ I_AXIS ] = 1.0 - y;
		particle->coord[ J_AXIS ] = x;
	}

	Swarm_UpdateAllParticleOwners( swarm );
}
	


int main( int argc, char* argv[] ) {
	MPI_Comm            CommWorld;
	int                 rank;
	int                 numProcessors;
	Dictionary*         dictionary;
	Dictionary*         componentDict;
	Stg_ComponentFactory*   cf;
	XML_IO_Handler*     ioHandler;
	DiscretisationContext* context;
	ExtensionManager_Register* extensionMgr_Register;
	SwarmVariable_Register* swarmVariable_Register;

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

	dictionary = Dictionary_New();
	ioHandler = XML_IO_Handler_New();
	IO_Handler_ReadAllFromCommandLine( ioHandler, argc, argv, dictionary );

	Journal_ReadFromDictionary( dictionary );

	/* Construction phase -------------------------------------------------------------------------------------------*/
	
	/* Create the Context */
	context = DiscretisationContext_New(
			"context",
			0,
			0,
			MPI_COMM_WORLD,
			dictionary );

	ContextEP_Append( context, AbstractContext_EP_Dt, Dt );
	ContextEP_Append( context, AbstractContext_EP_Step, MoveParticles );
	
	componentDict = Dictionary_GetDictionary( dictionary, "components" );
	
	assert( componentDict );

	cf = context->CF = Stg_ComponentFactory_New( dictionary, componentDict, context->register_Register );

	LiveComponentRegister_Add( cf->LCRegister, (Stg_Component*) context );

	extensionMgr_Register = ExtensionManager_Register_New();
	swarmVariable_Register = SwarmVariable_Register_New( NULL );
	Stg_ObjectList_ClassAppend( cf->registerRegister, (void*)extensionMgr_Register, "ExtensionManager_Register" );
	Stg_ObjectList_ClassAppend( cf->registerRegister, (void*)swarmVariable_Register, "SwarmVariable_Register" );

	Stg_ComponentFactory_CreateComponents( cf );
	Stg_ComponentFactory_ConstructComponents( cf, 0 /* dummy */ );

	LiveComponentRegister_BuildAll( cf->LCRegister, context );
	LiveComponentRegister_InitialiseAll( cf->LCRegister, context );

	Stg_Component_Build( context, 0 /* dummy */, False );
	Stg_Component_Initialise( context, 0 /* dummy */, False );
	AbstractContext_Dump( context );
	Stg_Component_Execute( context, 0 /* dummy */, False );
	Stg_Component_Destroy( context, 0 /* dummy */, False );
	
	/* Destroy stuff */
	/* TODO LiveComponentRegister_DeleteAll( cf->LCRegister ); */
	Stg_Class_Delete( extensionMgr_Register );
	Stg_Class_Delete( swarmVariable_Register );
	
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

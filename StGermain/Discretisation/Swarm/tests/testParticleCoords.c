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
** $Id: testParticleCoords.c 3851 2006-10-12 08:57:22Z SteveQuenette $
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

int main( int argc, char* argv[] ) {
	MPI_Comm            CommWorld;
	int                 rank;
	int                 numProcessors;
	int                 procToWatch;
	Dictionary*         dictionary;
	Dictionary*         componentDict;
	Stg_ComponentFactory*   cf;
	Swarm*              swarm;
	Stream*             stream;
	XML_IO_Handler*     ioHandler;
	ExtensionManager_Register* extensionMgr_Register;
	Variable_Register*  variable_Register;

	DiscretisationContext*    context;

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
	componentDict = Dictionary_GetDictionary( dictionary, "components" );
	
	assert( componentDict );

	context = DiscretisationContext_New( "context", 0, 0, MPI_COMM_WORLD, dictionary );

	context->CF = Stg_ComponentFactory_New( dictionary, componentDict, Stg_ObjectList_New() );
	cf = context->CF;
        LiveComponentRegister_Add( context->CF->LCRegister, (Stg_Component*) context );

	extensionMgr_Register = ExtensionManager_Register_New();
	variable_Register = Variable_Register_New( );
	Stg_ObjectList_ClassAppend( cf->registerRegister, (void*)extensionMgr_Register, "ExtensionManager_Register" );
	Stg_ObjectList_ClassAppend( cf->registerRegister, (void*)variable_Register, "Variable_Register" );

	Stg_ComponentFactory_CreateComponents( cf );
	Stg_ComponentFactory_ConstructComponents( cf, 0 /* dummy */ );

	LiveComponentRegister_BuildAll( cf->LCRegister, NULL );
	LiveComponentRegister_InitialiseAll( cf->LCRegister, NULL );

	stream = Journal_Register (Info_Type, "myStream");

	procToWatch = Dictionary_GetUnsignedInt( dictionary, "procToWatch" );
	if( rank == procToWatch )  {
		printf( "Watching rank: %i\n", rank );
	
		Stream_RedirectFile_WithPrependedPath( stream, Dictionary_GetString( dictionary, "outputPath" ), "output.dat" );

		swarm = (Swarm*) LiveComponentRegister_Get( cf->LCRegister, "swarm" );
		assert(swarm);

		Swarm_PrintParticleCoords( swarm, stream );
	}
		
	/* Destroy stuff */
	/* TODO LiveComponentRegister_DeleteAll( cf->LCRegister ); */
	Stg_Class_Delete( extensionMgr_Register );
	Stg_Class_Delete( variable_Register );
	
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

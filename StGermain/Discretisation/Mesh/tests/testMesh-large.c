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
** $Id: testMesh-large.c 3664 2006-07-04 04:26:57Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"
#include "Discretisation/Mesh/Mesh.h"

struct _Node
{
	double temp;
};


struct _Element
{
	double temp;
};


int main( int argc, char* argv[] ) {
	MPI_Comm		CommWorld;
	int			rank;
	int			numProcessors;
	int			procToWatch;
	Dictionary*		dictionary;
	ExtensionManager_Register*	extensionMgr_Register;
	Topology*		nTopology;
	ElementLayout*		eLayout;
	NodeLayout*		nLayout;
	HexaMD*		decomp;
	MeshLayout*		meshLayout;
	Mesh*			mesh;
	
	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size( CommWorld, &numProcessors );
	MPI_Comm_rank( CommWorld, &rank );
	
	procToWatch = (argc >= 2) ? atoi( argv[1] ) : 0;
	
	if ( rank == procToWatch ) printf( "Watching rank %d\n", rank );
	
	Base_Init( &argc, &argv );
	
	DiscretisationGeometry_Init( &argc, &argv );
	DiscretisationShape_Init( &argc, &argv );
	DiscretisationMesh_Init( &argc, &argv );
	MPI_Barrier( CommWorld ); /* Ensures copyright info always come first in output */

	dictionary = Dictionary_New();
	/* Use default settings ... see if not having G2L slows us down too much. */
	Dictionary_Add( dictionary, "meshSizeI", Dictionary_Entry_Value_FromUnsignedInt( 33 ) );
	Dictionary_Add( dictionary, "meshSizeJ", Dictionary_Entry_Value_FromUnsignedInt( 33 ) );
	Dictionary_Add( dictionary, "meshSizeK", Dictionary_Entry_Value_FromUnsignedInt( 33 ) );
	Dictionary_Add( dictionary, "shadowDepth", Dictionary_Entry_Value_FromUnsignedInt( 1 ) );
	Dictionary_Add( dictionary, "buildElementNodeTbl", Dictionary_Entry_Value_FromBool( True ) );
	Dictionary_Add( dictionary, "buildElementNeighbourTbl", Dictionary_Entry_Value_FromBool( True ) );
	Dictionary_Add( dictionary, "buildNodeElementTbl", Dictionary_Entry_Value_FromBool( True ) );
	Dictionary_Add( dictionary, "buildNodeNeighbourTbl", Dictionary_Entry_Value_FromBool( True ) );
	Dictionary_Add( dictionary, "allowPartitionOnNode", Dictionary_Entry_Value_FromBool( True ) );
	Dictionary_Add( dictionary, "allowUnbalancing", Dictionary_Entry_Value_FromBool( True ) );
	
	nTopology = (Topology*)IJK6Topology_New( "IJK6Topology", dictionary );
	eLayout = (ElementLayout*)ParallelPipedHexaEL_New( "PPHexaEL", 3, dictionary );
	nLayout = (NodeLayout*)CornerNL_New( "CornerNL", dictionary, eLayout, nTopology );
	decomp = HexaMD_New( "HexaMD", dictionary, MPI_COMM_WORLD, eLayout, nLayout );
	meshLayout = MeshLayout_New( "MeshLayout", eLayout, nLayout, (MeshDecomp*)decomp );
	
	extensionMgr_Register = ExtensionManager_Register_New();
	mesh = Mesh_New( "Mesh", meshLayout, sizeof(Node), sizeof(Element), extensionMgr_Register, dictionary );
	
	if ( rank == procToWatch ) printf( "Building mesh:\n" );
	Build( mesh, 0, False );
	Initialise(mesh, 0, False );
	if ( rank == procToWatch ) printf( "Done.\n" );
	if ( rank == procToWatch ) {
		Partition_Index proc_I;

		printf( "partitionedAxis: { %s, %s, %s }\n", decomp->partitionedAxis[0] ? "True" : "False", 
			decomp->partitionedAxis[1] ? "True" : "False", decomp->partitionedAxis[2] ? "True" : "False" );
		printf( "partitionCounts: { %u, %u, %u }\n", decomp->partition3DCounts[0], decomp->partition3DCounts[1], 
			decomp->partition3DCounts[2] );
		
		printf( "elementGlobalCounts: { %u, %u, %u }\n", decomp->elementGlobal3DCounts[0], decomp->elementGlobal3DCounts[1], 
			decomp->elementGlobal3DCounts[2] );
		for( proc_I = 0; proc_I < decomp->procsInUse; proc_I++ ) {
			printf( "\telementLocalCounts[%u]: { %u, %u, %u }\n", proc_I, decomp->elementLocal3DCounts[proc_I][0], 
				decomp->elementLocal3DCounts[proc_I][1], decomp->elementLocal3DCounts[proc_I][2] );
		}
		
		printf( "nodeGlobalCounts: { %u, %u, %u }\n", decomp->nodeGlobal3DCounts[0], decomp->nodeGlobal3DCounts[1], 
			decomp->nodeGlobal3DCounts[2] );
		for( proc_I = 0; proc_I < decomp->procsInUse; proc_I++ ) {
			printf( "\tnodeLocalCounts[%u]: { %u, %u, %u }\n", proc_I, decomp->nodeLocal3DCounts[proc_I][0], 
				decomp->nodeLocal3DCounts[proc_I][1], decomp->nodeLocal3DCounts[proc_I][2] );
		}
	}
	
	Stg_Class_Delete( mesh );
	Stg_Class_Delete( extensionMgr_Register );
	Stg_Class_Delete( meshLayout );
	Stg_Class_Delete( decomp );
	Stg_Class_Delete( nLayout );
	Stg_Class_Delete( eLayout );
	Stg_Class_Delete( nTopology );
	Stg_Class_Delete( dictionary );
	
	DiscretisationMesh_Finalise();
	DiscretisationShape_Finalise();
	DiscretisationGeometry_Finalise();
	
	Base_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0;
}

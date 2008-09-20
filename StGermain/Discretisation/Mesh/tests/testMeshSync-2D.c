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
** $Id: testMeshSync.c 2136 2004-09-30 02:47:13Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"
#include "Discretisation/Mesh/Mesh.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

struct _Node
{
	double temp;
};

struct _Element
{
	double temp;
};


int main( int argc, char* argv[] )
{
	MPI_Comm		CommWorld;
	int			rank;
	int			procCount;
	int			procToWatch;
	Dictionary*		dictionary;
	ExtensionManager_Register*	extensionMgr_Register;
	Topology*		nTopology;
	ElementLayout*		eLayout;
	NodeLayout*		nLayout;
	MeshDecomp*		decomp;
	MeshLayout*		ml;
	Mesh*			mesh;
	Stream*			stream;
	Node_DomainIndex	node_dI;
	Element_DomainIndex	element_dI;
	
	/* Initialise MPI, get world info */
	MPI_Init(&argc, &argv);
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size(CommWorld, &procCount);
	MPI_Comm_rank(CommWorld, &rank);

	Base_Init( &argc, &argv );
	
	DiscretisationGeometry_Init( &argc, &argv );
	DiscretisationShape_Init( &argc, &argv );
	DiscretisationMesh_Init( &argc, &argv );
	MPI_Barrier( CommWorld ); /* Ensures copyright info always come first in output */

	stream = Journal_Register (Info_Type, "myStream");
	procToWatch = argc >= 2 ? atoi(argv[1]) : 0;
	
	dictionary = Dictionary_New();
	Dictionary_Add( dictionary, "rank", Dictionary_Entry_Value_FromUnsignedInt( rank ) );
	Dictionary_Add( dictionary, "numProcessors", Dictionary_Entry_Value_FromUnsignedInt( procCount ) );
	Dictionary_Add( dictionary, "meshSizeI", Dictionary_Entry_Value_FromUnsignedInt( 3 ) );
	Dictionary_Add( dictionary, "meshSizeJ", Dictionary_Entry_Value_FromUnsignedInt( 3 ) );
	Dictionary_Add( dictionary, "meshSizeK", Dictionary_Entry_Value_FromUnsignedInt( 3 ) );
	Dictionary_Add( dictionary, "allowUnusedCPUs", Dictionary_Entry_Value_FromBool( False ) );
	Dictionary_Add( dictionary, "allowPartitionOnElement", Dictionary_Entry_Value_FromBool( False ) );
	Dictionary_Add( dictionary, "allowPartitionOnNode", Dictionary_Entry_Value_FromBool( True ) );
	Dictionary_Add( dictionary, "allowUnbalancing", Dictionary_Entry_Value_FromBool( False ) );
	Dictionary_Add( dictionary, "shadowDepth", Dictionary_Entry_Value_FromUnsignedInt( 1 ) );
	
	nTopology = (Topology*)IJK6Topology_New( "IJK6Topology", dictionary );
	eLayout = (ElementLayout*)ParallelPipedHexaEL_New( "PPHexaEL", 3, dictionary );
	nLayout = (NodeLayout*)CornerNL_New( "CornerNL", dictionary, eLayout, nTopology );
	decomp = (MeshDecomp*)HexaMD_New_All( "HexaMD", dictionary, MPI_COMM_WORLD, eLayout, nLayout, 2 );
	ml = MeshLayout_New( "MeshLayout", eLayout, nLayout, decomp );
	
	extensionMgr_Register = ExtensionManager_Register_New();
	mesh = Mesh_New( "Mesh", ml, sizeof(Node), sizeof(Element), extensionMgr_Register, dictionary );
	
	mesh->buildNodeLocalToGlobalMap = True;
	mesh->buildNodeDomainToGlobalMap = True;
	mesh->buildNodeGlobalToLocalMap = True;
	mesh->buildNodeGlobalToDomainMap = True;
	mesh->buildNodeNeighbourTbl = True;
	mesh->buildNodeElementTbl = True;
	mesh->buildElementLocalToGlobalMap = True;
	mesh->buildElementDomainToGlobalMap = True;
	mesh->buildElementGlobalToDomainMap = True;
	mesh->buildElementGlobalToLocalMap = True;
	mesh->buildElementNeighbourTbl = True;
	mesh->buildElementNodeTbl = True;
	Build( mesh, 0, False );
	Initialise(mesh, 0, False );
	
	for( node_dI = 0; node_dI < mesh->nodeDomainCount; node_dI++ ) {
		mesh->nodeCoord[node_dI][0] = (double)rank;
		mesh->nodeCoord[node_dI][1] = (double)rank;
		mesh->nodeCoord[node_dI][2] = (double)rank;
		mesh->node[node_dI].temp = 10.0 + (double)rank;
	}
	for( element_dI = 0; element_dI < mesh->elementDomainCount; element_dI++ ) {
		mesh->element[element_dI].temp = 20.0 + (double)rank;
	}
	
	if( rank == procToWatch ) {
		Print(mesh, stream);
		
		printf( "Pre-sync:\n" );
		for( node_dI = 0; node_dI < mesh->nodeDomainCount; node_dI++ ) {
			printf( "\tnodeCoord[%d]: { %g, %g, %g } - temp: %g\n",  node_dI, mesh->nodeCoord[node_dI][0],
				mesh->nodeCoord[node_dI][1], mesh->nodeCoord[node_dI][2], mesh->node[node_dI].temp );
		}
		for( element_dI = 0; element_dI < mesh->elementDomainCount; element_dI++ )
			printf( "\telement[%d]: temp: %g\n", element_dI, mesh->element[element_dI].temp );
	}
	
	Mesh_Sync( mesh );
	
	if( rank == procToWatch ) {
		printf( "Post-sync:\n" );
		for( node_dI = 0; node_dI < mesh->nodeDomainCount; node_dI++ ) {
			printf( "\tnodeCoord[%d]: { %g, %g, %g } - temp: %g\n",  node_dI, mesh->nodeCoord[node_dI][0],
				mesh->nodeCoord[node_dI][1], mesh->nodeCoord[node_dI][2], mesh->node[node_dI].temp );
		}
		for( element_dI = 0; element_dI < mesh->elementDomainCount; element_dI++ )
			printf( "\telement[%d]: temp: %g\n", element_dI, mesh->element[element_dI].temp );
	}
	
	Stg_Class_Delete( mesh );
	Stg_Class_Delete( ml );
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
	
	return 0; /* success */
}

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
** $Id: testMesh.c 2498 2005-01-07 03:57:00Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"
#include "Discretisation/Mesh/Mesh.h"
#include "Discretisation/Utils/Utils.h"

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


int main(int argc, char *argv[])
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
	MeshCoarsener_Hexa*	coarsener;
	Stream*			stream;
	
	/* Initialise MPI, get world info */
	MPI_Init(&argc, &argv);
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size(CommWorld, &procCount);
	MPI_Comm_rank(CommWorld, &rank);

	Base_Init( &argc, &argv );
	
	DiscretisationGeometry_Init( &argc, &argv );
	DiscretisationShape_Init( &argc, &argv );
	DiscretisationMesh_Init( &argc, &argv );

	MPI_Barrier( CommWorld );

	stream = Journal_Register (Info_Type, "myStream");
	procToWatch = argc >= 2 ? atoi(argv[1]) : 0;
	
	dictionary = Dictionary_New();
	Dictionary_Add( dictionary, "rank", Dictionary_Entry_Value_FromUnsignedInt( rank ) );
	Dictionary_Add( dictionary, "numProcessors", Dictionary_Entry_Value_FromUnsignedInt( procCount ) );
	Dictionary_Add( dictionary, "meshSizeI", Dictionary_Entry_Value_FromUnsignedInt( 21 ) );
	Dictionary_Add( dictionary, "meshSizeJ", Dictionary_Entry_Value_FromUnsignedInt( 21 ) );
	Dictionary_Add( dictionary, "meshSizeK", Dictionary_Entry_Value_FromUnsignedInt( 21 ) );
	Dictionary_Add( dictionary, "allowUnusedCPUs", Dictionary_Entry_Value_FromBool( True ) );
	Dictionary_Add( dictionary, "allowPartitionOnElement", Dictionary_Entry_Value_FromBool( True ) );
	Dictionary_Add( dictionary, "allowPartitionOnNode", Dictionary_Entry_Value_FromBool( True ) );
	Dictionary_Add( dictionary, "allowUnbalancing", Dictionary_Entry_Value_FromBool( False ) );
	Dictionary_Add( dictionary, "shadowDepth", Dictionary_Entry_Value_FromUnsignedInt( 0 ) );
	
	nTopology = (Topology*)IJK6Topology_New( "IJK6Topology", dictionary );
	eLayout = (ElementLayout*)ParallelPipedHexaEL_New( "PPHexaEL", 3, dictionary );
	nLayout = (NodeLayout*)CornerNL_New( "CornerNL", dictionary, eLayout, nTopology );
	decomp = (MeshDecomp*)HexaMD_New( "HexaMD", dictionary, MPI_COMM_WORLD, eLayout, nLayout );
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
	
	/* Coarsen by one level */
	coarsener = MeshCoarsener_Hexa_New("meshCoarsener");
	MeshCoarsener_Hexa_Coarsen( coarsener, mesh, 1, NULL, NULL );
	Stg_Class_Delete( coarsener );
	
	Build( mesh, 0, False );
	Initialise(mesh, 0, False );
	
	if (rank == procToWatch)
	{
		Print(mesh, stream);
	}
	
	Stg_Class_Delete(mesh);
	Stg_Class_Delete(ml);
	Stg_Class_Delete(decomp);
	Stg_Class_Delete(nLayout);
	Stg_Class_Delete(eLayout);
	Stg_Class_Delete( nTopology );
	Stg_Class_Delete(dictionary);
	
	DiscretisationMesh_Finalise();
	DiscretisationShape_Finalise();
	DiscretisationGeometry_Finalise();
	
	Base_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0; /* success */
}

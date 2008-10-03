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
** $Id: testSemiRegDeform.c 3124 2005-07-25 04:52:06Z RobertTurnbull $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#include "Base/Base.h"
#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"
#include "Discretisation/Mesh/Mesh.h"

#include "Discretisation/Utils/types.h"
#include "Discretisation/Utils/SemiRegDeform.h"


struct _Node {
	double tmp;
};

struct _Element {
	double tmp;
};


int main( int argc, char* argv[] ) {
	MPI_Comm			commWorld;
	int				rank;
	int				nProcs;
	int				procToWatch;
	Dictionary*			dict;
	ExtensionManager_Register*	extMgrReg;
	Topology*			nTopo;
	ElementLayout*			eLyt;
	NodeLayout*			nLyt;
	MeshDecomp*			decomp;
	MeshLayout*			mLyt;
	Mesh*				mesh;
	Stream*				stream;
	SemiRegDeform*			srd;


	/*
	** Initialise MPI, StGermain Base, get world info.
	*/

	MPI_Init( &argc, &argv );
	MPI_Comm_dup( MPI_COMM_WORLD, &commWorld );
	MPI_Comm_size( commWorld, &nProcs );
	MPI_Comm_rank( commWorld, &rank );

	Base_Init( &argc, &argv );
	DiscretisationGeometry_Init( &argc, &argv );
	DiscretisationShape_Init( &argc, &argv );
	DiscretisationMesh_Init( &argc, &argv );
	MPI_Barrier( commWorld ); /* Ensures copyright info always come first in output */

	stream = Journal_Register( Info_Type, "myStream" );
	procToWatch = argc >= 2 ? atoi(argv[1]) : 0;

	/*
	** Polpulate the dictionary.
	*/

	dict = Dictionary_New();
	Dictionary_Add( dict, "rank", Dictionary_Entry_Value_FromUnsignedInt( rank ) );
	Dictionary_Add( dict, "numProcessors", Dictionary_Entry_Value_FromUnsignedInt( nProcs ) );
	Dictionary_Add( dict, "meshSizeI", Dictionary_Entry_Value_FromUnsignedInt( 4 ) );
	Dictionary_Add( dict, "meshSizeJ", Dictionary_Entry_Value_FromUnsignedInt( 5 ) );
	Dictionary_Add( dict, "meshSizeK", Dictionary_Entry_Value_FromUnsignedInt( 4 ) );
	Dictionary_Add( dict, "allowUnusedCPUs", Dictionary_Entry_Value_FromBool( False ) );
	Dictionary_Add( dict, "allowPartitionOnElement", Dictionary_Entry_Value_FromBool( False ) );
	Dictionary_Add( dict, "allowPartitionOnNode", Dictionary_Entry_Value_FromBool( True ) );
	Dictionary_Add( dict, "allowUnbalancing", Dictionary_Entry_Value_FromBool( False ) );
	Dictionary_Add( dict, "shadowDepth", Dictionary_Entry_Value_FromUnsignedInt( 0 ) );


	/*
	** Create the mesh.
	*/

	nTopo = (Topology*)IJK6Topology_New( "IJK6Topology", dict );
	eLyt = (ElementLayout*)ParallelPipedHexaEL_New( "PPHexaEL", 3, dict );
	nLyt = (NodeLayout*)CornerNL_New( "CornerNL", dict, eLyt, nTopo );
	decomp = (MeshDecomp*)HexaMD_New( "HexaMD", dict, commWorld, eLyt, nLyt );
	mLyt = MeshLayout_New( "MeshLayout", eLyt, nLyt, decomp );
	
	extMgrReg = ExtensionManager_Register_New();
	mesh = Mesh_New( "Mesh", mLyt, sizeof(Node), sizeof(Element), extMgrReg, dict );

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


	/*
	** Build phase.
	*/

	Build( mesh, 0, False );


	/*
	** Initialisation phase.
	*/

	Initialise(mesh, 0, False );


	/*
	** Create the deformation.
	*/

	{
		IJK		ijk;
		unsigned	lower, upper;

		srd = SemiRegDeform_New( "SemiRegDeform" );
		SemiRegDeform_SetMesh( srd, mesh );

		/* Set up strips to remesh in the y direction. */
		for( ijk[2] = 0; ijk[2] < ((HexaMD*)decomp)->nodeGlobal3DCounts[2]; ijk[2]++ ) {
			for( ijk[0] = 0; ijk[0] < ((HexaMD*)decomp)->nodeGlobal3DCounts[0]; ijk[0]++ ) {
				ijk[1] = 0;
				GRM_Project( &srd->grm, ijk, &lower );

				ijk[1] = ((HexaMD*)decomp)->nodeGlobal3DCounts[1] - 1;
				GRM_Project( &srd->grm, ijk, &upper );

				SemiRegDeform_AddStrip( srd, lower, upper );
			}
		}

		/* Build and initialise. */
		Build( srd, 0, False );
		Initialise( srd, 0, False );

		/* Execute the deformation. */
		SemiRegDeform_Deform( srd );

		/* Check the deformation. */
		if (rank == procToWatch) {
		}

		/* Kill it. */
		Stg_Class_Delete( srd );
	}

	{
		IJK		ijk;
		unsigned	lower, upper;

		srd = SemiRegDeform_New( "SemiRegDeform" );
		SemiRegDeform_SetMesh( srd, mesh );

		/* Set up strips to remesh in the y direction. */
		for( ijk[2] = 0; ijk[2] < ((HexaMD*)decomp)->nodeGlobal3DCounts[2]; ijk[2]++ ) {
			for( ijk[0] = 0; ijk[0] < ((HexaMD*)decomp)->nodeGlobal3DCounts[0]; ijk[0]++ ) {
				ijk[1] = 0;
				GRM_Project( &srd->grm, ijk, &lower );
				ijk[1] = ((HexaMD*)decomp)->nodeGlobal3DCounts[1] - 2;
				GRM_Project( &srd->grm, ijk, &upper );
				SemiRegDeform_AddStrip( srd, lower, upper );

				lower = upper;
				ijk[1] = ((HexaMD*)decomp)->nodeGlobal3DCounts[1] - 1;
				GRM_Project( &srd->grm, ijk, &upper );
				SemiRegDeform_AddStrip( srd, lower, upper );
			}
		}

		/* Build and initialise. */
		Build( srd, 0, False );
		Initialise( srd, 0, False );

		/* Check the deformation. */
		if (rank == procToWatch) {
		}

		/* Kill it. */
		Stg_Class_Delete( srd );
	}


	/*
	** Cleanup.
	*/
	
	Stg_Class_Delete( mesh );
	Stg_Class_Delete( mLyt );
	Stg_Class_Delete( decomp );
	Stg_Class_Delete( nLyt );
	Stg_Class_Delete( eLyt );
	Stg_Class_Delete( nTopo );
	Stg_Class_Delete( dict );

	DiscretisationMesh_Finalise();
	DiscretisationShape_Finalise();
	DiscretisationGeometry_Finalise();
	Base_Finalise();

	/* Close off MPI */
	MPI_Finalize();

	return EXIT_SUCCESS;
}

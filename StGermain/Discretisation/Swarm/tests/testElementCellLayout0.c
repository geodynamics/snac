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
** $Id: testElementCellLayout0.c 3555 2006-05-10 07:05:46Z PatrickSunter $
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

#define ConvertNode(node) \
	(node == 2 ? 3 : node == 3 ? 2 : node == 6 ? 7 : node == 7 ? 6 : node)

struct _Node {
	double temp;
};

struct _Element {
	double temp;
};

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
	Dictionary_Add( dictionary, "buildElementNodeTbl", Dictionary_Entry_Value_FromBool( True ) );
	Dictionary_Add( dictionary, "minX", Dictionary_Entry_Value_FromDouble( 0.0f ) );
	Dictionary_Add( dictionary, "minY", Dictionary_Entry_Value_FromDouble( 0.0f ) );
	Dictionary_Add( dictionary, "minZ", Dictionary_Entry_Value_FromDouble( 0.0f ) );
	Dictionary_Add( dictionary, "maxX", Dictionary_Entry_Value_FromDouble( 300.0f ) );
	Dictionary_Add( dictionary, "maxY", Dictionary_Entry_Value_FromDouble( 12.0f ) );
	Dictionary_Add( dictionary, "maxZ", Dictionary_Entry_Value_FromDouble( 300.0f ) );
	
	/* Run the mesher */
	nTopology = (Topology*)IJK6Topology_New( "IJK6Topology", dictionary );
	eLayout = (ElementLayout*)ParallelPipedHexaEL_New( "PPHexaEL", 3, dictionary );
	nLayout = (NodeLayout*)CornerNL_New( "CornerNL", dictionary, eLayout, nTopology );
	decomp = (MeshDecomp*)HexaMD_New( "HexaMD", dictionary, MPI_COMM_WORLD, eLayout, nLayout );
	layout = MeshLayout_New( "MeshLayout", eLayout, nLayout, decomp );
	
	/* Init mesh */
	extensionMgr_Register = ExtensionManager_Register_New();
	mesh = Mesh_New( "Mesh", layout, sizeof(Node), sizeof(Element), extensionMgr_Register, dictionary );
	
	/* Configure the element-cell-layout */
	elementCellLayout = ElementCellLayout_New( "elementCellLayout", mesh );
	
	/* Build the mesh */
	Build( mesh, 0, False );
	Initialise( mesh, 0, False );
	
	if( rank == procToWatch ) {
		Cell_Index cell;
		Element_DomainIndex	element;
		GlobalParticle          testParticle;
		
		for( element = 0; element < mesh->elementLocalCount; element++ ) {
			Cell_PointIndex			point;
			Cell_PointIndex			count;
			Coord**				cellPoints;
			Bool				result;

			cell = CellLayout_MapElementIdToCellId( elementCellLayout, element );

			if ( cell != element ) { 
				printf( "Wrong result: CellLayout_MapElementIdToCellId returned %d, when %d "
					"expected.\n", cell, element );
				exit(0);
			}

			count = elementCellLayout->_pointCount( elementCellLayout, cell );
			printf( "cellPointTbl  [%2u][0-%u]:\n", cell, count );
			cellPoints = Memory_Alloc_Array( Coord*, count, "cellPoints" );
			elementCellLayout->_initialisePoints( elementCellLayout, cell, count, cellPoints );
			for( point = 0; point < count; point++ ) {
				printf( "\t{%.3g %.3g %.3g}\n", (*cellPoints[ConvertNode(point)])[0], (*cellPoints[ConvertNode(point)])[1], 
					(*cellPoints[ConvertNode(point)])[2] );
			}
			printf( "\n" );

			testParticle.coord[0] = ( (*cellPoints[0])[0] + (*cellPoints[1])[0] ) / 2;
			testParticle.coord[1] = ( (*cellPoints[0])[1] + (*cellPoints[3])[1] ) / 2;
			testParticle.coord[2] = ( (*cellPoints[0])[2] + (*cellPoints[4])[2] ) / 2;
			printf( "Testing if test particle at (%f,%f,%f) is in the cell: ",
				testParticle.coord[0], testParticle.coord[1], testParticle.coord[2] );
			result = CellLayout_IsInCell( elementCellLayout, cell, &testParticle );
			printf( "%d\n\n", result );

			testParticle.coord[0] = (*cellPoints[count-1])[0] + 1;
			testParticle.coord[1] = (*cellPoints[count-1])[1] + 1;
			testParticle.coord[2] = (*cellPoints[count-1])[2] + 1;
			printf( "Testing if test particle at (%f,%f,%f) is in the cell: ",
				testParticle.coord[0], testParticle.coord[1], testParticle.coord[2] );
			result = CellLayout_IsInCell( elementCellLayout, cell, &testParticle );
			printf( "%d\n\n", result );

			Memory_Free( cellPoints );
		}
	}
	
	/* Destroy stuff */
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

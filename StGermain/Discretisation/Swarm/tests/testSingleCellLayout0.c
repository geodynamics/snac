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
**	Test that the SingleCellLayout can correctly initialise a cell, and check if a particle is contained by it.
**
** Assumptions:
**	None as yet.
**
** Comments:
**	None as yet.
**
** $Id: testSingleCellLayout0.c 3555 2006-05-10 07:05:46Z PatrickSunter $
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

int main( int argc, char* argv[] ) {
	MPI_Comm			CommWorld;
	int				rank;
	int				numProcessors;
	int				procToWatch;
	Dictionary*			dictionary;
	unsigned int			dimExists[] = { True, True, True };
	SingleCellLayout*		singleCellLayout;
	
	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size( CommWorld, &numProcessors );
	MPI_Comm_rank( CommWorld, &rank );
	
	Base_Init( &argc, &argv );
	
	Stream_SetLevel( stgMemory->infoStream, 3 );
	
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
	Dictionary_Add( dictionary, "minX", Dictionary_Entry_Value_FromDouble( 0.0f ) );
	Dictionary_Add( dictionary, "minY", Dictionary_Entry_Value_FromDouble( 0.0f ) );
	Dictionary_Add( dictionary, "minZ", Dictionary_Entry_Value_FromDouble( 0.0f ) );
	Dictionary_Add( dictionary, "maxX", Dictionary_Entry_Value_FromDouble( 300.0f ) );
	Dictionary_Add( dictionary, "maxY", Dictionary_Entry_Value_FromDouble( 12.0f ) );
	Dictionary_Add( dictionary, "maxZ", Dictionary_Entry_Value_FromDouble( 300.0f ) );
	
	/* Configure the single-cell-layout */
	singleCellLayout = SingleCellLayout_New( "singleCellLayout", dimExists, NULL, NULL );
	
	if( rank == procToWatch ) {
		Cell_Index cell;
		Coord**				cellPoints = NULL;
		Coord testCoord = { 0, 0, 0 };
		
		for( cell = 0; cell < singleCellLayout->_cellLocalCount( singleCellLayout ); cell++ ) {
			Cell_PointIndex			point;
			Cell_PointIndex			count;
			
			count = singleCellLayout->_pointCount( singleCellLayout, cell );
			printf( "cellPointTbl  [%2u][0-%u]:\n", cell, count );
			cellPoints = Memory_Alloc_Array( Coord*, count, "cellsPoints" );
			singleCellLayout->_initialisePoints( singleCellLayout, cell, count, cellPoints );
			for( point = 0; point < count; point++ ) {
				printf( "\t{%.3g %.3g %.3g}\n", (*cellPoints[point])[I_AXIS], (*cellPoints[point])[J_AXIS], 
					(*cellPoints[point])[K_AXIS] );
			}
			printf( "\n" );
		}
		printf( "Testing CellLayout_MapElementIdToCellId() always returns 0:\n" );
		printf( "\tCellLayout_MapElementIdToCellId( singleCellLayout, %u ) = %u\n", 
			0, CellLayout_MapElementIdToCellId( singleCellLayout, 0 ) );
		printf( "\tCellLayout_MapElementIdToCellId( singleCellLayout, %u ) = %u\n", 
			5, CellLayout_MapElementIdToCellId( singleCellLayout, 5 ) );
		printf( "\tCellLayout_MapElementIdToCellId( singleCellLayout, %u ) = %u\n", 
			100, CellLayout_MapElementIdToCellId( singleCellLayout, 100 ) );

		printf( "Testing that particles always belong to first cell:\n" );
		printf( "CellLayout_CellOf( singleCellLayout, X ) = %u\n\n", 
			CellLayout_CellOf( singleCellLayout, cellPoints[0] ) );

		printf( "Testing SingleCellLayout_IsInCell:\n" );
		printf( "For particle {0,0,0}: %u\n", 
			CellLayout_IsInCell( singleCellLayout, 0, &testCoord ) );
		testCoord[I_AXIS] = testCoord[J_AXIS] = testCoord[K_AXIS] = 1;
		printf( "For particle {1,1,1}: %u\n",
			CellLayout_IsInCell( singleCellLayout, 0, &testCoord ) );
		testCoord[I_AXIS] = 2;
		printf( "For particle {2,1,1}: %u\n",
			CellLayout_IsInCell( singleCellLayout, 0, &testCoord ) );
		Memory_Free( cellPoints );
	}
	

	
	/* Destroy stuff */
	Stg_Class_Delete( singleCellLayout );
	Stg_Class_Delete( dictionary );
	
	DiscretisationSwarm_Finalise();
	DiscretisationUtils_Finalise();
	DiscretisationMesh_Finalise();
	DiscretisationShape_Finalise();
	DiscretisationGeometry_Finalise();
	
	//Memory_Print_Leak();
	
	Base_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0; /* success */
}

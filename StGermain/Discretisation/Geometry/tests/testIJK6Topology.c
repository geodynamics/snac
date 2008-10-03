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
** $Id: testIJK6Topology.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"

#include <stdio.h>


void Test_PrintNeighbours( IJK6Topology* topology, Index i, Index j, Index k, Bool dynamicSizes ) {
	Index elNum;
	NeighbourIndex nbrCount;
	NeighbourIndex* elNbrs;
	char* dynStr;

	topology->dynamicSizes = dynamicSizes;
	if ( dynamicSizes ) {
		dynStr = "on";
	} 
	else {
		dynStr = "off";
	}
	
	IJK_3DTo1D_3( topology, i, j, k, &elNum );
	nbrCount = Topology_NeighbourCount( topology, elNum );
	elNbrs = Memory_Alloc_Array( NeighbourIndex, nbrCount, "elNbrs" );
	Topology_BuildNeighbours( topology, elNum, elNbrs );
	printf( "\tElNbrs[0-%d] = {", nbrCount-1 );
	for ( i=0; i < nbrCount; i++ ) {
		printf( "%d", elNbrs[i] );
		if ( elNbrs[i] == Topology_Invalid( topology ) ) {
			printf( "(Inv)");
		}
		printf( ", ");
	}
	printf( "}\n" );
	Memory_Free( elNbrs );
}	


void Test_TestTopologyOfSize( Dictionary* dictionary, Index sizeI, Index sizeJ, Index sizeK ) {	
	IJK6Topology*	topology;
	Index		i,j,k;
	Index elNum;

	Dictionary_Set( dictionary, "meshSizeI", Dictionary_Entry_Value_FromUnsignedInt( sizeI ) );
	Dictionary_Set( dictionary, "meshSizeJ", Dictionary_Entry_Value_FromUnsignedInt( sizeJ ) );
	Dictionary_Set( dictionary, "meshSizeK", Dictionary_Entry_Value_FromUnsignedInt( sizeK ) );

	printf( "*** Construction test ***\n" );
	topology = IJK6Topology_New( "topology", dictionary );
	printf( "topology->size[0-2] = { " );
	for ( i=0; i < 3; i++ ) {
		printf( "%d, ", topology->size[i] );
	}
	printf( "}\n\n" );

	printf( "*** Topology_NeighbourCount() and BuildNeighbour() tests. ***\n\n" );
	for ( k=0; k < sizeK; k++ ) {
		for ( j=0; j < sizeJ; j++ ) {
			for ( i=0; i < sizeI; i++ ) {
				IJK_3DTo1D_3( topology, i, j, k, &elNum );
				printf( "Reference object, loc (%d,%d,%d) (%d)\n", 
					i, j, k, elNum );
				printf( "Dynamic:\n" );
				Test_PrintNeighbours( topology, i, j, k, True );
				printf( "Non-Dynamic\n" );
				Test_PrintNeighbours( topology, i, j, k, False );
				printf( "Non-Dynamic, Periodic\n" );
				topology->isPeriodic[I_AXIS] = True;
				topology->isPeriodic[J_AXIS] = True;
				topology->isPeriodic[K_AXIS] = True;
				Test_PrintNeighbours( topology, i, j, k, False );
				topology->isPeriodic[I_AXIS] = False;
				topology->isPeriodic[J_AXIS] = False;
				topology->isPeriodic[K_AXIS] = False;
				printf( "\n" );
			}
		}	
	}

	Stg_Class_Delete( topology );
}	


int main( int argc, char* argv[] ) {
	Dictionary*	dictionary;
	int		rank;
	int		numProcessors;
	MPI_Comm	CommWorld;
	
	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size( CommWorld, &numProcessors );
	MPI_Comm_rank( CommWorld, &rank );
	
	Base_Init( &argc, &argv );
	DiscretisationGeometry_Init( &argc, &argv );
	MPI_Barrier( CommWorld ); /* Ensures copyright info always come first in output */
	
	dictionary = Dictionary_New();
	Dictionary_Add( dictionary, "meshSizeI", Dictionary_Entry_Value_FromUnsignedInt( 1 ) );
	Dictionary_Add( dictionary, "meshSizeJ", Dictionary_Entry_Value_FromUnsignedInt( 1 ) );
	Dictionary_Add( dictionary, "meshSizeK", Dictionary_Entry_Value_FromUnsignedInt( 1 ) );

	printf( "+++ 1D Tests +++\n\n" );
	Test_TestTopologyOfSize( dictionary, 3, 1, 1 );
	printf( "+++ 2D Tests +++\n\n" );
	Test_TestTopologyOfSize( dictionary, 3, 3, 1 );
	printf( "+++ 3D Tests +++\n\n" );
	Test_TestTopologyOfSize( dictionary, 3, 3, 3 );

	Stg_Class_Delete( dictionary );
		
	DiscretisationGeometry_Finalise();
	Base_Finalise();
	MPI_Finalize();
	
	return 0;
}	

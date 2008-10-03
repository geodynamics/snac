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
** $Id: testParallelPipedHexaEL.c 3817 2006-09-27 06:55:42Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdlib.h>
#include <stdio.h>

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"
#include "Discretisation/Mesh/Mesh.h"


int main( int argc, char* argv[] ) {
	MPI_Comm		CommWorld;
	int			rank;
	int			procCount;
	Dictionary*		dictionary;
	Geometry*		geometry;
	ElementLayout*		eLayout;
	Topology*		nTopology;
	NodeLayout*		nLayout;
	HexaMD*			meshDecomp;
	Index			i;
	Processor_Index		procToWatch;
	
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
	
	procToWatch = argc >= 2 ? atoi(argv[1]) : 0;
	
	dictionary = Dictionary_New();
	Dictionary_Add( dictionary, "meshSizeI", Dictionary_Entry_Value_FromUnsignedInt( 13 ) );
	Dictionary_Add( dictionary, "meshSizeJ", Dictionary_Entry_Value_FromUnsignedInt( 4 ) );
	Dictionary_Add( dictionary, "meshSizeK", Dictionary_Entry_Value_FromUnsignedInt( 4 ) );
	Dictionary_Add( dictionary, "maxX", Dictionary_Entry_Value_FromUnsignedInt( 6 ) );
	Dictionary_Add( dictionary, "allowUnbalancing", Dictionary_Entry_Value_FromBool( True ) );
	Dictionary_Add( dictionary, "shadowDepth", Dictionary_Entry_Value_FromUnsignedInt( 1 ) );
	Dictionary_Add( dictionary, "isPeriodicI", Dictionary_Entry_Value_FromBool( True ) );

	
	eLayout = (ElementLayout*)ParallelPipedHexaEL_New( "PPHexaEL", 3, dictionary );
	nTopology = (Topology*)IJK6Topology_New( "IJK6Topology", dictionary );
	nLayout = (NodeLayout*)CornerNL_New( "CornerNL", dictionary, eLayout, nTopology );
	meshDecomp = HexaMD_New( "HexaMD", dictionary, MPI_COMM_WORLD, eLayout, nLayout );
	
	ElementLayout_Build( eLayout, meshDecomp );
	
	if (rank == procToWatch) {
		printf( "Element with point:\n" );
	}
	geometry = eLayout->geometry;
	for( i = 0; i < geometry->pointCount; i++ ) {
		Coord point;
		int excEl, incEl;
		
		geometry->pointAt( geometry, i, point );

		if (rank == procToWatch) {
			printf( "\tNode %u (%0.2f,%0.2f,%0.2f):\n", i, point[0], point[1], point[2] );
			excEl = eLayout->elementWithPoint( eLayout, meshDecomp, point, NULL, 
							   EXCLUSIVE_UPPER_BOUNDARY, 0, NULL );
			incEl = eLayout->elementWithPoint( eLayout, meshDecomp, point, NULL, 
							   INCLUSIVE_UPPER_BOUNDARY, 0, NULL );
			printf( "\t\tIncl %4u, Excl %4u\n", incEl, excEl );		
		}

		point[0] += 0.1;
		point[1] += 0.1;
		point[2] += 0.1;
		
		if (rank == procToWatch) {
			printf( "\tTest point %u (%0.2f,%0.2f,%0.2f):\n", i, point[0], point[1], point[2] );
			excEl = eLayout->elementWithPoint( eLayout, meshDecomp, point, NULL, 
							   EXCLUSIVE_UPPER_BOUNDARY, 0, NULL );
			incEl = eLayout->elementWithPoint( eLayout, meshDecomp, point, NULL, 
							   INCLUSIVE_UPPER_BOUNDARY, 0, NULL );
			printf( "\t\tIncl %4u, Excl %4u\n", incEl, excEl );		
		}
	}
	if (rank == procToWatch) {
		printf( "\n" );
	}	
	
	Stg_Class_Delete( dictionary );
	Stg_Class_Delete( meshDecomp );
	Stg_Class_Delete( nLayout );
	Stg_Class_Delete( nTopology );
	Stg_Class_Delete( eLayout );
	
	DiscretisationMesh_Finalise();
	DiscretisationShape_Finalise();
	DiscretisationGeometry_Finalise();
	
	Base_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0;
}

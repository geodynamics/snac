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
** $Id: testShellGeometry.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"

#include <stdio.h>


int main( int argc, char* argv[] ) {
	Dictionary*	dictionary;
	Geometry*	geometry;
	Coord_List	points;
	Index		i;
	MPI_Comm	CommWorld;
	int		rank;
	int		numProcessors;
	
	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size( CommWorld, &numProcessors );
	MPI_Comm_rank( CommWorld, &rank );
	
	Base_Init( &argc, &argv );
	
	DiscretisationGeometry_Init( &argc, &argv );
	MPI_Barrier( CommWorld ); /* Ensures copyright info always come first in output */
	
	dictionary = Dictionary_New();
	Dictionary_Add( dictionary, "meshSizeI", Dictionary_Entry_Value_FromUnsignedInt( 4 ) );
	Dictionary_Add( dictionary, "meshSizeJ", Dictionary_Entry_Value_FromUnsignedInt( 4 ) );
	Dictionary_Add( dictionary, "meshSizeK", Dictionary_Entry_Value_FromUnsignedInt( 4 ) );
	Dictionary_Add( dictionary, "minX", Dictionary_Entry_Value_FromDouble( 3.0 * M_PI / 4.0 ) );
	Dictionary_Add( dictionary, "minY", Dictionary_Entry_Value_FromDouble( 3.0 * M_PI / 4.0 ) );
	Dictionary_Add( dictionary, "minZ", Dictionary_Entry_Value_FromDouble( 0.5 ) );
	Dictionary_Add( dictionary, "maxX", Dictionary_Entry_Value_FromDouble( M_PI / 4.0 ) );
	Dictionary_Add( dictionary, "maxY", Dictionary_Entry_Value_FromDouble( M_PI / 4.0 ) );
	Dictionary_Add( dictionary, "maxZ", Dictionary_Entry_Value_FromDouble( 1.0 ) );
	
	geometry = (Geometry*)ShellGeometry_New( "shellGeometry", dictionary );
	
	printf( "Individual point test:\n" );
	for( i = 0; i < geometry->pointCount; i++ ) {
		Coord point;
		
		printf( "\tPoint %u : ", i );
		geometry->pointAt( geometry, i, point );
		printf( "{ %g, %g, %g }\n", point[0], point[1], point[2] );
	}
	printf( "\n" );
	
	printf( "Total point test:\n" );
	points = Memory_Alloc_Array( Coord, geometry->pointCount, "points" );
	geometry->buildPoints( geometry, points );
	for( i = 0; i < geometry->pointCount; i++ ) {
		printf( "\tPoint %u : ", i );
		geometry->pointAt( geometry, i, points[i] );
		printf( "{ %g, %g, %g }\n", points[i][0], points[i][1], points[i][2] );
	}
	printf( "\n" );
	if( points )
		Memory_Free( points );
	
	Stg_Class_Delete( dictionary );
	
	DiscretisationGeometry_Finalise();
	
	Base_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0;
}

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
** $Id: testIrregEL.c 3664 2006-07-04 04:26:57Z PatrickSunter $
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
	Topology*		topology;
	ElementLayout*		eLayout;
	Element_GlobalIndex	e_I;
	Index			i;
	XML_IO_Handler*		io_handler;
	
	/* Initialise MPI, get world info */
	MPI_Init(&argc, &argv);
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size(MPI_COMM_WORLD, &procCount);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	Base_Init( &argc, &argv );
	
	DiscretisationGeometry_Init( &argc, &argv );
	DiscretisationShape_Init( &argc, &argv );
	DiscretisationMesh_Init( &argc, &argv );
	MPI_Barrier( CommWorld ); /* Ensures copyright info always come first in output */
	
	io_handler = XML_IO_Handler_New();
	dictionary = Dictionary_New();
	IO_Handler_ReadAllFromFile( io_handler, "data/trisurf.xml", dictionary );
	
	geometry = (Geometry*)IrregGeometry_New( "irregGeometry", dictionary, "geometry" );
	topology = (Topology*)IrregTopology_New( "IrregTopology", dictionary, "topology" );
	eLayout = (ElementLayout*)IrregEL_New( "IrregEL", dictionary, geometry, topology, "elements" );
	
	/*
	ElementLayout_Build( eLayout, meshDecomp );
	*/
	
	printf( "Element corner indices:\n" );
	for( e_I = 0; e_I < eLayout->elementCount; e_I++ ) {
		Index* corners = Memory_Alloc_Array( Index, eLayout->elementCornerCount, "corners" );
		
		eLayout->buildCornerIndices( eLayout, e_I, corners );
		
		printf( "\tElement %u : { %u", e_I, corners[0] );
		for( i = 1; i < eLayout->elementCornerCount; i++ )
			printf( ", %u", corners[i] );
		printf( " }\n" );
	}
	printf( "\n" );
	
	printf( "Corner element indices:\n" );
	for( i = 0; i < eLayout->cornerCount; i++ ) {
		Element_GlobalIndex	elementCnt = eLayout->cornerElementCount( eLayout, i );
		Element_GlobalIndex*	elements = Memory_Alloc_Array( Element_GlobalIndex, elementCnt, "elements" );
		
		eLayout->buildCornerElements( eLayout, i, elements );
		
		printf( "\tCorner %u : { %u", i, elements[0] );
		for( e_I = 1; e_I < elementCnt; e_I++ )
			printf( ", %u", elements[e_I] );
		printf( " }\n" );
	}
	printf( "\n" );
	
	Stg_Class_Delete( dictionary );
	Stg_Class_Delete( eLayout );
	Stg_Class_Delete( geometry );
	Stg_Class_Delete( topology );
	
	DiscretisationMesh_Finalise();
	DiscretisationShape_Finalise();
	DiscretisationGeometry_Finalise();
	
	Base_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0;
}

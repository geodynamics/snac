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
** $Id: testEdge.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>

typedef enum pointsType_t{
	Irregular,
	Regular,
	Polygon
}pointsType;

#define PI 3.1415926535897932384626

void generatePoints( CoordF *sites, int numSites, pointsType *p )
{
	int num, i, j;
	
	assert( sites );

	num = numSites;
	
	if( *p == Irregular ){
		for( i=0; i<num; i++ ){
			sites[i][0] = drand48();
			sites[i][1] = drand48();
		}
	}
	else if (*p == Regular){
		int count = 0;
		num = sqrt((float)num);
		for( i=0; i<num; i++ ){
			for( j=0; j<num; j++ ){
				sites[count][0] = ((float)i)*(1/((float)num));
				sites[count][1] = ((float)j)*(1/((float)num));

				count++;
			}
		}
	}
	else if( *p == Polygon ){
		for( i=0; i<num-1; i++ ){
			sites[i][0] = cos( ((float)i)*2.0*PI/((float)(num-1)) );
			sites[i][1] = sin( ((float)i)*2.0*PI/((float)(num-1)) );
		}
	}
}

int compareFunction( const void *a, const void *b )
{
	int *p, *q;

	p = (int*)a;
	q = (int*)b;

	if( *p > *q ){
		return 1;
	}
	else if ( *p < *q ){
		return -1;
	}
	else{
		return 0;
	}
}

#define MAX_NEIGH 100
#define TOLERANCE 0.0001
int main( int argc, char* argv[] ) {
	MPI_Comm CommWorld;
	int rank;
	int numProcessors;
	Delaunay *d = NULL;
	ParallelDelaunay *pd = NULL;
	DelaunayAttributes attr;
	CoordF *sites;
	int i = 0, j = 0, k = 0, m = 0;
	pointsType p;
	int numSites = 0;
	int dNeighbours[MAX_NEIGH];
	int pdNeighbours[MAX_NEIGH];
	int numNeighboursTest = 0;
	int voronoiSidesTest = 0;
	int voronoiAreaTest = 0;
	int triangleCountTest = 0;
	int numSitesArray[4] = { 100, 400, 900, 1600 };

	Dictionary *dictionary = NULL;
	
	attr.BuildBoundingTriangle = 0;
	attr.BuildTriangleIndices = 1;
	attr.BuildTriangleNeighbours= 1;
	attr.CreateVoronoiVertices = 1;
	attr.CalculateVoronoiSides = 1;
	attr.CalculateVoronoiSurfaceArea = 1;
	attr.FindNeighbours = 1;
	
	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size( CommWorld, &numProcessors );
	MPI_Comm_rank( CommWorld, &rank );
	
	Base_Init( &argc, &argv );
	
	DiscretisationGeometry_Init( &argc, &argv );
	MPI_Barrier( CommWorld ); /* Ensures copyright info always come first in output */

	dictionary = Dictionary_New();

	/* Testing parallel triangulation on irregular poit sets first and then
	 * on regular point sets */
	
	p = Irregular;
	
label:	numNeighboursTest = 1;
	voronoiAreaTest = 1;
	voronoiSidesTest = 1;
	triangleCountTest = 1;
	
	if (rank == 0){
		if( p == Irregular ){
			printf( "\nTesting irregular parallel triangulation..\n\n" );
		}
		else if ( p == Regular ){
			printf( "\nTesting regular parallel triangulation..\n\n" );
		}
	}
	
	for( i=0; i<4; i++ ){

		numSites = numSitesArray[i];
		
		sites = NULL;
		if( rank == 0 ){
			sites = Memory_Alloc_Array(CoordF, numSites, "TestParallelDelauney_CoordF" );
			memset( sites, 0, sizeof( CoordF ) * numSites );
		
			generatePoints( sites, numSites, &p );
		}
		
		pd = ParallelDelaunay_New( "Delaunay", dictionary, sites, numSites, rank, numProcessors, &CommWorld, &attr );
		Stg_Component_Build( pd, NULL, True );

		ParallelDelaunay_GatherTriangulation( pd );
		MPI_Barrier( CommWorld );
		
		if( rank == 0 ){

			d = Delaunay_New( "Delaunay", dictionary, sites, numSites, 0, &attr );
			Stg_Component_Build( d, NULL, True );

			for( j=0; j<numSites; j++ ){
				
				/* Checking for parity between voronoiArea generated by the serial and parallel
				 * triangulations */

				if( fabs(d->voronoiArea[j] - pd->voronoiArea[j]) > TOLERANCE ){
					printf( "voronoi area %f of node %d not within tolerance\n", pd->voronoiArea[j], j );
					voronoiAreaTest = 0;
				}
				
				assert( pd->numNeighbours[j] == d->numNeighbours[j] );
			
				memset( dNeighbours, 0, sizeof( dNeighbours ) );
				memset( pdNeighbours, 0, sizeof( pdNeighbours ) );

				memcpy( dNeighbours, d->neighbours[j], sizeof( int ) * d->numNeighbours[j] );
				memcpy( pdNeighbours, pd->neighbours[j], sizeof( int ) * pd->numNeighbours[j] );
				
				/* The order of the neighbour nodes of each node in a serial triangulation is
				 * different to that of in a parallel triangulation. Hence we need to sort the
				 * neighbour nodes to check for parity */
				
				qsort( dNeighbours, d->numNeighbours[j], sizeof( int ), compareFunction );
				qsort( pdNeighbours, pd->numNeighbours[j], sizeof( int ), compareFunction );
	
				
				/* Checking for parity between the length of the voronoi sides generated by 
				 * the serial and parallel triangulations */

				for( k=0; k<d->numNeighbours[j]; k++ ){
					assert( dNeighbours[k] == pdNeighbours[k] );

					for( m=0; m<d->numNeighbours[j]; m++ ){
						if( d->neighbours[j][k] == pd->neighbours[j][m] ){
							if( fabs(d->voronoiSides[j][k] - pd->voronoiSides[j][m]) > TOLERANCE ){
								printf( "voronoi side %f of node %d not within tolerance\n", pd->voronoiSides[j][m], j );
								voronoiSidesTest = 0;
							}
						}
					}
				}
			}
			
			if( d->numTriangles != pd->numTriangles ){
				triangleCountTest = 0;
			}
			
			Stg_Class_Delete( d );
		}

		Stg_Class_Delete( pd );
		
		if( rank == 0 ){
			Memory_Free( sites );
		}
		
		if (rank == 0){

			printf( "\n\tRunning Parallel Triangulator on %d nodes\n\n", numSites );
	
			if( voronoiAreaTest ){
				printf( "\t\tParallel Voronoi Area test: passed\n" );
			}
	
			if( voronoiSidesTest ){
				printf( "\t\tParallel Voronoi Sides test: passed\n" );
			}
		
			if( triangleCountTest ){
				printf( "\t\tParallel Triangle count test: passed\n" );
			}
		}
	}
	
	if(p == Irregular){
		p = Regular;
		goto label;
	}
	
	Stg_Class_Delete( dictionary );
	
	DiscretisationGeometry_Finalise();
	
	Base_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	return 0;
}

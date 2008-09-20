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

int main( int argc, char* argv[] ) {
	MPI_Comm CommWorld;
	int rank;
	int numProcessors;
	int procToWatch;
	
	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size( CommWorld, &numProcessors );
	MPI_Comm_rank( CommWorld, &rank );
	
	Base_Init( &argc, &argv );
	
	DiscretisationGeometry_Init( &argc, &argv );
	MPI_Barrier( CommWorld ); /* Ensures copyright info always come first in output */
	
	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}
	if( rank == procToWatch ) {
		Delaunay *d = NULL;
		DelaunayAttributes attr;
		CoordF *sites;
		int i = 0, j = 0;
		pointsType p;
		Dictionary *dictionary = Dictionary_New();
		
		attr.BuildBoundingTriangle = 0;
		attr.BuildTriangleIndices = 1;
		attr.BuildTriangleNeighbours = 1;
		attr.CreateVoronoiVertices = 1;
		attr.CalculateVoronoiSides = 1;
		attr.CalculateVoronoiSurfaceArea = 1;
		attr.FindNeighbours = 1;

		/* Testing regular point-set */
		printf( "Testing triangulation of a regular mesh:\n" );
		{
			int pass = 1, k = 0;
			float sqroot = 0, area = 0;
			
			int array[10] = { 16, 25, 36, 49, 64, 81, 100, 121, 144, 269 };
			
			#define epsilon 0.001
			
			for( j=0; j<10; j++ ){
			
				i = array[j];
				
				p = Regular;

				sites = Memory_Alloc_Array(  CoordF, i, "TestDelauney_CoordF_pointSet" );
				memset( sites, 0, sizeof( CoordF ) * i );
		
				generatePoints( sites, i, &p );
			
				d = Delaunay_New( "Delaunay", dictionary, sites, i, 0, &attr );
		
				Stg_Component_Build( d, NULL, True );

				sqroot = sqrt( (float)i );

				area = (1.0f/sqroot) * (1.0f/sqroot);
				
				for( k=0; k<i; k++ ){
					if( d->hull[k] ) continue;
					
					if( (k > 0) && (k< sqrt((float)k)) )
						if( FABS( area - d->voronoiArea[k] ) > epsilon ) pass = 0;
				}

				Stg_Class_Delete( d );

				Memory_Free( sites );
			}

			printf( "Regular mesh triangulation test %s\n\n", pass?"passed..":"failed..!" );
			
		}

		
		/* Testing irregular point-set */
		printf( "Testing triangulation of irregular point-sets:\n" );
		{
			int pass = 1;
			
			for( i=10; i<210; i++ ){
			
				p = Irregular;
				
				sites = Memory_Alloc_Array( CoordF, i, "TestDelauney_CoordF_irregPointSet" );
				memset( sites, 0, sizeof( CoordF ) * i );
		
				generatePoints( sites, i, &p );
			
				d = Delaunay_New( "Delaunay", dictionary, sites, i, 0, &attr );
		
				Stg_Component_Build( d, NULL, True );

				if( d->numFaces !=  (d->numEdges - d->numSites+2) ){
					pass = 0;
				}

				Stg_Class_Delete( d );

				Memory_Free( sites );
			}

			printf( "Irregular point-set triangulation test %s\n\n", pass?"passed..":"failed..!" );
			
		}
		
		
		/* Testing regular polygons with a single node at the centre. */
		printf( "Testing triangulation of regular polygons:\n" );
		{
			float theta = 0.0, area = 0.0, a = 0.0;
			float side = 0.0, voronoiArea = 0.0;
			#define epsilon 0.001
			int pass = 1;
			
			for( i=10; i<210; i++ ){
				
				p = Polygon;
			
				theta = 2*PI/((float)(i-1));
				
				sites = Memory_Alloc_Array(CoordF, i, "TestDelauney_CoordF_regPolygon" );
				memset( sites, 0, sizeof( CoordF ) * i );
		
				generatePoints( sites, i, &p );
			
				d = Delaunay_New( "Delaunay", dictionary, sites, i, 0, &attr );
		
				Stg_Component_Build( d, NULL, True );
				
				for( j=0; j<i; j++ ){
					if( (sites[j][0] == 0.0) && (sites[j][1] == 0.0) ){
						side = d->voronoiSides[j][0];
						voronoiArea = d->voronoiArea[j];
						break;
					}
				}
				
				a = ( (side/2.0f)/sin(theta/2.0f) );
				area = (0.5 * a * a * sin( theta )) * ((float)(i-1));

				if( FABS(area - voronoiArea) > epsilon ){
					pass = 0;
				}
			
				Stg_Class_Delete( d );

				Memory_Free( sites );
			}

			printf( "Regular polygon triangulation test %s\n", pass?"passed..":"failed..!" );
		}
		Stg_Class_Delete( dictionary );
	}
	
	DiscretisationGeometry_Finalise();
	
	Base_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0;
}

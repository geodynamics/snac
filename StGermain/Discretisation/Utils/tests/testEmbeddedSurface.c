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
** $Id: testEmbeddedSurface.c 3995 2007-02-07 02:20:14Z PatrickSunter $
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


struct _Node {
	struct { __Advectable };
	double temp;
};


struct _Element {
	struct { __Advectable };
	double temp;
};


int main(int argc, char **argv)
{
	MPI_Comm		CommWorld;
	XML_IO_Handler 		*io_handler = XML_IO_Handler_New();
	int			rank, procCount, procToWatch;
	Dictionary		*dictionary;
	EmbeddedSurface		*surface;
	MeshTopology		*rmt, *imt;
	MeshGeometry		*rmg, *img;
	MeshDecomp		*rmd, *imd;
	MeshLayout		*rml, *isl;
	ExtensionManager_Register	*extensionMgr_Register;
	Mesh			*mesh;
	Element_GlobalIndex	intersectCnt, *intersect;
	Index			i;
	
	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size( CommWorld, &procCount );
	MPI_Comm_rank( CommWorld, &rank );
	
	Base_Init( &argc, &argv );
	
	DiscretisationGeometry_Init( &argc, &argv );
	DiscretisationShape_Init( &argc, &argv );
	DiscretisationMesh_Init( &argc, &argv );
	DiscretisationUtils_Init( &argc, &argv );
	MPI_Barrier( CommWorld ); /* Ensures copyright info always come first in output */
	
	if( argc >= 2 )
		procToWatch = atoi( argv[1] );
	else
		procToWatch = 0;
	if( rank == procToWatch ) printf( "Watching rank: %i\n", rank );
	
	dictionary = Dictionary_New();
	Dictionary_Add(dictionary, "outputPath", Dictionary_Entry_Value_FromString("./output"));
	Dictionary_Add( dictionary, "rank", Dictionary_Entry_Value_FromUnsignedInt( rank ) );
	Dictionary_Add( dictionary, "numProcessors", Dictionary_Entry_Value_FromUnsignedInt( procCount ) );
	Dictionary_Add( dictionary, "meshSizeI", Dictionary_Entry_Value_FromUnsignedInt( 2 ) );
	Dictionary_Add( dictionary, "meshSizeJ", Dictionary_Entry_Value_FromUnsignedInt( 2 ) );
	Dictionary_Add( dictionary, "meshSizeK", Dictionary_Entry_Value_FromUnsignedInt( 4 ) );
	Dictionary_Add( dictionary, "allowUnusedCPUs", Dictionary_Entry_Value_FromBool( True ) );
	Dictionary_Add( dictionary, "allowPartitionOnElement", Dictionary_Entry_Value_FromBool( True ) );
	Dictionary_Add( dictionary, "allowPartitionOnNode", Dictionary_Entry_Value_FromBool( True ) );
	Dictionary_Add( dictionary, "shadowDepth", Dictionary_Entry_Value_FromUnsignedInt( 0 ) );
	IO_Handler_ReadAllFromFile(io_handler, "data/surface.xml", dictionary);
	
	rmt = (MeshTopology *)HexaMeshTopology_New(dictionary);
	rmg = (MeshGeometry *)HexaMeshGeometry_New(dictionary);
	rmd = (MeshDecomp *)HexaMeshDecomp_New(dictionary, MPI_COMM_WORLD, (HexaMeshTopology *)rmt);
	rml = MeshLayout_New(dictionary, rmt, rmg, rmd);
	
	imt = (MeshTopology *)TriSurfTopology_New(dictionary, "imElements");
	img = (MeshGeometry *)TriSurfGeometry_New(dictionary, "imNodes");
	imd = (MeshDecomp *)IrregularMeshDecomp_New1(dictionary, MPI_COMM_WORLD, imt, img, rml);
	isl = MeshLayout_New(dictionary, imt, img, imd);
	
	extensionMgr_Register = ExtensionManager_Register_New( );
	mesh = Mesh_New( isl, sizeof(Node), sizeof(Element), extensionMgr_Register, dictionary );
	Mesh_Build( mesh );
	Mesh_Initialise(mesh);
	
	surface = EmbeddedSurface_New(mesh);
	
	if (procToWatch == rank)
	{
		intersect = Memory_Alloc_Array( Element_GlobalIndex, isl->decomp->elementGlobalCount(isl->decomp), "intersect" );
		intersectCnt = EmbeddedSurface_BuildIntersection(surface, intersect);
	
		printf("Intersects: %u\n", intersectCnt);
		for (i = 0; i < intersectCnt; i++)
			printf("\tinstersect[%u]: %u\n", i, intersect[i]);
		printf("\n");
	
		if (intersect) Memory_Free(intersect);
	
		for (i = 0; i < isl->decomp->nodeGlobalCount(isl->decomp); i++)
		{
			Coord point;
		
			point[0] = ((TriSurfGeometry *)img)->node[i][0] + 1.0;
			point[1] = ((TriSurfGeometry *)img)->node[i][1];
			point[2] = ((TriSurfGeometry *)img)->node[i][2];

			printf("Distance to point {%.3f, %.3f, %.3f}: ", point[0], point[1], point[2]);
			printf("%.3f\n", EmbeddedSurface_DistanceToPoint(surface, point));
		}
	}
	
	Stg_Class_Delete(surface);
	Stg_Class_Delete(isl);
	Stg_Class_Delete(imd);
	Stg_Class_Delete(img);
	Stg_Class_Delete(imt);
	Stg_Class_Delete(rml);
	Stg_Class_Delete(rmd);
	Stg_Class_Delete(rmg);
	Stg_Class_Delete(rmt);
	
	DiscretisationUtils_Finalise();
	DiscretisationMesh_Finalise();
	DiscretisationShape_Finalise();
	DiscretisationGeometry_Finalise();
	
	Base_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0;
}

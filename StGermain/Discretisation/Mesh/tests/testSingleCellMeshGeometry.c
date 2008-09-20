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
** $Id: testSingleCellMeshGeometry.c 3664 2006-07-04 04:26:57Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"
#include "Discretisation/Mesh/Mesh.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>


int main( int argc, char *argv[] ) {
	int		rank;
	int		procCount;
	int		procToWatch;
	
	Dictionary	*dictionary;
	MeshTopology	*pmt;
	MeshGeometry	*mg, *pmg;
	MeshDecomp	*pmd;
	MeshLayout	*pml;
	Coord		coord;
	Node_Index	sizeI, sizeJ, sizeK;
	Index		i;
	
	/* Initialise MPI, get world info */
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &procCount);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	Base_Init( &argc, &argv );
	
	DiscretisationGeometry_Init( &argc, &argv );
	DiscretisationShape_Init( &argc, &argv );
	DiscretisationMesh_Init( &argc, &argv );
	MPI_Barrier( CommWorld ); /* Ensures copyright info always come first in output */

	procToWatch = argc >= 2 ? atoi(argv[1]) : 0;
	
	/* Read input */
	dictionary = Dictionary_New();
	Dictionary_Add( dictionary, "meshSizeI", Dictionary_Entry_Value_FromUnsignedInt( 1 ) );
	Dictionary_Add( dictionary, "meshSizeJ", Dictionary_Entry_Value_FromUnsignedInt( 1 ) );
	Dictionary_Add( dictionary, "meshSizeK", Dictionary_Entry_Value_FromUnsignedInt( 1 ) );
	
	for (sizeK = 1; sizeK < 4; sizeK++) {
		for (sizeJ = 1; sizeJ < 4; sizeJ++) {
			for (sizeI = 1; sizeI < 4; sizeI++) {
				Dictionary_Set( dictionary, "meshSizeI", Dictionary_Entry_Value_FromUnsignedInt( sizeI ) );
				Dictionary_Set( dictionary, "meshSizeJ", Dictionary_Entry_Value_FromUnsignedInt( sizeJ ) );
				Dictionary_Set( dictionary, "meshSizeK", Dictionary_Entry_Value_FromUnsignedInt( sizeK ) );
				
				pmt = (MeshTopology*)HexaMeshTopology_New(dictionary);
				pmg = (MeshGeometry*)HexaMeshGeometry_New(dictionary);
				pmd = (MeshDecomp*)RegularMeshDecomp_New(dictionary, MPI_COMM_WORLD, pmt);
				pml = (MeshLayout*)MeshLayout_New(pmt, pmg, pmd);
				
				mg = (MeshGeometry *)SingleCellMeshGeometry_New(pml);
				
				Print(mg);
	
				for (i = 0; i < mg->nodeCount; i++)
				{
					printf("Node %u : ", i);
					mg->nodeAt(mg, i, coord);
					printf("{%lf, %lf, %lf}\n", coord[0], coord[1], coord[2]);
				}
				printf("\n");
				
				Stg_Class_Delete(mg);
				Stg_Class_Delete(pml);
				Stg_Class_Delete(pmd);
				Stg_Class_Delete(pmg);
				Stg_Class_Delete(pmt);
			}
		}
	}
	
	/* Destroy stuff */
	Stg_Class_Delete(dictionary);
	
	DiscretisationMesh_Finalise();
	DiscretisationShape_Finalise();
	DiscretisationGeometry_Finalise();
	
	Base_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();

	return 0; /* success */
}

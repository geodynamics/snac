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
** $Id: testDofLayout-remap.c 2421 2004-12-14 02:09:26Z AlanLo $
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


int main( int argc, char *argv[] ) {
	MPI_Comm		CommWorld;
	int		rank;
	int		procCount;
	int		procToWatch;
	
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
	
	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}
	if( rank == procToWatch ) {
		DofLayout*		dof;
		double			dummyVar;
		double*			dummyPtr = &dummyVar;
		Variable_Register*	variableRegister;
		Variable*		var[6];
		char*			varName[] = {"x", "y", "z", "vx", "vy", "vz"};
		Index			i, j;
		Index			arraySize = 1;
		IndexMap*		map;

		/* Create variable register */
		variableRegister = Variable_Register_New();

		/* Create variables */
		for (i = 0; i < 6; i++) {
			var[i] = Variable_NewScalar( varName[i], Variable_DataType_Double, &arraySize, (void**)&dummyPtr, 0 );
			Variable_Register_Add(variableRegister, var[i]);
		}

		/* Simple test */
		dof = DofLayout_New( "dofLayout", variableRegister, 27 );
		for (i = 0; i < 6; i++) {
			for (j = 0; j < 27; j++) {
				DofLayout_AddDof_ByVarName(dof, varName[i], j);
			}
		}
		
		/* Build the IndexMap */
		map = IndexMap_New();
		for( i = 0; i < 27; i++ ) {
			if( i % 2 == 0 ) {
				IndexMap_Append( map, i, i / 2 );
			}
			else {
				IndexMap_Append( map, i, 27 );
			}
		}
		
		/* Perform remap */
		DofLayout_Remap( dof, 14, map );
		Stg_Class_Delete( map );

		Build(dof, 0, False);

		printf("Simple test:\n");
		for (i = 0; i < 14; i++)
			printf("\t%u\n", dof->dofCounts[i]);

		Stg_Class_Delete(dof);

		Stg_Class_Delete(variableRegister);
		for (i = 0; i < 6; i++)
			if (var[i]) Stg_Class_Delete(var[i]);
	}
	
	DiscretisationUtils_Finalise();
	DiscretisationMesh_Finalise();
	DiscretisationShape_Finalise();
	DiscretisationGeometry_Finalise();
	
	Base_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0; /* success */
}

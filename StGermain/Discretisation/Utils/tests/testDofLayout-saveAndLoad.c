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
** $Id: testDofLayout-saveAndLoad.c 3555 2006-05-10 07:05:46Z PatrickSunter $
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
	MPI_Comm_size( MPI_COMM_WORLD, &procCount );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	
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
		Variable_Register*	variableRegister;
		Variable*		var[6];
		char*			varName[] = {"x", "y", "z", "vx", "vy", "vz"};
		Index			ii, var_I;
		Index			arraySize = 27;
		double*			varArrays[6];

		/* Create variable register */
		variableRegister = Variable_Register_New();

		/* Create variables */
		for (var_I = 0; var_I < 6; var_I++) {
			varArrays[var_I] = Memory_Alloc_Array_Unnamed( double, arraySize );
			var[var_I] = Variable_NewScalar( varName[var_I], Variable_DataType_Double, &arraySize, 
				(void**)&(varArrays[var_I]), variableRegister );
			Build( var[var_I], 0, False );	
			Initialise( var[var_I], 0, False );	
		}
			
		for (ii = 0; ii < arraySize; ii++) {
			for (var_I = 0; var_I < 6; var_I++) {
				Variable_SetValueDouble( var[var_I], ii, (ii*10 + var_I) );
			}
		}	

		/* Simple test */
		dof = DofLayout_New( "dofLayout", variableRegister, arraySize );
		for (ii = 0; ii < arraySize; ii++) {
			for (var_I = 0; var_I < 6; var_I++) {
				DofLayout_AddDof_ByVarName(dof, varName[var_I], ii);
			}
		}	
		Build(dof, 0, False);

		DofLayout_SaveAllVariablesToFiles( dof, "output/testDofSave", rank );

		DofLayout_SetAllToZero( dof );

		DofLayout_LoadAllVariablesFromFiles( dof, "output/testDofSave", rank );

		for (ii = 0; ii < arraySize; ii++) {
			printf("\tIndex %d - %2g,%2g,%2g,%2g,%2g,%2g\n", ii,
				DofLayout_GetValueDouble( dof, ii, 0 ),
				DofLayout_GetValueDouble( dof, ii, 1 ),
				DofLayout_GetValueDouble( dof, ii, 2 ),
				DofLayout_GetValueDouble( dof, ii, 3 ),
				DofLayout_GetValueDouble( dof, ii, 4 ),
				DofLayout_GetValueDouble( dof, ii, 5 ) );
		}	


		/* Cleanup */
		Stg_Class_Delete(dof);

		Stg_Class_Delete(variableRegister);
		for (var_I = 0; var_I < 6; var_I++) {
			if (var[var_I]) Stg_Class_Delete(var[var_I]);
			Memory_Free( varArrays[var_I] );
		}
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

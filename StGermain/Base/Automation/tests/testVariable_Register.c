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
** $Id: testVariable_Register.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"
#include "Base/Automation/Automation.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE	4
#define STRUCT_SIZE	4


int main( int argc, char *argv[] ) {
	int		rank;
	int		procCount;
	int		procToWatch;
	Stream*		stream;
	
	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_size( MPI_COMM_WORLD, &procCount );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	
	BaseFoundation_Init( &argc, &argv );
	BaseIO_Init( &argc, &argv );
	BaseContainer_Init( &argc, &argv );
	BaseAutomation_Init( &argc, &argv );

	stream = Journal_Register (Info_Type, "myStream");	

	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}
	if( rank == procToWatch ) {
		Variable_Register*	reg;
		Variable*		var[10];
		double			array[ARRAY_SIZE];
		Index			arraySize = ARRAY_SIZE;
		char*			name[10] = {"testVar0", "testVar1", "testVar2", "testVar3",
						"testVar4", "testVar5", "testVar6", "testVar7",
						"testVar8", "testVar9"};
		Index			i;

		for (i = 0; i < 10; i++) {
			var[i] = Variable_NewVector( name[i], Variable_DataType_Double, 4, &arraySize, (void**)&array, 0 );
		}

		reg = Variable_Register_New();

		printf("Add test:\n");
		for (i = 0; i < 10; i++)
		{
			Variable_Register_Add(reg, var[i]);
			/*Print(reg, stream);*/
		}

		printf("\nGetIndex test:\n");
		for (i = 0; i < 10; i++)
			printf("\tname: %s - index: %u\n", name[i], Variable_Register_GetIndex(reg, name[i]));

		Stg_Class_Delete(reg);
		for (i = 0; i < 10; i++)	
			Stg_Class_Delete(var[i]);
	}
	
	BaseAutomation_Finalise();
	BaseContainer_Finalise();
	BaseIO_Finalise();
	BaseFoundation_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0; /* success */
}

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
** $Id: testSetVC.c 3462 2006-02-19 06:53:24Z WalterLandry $
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


void quadratic(Index index, Variable_Index var_I, void* context, void* result)
{
	*(double *)result = 20.0;
}


int main(int argc, char *argv[])
{
	int		rank;
	int		procCount;
	int		procToWatch;
	Stream*		stream;
	
	Dictionary*		dictionary;
	XML_IO_Handler*		io_handler;
	
	Variable*			var[7];
	Variable_Register*		variable_Register;
	VariableCondition*		vc;
	ConditionFunction*		quadCF;
	ConditionFunction_Register*	conFunc_Register;
	
	double*		array[7];
	Index		arraySize = 4*4*4;
	char*		vcKey = "SetVC";
	char*		varName[] = {"x", "y", "z", "vx", "vy", "vz", "temp"};
	
	Index	i, j, k;
	
	/* Initialise MPI, get world info */
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &procCount);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	BaseFoundation_Init( &argc, &argv );
	BaseIO_Init( &argc, &argv );
	BaseContainer_Init( &argc, &argv );
	BaseAutomation_Init( &argc, &argv );

	stream = Journal_Register (Info_Type, "myStream");

	io_handler = XML_IO_Handler_New();

	procToWatch = argc >= 2 ? atoi(argv[1]) : 0;
	
	dictionary = Dictionary_New();
	IO_Handler_ReadAllFromFile(io_handler, "data/setVC.xml", dictionary);
	fflush(stdout);
	MPI_Barrier(MPI_COMM_WORLD);
	Dictionary_Add(dictionary, "rank", Dictionary_Entry_Value_FromUnsignedInt(rank));
	Dictionary_Add(dictionary, "numProcessors", Dictionary_Entry_Value_FromUnsignedInt(procCount));
	Dictionary_Add(dictionary, "meshSizeI", Dictionary_Entry_Value_FromUnsignedInt(4));
	Dictionary_Add(dictionary, "meshSizeJ", Dictionary_Entry_Value_FromUnsignedInt(4));
	Dictionary_Add(dictionary, "meshSizeK", Dictionary_Entry_Value_FromUnsignedInt(4));
	
	/* Create CF stuff */
	quadCF = ConditionFunction_New(quadratic, "quadratic");
	conFunc_Register = ConditionFunction_Register_New();
	ConditionFunction_Register_Add(conFunc_Register, quadCF);
	
	/* Create variable register */
	variable_Register = Variable_Register_New();
	
	/* Create variables */
	for (i = 0; i < 6; i++) {
		array[i] = Memory_Alloc_Array( double, arraySize, "array[]" );
		var[i] = Variable_NewScalar( varName[i], Variable_DataType_Double, &arraySize, (void**)&array[i], 0 ); 
		Variable_Register_Add(variable_Register, var[i]);
	}
	array[6] = Memory_Alloc_Array( double , arraySize*5, "array[6]" );
	var[6] = Variable_NewVector( varName[6], Variable_DataType_Double, 5, &arraySize, (void**)&array[6], 0,
		"a", "b", "c", "d", "e" );
	Variable_Register_Add(variable_Register, var[6]);
	Variable_Register_BuildAll(variable_Register);
	
	/* Create AllVC */
	vc = (VariableCondition*)SetVC_New( "SetVC", vcKey, variable_Register, conFunc_Register, dictionary );
	Build( vc, 0, False );
	
	for (j = 0; j < 6; j++)
		memset(array[j], 0, sizeof(double)*4*4*4);
	memset(array[6], 0, sizeof(double)*4*4*4*5);
	VariableCondition_Apply(vc, NULL);
	
	if (rank == procToWatch)
	{
		printf("Testing for %s\n", vcKey);
		Print(vc, stream);
		printf("\n");
		for (j = 0; j < 6; j++)
		{
			printf("\nvar[%u]: %.2f", j, array[j][0]);
			for (k = 1; k < 4*4*4; k++)
				printf(", %.2f", array[j][k]);
		}
		printf("\nvar[6]: %.2f", array[6][0]);
		for (j = 1; j < 4*4*4*5; j++)
			printf(", %.2f", array[6][j]);
		printf("\n\n");
			
		for (j = 0; j < 7; j++)
		{
			for (k = 0; k < 4*4*4; k++)
				printf("%s ", VariableCondition_IsCondition(vc, k, j) ? "True " : "False");
			printf("\n");
		}
		printf("\n");
			
		for (j = 0; j < 7; j++)
		{
			for (k = 0; k < 4*4*4; k++)
			{
				VariableCondition_ValueIndex	valIndex;
			
				valIndex = VariableCondition_GetValueIndex(vc, k, j);
				if (valIndex != (unsigned)-1)
					printf("%03u ", valIndex);
				else
					printf("XXX ");
			}
			printf("\n");
		}
		printf("\n");
	}
	
	Stg_Class_Delete(vc);
		
	Stg_Class_Delete(variable_Register);
	for (i = 0; i < 7; i++)
	{
		Stg_Class_Delete(var[i]);
		if (array[i]) Memory_Free(array[i]);
	}
	Stg_Class_Delete(conFunc_Register);
	Stg_Class_Delete(quadCF);
	Stg_Class_Delete(dictionary);
	
	BaseAutomation_Finalise();
	BaseContainer_Finalise();
	BaseIO_Finalise();
	BaseFoundation_Finalise();

	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0; /* success */
}

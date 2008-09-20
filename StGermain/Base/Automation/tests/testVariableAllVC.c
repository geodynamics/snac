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
** $Id: testSetVC.c 2421 2004-12-14 02:09:26Z AlanLo $
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

int main(int argc, char *argv[])
{
	int		rank;
	int		procCount;
	Stream*		stream;
	
	Dictionary*		dictionary;

	Dictionary_Entry_Value* info;
	Dictionary_Entry_Value* varList;
	Dictionary_Entry_Value* varValue;
	
	Variable_Register*		variable_Register;
	ConditionFunction_Register*	conFunc_Register;

	VariableAllVC* vc;

	double*		data;
	Index		length = 3;

	/* Initialise MPI, get world info */
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &procCount);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	BaseFoundation_Init( &argc, &argv );
	BaseIO_Init( &argc, &argv );
	BaseContainer_Init( &argc, &argv );
	BaseAutomation_Init( &argc, &argv );

	data = Memory_Alloc_Array( double, length, "test" );

	stream = Journal_Register (Info_Type, "myStream");

	dictionary = Dictionary_New();

	info = Dictionary_Entry_Value_NewStruct();
	varList = Dictionary_Entry_Value_NewList();
	varValue = Dictionary_Entry_Value_NewStruct();

	Dictionary_Entry_Value_AddMember( varValue, "name", Dictionary_Entry_Value_FromString( "test" ) );
	Dictionary_Entry_Value_AddMember( varValue, "type", Dictionary_Entry_Value_FromString( "double" ) );
	Dictionary_Entry_Value_AddMember( varValue, "value", Dictionary_Entry_Value_FromDouble( 2.0 ) );

	Dictionary_Entry_Value_AddElement( varList, varValue );

	Dictionary_Entry_Value_AddMember( info, "variables", varList );

	Dictionary_Add( dictionary, "VariableAllVC", info );
	
	
	conFunc_Register = ConditionFunction_Register_New();
	
	variable_Register = Variable_Register_New();
	
	Variable_NewScalar(
		"test",
		Variable_DataType_Double,
		&length,
		(void*)&data,
		variable_Register );
		
	Variable_Register_BuildAll(variable_Register);
	
	vc = VariableAllVC_New( "variableAllVC", "VariableAllVC", variable_Register, conFunc_Register, dictionary, NULL );
	
	Build( vc, 0, False );
	
	VariableCondition_Apply(vc, NULL);
	
	if (rank == 0) {
		int i;

		for (i = 0; i < length; ++i )  {
			Journal_Printf( stream, "%lf\n", data[i] );
		}
	}

	Memory_Free( data );

	Stg_Class_Delete(vc);
		
	Stg_Class_Delete(variable_Register);
	Stg_Class_Delete(conFunc_Register);
	Stg_Class_Delete(dictionary);
	
	BaseAutomation_Finalise();
	BaseContainer_Finalise();
	BaseIO_Finalise();
	BaseFoundation_Finalise();

	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0; /* success */
}

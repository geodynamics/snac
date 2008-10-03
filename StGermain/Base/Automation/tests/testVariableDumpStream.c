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
** $Id: ClassName.c 786 2004-02-10 12:31:53Z LukeHodkinson $
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
#include <math.h>


int main( int argc, char *argv[] ) {
	int		rank;
	int		procCount;
	int		procToWatch;
	Stream*		stream;
	Stream*		dumpStream;

	
	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_size( MPI_COMM_WORLD, &procCount );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	
	BaseFoundation_Init( &argc, &argv );
	BaseIO_Init( &argc, &argv );
	BaseContainer_Init( &argc, &argv );
	BaseAutomation_Init( &argc, &argv );

	stream = Journal_Register( Info_Type, "testVariable" );
	
	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}
	if( rank == procToWatch ) {
		typedef double Triple[3];

		double* array;
		Triple* structArray;
		Index length = 10;
		
		Variable* var;
		Variable* vec;

		Variable_Register* reg;

		int i;

		array = Memory_Alloc_Array( double, length, "test" );
		structArray = Memory_Alloc_Array( Triple, length, "test" );

		reg = Variable_Register_New();

		var = Variable_NewScalar(
			"Scalar",
			Variable_DataType_Double,
			&length,
			(void**)&array,
			reg );

		vec = Variable_NewVector(
			"Three",
			Variable_DataType_Double,
			3,
			&length,
			(void**)&structArray,
			reg,
			"a",
			"b",
			"c" );

		Variable_Register_BuildAll( reg );

		for ( i = 0; i < length; ++i ) {
			Variable_SetValueDouble( var, i, 123.456 );

			Variable_SetValueAtDouble( vec, i, 0, 1.2 );
			Variable_SetValueAtDouble( vec, i, 1, 3.4 );
			Variable_SetValueAtDouble( vec, i, 2, 5.6 );
		}

		dumpStream = Journal_Register( VariableDumpStream_Type, "scalar dump" );
		VariableDumpStream_SetVariable( dumpStream, var, 1, 0, "data/scalardump.dat" );

		Journal_Printf( stream, "Dumping scalar\n" );
		Journal_Dump( dumpStream, NULL );

		dumpStream = Journal_Register( VariableDumpStream_Type, "vector dump" );
		VariableDumpStream_SetVariable( dumpStream, vec, 1, 0, "data/vectordump.dat" );

		Journal_Printf( stream, "Dumping vector\n" );
		Journal_Dump( dumpStream, NULL );
	}
	
	BaseAutomation_Finalise();
	BaseContainer_Finalise();
	BaseIO_Finalise();
	BaseFoundation_Finalise();

	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0; /* success */
}




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
** Role:
** 	Tests the Double macros of Variable. Will test a single scalar, vector and each scalar created from the vector.
**
** $Id: testVariable-Double.c 2136 2004-09-30 02:47:13Z PatrickSunter $
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

		double testValues[] = { 123456789.0, 0.987654321 };
		Index testValueCount = 2;
		Index test_I;
		double testValue;

		Variable* var;
		Variable* vec;
		Variable* vecVar[3];

		Variable_Register* reg;

		int i, j;

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
		vecVar[0] = Variable_Register_GetByName( reg, "a" );
		vecVar[1] = Variable_Register_GetByName( reg, "b" );
		vecVar[2] = Variable_Register_GetByName( reg, "c" );

		Variable_Register_BuildAll( reg );

		Print( reg, stream );

		for ( test_I = 0; test_I < testValueCount; ++test_I ) {	

			testValue = testValues[test_I];

			Journal_Printf( stream, "Setting value %lf\n", (double)(double)testValue );

			for ( i = 0; i < length; ++i ) {
				Variable_SetValueDouble( var, i, testValue );

				Variable_SetValueAtDouble( vec, i, 0, testValue );
				Variable_SetValueAtDouble( vec, i, 1, testValue );
				Variable_SetValueAtDouble( vec, i, 2, testValue );
			}

			Journal_Printf( stream, "~~~Scalar~~~\n" );
			for ( i = 0; i < length; ++i ) {
				Journal_Printf( stream, "i=%d: %d %d %d %d %d \n",
						i,
						Variable_GetValueDouble( var, i ) == (double)(double)testValue,
						Variable_GetValueDoubleAsChar( var, i ) == (char)(double)testValue,
						Variable_GetValueDoubleAsShort( var, i ) == (short)(double)testValue,
						Variable_GetValueDoubleAsInt( var, i ) == (int)(double)testValue,
						Variable_GetValueDoubleAsFloat( var, i ) == (float)(double)testValue );
			}

			Journal_Printf( stream, "~~~Vector~~~\n" );
			for ( i = 0; i < length; ++i ) {
				Journal_Printf( stream, "a, i=%d: %d %d %d %d %d ",
						i,
						Variable_GetValueAtDouble( vec, i, 0 ) == (double)(double)testValue,
						Variable_GetValueAtDoubleAsChar( vec, i, 0 ) == (char)(double)testValue,
						Variable_GetValueAtDoubleAsShort( vec, i, 0 ) == (short)(double)testValue,
						Variable_GetValueAtDoubleAsInt( vec, i, 0 ) == (int)(double)testValue,
						Variable_GetValueAtDoubleAsFloat( vec, i, 0 ) == (float)(double)testValue );
				Journal_Printf( stream, "ptr test %s\n",
						( Variable_GetPtrAtDouble( vec, i, 0 ) == &structArray[i][0] ) ? "OK" : "FAILED" );

				Journal_Printf( stream, "b, i=%d: %d %d %d %d %d ",
						i,
						Variable_GetValueAtDouble( vec, i, 1 ) == (double)(double)testValue,
						Variable_GetValueAtDoubleAsChar( vec, i, 1 ) == (char)(double)testValue,
						Variable_GetValueAtDoubleAsShort( vec, i, 1 ) == (short)(double)testValue,
						Variable_GetValueAtDoubleAsInt( vec, i, 1 ) == (int)(double)testValue,
						Variable_GetValueAtDoubleAsFloat( vec, i, 1 ) == (float)(double)testValue );
				Journal_Printf( stream, "ptr test %s\n",
						( Variable_GetPtrAtDouble( vec, i, 1 ) == &structArray[i][1] ) ? "OK" : "FAILED" );

				Journal_Printf( stream, "c, i=%d: %d %d %d %d %d ",
						i,
						Variable_GetValueAtDouble( vec, i, 2 ) == (double)(double)testValue,
						Variable_GetValueAtDoubleAsChar( vec, i, 2 ) == (char)(double)testValue,
						Variable_GetValueAtDoubleAsShort( vec, i, 2 ) == (short)(double)testValue,
						Variable_GetValueAtDoubleAsInt( vec, i, 2 ) == (int)(double)testValue,
						Variable_GetValueAtDoubleAsFloat( vec, i, 2 ) == (float)(double)testValue );
				Journal_Printf( stream, "ptr test %s\n",
						( Variable_GetPtrAtDouble( vec, i, 2 ) == &structArray[i][2] ) ? "OK" : "FAILED" );

			}

			Journal_Printf( stream, "~~~Vector: Sub-Variable~~~\n" );
			for ( i = 0; i < length; ++i ) {
				for ( j = 0; j < 3; ++j ) {
					Journal_Printf( stream, "j=%d, i=%d: %s\n",
						j,
						i,
						( _Variable_GetStructPtr( vecVar[j], i ) == &structArray[i][j] ) ? "OK" : "FAIL" );
				}
			}

			
		}
	
	}
	
	BaseAutomation_Finalise();
	BaseContainer_Finalise();
	BaseIO_Finalise();
	BaseFoundation_Finalise();

	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0; /* success */
}

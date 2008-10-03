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
** 	Tests the Float macros of Variable. Will test a single scalar, vector and each scalar created from the vector.
**
** $Id: testLiveComponentRegister.c 2136 2005-05-10 02:47:13Z RaquibulHassan $
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

	stream = Journal_Register( Info_Type, "testLiveComponentRegister" );
	
	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}
	if( rank == procToWatch ) {
		typedef float Triple[3];

		float* array;
		Triple* structArray;

		LiveComponentRegister *LCRegister = NULL;
		Variable* var;
		Variable* vec;
		Variable* vecVar[3];
		Variable* tempVar = NULL;
		Index length = 10;

		Variable_Register* reg;

		array = Memory_Alloc_Array( float, length, "test" );
		structArray = Memory_Alloc_Array( Triple, length, "test" );

		reg = Variable_Register_New();

		var = Variable_NewScalar(
			"Scalar",
			Variable_DataType_Float,
			&length,
			(void**)&array,
			reg );

		vec = Variable_NewVector(
			"Three",
			Variable_DataType_Float,
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

		LCRegister = LiveComponentRegister_New( );
		Journal_Printf( stream, "Inserting live components into the LiveComponentRegistry\n" );
		LiveComponentRegister_Add( LCRegister, (Stg_Component*) var );
		LiveComponentRegister_Add( LCRegister, (Stg_Component*) vec );
		LiveComponentRegister_Add( LCRegister, (Stg_Component*) vecVar[0] );
		LiveComponentRegister_Add( LCRegister, (Stg_Component*) vecVar[1] );
		LiveComponentRegister_Add( LCRegister, (Stg_Component*) vecVar[2] );

		Journal_Printf( stream, "Printing the LiveComponentRegister:" );
		Print( LCRegister, stream );
		
		Journal_Printf( stream, "Retrieving live component references from the LiveComponentRegistry\n" );
	
		tempVar = (Variable*) LiveComponentRegister_Get( LCRegister, "Scalar" );
		if( tempVar != var ){
			Journal_Printf( stream, "Retrieval Error..!\n" );
		}
		else{
			Journal_Printf( stream, "Retrieval Successful\n" );
		}
		tempVar = (Variable*) LiveComponentRegister_Get( LCRegister, "Three" );
		if( tempVar != vec ){
			Journal_Printf( stream, "Retrieval Error..!\n" );
		}
		else{
			Journal_Printf( stream, "Retrieval Successful\n" );
		}
		tempVar = (Variable*) LiveComponentRegister_Get( LCRegister, "a" );
		if( tempVar != vecVar[0] ){
			Journal_Printf( stream, "Retrieval Error..!\n" );
		}
		else{
			Journal_Printf( stream, "Retrieval Successful\n" );
		}
		tempVar = (Variable*) LiveComponentRegister_Get( LCRegister, "b" );
		if( tempVar != vecVar[1] ){
			Journal_Printf( stream, "Retrieval Error..!\n" );
		}
		else{
			Journal_Printf( stream, "Retrieval Successful\n" );
		}
		tempVar = (Variable*) LiveComponentRegister_Get( LCRegister, "c" );
		if( tempVar != vecVar[2] ){
			Journal_Printf( stream, "Retrieval Error..!\n" );
		}
		else{
			Journal_Printf( stream, "Retrieval Successful\n" );
		}

		Journal_Printf( stream, "Deleting the LiveComponentRegister:" );
		Stg_Class_Delete( LCRegister );
	}
	
	BaseAutomation_Finalise();
	BaseContainer_Finalise();
	BaseIO_Finalise();
	BaseFoundation_Finalise();

	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0; /* success */
}


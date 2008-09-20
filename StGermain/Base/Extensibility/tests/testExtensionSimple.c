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
** $Id: testExtension-ofStruct.c 2933 2005-05-12 06:13:10Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"
#include "Base/Automation/Automation.h"
#include "Base/Extensibility/Extensibility.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

typedef struct {
	double x;
	double y;
	double z;
} BaseClass;


typedef struct {
	char type;
} ExtensionStruct0;
const Type Type0 = "Type0";

typedef struct {
	double temp;
} ExtensionStruct1;
const Type Temp0 = "Temp0";

typedef struct {
	float vel;
} ExtensionStruct2;
const Type Vel0 = "Vel0";

int main( int argc, char* argv[] ) {
	MPI_Comm CommWorld;
	int rank;
	int numProcessors;
	int procToWatch;
	Stream* stream;
	
	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size( CommWorld, &numProcessors );
	MPI_Comm_rank( CommWorld, &rank );
	
	BaseFoundation_Init( &argc, &argv );
	BaseIO_Init( &argc, &argv );
	BaseContainer_Init( &argc, &argv );
	BaseAutomation_Init( &argc, &argv );
	BaseExtensibility_Init( &argc, &argv );

	/* creating a stream */
	stream =  Journal_Register( InfoStream_Type, __FILE__ );

	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}
	if( rank == procToWatch ) {
		ExtensionManager*	objExtension;
		ExtensionManager*	structExtension;
		ExtensionManager*	arrayExtension;
		BaseClass*		baseObject;
		BaseClass*		nArray;

		ExtensionManager*	objExtensionCopy;
		ExtensionManager*	structExtensionCopy;
		ExtensionManager*	arrayExtensionCopy;
		BaseClass*		baseObjectCopy;
		BaseClass*		nArrayCopy;

		PtrMap*			copyMap;
		
		ExtensionStruct0*	objType0;
		ExtensionStruct1*	objTemp0;
		
		BaseClass* 		current;
		ExtensionStruct0*	nType0;
		ExtensionStruct1*	nTemp0;
		ExtensionStruct2*	nVel0;
		Index			i;
		Index			array_I;
		const Index		ArraySize = 8;
		
		Journal_Printf( (void*) stream, "Watching rank: %i\n", rank );
		
		/* Testing Simple (Single and Array) on Object */
		
		baseObject = Memory_Alloc( BaseClass, "BaseObject" );
		objExtension = ExtensionManager_New_OfExistingObject( "obj", baseObject );

		ExtensionManager_Add( objExtension, Type0, sizeof(ExtensionStruct0) );
		ExtensionManager_AddArray( objExtension, Temp0, sizeof(ExtensionStruct1), 10 );
		
		objType0 = (ExtensionStruct0*)ExtensionManager_Get(
			objExtension, 
			baseObject, 
			ExtensionManager_GetHandle( objExtension, "Type0" ) );
		objType0 = (ExtensionStruct0*)ExtensionManager_HashGet(
			objExtension, 
			baseObject, 
			Type0 );
		
		objTemp0 = (ExtensionStruct1*)ExtensionManager_Get( 
			objExtension, 
			baseObject, 
			ExtensionManager_GetHandle( objExtension, "Temp0" ) );
		objTemp0 = (ExtensionStruct1*)ExtensionManager_HashGet( 
			objExtension, 
			baseObject, 
			Temp0 );
	
		objType0->type = 'a';
		for ( i = 0; i < 10; ++i ) {
			objTemp0[i].temp = (double)i;
		}

		Journal_Printf( stream, "OfObject Final size %d\n", ExtensionManager_GetFinalSize( objExtension ) );
		Journal_Printf(
			stream, 
			"OfObject Type0 size %d, value %3c\n", 
			ExtensionInfo_At( objExtension->extInfos, ExtensionManager_GetHandle(objExtension, "Type0" ))->size,
			objType0->type );
	
		Journal_Printf(
			stream, 
			"OfObject Temp0 size %d, value ", 
			ExtensionInfo_At( objExtension->extInfos, ExtensionManager_GetHandle(objExtension, "Temp0" ))->size );
		for ( i = 0; i < 10; ++i ) {
			Journal_Printf( stream, "%3.0f ", objTemp0[i].temp );
		}
		Journal_Printf( stream, "\n" );

		/* Testing Simple (Single and Array) on Struct */
			
		structExtension = ExtensionManager_New_OfStruct( "Node", sizeof(BaseClass) );

		ExtensionManager_Add( structExtension, Type0, sizeof(ExtensionStruct0) );
		ExtensionManager_AddArray( structExtension, Temp0, sizeof(ExtensionStruct1), 10 );
		
		nArray = (BaseClass*)ExtensionManager_Malloc( structExtension, ArraySize );

		/* Testing Simple extension on allocated extended struct */
		arrayExtension = ExtensionManager_New_OfExtendedArray( "Ext", nArray, structExtension, ArraySize );

		ExtensionManager_Add( arrayExtension, Vel0, sizeof(ExtensionStruct2) );
	
		for ( array_I = 0; array_I < ArraySize; ++array_I ) {
			current = ExtensionManager_At( structExtension, nArray, array_I );

			nType0 = (ExtensionStruct0*)ExtensionManager_Get(
				structExtension,
				current,
				ExtensionManager_GetHandle( structExtension, "Type0" ) );
			nType0 = (ExtensionStruct0*)ExtensionManager_HashGet(
				structExtension,
				current,
				Type0 );
			nType0->type = 'a';
		
			nTemp0 = (ExtensionStruct1*)ExtensionManager_Get(
				structExtension,
				current,
				ExtensionManager_GetHandle( structExtension, "Temp0" ) );
			nTemp0 = (ExtensionStruct1*)ExtensionManager_HashGet(
				structExtension,
				current,
				Temp0 );

			for ( i = 0; i < 10; ++i ) {
				nTemp0[i].temp = (double)((array_I * 10) + i);
			}
			
			nVel0 = (ExtensionStruct2*)ExtensionManager_Get(
				arrayExtension,
				ExtensionManager_At( arrayExtension, nArray, array_I ),
				ExtensionManager_GetHandle( arrayExtension, "Vel0" ) );
			nVel0 = (ExtensionStruct2*)ExtensionManager_HashGet(
				arrayExtension,
				ExtensionManager_At( arrayExtension, nArray, array_I ),
				Vel0 );
			
			nVel0->vel = (float)array_I;
		}
		
		Journal_Printf( stream, "OfStruct Final size %d\n", ExtensionManager_GetFinalSize( structExtension ) );
		Journal_Printf(
			stream, 
			"OfStruct Type0 size %d\n", 
			ExtensionInfo_At( structExtension->extInfos, ExtensionManager_GetHandle(structExtension, "Type0"))->size );
		Journal_Printf(
			stream, 
			"OfStruct Temp0 size %d\n", 
			ExtensionInfo_At( structExtension->extInfos, ExtensionManager_GetHandle(structExtension, "Temp0"))->size );


		for ( array_I = 0; array_I < ArraySize; ++array_I ) {
			current = ExtensionManager_At( arrayExtension, nArray, array_I );

			nType0 = (ExtensionStruct0*)ExtensionManager_Get(
				arrayExtension,
				current,
				ExtensionManager_GetHandle( arrayExtension, "Type0" ) );
			nType0 = (ExtensionStruct0*)ExtensionManager_HashGet(
				arrayExtension,
				current,
				Type0 );
			
			nTemp0 = (ExtensionStruct1*)ExtensionManager_Get(
				arrayExtension,
				current,
				ExtensionManager_GetHandle( arrayExtension, "Temp0" ) );
			nTemp0 = (ExtensionStruct1*)ExtensionManager_HashGet(
				arrayExtension,
				current,
				Temp0 );
		
			Journal_Printf( stream, "OfArray Type0 value %3c\n", nType0->type );
			
			Journal_Printf( stream, "OfArray Temp0 value " );
			for ( i = 0; i < 10; ++i ) {
				Journal_Printf( stream, "%3.0f ", nTemp0[i].temp );
			}
			Journal_Printf( stream, "\n" );
			
			nVel0 = (ExtensionStruct2*)ExtensionManager_Get(
				arrayExtension,
				ExtensionManager_At( arrayExtension, nArray, array_I ),
				ExtensionManager_GetHandle( arrayExtension, "Vel0" ) );
			nVel0 = (ExtensionStruct2*)ExtensionManager_HashGet(
				arrayExtension,
				ExtensionManager_At( arrayExtension, nArray, array_I ),
				Vel0 );
			
			Journal_Printf( stream, "OfArray Vel0 value %f\n", nVel0->vel );
		}


		/* Copy time */
		copyMap = PtrMap_New( 1 );

		baseObjectCopy = Memory_Alloc( BaseClass, "BaseObject" );
		memcpy( baseObjectCopy, baseObject, sizeof(BaseClass) );
		PtrMap_Append( copyMap, baseObject, baseObjectCopy );
		objExtensionCopy = Stg_Class_Copy( objExtension, NULL, True, "_dup", copyMap );

		if ( objExtensionCopy->_existingObject != baseObjectCopy ) {
			Journal_Printf( stream, "Failed to copy baseObject properly\n" );
		}
		else {
			Journal_Printf( stream, "Sucessfully copied baseObject properly\n" );
		}
		objType0 = (ExtensionStruct0*)ExtensionManager_HashGet(
			objExtensionCopy, 
			baseObjectCopy, 
			Type0 );
		Journal_Printf(
			stream, 
			"OfObject Copy Type0 size %d, value %3c\n", 
			ExtensionInfo_At( 
				objExtensionCopy->extInfos, 
				ExtensionManager_GetHandle(objExtensionCopy, Type0 ))->size,
			objType0->type );
		objTemp0 = (ExtensionStruct1*)ExtensionManager_HashGet( 
			objExtensionCopy, 
			baseObjectCopy, 
			Temp0 );
	
		objType0->type = 'a';
		Journal_Printf(
			stream, 
			"OfObject Copy Temp0 size %d, value ", 
			ExtensionInfo_At( objExtension->extInfos, ExtensionManager_GetHandle(objExtension, "Temp0" ))->size );
		for ( i = 0; i < 10; ++i ) {
			Journal_Printf( stream, "%3.0f ", objTemp0[i].temp );
		}
		Journal_Printf( stream, "\n" );

		Stg_Class_Delete( copyMap );
		
	
		copyMap = PtrMap_New( 1 );
		arrayExtensionCopy = Stg_Class_Copy( arrayExtension, NULL, True, "_dup", copyMap );
		structExtensionCopy = Stg_Class_Copy( structExtension, NULL, True, "_dup", copyMap );
		nArrayCopy = PtrMap_Find( copyMap, arrayExtension->_array );

		if ( nArrayCopy != NULL && nArrayCopy == arrayExtensionCopy->_array ) {
			Journal_Printf( stream, "Array extension copy success\n" );
		}
		else {
			Journal_Printf( stream, "Array extension copy fail\n" );
		}
	
		for ( array_I = 0; array_I < ArraySize; ++array_I ) {
			current = ExtensionManager_At( arrayExtensionCopy, nArrayCopy, array_I );

			nType0 = (ExtensionStruct0*)ExtensionManager_HashGet(
				arrayExtensionCopy,
				current,
				Type0 );
			nTemp0 = (ExtensionStruct1*)ExtensionManager_HashGet(
				arrayExtensionCopy,
				current,
				Temp0 );
		
			Journal_Printf( stream, "OfArray Copy Type0 value %3c\n", nType0->type );
			
			Journal_Printf( stream, "OfArray Copy Temp0 value " );
			for ( i = 0; i < 10; ++i ) {
				Journal_Printf( stream, "%3.0f ", nTemp0[i].temp );
			}
			Journal_Printf( stream, "\n" );
			
			nVel0 = (ExtensionStruct2*)ExtensionManager_HashGet(
				arrayExtensionCopy,
				current,
				Vel0 );
			
			Journal_Printf( stream, "OfArray Copy Vel0 value %f\n", nVel0->vel );
		}

		Stg_Class_Delete( copyMap );
		
		/* Clean up */

		
		Memory_Free( baseObject );
		ExtensionManager_Free( structExtension, nArray );

		Stg_Class_Delete( objExtension );
		Stg_Class_Delete( structExtension );
		Stg_Class_Delete( arrayExtension );

		Memory_Free( baseObjectCopy );
		Stg_Class_Delete( objExtensionCopy );
	}
	
	BaseExtensibility_Finalise();
	BaseAutomation_Finalise();
	BaseContainer_Finalise();
	BaseIO_Finalise();
	BaseFoundation_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0; /* success */
}


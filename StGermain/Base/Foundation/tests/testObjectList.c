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
** $Id: testNamedStg_ObjectList.c 2432 2005-08-08 23:01:59Z Raquibul Hassan $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "Base/Foundation/Foundation.h"
#include "Regresstor/libRegresstor/Regresstor.h"

#include "JournalWrappers.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <mpi.h>
#include <string.h>


const Type DummyClass_Type = "DummyClass_Type";

Stg_Class* DummyClass_New( void ) {
	return _Stg_Class_New( 
		sizeof(Stg_Class),
		DummyClass_Type,
		_Stg_Class_Delete,
		_Stg_Class_Print,
		_Stg_Class_Copy );
}


int* DummyPointer_New( void ) {
	return Memory_Alloc_Unnamed( int );
}

void DummyPointer_Print( void* ptr, struct Stream* stream ) {
	Journal_Printf( stream, "value: %i\n", *(int*)ptr );
}

void* DummyPointer_Copy( void* ptr, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap ) {
	int* newInt;
	
	newInt = Memory_Alloc_Unnamed( int );
	*newInt = *(int*)ptr;
	
	return newInt;
}


void DummyFunc1( void ) {
}

void DummyFunc2( void ) {
}

void DummyFunc3( void ) {
}

void DummyFunc4( void ) {
}

void DummyFunc5( void ) {
}

void DummyFunc6( void ) {
}

int main(int argc, char *argv[])
{
	int			rank;
	int			procCount;
	int			procToWatch;
	Stream*			stream;

	/* Initialise MPI, get world info */
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &procCount);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	BaseFoundation_Init( &argc, &argv );

	RegressionTest_Init( "Base/Foundation/ObjectList" );

	stream = Journal_Register( "info", "myStream" );
	
	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}

	if ( rank == procToWatch ) {
		Stg_ObjectList* ol0;
		Stg_ObjectList* ol0deep;
		Stg_Class* addPtr0;
		Stg_ObjectList ol1;
		Stg_ObjectList* ol1deep;
		Stg_Class* addPtr1;
		Stg_ObjectList* ol2;
		Stg_ObjectList* ol2deep;
		int* addPtr2;
		Stg_ObjectList* ol3;
		Stg_ObjectList* ol3deep;
		int* addPtr3;
		
		
		/* Test 1: Construction */
		ol0 = Stg_ObjectList_New();
		Stg_ObjectList_Init( &ol1 );
		ol2 = Stg_ObjectList_New();
		ol3 = Stg_ObjectList_New();
		RegressionTest_Check(
			ol0 &&
			ol2 &&
			ol3,
			stream,
			"Construction",
			"Can we use New()?" );
		
		/* Test 2: Can we append the first entry? */
		Stg_ObjectList_ClassAppend(  ol0, (addPtr0 = DummyClass_New()), "a" );
		Stg_ObjectList_ClassAppend( &ol1, (addPtr1 = DummyClass_New()), "a" );
		Stg_ObjectList_PointerAppend(  ol2, (addPtr2 = DummyPointer_New()), "a", 0, DummyPointer_Print, DummyPointer_Copy );
		Stg_ObjectList_GlobalPointerAppend(  ol3, (addPtr3 = (void*)DummyFunc1), "a" );
		RegressionTest_Check(
			addPtr0 == Stg_ObjectList_Get( ol0, "a" ) &&
			addPtr0 == Stg_ObjectList_ObjectAt( ol0, 0 ) &&
			Stg_ObjectList_Count( ol0 ) == 1 &&
			addPtr1 == Stg_ObjectList_Get( &ol1, "a" ) &&
			addPtr1 == Stg_ObjectList_ObjectAt( &ol1, 0 ) &&
			Stg_ObjectList_Count( &ol1 ) == 1 &&
			addPtr2 == Stg_ObjectList_Get( ol2, "a" ) &&
			addPtr2 == Stg_ObjectList_ObjectAt( ol2, 0 ) &&
			Stg_ObjectList_Count( ol2 ) == 1 &&
			addPtr3 == Stg_ObjectList_Get( ol3, "a" ) &&
			addPtr3 == Stg_ObjectList_ObjectAt( ol3, 0 ) &&
			Stg_ObjectList_Count( ol3 ) == 1,
			stream,
			"Append",
			"Can we append the first entry?" );
		
		/* Test 3: Can we prepend the second entry? */
		Stg_ObjectList_ClassPrepend(  ol0, (addPtr0 = DummyClass_New()), "b" );
		Stg_ObjectList_ClassPrepend( &ol1, (addPtr1 = DummyClass_New()), "b" );
		Stg_ObjectList_PointerPrepend(  ol2, (addPtr2 = DummyPointer_New()), "b", 0, DummyPointer_Print, DummyPointer_Copy );
		Stg_ObjectList_GlobalPointerPrepend(  ol3, (addPtr3 = (void*)DummyFunc2), "b" );
		RegressionTest_Check(
			addPtr0 == Stg_ObjectList_Get( ol0, "b" ) &&
			addPtr0 == Stg_ObjectList_ObjectAt( ol0, 0 ) &&
			Stg_ObjectList_Count( ol0 ) == 2 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol0, 1 ) ), "a" ) == 0 &&
			addPtr1 == Stg_ObjectList_Get( &ol1, "b" ) &&
			addPtr1 == Stg_ObjectList_ObjectAt( &ol1, 0 ) &&
			Stg_ObjectList_Count( &ol1 ) == 2 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( &ol1, 1 ) ), "a" ) == 0 &&
			addPtr2 == Stg_ObjectList_Get( ol2, "b" ) &&
			addPtr2 == Stg_ObjectList_ObjectAt( ol2, 0 ) &&
			Stg_ObjectList_Count( ol2 ) == 2 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol2, 1 ) ), "a" ) == 0 &&
			addPtr3 == Stg_ObjectList_Get( ol3, "b" ) &&
			addPtr3 == Stg_ObjectList_ObjectAt( ol3, 0 ) &&
			Stg_ObjectList_Count( ol3 ) == 2 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol3, 1 ) ), "a" ) == 0,
			stream,
			"Prepend",
			"Can we prepend the second entry?" );
		
		/* Test 4: Can we insert before "a" the third entry? */
		Stg_ObjectList_ClassInsertBefore(  ol0, "a", (addPtr0 = DummyClass_New()), "c" );
		Stg_ObjectList_ClassInsertBefore( &ol1, "a", (addPtr1 = DummyClass_New()), "c" );
		Stg_ObjectList_PointerInsertBefore(  
			ol2, 
			"a", 
			(addPtr2 = DummyPointer_New()), 
			"c", 
			0, 
			DummyPointer_Print, 
			DummyPointer_Copy );
		Stg_ObjectList_GlobalPointerInsertBefore(  ol3, "a", (addPtr3 = (void*)DummyFunc3), "c" );
		RegressionTest_Check(
			addPtr0 == Stg_ObjectList_Get( ol0, "c" ) &&
			addPtr0 == Stg_ObjectList_ObjectAt( ol0, 1 ) &&
			Stg_ObjectList_Count( ol0 ) == 3 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol0, 0 ) ), "b" ) == 0 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol0, 2 ) ), "a" ) == 0 &&
			addPtr1 == Stg_ObjectList_Get( &ol1, "c" ) &&
			addPtr1 == Stg_ObjectList_ObjectAt( &ol1, 1 ) &&
			Stg_ObjectList_Count( &ol1 ) == 3 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( &ol1, 0 ) ), "b" ) == 0 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( &ol1, 2 ) ), "a" ) == 0 &&
			addPtr2 == Stg_ObjectList_Get( ol2, "c" ) &&
			addPtr2 == Stg_ObjectList_ObjectAt( ol2, 1 ) &&
			Stg_ObjectList_Count( ol2 ) == 3 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol2, 0 ) ), "b" ) == 0 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol2, 2 ) ), "a" ) == 0 &&
			addPtr3 == Stg_ObjectList_Get( ol3, "c" ) &&
			addPtr3 == Stg_ObjectList_ObjectAt( ol3, 1 ) &&
			Stg_ObjectList_Count( ol3 ) == 3 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol3, 0 ) ), "b" ) == 0 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol3, 2 ) ), "a" ) == 0,
			stream,
			"Insert Before",
			"Can we insert before \"a\" the third entry" );
		
		/* Test 5: Can we insert after "c" the fourth entry? */
		Stg_ObjectList_ClassInsertAfter(  ol0, "c", (addPtr0 = DummyClass_New()), "d" );
		Stg_ObjectList_ClassInsertAfter( &ol1, "c", (addPtr1 = DummyClass_New()), "d" );
		Stg_ObjectList_PointerInsertAfter(  
			ol2, 
			"c", 
			(addPtr2 = DummyPointer_New()), 
			"d", 
			0, 
			DummyPointer_Print, 
			DummyPointer_Copy );
		Stg_ObjectList_GlobalPointerInsertAfter(  ol3, "c", (addPtr3 = (void*)DummyFunc4), "d" );
		RegressionTest_Check(
			addPtr0 == Stg_ObjectList_Get( ol0, "d" ) &&
			addPtr0 == Stg_ObjectList_ObjectAt( ol0, 2 ) &&
			Stg_ObjectList_Count( ol0 ) == 4 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol0, 0 ) ), "b" ) == 0 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol0, 1 ) ), "c" ) == 0 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol0, 3 ) ), "a" ) == 0 &&
			addPtr1 == Stg_ObjectList_Get( &ol1, "d" ) &&
			addPtr1 == Stg_ObjectList_ObjectAt( &ol1, 2 ) &&
			Stg_ObjectList_Count( &ol1 ) == 4 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( &ol1, 0 ) ), "b" ) == 0 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( &ol1, 1 ) ), "c" ) == 0 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( &ol1, 3 ) ), "a" ) == 0 &&
			addPtr2 == Stg_ObjectList_Get( ol2, "d" ) &&
			addPtr2 == Stg_ObjectList_ObjectAt( ol2, 2 ) &&
			Stg_ObjectList_Count( ol2 ) == 4 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol2, 0 ) ), "b" ) == 0 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol2, 1 ) ), "c" ) == 0 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol2, 3 ) ), "a" ) == 0 &&
			addPtr3 == Stg_ObjectList_Get( ol3, "d" ) &&
			addPtr3 == Stg_ObjectList_ObjectAt( ol3, 2 ) &&
			Stg_ObjectList_Count( ol3 ) == 4 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol3, 0 ) ), "b" ) == 0 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol3, 1 ) ), "c" ) == 0 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol3, 3 ) ), "a" ) == 0,
			stream,
			"Insert After",
			"Can we insert after \"c\" the fourth entry?" );
		
		/* Test 6: Can we replace "d" with the fifth entry? */
		Stg_ObjectList_ClassReplace(  ol0, "d", DELETE, (addPtr0 = DummyClass_New()), "e" );
		Stg_ObjectList_ClassReplace( &ol1, "d", DELETE, (addPtr1 = DummyClass_New()), "e" );
		Stg_ObjectList_PointerReplace(  
			ol2, 
			"d", 
			DELETE, 
			(addPtr2 = DummyPointer_New()), 
			"e", 
			0, 
			DummyPointer_Print, 
			DummyPointer_Copy );
		Stg_ObjectList_GlobalPointerReplace(  ol3, "d", DELETE, (addPtr3 = (void*)DummyFunc5), "e" );
		RegressionTest_Check(
			addPtr0 == Stg_ObjectList_Get( ol0, "e" ) &&
			addPtr0 == Stg_ObjectList_ObjectAt( ol0, 2 ) &&
			Stg_ObjectList_Count( ol0 ) == 4 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol0, 0 ) ), "b" ) == 0 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol0, 1 ) ), "c" ) == 0 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol0, 3 ) ), "a" ) == 0 &&
			addPtr1 == Stg_ObjectList_Get( &ol1, "e" ) &&
			addPtr1 == Stg_ObjectList_ObjectAt( &ol1, 2 ) &&
			Stg_ObjectList_Count( &ol1 ) == 4 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( &ol1, 0 ) ), "b" ) == 0 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( &ol1, 1 ) ), "c" ) == 0 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( &ol1, 3 ) ), "a" ) == 0 &&
			addPtr2 == Stg_ObjectList_Get( ol2, "e" ) &&
			addPtr2 == Stg_ObjectList_ObjectAt( ol2, 2 ) &&
			Stg_ObjectList_Count( ol2 ) == 4 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol2, 0 ) ), "b" ) == 0 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol2, 1 ) ), "c" ) == 0 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol2, 3 ) ), "a" ) == 0 &&
			addPtr3 == Stg_ObjectList_Get( ol3, "e" ) &&
			addPtr3 == Stg_ObjectList_ObjectAt( ol3, 2 ) &&
			Stg_ObjectList_Count( ol3 ) == 4 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol3, 0 ) ), "b" ) == 0 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol3, 1 ) ), "c" ) == 0 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol3, 3 ) ), "a" ) == 0,
			stream,
			"Replace",
			"Can we replace \"d\" with the fifth entry?" );
		
		/* Test 7: Can we remove the "c" entry? */
		Stg_ObjectList_Remove(  ol0, "c", DELETE );
		Stg_ObjectList_Remove( &ol1, "c", DELETE );
		Stg_ObjectList_Remove(  ol2, "c", DELETE );
		Stg_ObjectList_Remove(  ol3, "c", DELETE );
		RegressionTest_Check(
			Stg_ObjectList_Count( ol0 ) == 3 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol0, 0 ) ), "b" ) == 0 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol0, 1 ) ), "e" ) == 0 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol0, 2 ) ), "a" ) == 0 &&
			Stg_ObjectList_Count( &ol1 ) == 3 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( &ol1, 0 ) ), "b" ) == 0 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( &ol1, 1 ) ), "e" ) == 0 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( &ol1, 2 ) ), "a" ) == 0 &&
			Stg_ObjectList_Count( ol2 ) == 3 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol2, 0 ) ), "b" ) == 0 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol2, 1 ) ), "e" ) == 0 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol2, 2 ) ), "a" ) == 0 &&
			Stg_ObjectList_Count( ol3 ) == 3 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol3, 0 ) ), "b" ) == 0 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol3, 1 ) ), "e" ) == 0 &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol3, 2 ) ), "a" ) == 0,
			stream,
			"Remove",
			"Can we remove the \"c\" entry?" );
		
		/* Test 8: Can we replace all with the sixth entry? */
		Stg_ObjectList_ClassReplaceAll(  ol0, DELETE, (addPtr0 = DummyClass_New()), "f" );
		Stg_ObjectList_ClassReplaceAll( &ol1, DELETE, (addPtr1 = DummyClass_New()), "f" );
		Stg_ObjectList_PointerReplaceAll(  
			ol2, 
			DELETE, 
			(addPtr2 = DummyPointer_New()), 
			"f", 
			0, 
			DummyPointer_Print, 
			DummyPointer_Copy );
		Stg_ObjectList_GlobalPointerReplaceAll(  ol3, DELETE, (addPtr3 = (void*)DummyFunc6), "f" );
		RegressionTest_Check(
			addPtr0 == Stg_ObjectList_Get( ol0, "f" ) &&
			addPtr0 == Stg_ObjectList_ObjectAt( ol0, 0 ) &&
			Stg_ObjectList_Count( ol0 ) == 1 &&
			addPtr1 == Stg_ObjectList_Get( &ol1, "f" ) &&
			addPtr1 == Stg_ObjectList_ObjectAt( &ol1, 0 ) &&
			Stg_ObjectList_Count( &ol1 ) == 1 &&
			addPtr2 == Stg_ObjectList_Get( ol2, "f" ) &&
			addPtr2 == Stg_ObjectList_ObjectAt( ol2, 0 ) &&
			Stg_ObjectList_Count( ol2 ) == 1 &&
			addPtr3 == Stg_ObjectList_Get( ol3, "f" ) &&
			addPtr3 == Stg_ObjectList_ObjectAt( ol3, 0 ) &&
			Stg_ObjectList_Count( ol3 ) == 1,
			stream,
			"ReplaceAll",
			"Can we replace all with the sixth entry?" );
		
		/* Test 9: Copying */
		/* Shallow copying not yet implemented */
		/* Deep copying "OfPointer" (i.e. non-StGermain class) not yet implemented */
		ol0deep = Stg_Class_Copy(  ol0, 0, True, 0, 0 );
		ol1deep = Stg_Class_Copy( &ol1, 0, True, 0, 0 );
		ol2deep = Stg_Class_Copy(  ol2, 0, True, 0, 0 );
		ol3deep = Stg_Class_Copy(  ol3, 0, True, 0, 0 );
		RegressionTest_Check(
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol0deep, 0 ) ), "f" ) == 0 &&
			Stg_ObjectList_Count( ol0deep ) == 1 &&
			Stg_ObjectList_ObjectAt( ol0, 0 ) != Stg_ObjectList_ObjectAt( ol0deep, 0 ) &&
			Stg_Class_GetType( (Stg_Class*)Stg_ObjectList_ObjectAt( ol0, 0 ) ) == Stg_Class_GetType( (Stg_Class*)Stg_ObjectList_ObjectAt( ol0deep, 0 ) ) &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol1deep, 0 ) ), "f" ) == 0 &&
			Stg_ObjectList_Count( ol1deep ) == 1 &&
			Stg_ObjectList_ObjectAt( &ol1, 0 ) != Stg_ObjectList_ObjectAt( ol1deep, 0 ) &&
			Stg_Class_GetType( (Stg_Class*)Stg_ObjectList_ObjectAt( &ol1, 0 ) ) == Stg_Class_GetType( (Stg_Class*)Stg_ObjectList_ObjectAt( ol1deep, 0 ) ) &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol2deep, 0 ) ), "f" ) == 0 &&
			Stg_ObjectList_Count( ol2deep ) == 1 &&
			Stg_ObjectList_ObjectAt( ol2, 0 ) != Stg_ObjectList_ObjectAt( ol2deep, 0 ) &&
			strcmp( Stg_Object_GetName( (Stg_Object*)Stg_ObjectList_At( ol3deep, 0 ) ), "f" ) == 0 &&
			Stg_ObjectList_Count( ol3deep ) == 1 &&
			Stg_ObjectList_ObjectAt( ol3, 0 ) == Stg_ObjectList_ObjectAt( ol3deep, 0 ) && /* different to others */
			Stg_Class_GetType( (Stg_Class*)Stg_ObjectList_ObjectAt( ol3, 0 ) ) == Stg_Class_GetType( (Stg_Class*)Stg_ObjectList_ObjectAt( ol3deep, 0 ) ),
			stream,
			"Copy",
			"Copying" );
		
		/* Test 9: Destruction */
		Stg_Class_Delete(  ol3deep );
		Stg_Class_Delete(  ol2deep );
		Stg_Class_Delete(  ol1deep );
		Stg_Class_Delete(  ol0deep );
		Stg_Class_Delete(  ol3 );
		Stg_Class_Delete(  ol2 );
		Stg_Class_Delete( &ol1 );
		Stg_Class_Delete(  ol0 );
		RegressionTest_Check( 1, stream, "Destruction", "Deleting all allocated memory" );
	}

	RegressionTest_Finalise();

	BaseFoundation_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0; /* success */
}

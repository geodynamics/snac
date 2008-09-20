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
**	Tests accuracy of memory statistics generation.
**
** $Id: testMemory2.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "Base/Foundation/Foundation.h"
#include "JournalWrappers.h"

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

struct StructA
{
	int x;
	float y;
	char z;
};
typedef struct StructA StructA;

struct StructB
{
	double x;
};
typedef struct StructB StructB;

struct StructC
{
	char* x;
	StructA a;
};
typedef struct StructC StructC;

int main( int argc, char* argv[] )
{	
	MPI_Comm CommWorld;
	int rank;
	int numProcessors;
	int procToWatch;
	
	Stream* stream;

	MemoryReport* report;

	void* bytesObj;
	void* bytesArray;

	StructA*	object;
	StructB*	array1d;
	StructC**	array2d;
	StructA***	array3d;
	StructB****	array4d;
	StructC*	one2d;
	StructA*	one3d;
	StructB*	one4d;
	StructC**	complex2d;

	Index**		setup;
	StructA***	complex3d;

	Index x1 = 4;
	Index y1[] = { 1, 2, 3, 4 };

	Index x2 = 2;
	Index y2[] = { 1, 1 };

	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size( CommWorld, &numProcessors );
	MPI_Comm_rank( CommWorld, &rank );
	
	BaseFoundation_Init( &argc, &argv );
	
	stream = Journal_Register ( "info", "MyInfo" );
	
	if( argc >= 2 )
	{
		procToWatch = atoi( argv[1] );
	}
	else
	{
		procToWatch = 0;
	}
	if( rank == procToWatch ) Journal_Printf( stream,  "Watching rank: %i\n", rank );

	bytesObj = Memory_Alloc_Bytes( 5, "Bytes", "Test1" );
	bytesArray = Memory_Alloc_Array_Bytes( 3, 10, "Bytes", "Test1" );

	object = Memory_Alloc( StructA, "Test1" );
	array1d = Memory_Alloc_Array( StructB, 3, "Test2" );
	array2d = Memory_Alloc_2DArray( StructC, 4, 5, "Test2" );
	array3d = Memory_Alloc_3DArray( StructA, 2, 3, 4, "Test2" );
	array4d = Memory_Alloc_4DArray_Unnamed( StructB, 5, 4, 3, 2 );
	one2d = Memory_Alloc_2DArrayAs1D_Unnamed( StructC, 4, 2 );
	one3d = Memory_Alloc_3DArrayAs1D_Unnamed( StructA, 2, 2, 3 );
	one4d = Memory_Alloc_4DArrayAs1D( StructB, 4, 2, 3, 5, "Test1" );

	complex2d = Memory_Alloc_2DComplex( StructC, x1, y1, "Test1" );

	setup = Memory_Alloc_3DSetup( x2, y2 );
	setup[0][0] = 2;
	setup[1][0] = 3;
	complex3d = Memory_Alloc_3DComplex( StructA, x2, y2, setup, "Test1" );

	if( rank == procToWatch )
	{
		Journal_Printf( stream,  "Before Freeing memory\n" );

		report = MemoryReport_New();
		MemoryReport_AddGroup( report, MEMORYREPORT_TYPE );
		MemoryReport_AddGroup( report, MEMORYREPORT_NAME );
		MemoryReport_Print( report );
	
		report = MemoryReport_New();
		MemoryReport_AddCondition( report, MEMORYREPORT_NAME, "Test1" );
		MemoryReport_AddGroup( report, MEMORYREPORT_TYPE );
		MemoryReport_Print( report );

		report = MemoryReport_New();
		MemoryReport_AddGroup( report, MEMORYREPORT_FILE );
		MemoryReport_AddGroup( report, MEMORYREPORT_FUNC );
		MemoryReport_AddGroup( report, MEMORYREPORT_TYPE );
		MemoryReport_AddGroup( report, MEMORYREPORT_NAME );
		MemoryReport_Print( report );

		report = MemoryReport_New();
		MemoryReport_AddCondition( report, MEMORYREPORT_TYPE, "StructA" );
		MemoryReport_AddCondition( report, MEMORYREPORT_NAME, "Test2" );
		MemoryReport_AddGroup( report, MEMORYREPORT_FUNC );
		MemoryReport_Print( report );
	}

	Memory_Free( bytesObj );
	Memory_Free( bytesArray );
	Memory_Free_Type( StructA );
	Memory_Free_Type( StructB );
	Memory_Free_Type( StructC );
	
	BaseFoundation_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();

	return 0; /* success */
}

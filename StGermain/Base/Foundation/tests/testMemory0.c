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
** $Id: testMemory0.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "Base/Foundation/Foundation.h"

#include "JournalWrappers.h"

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main( int argc, char* argv[] )
{
	MPI_Comm CommWorld;
	int rank;
	int numProcessors;
	int procToWatch;
	
	Stream* stream;

	double**	array2d;
	double***	array3d;
	double****	array4d;
	
	double*		one2d;
	double*		one3d;
	double*		one4d;

	double**	complex2d;

	Index**		setup;
	double***	complex3d;

	Index x1 = 5;
	Index y1[] = { 1, 2, 3, 4, 5 };

	Index x2 = 3;
	Index y2[] = { 4, 2, 3 };

	Index i, j, k, l;

	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size( CommWorld, &numProcessors );
	MPI_Comm_rank( CommWorld, &rank );
	
	BaseFoundation_Init( &argc, &argv );
	
	stream = Journal_Register ( "info", "MyInfo" );
	
	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}
	if( rank == procToWatch ) Journal_Printf( stream,  "Watching rank: %i\n", rank );

	Journal_Printf( stream, "2D Array\n" );
	array2d = Memory_Alloc_2DArray_Unnamed( double, 2, 3 );
	for ( i = 0; i < 2; ++i )
	{
		for ( j = 0; j < 3; ++j )
		{
			array2d[i][j] = i + (j / 10.0);
			Journal_Printf( stream, "%lf ", array2d[i][j] );			
		}
		Journal_Printf( stream, "\n");
	}

	Journal_Printf( stream, "3D Array\n" );
	array3d = Memory_Alloc_3DArray_Unnamed( double, 4, 3, 2 );
	for ( i = 0; i < 4; ++i )
	{
		for ( j = 0; j < 3; ++j )
		{
			for ( k = 0; k < 2; ++k )
			{
				array3d[i][j][k] = i + (j / 10.0) + (k / 100.0);
				Journal_Printf( stream, "%lf ", array3d[i][j][k] );
			}
			Journal_Printf( stream, "\n");
		}
	}

	Journal_Printf( stream, "4D Array\n" );
	array4d = Memory_Alloc_4DArray_Unnamed( double, 4, 3, 2, 3 );
	for ( i = 0; i < 4; ++i )
	{
		for ( j = 0; j < 3; ++j )
		{
			for ( k = 0; k < 2; ++k )
			{
				for ( l = 0; l < 3; ++l )
				{
					array4d[i][j][k][l] = i + (j / 10.0) + (k / 100.0) + (l / 1000.0);
					Journal_Printf( stream, "%lf ", array4d[i][j][k][l] );
				}
				Journal_Printf( stream, "\n");
			}
		}
	}

	Journal_Printf( stream, "2D as 1D\n" );
	one2d = Memory_Alloc_2DArrayAs1D_Unnamed( double, 3, 2 );
	for ( i = 0; i < 3; ++i )
	{
		for ( j = 0; j < 2; ++j )
		{
			Memory_Access2D( one2d, i, j, 2 ) = i + (j / 10.0);
			Journal_Printf( stream, "%lf ", Memory_Access2D( one2d, i, j, 2 ) );
		}
		Journal_Printf( stream, "\n" );
	}


	Journal_Printf( stream, "3D as 1D \n" );
	one3d = Memory_Alloc_3DArrayAs1D_Unnamed( double, 4, 3, 2 );
	for ( i = 0; i < 4; ++i )
	{
		for ( j = 0; j < 3; ++j )
		{
			for ( k = 0; k < 2; ++k )
			{
				Memory_Access3D( one3d, i, j, k, 3, 2 ) = i + (j / 10.0) + (k / 100.0);
				Journal_Printf( stream, "%lf ", Memory_Access3D( one3d, i, j, k, 3, 2 ) );
			}
			Journal_Printf( stream, "\n");
		}
	}


	Journal_Printf( stream, "4D as 1D\n" );
	one4d = Memory_Alloc_4DArrayAs1D_Unnamed( double, 4, 3, 2, 3 );
	for ( i = 0; i < 4; ++i )
	{
		for ( j = 0; j < 3; ++j )
		{
			for ( k = 0; k < 2; ++k )
			{
				for ( l = 0; l < 3; ++l )
				{
					Memory_Access4D( one4d, i, j, k, l, 3, 2, 3 ) = i + (j / 10.0) + (k / 100.0) + (l / 1000.0);
					Journal_Printf( stream, "%lf ", Memory_Access4D( one4d, i, j, k, l, 3, 2, 3 ) );
				}
				Journal_Printf( stream, "\n");
			}
		}
	}

	Journal_Printf( stream, "2D Complex\n" );
	complex2d = Memory_Alloc_2DComplex_Unnamed( double, x1, y1 );
	for (i = 0; i < x1; ++i)
	{
		for (j = 0; j < y1[i]; ++j)
		{
			complex2d[i][j] = i + (double)(j / 10.0);
			Journal_Printf( stream, "%lf ", complex2d[i][j] );
		}
		Journal_Printf( stream, "\n" );
	}
	
	Journal_Printf( stream, "3D Complex\n" );
	setup = Memory_Alloc_3DSetup( x2, y2 );
	setup[0][0] = 2;
	setup[0][1] = 3;
	setup[0][2] = 2;
	setup[0][3] = 3;
	setup[1][0] = 1;
	setup[1][1] = 5;
	setup[2][0] = 2;
	setup[2][1] = 4;
	setup[2][2] = 2;

	complex3d = Memory_Alloc_3DComplex_Unnamed( double, x2, y2, setup );	
	for (i = 0; i < x2; ++i)
	{
		for (j = 0; j < y2[i]; ++j)
		{
			for (k = 0; k < setup[i][j]; ++k)
			{
				complex3d[i][j][k] = i + (j / 10.0) + (k / 100.0);
				Journal_Printf( stream, "%lf ", complex3d[i][j][k] );
			}
			Journal_Printf( stream, "\n" );
		}
	}

	Journal_Printf( 
		stream, 
		"Testing Memory_IsAllocated() for true: %s\n",
		( Memory_IsAllocated( array2d ) ) ? "OK" : "Fail" );

	Journal_Printf(
		stream,
		"Testing instance counter init value: %s\n",
		( Memory_CountGet( array2d ) == 0 ) ? "OK" : "Fail" );

	Memory_CountInc( array2d );
	
	Journal_Printf(
		stream,
		"Testing instance counter inc: %s\n",
		( Memory_CountGet( array2d ) == 1 ) ? "OK" : "Fail" );

	Memory_CountDec( array2d );
	
	Journal_Printf(
		stream,
		"Testing instance counter dec: %s\n",
		( Memory_CountGet( array2d ) == 0 ) ? "OK" : "Fail" );
	
	Memory_Free( array2d );
	Memory_Free( array3d );
	Memory_Free( array4d );
	
	Journal_Printf( 
		stream, 
		"Testing Memory_IsAllocated() for false: %s\n",
		( ! Memory_IsAllocated( array2d ) ) ? "OK" : "Fail" );
	
	Memory_Free( one2d );
	Memory_Free( one3d );
	Memory_Free( one4d );
	
	Memory_Free( complex2d );
	Memory_Free( setup );
	Memory_Free( complex3d );

	BaseFoundation_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();

	return 0; /* success */
}

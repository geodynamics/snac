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
**	Tests the allocation, reading and writing of 2D arrays in 1D form.
**
** $Id: testMemory2DArrayAs1D.c 3462 2006-02-19 06:53:24Z WalterLandry $
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

	double*		one2d;
	Index i, j;

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
	if( rank == procToWatch )
	{
		Journal_Printf( stream,  "Watching rank: %i\n", rank );
	}

	Journal_Printf( stream, "2D as 1D\n" );
	one2d = Memory_Alloc_2DArrayAs1D_Unnamed( double, 3, 2 );

	Memory_Print();

	/* write */	
	for ( i = 0; i < 3; ++i )
	{
		for ( j = 0; j < 2; ++j )
		{
			Memory_Access2D( one2d, i, j, 2 ) = i + (j / 10.0);
		}
	}

	/* read */
	for ( i = 0; i < 3; ++i )
	{
		for ( j = 0; j < 2; ++j )
		{
			Journal_Printf( stream, "%lf ", Memory_Access2D( one2d, i, j, 2 ) );
		}
		Journal_Printf( stream, "\n" );
	}

	one2d = Memory_Realloc_2DArrayAs1D( one2d, double, 3, 2, 4, 4 );

	Memory_Print();

	/* read again */
	for ( i = 0; i < 3; ++i )
	{
		for ( j = 0; j < 2; ++j )
		{
			Journal_Printf( stream, "%lf ", Memory_Access2D( one2d, i, j, 4 ) );
		}
		Journal_Printf( stream, "\n" );
	}
	
	/* write */	
	for ( i = 0; i < 4; ++i )
	{
		for ( j = 0; j < 4; ++j )
		{
			Memory_Access2D( one2d, i, j, 4 ) = i + (j / 10.0);
		}
	}

	/* read */
	for ( i = 0; i < 4; ++i )
	{
		for ( j = 0; j < 4; ++j )
		{
			Journal_Printf( stream, "%lf ", Memory_Access2D( one2d, i, j, 4 ) );
		}
		Journal_Printf( stream, "\n" );
	}


	Memory_Free( one2d );

	Memory_Print();

	BaseFoundation_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();

	return 0; /* success */
}

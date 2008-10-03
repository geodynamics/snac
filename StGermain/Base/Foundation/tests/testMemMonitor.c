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

#include <unistd.h>


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

	RegressionTest_Init( "Base/Foundation/MemMonitor" );

	stream = Journal_Register( "info", "myStream" );
	
	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}

	Stg_MemMonitor_SetMemoryWatchCriteria( 0.25 );

	{
		char *a;
		char *b;
		char *c;
		char *d;
		char *e;
		char *f;
		
		Stg_MemMonitor* tm = Stg_MemMonitor_New( "test", True, True, MPI_COMM_WORLD );
		a = Memory_Alloc_Array( char, 1000, "a" );
	
		Stg_MemMonitor_Begin( tm );
		b = Memory_Alloc_Array( char, 2000, "b" );
		Stg_MemMonitor_End( tm );
	
		Stg_MemMonitor_Begin( tm );
		c = Memory_Alloc_Array( char, 100, "c" );
		d = Memory_Alloc_Array( char, 100000, "d" );
		Stg_MemMonitor_End( tm );
		
		Stg_MemMonitor_Begin( tm );
		Memory_Free( a );
		Memory_Free( c );
		Memory_Free( d );
		Stg_MemMonitor_End( tm );

		Stg_MemMonitor_Begin( tm );
		e = Memory_Alloc_Array( char, 10000, "e" );
		f = Memory_Alloc_Array( char, 10000, "f" );
		Stg_MemMonitor_End( tm );
		
		Memory_Free( b );
		Memory_Free( e );
		Memory_Free( f );
		Stg_MemMonitor_Delete( tm );
	}

	RegressionTest_Finalise();
	BaseFoundation_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0; /* success */
}

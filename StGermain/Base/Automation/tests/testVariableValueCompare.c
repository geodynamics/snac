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
** $Id: testVariableValueCompare.c 3462 2006-02-19 06:53:24Z WalterLandry $
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

	stream = Journal_Register( Info_Type, "test" );
	
	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}
	if( rank == procToWatch ) {
		Variable*           orig;
		Variable*           compare;
		double*             data;
		double*             data2;
		Index               arrayCount      = 150;
		Index               componentCount  = 4;
		Index               index;
		double              amp             = 0.1;
		double              tolerance       = 0.04;
		Variable_Register*	vr;

		data = Memory_Alloc_Array( double, arrayCount * componentCount, "test" );
		data2 = Memory_Alloc_Array( double, arrayCount * componentCount, "test" );
		for( index = 0; index < arrayCount * componentCount; index++ ) {
			data[index] = 1.0f / (arrayCount+2) * (index+1); 
			data2[ index ] = data[ index ] + amp * cos( index );
		}		
		
		vr = Variable_Register_New();
		orig = Variable_NewVector( 
				"orig", 
				Variable_DataType_Double, 
				componentCount,
				&arrayCount,
				(void**)&data,
				vr,
				"orig1",
				"orig2",
				"orig3",
				"orig4" );
		compare = Variable_NewVector( 
				"compare", 
				Variable_DataType_Double, 
				componentCount,
				&arrayCount,
				(void**)&data2,
				vr,
				"compare1",
				"compare2",
				"compare3",
				"compare4" );
		Build( orig, 0, False );
		Build( compare, 0, False );

		Journal_PrintDouble( stream, Variable_ValueCompare( orig, compare ) );
		Journal_PrintBool( stream, Variable_ValueCompareWithinTolerance( orig, compare, tolerance ) );

		Memory_Free( data );
		Memory_Free( data2 );
	}
	
	BaseAutomation_Finalise();
	BaseContainer_Finalise();
	BaseIO_Finalise();
	BaseFoundation_Finalise();

	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0; /* success */
}

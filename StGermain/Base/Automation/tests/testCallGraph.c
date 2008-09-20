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
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"
#include "Base/Automation/Automation.h"
#include "Regresstor/libRegresstor/Regresstor.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <mpi.h>
#include <string.h>


const Type TestFunc1_Type = "TestFunc1";
void TestFunc1( void ) {}

const Type TestFunc2_Type = "TestFunc2";
void TestFunc2( void ) {}

int main( int argc, char *argv[] ) {
	int			rank;
	int			procCount;
	int			procToWatch;
	Stream*			stream;

	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_size( MPI_COMM_WORLD, &procCount );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );

	BaseFoundation_Init( &argc, &argv );
	BaseIO_Init( &argc, &argv );
	BaseContainer_Init( &argc, &argv );
	BaseAutomation_Init( &argc, &argv );

	RegressionTest_Init( "Base/Automation/CallGraph" );

	stream = Journal_Register( "info", "myStream" );
	
	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}

	if( rank == procToWatch ) {
		Stg_CallGraph*	cg0;
		Stg_CallGraph 	cg1;
		Stg_CallGraph*	cg2;
		Stg_CallGraph*	cg3;
		Stg_CallGraph*	cg3deep;
		Index		count;
		Index		size;
		Index		i;
		
		
		/* Test 1: Construction */
		cg0 = Stg_CallGraph_New();
		Stg_CallGraph_Init( &cg1 );
		cg2 = Stg_CallGraph_New();
		cg3 = Stg_CallGraph_New();
		RegressionTest_Check( cg0 && cg2 && cg3, stream, "Construction", "Can we use New()?" );
		
		
		/* Test 2: Can we push the first call on the stack? */
		Stg_CallGraph_Push( cg0, TestFunc1, TestFunc1_Type );
		Stg_CallGraph_Push( &cg1, TestFunc1, TestFunc1_Type );
		RegressionTest_Check( 
			cg0->_stack && 
			!cg0->_stack->pop && 
			cg0->_stack->entry_I == 0 && 
			cg0->_stack->functionPtr == TestFunc1 &&
			cg0->_tableCount == 1 && 
			cg0->table[0].name == TestFunc1_Type &&
			cg0->table[0].functionPtr == TestFunc1 &&
			cg0->table[0].parentPtr == 0 &&
			cg0->table[0].returned == 0 &&
			cg0->table[0].called == 1 &&
			cg1._stack && 
			!cg1._stack->pop && 
			cg1._stack->entry_I == 0 && 
			cg1._stack->functionPtr == TestFunc1 &&
			cg1._tableCount == 1 && 
			cg1.table[0].name == TestFunc1_Type &&
			cg1.table[0].functionPtr == TestFunc1 &&
			cg1.table[0].parentPtr == 0 &&
			cg1.table[0].returned == 0 &&
			cg1.table[0].called == 1, 
			stream, "One push", "First push?" );
		
		
		/* Test 3: Can we pop the first call on the stack? */
		Stg_CallGraph_Pop( cg0 );
		Stg_CallGraph_Pop( &cg1 );
		RegressionTest_Check( 
			!cg0->_stack && 
			cg0->table[0].name == TestFunc1_Type &&
			cg0->table[0].functionPtr == TestFunc1 &&
			cg0->table[0].parentPtr == 0 &&
			cg0->table[0].returned == 1 &&
			cg0->table[0].called == 1 &&
			!cg1._stack && 
			cg1.table[0].name == TestFunc1_Type &&
			cg1.table[0].functionPtr == TestFunc1 &&
			cg1.table[0].parentPtr == 0 &&
			cg1.table[0].returned == 1 &&
			cg1.table[0].called == 1, 
			stream, "One pop", "Pop first push?" );
		
		
		/* Test 4: Ensure that each function pointer - parent pointer - name is a unique entry */
		Stg_CallGraph_Push( cg2, TestFunc1, TestFunc1_Type ); /* p0: f:1 n:1  Should add */
		Stg_CallGraph_Push( cg2, TestFunc2, TestFunc2_Type ); /* p1: f:2 n:2  Should add */
		Stg_CallGraph_Pop( cg2 );
		Stg_CallGraph_Push( cg2, TestFunc2, TestFunc2_Type ); /* p1: f:2 n:2  Should increment */
		Stg_CallGraph_Pop( cg2 );
		Stg_CallGraph_Push( cg2, TestFunc1, TestFunc2_Type ); /* p1: f:1 n:2  Should add (recursive case) */
		Stg_CallGraph_Pop( cg2 );
		Stg_CallGraph_Push( cg2, TestFunc2, TestFunc1_Type ); /* p1: f:2 n:1  Should add */
		Stg_CallGraph_Push( cg2, TestFunc1, TestFunc1_Type ); /* p2: f:1 n:1  Should add */
		Stg_CallGraph_Pop( cg2 );
		Stg_CallGraph_Pop( cg2 );
		Stg_CallGraph_Pop( cg2 );
		RegressionTest_Check( 
			!cg2->_stack && 
			cg2->_tableCount == 5 && 
			cg2->table[0].returned == 1 &&
			cg2->table[0].called == 1 &&
			cg2->table[1].returned == 2 &&
			cg2->table[1].called == 2 &&
			cg2->table[2].returned == 1 &&
			cg2->table[2].called == 1 &&
			cg2->table[3].returned == 1 &&
			cg2->table[3].called == 1 &&
			cg2->table[4].returned == 1 &&
			cg2->table[4].called == 1,
			stream, "New entries", "Unique for function-parent-name?" );
		
		
		/* Test 5: Force a realloc of the table */
		count = (Index)((double)1.5 * cg3->_tableSize);
		size = cg3->_tableSize;
		for( i = 0; i < count; i++ ) {
			/* Use "i" as a unique string (unique pointer value)... don't try to print! */
			Stg_CallGraph_Push( cg3, TestFunc1, (Name)i );
		}
		RegressionTest_Check( 
			cg3->_tableCount == count && 
			cg3->_tableSize == (size * 2),
			stream, "Table realloc", "Does the table grow when needed?" );
		
		
		/* Test 9: Copying */
		/* Shallow copying not yet implemented */
		cg3deep = Stg_Class_Copy( cg3, 0, True, 0, 0 );
		RegressionTest_Check(
			cg3->_tableCount == cg3deep->_tableCount &&
			cg3->_tableSize == cg3deep->_tableSize &&
			/* TODO: check not just the table, but the stack too */
			memcmp( cg3->table, cg3deep->table, sizeof(_Stg_CallGraph_Entry) * cg3->_tableCount ) == 0,
			stream,
			"Copy",
			"Copying" );
		
		/* Test 9: Destruction */
		Stg_Class_Delete(  cg3deep );
		Stg_Class_Delete(  cg3 );
		Stg_Class_Delete(  cg2 );
		Stg_Class_Delete( &cg1 );
		Stg_Class_Delete(  cg0 );
		RegressionTest_Check( 1, stream, "Destruction", "Deleting all allocated memory" );
	}

	RegressionTest_Finalise();

	BaseAutomation_Finalise();
	BaseContainer_Finalise();
	BaseIO_Finalise();
	BaseFoundation_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0; /* success */
}

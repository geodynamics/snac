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
** $Id: testRegisterFactory.c 2421 2004-12-14 02:09:26Z AlanLo $
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

typedef struct {
	__Stg_Object
} TestObject;

TestObject* TestObject_New( Name name ) {
	return (TestObject*)_Stg_Object_New(
		sizeof(TestObject),
		"Test",
		_Stg_Object_Delete,
		_Stg_Object_Print,
		NULL, 
		name,
		NON_GLOBAL );
}

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

	stream = Journal_Register( Info_Type, "testRegisterFactory" );
	
	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}
	if( rank == procToWatch ) {
		RegisterFactory* factory;

		NamedObject_Register* reg;

		factory = RegisterFactory_New();

		RegisterFactory_Register( factory, NamedObject_Register_Type, (void*)NamedObject_Register_New );
		RegisterFactory_Register( factory, Variable_Register_Type, (void*)Variable_Register_New );

		reg = (NamedObject_Register*)RegisterFactory_Create( factory, NamedObject_Register_Type );

		NamedObject_Register_Add( reg, TestObject_New( "a" ) );
		NamedObject_Register_Add( reg, TestObject_New( "b" ) );
		NamedObject_Register_Add( reg, TestObject_New( "c" ) );
	
		Print( factory, stream );
	
		Stg_Class_Delete( reg );
		Stg_Class_Delete( factory );
	}
	
	BaseAutomation_Finalise();
	BaseContainer_Finalise();
	BaseIO_Finalise();
	BaseFoundation_Finalise();

	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0; /* success */
}

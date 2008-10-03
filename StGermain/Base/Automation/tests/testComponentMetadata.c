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
** 	Tests Stg_Component copying
**
** $Id: testLiveComponentRegister.c 2136 2005-05-10 02:47:13Z RaquibulHassan $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"
#include "Base/Automation/Automation.h"

#include "Regresstor/libRegresstor/Regresstor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


/* This prototype might be removed later on */
const char* Stg_Component_GetMetadata();

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

	RegressionTest_Init( "Base/Automation/Stg_Component" );
	
	BaseIO_Init( &argc, &argv );
	BaseContainer_Init( &argc, &argv );
	BaseAutomation_Init( &argc, &argv );

	stream = Journal_Register( Info_Type, __FILE__ );
	
	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}
	if( rank == procToWatch ) {
		Stg_ComponentMeta* metaTest;

		XML_IO_Handler* io;
		Dictionary* allDict;
		Dictionary* compDict;

		CompositeVC* vc;
		
		Journal_Printf( stream, "%s\n", Stg_Component_GetMetadata() );

		metaTest = Stg_Component_CreateMeta( "blah", Variable_Type );
		Stg_Class_Print( metaTest, stream );
		Stg_Class_Delete( metaTest );

		allDict = Dictionary_New();
		io = XML_IO_Handler_New();
		IO_Handler_ReadAllFromFile( io, "data/metatest.xml", allDict );
		compDict = Dictionary_GetDictionary( allDict, "components" );
		vc = CompositeVC_DefaultNew( "vc" );	
		
		metaTest = Stg_Component_Validate( vc, CompositeVC_Type, compDict );
		Stg_Class_Print( metaTest, stream );
		Stg_Class_Delete( metaTest );

		Stg_Class_Delete( io );
		Stg_Class_Delete( compDict );
	}

	
	BaseAutomation_Finalise();
	BaseContainer_Finalise();
	BaseIO_Finalise();

	RegressionTest_Finalise();
	
	BaseFoundation_Finalise();

	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0; /* success */
}


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
** $Id: testEntryPoint-min-max.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"
#include "Base/Automation/Automation.h"
#include "Base/Extensibility/Extensibility.h"

#include "stdio.h"
#include "stdlib.h"
#include "mpi.h"

double Return1( Stream* stream ) {
	Journal_Printf( stream, "In func %s\n", __func__ );
	return 1.0;
}
	
double Return89( Stream* stream ) {
	Journal_Printf( stream, "In func %s\n", __func__ );
	return 89.0;
}

double ReturnNeg43( Stream* stream ) {
	Journal_Printf( stream, "In func %s\n", __func__ );
	return -43;
}
double ReturnZero( Stream* stream ) {
	Journal_Printf( stream, "In func %s\n", __func__ );
	return 0.0;
}

const Name testEpName = "test";

int main( int argc, char* argv[] ) {
	MPI_Comm CommWorld;
	int rank;
	int numProcessors;
	int procToWatch;
	EntryPoint* entryPoint;
	Stream* stream;
	double  result;
	
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

	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}
	
	/* creating a stream */
	stream =  Journal_Register( InfoStream_Type, "myStream" );
	Stream_SetPrintingRank( stream, procToWatch );
	
	Journal_Printf( stream, "Watching rank: %i\n", rank );
	
	/* Get Maximum of Values */
	entryPoint = EntryPoint_New( testEpName, EntryPoint_Maximum_VoidPtr_CastType );
	EP_Append( entryPoint, Return1 );
	EP_Append( entryPoint, Return89 );
	EP_Append( entryPoint, ReturnNeg43 );
	EP_Append( entryPoint, ReturnZero );
	result = ((EntryPoint_Maximum_VoidPtr_CallCast*) entryPoint->run)( entryPoint, stream );
	Journal_PrintDouble( stream, result );
	Stg_Class_Delete( entryPoint );

	/* Get Minimum of Values */
	entryPoint = EntryPoint_New( testEpName, EntryPoint_Minimum_VoidPtr_CastType );
	EP_Append( entryPoint, Return1 );
	EP_Append( entryPoint, Return89 );
	EP_Append( entryPoint, ReturnNeg43 );
	EP_Append( entryPoint, ReturnZero );
	result = ((EntryPoint_Minimum_VoidPtr_CallCast*) entryPoint->run)( entryPoint, stream );
	Journal_PrintDouble( stream, result );
	Stg_Class_Delete( entryPoint );	

	BaseExtensibility_Finalise();
	BaseAutomation_Finalise();
	BaseContainer_Finalise();
	BaseIO_Finalise();
	BaseFoundation_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();

	return 0; /* success */
}

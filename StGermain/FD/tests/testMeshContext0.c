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
**	Tests MeshContext in the most basic sence... creation on default values, run, and delete.
**
** $Id: testMeshContext0.c 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "StGermain.h"
#include "StGermain/FD/FD.h"
#include "Base/Base.h"
#include <stdio.h>
#include <stdlib.h>

Stream* testInfoStream;

struct _Node {
	Coord coord;
	double temp;
};

struct _Element {
	Coord coord;
	double temp;
};

void MyDelete( void* meshContext ) {
	_MeshContext_Delete( meshContext );
}

void MyPrint( void* meshContext, Stream* stream ) {
	_MeshContext_Print( meshContext, stream );
}

void MyBuild( void* context ) {
	printf( "This is Build\n" );
}

void MyInitialConditions( void* context ) {
	printf( "This is InitialConditions\n" );
}

double dt = 2.0f;

double MyDt( void* context ) {
	printf( "This is Dt, returning %g\n", dt );
	return dt;
}

void MySolve( void* context ) {
	printf( "This is Solve\n" );
}

void MySync( void* context ) {
	printf( "This is Sync\n" );
}

void MyOutput( void* context ) {
	printf( "This is Output\n" );
}

void MySetDt( void* context, double _dt ) {
	printf( "This is SetDt... setting to %g\n", _dt );
	dt = _dt;
}

int main( int argc, char* argv[] ) {
	MPI_Comm CommWorld;
	int rank;
	int numProcessors;
	int procToWatch;
	Dictionary* dictionary;
	Topology*			nTopology;
	ElementLayout*			eLayout;
	NodeLayout*			nLayout;
	MeshDecomp*			decomp;
	MeshLayout*			meshLayout;
	MeshContext* meshContext;
	
	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size( CommWorld, &numProcessors );
	MPI_Comm_rank( CommWorld, &rank );
	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}
	if( rank == procToWatch ) printf( "Watching rank: %i\n", rank );
	
	StGermain_Init( &argc, &argv );
	FD_Init( &argc, &argv );
	
	/* Initialize Stream */
	testInfoStream =  Journal_Register( InfoStream_Type, "testInfoStream" );

	/* Read input */
	dictionary = Dictionary_New();
	dictionary->add( dictionary, "meshSizeI", Dictionary_Entry_Value_FromUnsignedInt( 4 ) );
	dictionary->add( dictionary, "meshSizeJ", Dictionary_Entry_Value_FromUnsignedInt( 3 ) );
	dictionary->add( dictionary, "meshSizeK", Dictionary_Entry_Value_FromUnsignedInt( 4 ) );
	dictionary->add( dictionary, "minX", Dictionary_Entry_Value_FromDouble( 0.0f ) );
	dictionary->add( dictionary, "minY", Dictionary_Entry_Value_FromDouble( 0.0f ) );
	dictionary->add( dictionary, "minZ", Dictionary_Entry_Value_FromDouble( 0.0f ) );
	dictionary->add( dictionary, "maxX", Dictionary_Entry_Value_FromDouble( 300.0f ) );
	dictionary->add( dictionary, "maxY", Dictionary_Entry_Value_FromDouble( 10.0f ) );
	dictionary->add( dictionary, "maxZ", Dictionary_Entry_Value_FromDouble( 300.0f ) );
	
	/* Build the context */
	nTopology = (Topology*)IJK6Topology_New( "IJKTopology", dictionary );
	eLayout = (ElementLayout*)ParallelPipedHexaEL_New( "PPHexaEL", 3, dictionary );
	nLayout = (NodeLayout*)CornerNL_New( "CornerNL", dictionary, eLayout, nTopology );
	decomp = (MeshDecomp*)HexaMD_New( "HexaMD", dictionary, MPI_COMM_WORLD, eLayout, nLayout );
	meshLayout = MeshLayout_New( "MeshLayout", eLayout, nLayout, decomp );
	meshContext = _MeshContext_New( 
		sizeof(MeshContext), 
		"TestContext", 
		MyDelete, 
		MyPrint, 
		NULL,
		NULL, 
		NULL, 
		_AbstractContext_Build, 
		_AbstractContext_Initialise, 
		_AbstractContext_Execute, 
		_AbstractContext_Destroy, 
		"meshContext", 
		True, 
		MySetDt, 
		0, 
		10, 
		meshLayout, 
		sizeof(Node), 
		sizeof(Element), 
		CommWorld, 
		dictionary );

	/* Add entry points to the context */
	EntryPoint_ReplaceAll( 
		Context_GetEntryPoint( meshContext, AbstractContext_EP_Solve ),
		"test", 
		MySolve, 
		"TestMeshContext" );
	EntryPoint_ReplaceAll( Context_GetEntryPoint( meshContext, AbstractContext_EP_Dt ),
		"test", 
		MyDt, 
		"TestMeshContext" );
	EntryPoint_ReplaceAll( Context_GetEntryPoint( meshContext, AbstractContext_EP_Sync ),
		"test", 
		MySync, 
		"TestMeshContext" );
	EntryPoint_ReplaceAll( Context_GetEntryPoint( meshContext, AbstractContext_EP_FrequentOutput ),
		"test", 
		MyOutput, 
		"TestMeshContext" );

	if( rank == procToWatch ) {
		Stream* stream = Journal_Register( InfoStream_Type, MeshContext_Type );

		printf( "meshContext->entryPointList->count: %u\n", meshContext->entryPoint_Register->count );
		printf( "meshContext->entryPointList->_size: %lu\n", meshContext->entryPoint_Register->_size );
		Context_PrintConcise( meshContext, stream );
	}
	
	/* Run the context */
	if( rank == procToWatch ) {
		Stg_Component_Build( meshContext, 0 /* dummy */, False );
		Stg_Component_Initialise( meshContext, 0 /* dummy */, False );
		Stg_Component_Execute( meshContext, 0 /* dummy */, False );
	}
	
	/* Stg_Class_Delete stuff */
	Stg_Component_Destroy( meshContext, 0 /* dummy */, False );
	Stg_Class_Delete( meshContext );
	Stg_Class_Delete( meshLayout );
	Stg_Class_Delete( decomp );
	Stg_Class_Delete( nLayout );
	Stg_Class_Delete( eLayout );
	Stg_Class_Delete( nTopology );
	Stg_Class_Delete( dictionary );
	
	FD_Finalise();
	StGermain_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0; /* success */
}

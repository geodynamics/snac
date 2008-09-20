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
** $Id: testExtension-ofStruct.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"
#include "Base/Automation/Automation.h"
#include "Base/Extensibility/Extensibility.h"

#include "Regresstor/libRegresstor/Regresstor.h"

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

typedef struct {
	double x;
	double y;
	double z;
	char dim;
} BaseClass;


typedef struct {
	char type;
} ExtensionStruct0;
const Type Type0 = "Type0";
const Type Type1 = "Type1";

typedef struct {
	double temp;
} ExtensionStruct1;
const Type Temp0 = "Temp0";
const Type Temp1 = "Temp1";

typedef struct {
	double pres;
	char flag;
} ExtensionStruct2;
const Type Pres0 = "Pres0";
const Type Pres1 = "Pres1";

typedef struct {
	char dd;
	char cc;
	int bc;
} ExtensionStruct3;
const Type BC_Set0 = "BC_Set0";
const Type BC_Set1 = "BC_Set1";

typedef struct {
	float wf;
} ExtensionStruct4;
const Type Weight0 = "Weight0";
const Type Weight1 = "Weight1";


int main( int argc, char* argv[] ) {
	MPI_Comm CommWorld;
	int rank;
	int numProcessors;
	int procToWatch;
	Stream* stream;
	
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

	RegressionTest_Init( "Base/Extensibility/ExtensionManager" );

	/* creating a stream */
	stream =  Journal_Register( InfoStream_Type, "myStream" );

	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}
	if( rank == procToWatch ) {
		ExtensionManager*	extensionMgr;
		BaseClass*		nArray;
		BaseClass*		n;
		ExtensionStruct0*	nType0;
		ExtensionStruct1*	nTemp0;
		ExtensionStruct2*	nPres0;
		ExtensionStruct3*	nBC_Set0;
		ExtensionStruct4*	nWeight0;
		ExtensionStruct0*	nType1;
		ExtensionStruct1*	nTemp1;
		ExtensionStruct2*	nPres1;
		ExtensionStruct3*	nBC_Set1;
		ExtensionStruct4*	nWeight1;
		Index			i;
		const Index		ArraySize = 1024;

		Index			numExtensions = 0;		/* variable to check number of extensions */
		
		SizeT			size0;
		SizeT			size1;
		SizeT			size2;
		SizeT			size3;
		SizeT			size4;

		size0 = ExtensionManager_Align( sizeof(ExtensionStruct0) );
		size1 = ExtensionManager_Align( sizeof(ExtensionStruct1) );
		size2 = ExtensionManager_Align( sizeof(ExtensionStruct2) );
		size3 = ExtensionManager_Align( sizeof(ExtensionStruct3) );
		size4 = ExtensionManager_Align( sizeof(ExtensionStruct4) );

		Journal_Printf( (void*) stream, "Watching rank: %i\n", rank );
		
		/* Build the extensionMgr */
		extensionMgr = ExtensionManager_New_OfStruct( "Node", sizeof(BaseClass) );
		ExtensionManager_Add( extensionMgr, Type0, sizeof(ExtensionStruct0) );		numExtensions++;
		ExtensionManager_Add( extensionMgr, Temp0, sizeof(ExtensionStruct1) );		numExtensions++;
		ExtensionManager_Add( extensionMgr, Pres0, sizeof(ExtensionStruct2) );		numExtensions++;
		ExtensionManager_Add( extensionMgr, BC_Set0, sizeof(ExtensionStruct3) );	numExtensions++;
		ExtensionManager_Add( extensionMgr, Weight0, sizeof(ExtensionStruct4) );	numExtensions++;
		ExtensionManager_Add( extensionMgr, Type1, sizeof(ExtensionStruct0) );		numExtensions++;
		ExtensionManager_Add( extensionMgr, Temp1, sizeof(ExtensionStruct1) );		numExtensions++;
		ExtensionManager_Add( extensionMgr, Pres1, sizeof(ExtensionStruct2) );		numExtensions++;
		ExtensionManager_Add( extensionMgr, BC_Set1, sizeof(ExtensionStruct3) );	numExtensions++;
		ExtensionManager_Add( extensionMgr, Weight1, sizeof(ExtensionStruct4) );	numExtensions++;
	
		RegressionTest_Check(
			extensionMgr->initialSize == sizeof(BaseClass),
			stream,
			"initialSize",
			"Is the initial size correct" );

		RegressionTest_Check(
			ExtensionInfo_At( extensionMgr->extInfos, 0 )->offset == sizeof(BaseClass),
			stream,
			"First offset",
			"Is the first offset at the correct position" );

		RegressionTest_Check(
			extensionMgr->finalSize == 
				sizeof(BaseClass) + 
				size0 + size1 + size2 + size3 + size4 +
				size0 + size1 + size2 + size3 + size4,
			stream,
			"finalSize",
			"Is the final size correct" );

		RegressionTest_Check(
			extensionMgr->extInfos->count == numExtensions,
			stream,
			"count",
			"Is the number of extensions correct" );

		/* Alloc, initialise and view*/
		nArray = (BaseClass*)ExtensionManager_Malloc( extensionMgr, ArraySize );

		RegressionTest_Check(
			nArray != NULL,
			stream,
			"ExtensionManager_Malloc()",
			"Can we malloc the extension" );
	
		n = (BaseClass*)ExtensionManager_At( extensionMgr, nArray, 0 );

		nType0 = (ExtensionStruct0*)ExtensionManager_Get( extensionMgr, n, 0 );
		nType0 = (ExtensionStruct0*)ExtensionManager_HashGet( extensionMgr, n, Type0 );
		RegressionTest_Check(
			((ArithPointer)&nType0->type - (ArithPointer)n) == sizeof(BaseClass),
			stream,
			"Extension 0",
			"Is offset correct" );
	
		nTemp0 = (ExtensionStruct1*)ExtensionManager_Get( extensionMgr, n, 1 );
		nTemp0 = (ExtensionStruct1*)ExtensionManager_HashGet( extensionMgr, n, Temp0 );
		RegressionTest_Check(
			((ArithPointer)&nTemp0->temp - (ArithPointer)n) == 
				sizeof(BaseClass) + size0,
			stream,
			"Extension 1",
			"Is offset correct" );
		
		nPres0 = (ExtensionStruct2*)ExtensionManager_Get( extensionMgr, n, 2 );		
		nPres0 = (ExtensionStruct2*)ExtensionManager_HashGet( extensionMgr, n, Pres0 );
		RegressionTest_Check(
			((ArithPointer)&nPres0->pres - (ArithPointer)n) == 
				sizeof(BaseClass) + size0 + size1,
			stream,
			"Extension 2",
			"Is offset correct" );

		nBC_Set0 = (ExtensionStruct3*)ExtensionManager_Get( extensionMgr, n, 3 );
		nBC_Set0 = (ExtensionStruct3*)ExtensionManager_HashGet( extensionMgr, n, BC_Set0 );
		RegressionTest_Check(
			((ArithPointer)&nBC_Set0->dd - (ArithPointer)n) == 
				sizeof(BaseClass) + size0 + size1 + size2,
			stream,
			"Extension 3",
			"Is offset correct" );

		nWeight0 = (ExtensionStruct4*)ExtensionManager_Get( extensionMgr, n, ExtensionManager_GetHandle( extensionMgr, "Weight0" ));
		nWeight0 = (ExtensionStruct4*)ExtensionManager_HashGet( extensionMgr, n, Weight0 );
		RegressionTest_Check(
			((ArithPointer)&nWeight0->wf - (ArithPointer)n) == 
				sizeof(BaseClass) + size0 + size1 + size2 + size3,
			stream,
			"Extension 4",
			"Is offset correct" );

		nType1 = (ExtensionStruct0*)ExtensionManager_Get( extensionMgr, n, ExtensionManager_GetHandle( extensionMgr, "Type1" ) );
		nType1 = (ExtensionStruct0*)ExtensionManager_HashGet( extensionMgr, n, Type1 );
		RegressionTest_Check(
			((ArithPointer)&nType1->type - (ArithPointer)n) == 
				sizeof(BaseClass) + size0 + size1 + size2 + size3 + size4,
			stream,
			"Extension 5",
			"Is offset correct" );

		nTemp1 = (ExtensionStruct1*)ExtensionManager_Get( extensionMgr, n, ExtensionManager_GetHandle( extensionMgr, "Temp1" ) );
		nTemp1 = (ExtensionStruct1*)ExtensionManager_HashGet( extensionMgr, n, Temp1);
		RegressionTest_Check(
			((ArithPointer)&nTemp1->temp - (ArithPointer)n) == 
				sizeof(BaseClass) + size0 + size1 + size2 + size3 + size4 + size0,
			stream,
			"Extension 6",
			"Is offset correct" );

		nPres1 = (ExtensionStruct2*)ExtensionManager_Get( extensionMgr, n, ExtensionManager_GetHandle( extensionMgr, "Pres1" ) );
		nPres1 = (ExtensionStruct2*)ExtensionManager_HashGet( extensionMgr, n, Pres1 );
		RegressionTest_Check(
			((ArithPointer)&nPres1->pres - (ArithPointer)n) == 
				sizeof(BaseClass) + size0 + size1 + size2 + size3 + size4 + size0 + size1,
			stream,
			"Extension 7",
			"Is offset correct" );

		nBC_Set1 = (ExtensionStruct3*)ExtensionManager_Get( extensionMgr, n, ExtensionManager_GetHandle( extensionMgr, "BC_Set1" ) );
		nBC_Set1 = (ExtensionStruct3*)ExtensionManager_HashGet( extensionMgr, n, BC_Set1 );
		RegressionTest_Check(
			((ArithPointer)&nBC_Set1->dd - (ArithPointer)n) == 
				sizeof(BaseClass) + size0 + size1 + size2 + size3 + size4 + size0 + size1 + size2,
			stream,
			"Extension 8",
			"Is offset correct" );

		nWeight1 = (ExtensionStruct4*)ExtensionManager_Get( extensionMgr, n, ExtensionManager_GetHandle( extensionMgr, "Weight1" ) );
		nWeight1 = (ExtensionStruct4*)ExtensionManager_HashGet( extensionMgr, n, Weight1 );
		RegressionTest_Check(
			((ArithPointer)&nWeight1->wf - (ArithPointer)n) == 
				sizeof(BaseClass) + size0 + size1 + size2 + size3 + size4 + size0 + size1 + size2 + size3,
			stream,
			"Extension 9",
			"Is offset correct" );

		for( i = 0; i < ArraySize; i++ ) {
			Journal_Printf( (void*) stream, "Index: %u\n", i );
			n = (BaseClass*)ExtensionManager_At( extensionMgr, nArray, i );
			
			n->x = 1.0f;
			n->y = 2.0f;
			n->z = 3.0f;
			n->dim = 4;
			
			nType0 = (ExtensionStruct0*)ExtensionManager_Get( extensionMgr, n, 0 );
			nType0 = (ExtensionStruct0*)ExtensionManager_HashGet( extensionMgr, n, Type0 );
			nType0->type = 5;
			
			nTemp0 = (ExtensionStruct1*)ExtensionManager_Get( extensionMgr, n, 1 );
			nTemp0 = (ExtensionStruct1*)ExtensionManager_HashGet( extensionMgr, n, Temp0 );
			nTemp0->temp = 6.0f;
			
			nPres0 = (ExtensionStruct2*)ExtensionManager_Get( extensionMgr, n, 2 );
			nPres0 = (ExtensionStruct2*)ExtensionManager_HashGet( extensionMgr, n, Pres0 );
			nPres0->pres = 7.0f;
			nPres0->flag = 8;
			
			nBC_Set0 = (ExtensionStruct3*)ExtensionManager_Get( extensionMgr, n, 3 );
			nBC_Set0 = (ExtensionStruct3*)ExtensionManager_HashGet( extensionMgr, n, BC_Set0 );
			nBC_Set0->dd = 9;
			nBC_Set0->cc = 10;
			nBC_Set0->bc = 11;
			
			nWeight0 = (ExtensionStruct4*)ExtensionManager_Get( extensionMgr, n, ExtensionManager_GetHandle( extensionMgr, "Weight0" ) );
			nWeight0 = (ExtensionStruct4*)ExtensionManager_HashGet( extensionMgr, n, Weight0 );
			nWeight0->wf = 12.0f;
			
			nType1 = (ExtensionStruct0*)ExtensionManager_Get( extensionMgr, n, ExtensionManager_GetHandle( extensionMgr, "Type1" ) );
			nType1 = (ExtensionStruct0*)ExtensionManager_HashGet( extensionMgr, n, Type1 );
			nType1->type = 13;
			
			nTemp1 = (ExtensionStruct1*)ExtensionManager_Get( extensionMgr, n, ExtensionManager_GetHandle( extensionMgr, "Temp1" ) );
			nTemp1 = (ExtensionStruct1*)ExtensionManager_HashGet( extensionMgr, n, Temp1 );
			nTemp1->temp = 14.0f;
			
			nPres1 = (ExtensionStruct2*)ExtensionManager_Get( extensionMgr, n, ExtensionManager_GetHandle( extensionMgr, "Pres1" ) );
			nPres1 = (ExtensionStruct2*)ExtensionManager_HashGet( extensionMgr, n, Pres1 );
			nPres1->pres = 15.0f;
			nPres1->flag = 16;
			
			nBC_Set1 = (ExtensionStruct3*)ExtensionManager_Get( extensionMgr, n, ExtensionManager_GetHandle( extensionMgr, "BC_Set1" ) );
			nBC_Set1 = (ExtensionStruct3*)ExtensionManager_HashGet( extensionMgr, n, BC_Set1 );
			nBC_Set1->dd = 17;
			nBC_Set1->cc = 18;
			nBC_Set1->bc = 19;
			
			nWeight1 = (ExtensionStruct4*)ExtensionManager_Get( extensionMgr, n, ExtensionManager_GetHandle( extensionMgr, "Weight1" ) );
			nWeight1 = (ExtensionStruct4*)ExtensionManager_HashGet( extensionMgr, n, Weight1 );
			nWeight1->wf = 20.0f;

			Journal_Printf( (void*) stream, "\'n(BaseClass)\'->x: %g\n", n->x );
			Journal_Printf( (void*) stream, "\'n(BaseClass)\'->y: %g\n", n->y );
			Journal_Printf( (void*) stream, "\'n(BaseClass)\'->z: %g\n", n->z );
			Journal_Printf( (void*) stream, "\'n(BaseClass)\'->dim: %i\n", n->dim );
			Journal_Printf( (void*) stream, "\'n(Type0)\'->type: %i\n", nType0->type );
			Journal_Printf( (void*) stream, "\'n(Temp0)\'->temp: %g\n", nTemp0->temp );
			Journal_Printf( (void*) stream, "\'n(Pres0)\'->pres: %g\n", nPres0->pres );
			Journal_Printf( (void*) stream, "\'n(Pres0)\'->flag: %i\n", nPres0->flag );
			Journal_Printf( (void*) stream, "\'n(BC_Set0)\'->dd: %u\n", nBC_Set0->dd );
			Journal_Printf( (void*) stream, "\'n(BC_Set0)\'->cc: %u\n", nBC_Set0->cc );
			Journal_Printf( (void*) stream, "\'n(BC_Set0)\'->bc: %u\n", nBC_Set0->bc );
			Journal_Printf( (void*) stream, "\'n(Weight0)\'->wf: %g\n", nWeight0->wf );
			Journal_Printf( (void*) stream, "\'n(Type1)\'->type: %i\n", nType1->type );
			Journal_Printf( (void*) stream, "\'n(Temp1)\'->temp: %g\n", nTemp1->temp );
			Journal_Printf( (void*) stream, "\'n(Pres1)\'->pres: %g\n", nPres1->pres );
			Journal_Printf( (void*) stream, "\'n(Pres1)\'->flag: %i\n", nPres1->flag );
			Journal_Printf( (void*) stream, "\'n(BC_Set1)\'->dd: %u\n", nBC_Set1->dd );
			Journal_Printf( (void*) stream, "\'n(BC_Set1)\'->cc: %u\n", nBC_Set1->cc );
			Journal_Printf( (void*) stream, "\'n(BC_Set1)\'->bc: %u\n", nBC_Set1->bc );
			Journal_Printf( (void*) stream, "\'n(Weight1)\'->wf: %g\n", nWeight1->wf );
		}
		RegressionTest_Check(
			1,
			stream,
			"Write/Read",
			"Can we write to the extensions" );
		
		/* Stg_Class_Delete stuff */
		ExtensionManager_Free( extensionMgr, nArray );
		Stg_Class_Delete( extensionMgr );

		RegressionTest_Check(
			1,
			stream,
			"Delete",
			"Can we delete the extensions" );
	}

	RegressionTest_Finalise();
	
	BaseExtensibility_Finalise();
	BaseAutomation_Finalise();
	BaseContainer_Finalise();
	BaseIO_Finalise();
	BaseFoundation_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0; /* success */
}

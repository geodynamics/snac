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
** $Id: testVariable.c 2933 2005-05-12 06:13:10Z SteveQuenette $
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

#define VECTOR_DATA_COUNT (3)
typedef double Triple[VECTOR_DATA_COUNT];

struct ComplexStuff{
	int x;
	float y;
	char z; /* variablise the y and z member */
};
typedef struct ComplexStuff ComplexStuff;

#define __MockContext \
	__Stg_Class \
	float* scalars; \
	Index scalarCount; \
	Triple* vectors; \
	Index vectorCount; \
	ComplexStuff* stuff; \
	Index stuffCount; \
	SizeT complexStuffSize; \
	Variable_Register* vr;

struct MockContext { __MockContext };
typedef struct MockContext MockContext;

void _MockContext_Delete( void* mock );
void _MockContext_Print( void* mock, Stream* stream );
void* _MockContext_Copy( void* mock, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );

MockContext* MockContext_New( Index scalarCount, Index vectorCount, Index stuffCount ) {
	MockContext* self;
	
	self = (MockContext*)_Stg_Class_New(
		sizeof( MockContext ),
		"MockContext",
		_MockContext_Delete,
		_MockContext_Print,
		_MockContext_Copy );

	self->scalarCount = scalarCount;
	self->scalars = Memory_Alloc_Array( float, self->scalarCount, "scalars" );

	self->vectorCount = vectorCount;
	self->vectors = Memory_Alloc_Array( Triple, self->vectorCount, "vectors" );

	self->stuffCount = stuffCount;
	self->stuff = Memory_Alloc_Array( ComplexStuff, self->stuffCount, "stuff" );

	self->complexStuffSize = sizeof( ComplexStuff );
	
	self->vr = Variable_Register_New();

	Variable_NewScalar(
		"Scalar",
		Variable_DataType_Float,
		&(self->scalarCount),
		(void**)&(self->scalars),
		self->vr );

	Variable_NewVector(
		"Vector",
		Variable_DataType_Double,
		VECTOR_DATA_COUNT,
		&(self->vectorCount),
		(void**)&(self->vectors),
		self->vr,
		"x",
		"y",
		"z" );
	{
		ComplexStuff tmp;
		SizeT dataOffsets[] = { 0, 0 };
		Variable_DataType dataTypes[] = { Variable_DataType_Float, Variable_DataType_Char };
		Index dataTypeCounts[] = { 1, 1 };
		Name dataNames[] = { "complexY", "complexZ" };
		
		dataOffsets[0] = (ArithPointer)&tmp.y - (ArithPointer)&tmp;
		dataOffsets[1] = (ArithPointer)&tmp.z - (ArithPointer)&tmp;

		Variable_New(
			"Complex",
			2,
			dataOffsets,
			dataTypes,
			dataTypeCounts,
			dataNames,
			&(self->complexStuffSize),
			&(self->stuffCount),
			(void**)&(self->stuff),
			self->vr );
	}

	Variable_Register_BuildAll( self->vr );

	return self;
}

void _MockContext_Delete( void* mock ) {
	MockContext* self = (MockContext*)mock;

	Index var_I;

	for ( var_I = 0; var_I < self->vr->count; ++var_I ) {
		Stg_Class_Delete( self->vr->_variable[var_I] );
	}

	Memory_Free( self->scalars );
	Memory_Free( self->vectors );
	Memory_Free( self->stuff );

	Stg_Class_Delete( self->vr );

	_Stg_Class_Delete( self );
}
void _MockContext_Print( void* mock, Stream* stream ) {
	MockContext* self = (MockContext*)mock;
	int ii;
	Index var_I;

	Variable* scalar;
	Variable* vector;
	Variable* complexStuff;

	scalar = Variable_Register_GetByName( self->vr, "Scalar" );
	vector = Variable_Register_GetByName( self->vr, "Vector" );
	complexStuff = Variable_Register_GetByName( self->vr, "Complex" );
	
	_Stg_Class_Print( self, stream );
	Journal_Printf( stream, "self->scalars (ptr): %p\n", self->scalars );
	for ( ii = 0; ii < self->scalarCount; ++ii ) {
		Journal_Printf( stream, "%f\n", self->scalars[ii] );
	}
	
	Journal_Printf( stream, "self->vectors (ptr): %p\n", self->vectors );
	for ( ii = 0; ii < self->vectorCount; ++ii ) {
		Journal_Printf( 
			stream, 
			"%f %f %f\n", 
			self->vectors[ii][0], 
			self->vectors[ii][1],
			self->vectors[ii][2] );
	}
	
	Journal_Printf( stream, "self->stuff (ptr): %p\n", self->stuff );
	for ( ii = 0; ii < self->stuffCount; ++ii ) {
		Journal_Printf( stream, "%f %c\n", self->stuff[ii].y, self->stuff[ii].z );
	}
	

	for ( var_I = 0; var_I < self->vr->count; ++var_I ) {
		Journal_Printf( 
			stream, 
			"Variable %s: arrayptr (ptr) %p\n", 
			Variable_Register_GetByIndex( self->vr, var_I )->name,
			Variable_Register_GetByIndex( self->vr, var_I )->arrayPtr );
	}

	for ( ii = 0; ii < self->scalarCount; ++ii ) {
		Journal_Printf( stream, "scalar[%d]: %f\n", ii, Variable_GetValueFloat( scalar, ii ) );
	}
	for ( ii = 0; ii < self->vectorCount; ++ii ) {
		Journal_Printf( 
			stream, 
			"vector[%d]: %f %f %f\n", 
			ii, 
			Variable_GetValueAtDouble( vector, ii, 0 ),
			Variable_GetValueAtDouble( vector, ii, 1 ),
			Variable_GetValueAtDouble( vector, ii, 2 ) );
	}
	for ( ii = 0; ii < self->stuffCount; ++ii ) {
		ComplexStuff* stuff = Variable_GetStructPtr( complexStuff, ii );
		Journal_Printf(
			stream,
			"complex[%d]: %f %c\n",
			ii,
			stuff->y,
			stuff->z );
	}
}
void* _MockContext_Copy( void* mock, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	MockContext* self = (MockContext*)mock;
	MockContext* newContext;

	newContext = (MockContext*)_Stg_Class_Copy( self, dest, deep, nameExt, ptrMap );
	PtrMap_Append( ptrMap, self, newContext );

	newContext->scalarCount = self->scalarCount;
	newContext->vectorCount = self->vectorCount;
	newContext->stuffCount = self->stuffCount;
	newContext->complexStuffSize = self->complexStuffSize;

	/* Indicate the area of memory which is given for data so that Variables can attach to it */
	PtrMap_Append( ptrMap, &(self->scalars), &(newContext->scalars) );
	PtrMap_Append( ptrMap, &(self->vectors), &(newContext->vectors) );
	PtrMap_Append( ptrMap, &(self->stuff), &(newContext->stuff) );
	
	PtrMap_Append( ptrMap, &(self->scalarCount), &(newContext->scalarCount) );
	PtrMap_Append( ptrMap, &(self->vectorCount), &(newContext->vectorCount) );
	PtrMap_Append( ptrMap, &(self->stuffCount), &(newContext->stuffCount) );
	PtrMap_Append( ptrMap, &(self->complexStuffSize), &(newContext->complexStuffSize) );


	newContext->scalars = Memory_Alloc_Array( float, self->scalarCount, "scalars" );
	PtrMap_Append( ptrMap, self->scalars, newContext->scalars );
	/* usually copy contents but let variables do it */
	
	newContext->vectors = Memory_Alloc_Array( Triple, self->vectorCount, "vectors" );
	PtrMap_Append( ptrMap, self->vectors, newContext->vectors );
	/* usually copy contents but let variables do it */

	newContext->stuff = Memory_Alloc_Array( ComplexStuff, self->stuffCount, "stuff" );
	PtrMap_Append( ptrMap, self->stuff, newContext->stuff );
	/* usually copy contents but let variables do it */
	

	newContext->vr = Stg_Class_Copy( self->vr, NULL, deep, nameExt, ptrMap );

	return newContext;
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

	RegressionTest_Init( "Base/Automation/Variable" );

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
		MockContext* context;
		MockContext* contextCopy;
		int ii;

		Variable* scalar;
		Variable* vector;
		Variable* complexStuff;

		context = MockContext_New( 10, 10, 10 );
		
		scalar = Variable_Register_GetByName( context->vr, "Scalar" );
		vector = Variable_Register_GetByName( context->vr, "Vector" );
		complexStuff = Variable_Register_GetByName( context->vr, "Complex" );
	
		RegressionTest_Check( 
			scalar != NULL &&
			vector != NULL &&
			complexStuff != NULL,
			stream,
			"Variable creation",
			"Can we make the test variables properly" );
			
		for ( ii = 0; ii < context->scalarCount; ++ii ) {
			Variable_SetValueFloat( scalar, ii, (float)ii );
		}
		for ( ii = 0; ii < context->vectorCount; ++ii ) {
			Variable_SetValueAtDouble( vector, ii, 0, (double)ii );
			Variable_SetValueAtDouble( vector, ii, 1, (double)ii );
			Variable_SetValueAtDouble( vector, ii, 2, (double)ii );
		}
		for ( ii = 0; ii < context->stuffCount; ++ii ) {
			ComplexStuff* stuff = Variable_GetStructPtr( complexStuff, ii );
			stuff->y = (float)ii;
			stuff->z = '0' + ii;
		}
		Print( context, stream );
	
		/* No name extension! otherwise Variable_Register_GetByName() will fail */
		contextCopy = (MockContext*)Stg_Class_Copy( context, NULL, True, NULL, NULL );

		RegressionTest_Check(
			contextCopy != NULL,
			stream,
			"Variable Copying",
			"Can the variable register and variables be copied" );

		Print( contextCopy, stream );

		Stg_Class_Delete( context );
		Stg_Class_Delete( contextCopy );
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

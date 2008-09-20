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
** $Id: Variable.c 4051 2007-03-26 10:36:19Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdarg.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"

#include "types.h"
#include "shortcuts.h"
#include "Stg_Component.h"
#include "Stg_ComponentFactory.h"
#include "LiveComponentRegister.h"
#include "Variable.h"
#include "Variable_Register.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <math.h>

/* Textual name of this class */
const Type Variable_Type = "Variable";


/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

Variable* Variable_New(
		Name						name,
		Index						dataCount,
		SizeT*						dataOffsets,
		Variable_DataType*				dataTypes,
		Index*						dataTypeCounts,
		Name*						dataNames,
		SizeT*						structSizePtr,
		Index*						arraySizePtr,
		void**						arrayPtrPtr,
		Variable_Register*				vr )
{
	Variable*	self;
	
	self = _Variable_New( 
		sizeof(Variable), 
		Variable_Type, 
		_Variable_Delete, 
		_Variable_Print, 
		_Variable_Copy,
		(Stg_Component_DefaultConstructorFunction*)Variable_DefaultNew,
		_Variable_Construct,
		_Variable_Build, 
		_Variable_Initialise, 
		_Variable_Execute, 
		_Variable_Destroy,
		name,
		True,
		dataCount, 
		dataOffsets, 
		dataTypes, 
		dataTypeCounts, 
		dataNames, 
		structSizePtr, 
		arraySizePtr,
		arrayPtrPtr, 
		vr );
	
	return self;
}

Variable* Variable_DefaultNew( Name name )
{
	Variable*	self;
	
	self = _Variable_New( 
		sizeof(Variable), 
		Variable_Type, 
		_Variable_Delete, 
		_Variable_Print, 
		_Variable_Copy,
		(Stg_Component_DefaultConstructorFunction*)Variable_DefaultNew,
		_Variable_Construct,
		_Variable_Build, 
		_Variable_Initialise, 
		_Variable_Execute, 
		_Variable_Destroy,
		name,
		False,
		0, 
		NULL, 
		NULL, 
		NULL, 
		NULL, 
		NULL, 
		NULL,
		NULL, 
		NULL );
	
	return self;
}

Variable* Variable_NewScalar( 
		Name						name,
		Variable_DataType				dataType,
		Index*						arraySizePtr,
		void**						arrayPtrPtr,
		Variable_Register*				vr )
{
	Variable*		self;
	SizeT			dataOffsets[] = { 0 };
	Variable_DataType	dataTypes[] = { 0 };		/* Init value later */
	Index			dataTypeCounts[] = { 1 };
	
	dataTypes[0] = dataType;
	
	self = _Variable_New( 
		sizeof(Variable), 
		Variable_Type, 
		_Variable_Delete, 
		_Variable_Print, 
		_Variable_Copy,
		(Stg_Component_DefaultConstructorFunction*)Variable_DefaultNew,
		_Variable_Construct,
		_Variable_Build, 
		_Variable_Initialise, 
		_Variable_Execute,
		_Variable_Destroy,
		name,
		True,
		1, 
		dataOffsets, 
		dataTypes, 
		dataTypeCounts, 
		0, /* no component names */
		0, /* means work out from dataType at build phase */
		arraySizePtr,
		arrayPtrPtr, 
		vr );
	
	return self;
}

Variable* Variable_NewVector( 
		Name						name,
		Variable_DataType				dataType,
		Index						dataTypeCount,
		Index*						arraySizePtr,
		void**						arrayPtrPtr,
		Variable_Register*				vr,
		... 						/* vector component names */ )
{
	Variable*		self;
	SizeT			dataOffsets[] = { 0 };
	Variable_DataType	dataTypes[] = { 0 };				/* Init later... */
	Index			dataTypeCounts[] = { 0 };			/* Init later... */
	Name*			dataNames;
	Index			vector_I;
	va_list			ap;

	dataTypes[0] = dataType;
	dataTypeCounts[0] = dataTypeCount;

	dataNames = Memory_Alloc_Array( Name, dataTypeCount, "dataNames" );

	va_start( ap, vr );
	for( vector_I = 0; vector_I < dataTypeCount; vector_I++ ) {
		dataNames[vector_I] = va_arg( ap, Name );
	}
	va_end( ap );
	
	self = _Variable_New( 
		sizeof(Variable), 
		Variable_Type, 
		_Variable_Delete, 
		_Variable_Print, 
		_Variable_Copy, 
		(Stg_Component_DefaultConstructorFunction*)Variable_DefaultNew,
		_Variable_Construct,
		_Variable_Build, 
		_Variable_Initialise, 
		_Variable_Execute, 
		_Variable_Destroy,
		name,
		True,
		1, 
		dataOffsets, 
		dataTypes, 
		dataTypeCounts, 
		dataNames,
		0, /* means work out from dataType at build phase */
		arraySizePtr,
		arrayPtrPtr, 
		vr );

	Memory_Free( dataNames );
	
	return self;
}


void Variable_Init(
		Name						name,
		Variable*					self,
		Index						dataCount,
		SizeT*						dataOffsets,
		Variable_DataType*				dataTypes,
		Index*						dataTypeCounts,
		Name*						dataNames,
		SizeT*						structSizePtr,
		Index*						arraySizePtr,
		void**						arrayPtrPtr,
		Bool						allocateSelf,
		Variable_Register*				vr )
{
	/* General info */
	self->type = Variable_Type;
	self->_sizeOfSelf = sizeof(Variable);
	self->_deleteSelf = False;
		
	/* Virtual info */
	self->_delete = _Variable_Delete;
	self->_print = _Variable_Print;
	self->_copy = _Variable_Copy;
	self->_build = _Variable_Build;
	self->_initialise = _Variable_Initialise;
	self->_execute = _Variable_Execute;
	
	_Stg_Class_Init( (Stg_Class*)self );
	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );
	_Stg_Component_Init( (Stg_Component*)self );
	/* Variable info */
	_Variable_Init( 
		self, 
		dataCount, 
		dataOffsets, 
		dataTypes, 
		dataTypeCounts, 
		dataNames, 
		structSizePtr,
		arraySizePtr,
		arrayPtrPtr, 
		allocateSelf,
		vr );
}


Variable* _Variable_New(
		SizeT						_sizeOfSelf, 
		Type						type,
		Stg_Class_DeleteFunction*				_delete,
		Stg_Class_PrintFunction*				_print, 
		Stg_Class_CopyFunction*				_copy, 
		Stg_Component_DefaultConstructorFunction*	_defaultConstructor,
		Stg_Component_ConstructFunction*		_construct,
		Stg_Component_BuildFunction*			_build,
		Stg_Component_InitialiseFunction*			_initialise,
		Stg_Component_ExecuteFunction*			_execute,
		Stg_Component_DestroyFunction*		_destroy,
		Name						name,
		Bool						initFlag,
		Index						dataCount,
		SizeT*						dataOffsets,
		Variable_DataType*				dataTypes,
		Index*						dataTypeCounts,
		Name*						dataNames,
		SizeT*						structSizePtr,
		Index*						arraySizePtr,
		void**						arrayPtrPtr,
		Variable_Register*				vr )
{
	Variable*	self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(Variable) );
	self = (Variable*)_Stg_Component_New( _sizeOfSelf, type, _delete, _print, _copy, _defaultConstructor,
										_construct, _build, _initialise, _execute, _destroy, name, NON_GLOBAL );
	
	/* General info */
	
	/* Virtual info */
	self->_build = _build;
	self->_initialise = _initialise;
	
	/* Variable info */
	if( initFlag ){
		_Variable_Init( 
			self, 
			dataCount, 
			dataOffsets, 
			dataTypes, 
			dataTypeCounts, 
			dataNames, 
			structSizePtr, 
			arraySizePtr,
			arrayPtrPtr, 
			False,
			vr );
	}
	
	return self;
}

void _Variable_Init(
		Variable*					self, 
		Index						dataCount,
		SizeT*						dataOffsets,
		Variable_DataType*				dataTypes,
		Index*						dataTypeCounts,
		Name*						dataNames,
		SizeT*						structSizePtr,
		Index*						arraySizePtr,
		void**						arrayPtrPtr,
		Bool                                            allocateSelf,
		Variable_Register*				vr )
{
	/* General and Virtual info should already be set */
	
	/* Variable info */
	self->isConstructed = True;
	self->allocateSelf  = allocateSelf;
	self->offsetCount = dataCount;
	self->structSizePtr = structSizePtr;
	self->arraySizePtr = arraySizePtr;
	self->arrayPtrPtr = arrayPtrPtr;

	/* Use of this class has increased... can't assume the info arrays are on persistant memory... copy by default. They will
	   be deleted. */
	Journal_Firewall( dataOffsets ? True : False, Journal_Register( Error_Type, Variable_Type ), "dataOffsets is null\n" );
	self->offsets = Memory_Alloc_Array( SizeT, self->offsetCount, "Variable->offsets" );
	memcpy( self->offsets, dataOffsets, sizeof(SizeT) * self->offsetCount );
	
	Journal_Firewall( dataTypes ? True : False, Journal_Register( Error_Type, Variable_Type ), "dataTypes is null\n" );
	self->dataTypes = Memory_Alloc_Array( Variable_DataType, self->offsetCount, "Variable->dataTypes" );
	memcpy( self->dataTypes, dataTypes, sizeof(Variable_DataType) * self->offsetCount );
	
	Journal_Firewall( 
		dataTypeCounts ? True : False , 
		Journal_Register( Error_Type, Variable_Type ), 
		"dataTypeCounts is null\n" );
	self->dataTypeCounts = Memory_Alloc_Array( Index, self->offsetCount, "Variable->dataTypeCounts" );
	memcpy( self->dataTypeCounts, dataTypeCounts, sizeof(Index) * self->offsetCount );
	
	/* These get set at the build phase (when we assume the variable's data memory has been allocated) */
	self->structSize = 0;
	self->arrayPtr = 0;
	self->dataSizes = 0;
	
	if ( dataNames && self->offsetCount == 1 && self->dataTypeCounts[0] > 1 ) {
		/* Vector case */
		self->subVariablesCount = self->dataTypeCounts[0];
	}
	else {
		/* Scalar or Complex case */
		self->subVariablesCount = self->offsetCount;
	}
	self->components = Memory_Alloc_Array( Variable*, self->subVariablesCount, "Variable->components" );
	memset( self->components, 0, sizeof(Variable*) * self->subVariablesCount );
	
	/* If the variable register is provided, add this and component variable(s) to the register */
	if( vr ) {
		self->vr = vr;
		Variable_Register_Add( vr, self );
		
		/* If we have component names, create the associated variables. Don't do if there is only one component. */
		if( dataNames && self->offsetCount > 1 ) {
			Index			component_I;
			
			for( component_I = 0; component_I < self->offsetCount; component_I++ ) {
				if( dataNames[component_I] ) {
					SizeT			componentOffsets[] = { 0 };
					Variable_DataType	componentTypes[] = { 0 };
					Index			componentTypeCounts[] = { 0 };

					componentOffsets[0] = self->offsets[component_I];
					componentTypes[0] = self->dataTypes[component_I];
					componentTypeCounts[0] = self->dataTypeCounts[component_I];
					
					/* Assumption: components are scalar or vector, but cannot be complex */
					self->components[component_I] = Variable_New( 
						dataNames[component_I], 
						1, 
						componentOffsets, 
						componentTypes, 
						componentTypeCounts,
						0,
						self->structSizePtr,
						self->arraySizePtr,
						self->arrayPtrPtr,
						vr );
				}
			}
		}
		/* Else if we have vector-component names, create the associated variables. Do only if non-complex and a vector. */
		else if( dataNames && self->offsetCount == 1 && self->dataTypeCounts[0] > 1 ) {
			Index			vector_I;

			for( vector_I = 0; vector_I < self->dataTypeCounts[0]; vector_I++ ) {
				if( dataNames[vector_I] ) {
					/* Unfortunately we cannot call any of our fancy macros here as the array is not resolved
					 * yet. As a consequence we have to manually work out the vector's indecis offsets. Ouch
					 * only from a code-maintenance point of view. */
					SizeT			componentOffsets[] = { 0 };		/* Init later... */
					Variable_DataType	componentTypes[] = { 0 };		/* Init later... */
					Index			componentTypeCounts[] = { 1 };
				
					componentOffsets[0] = 
						(ArithPointer)self->offsets[0] + 
						( self->dataTypes[0] == Variable_DataType_Char ? ( sizeof(char) * vector_I ) :
						  self->dataTypes[0] == Variable_DataType_Short ? ( sizeof(short) * vector_I ) :
						  self->dataTypes[0] == Variable_DataType_Int ? ( sizeof(int) * vector_I ) :
						  self->dataTypes[0] == Variable_DataType_Float ? ( sizeof(float) * vector_I ) :
						  self->dataTypes[0] == Variable_DataType_Double ? ( sizeof(double) * vector_I ) :
						  self->dataTypes[0] == Variable_DataType_Pointer ? ( sizeof(void*) * vector_I ) :
						  Journal_Firewall(
							0,
							Journal_Register( Error_Type, Variable_Type ),
							"Vector is of a non-builtin type\n" ) );
					componentTypes[0] = self->dataTypes[0];

							
					/* Assumption: vector-components are scalar, but cannot be complex */
					self->components[vector_I] = Variable_New( 
						dataNames[vector_I],
						1, 
						componentOffsets, 
						componentTypes, 
						componentTypeCounts,
						0,
						self->structSizePtr,
						self->arraySizePtr,
						self->arrayPtrPtr,
						vr );
				}
			}
		}
	}
	else {
		self->vr = NULL;
	}
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _Variable_Delete( void* variable ) {
	Variable*	self = (Variable*)variable;

	if ( self->dataSizes ) {
		Memory_Free( self->dataSizes );
	}
	if (self->allocateSelf) {
		Memory_Free( self->arrayPtr );
	}
	
	Memory_Free( self->dataTypeCounts );
	Memory_Free( self->dataTypes );
	Memory_Free( self->offsets );
	Memory_Free( self->components );
	
	/* Stg_Class_Delete parent */
	_Stg_Component_Delete( self );
}


void _Variable_Print( void* variable, Stream* stream ) {
	Variable*	self = (Variable*)variable;
	Index		component_I;
	
	/* Print parent */
	_Stg_Component_Print( self, stream );
	
	/* General info */
	Journal_Printf( stream, "Variable (ptr): (%p)\n", self );
	
	/* Virtual info */
	
	/* Variable info */
	Journal_Printf( stream, "\toffsetCount: %u\n", self->offsetCount );
	
	Journal_Printf( stream, "\toffsets (ptr): %p\n", self->offsets );
	for( component_I = 0; component_I < self->offsetCount; component_I++ ) {
		Journal_Printf( stream, "\t\toffsets[%u]: %lu\n", component_I, self->offsets[component_I] );
	}
	
	Journal_Printf( stream, "\tdataTypes (ptr): %p\n", self->dataTypes );
	for( component_I = 0; component_I < self->offsetCount; component_I++ ) {
		Journal_Printf( stream, "\t\tdataTypes[%u]: %lu\n", component_I, self->dataTypes[component_I] );
	}
	
	Journal_Printf( stream, "\tdataTypeCounts (ptr): %p\n", self->dataTypeCounts );
	for( component_I = 0; component_I < self->offsetCount; component_I++ ) {
		Journal_Printf( stream, "\t\tdataTypeCounts[%u]: %lu\n", component_I, self->dataTypeCounts[component_I] );
	}

	Journal_Printf( stream, "\tdataSizes (ptr): %p\n", self->dataSizes );
	for( component_I = 0; component_I < self->offsetCount && self->dataSizes; component_I++ ) {
		Journal_Printf( stream, "\t\tdataSizes[%u]: %lu\n", component_I, self->dataSizes[component_I] );
	}

	Journal_Printf( stream, "\tcomponents (ptr): %p\n", self->components );
	for( component_I = 0; component_I < self->offsetCount; component_I++ ) {
		Journal_Printf( stream, "\t\tcomponents[%u] (ptr): %p\n", component_I, self->components[component_I] );
		if( self->components[component_I] ) {
			Journal_Printf( stream, "\t\tcomponents[%u]: %s\n", component_I, self->components[component_I]->name );
		}
	}

	Journal_Printf( stream, "\tstructSize: %lu\n", self->structSize );
	Journal_Printf( stream, "\tstructSizePtr (ptr): %p\n", self->structSizePtr );

	Journal_Printf( stream, "\tarrayPtr (ptr): %p\n", self->arrayPtr );
	Journal_Printf( stream, "\tarrayPtrPtr (ptr): %p\n", self->arrayPtrPtr );

	Journal_Printf( stream, "\tarraySize: %lu\n", self->arraySize );
	Journal_Printf( stream, "\tarraySizePtr (ptr): %p\n", self->arraySizePtr );

	Journal_Printf( stream, "\tallocateSelf = %s\n", self->allocateSelf ? "True" : "False" );
}


void* _Variable_Copy( void* variable, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	Variable*	self = (Variable*)variable;
	Variable*	newVariable;
	PtrMap*		map = ptrMap;
	
	newVariable = (Variable*)_Stg_Component_Copy( self, dest, deep, nameExt, map );
	PtrMap_Append( map, self, newVariable );
	
	/* virtual methods */
	newVariable->offsetCount = self->offsetCount;
	newVariable->structSize = self->structSize;
	newVariable->arraySize = self->arraySize;
	
	newVariable->allocateSelf = self->allocateSelf; /* This may change depending on whether arrayPtr is found in map */

	/* single valued members */
	
	if( deep ) {
		if( (newVariable->offsets = PtrMap_Find( map, self->offsets )) == NULL && self->offsets != NULL ) {
			newVariable->offsets = Memory_Alloc_Array( SizeT, newVariable->offsetCount, "Variable->offsets" );
			memcpy( newVariable->offsets, self->offsets, sizeof(SizeT) * newVariable->offsetCount );
			PtrMap_Append( map, self->offsets, newVariable->offsets );
		}
		
		if( (newVariable->dataTypes = PtrMap_Find( map, self->dataTypes )) == NULL && self->offsets != NULL ) {
			newVariable->dataTypes = Memory_Alloc_Array( Variable_DataType, newVariable->offsetCount, "Variable->dataTypes" );
			memcpy( newVariable->dataTypes, self->dataTypes, sizeof(Variable_DataType) * newVariable->offsetCount );
			PtrMap_Append( map, self->dataTypes, newVariable->dataTypes );
		}
		
		if( (newVariable->dataTypeCounts = PtrMap_Find( map, self->dataTypeCounts )) == NULL && self->dataTypeCounts != NULL ) {
			newVariable->dataTypeCounts = Memory_Alloc_Array( Index, newVariable->offsetCount, "Variable->dataTypeCounts" );
			memcpy( newVariable->dataTypeCounts, self->dataTypeCounts, sizeof(Index) * newVariable->offsetCount );
			PtrMap_Append( map, self->dataTypeCounts, newVariable->dataTypeCounts );
		}
		
		if ( self->structSizePtr != NULL ) {
			if( (newVariable->structSizePtr = PtrMap_Find( map, self->structSizePtr )) == NULL ) {
				newVariable->structSizePtr = Memory_Alloc_Array( 
					SizeT, 
					1, 
					"Variable->structSizePtr" );
				memcpy( newVariable->structSizePtr, self->structSizePtr, sizeof(SizeT) );
				PtrMap_Append( map, self->structSizePtr, newVariable->structSizePtr );
			}
		}
		else {
			newVariable->structSizePtr = NULL;
		}
		
		if ( self->arraySizePtr != NULL ) {
			if( (newVariable->arraySizePtr = PtrMap_Find( map, self->arraySizePtr )) == NULL ) {
				newVariable->arraySizePtr = Memory_Alloc_Array(
					Index,
					1,
					"Variable->arraySizePtr" );
				memcpy( newVariable->arraySizePtr, self->arraySizePtr, sizeof(Index) );
				PtrMap_Append( map, self->arraySizePtr, newVariable->arraySizePtr );
			}
		}
		else {
			newVariable->arraySizePtr = NULL;
		}
		
		if( (newVariable->dataSizes = PtrMap_Find( map, self->dataSizes )) == NULL && self->dataSizes != NULL ) {
			newVariable->dataSizes = Memory_Alloc_Array( SizeT, newVariable->offsetCount, "Variable->dataSizes" );
			memcpy( newVariable->dataSizes, self->dataSizes, sizeof(SizeT) * newVariable->offsetCount );
			PtrMap_Append( map, self->dataSizes, newVariable->dataSizes );
		}

		if( (newVariable->arrayPtrPtr = PtrMap_Find( map, self->arrayPtrPtr )) == NULL && self->arrayPtrPtr != NULL ) {	
			if( (newVariable->arrayPtr = PtrMap_Find( map, self->arrayPtr )) == NULL && self->arrayPtr != NULL ) {
				Index memoryToAllocSize = self->arraySize * self->structSize;
				newVariable->arrayPtr = Memory_Alloc_Bytes( 
					memoryToAllocSize,
					Variable_Type,
					"SelfAllocedArray" );
				memcpy( newVariable->arrayPtr, self->arrayPtr, memoryToAllocSize );
				PtrMap_Append( map, self->arrayPtr, newVariable->arrayPtr );

				newVariable->arrayPtrPtr = &(newVariable->arrayPtr);
				newVariable->allocateSelf = True;
			}
		}
		else {
			newVariable->arrayPtr = *newVariable->arrayPtrPtr;
			memcpy( newVariable->arrayPtr, self->arrayPtr, self->arraySize * self->structSize );
		}
		
		if( (newVariable->components = PtrMap_Find( map, self->components )) == NULL && self->components != NULL ) {
			Index	comp_I;
			
			if( newVariable->offsetCount == 1 && newVariable->dataTypeCounts[0] > 1 ) {
				/* Vector case */
				newVariable->components = Memory_Alloc_Array( 
					Variable*, 
					newVariable->dataTypeCounts[0], 
					"Variable->components" );
				memset( newVariable->components, 0, sizeof(Variable*) * newVariable->dataTypeCounts[0] );
				
				for( comp_I = 0; comp_I < newVariable->dataTypeCounts[0]; comp_I++ ) {
					newVariable->components[comp_I] = (Variable*)Stg_Class_Copy( 
						self->components[comp_I], 
						NULL, 
						deep, 
						nameExt, 
						map );
				}
			}
			else {
				/* Scalar or Complex case */
				newVariable->components = Memory_Alloc_Array( 
					Variable*, 
					newVariable->offsetCount, 
					"Variable->components" );
				memset( newVariable->components, 0, sizeof(Variable*) * newVariable->offsetCount );
				
				for( comp_I = 0; comp_I < newVariable->offsetCount; comp_I++ ) {
					if ( self->components[comp_I] ) {
						newVariable->components[comp_I] = (Variable*)Stg_Class_Copy( 
							self->components[comp_I], 
							NULL, 
							deep, 
							nameExt, 
							map );
					}
				}
			}
			
			PtrMap_Append( map, self->components, newVariable->components );
		}

		if ( self->vr ) {
			newVariable->vr = Stg_Class_Copy( self->vr, NULL, deep, nameExt, map );
		}
	}
	else {
		fprintf( stderr, "Warning: please don't shallow copy variables yet...or ever!\n" );
	}
	
	return (void*)newVariable;
}


void _Variable_Build( void* variable, void* data ) {
	Variable*	self = (Variable*)variable;
	Index 		component_I;
	Index 		subVariable_I;
	
	/* Obtain the actual array size, and array pointer */
	Journal_Firewall( 
		self->arraySizePtr ? True : False, 
		Journal_Register( Error_Type, Variable_Type ), 
		"arraySizePtr is null\n" );
	self->arraySize = *self->arraySizePtr;


	/* Work out the actual data sizes from the data types */
	if( self->dataSizes ) {
		Memory_Free( self->dataSizes );
	}
	self->dataSizes = Memory_Alloc_Array( SizeT, self->offsetCount, "Variable->dataSizes" );
	for( component_I = 0; component_I < self->offsetCount; component_I++ ) {
		switch ( self->dataTypes[component_I] ) {
			case Variable_DataType_Variable:
				Journal_Firewall( 
					self->dataTypes[component_I] != Variable_DataType_Variable, 
					Journal_Register( Error_Type, Variable_Type ),
					"\"dataType[?] == Variable_DataType_Variables\" not yet implemented\n" );
				break;
			case Variable_DataType_Char:
				self->dataSizes[component_I] = sizeof( char ) * self->dataTypeCounts[component_I];
				break;
			case Variable_DataType_Short:
				self->dataSizes[component_I] = sizeof( short ) * self->dataTypeCounts[component_I];
				break;
			case Variable_DataType_Int:
				self->dataSizes[component_I] = sizeof( int ) * self->dataTypeCounts[component_I];
				break;
			case Variable_DataType_Float:
				self->dataSizes[component_I] = sizeof( float ) * self->dataTypeCounts[component_I];
				break;
			case Variable_DataType_Double:
				self->dataSizes[component_I] = sizeof( double ) * self->dataTypeCounts[component_I];
				break;
			case Variable_DataType_Pointer:
				self->dataSizes[component_I] = sizeof( void* ) * self->dataTypeCounts[component_I];
				break;
			default:
				Journal_Firewall( 
					self->dataTypes[component_I] >= Variable_DataType_Size, 
					Journal_Register( Error_Type, Variable_Type ),
					"\"dataType[?]\" is unrecognised\n" );
				break;
		}
	}		
	
	/* Non-complex variables: Scalar & Vector can workout struct size for themselves.
	 * Now that we know all the data sizes, work out the actual struct size (which may need the data sizes) */
	if( self->offsetCount == 1 && !self->structSizePtr ) {
		/* Vector and Scalars which are not apart of a Vector can work out their own size */
		if ( self->structSize == 0 ) {
			self->structSize = self->dataSizes[0];
		}

		/* If this is a vector, set its sub variables to the same struct size */
		if ( self->dataTypeCounts[0] > 1 ) {
			for ( subVariable_I = 0; subVariable_I < self->dataTypeCounts[0]; ++subVariable_I ) {
				if ( self->components[subVariable_I] ) {
					self->components[subVariable_I]->structSize = self->structSize;
				}
				else
					/* break here because if one subVariable isn't allocated - then the rest shouldn't be */
					break;
			}
		}
	}
	else {
		Journal_Firewall( 
			self->structSizePtr ? True: False, 
			Journal_Register( Error_Type, Variable_Type ), 
			"structSizePtr is null\n" );
		self->structSize = *self->structSizePtr;
	}
	
	if (self->allocateSelf) {
		Index memoryToAllocSize = self->arraySize * self->structSize;
		Index subVariable_I;

		Journal_Firewall( self->offsetCount == 1, 
				Journal_Register( Error_Type, Variable_Type ), 
				"Variable '%s' has 'offsetCount = %u', which should be 1 for self allocation.\n", 
				self->name, self->offsetCount );

		self->arrayPtr = Memory_Alloc_Bytes( memoryToAllocSize, Variable_Type, "SelfAllocedArray" );
		memset( self->arrayPtr, 0, memoryToAllocSize );
		
		/* If this is a vector then set arrayPtr to be the same for each element of the vector */
		if ( self->dataTypeCounts[0] > 1 ) {
			for ( subVariable_I = 0 ; subVariable_I < self->dataTypeCounts[0] ; subVariable_I++ ) {
				if ( self->components[subVariable_I] ) {
					self->components[ subVariable_I ]->arrayPtr = self->arrayPtr;
					self->components[ subVariable_I ]->arrayPtrPtr = &self->arrayPtr;
				}
				else {
					/* break here because if one subVariable isn't allocated - then the rest shouldn't be */
					break;
				}
			}
		}
	}
	else {
		Journal_Firewall( 
			self->arrayPtrPtr ? True : False, 
			Journal_Register( Error_Type, Variable_Type ), 
			"Variable '%s' has NULL arrayPtrPtr\n", self->name );
		self->arrayPtr = *self->arrayPtrPtr;
	}

	/* Lastly, build all components. It is reasonable if the user calls Build on "velocity", then
	 * "velocity-x" etc are automatically build also. */	
	for ( subVariable_I = 0; subVariable_I < self->subVariablesCount; subVariable_I++ ) {
		if ( self->components[subVariable_I] ) {
			Build( self->components[subVariable_I], data, False );
		}
	}
}


void _Variable_Initialise( void* variable, void* data ) {
	Variable*	self = (Variable*)variable;
	Index 		subVariable_I;

	for ( subVariable_I = 0; subVariable_I < self->subVariablesCount; subVariable_I++ ) {
		if ( self->components[subVariable_I] ) {
			Initialise( self->components[subVariable_I], data, False );
		}
	}
}


void _Variable_Execute( void* variable, void* data ) {
}

void _Variable_Construct( void* variable, Stg_ComponentFactory* cf, void* data ) {
	Variable*           self              = (Variable*) variable;
	SizeT			    dataOffsets[]     = { 0 };
	Variable_DataType	dataTypes[]       = { 0 };		/* Init value later */
	Index			    dataTypeCounts[]  = { 1 };
	Dictionary *        componentDict     = NULL;
	Dictionary *        thisComponentDict = NULL;
	Name                dataTypeName      = NULL;
	Name                rankName          = NULL;
	Name                countName         = NULL;
	unsigned int*       count             = NULL;
	void *              variableRegister  = NULL;
	void *              pointerRegister   = NULL;
	Name*               names             = NULL;
	Stream*             error             = Journal_Register( Error_Type, self->type );
	
	assert( self );

	componentDict = cf->componentDict;
	assert( componentDict );
	thisComponentDict = Dictionary_GetDictionary( componentDict, self->name );
	assert( thisComponentDict );
	
	/* Grab Registers */
	variableRegister = Stg_ObjectList_Get( cf->registerRegister, "Variable_Register" );
	assert( variableRegister );
	pointerRegister = Stg_ObjectList_Get( cf->registerRegister, "Pointer_Register" );
	assert( pointerRegister );
	
	Stg_ComponentFactory_ConstructByKey( cf, self->name, "Dependency", Stg_Component, False, data );
			
	/* Get Pointer to number of elements in array */
	countName = Dictionary_GetString( thisComponentDict, "Count" );
	count = Stg_ObjectList_Get( pointerRegister, countName );
	assert( count );
	
	/* Get Type of Variable */
	dataTypeName = Dictionary_GetString( thisComponentDict, "DataType" );
	if ( !strcasecmp( dataTypeName, "Double" ) )
		dataTypes[0] = Variable_DataType_Double;
	else if ( !strcasecmp( dataTypeName, "Float" ) )
		dataTypes[0] = Variable_DataType_Float;
	else if ( !strcasecmp( dataTypeName, "Int" ) )
		dataTypes[0] = Variable_DataType_Int;
	else if ( !strcasecmp( dataTypeName, "Char" ) )
		dataTypes[0] = Variable_DataType_Char;
	else if ( !strcasecmp( dataTypeName, "Short" ) )
		dataTypes[0] = Variable_DataType_Short;
	else 
		Journal_Firewall( False, error, "Variable '%s' cannot understand data type '%s'\n", self->name, dataTypeName );

	/* Get Rank of Variable - i.e. Scalar or Vector */
	rankName = Dictionary_GetString( thisComponentDict, "Rank" );
	if( !strcasecmp( rankName, "Scalar" ) ){
		dataTypeCounts[0] = 1;
	}
	else if ( !strcasecmp( rankName, "Vector" ) ){
		Dictionary_Entry_Value* list;
		Index                   nameCount = 0;

		/* Get Names from list */
		if (( list = Dictionary_Get( thisComponentDict, "names" ) )) {
			Index entry_I;

			nameCount = Dictionary_Entry_Value_GetCount( list );
			names = Memory_Alloc_Array( Name, nameCount, "Variable Names" );

			for ( entry_I = 0 ; entry_I < nameCount ; entry_I++ )
				names[ entry_I ] = Dictionary_Entry_Value_AsString( Dictionary_Entry_Value_GetElement(list, entry_I ) );
		}
		dataTypeCounts[0] = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, "VectorComponentCount", nameCount );

		Journal_Firewall( nameCount >= dataTypeCounts[0], error,
				"Variable '%s' has too few names in list for %d vector components.\n", self->name, dataTypeCounts[0] );
	}
	else
		Journal_Firewall( False, error, "Variable '%s' cannot understand rank '%s'\n", self->name, rankName );

	_Variable_Init( 
			self, 
			1, 
			dataOffsets,
			dataTypes,
			dataTypeCounts, 
			names, 
			0, 
			count, 
			(void**)&self->arrayPtr,
			True,
			variableRegister );

	/* Clean Up */
	if (names)
		Memory_Free(names);
}
	
void _Variable_Destroy( void* variable, void* data )
{
	
}

/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void Variable_SetValue( void* variable, Index array_I, void* value ) {
	Variable*	self = (Variable*)variable;
	ArithPointer*	vPtr;
	Index		component_I;
	
	vPtr = value;
	for( component_I = 0; component_I < self->offsetCount; component_I++ ) {
		memcpy( _Variable_GetPtr( self, array_I, component_I, 0 ), (void*)vPtr, self->dataSizes[component_I] );
		
		vPtr += self->dataSizes[component_I];
	}
}


void Variable_GetValue( void* variable, Index array_I, void* value ) {
	Variable*	self = (Variable*)variable;
	ArithPointer*	vPtr;
	Index		component_I;
	
	vPtr = value;
	for( component_I = 0; component_I < self->offsetCount; component_I++ ) {
		memcpy( (void*)vPtr, _Variable_GetPtr( self, array_I, component_I, 0 ), self->dataSizes[component_I] );
		
		vPtr += self->dataSizes[component_I];
	}
}

void Variable_Update( void* variable ) {
	Variable* self = (Variable*)variable;

	/* array may have resized, assign local properties again */
	self->arrayPtr = *self->arrayPtrPtr;
	self->arraySize = *self->arraySizePtr;
	
	if( !( self->offsetCount == 1 && !self->structSizePtr ) ) {
		/* For non-scalar or non-vector variables, the targets may have been extended. */
		self->structSize = *self->structSizePtr;
	}
}

void Variable_SetValueDoubleAll( void* variable, double value ) {
	Variable* self       = (Variable*)  variable;
	Index     arraySize  = self->arraySize;
	Index     array_I;

	for ( array_I = 0 ; array_I < arraySize ; array_I++ ) 
		Variable_SetValueDouble( self, array_I, value );
}
		

void Variable_SaveToFileAsDoubles( void* variable, char* filename ) {
	Variable* self       = (Variable*)  variable;
	Index     arraySize  = self->arraySize;
	Index     array_I;
	double    value;
	FILE*     outputFile;
	Index     comp_I;

	outputFile = fopen( filename, "w" );

	for ( array_I = 0 ; array_I < arraySize ; array_I++ ) {
		if ( *self->dataTypeCounts > 1 ) {
			for ( comp_I = 0; comp_I < *self->dataTypeCounts; comp_I++ ) {	
				value = Variable_GetValueAtDouble( self, array_I, comp_I );
				fwrite( &value, sizeof(double), 1, outputFile );
			}
		}
		else {
			value = Variable_GetValueDouble( self, array_I );
			fwrite( &value, sizeof(double), 1, outputFile );
		}	
	
	}	

	fclose( outputFile );
}


void Variable_ReadFromFileAsDoubles( void* variable, char* filename ) {
	Variable* self       = (Variable*)  variable;
	Index     arraySize  = self->arraySize;
	Index     array_I;
	double    value;
	FILE*     inputFile;
	Index     comp_I;

	inputFile = fopen( filename, "r" );

	for ( array_I = 0 ; array_I < arraySize ; array_I++ ) {
		if ( *self->dataTypeCounts > 1 ) {
			for ( comp_I = 0; comp_I < *self->dataTypeCounts; comp_I++ ) {	
				fread( &value, sizeof(double), 1, inputFile );
				Variable_SetValueAtDouble( self, array_I, comp_I, value );
			}
		}
		else {
			fread( &value, sizeof(double), 1, inputFile );
			Variable_SetValueDouble( self, array_I, value );
		}
	}	

	fclose( inputFile );
}


/** Returns || variable - comparison || / || variable ||, 
  * where ||x|| indicates the Euclidean Norm which is the square root of the sum of the squares for each component in x
  * i.e.  ||x|| = \sqrt{\sum_i x_i^2}*/
double Variable_ValueCompare( void* variable, void* _comparison ) {
	Variable* self            = (Variable*)  variable;
	Variable* comparison      = (Variable*) _comparison;
	Index     arraySize       = self->arraySize;
	Index     dataTypeCounts  = *self->dataTypeCounts;
	Index     array_I;
	Index     component_I;
	double    differenceValue;
	double    selfValue;
	double    normDifference  = 0.0;
	double    normVariable    = 0.0;
	Stream*   error           = Journal_Register( Error_Type, self->type );

	/* Simple Error Checking */
	Journal_Firewall( arraySize == comparison->arraySize, error, 
			"In func %s: Array sizes different for variables '%s' and '%s'\n", __func__, self->name, comparison->name );
	Journal_Firewall( dataTypeCounts == *comparison->dataTypeCounts, error, 
			"In func %s: Data Type Counts different for variables '%s' and '%s'\n",  __func__, self->name,comparison->name);

	for ( component_I = 0 ; component_I < self->offsetCount ; component_I++ ) {
		Journal_Firewall( self->dataTypes[component_I] == Variable_DataType_Double, error, 
				"In func %s: Stg_Component %d of variable '%s' has different type to Variable_DataType_Double.\n", 
				 __func__, component_I, self->name );
		Journal_Firewall( comparison->dataTypes[component_I] == Variable_DataType_Double, error, 
				"In func %s: Stg_Component %d of variable '%s' has different type to Variable_DataType_Double.\n", 
				 __func__, component_I, comparison->name );
	}

	/* Calculate Euclidean Norms */
	for ( array_I = 0 ; array_I < arraySize ; array_I++ ) {
		for ( component_I = 0 ; component_I < dataTypeCounts ; component_I++ ) {
			selfValue       = Variable_GetValueAtDouble( self, array_I, component_I );
			differenceValue = selfValue - Variable_GetValueAtDouble( comparison, array_I, component_I );

			normDifference += differenceValue * differenceValue;
			normVariable   += selfValue * selfValue;
		}
	}

	normDifference = sqrt( normDifference );
	normVariable   = sqrt( normVariable );

	return normDifference/normVariable;
}

/** Checks whether || variable - comparison || / || variable || < tolerance */
Bool Variable_ValueCompareWithinTolerance( void* variable, void* comparison, double tolerance ) {
	return ( Variable_ValueCompare( variable, comparison ) < tolerance );
}

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
** $Id: Memory.c 3788 2006-09-08 06:58:17Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


#include "types.h"
#include "forwardDecl.h"

#include "MemoryTag.h"
#include "MemoryField.h"
#include "MemoryPointer.h"
#include "Memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stddef.h>

#ifndef MAX
#define MAX( x, y ) ((x > y) ? x : y)
#endif

const Type Memory_Type = "Memory";

Memory* stgMemory = 0;

const int MEMORY_INITIAL_SIZE = 4;	/**< Number of pointers the array begins with. */
const int MEMORY_DELTA_SIZE = 4;	/**< Number of pointers to extend array by. */

/** Adds a MemoryPointer to the database. */
void Memory_Add_Pointer( MemoryPointer* memoryPointer );

/** Frees the allocation associated with MemoryPointer, but not the pointer itself. */
void Memory_Free_Pointer( MemoryPointer* memoryPointer );

/** Attempts to find the pointer in database. */
MemoryPointer* Memory_Find_Pointer( Pointer ptr );

/** Setups the pointer locations in a 2D array. */
void Memory_SetupPointer_2DArray(
	void* ptr,
	SizeT itemSize,
	Index xLength,
	Index yLength );

/** Setups the pointer locations in a 3D array. */
void Memory_SetupPointer_3DArray(
	void* ptr,
	SizeT itemSize,
	Index xLength,
	Index yLength,
	Index zLength );

/** Moves data inside an enlarged 2D array to the appropriate position. */
void Memory_Relocate_2DArray(
	void* destPtr, 
	void* srcPtr,
	SizeT itemSize, 
	Index oldX, 
	Index oldY, 
	Index newX, 
	Index newY );

/** Moves data inside an enlarged 3D array to the appropriate position. */
void Memory_Relocate_3DArray( 
	void* destPtr, 
	void* srcPtr,
	SizeT itemSize, 
	Index oldX, 
	Index oldY, 
	Index oldZ,
	Index newX, 
	Index newY,
	Index newZ );

#define Memory_OutOfMemoryError( size ) \
	_Memory_OutOfMemoryErrorFunc( __func__, __LINE__, size )

/** Display an out of memory error after an alloc call */
void _Memory_OutOfMemoryErrorFunc( const char* func, int line, SizeT size );


int compareFunction_NodeData_To_NodeData (void *nodeData1, void *nodeData2){
	
	MemoryPointer *memPtr1;
	MemoryPointer *memPtr2;
	
	memPtr1 = (MemoryPointer*) nodeData1;
	memPtr2 = (MemoryPointer*) nodeData2;
	
	#if 0
		printf ("comparing node data %p with data %p\t", memPtr->ptr, data2);
		if (memPtr->ptr == data2){
			printf (";pointers are equal");
		}
		printf ("\n");
	#endif
	
	if ( memPtr1->ptrReference > memPtr2->ptrReference ){
		return 1;
	}
	else if ( memPtr1->ptrReference == memPtr2->ptrReference && memPtr1->status != MEMORY_POINTER_RELEASED ){
		return 0;
	}
	else{
		return -1;
	}
}

int compareFunction_NodeData_To_Value (void *nodeData, void *value){
	
	MemoryPointer *memPtr;
	
	memPtr = (MemoryPointer*) nodeData;
	
	#if 0
		printf ("comparing node data %p with data %p\t", memPtr->ptrReference, value);
		if (memPtr->ptrReference == value){
			printf (";pointers are equal");
		}
		printf ("\n");
	#endif
	
	if ( memPtr->ptrReference > value ){
		return 1;
	}
	else if ( memPtr->ptrReference == value && memPtr->status != MEMORY_POINTER_RELEASED ){
		return 0;
	}
	else{
		return -1;
	}
}

Memory* Memory_Init()
{
	Memory* result;
	
	
	result = (Memory*) malloc( sizeof(Memory) );
	if ( result == NULL ) {
		Memory_OutOfMemoryError( sizeof(Memory) );
	}
	
	result->enable = True;
	result->stamp = 0;
	
	result->types = MemoryField_New( "Memory Statistics" );
	result->files = MemoryField_New( "Memory Statistics" );
	
	result->ptrCount = 0;
	result->ptrSize = MEMORY_INITIAL_SIZE;
	
	result->pointers = BTree_New ( compareFunction_NodeData_To_NodeData, NULL, NULL, NULL, BTREE_ALLOW_DUPLICATES );
	
	result->ptrCache = NULL;
	result->memCache = NULL;
	
	result->infoStream = NULL;
	result->errorStream = NULL;
	result->debugStream = NULL;

	result->stgCurrentMemory = 0;
	result->stgPeakMemory = 0;
	
	return result;
}

void Memory_Delete()
{
	if ( stgMemory == NULL )
	{
		return;
	}
	
	MemoryField_Delete( stgMemory->types );
	MemoryField_Delete( stgMemory->files );
	
	/* Releasing the memory in the tree, that might not have been released by the user, before deleting the tree */
	BTree_ParseTree ( stgMemory->pointers, MemoryPointer_Delete_Helper, NULL );
	
	/** Checking whether the memory module has been initialised before trying to delete it */
	if( stgMemory->pointers != NULL){
		BTree_Delete( stgMemory->pointers );
	}
}


void* _Memory_Alloc_Func(
	SizeT size,
	Type type,
	Name name,
	const char* fileName,
	const char* funcName,
	int lineNumber )
{
	Pointer result;
	
	MemoryPointer* memoryPointer;
	
	result = _Memory_InternalMalloc( size );
	
	if ( stgMemory->enable ) {
		memoryPointer = MemoryPointer_New( result, stgMemory->stamp++, type, name, fileName, funcName, lineNumber, MEMORY_OBJECT, size, size );
		memoryPointer->length.oneD = 1;
		
		Memory_Add_Pointer( memoryPointer );
	}
	return result;
}

void* _Memory_Alloc_Array_Func(
	SizeT itemSize,
	Index arrayLength,
	Type type,
	Name name,
	const char* fileName,
	const char* funcName,
	int lineNumber )
{
	Pointer result;
	SizeT size;
	
	MemoryPointer* memoryPointer;
	
	size = Memory_Length_1DArray( itemSize, arrayLength );
	result = _Memory_InternalMalloc( size );
	
	if ( stgMemory->enable ) {
		memoryPointer = MemoryPointer_New( result, stgMemory->stamp++, type, name, fileName, funcName, lineNumber, MEMORY_1DARRAY, itemSize, size );
		memoryPointer->length.oneD = arrayLength;
		
		Memory_Add_Pointer( memoryPointer );
	}
	return result;
}

void* _Memory_Alloc_2DArray_Func(
	SizeT itemSize,
	Index xLength,
	Index yLength,
	Type type,
	Name name,
	const char* fileName,
	const char* funcName,
	int lineNumber )
{
	Pointer result;
	SizeT size;
	
	
	#ifdef MEMORY_STATS
		MemoryPointer* memoryPointer;
	#endif
	
	size = Memory_Length_2DArray( itemSize, xLength, yLength );
	result = _Memory_InternalMalloc( size );
	
	
	Memory_SetupPointer_2DArray( result, itemSize, xLength, yLength );
	
	#ifdef MEMORY_STATS
		if ( stgMemory->enable ) {
			memoryPointer = MemoryPointer_New( result, stgMemory->stamp++, type, name, fileName, funcName, lineNumber, MEMORY_2DARRAY, itemSize, size );
			memoryPointer->length.twoD[0] = xLength;
			memoryPointer->length.twoD[1] = yLength;
			
			Memory_Add_Pointer( memoryPointer );
		}
	#endif
	
	return result;
}

void* _Memory_Alloc_3DArray_Func(
	SizeT itemSize,
	Index xLength,
	Index yLength,
	Index zLength,
	Type type,
	Name name,
	const char* fileName,
	const char* funcName,
	int lineNumber )
{
	Pointer result;
	SizeT size;
	
	#ifdef MEMORY_STATS
		MemoryPointer* memoryPointer;
	#endif
	
	size = Memory_Length_3DArray( itemSize, xLength, yLength, zLength );
	result = _Memory_InternalMalloc( size );
	
	Memory_SetupPointer_3DArray( result, itemSize, xLength, yLength, zLength );
	
	#ifdef MEMORY_STATS
		if ( stgMemory->enable ) {
			memoryPointer = MemoryPointer_New( result, stgMemory->stamp++, type, name, fileName, funcName, lineNumber, MEMORY_3DARRAY, itemSize, size );
			memoryPointer->length.threeD[0] = xLength;
			memoryPointer->length.threeD[1] = yLength;
			memoryPointer->length.threeD[2] = zLength;
			
			Memory_Add_Pointer( memoryPointer );
		}
	#endif
	
	return result;

}
		
void* _Memory_Alloc_4DArray_Func(
	SizeT itemSize,
	Index xLength,
	Index yLength,
	Index zLength,
	Index wLength,
	Type type,
	Name name,
	const char* fileName,
	const char* funcName,
	int lineNumber )
{
	Pointer		result;
	Pointer*	array1;
	Pointer**	array2;
	Pointer***	array3;
	SizeT size;
	Index i, j, k;
	int ptrHeader1, ptrHeader2, ptrHeader3;
	int ySize, yzProduct, yzwProduct, zwProduct;
	
	#ifdef MEMORY_STATS
		MemoryPointer* memoryPointer;
	#endif
	
	size = Memory_Length_4DArray( itemSize, xLength, yLength, zLength, wLength );
	result = _Memory_InternalMalloc( size );
	
	ptrHeader1 = sizeof(Pointer) * xLength;
	ptrHeader2 = sizeof(Pointer) * xLength * yLength;
	ptrHeader3 = sizeof(Pointer) * xLength * yLength * zLength;
	
	ySize = sizeof(Pointer) * yLength;
	yzProduct = yLength * zLength;
	yzwProduct = yLength * zLength * wLength;
	zwProduct = zLength * wLength;
	
	array1 = (Pointer*) result;
	for ( i = 0; i < xLength; ++i )
	{
		array1[i] = (Pointer)( (ArithPointer)result + ptrHeader1 + ( i * ySize ) );
	}
	
	array2 = (Pointer**) result;
	for ( i = 0; i < xLength; ++i )
	{
		for ( j = 0; j < yLength; ++j )
		{
			array2[i][j] = (Pointer)( (ArithPointer)result
				+ ptrHeader1
				+ ptrHeader2
				+ ( ( (i * yzProduct) + (j * zLength) ) * sizeof(Pointer) ) );
		}
	}
	
	array3 = (Pointer***) result;
	for ( i = 0; i < xLength; ++i )
	{
		for ( j = 0; j < yLength; ++j )
		{
			for ( k = 0; k < zLength; ++k )
			{
				array3[i][j][k] = (Pointer)( (ArithPointer)result
					+ ptrHeader1
					+ ptrHeader2
					+ ptrHeader3
					+ ( ( (i * yzwProduct) + (j * zwProduct) + (k * wLength) ) * itemSize ) );
			}
		}
	}
	
	#ifdef MEMORY_STATS
		if ( stgMemory->enable ) {
			memoryPointer = MemoryPointer_New( result, stgMemory->stamp++, type, name, fileName, funcName, lineNumber, MEMORY_3DARRAY, itemSize, size );
			memoryPointer->length.fourD[0] = xLength;
			memoryPointer->length.fourD[1] = yLength;
			memoryPointer->length.fourD[2] = zLength;
			memoryPointer->length.fourD[3] = wLength;
			
			Memory_Add_Pointer( memoryPointer );
		}
	#endif
	
	return result;
}


void* _Memory_Alloc_2DArrayAs1D_Func(
	SizeT itemSize,
	Index xLength,
	Index yLength,
	Type type,
	Name name,
	const char* fileName,
	const char* funcName,
	int lineNumber )
{
	Pointer result;
	SizeT size;
	
	MemoryPointer* memoryPointer;
	
	size = Memory_Length_2DAs1D( itemSize, xLength, yLength );
	result = _Memory_InternalMalloc( size );
	
	if ( stgMemory->enable ) {
		memoryPointer = MemoryPointer_New( result, stgMemory->stamp++, type, name, fileName, funcName, lineNumber, MEMORY_2DAS1D, itemSize, size );
		memoryPointer->length.twoD[0] = xLength;
		memoryPointer->length.twoD[1] = yLength;
		
		Memory_Add_Pointer( memoryPointer );
	}
	return result;
}

void* _Memory_Alloc_3DArrayAs1D_Func(
	SizeT itemSize,
	Index xLength,
	Index yLength,
	Index zLength,
	Type type,
	Name name,
	const char* fileName,
	const char* funcName,
	int lineNumber )
{
	Pointer result;
	SizeT size;
	
	MemoryPointer* memoryPointer;
	
	size = Memory_Length_3DAs1D( itemSize, xLength, yLength, zLength );
	result = _Memory_InternalMalloc( size );
	
	if ( stgMemory->enable ) {
		memoryPointer = MemoryPointer_New( result, stgMemory->stamp++, type, name, fileName, funcName, lineNumber, MEMORY_3DAS1D, itemSize, size );
		memoryPointer->length.threeD[0] = xLength;
		memoryPointer->length.threeD[1] = yLength;
		memoryPointer->length.threeD[2] = zLength;
		
		Memory_Add_Pointer( memoryPointer );
	}
	
	return result;
}

void* _Memory_Alloc_4DArrayAs1D_Func(
	SizeT itemSize,
	Index xLength,
	Index yLength,
	Index zLength,
	Index wLength,
	Type type,
	Name name,
	const char* fileName,
	const char* funcName,
	int lineNumber )
{
	Pointer result;
	SizeT size;
	
	MemoryPointer* memoryPointer;
	
	size = Memory_Length_4DAs1D( itemSize, xLength, yLength, zLength, wLength );
	result = _Memory_InternalMalloc( size );
	
	
	if ( stgMemory->enable ) {
		memoryPointer = MemoryPointer_New( result, stgMemory->stamp++, type, name, fileName, funcName, lineNumber, MEMORY_4DAS1D, itemSize, size );
		memoryPointer->length.fourD[0] = xLength;
		memoryPointer->length.fourD[1] = yLength;
		memoryPointer->length.fourD[2] = zLength;
		memoryPointer->length.fourD[3] = wLength;
	
		Memory_Add_Pointer( memoryPointer );
	}
	
	return result;
}

#if 0
void parseFunc (void *object, void *args){

	MemoryPointer *memPtr = (MemoryPointer*)object;
	printf ("\tdata - %p\n", memPtr->ptr);
}
#endif

void* _Memory_Alloc_2DComplex_Func(
	SizeT itemSize,
	Index xLength,
	Index* yLengths,
	Type type,
	Name name,
	const char* fileName,
	const char* funcName,
	int lineNumber )
{
	Pointer		result;
	Pointer*	array;
	SizeT size;
	Index i;
	
	#ifdef MEMORY_STATS
	MemoryPointer* memoryPointer;
	#endif
	
	size = Memory_Length_2DComplex( itemSize, xLength, yLengths );
	
	if ( 0 == size ) {
		Journal_Printf( stgMemory->errorStream, "Warning: %s() called to allocate a 2D variable "
			"length array where the total size calculated was 0. Returning NULL.\n" );
		return NULL;
	}
	
	result = _Memory_InternalMalloc( size );
	
	
	array = (Pointer*) result;
	
	/* Start of data area for the first array */
	array[0] = (Pointer) ( (ArithPointer)array + sizeof(Pointer) * xLength);
	
	for ( i = 1; i < xLength; ++i )
	{
		/* The previous array plus the previous length offset */
		array[i] = (Pointer)( (ArithPointer)array[i - 1] + ( itemSize * yLengths[i - 1] ) );
	}
	
	#ifdef MEMORY_STATS
	if ( stgMemory->enable ) {
		memoryPointer = MemoryPointer_New( result, stgMemory->stamp++, type, name, fileName, funcName, lineNumber, MEMORY_2DCOMPLEX, itemSize, size );
		memoryPointer->length.xyz.x = xLength;
		memoryPointer->length.xyz.y = (Index*) malloc( sizeof(Index) * xLength );
		if ( memoryPointer->length.xyz.y == NULL ) {
			Memory_OutOfMemoryError( sizeof(Index) * length );
		}
		
		for ( i = 0; i < xLength; ++i )
		{
			memoryPointer->length.xyz.y[i] = yLengths[i];
		}
		
		Memory_Add_Pointer( memoryPointer );
	}
	#endif
	
	return result;
}


Index** _Memory_Alloc_3DSetup_Func(
	Index xLength,
	Index* yLengths,
	const char* fileName,
	const char* funcName,
	int lineNumber )
{
	Index** result;
	#ifdef MEMORY_STATS
	MemoryPointer *memPtr;
	#endif
	
	result = (Index**) _Memory_Alloc_2DComplex_Func( sizeof(Index), xLength, yLengths,
		"Index", "MEMORY_SETUP", fileName, funcName, lineNumber );
	
	#ifdef MEMORY_STATS
	if ( stgMemory->enable ) {
		memPtr = Memory_Find_Pointer ( result );
		assert( memPtr );
		memPtr->allocType = MEMORY_3DSETUP;
	}
	#endif
	
	return result;
}


void* _Memory_Alloc_3DComplex_Func(
	SizeT itemSize,
	Index xLength,
	Index* yLengths,
	Index** zLengths,
	Type type,
	Name name,
	const char* fileName,
	const char* funcName,
	int lineNumber )
{
	Pointer		result;
	Pointer*	array1;
	Pointer**	array2;
	SizeT size;
	Index i, j;
	ArithPointer startPos, diffSize;
	
	#ifdef MEMORY_STATS
	MemoryPointer* memoryPointer;
	#endif
	
	size = Memory_Length_3DComplex( itemSize, xLength, yLengths, zLengths );
	result = _Memory_InternalMalloc( size );
	
	
	array1 = (Pointer*) result;
	
	/* Start of data area for the first array */
	startPos = (ArithPointer)result;
	diffSize = (ArithPointer)( sizeof(Pointer) * xLength );
	for (i = 0; i < xLength; ++i)
	{
		array1[i] = (Pointer)( startPos + diffSize );
		
		/* The previous array plus the previous length offset */
		startPos = (ArithPointer)array1[i];
		diffSize = (ArithPointer)( sizeof(Pointer) * yLengths[i] );
	}
	
	array2 = (Pointer**)result;
	
	for ( i = 0; i < xLength; ++i )
	{
		for ( j = 0; j < yLengths[i]; ++j )
		{
			array2[i][j] = (Pointer)( startPos + diffSize );
			startPos = (ArithPointer)array2[i][j];
			diffSize = (ArithPointer)( zLengths[i][j] * itemSize );
		}
	}
	
	
	#ifdef MEMORY_STATS
	if ( stgMemory->enable ) {
		memoryPointer = MemoryPointer_New( result, stgMemory->stamp++, type, name, fileName, funcName, lineNumber, MEMORY_3DCOMPLEX, itemSize, size );
		memoryPointer->length.xyz.x = xLength;
		memoryPointer->length.xyz.y = (Index*) malloc( sizeof(Index) * xLength );
		if ( memoryPointer->length.xyz.y == NULL ) {
			Memory_OutOfMemoryError( sizeof(Index) * xLength );
		}
		
		for ( i = 0; i < xLength; ++i )
		{
			memoryPointer->length.xyz.y[i] = yLengths[i];
		}
		
		memoryPointer->length.xyz.z = (Index**) malloc( Memory_Length_2DComplex( sizeof(Index), xLength, yLengths ) );
		if ( memoryPointer->length.xyz.z == NULL ) {
			Memory_OutOfMemoryError( Memory_Length_2DComplex( sizeof(Index), xLength, yLengths ) );
		}
		array1 = (Pointer*) memoryPointer->length.xyz.z;
		
		/* Start of data area for the first array */	
		startPos = (ArithPointer)memoryPointer->length.xyz.z;
		diffSize = (ArithPointer)( sizeof(Pointer) * xLength );
		for (i = 0; i < xLength; ++i)
		{
			array1[i] = (Pointer)( startPos + diffSize );
			
			/* The previous array plus the previous length offset */
			startPos = (ArithPointer)array1[i];
			diffSize = (ArithPointer)( sizeof(Index) * yLengths[i] );
		}
		
		/* Copy the z values */
		for (i = 0; i < xLength; ++i)
			{
			for (j = 0; j < yLengths[i]; ++j)
			{
				memoryPointer->length.xyz.z[i][j] = zLengths[i][j];
			}
		}
		
		Memory_Add_Pointer( memoryPointer );
	}
	#endif
	
	return result;
}


void* _Memory_Realloc_Func( 
	void* ptr, 
	SizeT newSize,
	Type type,
	const char* fileName,
	const char* funcName,
	int lineNumber )
{
	MemoryPointer* memoryPointer;
	BTreeNode *node;
	SizeT diffBytes;
	
	Pointer result = NULL;
	
	memoryPointer = Memory_Find_Pointer( ptr );
	result = _Memory_InternalRealloc( ptr, newSize );
	
	if ( stgMemory->enable ) {
		if ( memoryPointer )
		{
			if ( memoryPointer->allocType == MEMORY_OBJECT )
			{
				if ( result != NULL )
				{
					if (stgMemory->pointers){
						node = BTree_FindNode( stgMemory->pointers, (void*) memoryPointer );
						assert( node );
					
						BTree_DeleteNode( stgMemory->pointers, (BTreeNode*) node );
					}
					
					diffBytes = newSize - memoryPointer->totalSize;
					MemoryField_Update( memoryPointer->name, diffBytes );
					MemoryField_Update( memoryPointer->func, diffBytes );
					memoryPointer->ptr = result;
					memoryPointer->ptrReference = result;
					memoryPointer->stamp = stgMemory->stamp++;
					
					memoryPointer->itemSize = newSize;
					memoryPointer->totalSize = newSize;
					stgMemory->ptrCache = result;
					stgMemory->memCache = memoryPointer;
		
					if (stgMemory->pointers){
						BTree_InsertNode( stgMemory->pointers, memoryPointer, sizeof( MemoryPointer* ) );
					}
				}
				else
				{
					memoryPointer->ptr = NULL;
					memoryPointer->status = MEMORY_POINTER_RELEASED;
					memoryPointer->stamp = stgMemory->stamp++;
				}
			}
			/* Any other pointer type is invalid use of function */
		}
		else
		{
			memoryPointer = MemoryPointer_New( result, stgMemory->stamp++, type, Name_Invalid,
				fileName, funcName, lineNumber, MEMORY_OBJECT, newSize, newSize );
			memoryPointer->length.oneD = 1;
			Memory_Add_Pointer( memoryPointer );
		}
	}
	
	return result;
}


void* _Memory_Realloc_Array_Func( 
	void* ptr, 
	SizeT itemSize, 
	Index newLength,
	Type type,
	const char* fileName,
	const char* funcName,
	int lineNumber )
{
	MemoryPointer* memoryPointer;
	BTreeNode *node;
	SizeT newSize;
	SizeT diffBytes;
	
	Pointer result = NULL;
	
	memoryPointer = Memory_Find_Pointer( ptr );
	
	newSize = itemSize * newLength;
	result = _Memory_InternalRealloc( ptr, newSize );
	
	if ( stgMemory->enable ) {
		if ( memoryPointer )
		{
			if ( memoryPointer->allocType == MEMORY_1DARRAY )
			{
				if ( result != NULL )
				{
					MemoryPointer* copy;
					
					if (stgMemory->pointers){
						node = BTree_FindNode( stgMemory->pointers, (void*) memoryPointer );
						assert( node );
					
						BTree_DeleteNode( stgMemory->pointers, (BTreeNode*) node );
					}
					
					diffBytes = newSize - memoryPointer->totalSize;
					MemoryField_Update( memoryPointer->name, diffBytes );
					MemoryField_Update( memoryPointer->func, diffBytes );
					
					memoryPointer->ptr = result;
					memoryPointer->ptrReference = result;
					memoryPointer->stamp = stgMemory->stamp++;
					memoryPointer->itemSize = itemSize;
					memoryPointer->totalSize = newSize;
					memoryPointer->length.oneD = newLength;
					
					copy = (MemoryPointer*)_Memory_InternalMalloc( sizeof( MemoryPointer ) );
					memcpy( copy, memoryPointer, sizeof( MemoryPointer) );
					
					copy->ptr = result;
					copy->ptrReference = result;
					
					
					stgMemory->ptrCache = result;
					stgMemory->memCache = memoryPointer;
					
					if (stgMemory->pointers){
						BTree_InsertNode( stgMemory->pointers, memoryPointer, sizeof( MemoryPointer* ) );
					}
					
				}
				else
				{
					memoryPointer->ptr = NULL;
					memoryPointer->status = MEMORY_POINTER_RELEASED;
					memoryPointer->stamp = stgMemory->stamp++;
				}
			}
			/* Any other pointer type is invalid use of function */
		}
		else
		{
			memoryPointer = MemoryPointer_New( result, stgMemory->stamp++, type, Name_Invalid,
				fileName, funcName, lineNumber, MEMORY_1DARRAY, itemSize, newSize );
			memoryPointer->length.oneD = newLength;
			
			Memory_Add_Pointer( memoryPointer );
		}
	}
	return result;
}


void* _Memory_Realloc_2DArray_Func(
	void* ptr,
	SizeT itemSize, 
	Index oldX,
	Index oldY,
	Index newX, 
	Index newY,
	Type type,
	const char* fileName,
	const char* funcName,
	int lineNumber )
{
	#ifdef MEMORY_STATS
	MemoryPointer* memoryPointer;
	SizeT diffBytes;
	BTreeNode *node;
	#endif
	
	SizeT newSize = 0;
	Pointer result = NULL;
	Pointer* array;
	
	if ( ptr != NULL )
	{
		array = (Pointer*)ptr;
		oldX = ( (ArithPointer)array[0] - (ArithPointer)ptr ) / sizeof(Pointer);
		oldY = ( (ArithPointer)array[1] - (ArithPointer)array[0] ) / itemSize;
	}
	
	newSize = Memory_Length_2DArray( itemSize, newX, newY );
	result = _Memory_InternalRealloc( ptr, newSize );
	
	if ( ptr != NULL )
	{
		Memory_Relocate_2DArray( (Pointer)( (ArithPointer)result + (newX * sizeof(Pointer)) ),
					 (Pointer)( (ArithPointer)result + (oldX * sizeof(Pointer)) ),
					 itemSize, oldX, oldY, newX, newY );
	}
	
	Memory_SetupPointer_2DArray( result, itemSize, newX, newY );
	
	#ifdef MEMORY_STATS
	if ( stgMemory->enable ) {
		memoryPointer = Memory_Find_Pointer( ptr );
		if ( memoryPointer )
		{
			if ( memoryPointer->allocType == MEMORY_2DARRAY )
			{
				if (stgMemory->pointers){
					node = BTree_FindNode( stgMemory->pointers, (void*) memoryPointer );
					assert( node );
				
					BTree_DeleteNode( stgMemory->pointers, (void*) node );
				}
				
				diffBytes = newSize - memoryPointer->totalSize;
				MemoryField_Update( memoryPointer->name, diffBytes );
				MemoryField_Update( memoryPointer->func, diffBytes );
				memoryPointer->ptr = result;
				memoryPointer->ptrReference = result;
				memoryPointer->stamp = stgMemory->stamp++;
				memoryPointer->itemSize = itemSize;
				memoryPointer->totalSize = newSize;
				memoryPointer->length.twoD[0] = newX;
				memoryPointer->length.twoD[1] = newY;
				stgMemory->ptrCache = result;
				stgMemory->memCache = memoryPointer;
				
				if (stgMemory->pointers){
					BTree_InsertNode( stgMemory->pointers, memoryPointer, sizeof( MemoryPointer* ) );
				}
			}
			/* Any other pointer type is invalid use of function */
		}
		else
		{
			memoryPointer = MemoryPointer_New( result, stgMemory->stamp++, type, Name_Invalid,
				fileName, funcName, lineNumber, MEMORY_2DARRAY, itemSize, newSize );
			memoryPointer->length.twoD[0] = newX;
			memoryPointer->length.twoD[1] = newY;
			
			Memory_Add_Pointer( memoryPointer );
		}
	}
	#endif
	
	return result;
}

void* _Memory_Realloc_3DArray_Func( 
	void* ptr, 
	SizeT itemSize, 
	Index oldX,
	Index oldY,
	Index oldZ,
	Index newX, 
	Index newY, 
	Index newZ,
	Type type,
	const char* fileName,
	const char* funcName,
	int lineNumber )
{
	#ifdef MEMORY_STATS
	MemoryPointer* memoryPointer;
	SizeT diffBytes;
	BTreeNode *node;
	#endif
	
	SizeT newSize = 0;
	Pointer result = NULL;
	Pointer* array;
	Pointer** array2;
	
	if ( ptr != NULL )
	{
		array = (Pointer*)ptr;
		array2 = (Pointer**)ptr;
		
		oldX = ( (ArithPointer)array[0] - (ArithPointer)ptr ) / sizeof(Pointer);
		oldY = ( (ArithPointer)array[1] - (ArithPointer)array[0] ) / sizeof(Pointer);
		oldZ = ( (ArithPointer)array2[0][1] - (ArithPointer)array2[0][0] ) / itemSize;
	}
	
	newSize = Memory_Length_3DArray( itemSize, newX, newY, newZ );
	result = _Memory_InternalRealloc( ptr, newSize );
	
	if ( ptr != NULL )
	{
		Memory_Relocate_3DArray( (Pointer)( (ArithPointer)result + ( (newX + (newX * newY)) * sizeof(Pointer) ) ),
					 (Pointer)( (ArithPointer)result + ( (oldX + (oldX * oldY)) * sizeof(Pointer) ) ),
					 itemSize, oldX, oldY, oldZ, newX, newY, newZ );
	}
	
	Memory_SetupPointer_3DArray( result, itemSize, newX, newY, newZ );
	
	#ifdef MEMORY_STATS
	if ( stgMemory->enable ) {
		memoryPointer = Memory_Find_Pointer( ptr );
		if ( memoryPointer )
		{
			if ( memoryPointer->allocType == MEMORY_3DARRAY )
			{
				if (stgMemory->pointers){
					node = BTree_FindNode( stgMemory->pointers, (void*) memoryPointer );
					assert( node );
				
					BTree_DeleteNode( stgMemory->pointers, (void*) node );
				}
				
				diffBytes = newSize - memoryPointer->totalSize;
				MemoryField_Update( memoryPointer->name, diffBytes );
				MemoryField_Update( memoryPointer->func, diffBytes );
				memoryPointer->ptr = result;
				memoryPointer->ptrReference = result;
				memoryPointer->stamp = stgMemory->stamp++;
				memoryPointer->itemSize = itemSize;
				memoryPointer->totalSize = newSize;
				memoryPointer->length.threeD[0] = newX;
				memoryPointer->length.threeD[1] = newY;
				memoryPointer->length.threeD[2] = newZ;
				stgMemory->ptrCache = result;
				stgMemory->memCache = memoryPointer;
				
				if (stgMemory->pointers){
					BTree_InsertNode( stgMemory->pointers, memoryPointer, sizeof( MemoryPointer* ) );
				}
			}
			/* Any other pointer type is invalid use of function */
		}
		else
		{
			memoryPointer = MemoryPointer_New( result, stgMemory->stamp++, type, Name_Invalid,
				fileName, funcName, lineNumber, MEMORY_3DARRAY, itemSize, newSize );
			memoryPointer->length.twoD[0] = newX;
			memoryPointer->length.twoD[1] = newY;
			memoryPointer->length.twoD[2] = newY;
			
			Memory_Add_Pointer( memoryPointer );
		}
	}
	#endif
	
	return result;
}

void* _Memory_Realloc_2DArrayAs1D_Func( 
	void* ptr, 
	SizeT itemSize, 
	Index oldX, 
	Index oldY, 
	Index newX, 
	Index newY,
	Type type,
	const char* fileName,
	const char* funcName,
	int lineNumber )
{
	#ifdef MEMORY_STATS
	MemoryPointer* memoryPointer;
	BTreeNode *node;
	SizeT diffBytes;
	#endif
	
	SizeT newSize;
	Pointer result = NULL;
	
	newSize = itemSize * newX * newY;
	result = _Memory_InternalRealloc( ptr, newSize );
	
	if ( ptr != NULL )
	{
		Memory_Relocate_2DArray( result, result, itemSize, oldX, oldY, newX, newY );
	}
	
	#ifdef MEMORY_STATS
	if ( stgMemory->enable ) {
		memoryPointer = Memory_Find_Pointer( ptr );
		if ( memoryPointer )
		{
			if ( memoryPointer->allocType == MEMORY_2DAS1D )
			{
				if (stgMemory->pointers){
					node = BTree_FindNode( stgMemory->pointers, (void*) memoryPointer );
					assert( node );
				
					BTree_DeleteNode( stgMemory->pointers, (void*) node );
				}
				
				diffBytes = newSize - memoryPointer->totalSize;
				MemoryField_Update( memoryPointer->name, diffBytes );
				MemoryField_Update( memoryPointer->func, diffBytes );
				memoryPointer->ptr = result;
				memoryPointer->ptrReference = result;
				memoryPointer->stamp = stgMemory->stamp;
				memoryPointer->itemSize = itemSize;
				memoryPointer->totalSize = newSize;
				memoryPointer->length.twoD[0] = newX;
				memoryPointer->length.twoD[1] = newY;
				stgMemory->ptrCache = result;
				stgMemory->memCache = memoryPointer;
				
				if (stgMemory->pointers){
					BTree_InsertNode( stgMemory->pointers, memoryPointer, sizeof( MemoryPointer* ) );
				}
			}
			/* Any other pointer type is invalid use of function */
		}
		else
		{
			memoryPointer = MemoryPointer_New( result, stgMemory->stamp++, type, Name_Invalid,
				fileName, funcName, lineNumber, MEMORY_2DAS1D, itemSize, newSize );
			memoryPointer->length.twoD[0] = newX;
			memoryPointer->length.twoD[1] = newY;
			
			Memory_Add_Pointer( memoryPointer );
		}
	}
	#endif
	
	return result;
}

void* _Memory_Realloc_3DArrayAs1D_Func( 
	void* ptr, 
	SizeT itemSize,
	Index oldX, 
	Index oldY, 
	Index oldZ, 
	Index newX, 
	Index newY, 
	Index newZ,
	Type type,
	const char* fileName,
	const char* funcName,
	int lineNumber )
{
	#ifdef MEMORY_STATS
	MemoryPointer* memoryPointer;
	BTreeNode *node;
	SizeT diffBytes;
	#endif
	
	SizeT newSize;
	Pointer result = NULL;
	
	newSize = itemSize * newX * newY * newZ;
	result = _Memory_InternalRealloc( ptr, newSize );
	
	if ( ptr != NULL )
	{
		Memory_Relocate_3DArray( result, result, itemSize, oldX, oldY, oldZ, newX, newY, newZ );
	}
	
	#ifdef MEMORY_STATS
	if ( stgMemory->enable ) {
		memoryPointer = Memory_Find_Pointer( ptr );
		if ( memoryPointer )
		{
			if ( memoryPointer->allocType == MEMORY_3DAS1D )
			{
				if (stgMemory->pointers){
					node = BTree_FindNode( stgMemory->pointers, (void*) memoryPointer );
					assert( node );
				
					BTree_DeleteNode( stgMemory->pointers, (void*) node );
				}
				
				diffBytes = newSize - memoryPointer->totalSize;
				MemoryField_Update( memoryPointer->name, diffBytes );
				MemoryField_Update( memoryPointer->func, diffBytes );
				memoryPointer->ptr = result;
				memoryPointer->ptrReference = result;
				memoryPointer->stamp = stgMemory->stamp++;
				memoryPointer->itemSize = itemSize;
				memoryPointer->totalSize = newSize;
				memoryPointer->length.threeD[0] = newX;
				memoryPointer->length.threeD[1] = newY;
				memoryPointer->length.threeD[2] = newZ;
				stgMemory->ptrCache = result;
				stgMemory->memCache = memoryPointer;
				
				if (stgMemory->pointers){
					BTree_InsertNode( stgMemory->pointers, memoryPointer, sizeof( MemoryPointer* ) );
				}
			}
			/* Any other pointer type is invalid use of function */
		}
		else
		{
			memoryPointer = MemoryPointer_New( result, stgMemory->stamp++, type, Name_Invalid,
				fileName, funcName, lineNumber, MEMORY_3DAS1D, itemSize, newSize );
			memoryPointer->length.twoD[0] = newX;
			memoryPointer->length.twoD[1] = newY;
			memoryPointer->length.twoD[2] = newY;
			
			Memory_Add_Pointer( memoryPointer );
		}
	}
	#endif
	
	return result;
}


void _Memory_Free_Func( void* ptr )
{
	MemoryPointer* memoryPointer = NULL;
	
	memoryPointer = Memory_Find_Pointer( ptr );
	
	if ( memoryPointer )
	{
		Memory_Free_Pointer( memoryPointer );
	}
else
{
		_Memory_InternalFree( ptr );
	}
}
	
SizeT Memory_Length_1DArray( SizeT itemSize, Index length )
{
	return itemSize * length;
}

SizeT Memory_Length_2DArray( SizeT itemSize, Index xLength, Index yLength )
{
	return ( sizeof(Pointer) * xLength ) + ( itemSize * xLength * yLength );
}

SizeT Memory_Length_3DArray( SizeT itemSize, Index xLength, Index yLength, Index zLength )
{
	return ( sizeof(Pointer) * ( xLength + xLength * yLength ) ) + ( itemSize * xLength * yLength * zLength );
}


SizeT Memory_Length_4DArray( SizeT itemSize, Index xLength, Index yLength, Index zLength, Index wLength )
{
	return ( sizeof(Pointer) * ( xLength + xLength * yLength + xLength * yLength * zLength ) )
		 + ( itemSize * xLength * yLength * zLength * wLength );
}

SizeT Memory_Length_2DComplex( SizeT itemSize, Index xLength, Index* yLengths )
{
	Index i;
	SizeT result = 0;
	for ( i = 0; i < xLength; ++i )
	{
		result += yLengths[i];
	}
	result *= itemSize;
	result += sizeof(Pointer) * xLength;
	
	return result;
	
}

SizeT Memory_Length_3DComplex( SizeT itemSize, Index xLength, Index* yLengths, Index** zLengths )
{
	Index i, j;
	SizeT result = 0;
	SizeT ptrSize = xLength;
	
	for ( i = 0; i < xLength; ++i )
	{
		for ( j = 0; j < yLengths[i]; ++j )
		{
			result += zLengths[i][j];
		}
		ptrSize += yLengths[i];
	}
	result *= itemSize;
	result += sizeof(Pointer) * ptrSize; 
	
	return result;
}

SizeT Memory_Length_2DAs1D( SizeT itemSize, Index xLength, Index yLength )
{
	return itemSize * xLength * yLength;
}

SizeT Memory_Length_3DAs1D( SizeT itemSize, Index xLength, Index yLength, Index zLength )
{
	return itemSize * xLength * yLength * zLength;
}

SizeT Memory_Length_4DAs1D( SizeT itemSize, Index xLength, Index yLength, Index zLength, Index wLength )
{
	return itemSize * xLength * yLength * zLength * wLength;
}

void Memory_Print_Summary()
{
	Index i;
	
	MemoryField_PrintSummary( stgMemory->types, "Type" );
	
	Stream_Indent( stgMemory->infoStream );
	
	for ( i = 0; i < stgMemory->types->subCount; ++i )
	{
		Journal_Printf( stgMemory->infoStream, "\n" );
		MemoryField_PrintSummary( stgMemory->types->subFields[i], "Name" );
	}
	
	Stream_UnIndent( stgMemory->infoStream );
}

void Memory_Print()
{
	Memory_Print_Summary();
}

void Memory_Print_Leak()
{
	Memory_Print_Leak_Helper_Arg arguments;
	
	arguments.printOptions = MEMORYPOINTER_ALL;
	arguments.numLeaks = 0;
	arguments.warningPrinted = False;
	
	BTree_ParseTree ( stgMemory->pointers, Memory_Print_Leak_Helper,  (void*)( &arguments ) );
	
	if ( arguments.numLeaks > 0 )
	{
		printf( "\nPossible leaks detected: %d\n", arguments.numLeaks );
	}
	
}

void Memory_Print_Leak_Helper( void *memoryPointer, void *args )
{
	MemoryPointer *memPtr = NULL;
	Memory_Print_Leak_Helper_Arg *arguments = NULL;
	
	assert( memoryPointer );
	assert( args );
	
	memPtr = (MemoryPointer*) memoryPointer;
	arguments = (Memory_Print_Leak_Helper_Arg*) args;
	
	if ( memPtr->ptr != NULL && memPtr->status != MEMORY_POINTER_RELEASED ){
		if ( !arguments->warningPrinted )
		{
			/* A leak exists, so print warning title. */
			Journal_Printf( stgMemory->infoStream, "Warning. Possible memory leak:\n" );
			arguments->warningPrinted = True;
		}
		
		MemoryPointer_PrintAll( memPtr );
		++arguments->numLeaks;
	}
}


void Memory_Print_Type_Func( Type type )
{
	MemoryField* typeField;
	
	typeField = MemoryField_Register( stgMemory->types, type );
	
	MemoryField_PrintSummary( typeField, "Name" );
}

void Memory_Print_Type_Name_Func( Type type, Name name )
{
	MemoryField* typeField;
	MemoryField* nameField;
	MemoryPointer* memPtr;
	MemoryPointer_Print_Type_Name_Func_Helper_Arg arguments;
	
	
	typeField = MemoryField_Register( stgMemory->types, type );
	nameField = MemoryField_Register( typeField, name );
	
	Journal_PrintfL( stgMemory->infoStream, 1, "Type: %s\n", type );
	MemoryField_PrintHeaderAll( "Name" );
	MemoryField_PrintAll( nameField );
	
	Stream_Indent( stgMemory->infoStream );
	
	memPtr = NULL;
	arguments.printOptions = (MemoryPointerColumn)(
				MEMORYPOINTER_PTR |
				MEMORYPOINTER_FILE |
				MEMORYPOINTER_FUNC |
				MEMORYPOINTER_LINE |
				MEMORYPOINTER_ALLOCTYPE |
				MEMORYPOINTER_ITEMSIZE |
				MEMORYPOINTER_TOTALSIZE |
				MEMORYPOINTER_LENGTH);
	arguments.type = type;
	arguments.name = name;
	
	BTree_ParseTree ( stgMemory->pointers, MemoryPointer_Print_Type_Name_Func_Helper, (void *) ( &arguments ) );
	
	Stream_UnIndent( stgMemory->infoStream );
}


void Memory_Print_File( char* fileName )
{
	MemoryField* fileField;
	
	fileField = MemoryField_Register( stgMemory->files, fileName );
	
	MemoryField_PrintSummary( fileField, "Function" );
}

void Memory_Print_File_Function( char* fileName, char* funcName )
{
	MemoryField* fileField;
	MemoryField* funcField;
	MemoryPointer* memPtr;
	MemoryPointer_Print_File_Func_Helper_Arg arguments;
	
	fileField = MemoryField_Register( stgMemory->files, fileName );
	funcField = MemoryField_Register( fileField, funcName );
	
	Journal_PrintfL( stgMemory->infoStream, 1, "File: %s\n", fileName );
	MemoryField_PrintHeaderAll( "Function" );
	MemoryField_PrintAll( funcField );
	
	Stream_Indent( stgMemory->infoStream );
	
	memPtr = NULL;
	arguments.printOptions = (MemoryPointerColumn)(
				MEMORYPOINTER_PTR |
				MEMORYPOINTER_TYPE |
				MEMORYPOINTER_NAME |
				MEMORYPOINTER_LINE |
				MEMORYPOINTER_ALLOCTYPE |
				MEMORYPOINTER_ITEMSIZE |
				MEMORYPOINTER_TOTALSIZE |
				MEMORYPOINTER_LENGTH);
	arguments.fileName = fileName;
	arguments.funcName = funcName;
		
	BTree_ParseTree ( stgMemory->pointers, MemoryPointer_Print_File_Func_Helper, (void*) ( &arguments ) );
	
	Stream_UnIndent( stgMemory->infoStream );

}

void Memory_Print_Pointer( void* ptr )
{
	MemoryPointer* memoryPointer;
	
	BTree_SetCompareFunction( stgMemory->pointers, compareFunction_NodeData_To_Value );
	memoryPointer = Memory_Find_Pointer( ptr );
	if ( memoryPointer )
	{
		MemoryPointer_PrintAll( memoryPointer );
	}
	else
	{
		Journal_Printf( stgMemory->infoStream, "No entry exists for (ptr) %p\n", ptr );
	}
	BTree_SetCompareFunction( stgMemory->pointers, compareFunction_NodeData_To_NodeData );
}



void _Memory_Free_Type_Name_Func( Type type, Name name )
{
	Memory_Free_Type_Name_Func_Helper_Arg arguments;
	
	arguments.name = name;
	arguments.type = type;
	
	BTree_ParseTree ( stgMemory->pointers, _Memory_Free_Type_Name_Func_Helper, &arguments );
}

void _Memory_Free_Type_Name_Func_Helper( void *memoryPointer, void *args ){
	MemoryPointer *memPtr = NULL;
	MemoryField* typeField;
	MemoryField* nameField;
	Memory_Free_Type_Name_Func_Helper_Arg *arguments;
	
	assert ( memoryPointer );
	assert ( args );
	
	arguments = (Memory_Free_Type_Name_Func_Helper_Arg *) args;
	memPtr = (MemoryPointer*) memoryPointer;
	
	typeField = MemoryField_Register( stgMemory->types, arguments->type );
	nameField = MemoryField_Register( typeField, arguments->name );
	
	
	if ( memPtr->ptr != NULL
			&& memPtr->type == typeField
			&& memPtr->name == nameField )
	{
		Memory_Free_Pointer( memPtr );
	}
}

void _Memory_Free_Type_Func( Type type )
{
	BTree_ParseTree( stgMemory->pointers, _Memory_Free_Type_Func_Helper, (void*) type );
}

void _Memory_Free_Type_Func_Helper ( void *memoryPointer, void *args )
{
	MemoryPointer *memPtr = NULL;
	MemoryField* typeField;
	Type type;
	
	assert ( memoryPointer );
	assert ( args );
	
	type = (Type) args;
	memPtr = (MemoryPointer*) memoryPointer;
	
	typeField = MemoryField_Register( stgMemory->types, type );
	
	if ( memPtr->ptr != NULL
		&& memPtr->type == typeField )
	{
		Memory_Free_Pointer( memPtr );
	}
}

Bool Memory_IsAllocated( void* ptr ) {

	#ifdef MEMORY_STATS
		if ( stgMemory->enable ) {
			return Memory_Find_Pointer( ptr ) != NULL;
		}
		else {
			#ifdef DEBUG
				Journal_Printf( 
					stgMemory->debugStream,
					"Warning: Memory module not enabled. Memory_IsAllocated() is meaningless\n" );
			#endif
			return True;
		}
	#else
		#ifdef DEBUG
			Journal_Printf(
				stgMemory->debugStream,
				"Warning: Memory module not compiled - Memory_IsAllocated() is meaningless\n" );
		#endif
		return True;
	#endif
}


/* Begin private functions */

void Memory_Add_Pointer( MemoryPointer* memoryPointer )
{
	assert ( memoryPointer );
	
	stgMemory->ptrCache = memoryPointer->ptr;
	stgMemory->memCache = memoryPointer;
	
		if ( !memoryPointer->ptr ){
#if 0 			
			printf ("trying to insert a null pointer\n");
#endif
			return;
		}

	BTree_InsertNode ( stgMemory->pointers, memoryPointer, sizeof( MemoryPointer* ) );
}

void Memory_Free_Pointer( MemoryPointer* memoryPointer )
{
	_Memory_InternalFree( memoryPointer->ptr );
	memoryPointer->ptr = NULL;
	memoryPointer->status = MEMORY_POINTER_RELEASED;
	memoryPointer->stamp = stgMemory->stamp++;
	
	memoryPointer->name->freeCount++;
	memoryPointer->func->freeCount++;
	
	MemoryField_Update( memoryPointer->name, -(memoryPointer->totalSize) );
	MemoryField_Update( memoryPointer->func, -(memoryPointer->totalSize) );
}

MemoryPointer* Memory_Find_Pointer( Pointer ptr )
{
	BTreeNode *node = NULL;
	MemoryPointer *result = NULL;
	
	if ( stgMemory->ptrCache == ptr ){
		return stgMemory->memCache;
	}
	
	BTree_SetCompareFunction( stgMemory->pointers, compareFunction_NodeData_To_Value );
	
	node = BTree_FindNode( stgMemory->pointers, (void*) ptr);
	
	BTree_SetCompareFunction( stgMemory->pointers, compareFunction_NodeData_To_NodeData );
	
	if ( node != NULL ){
		result = (MemoryPointer*) BTree_GetData( node );
	}
	else{
		result = NULL;
	}
	
	return result;
}

void Memory_SetupPointer_2DArray(
	void* ptr,
	SizeT itemSize,
	Index xLength,
	Index yLength )
{
	Pointer* array;
	int ptrHeaderSize;
	int ySize;
	Index i;
	
	ptrHeaderSize = sizeof(Pointer) * xLength;
	ySize = itemSize * yLength;
	
	array = (Pointer*) ptr;
	for (i = 0; i < xLength; ++i )
	{
		array[i] = (Pointer)( (ArithPointer)ptr + ptrHeaderSize + ( i * ySize ) ) ;
	}

}

void Memory_SetupPointer_3DArray(
	void* ptr,
	SizeT itemSize,
	Index xLength,
	Index yLength,
	Index zLength )
{
	Pointer*	array1;
	Pointer**	array2;
	Index i, j;
	int ptrHeader1, ptrHeader2;
	int ySize, yzProduct;
	
	ptrHeader1 = sizeof(Pointer) * xLength;
	ptrHeader2 = sizeof(Pointer) * xLength * yLength;
	
	ySize = sizeof(Pointer) * yLength;
	yzProduct = yLength * zLength;
	
	array1 = (Pointer*) ptr;
	for ( i = 0; i < xLength; ++i )
	{
		array1[i] = (Pointer)( (ArithPointer)ptr + ptrHeader1 + ( i * ySize ) );
	}
	
	array2 = (Pointer**) ptr;
	for ( i = 0; i < xLength; ++i )
	{
		for ( j = 0; j < yLength; ++j )
		{
			array2[i][j] = (Pointer)( (ArithPointer)ptr
				+ ptrHeader1
				+ ptrHeader2
				+ ( ( (i * yzProduct) + (j * zLength) ) * itemSize ) );
		}
	}
}


void Memory_Relocate_2DArray(
	void* destPtr, 
	void* srcPtr,
	SizeT itemSize, 
	Index oldX, 
	Index oldY, 
	Index newX, 
	Index newY )
{
	int i, j;
	
	if ( ( oldX > newX ) || ( oldY > newY ) )
	{
		/* Cannot handle these combinations yet. */
		assert( 0 );
		return;
	}
	
	/* copy in reverse order to avoid overwritting data */
	for ( i = oldX - 1; i >= 0; --i )
	{
		for ( j = oldY - 1; j >= 0; --j )
		{
			memcpy( (Pointer)((ArithPointer)destPtr + ( ((i * newY) + j) * itemSize ) ),
				(Pointer)((ArithPointer)srcPtr + (  ((i * oldY) + j) * itemSize ) ),
				itemSize );
		}
	}
}

void Memory_Relocate_3DArray( 
	void* destPtr, 
	void* srcPtr,
	SizeT itemSize, 
	Index oldX, 
	Index oldY, 
	Index oldZ,
	Index newX, 
	Index newY,
	Index newZ )
{
	int i, j, k;
	
	if ( ( oldX > newX ) || ( oldY > newY ) || ( oldZ > newZ ) )
	{
		/* Cannot handle these combinations yet. */
		return;
	}
	
	/* copy in reverse order to avoid overwritting data */
	for ( i = oldX - 1; i >= 0; --i )
	{
		for ( j = oldY - 1; j >= 0; --j )
		{
			for ( k = oldZ - 1; k >= 0; --k )
			{
				memcpy( (Pointer)((ArithPointer)destPtr + ( ((i * newY * newZ) + (j * newZ) + k) * itemSize ) ),
					(Pointer)((ArithPointer)srcPtr  + ( ((i * oldY * oldZ) + (j * oldZ) + k) * itemSize ) ),
					itemSize );
			}
		}
	}
}

void* _Memory_InternalMalloc( SizeT size ) {
	void* result;
	void* data;
#ifdef NOEXPERIMENT			
	result = malloc( size );
	if ( result == NULL ) {
		Memory_OutOfMemoryError( size );
	}
	return result;
#else
	unsigned long prev = stgMemory->stgCurrentMemory;
	stgMemory->stgCurrentMemory += size;
	stgMemory->stgPeakMemory = MAX( stgMemory->stgCurrentMemory, prev );
	
	result = malloc( size + sizeof( MemoryTag ) );
	if ( result == NULL ) {
		result = malloc( size + sizeof( MemoryTag ) );
		if ( result == NULL ) {
			Memory_OutOfMemoryError( size + sizeof( MemoryTag ) );
		}
	}
	data = (void*)((ArithPointer)result + sizeof( MemoryTag ));
	
	Memory_CountGet( data ) = 0;
	Memory_SizeGet( data ) = size;

	return data;
#endif
}

void* _Memory_InternalRealloc( void* ptr, SizeT size ) {
	void* result;
	void* data;
	int count = 0;
#ifdef NOEXPERIMENT
	result =  realloc( ptr, size );
	if ( result == NULL ) {
		Memory_OutOfMemoryError( size );
	}
	return result;
#else
	unsigned long prev = stgMemory->stgCurrentMemory;

	if ( ptr == 0 ) {
		return _Memory_InternalMalloc( size );
	}
	count = Memory_CountGet( ptr );		/* Store previous count */
	stgMemory->stgCurrentMemory -= Memory_SizeGet( ptr );
	
	result = realloc( (void*)( (ArithPointer)ptr - sizeof(MemoryTag) ), size + sizeof(MemoryTag) );
	if ( result == NULL ) {
		/* have second attempt */
		result = realloc( (void*)( (ArithPointer)ptr - sizeof(MemoryTag) ), size + sizeof(MemoryTag) );
		if ( result == NULL ) {
			Memory_OutOfMemoryError( size + sizeof(MemoryTag) );
		}
	}
	data = (void*)((ArithPointer)result + sizeof( MemoryTag ));

	Memory_CountGet( data ) = count;
	Memory_SizeGet( data )  = size;
	stgMemory->stgCurrentMemory += size;
	stgMemory->stgPeakMemory = MAX( stgMemory->stgCurrentMemory, prev );

	return data;
#endif

}

void _Memory_InternalFree( void* ptr ) {
#ifdef NOEXPERIMENT
	free( ptr );
#else
	stgMemory->stgCurrentMemory -= Memory_SizeGet( ptr );
	free( (void*)((ArithPointer)ptr - sizeof( MemoryTag)) );
#endif
}


void _Memory_OutOfMemoryErrorFunc( const char* func, int line, SizeT size ) {
	Journal_Firewall(
		0,
		Journal_Register( Error_Type, "Memory" ),
		"StGermain Memory Error!\n"
		"\tAttempting to allocate %d bytes, in %s() line %d\n"
		"\tCurrent StGermain memory usage is %d bytes\n",
		size,
		func,
		line,
		stgMemory->stgCurrentMemory );
}


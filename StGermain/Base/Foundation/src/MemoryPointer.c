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
** $Id: MemoryPointer.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


#include "types.h"
#include "forwardDecl.h"

#include "Memory.h"
#include "MemoryField.h"
#include "MemoryPointer.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stddef.h>


const Type MemoryPointer_Type = "MemoryPointer";


MemoryPointer* MemoryPointer_New(
	Pointer ptr,
	MemoryOpStamp stamp,
	const char* type,
	const char* name,
	const char* file,
	const char* func,
	Index line,
	MemoryAllocType allocType,
	SizeT itemSize,
	SizeT totalSize )
{
	MemoryPointer* result = (MemoryPointer*) malloc( sizeof(MemoryPointer) );
	
	MemoryPointer_Init( result, ptr, stamp, type, name, file, func, line, allocType, itemSize, totalSize );
	
	return result;
}
	
void MemoryPointer_Init(
	MemoryPointer* memoryPointer,
	Pointer ptr,
	MemoryOpStamp stamp,
	const char* type,
	const char* name,
	const char* file,
	const char* func,
	Index line,
	MemoryAllocType allocType,
	SizeT itemSize,
	SizeT totalSize )
{
	memoryPointer->ptr = ptr;
	memoryPointer->ptrReference = ptr;
	memoryPointer->status = MEMORY_POINTER_PERSISTENT;
	memoryPointer->stamp = stamp;

	/* Get/Create records in Memory */
	memoryPointer->type = MemoryField_Register( stgMemory->types, type );
	memoryPointer->name = MemoryField_Register( memoryPointer->type, name );
	memoryPointer->file = MemoryField_Register( stgMemory->files, file );
	memoryPointer->func = MemoryField_Register( memoryPointer->file, func );
	
	memoryPointer->line = line;

	memoryPointer->name->allocCount++;
	memoryPointer->func->allocCount++;
	
	memoryPointer->allocType = allocType;
	memoryPointer->itemSize = itemSize;
	memoryPointer->totalSize = totalSize;

	/* Update the statistics of leaf fields. Parent fields can derive values. */
	MemoryField_Update( memoryPointer->name, totalSize );
	MemoryField_Update( memoryPointer->func, totalSize );
}
	
void MemoryPointer_Delete( MemoryPointer* memoryPointer )
{
	/* Deallocate length according to allocation type */
	switch ( memoryPointer->allocType )
	{
		case MEMORY_OBJECT:
		case MEMORY_1DARRAY:
		case MEMORY_2DARRAY:
		case MEMORY_3DARRAY:
		case MEMORY_4DARRAY:
		case MEMORY_2DAS1D:
		case MEMORY_3DAS1D:
		case MEMORY_4DAS1D:
			break;
		case MEMORY_2DCOMPLEX:
		case MEMORY_3DSETUP:
			free( memoryPointer->length.xyz.y );
			break;
		case MEMORY_3DCOMPLEX:
			free( memoryPointer->length.xyz.y );
			free( memoryPointer->length.xyz.z );
			break;
	}
	
	if ( memoryPointer ){
		free( memoryPointer );
	}
}

void MemoryPointer_Delete_Helper( void *memoryPointer, void *args )
{
	assert( memoryPointer );
	MemoryPointer_Delete( (MemoryPointer*) memoryPointer );
}

void MemoryPointer_Print( MemoryPointer* memoryPointer, MemoryPointerColumn columns )
{
	Index i, j;

	/*
	 * Bit compare which flags are set in order.
	 * Print each field accordingly
	 */
	
	if ( columns & MEMORYPOINTER_ALLOCTYPE )
	{
		switch ( memoryPointer->allocType )
		{
			case MEMORY_OBJECT:
				Journal_PrintfL( stgMemory->infoStream, 2, "Object,     " );
				break;
			case MEMORY_1DARRAY:
				Journal_PrintfL( stgMemory->infoStream, 2, "1D Array,   " );
				break;
			case MEMORY_2DARRAY:
				Journal_PrintfL( stgMemory->infoStream, 2, "2D Array,   " );
				break;
			case MEMORY_3DARRAY:
				Journal_PrintfL( stgMemory->infoStream, 2, "3D Array,   " );
				break;						
			case MEMORY_4DARRAY:
				Journal_PrintfL( stgMemory->infoStream, 2, "4D Array,   " );
				break;				
			case MEMORY_2DAS1D:
				Journal_PrintfL( stgMemory->infoStream, 2, "2D as 1D,   " );
				break;
			case MEMORY_3DAS1D:
				Journal_PrintfL( stgMemory->infoStream, 2, "3D as 1D,   " );
				break;					
			case MEMORY_4DAS1D:
				Journal_PrintfL( stgMemory->infoStream, 2, "4D as 1D,   " );
				break;
			case MEMORY_2DCOMPLEX:
				Journal_PrintfL( stgMemory->infoStream, 2, "2D Complex, " );
				break;
			case MEMORY_3DSETUP:
				Journal_PrintfL( stgMemory->infoStream, 2, "3D Setup,   " );
				break;
			case MEMORY_3DCOMPLEX:
				Journal_PrintfL( stgMemory->infoStream, 2, "3D Complex, " );
				break;
		}
	}
	if ( columns & MEMORYPOINTER_PTR )
	{
		Journal_PrintfL( stgMemory->infoStream, 2, "(ptr)%p, ", memoryPointer->ptr );
	}
	if ( columns & MEMORYPOINTER_TYPE )
	{
		Journal_PrintfL( stgMemory->infoStream, 2, "%s, ", memoryPointer->type->value );
	}
	if ( columns & MEMORYPOINTER_NAME )
	{
		Journal_PrintfL( stgMemory->infoStream, 2, "%s, ", memoryPointer->name->value );
	}
	if ( columns & MEMORYPOINTER_FILE )
	{
		Journal_PrintfL( stgMemory->infoStream, 2, "%s, ", memoryPointer->file->value );
	}
	if ( columns & MEMORYPOINTER_FUNC )
	{
		Journal_PrintfL( stgMemory->infoStream, 2, "%s(), ", memoryPointer->func->value );
	}
	if ( columns & MEMORYPOINTER_LINE )
	{
		Journal_PrintfL( stgMemory->infoStream, 2, "Line:%d, ", memoryPointer->line );
	}
	if ( columns & MEMORYPOINTER_TOTALSIZE )
	{
		Journal_PrintfL( stgMemory->infoStream, 2, "Total:%d, ", memoryPointer->totalSize );
	}
	if ( columns & MEMORYPOINTER_ITEMSIZE )
	{
		Journal_PrintfL( stgMemory->infoStream, 2, "ItemSize:%d, ", memoryPointer->itemSize );
	}
	if ( columns & MEMORYPOINTER_LENGTH )
	{
		Journal_PrintfL( stgMemory->infoStream, 2, "Count: " );
		switch ( memoryPointer->allocType )
		{
			case MEMORY_OBJECT:
				Journal_PrintfL( stgMemory->infoStream, 2, "N/A" );
				break;
			case MEMORY_1DARRAY:
				Journal_PrintfL( stgMemory->infoStream, 2, "%d", memoryPointer->length );
				break;
			case MEMORY_2DARRAY:
			case MEMORY_2DAS1D:
				Journal_PrintfL( stgMemory->infoStream, 2, "%d x %d",
					memoryPointer->length.twoD[0],
					memoryPointer->length.twoD[1] );
				break;
			case MEMORY_3DARRAY:
			case MEMORY_3DAS1D:
				Journal_PrintfL( stgMemory->infoStream, 2, "%d x %d x %d",
					memoryPointer->length.threeD[0],
					memoryPointer->length.threeD[1],
					memoryPointer->length.threeD[2] );
				break;
			case MEMORY_4DARRAY:
			case MEMORY_4DAS1D:
				Journal_PrintfL( stgMemory->infoStream, 2, "%d x %d x %d x %d",
					memoryPointer->length.fourD[0],
					memoryPointer->length.fourD[1],
					memoryPointer->length.fourD[2],
					memoryPointer->length.fourD[3] );
				break;				
			case MEMORY_2DCOMPLEX:
			case MEMORY_3DSETUP:
				Journal_PrintfL( stgMemory->infoStream, 3, "%d by { ", memoryPointer->length.xyz.x );
				for ( i = 0; i < memoryPointer->length.xyz.x; ++i )
				{
					Journal_PrintfL( stgMemory->infoStream, 3, "%d", memoryPointer->length.xyz.y[i] );
					if ( i != memoryPointer->length.xyz.x - 1 )
					{
						Journal_PrintfL( stgMemory->infoStream, 3, ", " );
					}
				}
				Journal_PrintfL( stgMemory->infoStream, 3, " }" );
				break;
			case MEMORY_3DCOMPLEX:
				Journal_PrintfL( stgMemory->infoStream, 3, "\n" );
				for ( i = 0; i < memoryPointer->length.xyz.x; ++i )
				{
					Journal_PrintfL( stgMemory->infoStream, 3, "{ " );
					for ( j = 0; j < memoryPointer->length.xyz.y[i]; ++j )
					{
						Journal_PrintfL( stgMemory->infoStream, 3, "%d", memoryPointer->length.xyz.z[i][j] );
						if ( j != memoryPointer->length.xyz.y[i] - 1 )
						{
							Journal_PrintfL( stgMemory->infoStream, 3, ", " );
						}
					}
					Journal_PrintfL( stgMemory->infoStream, 3, " }\n" );
				}
				Journal_PrintfL( stgMemory->infoStream, 3, " }" );
				break;
		}
	}
	Journal_PrintfL( stgMemory->infoStream, 2, "\n" );

}

void MemoryPointer_Print_Type_Name_Func_Helper( void *memoryPointer, void *args )
{
	MemoryPointer_Print_Type_Name_Func_Helper_Arg *arguments;
	MemoryPointer *memPtr;
	MemoryField* typeField;
	MemoryField* nameField;
	
	
	assert( memoryPointer );
	assert( args );

	memPtr = (MemoryPointer*) memoryPointer;
	arguments = (MemoryPointer_Print_Type_Name_Func_Helper_Arg*) args;
	
	typeField = MemoryField_Register( stgMemory->types, arguments->type );
	nameField = MemoryField_Register( typeField, arguments->name );	
	
	if ( memPtr->type == typeField && memPtr->name == nameField ){
		MemoryPointer_Print ( (MemoryPointer*) memoryPointer, arguments->printOptions );
	}
}

void MemoryPointer_Print_File_Func_Helper( void *memoryPointer, void *args )
{
	MemoryPointer_Print_File_Func_Helper_Arg *arguments;
	MemoryPointer *memPtr;
	MemoryField* fileField;
	MemoryField* funcField;
	
	
	assert( memoryPointer );
	assert( args );

	memPtr = (MemoryPointer*) memoryPointer;
	arguments = (MemoryPointer_Print_File_Func_Helper_Arg*) args;
	
	fileField = MemoryField_Register( stgMemory->types, arguments->fileName );
	funcField = MemoryField_Register( fileField, arguments->funcName );	
	
	if ( memPtr->file == fileField && memPtr->func == funcField ){
		MemoryPointer_Print ( (MemoryPointer*) memoryPointer, arguments->printOptions );
	}
}

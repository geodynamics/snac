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
** $Id: MemoryField.c 3803 2006-09-27 03:17:12Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdarg.h>

#include "types.h"
#include "forwardDecl.h"

#include "MemoryPointer.h"
#include "MemoryField.h"
#include "Memory.h"


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stddef.h>

#define MAX( x, y ) ((x > y) ? x : y)

const Type MemoryField_Type = "MemoryField";
const Name Memory_IgnoreName = "Memory_IgnoreName";

const Index MEMORYFIELD_DELTA = 4;	/**< Number of fields to extend by when array resizes. */


MemoryField* MemoryField_New( const char* value )
{
	MemoryField* result = (MemoryField*) malloc( sizeof(MemoryField) );
	_MemoryField_Init( result, value );

	return result;	
}
	
void _MemoryField_Init( MemoryField* memoryField, const char* value )
{
	if ( value )
	{
		memoryField->value = (char*)malloc( (strlen(value) + 1) * sizeof(char) );
		strcpy( memoryField->value, value );
	}
	else
	{
		memoryField->value = NULL;
	}
	
	memoryField->allocCount = 0;
	memoryField->freeCount = 0;
	memoryField->currentAllocation = 0;
	memoryField->peakAllocation = 0;
	memoryField->totalAllocation = 0;
	
	memoryField->subCount = 0;
	memoryField->subSize = 0;
	memoryField->subFields = NULL;	
	
	memoryField->memCache = NULL;
}
	
void MemoryField_Delete( MemoryField* memoryField )
{
	Index i;
	
	if ( memoryField->value )
	{
		free( memoryField->value );	
	}
	
	if ( memoryField->subFields )
	{
		for ( i = 0; i < memoryField->subCount; ++i )
		{
			MemoryField_Delete( memoryField->subFields[i] );
		}
		free( memoryField->subFields );
	}
	free( memoryField );
}
	

MemoryField* MemoryField_Register( MemoryField* memoryField, const char* subValue )
{
	Index i;
	
	/* Search cache first for localisation. */
	if ( memoryField->memCache )
	{
		if ( MemoryField_StringCompare( memoryField->memCache->value, subValue ) == 0 )
		{
			return memoryField->memCache;
		}
	}
	
	/* Not in cache so linear search. */
	for ( i = 0; i < memoryField->subCount; ++i )
	{
		if ( MemoryField_StringCompare( memoryField->subFields[i]->value, subValue ) == 0 )
		{
			/* Value exists, update cache and return the field. */
			memoryField->memCache = memoryField->subFields[i];
			return memoryField->subFields[i];
		}
	}
	
	/* Not exist, add to registry. */
	
	/* Extend array if needed. */
	if ( memoryField->subCount == memoryField->subSize )
	{
		memoryField->subSize += MEMORYFIELD_DELTA;
		memoryField->subFields = (MemoryField**)
			realloc( memoryField->subFields, sizeof(MemoryField*) * memoryField->subSize );
	}
	
	memoryField->subFields[memoryField->subCount] = MemoryField_New( subValue );
	memoryField->memCache = memoryField->subFields[memoryField->subCount];
	
	memoryField->subCount++;
	
	return memoryField->subFields[memoryField->subCount - 1];
}

void MemoryField_Update( MemoryField* memoryField, SizeT bytes )
{
	memoryField->currentAllocation += bytes;
	
	memoryField->peakAllocation = MAX( memoryField->currentAllocation, memoryField->peakAllocation );
	
	if ( bytes > 0 )
	{
		memoryField->totalAllocation += bytes;
	}
}

void MemoryField_UpdateAsSumOfSubFields( MemoryField* memoryField )
{
	Index i;

	if( memoryField->subCount < 1 )
	{
		return;	
	}
	
	memoryField->allocCount = 0;
	memoryField->freeCount = 0;
	memoryField->currentAllocation = 0;
	memoryField->peakAllocation = 0;
	memoryField->totalAllocation = 0;
	
	for ( i = 0; i < memoryField->subCount; ++i )
	{
		if ( !MemoryField_StringCompare( memoryField->subFields[i]->value, Memory_IgnoreName ) == 0 )
		{
			MemoryField_UpdateAsSumOfSubFields( memoryField->subFields[i] );
			memoryField->allocCount += memoryField->subFields[i]->allocCount;
			memoryField->freeCount += memoryField->subFields[i]->freeCount;
			memoryField->currentAllocation += memoryField->subFields[i]->currentAllocation;
			memoryField->peakAllocation += memoryField->subFields[i]->peakAllocation;
			memoryField->totalAllocation += memoryField->subFields[i]->totalAllocation;
		}
	}
}

void MemoryField_Print( MemoryField* memoryField, MemoryFieldColumn columns )
{
	if ( MemoryField_StringCompare( memoryField->value, Memory_IgnoreName ) == 0 )
	{
		return;
	}

	if ( columns & MEMORYFIELD_VALUE )
	{
		Journal_PrintfL( stgMemory->infoStream, 1, "%-20s ", memoryField->value );
	}
	if ( columns & MEMORYFIELD_ALLOC )
	{
		Journal_PrintfL( stgMemory->infoStream, 1, "%15d ", memoryField->allocCount );
	}
	if ( columns & MEMORYFIELD_FREE )
	{
		Journal_PrintfL( stgMemory->infoStream, 1, "%10d ", memoryField->freeCount );
	}
	if ( columns & MEMORYFIELD_CURRENT )
	{
		Journal_PrintfL( stgMemory->infoStream, 1, "%13d ", memoryField->currentAllocation );
	}
	if ( columns & MEMORYFIELD_PEAK )
	{
		Journal_PrintfL( stgMemory->infoStream, 1, "%10d ", memoryField->peakAllocation );
	}
	if ( columns & MEMORYFIELD_TOTAL )
	{
		Journal_PrintfL( stgMemory->infoStream, 1, "%11d", memoryField->totalAllocation );
	}
	Journal_PrintfL( stgMemory->infoStream, 1, "\n" );

}
void MemoryField_PrintHeader( const char* fieldName, MemoryFieldColumn columns )
{
	if ( columns & MEMORYFIELD_VALUE )
	{
		Journal_PrintfL( stgMemory->infoStream, 1, "%-20s ", fieldName );
	}
	if ( columns & MEMORYFIELD_ALLOC )
	{
		Journal_PrintfL( stgMemory->infoStream, 1, "Times Allocated " );
	}
	if ( columns & MEMORYFIELD_FREE )
	{
		Journal_PrintfL( stgMemory->infoStream, 1, "Times Free " );
	}
	if ( columns & MEMORYFIELD_CURRENT )
	{
		Journal_PrintfL( stgMemory->infoStream, 1, "Current bytes " );
	}
	if ( columns & MEMORYFIELD_PEAK )
	{
		Journal_PrintfL( stgMemory->infoStream, 1, "Peak bytes " );
	}
	if ( columns & MEMORYFIELD_TOTAL )
	{
		Journal_PrintfL( stgMemory->infoStream, 1, "Total bytes " );
	}

	Journal_PrintfL( stgMemory->infoStream, 1, "\n" );
}

void MemoryField_PrintSummary( MemoryField* memoryField, const char* tableTitle )
{
	Index i;

	if ( MemoryField_StringCompare( memoryField->value, Memory_IgnoreName ) == 0 )
	{
		return;
	}

	if ( memoryField->subCount > 0 )
	{
		MemoryField_UpdateAsSumOfSubFields( memoryField );

		MemoryField_Sort( memoryField );

		Journal_Printf( stgMemory->infoStream, "%s\n", memoryField->value );

		Stream_Indent( stgMemory->infoStream );
		MemoryField_PrintHeader( NULL, (MemoryFieldColumn)(
			MEMORYFIELD_ALLOC |
			MEMORYFIELD_FREE |
			MEMORYFIELD_CURRENT | 
			MEMORYFIELD_PEAK |
			MEMORYFIELD_TOTAL) );
		MemoryField_Print( memoryField, (MemoryFieldColumn)(
			MEMORYFIELD_ALLOC |
			MEMORYFIELD_FREE |
			MEMORYFIELD_CURRENT |
			MEMORYFIELD_PEAK |
			MEMORYFIELD_TOTAL) );

		Stream_Indent( stgMemory->infoStream );
		
		MemoryField_PrintHeaderAll( tableTitle );
		for ( i = 0; i < memoryField->subCount; ++i )
		{
			if ( memoryField->subFields[i]->allocCount > 0 )
			{
				MemoryField_PrintAll( memoryField->subFields[i] );
			}
		}
		
		Stream_UnIndent( stgMemory->infoStream );

		Stream_UnIndent( stgMemory->infoStream );
	}
}

void MemoryField_Sort( MemoryField* memoryField )
{
	/* insertion sort */
	Index i, j;
	MemoryField* tmp;

	for ( i = 0; i < memoryField->subCount; ++i )
	{
		j = i;
		tmp = memoryField->subFields[j];
		while ( j > 0 && MemoryField_StringCompare( memoryField->subFields[j - 1]->value, tmp->value ) > 0 )
		{
			memoryField->subFields[j] = memoryField->subFields[j - 1];
			--j;
		}
		memoryField->subFields[j] = tmp;
	}
}

int MemoryField_StringCompare( const char* s1, const char* s2 )
{
	if ( s1 && s2 )
	{
		return strcmp( s1, s2 );
	}
	return (long)s1 - (long)s2;
}

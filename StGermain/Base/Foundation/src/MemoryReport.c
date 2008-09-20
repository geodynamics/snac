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
** $Id: MemoryReport.c 3803 2006-09-27 03:17:12Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


#include "types.h"
#include "forwardDecl.h"

#include "MemoryField.h"
#include "MemoryPointer.h"
#include "MemoryReport.h"
#include "Memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stddef.h>

const Type MemoryReport_Type = "MemoryReport";

const int MEMORYREPORT_DELTA = 1;	/**< Number of items to grow by when array resizes. */
const int MEMORYREPORT_SIZE = 2;	/**< Number of items an array begins with. */


/** Returns the index of a given group in an array and -1 if not found. */
int MemoryReport_Find_Group( int numGroups, MemoryReportGroup* groups, MemoryReportGroup search );


MemoryReport* MemoryReport_New( )
{
	MemoryReport* result = (MemoryReport*) malloc( sizeof(MemoryReport) );
	
	_MemoryReport_Init( result );
	
	return result;
}
	
void _MemoryReport_Init( MemoryReport* memoryReport )
{
	memoryReport->groupCount = 0;
	memoryReport->groupSize = MEMORYREPORT_SIZE;
	memoryReport->groups = (MemoryReportGroup*) malloc( sizeof(MemoryReportGroup) * MEMORYREPORT_SIZE );
	memoryReport->conditionCount = 0;
	memoryReport->conditionSize = MEMORYREPORT_SIZE;
	memoryReport->conditionGroups = (MemoryReportGroup*) malloc( sizeof(MemoryReportGroup) * MEMORYREPORT_SIZE );
	memoryReport->conditionValues = (char**) malloc( sizeof(char*) * MEMORYREPORT_SIZE );
}
	
void MemoryReport_Delete( MemoryReport* memoryReport )
{
	Index i;
	
	free( memoryReport->groups );
	free( memoryReport->conditionGroups );
	
	for ( i = 0; i < memoryReport->conditionCount; ++i )
	{
		if ( memoryReport->conditionValues[i] != NULL )
		{
	 		free( memoryReport->conditionValues[i] );
	 	}
	}
	free( memoryReport->conditionValues );
}

void MemoryReport_AddGroup( MemoryReport* memoryReport, MemoryReportGroup group )
{
	if ( MemoryReport_Find_Group( memoryReport->groupCount, memoryReport->groups, group ) >= 0 )
	{
		return;
	}

	/* Extend the groups array if needed. */
	if ( memoryReport->groupCount == memoryReport->groupSize )
	{
		memoryReport->groupSize += MEMORYREPORT_DELTA;
		memoryReport->groups = (MemoryReportGroup*)
			realloc( memoryReport->groups, sizeof(MemoryReportGroup) * memoryReport->groupSize );	
	}
	
	memoryReport->groups[memoryReport->groupCount] = group;
	memoryReport->groupCount++;
}

void MemoryReport_AddCondition( MemoryReport* memoryReport, MemoryReportGroup group, const char* condition )
{
	/* Add this group if it does not already exist. */
	if ( MemoryReport_Find_Group( memoryReport->groupCount, memoryReport->groups, group ) < 0 )
	{
		MemoryReport_AddGroup( memoryReport, group );	
	}
	
	/* Extend the condition arrays if needed. */
	if ( memoryReport->conditionCount == memoryReport->conditionSize )
	{
		memoryReport->conditionSize += MEMORYREPORT_DELTA;
		memoryReport->conditionGroups = (MemoryReportGroup*)
			realloc( memoryReport->conditionGroups, sizeof(MemoryReportGroup) * memoryReport->conditionSize );
		memoryReport->conditionValues = (char**)
			realloc( memoryReport->conditionValues, sizeof(char*) * memoryReport->conditionSize );		
	}
	
	memoryReport->conditionGroups[memoryReport->conditionCount] = group;
	
	if ( condition )
	{
		char*	ptr = memoryReport->conditionValues[memoryReport->conditionCount];

		ptr = (char*)malloc( (strlen(condition) + 1) * sizeof(char) );
		strcpy( ptr, condition );
	}
	else
	{
		/* NULL is a condition as well, such as Type_Invalid and Name_Invalid. */
		memoryReport->conditionValues[memoryReport->conditionCount] = NULL;
	}
	
	memoryReport->conditionCount++;
}

void MemoryReport_Print( MemoryReport* memoryReport )
{
	BTree_ParseTree ( stgMemory->pointers, MemoryReport_Print_Helper, (void*) memoryReport );
}

void MemoryReport_Print_Helper( void *memoryPointer, void* memReport )
{
	MemoryField* rootField;		/* The top level container for where the results begin. */
	MemoryField* prevField; 	/* A temporary pointer to hold previous fields. */
	Bool valid;	/* Whether a memory pointer record matches the conditions. */
	MemoryReport* memoryReport;
	MemoryPointer* memPtr;
	Index iGroup, iCondition;	/* Iterators. */
	
	assert ( memoryPointer );
	assert ( memReport );

	memoryReport = (MemoryReport*) memReport;
		
	if ( memoryReport->groupCount == 0 )
	{
		return;
	}
	
	rootField = MemoryField_New( "Report Query:" );

	/* Algorithm:
	 * - Iterate through all MemoryPointers recorded.
	 * - Tuples matching the condition are added to the results.
	 * - Statistics are derived from tuples.
	 *
	 * Reason:
	 * Allows flexibility to produce any report required. The down side is that the peak bytes used cannot be derived this way.
	 *
	 * The alternative is to always record stats for all combinations (useful ones) but that will have a large impact on run
	 * time as well as memory space.
	 */
	memPtr = (MemoryPointer*) memoryPointer;

	/* check condition */
	valid = True;
	for ( iCondition = 0; iCondition < memoryReport->conditionCount && valid; ++iCondition )
	{
		switch ( memoryReport->conditionGroups[iCondition] )
		{
			case MEMORYREPORT_TYPE:
				if ( MemoryField_StringCompare( memPtr->type->value,
					memoryReport->conditionValues[iCondition] ) != 0 )
				{
					valid = False;
				}
				break;
			case MEMORYREPORT_NAME:
				if ( MemoryField_StringCompare( memPtr->name->value,
					memoryReport->conditionValues[iCondition] ) != 0 )
				{
					valid = False;
				}
				break;
			case MEMORYREPORT_FILE:
				if ( MemoryField_StringCompare( memPtr->file->value,
					memoryReport->conditionValues[iCondition] ) != 0 )
				{
					valid = False;
				}
				break;
			case MEMORYREPORT_FUNC:
				if ( MemoryField_StringCompare( memPtr->func->value,
					memoryReport->conditionValues[iCondition] ) != 0 )
				{
					valid = False;
				}
				break;
		}
	}
		
	if ( valid )
	{
		/* Add this entry, sorted by the groups of the report. */
		prevField = rootField;
		for ( iGroup = 0; iGroup < memoryReport->groupCount; ++iGroup )
		{
			switch ( memoryReport->groups[iGroup] )
			{
				case MEMORYREPORT_TYPE:
					prevField = MemoryField_Register( prevField, memPtr->type->value );
					break;
				case MEMORYREPORT_NAME:
					prevField = MemoryField_Register( prevField, memPtr->name->value );
					break;
				case MEMORYREPORT_FILE:
					prevField = MemoryField_Register( prevField, memPtr->file->value );
					break;
				case MEMORYREPORT_FUNC:
					prevField = MemoryField_Register( prevField, memPtr->func->value );
					break;
			}
		}
		
		/* Derive the statistics. */
		prevField->allocCount++;
		if ( memPtr->ptr == NULL )
		{
			prevField->freeCount++;
		}
		else
		{
			prevField->currentAllocation += memPtr->totalSize;
		}
		prevField->totalAllocation += memPtr->totalSize;
	}
	
	
	prevField = rootField;
	while ( prevField->subCount == 1 )
	{
		Journal_Printf( stgMemory->infoStream, "%s ", prevField->value );
		prevField = prevField->subFields[0];
	}
	
	MemoryField_PrintSummary( prevField, "~Report~" );

	MemoryField_Delete( rootField );
}

int MemoryReport_Find_Group( int numGroups, MemoryReportGroup* groups, MemoryReportGroup search )
{
	int result;
	
	for ( result = 0; result < numGroups; ++result )
	{
		if ( groups[result] == search )
		{
			return result;
		}
	}
	
	return -1;
}

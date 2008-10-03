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
** $Id: MemMonitor.c 3157 2005-08-07 23:43:05Z AlanLo $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdarg.h>
#include <mpi.h>

#include "types.h"
#include "shortcuts.h"
#include "forwardDecl.h"
#include "MemoryPointer.h"
#include "MemoryField.h"
#include "Memory.h"
#include "MemMonitor.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

double Stg_MemoryWatchCriteria = -1;

const Type Stg_MemMonitor_Type = "Stg_MemMonitor";
const Type Stg_MemMonitor_InfoStreamName = "MemMonitor";
const Type Stg_MemMonitor_TagType = "Stg_MemMonitor_Tag";

void _Memory_Print_AllocsAboveThreshold_Helper( void* memoryPointer, void* args ) {
	MemoryPointer* memPtr;
	void** arguments;
	SizeT threshold;
	MemoryOpStamp begin;
	MemoryOpStamp end;

	assert( memoryPointer );
	assert( args );

	memPtr = (MemoryPointer*)memoryPointer;
	arguments = (void**)args;
	threshold = *((SizeT*)arguments[0]);
	begin = *((MemoryOpStamp*)arguments[1]);
	end = *((MemoryOpStamp*)arguments[2]);

	if ( memPtr->ptr != NULL && memPtr->status != MEMORY_POINTER_RELEASED ) {
		if ( memPtr->totalSize > threshold && memPtr->stamp >= begin && memPtr->stamp <= end ) {
			MemoryPointer_Print( memPtr, MEMORYPOINTER_NAME | MEMORYPOINTER_TOTALSIZE );
		}
	}
}

void Stg_MemMonitor_Initialise() {
	Stg_MemoryWatchCriteria = 0.2;
}
void Stg_MemMonitor_Finalise() {

}
void Stg_MemMonitor_SetMemoryWatchCriteria( double ratioOfTotalMemory ) {
	Stg_MemoryWatchCriteria = ratioOfTotalMemory;
}

Stg_MemMonitor* Stg_MemMonitor_New( char* tag, Bool criteria, Bool print, int comm ) {
	Stg_MemMonitor* tm;
	
	tm = Memory_Alloc_Unnamed( Stg_MemMonitor );
	tm->tag = Memory_Alloc_Bytes_Unnamed( strlen( tag ) + 1, Stg_MemMonitor_TagType );
	strcpy( tm->tag, tag );
	tm->criteria = criteria;
	tm->print = print;
	tm->comm = comm;
	
	return tm;
}

void Stg_MemMonitor_Delete( Stg_MemMonitor* tm ) {
	if( tm->tag ) {
		Memory_Free( tm->tag );
	}
	
	Memory_Free( tm );
}

void Stg_MemMonitor_Begin( Stg_MemMonitor* tm ) {
#ifdef MEMORY_STATS
	tm->t1 = stgMemory->stamp;
	tm->t2 = tm->t1;
	MemoryField_UpdateAsSumOfSubFields( stgMemory->types );
	tm->totalMem1 = stgMemory->types->currentAllocation;
	tm->totalMem2 = tm->totalMem1;
#endif
}

double Stg_MemMonitor_End( Stg_MemMonitor* tm ) {
	double memAvgDiff = 0.0;
#ifdef MEMORY_STATS
	long memDiff;
	long memMaxDiff;
	long memMinDiff;
	long memSumDiff;
	long memMax;
	
	int rank;
	int size;
	
	tm->t2 = stgMemory->stamp;
	MemoryField_UpdateAsSumOfSubFields( stgMemory->types );
	tm->totalMem2 = stgMemory->types->currentAllocation;
	memDiff = tm->totalMem2 - tm->totalMem1;
	
	MPI_Comm_size( tm->comm, &size );
	/*
	MPI_Reduce( &memDiff, &memMaxDiff, 1, MPI_LONG, MPI_MAX, 0, tm->comm );
	MPI_Reduce( &memDiff, &memMinDiff, 1, MPI_LONG, MPI_MIN, 0, tm->comm );
	MPI_Allreduce( &memDiff, &memSumDiff, 1, MPI_LONG, MPI_SUM, tm->comm );
	MPI_Reduce( &tm->totalMem2, &memMax, 1, MPI_LONG, MPI_MAX, 0, tm->comm );
	
	memAvgDiff = (double)memSumDiff / size;
	*/
	/* Above is commented and replaced with below. See TimeMonitor.c for reason */
	memMaxDiff = memDiff;
	memMinDiff = memDiff;
	memSumDiff = memDiff;
	memMax = tm->totalMem2;
	memAvgDiff = (double)memDiff;
	
	/* Note: maybe Stg_Components should store rank and comm??? how do the find their comm? */
	
	MPI_Comm_rank( tm->comm, &rank );
	if( rank == 0 && tm->print ) {
		if( !tm->criteria || (double)memMaxDiff > Stg_MemoryWatchCriteria * memMax ) {
			void* args[3];
			SizeT threshold = (SizeT)(Stg_MemoryWatchCriteria * memMax);
			
			if( size == 1 ) {
				Journal_Printf( 
					Journal_Register( Info_Type, Stg_MemMonitor_InfoStreamName ),
					"\t%s(%s): ms: %.2gmb, dt(%.2f%%): %.2gmb\n", 
					Stg_MemMonitor_InfoStreamName,
					tm->tag,
					(double)memMax / 1048576,
					(memAvgDiff) / memMax * 100.0,
					(double)memDiff / 1048576 );
			}
			else {
				Journal_Printf( 
					Journal_Register( Info_Type, Stg_MemMonitor_InfoStreamName ),
					"\t%s(%s): ms: %.2gmb, dt(%.f%%): %.2g/%.2g/%.2gmb\n", 
					Stg_MemMonitor_InfoStreamName,
					tm->tag,
					(double)memMax / 1048576,
					(memAvgDiff) / memMax * 100.0,
					(double)memMaxDiff / 1048576,
					(double)memMinDiff / 1048576,
					(double)memAvgDiff / 1048576 );
			}
			args[0] = &threshold;
			args[1] = &tm->t1;
			args[2] = &tm->t2;
			BTree_ParseTree( stgMemory->pointers, _Memory_Print_AllocsAboveThreshold_Helper, (void*)args );
		}
	}
	
#endif
	return memAvgDiff;
}

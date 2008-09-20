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
** $Id: TimeMonitor.c 3157 2005-08-07 23:43:05Z AlanLo $
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
#include "TimeMonitor.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define Stg_TimeMonitor_t0_ZERO_CONDITION (-1.0)

double Stg_TimeMonitor_t0 = Stg_TimeMonitor_t0_ZERO_CONDITION;
double Stg_TimerWatchCriteria;

const Type Stg_TimeMonitor_Type = "Stg_TimeMonitor";
const Type Stg_TimeMonitor_InfoStreamName = "TimeMonitor";
const Type Stg_TimeMonitor_TagType = "Stg_TimeMonitor_Tag";

void Stg_TimeMonitor_Initialise() {
	Stg_TimeMonitor_t0 = MPI_Wtime();
	Stg_TimerWatchCriteria = 0.2;
}
void Stg_TimeMonitor_Finalise() {

}
void Stg_TimeMonitor_SetTimerWatchCriteria( double ratioOfTotalTime ) {
	Stg_TimerWatchCriteria = ratioOfTotalTime;
}

Stg_TimeMonitor* Stg_TimeMonitor_New( char* tag, Bool criteria, Bool print, int comm ) {
	Stg_TimeMonitor* tm;
	
	tm = Memory_Alloc_Unnamed( Stg_TimeMonitor );
	tm->tag = Memory_Alloc_Bytes_Unnamed( strlen( tag ) + 1, Stg_TimeMonitor_TagType );
	strcpy( tm->tag, tag );
	tm->criteria = criteria;
	tm->print = print;
	tm->comm = comm;
	
	return tm;
}

void Stg_TimeMonitor_Delete( Stg_TimeMonitor* tm ) {
	if( tm->tag ) {
		Memory_Free( tm->tag );
	}
	
	Memory_Free( tm );
}

void Stg_TimeMonitor_Begin( Stg_TimeMonitor* tm ) {
	Journal_Firewall(
		Stg_TimeMonitor_t0 > Stg_TimeMonitor_t0_ZERO_CONDITION,
		Journal_Register( Error_Type, Stg_TimeMonitor_Type ),
		"Time monitor used but not initialised!\n" );
	
	tm->t1 = MPI_Wtime();
	tm->t2 = tm->t1;	
}

double Stg_TimeMonitor_End( Stg_TimeMonitor* tm ) {
	double dt;
	double maxdt;
	double mindt;
	double sumdt;
	double avedt;
	double maxt;
	int rank;
	int size;
	
	tm->t2 = MPI_Wtime();
	dt = tm->t2 - tm->t1;
	MPI_Comm_size( tm->comm, &size );
	/*
	MPI_Reduce( &dt, &maxdt, 1, MPI_DOUBLE, MPI_MAX, 0, tm->comm );
	MPI_Reduce( &dt, &mindt, 1, MPI_DOUBLE, MPI_MIN, 0, tm->comm );
	MPI_Allreduce( &dt, &sumdt, 1, MPI_DOUBLE, MPI_SUM, tm->comm );
	MPI_Reduce( &tm->t2, &maxt, 1, MPI_DOUBLE, MPI_MAX, 0, tm->comm );
	avedt = sumdt / size;
	*/
	/* Note: Above is commented out because cannot use MPI_Reduce functions unless we are sure
	 * that ALL procs will call end(). This is currently not the case with Stg_Component_Initialise()
	 * phase as some procs will have more/less variables to call Initialise() on via Variable_Condition
	 * due to decomposition and Wall boundary conditions */
	maxdt = dt;
	mindt = dt;
	sumdt = dt * size;
	avedt = (double)dt;
	maxt = tm->t2;

	/* Note: maybe Stg_Components should store rank and comm??? how do the find their comm? */
	
	MPI_Comm_rank( tm->comm, &rank );
	if( rank == 0 && tm->print ) {
		if( !tm->criteria || maxdt > Stg_TimerWatchCriteria * (maxt - Stg_TimeMonitor_t0) ) {
			if( size == 1 ) {
				Journal_Printf( 
					Journal_Register( Info_Type, Stg_TimeMonitor_InfoStreamName ),
					"\t%s(%s):  ts: %.2g (secs), dt(%.2g%%): %.2gs\n", 
					Stg_TimeMonitor_InfoStreamName,
					tm->tag,
					maxt - Stg_TimeMonitor_t0,
					(avedt) / (maxt - Stg_TimeMonitor_t0) * 100.0,
					avedt  );
			}
			else {
				Journal_Printf( 
					Journal_Register( Info_Type, Stg_TimeMonitor_InfoStreamName ),
					"\t%s(%s):  ts: %.2g (secs), dt(%.g%%): %.2g/%.2g/%.2gs\n", 
					Stg_TimeMonitor_InfoStreamName,
					tm->tag,
					maxt - Stg_TimeMonitor_t0,
					(avedt) / (maxt - Stg_TimeMonitor_t0) * 100.0,
					maxdt,
					mindt,
					avedt );
			}
		}
	}
	
	return avedt;
}

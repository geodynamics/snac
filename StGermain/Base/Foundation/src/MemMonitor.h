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

#ifndef __Base_Foundation_MemMonitor_h__
#define __Base_Foundation_MemMonitor_h__

extern double Stg_MemoryWatchCriteria;

extern const Type Stg_MemMonitor_Type;
extern const Type Stg_MemMonitor_InfoStreamName;
extern const Type Stg_MemMonitor_TagType;

typedef struct {
	MemoryOpStamp t1;
	MemoryOpStamp t2;
	SizeT totalMem1;
	SizeT totalMem2;
	char* tag;
	Bool criteria;
	Bool print;
	int comm;
} Stg_MemMonitor;

void Stg_MemMonitor_Initialise();
void Stg_MemMonitor_Finalise();
void Stg_MemMonitor_SetMemoryWatchCriteria( double ratioOfTotalMemory );

Stg_MemMonitor* Stg_MemMonitor_New( char* tag, Bool criteria, Bool print, int comm );
void Stg_MemMonitor_Delete( Stg_MemMonitor* tm );

void Stg_MemMonitor_Begin( Stg_MemMonitor* tm );
double Stg_MemMonitor_End( Stg_MemMonitor* tm );

#endif

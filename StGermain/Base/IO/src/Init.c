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
** $Id: Init.c 3573 2006-05-15 07:51:14Z AlanLo $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "mpi.h"

#include "Base/Foundation/Foundation.h"

#include "types.h"
#include "shortcuts.h"
#include "JournalFile.h"
#include "Stream.h"
#include "CStream.h"
#include "MPIStream.h"
#include "StreamFormatter.h"
#include "LineFormatter.h"
#include "RankFormatter.h"
#include "Journal.h"
#include "Init.h"

#include <stdio.h>
#include <stdarg.h>

const Name     LiveDebugName = "LiveDebug";
Stream*        LiveDebug = NULL;

Bool BaseIO_Init( int* argc, char** argv[] )
{
	Stream* typedStream;

	stgStreamFormatter_Buffer = StreamFormatter_Buffer_New(); 

	stJournal = Journal_New();

	/* Create default Typed Streams. */

	/* info */
	typedStream = CStream_New( Info_Type );
	Stream_Enable( typedStream, True );
	Stream_SetLevel( typedStream, 1 );
	Stream_SetFile( typedStream, stJournal->stdOut );
	Stream_SetAutoFlush( typedStream, True );
	
	Journal_RegisterTypedStream( typedStream );

	/* debug */
	typedStream = CStream_New( Debug_Type );
	Stream_Enable( typedStream, False );
	Stream_SetLevel( typedStream, 1 );
	Stream_SetFile( typedStream, stJournal->stdOut );
	Stream_SetAutoFlush( typedStream, True );
	Stream_AddFormatter( typedStream, RankFormatter_New() );
	
	Journal_RegisterTypedStream( typedStream );
	
	/* dump */
	typedStream = CStream_New( Dump_Type );
	Stream_Enable( typedStream, False );
	Stream_SetLevel( typedStream, 1 );
	Stream_SetFile( typedStream, stJournal->stdOut );
	
	Journal_RegisterTypedStream( typedStream );
	
	/* error */
	typedStream = CStream_New( Error_Type );
	Stream_Enable( typedStream, True );
	Stream_SetLevel( typedStream, 1 );
	Stream_SetFile( typedStream, stJournal->stdErr );
	Stream_SetAutoFlush( typedStream, True );
	Stream_AddFormatter( typedStream, RankFormatter_New() );
	
	Journal_RegisterTypedStream( typedStream );
	
	/* mpi stream */
	typedStream = MPIStream_New( MPIStream_Type );
	Stream_Enable( typedStream, True );
	Stream_SetLevel( typedStream, 1 );
	
	Journal_RegisterTypedStream( typedStream );

	Journal_Printf( Journal_Register( DebugStream_Type, "Context" ), "In: %s\n", __func__ ); /* DO NOT CHANGE OR REMOVE */
	
	stgMemory->infoStream = Stg_Class_Copy( Journal_GetTypedStream( Info_Type ), 0, True, 0, 0 );
	stgMemory->debugStream = Stg_Class_Copy( Journal_GetTypedStream( Debug_Type ), 0, True, 0, 0 );
	stgMemory->errorStream = Stg_Class_Copy( Journal_GetTypedStream( Error_Type ), 0, True, 0, 0 );


	/* more inits from the Foundation level */
	Stream_Enable( Journal_Register( Info_Type, Stg_TimeMonitor_InfoStreamName ), False );
	Stream_Enable( Journal_Register( Info_Type, Stg_MemMonitor_InfoStreamName ), False );

	/* The LiveDebug stream */
	LiveDebug = Journal_Register( Info_Type, LiveDebugName );
	Stream_Enable( LiveDebug, True );
	Stream_SetLevel( LiveDebug, 1 );
	
	return True;
}

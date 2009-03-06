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
** $Id: Journal.c 3855 2006-10-13 07:20:36Z KathleenHumble $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdarg.h>
#include <mpi.h>


#include "Base/Foundation/Foundation.h"

#include "types.h"
#include "shortcuts.h"
#include "Dictionary.h"
#include "Dictionary_Entry.h"
#include "Dictionary_Entry_Value.h"
#include "Journal.h"
#include "JournalFile.h"
#include "CFile.h"
#include "Stream.h"
#include "CStream.h"
#include "MPIStream.h"
#include "StreamFormatter.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


Journal* stJournal = NULL;

const Type Journal_Type = "Journal";

/* String constants for default Journal typed streams. */
const Type Info_Type = "info";
const Type Debug_Type = "debug";
const Type Dump_Type = "dump";
const Type Error_Type = "error";

/* Depricated string constants. */
const Type InfoStream_Type = "info";
const Type DebugStream_Type = "debug";
const Type DumpStream_Type = "dump";
const Type ErrorStream_Type = "error";

/* Parsing constants for Dictionary reading. */
const char* JOURNAL_DELIMITER = ".";
const char* JOURNAL_KEY = "journal";
const char* JOURNAL_ENABLE_KEY = "journal";
const char* JOURNAL_ENABLE_BRANCH_KEY = "journal-enable-branch";
const char* JOURNAL_LEVEL_KEY = "journal-level";
const char* JOURNAL_LEVEL_BRANCH_KEY = "journal-level-branch";
const char* JOURNAL_FILE_KEY = "journal-file";
const char* JOURNAL_RANK_KEY = "journal-rank";
const char* JOURNAL_FLUSH_KEY = "journal-autoflush";
const char* JOURNAL_MPI_OFFSET_KEY = "journal-mpi-offset";

const char* JOURNAL_ON = "on";
const char* JOURNAL_OFF = "off";


static const int JOURNAL_START_SIZE = 16;	/**< Initial size of lists in Journal. */
static const int JOURNAL_DELTA_SIZE = 8;	/**< Number of elements to extend by for lists in Journal. */


/** Retrieves the given named stream. Dotted-decimal notation may be used to get sub streams.
 ** If the stream does not exists, a new stream is created from the parent stream.
 **/
Stream* Journal_GetNamedStream( Stream* typedStream, const Name name );


Journal* Journal_New()
{
	Journal* self = Memory_Alloc( Journal, "Singleton" );
	
	self->_files = Stg_ObjectList_New2( JOURNAL_START_SIZE, JOURNAL_DELTA_SIZE );
	self->_typedStreams = Stg_ObjectList_New2( JOURNAL_START_SIZE, JOURNAL_DELTA_SIZE );
	self->enable = True;

	/** Initialise default files. */
	self->stdOut = CFile_New();
	self->stdOut->fileHandle = stdout;
	self->stdOut->_opened = True;
	
	self->stdErr = CFile_New();
	self->stdErr->fileHandle = stderr;
	self->stdErr->_opened = True;
	
	self->firewallProducesAssert = True;

	self->flushCount = 0;
	
	return self;
}

void Journal_Delete( )
{
	int i;
	
	Stg_Class_Delete( stJournal->stdOut );
	Stg_Class_Delete( stJournal->stdErr );
	
	Stg_ObjectList_DeleteAllObjects( stJournal->_typedStreams );
	Stg_Class_Delete( stJournal->_typedStreams );
	
	for ( i = 0; i < stJournal->_files->count; ++i )
	{
		if ( !JournalFile_Close( (JournalFile*) stJournal->_files->data[i] ) )
		{
			/* TODO: warning can't close */
		}
	}
	Stg_ObjectList_DeleteAllObjects( stJournal->_files );
	Stg_Class_Delete( stJournal->_files );
	
	Memory_Free( stJournal );

}

void Journal_ReadFromDictionary( Dictionary* dictionary )
{
	Dictionary_Index index;

	Stream* stream = NULL; /* The stream which the operation will be applied to. */
	
	char* keyCopy;	
	char* typedToken;
	char* namedToken;
	
	char* operation;
				
	Bool valid;
	
	/* Iterate through the whole dictionary, checking for journal related commands */
	for ( index = 0; index < dictionary->count; ++index )
	{
		/* Check to see if it is a journal operation by searching for the JOURNAL_KEY. */
		if ( (0 == strncasecmp( dictionary->entryPtr[index]->key, JOURNAL_KEY, strlen(JOURNAL_KEY))) )
		{
			valid = True;

			/* Make copy to tokenise so original contents will not be destroyed */
			keyCopy = StG_Strdup( dictionary->entryPtr[index]->key );
			
			/* The first token determines the operation to perform. */
			operation = strtok( keyCopy, JOURNAL_DELIMITER );
			
			typedToken = strtok( NULL, JOURNAL_DELIMITER );
			namedToken = strtok( NULL, "" );
				
			if ( strcmp( operation, JOURNAL_ENABLE_KEY ) == 0 )
			{
				/* Enable function */
				Bool enable = Dictionary_Entry_Value_AsBool( dictionary->entryPtr[index]->value );
				
				if ( typedToken == NULL )
				{
					/* no more tokens, set the entire journal */
					stJournal->enable = enable;
				}
				else
				{
					if ( namedToken == NULL )
					{
						/* no more tokens, set the entire type */
						Journal_Enable_TypedStream( typedToken, enable );
					}
					else
					{
						Journal_Enable_NamedStream( typedToken, namedToken, enable );
					}
				}
				
			} /* END Enable function */
			if ( strcmp( operation, JOURNAL_ENABLE_BRANCH_KEY ) == 0 )
			{
				Stream* branchStream;
				/* Enable function */
				Bool enable = Dictionary_Entry_Value_AsBool( dictionary->entryPtr[index]->value );
		
				assert( typedToken );
				if ( namedToken == NULL ) {
					branchStream = Journal_GetTypedStream( typedToken );
				}
				else {
					branchStream = Journal_Register( typedToken, namedToken );
				}
				Stream_EnableBranch( branchStream, enable );
				
			} /* END Enable function */
			else if ( strcmp( operation, JOURNAL_LEVEL_KEY ) == 0 )
			{
				/* Set level function */
				JournalLevel level;
				
				level = Dictionary_Entry_Value_AsUnsignedInt( dictionary->entryPtr[index]->value );
				
				if ( typedToken == NULL )
				{
					printf( "Warning - Can only change level for typed and named streams.\n" );
				}
				else
				{
					if ( namedToken == NULL )
					{
						stream = Journal_GetTypedStream( typedToken );
					}
					else
					{
						stream = Journal_Register( typedToken, namedToken );
					}
					Stream_SetLevel( stream, level );
				}
			} /* END SetLevel function */
			else if ( strcmp( operation, JOURNAL_LEVEL_BRANCH_KEY ) == 0 )
			{
				/* Set level function */
				JournalLevel level;
				
				level = Dictionary_Entry_Value_AsUnsignedInt( dictionary->entryPtr[index]->value );
				
				if ( typedToken == NULL )
				{
					printf( "Warning - Can only change level for typed and named streams.\n" );
				}
				else
				{
					if ( namedToken == NULL )
					{
						stream = Journal_GetTypedStream( typedToken );
					}
					else
					{
						stream = Journal_Register( typedToken, namedToken );
					}
					Stream_SetLevelBranch( stream, level );
				}
			} /* END SetLevelBranch function */
			else if ( strcmp( operation, JOURNAL_FILE_KEY ) == 0 )
			{
				/* Set file destination function */
				JournalFile* file = NULL;
				char* fileName = Dictionary_Entry_Value_AsString( dictionary->entryPtr[index]->value );
				
				if ( typedToken == NULL )
				{
					printf( "Warning - Can only change files for typed and named streams.\n" );
				}
				else
				{
					if ( namedToken == NULL )
					{
						/* setting file for typed stream */
						stream = Journal_GetTypedStream( typedToken );
					}
					else
					{
						stream = Journal_Register( typedToken, namedToken );
					}
					
					file = Journal_GetFile( fileName );
					if ( file == NULL )
					{
						file = stream->defaultFileType();
						if ( JournalFile_Open( file, fileName ) ) {
							Journal_RegisterFile( file );
						}
						else {
							/* error/warning */
						}
					}
					if ( !Stream_SetFile( stream, file ) )
					{
						/* file is not valid */
					}
				}
			} /* END File function */
			else if ( strcmp( operation, JOURNAL_RANK_KEY ) == 0 )
			{
				int rank;

				rank = Dictionary_Entry_Value_AsUnsignedInt( dictionary->entryPtr[index]->value );

				if ( typedToken == NULL )
				{
					printf( "Warning - Can only set printing rank for typed and named streams.\n" );
				}
				else
				{
					if ( namedToken == NULL )
					{
						stream = Journal_GetTypedStream( typedToken );
					}
					else
					{
						stream = Journal_Register( typedToken, namedToken );
					}
					Stream_SetPrintingRank( stream, rank );
				}
			} /* END Printing Rank funciton. */
			else if ( strcmp( operation, JOURNAL_FLUSH_KEY ) == 0 )
			{
				Bool flush = Dictionary_Entry_Value_AsBool( dictionary->entryPtr[index]->value );

				if ( typedToken == NULL )
				{
					printf( "Warning - Can only set auto-flush for typed and named streams.\n" );
				}
				else
				{
					if ( namedToken == NULL )
					{
						stream = Journal_GetTypedStream( typedToken );
					}
					else
					{
						stream = Journal_Register( typedToken, namedToken );
					}

					Stream_SetAutoFlush( stream, flush );
				}
			}
			else if ( strcmp( operation, JOURNAL_MPI_OFFSET_KEY ) == 0 )
			{
				MPI_Offset offset = Dictionary_Entry_Value_AsUnsignedInt( dictionary->entryPtr[index]->value );

				if ( typedToken == NULL ) 
				{
					printf( "Warning - Can only set mpi-offset for typed and named streams.\n" );
				}	
				else
				{
					if ( namedToken == NULL )
					{
						stream = Journal_GetTypedStream( typedToken );
					}
					else
					{
						stream = Journal_Register( typedToken, namedToken );
					}

					if ( ! MPIStream_SetOffset( stream, offset, MPI_COMM_WORLD ) )
					{
						printf( "Warning - Failed to set mpi-offset for %s", typedToken );
						if ( namedToken )
						{
							printf( ".%s", namedToken );
						}
						printf( "\n" );
					}	
				}
			}	
			
			Memory_Free( keyCopy );
		}	
	}
}

void Journal_RegisterTypedStream( Stream* typedStream )
{
	/* check exists and update */
	Stg_ObjectList_Append( stJournal->_typedStreams, typedStream );
}
Stream* Journal_GetTypedStream( const Type type )
{
	Stream* typedStream = NULL;

	typedStream = (Stream*)Stg_ObjectList_Get( stJournal->_typedStreams, (Name)type );	
	if ( typedStream == NULL )
	{
		/* Make a default stream for this name. */
		typedStream = CStream_New( type );
		Stream_Enable( typedStream, True );
		Stream_SetLevel( typedStream, 1 );
		Stream_SetFile( typedStream, stJournal->stdOut );

		Journal_RegisterTypedStream( typedStream );
	}
	
	return typedStream;
}
Stream* Journal_GetNamedStream( Stream* typedStream, const Name name )
{
	Stream* currentStream ;
	char* nameCopy;
	char* token;
	
	currentStream = typedStream;
	nameCopy = StG_Strdup( name );
	
	
	token = strtok( nameCopy, JOURNAL_DELIMITER );
	while ( token != NULL )
	{
		currentStream = Stream_RegisterChild( currentStream, token );
		token = strtok( NULL, JOURNAL_DELIMITER );
	}
	
	Memory_Free( nameCopy );
	
	if ( currentStream == typedStream )
	{
		return NULL;
	}
	
	return currentStream;
}

Stream* Journal_Register( const Type type, const Name name )
{
	Stream* typedStream = NULL;
	Stream* namedStream = NULL;

	typedStream = Journal_GetTypedStream( type );
	namedStream = Journal_GetNamedStream( typedStream, name );	

	return namedStream;
}
Stream* Journal_Register2( const Type streamType, const Type componentType, const Name componentName ) {
	Stream* componentStream;
	Stream* instanceStream;
	componentStream = Journal_Register( streamType, componentType );
	instanceStream = Stream_RegisterChild( componentStream, componentName );

	return instanceStream;
}

JournalFile* Journal_GetFile( char* fileName )
{
	return (JournalFile*) Stg_ObjectList_Get( stJournal->_files, fileName );
}


void Journal_RegisterFile( JournalFile* newFile )
{
	if ( newFile == NULL )
	{
		/* Warning */
		return;
	}
	Stg_ObjectList_Append( stJournal->_files, newFile );
}

void Journal_DeregisterFile( JournalFile* file )
{
	if ( file == NULL ) {
		return;
	}
	Stg_ObjectList_Remove( stJournal->_files, file->name, KEEP );
}

void Journal_Enable_TypedStream( const Type type, Bool enable )
{
	Stream* typedStream = Journal_GetTypedStream( type );
	Stream_Enable( typedStream, enable );
}

void Journal_Enable_NamedStream( const Type type, const Name name, Bool enable )
{
	Stream* stream = Journal_Register( type, name );
	Stream_Enable( stream, enable );
}

void Journal_PrintConcise()
{
	Stream* stream;
	int i;
	
	if ( stJournal == NULL )
	{
		return;
	}

	stream = Journal_Register( Info_Type, "Journal" );

	Journal_Printf( stream, "\nSummary of Journal and Stream Status:\n" );
	Journal_Printf( stream, "Journal Enabled: %s\n", stJournal->enable ? "True" : "False" );
	
	Journal_Printf( stream, "Explanation of the following stream status printout:\n"
		"The streams are organised hierarchically, as trees, according to the\n"
		"purpose of the stream - the key stream types \"info\", \"debug\" and \"error\"\n"
		"are at the top, with all other streams as their children.\n"
		"We also display the status of each stream: the \"Enabled\" parameter, which can\n"
		"be individually set from input files/command line, determines whether output sent\n"
		"to a given stream will print or not. The \"Level\" parameter determines the level\n"
		"of detail that it will output - 1 is the default, 4 means very verbose.\n"
		"For full details on Journalling, go to https://csd.vpac.org/twiki/bin/view/Stgermain/Journal\n\n");

	Journal_Printf( stream, "Typed Streams:\n" );
	for ( i = 0; i < stJournal->_typedStreams->count; ++i )
	{
		Stream_PrintConcise( stJournal->_typedStreams->data[i], stream );
	}
	Journal_Printf( stream, "\n" );
}

int Journal_Printf ( void* _stream, char* fmt, ... )
{
	int result;
	Stream* stream = (Stream*)_stream;
	
	va_list ap;

	if ( !stJournal->enable || !Stream_IsEnable( stream ) )
	{
		return 0;
	}

	va_start( ap, fmt );
 	
	result = Stream_Printf( stream, fmt, ap );
	
	va_end(ap);
	
	return result;
}

int Journal_PrintfL( void* _stream, JournalLevel level, char* fmt, ... )
{
	int result;
	Stream* stream = (Stream*)_stream;
	
	va_list ap;

	if ( !stJournal->enable || !Stream_IsEnable( stream ) || !Stream_IsPrintableLevel( stream, level ) )
	{
		return 0;
	}

	va_start( ap, fmt );
 	
	result = Stream_Printf( stream, fmt, ap );
	
	va_end(ap);
	return result;
}
/*
int Journal_Firewall_Func( int expression, char* expressionText, 
	const char* file, const char* func, int line, void* _stream, char* fmt, ... )
*/
int Journal_Firewall( int expression, void* _stream, char* fmt, ... )
{
	int result = 0;
	Stream* stream = (Stream*)_stream;
	int nProc = 0;

	va_list ap;
	
	MPI_Comm_size( MPI_COMM_WORLD, &nProc );
	
	if ( expression )
	{
		/* Every thing is OK! Back to work as normal */
		return 0;
	}
	
	Stream_Enable( stream, True ); /* Enforce enabling of stream because we really do want to see this */

	if ( stJournal->enable && Stream_IsEnable( stream ) )
	{
		va_start( ap, fmt );
/*
 		Journal_Printf( stream, "Expression: %s\n", expressionText );
 		Journal_Printf( stream, "From File: %s\n", file ); 
 		Journal_Printf( stream, "     Function: %s\n", func );
 		Journal_Printf( stream, "     Line: %d\n", line );
*/
		result = Stream_Printf( stream, fmt, ap );
		Stream_Flush( stream );

		va_end(ap);
	}
	
	if ( stJournal->firewallProducesAssert == True ) {
		assert( expression );
	}
	else {
		/* TODO: Don't use FAILURE until Pat beef's up the test scripts to do .error checks
			exit( EXIT_FAILURE );
		*/

		// SGI MPI on the ess does not always print out everything before it exits
		// To ensure all output is displayed before the program quits on firewall, sleep for one second
		// Alan & Kath 20061006
		sleep( 1 );
		if ( nProc == 1 ) {
			exit(EXIT_SUCCESS);
		}
		else {
			MPI_Abort( MPI_COMM_WORLD, EXIT_SUCCESS );
		}
	}
	
	return result;	
}

int Journal_OptFirewall( int expression, void* _stream, const char* file, const char* func, int line, char* fmt,  ... )
{
	int result = 0;
	Stream* stream = (Stream*)_stream;
	int nProc = 0;

	va_list ap;
	
	MPI_Comm_size( MPI_COMM_WORLD, &nProc );
	
	if ( expression )
	{
		/* Every thing is OK! Back to work as normal */
		return 0;
	}
	
	Stream_Enable( stream, True ); /* Enforce enabling of stream because we really do want to see this */

	if ( stJournal->enable && Stream_IsEnable( stream ) )
	{
		va_start( ap, fmt );

		result = Stream_Printf( stream, fmt, ap );
		Stream_Flush( stream );

		va_end(ap);
	}
	
	if ( stJournal->firewallProducesAssert == True ) {
/* 		Journal_Printf( stream, "Expression: %s\n", expressionText ); */
		Journal_Printf( stream, "From File: %s\n", file );
		Journal_Printf( stream, "     Function: %s\n", func );
		Journal_Printf( stream, "     Line: %d\n", line );
		abort();
/* 		assert( expression ); */
	}
	else {
		/* TODO: Don't use FAILURE until Pat beef's up the test scripts to do .error checks
			exit( EXIT_FAILURE );
		*/

		// SGI MPI on the ess does not always print out everything before it exits
		// To ensure all output is displayed before the program quits on firewall, sleep for one second
		// Alan & Kath 20061006
		sleep( 1 );
		if ( nProc == 1 ) {
			exit(EXIT_SUCCESS);
		}
		else {
			MPI_Abort( MPI_COMM_WORLD, EXIT_SUCCESS );
		}
	}
	
	return result;	
}

SizeT Journal_Write( void* _stream, void* data, SizeT elem_size, SizeT num_elems )
{
	Stream* stream = (Stream*)_stream;

	return Stream_Write( stream, data, elem_size, num_elems );
}

Bool Journal_Dump( void* _stream, void* data )
{
	Stream* stream = (Stream*)_stream;

	return Stream_Dump( stream, data );
}

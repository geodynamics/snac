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
** $Id: Stream.c 3594 2006-05-18 06:34:43Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>

#include "Base/Foundation/Foundation.h"

#include "types.h"
#include "shortcuts.h"
#include "Journal.h"
#include "JournalFile.h"
#include "CFile.h"
#include "StreamFormatter.h"
#include "LineFormatter.h"
#include "IndentFormatter.h"
#include "Stream.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>


const Type Stream_Type = "stream";

static const int STREAM_FORMATTER_SIZE = 1;	/**< Initial size of a formatter array. */
static const int STREAM_FORMATTER_DELTA = 1;	/**< Number of formatters to resize by when extending the array. */

static const int STREAM_CHILDREN_SIZE = 4;	/**< Initial size of the sub-stream children array. */
static const int STREAM_CHILDREN_DELTA = 4;	/**< Number of streams to resize by when extending the array. */


/** Returns True if the current process is allowed to print. */
Bool _Stream_IsPrintingRank( Stream* stream );

Stream* _Stream_New(
	SizeT			_sizeOfSelf,
	Type			type,
	Stg_Class_DeleteFunction*	_delete,
	Stg_Class_PrintFunction*	_print,
	Stg_Class_CopyFunction*	_copy, 
	Name			name,
	Stream_PrintfFunction*	_printf,
	Stream_WriteFunction*	_write,
	Stream_DumpFunction*	_dump,
	Stream_SetFileFunction*	_setFile )
{
	Stream* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(Stream) );
	self = (Stream*)_Stg_Class_New( _sizeOfSelf, type, _delete, _print, _copy );
                                                                                
	_Stream_Init( self, name, _printf, _write, _dump, _setFile );

	return self;
}

void _Stream_Init(
	Stream*			self,
	Name			name,
	Stream_PrintfFunction*	_printf,
	Stream_WriteFunction*	_write,
	Stream_DumpFunction*	_dump,
	Stream_SetFileFunction*	_setFile )
{
	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );
	
	self->_printf = _printf;
	self->_write = _write;
	self->_dump = _dump;
	self->_setFile = _setFile;
	
	self->_enable = True;
	self->_level = 1;
	self->_file = NULL;
	self->_printingRank = STREAM_ALL_RANKS;
	self->_autoFlush = False;
	self->_indent = (IndentFormatter*)IndentFormatter_New();
	self->_formatter = NULL;
	self->_formatterCount = 0;
	self->_formatterSize = 0;
	self->_currentSource = NULL;
	self->_currentFunction = NULL;
	self->_currentLine = 0;
	
	self->_parent = NULL;
	self->_children = Stg_ObjectList_New2( STREAM_CHILDREN_SIZE, STREAM_CHILDREN_DELTA );
	self->_children->_noJournalingInCopy = True; /* Important... else _Stream_Copy ends up stuck in a recursive copy loop */
}


void _Stream_Delete( void* stream )
{
	int i;
	
	Stream* self = (Stream*)stream;
	
	#if DEBUG
		assert( self );
	#endif

	Stg_Class_Delete( self->_indent );

	if ( self->_formatter != NULL )
	{
		for ( i = 0; i < self->_formatterCount; ++i )
		{
			Stg_Class_Delete( self->_formatter[i] );
		}
		Memory_Free( self->_formatter );
	}

	Stg_ObjectList_DeleteAllObjects( self->_children );
	Stg_Class_Delete( self->_children );

	/* Stg_Class_Delete parent */
	_Stg_Object_Delete( self );
}


void _Stream_Print( void* stream, Stream* paramStream )
{
	Stream* self = (Stream*) stream;

	#if DEBUG
		assert( self );
	#endif
	
	/* General info */
	Journal_Printf( paramStream, "Stream (ptr): %p\n", self );
	
	/* Print parent */
	_Stg_Object_Print( self, paramStream );
	
	Stream_Indent( paramStream );
	/* Virtual info */
	Journal_Printf( paramStream, "_printf (func ptr): %p\n", self->_printf );
	Journal_Printf( paramStream, "_write (func ptr): %p\n", self->_write );
	Journal_Printf( paramStream, "_dump (func ptr): %p\n", self->_dump );
	Journal_Printf( paramStream, "_setFile (func ptr): %p\n", self->_setFile );
	Journal_Printf( paramStream, "_copy (func ptr): %p\n", self->_copy );
	
	/* Stream info */
	Journal_Printf( paramStream, "_enable: %d\n", self->_enable );
	Journal_Printf( paramStream, "_level: %d\n", self->_level );
	Journal_Printf( paramStream, "_file (ptr):  %p\n", self->_file );
	Journal_Printf( paramStream, "_printingRank: %d\n", self->_printingRank );
	Journal_Printf( paramStream, "_autoFlush: %d\n", self->_autoFlush );
	Journal_Printf( paramStream, "_indent: %d\n", self->_indent->_indent );
	Journal_Printf( paramStream, "_formatters (ptr):  %p\n", self->_formatter );
	Journal_Printf( paramStream, "_formatter count: %d\n", self->_formatterCount );
	Journal_Printf( paramStream, "_formatter size: %d\n", self->_formatterSize );
	/*
	Journal_Printf( paramStream, "current source file: %s\n", self->_currentSource );
	Journal_Printf( paramStream, "current function: %s\n", self->_currentFunction );
	Journal_Printf( paramStream, "current line: %d\n", self->_currentLine );
	*/
	Journal_Printf( paramStream, "_parent (ptr): %p\n", self->_parent );

	Stream_Indent( paramStream );

	Print( self->_children, paramStream );

	Stream_UnIndent( paramStream );

	Stream_UnIndent( paramStream );
}


void Stream_PrintConcise( void* stream, Stream* paramStream )
{
	Stream* self = (Stream*) stream;
	Index   child_I = 0;

	#if DEBUG
		assert( self );
	#endif
	
	/* General info */
	Journal_Printf( paramStream, "Stream \"%s\":\n", self->name );
	
	Stream_Indent( paramStream );
	
	/* Stream info */
	Journal_Printf( paramStream, "Enabled: %s\n", self->_enable ? "True" : "False" );
	Journal_Printf( paramStream, "Level: %d\n", self->_level );

	if ( self->_children->count > 0 ) {
		Journal_Printf( paramStream, "Children:\n" );
		Stream_Indent( paramStream );
		for ( child_I = 0; child_I < self->_children->count; child_I++ ) {
			Stream_PrintConcise( self->_children->data[child_I], paramStream );
		}
		Stream_UnIndent( paramStream );
	}

	Stream_UnIndent( paramStream );
}


void* _Stream_Copy( void* stream, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap ) {
	Stream*		self = (Stream*)stream;
	Stream*		newStream;
	unsigned	i;
	
	newStream = _Stg_Object_Copy( self, dest, deep, nameExt, ptrMap );
	
	newStream->_printf = self->_printf;
	newStream->_write = self->_write;
	newStream->_dump = self->_dump;
	newStream->_setFile = self->_setFile;
	newStream->defaultFileType = self->defaultFileType;
	newStream->_copy = self->_copy;
	newStream->_enable = self->_enable;
	newStream->_level = self->_level;
	newStream->_file = self->_file;
	newStream->_printingRank = self->_printingRank;
	newStream->_autoFlush = self->_autoFlush;
	newStream->_indent = (IndentFormatter*)IndentFormatter_New();
	newStream->_formatter = NULL;
	newStream->_formatterCount = 0;
	newStream->_formatterSize = 0;
	newStream->_currentSource = NULL;
	newStream->_currentFunction = NULL;
	newStream->_currentLine = 0;
	newStream->_parent = self->_parent;
/*	newStream->_children = Stg_Class_Copy( self->_children, 0 , deep, nameExt, ptrMap );*/
	newStream->_children = Stg_ObjectList_New2( STREAM_CHILDREN_SIZE, STREAM_CHILDREN_DELTA );

	
	IndentFormatter_SetIndent( newStream->_indent, self->_indent->_indent );
	for ( i = 0; i < self->_formatterCount; ++i ) {
		Stream_AddFormatter( newStream, Stg_Class_Copy( self->_formatter[i], 0, deep, nameExt, ptrMap ) );
		/**set stream for formatter */
	}
	
	return newStream;
}


SizeT Stream_Printf( Stream *stream, char *fmt, va_list args )
{
	int i;
	SizeT result;
	char* lastFormat;
	char* currentFormat;

	if ( stream->_file == NULL ) {
		return 0;
	}
	if ( !_Stream_IsPrintingRank( stream ) )
	{
		return 0;
	}

	JournalFile_ReopenIfClosed( stream->_file );

	currentFormat = StreamFormatter_Format( stream->_indent, fmt );
	lastFormat = currentFormat;

	for ( i = 0; i < stream->_formatterCount; ++i )
	{
		currentFormat = StreamFormatter_Format( stream->_formatter[i], lastFormat );
		lastFormat = currentFormat;
	}
	result = stream->_printf( stream, currentFormat, args );

	if ( stream->_autoFlush )
	{
		Stream_Flush( stream );
	}
	return result;
}
SizeT Stream_Write( Stream *stream, void *data, SizeT elem_size, SizeT num_elems )
{
	SizeT result;

	if ( stream->_file == NULL ) {
		return 0;
	}
	if ( !_Stream_IsPrintingRank( stream ) )
	{
		return 0;
	}
	
	JournalFile_ReopenIfClosed( stream->_file );

	result =  stream->_write( stream, data, elem_size, num_elems );

	if( stream->_autoFlush )
	{
		Stream_Flush( stream );
	}
	return result;
}
Bool Stream_Dump( Stream *stream, void *data )
{
	Bool result;
	
	if ( stream->_file == NULL ) {
		return False;
	}
	if ( !_Stream_IsPrintingRank( stream ) )
	{
		return False;
	}

	JournalFile_ReopenIfClosed( stream->_file );

	result = stream->_dump( stream, data );

	if ( stream->_autoFlush )
	{
		Stream_Flush( stream );
	}

	return result;
}

Bool Stream_RedirectFile( Stream* stream, char* fileName ) {
	JournalFile* file;

	file = Journal_GetFile( fileName );
	if ( file == NULL ) {
		file = stream->defaultFileType();
		if ( !JournalFile_Open( file, fileName ) ) {
			Stg_Class_Delete( file );
			return False;
		}
		Journal_RegisterFile( file );
	}

	return Stream_SetFile( stream, file );
}


Bool Stream_RedirectFileBranch( Stream* stream, char* fileName ) {
	JournalFile* file;

	file = Journal_GetFile( fileName );
	if ( file == NULL ) {
		file = stream->defaultFileType();
		JournalFile_Open( file, fileName );
		Journal_RegisterFile( file );
	}

	return Stream_SetFileBranch( stream, file );
}

Bool Stream_AppendFile( Stream* stream, char* fileName ) {
	JournalFile* file;

	file = Journal_GetFile( fileName );
	if ( file == NULL ) {
		file = stream->defaultFileType();
		if ( !JournalFile_Append( file, fileName ) ) {
			Stg_Class_Delete( file );
			return False;
		}
		Journal_RegisterFile( file );
	}

	return Stream_SetFile( stream, file );
}

Bool Stream_SetFile( Stream* stream, JournalFile* file )
{
	if ( file == NULL )
	{
		return False;
	}
	
	return stream->_setFile( stream, file );
}
Bool Stream_SetFileBranch( Stream* stream, JournalFile* file )
{
	Bool result = False;
	int i;
	
	if ( file == NULL )
	{
		return False;
	}
	
	result = stream->_setFile( stream, file );
	for ( i = 0; result && i < stream->_children->count; ++i )
	{
		result = Stream_SetFileBranch( (Stream*)stream->_children->data[i], file );
		if ( !result )
		{
			return False;
		}
	}
	return result;
}

Bool Stream_IsEnable( void* stream )
{
	Stream* self = (Stream*) stream;
	if (stream == NULL) return False;
	
	if ( self->_parent == NULL )
	{
		return self->_enable;
	}
	
	return self->_enable && Stream_IsEnable( self->_parent );
}

void Stream_Enable( void* stream, Bool enable )
{
	Stream* self = (Stream*) stream;
	self->_enable = enable;
	
	/* set path of enable is only set when true */	
	if ( enable && self->_parent != NULL )
	{
		Stream_Enable( self->_parent, True );
	}
}
void Stream_EnableBranch( void* stream, Bool enable )
{
	int i;
	Stream* self = (Stream*) stream;
	
	self->_enable = enable;
	
	for ( i = 0; i < self->_children->count; ++i )
	{
		Stream_EnableBranch( (Stream*)self->_children->data[i], enable );
	}
}
void Stream_EnableSelfOnly( void* stream, Bool enable )
{
	Stream* self = (Stream*) stream;
	self->_enable = enable;
}

void Stream_SetPrintingRank( void* stream, int rank )
{
	Stream* self = (Stream*) stream;
	self->_printingRank = rank;
}

int Stream_GetPrintingRank( void* stream )
{
	Stream* self = (Stream*) stream;
	return self->_printingRank;
}

void Stream_SetPrintingRankBranch( void* stream, int rank )
{
	int i;
	Stream* self = (Stream*) stream;
	
	self->_printingRank = rank;
	
	for ( i = 0; i < self->_children->count; ++i )
	{
		Stream_SetPrintingRankBranch( self->_children->data[i], rank );
	}
}

void Stream_SetAutoFlush( void* stream, Bool autoFlush )
{
	Stream* self = (Stream*) stream;
	self->_autoFlush = autoFlush;
}

void Stream_SetAutoFlushBranch( void* stream, Bool autoFlush )
{
	int i;
	Stream* self = (Stream*) stream;
	
	self->_autoFlush = autoFlush;
	
	for ( i = 0; i < self->_children->count; ++i )
	{
		Stream_SetAutoFlushBranch( self->_children->data[i], autoFlush );
	}
}

Bool Stream_IsPrintableLevel( void* stream, JournalLevel level )
{
	Stream* self = (Stream*) stream;
	return (self->_level >= level);
}
	
void Stream_SetLevel( void* stream, JournalLevel level )
{
	Stream* self = (Stream*) stream;
	self->_level = level;	
}
void Stream_SetLevelBranch( void* stream, JournalLevel level )
{
	int i;
	Stream* self = (Stream*) stream;
	
	self->_level = level;	
	for ( i = 0; i < self->_children->count; ++i )
	{
		Stream_SetLevelBranch( (Stream*)self->_children->data[i], level );
	}

}

void Stream_SetIndent( void* stream, Index indent )
{
	Stream* self = (Stream*) stream;
	IndentFormatter_SetIndent( self->_indent, indent );
}

void Stream_Indent( void* stream )
{
	Stream* self = (Stream*) stream;
	IndentFormatter_Increment( self->_indent );
}

void Stream_UnIndent( void* stream )
{
	Stream* self = (Stream*) stream;
	IndentFormatter_Decrement( self->_indent );
}

void Stream_ZeroIndent( void* stream )
{
	Stream* self = (Stream*) stream;
	IndentFormatter_Zero( self->_indent );
}


void Stream_SetIndentBranch( void* stream, Index indent )
{
	int i;
	Stream* self = (Stream*) stream;
	
	IndentFormatter_SetIndent( self->_indent, indent );

	for ( i = 0; i < self->_children->count; ++i )
	{
		Stream_SetIndentBranch( self->_children->data[i], indent );
	}
}
void Stream_IndentBranch( void* stream )
{
	int i;
	Stream* self = (Stream*) stream;
	
	IndentFormatter_Increment( self->_indent );

	for ( i = 0; i < self->_children->count; ++i )
	{
		Stream_IndentBranch( self->_children->data[i] );
	}
}
void Stream_UnIndentBranch( void* stream )
{
	int i;
	Stream* self = (Stream*) stream;
	
	IndentFormatter_Decrement( self->_indent );

	for ( i = 0; i < self->_children->count; ++i )
	{
		Stream_UnIndentBranch( self->_children->data[i] );
	}
}
void Stream_ZeroIndentBranch( void* stream )
{
	int i;
	Stream* self = (Stream*) stream;
	
	IndentFormatter_Zero( self->_indent );

	for ( i = 0; i < self->_children->count; ++i )
	{
		Stream_ZeroIndentBranch( self->_children->data[i] );
	}
}

void Stream_AddFormatter( void* stream, StreamFormatter* formatter )
{
	Stream* self = (Stream*) stream;
	
	if ( self->_formatterCount >= self->_formatterSize )
	{
		if ( self->_formatter == NULL )
		{
			self->_formatterSize = STREAM_FORMATTER_SIZE;
			self->_formatter = Memory_Alloc_Array( StreamFormatter*, self->_formatterSize, "Stream->_formatter" );
		}
		else
		{
			self->_formatterSize += STREAM_FORMATTER_DELTA;
			self->_formatter = Memory_Realloc_Array( self->_formatter, StreamFormatter*, self->_formatterSize );
		}
	}
	self->_formatter[self->_formatterCount] = formatter;
	self->_formatterCount++;
	
	formatter->_stream = self;
}

void Stream_SetCurrentInfo( void* stream, char* currentSource, char* currentFunction, int line )
{
	Stream* self = (Stream*)stream;

	self->_currentSource = currentSource;
	self->_currentFunction = currentFunction;
	self->_currentLine = line;
}


Stream* Stream_RegisterChild( void* stream, Name name )
{
	Stream* self = (Stream*) stream;
	Stream* child;
	
	child = (Stream*)Stg_ObjectList_Get( self->_children, name );
	
	if ( child == NULL )
	{
		child = Stg_Class_Copy( self, 0, True, 0, 0 );
		Stg_Object_SetName( child, name );
		child->_parent = self;
		Stg_ObjectList_Append( self->_children, child );
	}
	
	return child;
}

Bool Stream_Flush( void* stream )
{
	Stream* self = (Stream*)stream;
	return JournalFile_Flush( self->_file );
}

Bool Stream_CloseFile( void* stream ) {
	Stream* self = (Stream*)stream;

	return JournalFile_Close( self->_file );
}

void Stream_CloseAndFreeFile( void* stream ) {
	Stream* self = (Stream*)stream;

	Stream_CloseFile( self );
	Journal_DeregisterFile( self->_file );
	Stg_Class_Delete( self->_file );
}


Bool _Stream_IsPrintingRank( Stream* stream )
{
	int rank;
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	
	if ( stream->_printingRank > STREAM_ALL_RANKS && rank != stream->_printingRank )
	{
		return False;
	}

	return True;
}

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
** $Id: MPIStream.c 2807 2005-03-17 00:24:43Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>

#include "Base/Foundation/Foundation.h"

#include "types.h"
#include "shortcuts.h"
#include "JournalFile.h"
#include "MPIFile.h"
#include "Stream.h"
#include "MPIStream.h"

#include "Base/IO/mpirecord/mpimessaging.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <stdarg.h>   /* Subsequent files need this for variable argument lists. */

#include "Journal.h"


const Type MPIStream_Type = "MPIStream";


Stream* MPIStream_New( Name name )
{
	return (Stream*)_MPIStream_New( sizeof(MPIStream), MPIStream_Type, _MPIStream_Delete, _MPIStream_Print, _Stream_Copy, 
		name, _MPIStream_Printf, _MPIStream_Write, _MPIStream_Dump, _MPIStream_SetFile );
}

void MPIStream_Init( MPIStream* self, Name name )
{
	
}


MPIStream* _MPIStream_New( 
	SizeT			_sizeOfSelf, 
	Type			type, 
	Stg_Class_DeleteFunction*	_delete, 
	Stg_Class_PrintFunction* 	_print,
	Stg_Class_CopyFunction*	_copy, 
	Name			name,
	Stream_PrintfFunction*	_printf, 
	Stream_WriteFunction*	_write, 
	Stream_DumpFunction*	_dump,
	Stream_SetFileFunction*	_setFile )
{
	MPIStream* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(MPIStream) );
	self = (MPIStream*)_Stream_New( _sizeOfSelf, type, _delete, _print, _copy, name, 
		_printf, _write, _dump, _setFile );
	
	_MPIStream_Init( self );
	
	return self;
}

void _MPIStream_Init( MPIStream* self )
{
	self->defaultFileType = MPIFile_New;
}
	
void _MPIStream_Delete( void* cStream )
{
	MPIStream* self = (MPIStream*)cStream;
	
	/* Stg_Class_Delete parent */
	_Stream_Delete( self );
}

void _MPIStream_Print( void* mpiStream, Stream* stream ) {

	MPIStream* self = (MPIStream*)mpiStream;
	
	/* General info */
	Journal_Printf( stream, "MPIStream (ptr): %p\n", mpiStream );
	
	/* Print parent */
	_Stream_Print( self, stream );
		
}
	
SizeT _MPIStream_Printf( Stream* stream, char *fmt, va_list args )
{
	MPIStream* self = (MPIStream*)stream;
	MPI_Status status;
	char* buffer;
	SizeT numChars;
	SizeT result;
	
	if ( self->_file == NULL )
	{
		return 0;
	}

	numChars = Stg_vasprintf( &buffer, fmt, args );

	result = MPI_File_write( *(MPI_File*)(self->_file->fileHandle), buffer, numChars, MPI_BYTE, &status );

	Memory_Free( buffer );
	
	return result;
}
	
SizeT _MPIStream_Write( Stream* stream, void *data, SizeT elem_size, SizeT num_elems )
{
	MPIStream* self = (MPIStream*)stream;
	MPI_Status status;
	
	return MPI_File_write( *(MPI_File*)(self->_file->fileHandle), data, num_elems * elem_size, MPI_BYTE, &status );
}
	
Bool _MPIStream_Dump( Stream* stream, void *data )
{
	/* No specific dumping mechanism, can create in derived classes */
	return False;
}

Bool _MPIStream_SetFile( Stream* stream, JournalFile* file )
{
	if ( file->type == MPIFile_Type )
	{
		stream->_file = file;
		return True;
	}
	return False;
}

#define MPIStream_OffsetTag 167

Bool MPIStream_SetOffset( Stream* stream, SizeT sizeToWrite, MPI_Comm communicator ) {
	MPI_Offset offset    = 0;
	MPI_Offset endOffset = 0;
	MPI_Status status;
	int        rank;
	int        nproc;
	
	if ( stream->_file == NULL ) {
		return False;
	}

	if ( stream->_file->type != MPIFile_Type ) {
		return False;
	}
	
	MPI_Comm_rank( communicator, &rank );
	MPI_Comm_size( communicator, &nproc );

	/* Receive offset from guy on left to find out what point I want to write in the file */
	if ( rank != 0 ) {
		MPI_Recv( &offset, sizeof(MPI_Offset), MPI_BYTE, rank - 1, MPIStream_OffsetTag, communicator, &status );
	}

	/* Send offset to guy on right */
	endOffset = offset + sizeToWrite;
	if ( rank != nproc - 1 ) {
		MPI_Send( &endOffset, sizeof(MPI_Offset), MPI_BYTE, rank + 1, MPIStream_OffsetTag, communicator );
	}
	
	MPI_File_seek( *(MPI_File*)stream->_file->fileHandle, offset, MPI_SEEK_SET ); 
	
	return True;
}

SizeT MPIStream_WriteAllProcessors( Stream* stream, void *data, SizeT elem_size, SizeT num_elems, MPI_Comm communicator ) {
	SizeT sizeToWrite = elem_size * num_elems;

	MPIStream_SetOffset( stream, sizeToWrite, communicator );

	return Stream_Write( stream, data, elem_size, num_elems );
}

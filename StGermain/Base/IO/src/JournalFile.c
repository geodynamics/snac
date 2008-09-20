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
** $Id: JournalFile.c 3573 2006-05-15 07:51:14Z AlanLo $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "Base/Foundation/Foundation.h"

#include "types.h"
#include "shortcuts.h"
#include "JournalFile.h"
#include "Journal.h"
#include "Stream.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

const Type JournalFile_Type = "JournalFile";


JournalFile* _JournalFile_New(
	SizeT				_sizeOfSelf,
	Type				type,
	Stg_Class_DeleteFunction*	_delete,
	Stg_Class_PrintFunction*	_print,
	Stg_Class_CopyFunction*		_copy, 
	JournalFile_OpenFunction*	_open,
	JournalFile_AppendFunction*	_append,
	JournalFile_CloseFunction*	_close,
	JournalFile_FlushFunction*	_flush )
{
	JournalFile* self;
	
	assert( _sizeOfSelf >= sizeof(JournalFile) );
	self = (JournalFile*)_Stg_Class_New( _sizeOfSelf, type, _delete, _print, _copy );
	
	_JournalFile_Init( self, _open, _append, _close, _flush );
	
	return self;
}
	
void _JournalFile_Init(
	JournalFile*			self,
	JournalFile_OpenFunction*	_open,
	JournalFile_AppendFunction*	_append,
	JournalFile_CloseFunction*	_close,
	JournalFile_FlushFunction*	_flush )
{
	_Stg_Object_Init( (Stg_Object*)self, "", NON_GLOBAL );
	
	self->_open = _open;
	self->_append = _append;
	self->_close = _close;
	self->_flush = _flush;
	
	self->fileHandle = NULL;
	self->_opened = False;
}

void _JournalFile_Delete( void* journalFile )
{
	_Stg_Object_Delete( journalFile );
}
void _JournalFile_Print( void* journalFile, Stream* stream )
{
	JournalFile* self = (JournalFile*)journalFile;
                                                                                
        Journal_Printf( stream, "JournalFile (ptr): %p\n", self );
         
        _Stg_Object_Print( self, stream );
         
        Journal_Printf( stream, "\tfileHandle (ptr): %p\n", self->fileHandle );
}

Bool JournalFile_Open( void* file, char* fileName )
{
	JournalFile* self = (JournalFile*)file;
	
	Bool result = self->_open( self, fileName );
	
	if ( result )
	{
		/* File opened, set the name. */
		Stg_Object_SetName( self, fileName );
		self->_opened = True;
	}
	
	return result;
}
Bool JournalFile_Reopen( void* journalFile ) {
	JournalFile* self = (JournalFile*)journalFile;

	Bool result;

	if ( self->_opened ) {
		return True;
	}
	result = self->_open( self, self->name );
	if ( result ) {
		self->_opened = True;
	}
	return result;
}

Bool JournalFile_Append( void* file, char* fileName ) {
	JournalFile* self = (JournalFile*)file;
	
	Bool result = self->_append( self, fileName );
	
	if ( result )
	{
		/* File opened, set the name. */
		Stg_Object_SetName( self, fileName );
		self->_opened = True;
	}
	
	return result;
}
Bool JournalFile_Close( void* file )
{
	JournalFile* self = (JournalFile*)file;
	Bool result = False;
	if ( self->_opened ) {
		result = self->_close( file );
		if ( result ) {
			self->_opened = False;
		}
	}
	return result;
}
Bool JournalFile_IsOpened( void* journalFile ) {
	JournalFile* self = (JournalFile*)journalFile;
	return self->_opened;
}

Bool JournalFile_Flush( void* file )
{
	JournalFile* self = (JournalFile*)file;
	stJournal->flushCount++;
	return self->_flush( self );
}

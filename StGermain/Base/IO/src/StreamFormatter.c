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
** $Id: StreamFormatter.c 3774 2006-08-31 07:39:32Z KathleenHumble $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "Base/Foundation/Foundation.h"

#include "types.h"
#include "shortcuts.h"
#include "Journal.h"
#include "StreamFormatter.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

const Type StreamFormatter_Type = "StreamFormatter";

StreamFormatter* _StreamFormatter_New(
	SizeT 				_sizeOfSelf,
	Type 				type,
	Stg_Class_DeleteFunction*		_delete,
	Stg_Class_PrintFunction*		_print,
	Stg_Class_CopyFunction*		_copy, 
	StreamFormatter_FormatFunction*	_format )
{
	StreamFormatter* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(StreamFormatter) );
	self = (StreamFormatter*)_Stg_Class_New( _sizeOfSelf, type, _delete, _print, _copy );
                                                                                
	_StreamFormatter_Init( self, _format );

	return self;
}
void _StreamFormatter_Init(
	StreamFormatter*		self,
	StreamFormatter_FormatFunction*	_format )
{
	self->_format = _format;
	
	self->_stream = NULL;
}

void _StreamFormatter_Delete( void* formatter )
{
	StreamFormatter* self = (StreamFormatter*)formatter;
	
	#if DEBUG
		assert( self );
	#endif

	/* Stg_Class_Delete parent */
	_Stg_Class_Delete( self );
}
void _StreamFormatter_Print( void* formatter, Stream* stream )
{
	StreamFormatter* self = (StreamFormatter*) formatter;

	#if DEBUG
		assert( self );
		assert( stream );
	#endif
	
	/* General info */
	printf( "StreamFormatter (ptr): %p\n", formatter );
	
	/* Print parent */
	_Stg_Class_Print( self, stream );
	
	/* Virtual info */
	printf( "\t_format (func ptr): %p\n", (void*)self->_format );
	printf( "\t_copy (func ptr): %p\n", (void*)self->_copy );
	printf( "\t_stream (ptr): %p\n", (void*)self->_stream );
}

void* _StreamFormatter_Copy( void* streamFormatter, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap ) {
	StreamFormatter*	self = (StreamFormatter*)streamFormatter;
	StreamFormatter*	newStreamFormatter;
	
	newStreamFormatter = _Stg_Class_Copy( self, dest, deep, nameExt, ptrMap );
	
	newStreamFormatter->_format = self->_format;
	newStreamFormatter->_stream = NULL;
	
	return newStreamFormatter;
}


char* StreamFormatter_Format( void* formatter, char* fmt )
{
	StreamFormatter* self = (StreamFormatter*)formatter;
	return self->_format( self, fmt );
}


StreamFormatter_Buffer* stgStreamFormatter_Buffer;

StreamFormatter_Buffer* StreamFormatter_Buffer_New() {
	StreamFormatter_Buffer* result = Memory_Alloc( StreamFormatter_Buffer, "StreamFormatter_Buffer" );
	result->buffer1 = NULL;
	result->buffer2 = NULL;
	result->length1 = 0;
	result->length2 = 0;
	result->current = &result->buffer1;
	result->lengthPtr = &result->length1;

	return result;
}
void StreamFormatter_Buffer_Delete( StreamFormatter_Buffer* buffer ) {
	if ( buffer->buffer1 != NULL ) {
		Memory_Free( buffer->buffer1 );
	}
	if ( buffer->buffer2 != NULL ) {
		Memory_Free( buffer->buffer2 );
	}

	Memory_Free( buffer );
}
char* StreamFormatter_Buffer_AllocNext( Index size ) {

	/* Buffer swap */
	if ( stgStreamFormatter_Buffer->current == &stgStreamFormatter_Buffer->buffer1 ) {
		stgStreamFormatter_Buffer->current = &stgStreamFormatter_Buffer->buffer2;
		stgStreamFormatter_Buffer->lengthPtr = &stgStreamFormatter_Buffer->length2;
	}
	else {
		stgStreamFormatter_Buffer->current = &stgStreamFormatter_Buffer->buffer1;
		stgStreamFormatter_Buffer->lengthPtr = &stgStreamFormatter_Buffer->length1;
	}

	/* Realloc/alloc as needed */
	if ( size > *stgStreamFormatter_Buffer->lengthPtr ) {
		*stgStreamFormatter_Buffer->lengthPtr = size;
		*stgStreamFormatter_Buffer->current = (char*)Memory_Realloc( *stgStreamFormatter_Buffer->current, size );
	}

	return *stgStreamFormatter_Buffer->current;
}

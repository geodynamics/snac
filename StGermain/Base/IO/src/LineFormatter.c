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
** $Id: LineFormatter.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "Base/Foundation/Foundation.h"

#include "types.h"
#include "shortcuts.h"
#include "StreamFormatter.h"
#include "LineFormatter.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <mpi.h>
#include <stdarg.h>  


const Type LineFormatter_Type = "LineFormatter";

LineFormatter* _LineFormatter_New(
	SizeT 				_sizeOfSelf,
	Type 				type,
	Stg_Class_DeleteFunction*		_delete,
	Stg_Class_PrintFunction*		_print,
	Stg_Class_CopyFunction*		_copy, 
	StreamFormatter_FormatFunction*	_format )
{
	LineFormatter* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(LineFormatter) );
	self = (LineFormatter*)_Stg_Class_New( _sizeOfSelf, type, _delete, _print, _copy );
                                                                                
	_LineFormatter_Init( self, _format );

	return self;
}

void _LineFormatter_Init(
	LineFormatter*			self,
	StreamFormatter_FormatFunction*	_format )
{
	_StreamFormatter_Init( (StreamFormatter*)self, _format );
	self->_newLine = True;
	self->_tag = NULL;
}

void _LineFormatter_Delete( void* formatter )
{
	LineFormatter* self = (LineFormatter*)formatter;
	
	#if DEBUG
		assert( self );
	#endif

	if ( self->_tag != NULL )
	{
		Memory_Free( self->_tag );
	}

	/* Stg_Class_Delete parent */
	_StreamFormatter_Delete( self );
}
void _LineFormatter_Print( void* formatter, Stream* stream )
{
	LineFormatter* self = (LineFormatter*) formatter;

	#if DEBUG
		assert( self );
		assert (stream);
	#endif
	
	/* General info */
	printf( "LineFormatter (ptr): %p\n", formatter );
	
	/* Print parent */
	_Stg_Class_Print( self, stream );
	
	/* print _newLine and _tag */
}

void* _LineFormatter_Copy( void* lineFormatter, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap ) {
	LineFormatter*	self = (LineFormatter*)lineFormatter;
	LineFormatter*	newLineFormatter;
	
	/* Create new instance. Copy virtual info */
	newLineFormatter = _StreamFormatter_Copy( self, dest, deep, nameExt, ptrMap );
	
	/* Copy member info. */
	newLineFormatter->_newLine = self->_newLine;
	if ( self->_tag != NULL ) {
		newLineFormatter->_tag = StG_Strdup( self->_tag );
	}
	else {
		newLineFormatter->_tag = NULL;
	}
	
	return newLineFormatter;
}

char* _LineFormatter_Format( void* formatter, char* fmt )
{
	LineFormatter* self = (LineFormatter*)formatter;
	char* result;
	char* currentLine;
	int numInsert;
	int fmtLength;
	int prependLength;
	int newLength;

			
	char* destPtr;
	char* srcPtr;
		
	#if DEBUG
		assert( self );
	#endif

	if ( self->_tag == NULL )
	{
		/* No tag to add, so return same format */
		return fmt;
	}

	/* count number of inserts */
	numInsert = 0;
	currentLine = strchr( fmt, '\n' );
	while ( currentLine != NULL )
	{
		numInsert++;	
		currentLine = strchr( currentLine + 1, '\n' );
	}
	if ( self->_newLine )
	{
		numInsert++;
	}
	
	fmtLength = strlen( fmt );
	if ( fmtLength > 0 && fmt[fmtLength - 1] == '\n' )
	{
		numInsert--;
	}

	prependLength = strlen( self->_tag );
	
	/* Calculate length of new string */
	if ( numInsert > 0 )
	{
		newLength = fmtLength + ( prependLength * numInsert );
	}
	else
	{
		newLength = fmtLength;
	}
	
/*	result = Memory_Alloc_Array( char, newLength + 1, Memory_IgnoreName );*/
	result = StreamFormatter_Buffer_AllocNext( newLength + 1 );
	
	destPtr = result;
	srcPtr = fmt;
	
	/* If we were upto a new line, begin with a prepend. */
	if ( self->_newLine )
	{
		sprintf( destPtr, "%s", self->_tag );
		destPtr += prependLength;
	}
	
	self->_newLine = False;
	
	while ( *srcPtr != '\0' )
	{
		*destPtr = *srcPtr;
		
		destPtr++;
		
		if ( *srcPtr == '\n' )
		{
			/* if not the last endline */
			if ( (srcPtr - fmt) != (fmtLength - 1) )
			{
				sprintf( destPtr, "%s", self->_tag );
				destPtr += prependLength;
			}
			else
			{
				/* If we are at the last line, and it was a new line, prepend the next time this is called. */
				self->_newLine = True;
			}
		}
		srcPtr++;
	}
	*destPtr = '\0';
	
	return result;	
}


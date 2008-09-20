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
** $Id: IndentFormatter.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "Base/Foundation/Foundation.h"

#include "types.h"
#include "shortcuts.h"
#include "StreamFormatter.h"
#include "LineFormatter.h"
#include "IndentFormatter.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <stdarg.h>   /* subsequent files need this */


const Type IndentFormatter_Type = "IndentFormatter";
const char IndentFormatter_IndentChar = '\t';

/** Makes the string which forms the identation for printing. */
void IndentFormatter_MakeTag( IndentFormatter* formatter );


StreamFormatter* IndentFormatter_New()
{
	return (StreamFormatter*)_IndentFormatter_New( sizeof(IndentFormatter), IndentFormatter_Type,
		_IndentFormatter_Delete, _IndentFormatter_Print, _IndentFormatter_Copy, _LineFormatter_Format,
		IndentFormatter_IndentChar );
}

StreamFormatter* IndentFormatter_New2( char _character )
{
	return (StreamFormatter*)_IndentFormatter_New( sizeof(IndentFormatter), IndentFormatter_Type,
		_IndentFormatter_Delete, _IndentFormatter_Print, _IndentFormatter_Copy, _LineFormatter_Format,
		_character );
}

void IndentFormatter_Init( IndentFormatter* self )
{
	self->_sizeOfSelf = sizeof(IndentFormatter);
	self->type = IndentFormatter_Type;
	self->_delete = _IndentFormatter_Delete;
	self->_print = _IndentFormatter_Print;
	self->_copy = _IndentFormatter_Copy;
	
	_IndentFormatter_Init( self, _LineFormatter_Format, IndentFormatter_IndentChar );
}

IndentFormatter* _IndentFormatter_New(
	SizeT 				_sizeOfSelf,
	Type 				type,
	Stg_Class_DeleteFunction*		_delete,
	Stg_Class_PrintFunction*		_print,
	Stg_Class_CopyFunction*		_copy, 
	StreamFormatter_FormatFunction*	_format,
	char				_character )
{
	IndentFormatter* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(IndentFormatter) );
	self = (IndentFormatter*)_Stg_Class_New( _sizeOfSelf, type, _delete, _print, _copy );
                                                                                
	_IndentFormatter_Init( self, _format, _character );

	return self;
}

void _IndentFormatter_Init(
	IndentFormatter*		self,
	StreamFormatter_FormatFunction*	_format,
	char				_character )
{
	_LineFormatter_Init( (LineFormatter*)self, _format );
	
	self->_indent = 0;
	self->_character = _character;
}

void _IndentFormatter_Delete( void* formatter )
{
	IndentFormatter* self = (IndentFormatter*)formatter;
	
	#if DEBUG
		assert( self );
	#endif

	/* Stg_Class_Delete parent */
	_LineFormatter_Delete( self );
}
void _IndentFormatter_Print( void* formatter, Stream* stream )
{
	/*IndentFormatter* self = (IndentFormatter*) formatter;*/

	#if DEBUG
		assert( formatter );
		assert( stream );
	#endif
	
	/* General info */
	printf( "IndentFormatter (ptr): %p\n", formatter );
	
	_LineFormatter_Print( formatter, stream );
}

void* _IndentFormatter_Copy( void* indentFormatter, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap ) {
	IndentFormatter*	self = (IndentFormatter*)indentFormatter;
	IndentFormatter*	newIndentFormatter;
	
	newIndentFormatter = _LineFormatter_Copy( self, dest, deep, nameExt, ptrMap );
	
	IndentFormatter_SetIndent( newIndentFormatter, self->_indent );
	
	return newIndentFormatter;
}

void IndentFormatter_SetIndent( void* formatter, Index indent )
{
	IndentFormatter* self = (IndentFormatter*)formatter;
	
	self->_indent = indent;
	IndentFormatter_MakeTag( self );
}

void IndentFormatter_Increment( void* formatter )
{
	IndentFormatter* self = (IndentFormatter*)formatter;
	
	self->_indent++;
	IndentFormatter_MakeTag( self );
}

void IndentFormatter_Decrement( void* formatter )
{
	IndentFormatter* self = (IndentFormatter*)formatter;
	
	self->_indent = ( self->_indent <= 0 ) ? 0 : self->_indent - 1;
	IndentFormatter_MakeTag( self );
}

void IndentFormatter_Zero( void* formatter)
{
	IndentFormatter* self = (IndentFormatter*)formatter;
	
	self->_indent = 0;
	IndentFormatter_MakeTag( self );
}

void IndentFormatter_MakeTag( IndentFormatter* formatter )
{
	int i;
	
	/* Release previous tag if existing. */
	if ( formatter->_tag != NULL )
	{
		Memory_Free( formatter->_tag );
	}
	
	/* Stop if no indentation. */
	if ( formatter->_indent <= 0 )
	{
		formatter->_tag = NULL;
		return;
	}
	
	/* Produce tag. */
	formatter->_tag = Memory_Alloc_Array( char, formatter->_indent + 1, Memory_IgnoreName );
	for ( i = 0; i < formatter->_indent; ++i )
	{
		formatter->_tag[i] = formatter->_character;
	}
	formatter->_tag[i] = '\0';
}




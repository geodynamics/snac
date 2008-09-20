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
** $Id: RankFormatter.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "Base/Foundation/Foundation.h"

#include "types.h"
#include "shortcuts.h"
#include "Journal.h"
#include <stdarg.h>  
#include "StreamFormatter.h"
#include "LineFormatter.h"
#include "RankFormatter.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <mpi.h>


const Type RankFormatter_Type = "RankFormatter";
const char* PREPEND = ": ";


StreamFormatter* RankFormatter_New()
{
	return (StreamFormatter*)_RankFormatter_New( sizeof(RankFormatter), RankFormatter_Type,
		_RankFormatter_Delete, _RankFormatter_Print, _LineFormatter_Copy, _LineFormatter_Format );
}
	
void RankFormatter_Init( RankFormatter* self )
{
	self->_sizeOfSelf = sizeof(RankFormatter);
	self->type = RankFormatter_Type;
	self->_delete = _RankFormatter_Delete;
	self->_print = _RankFormatter_Print;
	self->_copy = _LineFormatter_Copy;

	_LineFormatter_Init( (LineFormatter*)self, _LineFormatter_Format );
	_RankFormatter_Init( self, _LineFormatter_Format );
}

RankFormatter* _RankFormatter_New(
	SizeT 				_sizeOfSelf,
	Type 				type,
	Stg_Class_DeleteFunction*		_delete,
	Stg_Class_PrintFunction*		_print,
	Stg_Class_CopyFunction*		_copy, 
	StreamFormatter_FormatFunction*	_format )
{
	RankFormatter* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(RankFormatter) );
	self = (RankFormatter*)_LineFormatter_New( _sizeOfSelf, type, _delete, _print, _copy, _format );
	
	_RankFormatter_Init( self, _format );

	return self;
}

void _RankFormatter_Init(
	RankFormatter*			self,
	StreamFormatter_FormatFunction*	_format )
{
	int rank, rankTemp;
	int rankDigits;
	int size;
	int prependLength;
	
	/* If there is only one processor, do not print rank */
	MPI_Comm_size( MPI_COMM_WORLD, &size );
	if ( size == 1 )
	{
		return;
	}

	/* Calculate length of rank string */
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	rankTemp = rank;
	rankDigits = 0;
	do
	{
		rankDigits++;
		rankTemp /= 10; /* Base 10 */
	} while ( rankTemp > 0 );
	
	prependLength = rankDigits + strlen( PREPEND );

	self->_tag = Memory_Alloc_Array( char, prependLength + 1, "RankFormatter->_tag" );

	sprintf( self->_tag, "%d%s", rank, PREPEND );
}

void _RankFormatter_Delete( void* formatter )
{
	RankFormatter* self = (RankFormatter*)formatter;
	
	#if DEBUG
		assert( self );
	#endif

	/* Stg_Class_Delete parent */
	_LineFormatter_Delete( self );
}
void _RankFormatter_Print( void* formatter, Stream* stream )
{
	#if DEBUG
		RankFormatter* self = (RankFormatter*) formatter;

		assert( self );
		assert (stream);
	#endif
	
	/* General info */
	printf( "RankFormatter (ptr): %p\n", formatter );
	
	_LineFormatter_Print( formatter, stream );
}



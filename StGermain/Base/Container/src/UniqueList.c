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
** $Id: UniqueList.c 2192 2004-10-15 02:45:38Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"

#include "types.h"
#include "List.h"
#include "UniqueList.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


/* Textual name of this class */
const Type UniqueList_Type = "UniqueList";


/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

UniqueList* UniqueList_New_Param(
		SizeT						elementSize,
		unsigned					delta )
{
	return _UniqueList_New( 
		sizeof(UniqueList), 
		UniqueList_Type, 
		_UniqueList_Delete, 
		_UniqueList_Print, 
		NULL, 
		_UniqueList_Append, 
		_List_Mutate, 
		elementSize,
		delta );
}


void UniqueList_Init(
		UniqueList*					self,
		SizeT						elementSize,
		unsigned					delta )
{
	/* General info */
	self->type = UniqueList_Type;
	self->_sizeOfSelf = sizeof(UniqueList);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _UniqueList_Delete;
	self->_print = _UniqueList_Print;
	self->_copy = NULL;
	self->_append = _UniqueList_Append;
	self->_mutate = _List_Mutate;
	_Stg_Class_Init( (Stg_Class*)self );
	_List_Init( (List*)self, elementSize, delta );
	
	/* UniqueList info */
	_UniqueList_Init( self );
}


UniqueList* _UniqueList_New(
		SizeT						_sizeOfSelf, 
		Type						type,
		Stg_Class_DeleteFunction*				_delete,
		Stg_Class_PrintFunction*				_print, 
		Stg_Class_CopyFunction*				_copy, 
		List_AppendFunc*				_append,
		List_MutateFunc*				_mutate, 
		SizeT						elementSize,
		unsigned					delta )
{
	UniqueList*		self;
	
	/* allocate memory */
	assert( _sizeOfSelf >= sizeof(UniqueList) );
	self = (UniqueList*)_List_New(
		_sizeOfSelf,
		type,
		_delete,
		_print, 
		_copy, 
		_append, 
		_mutate, 
		elementSize, 
		delta );
	
	/* general info */
	
	/* virtual info */
	
	/* UniqueList info */
	_UniqueList_Init( self );
	
	return self;
}


void _UniqueList_Init(
		UniqueList*					self )
{
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _UniqueList_Delete( void* uniqueList ) {
	UniqueList*		self = (UniqueList*)uniqueList;
	
	/* delete the class itself */
	
	/* delete parent */
	_List_Delete( self );
}


void _UniqueList_Print( void* uniqueList, Stream* stream ) {
	UniqueList*		self = (UniqueList*)uniqueList;
	Stream*		myStream = Journal_Register( InfoStream_Type, "UniqueListStream" );

	/* print parent */
	_List_Print( self, stream );
	
	/* general info */
	Journal_Printf( myStream, "UniqueList (ptr): (%p)\n", self );
	
	/* virtual info */
	
	/* UniqueList info */
}


unsigned _UniqueList_Append( void* uniqueList, void* data ) {
	UniqueList*	self = (UniqueList*)uniqueList;
	Index		el_I;
	
	for( el_I = 0; el_I < UniqueList_Size( self ); el_I++ ) {
		if( memcmp( &((unsigned char*)self->elements)[self->elementSize * el_I], data, 
			sizeof(unsigned char) * self->elementSize ) == 0 )
		{
			return el_I;
		}
	}
	
	return _List_Append( self, data );
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/


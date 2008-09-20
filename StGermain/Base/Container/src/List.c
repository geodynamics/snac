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
** $Id: List.c 2192 2004-10-15 02:45:38Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"

#include "units.h"
#include "types.h"
#include "List.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>


/* Textual name of this class */
const Type List_Type = "List";


/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

List* List_New_Param(
		SizeT						elementSize, 
		unsigned					delta )
{
	return _List_New( 
		sizeof(List), 
		List_Type, 
		_List_Delete, 
		_List_Print, 
		NULL, 
		_List_Append,
		_List_Mutate, 
		elementSize,
		delta );
}


void List_Init(
		List*						self,
		SizeT						elementSize, 
		unsigned					delta )
{
	/* General info */
	self->type = List_Type;
	self->_sizeOfSelf = sizeof(List);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _List_Delete;
	self->_print = _List_Print;
	self->_copy = NULL;
	self->_append = _List_Append;
	self->_mutate = _List_Mutate;
	_Stg_Class_Init( (Stg_Class*)self );
	
	/* List info */
	_List_Init( self, elementSize, delta );
}


List* _List_New(
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
	List*			self;
	
	/* Allocate memory */
	self = (List*)_Stg_Class_New(
		_sizeOfSelf,
		type,
		_delete,
		_print, 
		_copy );
	
	/* General info */
	
	/* Virtual info */
	self->_append = _append;
	self->_mutate = _mutate;
	
	/* List info */
	_List_Init( self, elementSize, delta );
	
	return self;
}


void _List_Init(
		List*						self,
		SizeT						elementSize, 
		unsigned					delta )
{
	/* General and Virtual info should already be set */
	
	/* List info */
	self->elementSize = elementSize;
	self->delta = delta;
	self->maxElements = self->delta;
	self->elements = Memory_Alloc_Bytes_Unnamed( self->elementSize * self->maxElements, "char");
	assert( self->elements ); 
	self->elementCnt = 0;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _List_Delete( void* list ) {
	List*			self = (List*)list;
	
	/* Stg_Class_Delete the class itself */
	if( self->elements )
		Memory_Free( self->elements );
	
	/* Stg_Class_Delete parent */
	_Stg_Class_Delete( self );
}


void _List_Print( void* list, Stream* stream ) {
	List*			self = (List*)list;
	
	/* Set the Journal for printing informations */
	Stream*			myStream;
	myStream = Journal_Register( InfoStream_Type, "ListStream" );

	/* Print parent */
	_Stg_Class_Print( self, stream );
	
	/* General info */
	Journal_Printf( myStream, "List (ptr): (%p)\n", self );
	
	/* Virtual info */
	
	/* List info */
	Journal_Printf( myStream, "\telementSize: %d\n", self->elementSize );
	Journal_Printf( myStream, "\tdelta: %d\n", self->delta );
	Journal_Printf( myStream, "\tmaxElements: %d\n", self->maxElements );
	Journal_Printf( myStream, "\telementCnt: %d\n", self->elementCnt );
	Journal_Printf( myStream, "\telements (ptr): %p\n", self->elements );
}


unsigned _List_Append( void* list, void* data ) {
	List*			self = (List*)list;
	
	List_Resize( self, self->elementCnt + 1 );
	memcpy( &((unsigned char*)self->elements)[self->elementSize * (self->elementCnt - 1)], data,
		sizeof(unsigned char) * self->elementSize );
	
	return self->elementCnt - 1;
}


void _List_Mutate( void* list, unsigned index, void* data ) {
	List*		self = (List*)list;
	
	memcpy( &((unsigned char*)self->elements)[self->elementSize * index], data,
		sizeof(unsigned char) * self->elementSize );
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void List_Resize( void* list, unsigned size ) {
	List*		self = (List*)list;
	
	if( size >= self->maxElements ) {
		unsigned		factor;
		void*			newElements;
		
		factor = ceil( (float)(size - self->maxElements) / (float)self->delta );
		self->maxElements += factor * self->delta;
		
		newElements = Memory_Alloc_Bytes_Unnamed( self->elementSize * self->maxElements, "char" );
		assert( newElements ); 
		if( self->elements ) {
			memcpy( newElements, self->elements, self->elementSize * self->elementCnt );
			Memory_Free( self->elements );
		}
		self->elements = newElements;
	}
	else if( size < self->elementCnt ) {
		Journal_Firewall( 0, Journal_Register( ErrorStream_Type, "List" ), "error in '%s'\n", __func__); 
	}
	
	self->elementCnt = size;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

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
** $Id: LinkedListIterator.c 2038 2005-04-16 07:11:01Z RaquibulHassan $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <Base/Foundation/Foundation.h>
#include <Base/IO/IO.h>

#include "types.h"
#include "LinkedListNode.h"
#include "LinkedList.h"
#include "LinkedListIterator.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

const Type LinkedListIterator_Type = "LinkedListIterator";

LinkedListIterator *_LinkedListIterator_New(
		SizeT					_sizeOfSelf,
		Type					type,
		Stg_Class_DeleteFunction*	_delete,
		Stg_Class_PrintFunction*	_print,
		Stg_Class_CopyFunction*		_copy
		)
{
	LinkedListIterator *self = NULL;

	/** LinkedListIterator using Class_New, because  Memory module has been initialized */
	
	self = (LinkedListIterator*)_Stg_Class_New(
		_sizeOfSelf,
		type,
		_delete,
		_print, 
		_copy );
	
	return self;
}

LinkedListIterator *LinkedListIterator_New( LinkedList *list )
{
	
	LinkedListIterator* self;
	
	assert( list );
	self = _LinkedListIterator_New(
			sizeof(LinkedListIterator),
			LinkedListIterator_Type,
			_LinkedListIterator_DeleteFunc,
			_LinkedListIterator_PrintFunc,
			NULL
			);
	/* General info */
	
	/* Virtual functions */
	assert( list );
	self->list = list;
	
	_LinkedListIterator_Init( self );	
	return self;
}

void _LinkedListIterator_Init( LinkedListIterator* self )
{

}

void _LinkedListIterator_DeleteFunc( void *self )
{
	if( self ){
		_Stg_Class_Delete( self );
	}
}

void _LinkedListIterator_PrintFunc( void *self, Stream *myStream )
{
	LinkedListIterator *iterator = NULL;

	iterator = (LinkedListIterator*) self;
	assert( iterator );

	/* print parent */
	_Stg_Class_Print( (void*) iterator, myStream );

	/* general info */
	Journal_Printf( myStream, "LinkedListIterator (ptr): (%p)\n", iterator );
	Journal_Printf( myStream, "\tlist (ptr): (%p)\n", iterator->list );
}

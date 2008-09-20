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
** $Id: BTreeIterator.c 2038 2005-04-16 07:11:01Z RaquibulHassan $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <Base/Foundation/Foundation.h>
#include <Base/IO/IO.h>

#include "types.h"
#include "BTreeNode.h"
#include "BTree.h"
#include "BTreeIterator.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

const Type BTreeIterator_Type = "BTreeIterator";

BTreeIterator *_BTreeIterator_New(
		SizeT					_sizeOfSelf,
		Type					type,
		Stg_Class_DeleteFunction*	_delete,
		Stg_Class_PrintFunction*	_print,
		Stg_Class_CopyFunction*		_copy
		)
{
	BTreeIterator *self = NULL;

	/** BTreeIterator has to be malloced instead of using Class_New, because Class_New uses Memory_Alloc, 
	 *  but the Memory module will not have been initialized at this stage */
	
	self = (BTreeIterator*)malloc( _sizeOfSelf );
	memset( self, 0, _sizeOfSelf );
	
	self->_sizeOfSelf = _sizeOfSelf;
	self->_deleteSelf = True;
	self->type = type;
	self->_delete = _delete;
	self->_print = _print;
	self->_copy = _copy;
	
	return self;
}

BTreeIterator *BTreeIterator_New( BTree *tree )
{
	
	BTreeIterator* self;
	
	assert( tree );
	self = _BTreeIterator_New(
			sizeof(BTreeIterator),
			BTreeIterator_Type,
			_BTreeIterator_DeleteFunc,
			_BTreeIterator_PrintFunc,
			NULL
			);
	/* General info */
	
	/* Virtual functions */
	
	self->tree = tree;
	
	BTreeIterator_Init( self );	
	return self;
}

void _BTreeIterator_Init( BTreeIterator* self )
{

	assert(self);
	self->depth = 0;
}

void BTreeIterator_Init( BTreeIterator *self )
{
	_BTreeIterator_Init( self );
}

void *BTreeIterator_First( BTreeIterator *self  )
{
	BTreeNode *node = NULL;
	assert( self );
	assert( self->tree );

	self->depth = -1;

	node = self->tree->root;
	while( node != NIL ){
		self->depth++;
		self->stack[self->depth] = node;
		node = node->left;
	}
	
	if( self->depth < 0 ){
		return NULL;
	}
	return self->stack[self->depth]->data;
}

void *BTreeIterator_Next( BTreeIterator *self )
{
	BTreeNode *node = NULL;
	
	assert( self );
	assert( self->tree );

	if( self->depth < 0 ){
		return NULL;
	}
	
	node = self->stack[self->depth];
	self->depth--;
	node = node->right;
	
	while( node != NIL ){
		self->depth ++;
		self->stack[self->depth] = node;
		node = node->left;
	}

	if( self->depth < 0 ){
		return NULL;
	}
	else{
		return self->stack[self->depth]->data;
	}
}

void _BTreeIterator_DeleteFunc( void *self )
{
	if( self ){
		free( self );
		/* freeing the Iterator instead of using class_delete, because it was initially malloced */
	}
}

void _BTreeIterator_PrintFunc( void *self, Stream *myStream )
{
	BTreeIterator *iterator = NULL;

	iterator = (BTreeIterator*) self;
	assert( iterator );

	/* print parent */
	_Stg_Class_Print( (void*) iterator, myStream );

	/* general info */
	Journal_Printf( myStream, "BTreeIterator (ptr): (%p)\n", iterator );
	Journal_Printf( myStream, "\tTree (ptr): (%p)\n", iterator->tree );
	Journal_Printf( myStream, "\tDepth : %d\n", iterator->depth );
}

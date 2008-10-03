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
** $Id: PtrSet.c 2192 2004-10-15 02:45:38Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"

#include "types.h"
#include "BTreeNode.h"
#include "BTree.h"
#include "Set.h"
#include "PtrSet.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


/* Textual name of this class */
const Type PtrSet_Type = "PtrSet";


/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

PtrSet* PtrSet_New(
		Dictionary*					dictionary )
{
	return _PtrSet_New( 
		sizeof(PtrSet), 
		PtrSet_Type, 
		_PtrSet_Delete, 
		_PtrSet_Print, 
		NULL, 
		_PtrSet_Union, 
		_PtrSet_Intersection, 
		_PtrSet_Subtraction, 
		dictionary, 
		0, 
		_PtrSet_CompareData, 
		NULL, 
		_PtrSet_DeleteData );
}


void PtrSet_Init(
		PtrSet*						self,
		Dictionary*					dictionary )
{
	/* General info */
	self->type = PtrSet_Type;
	self->_sizeOfSelf = sizeof(PtrSet);
	self->_deleteSelf = False;
	self->dictionary = dictionary;
	
	/* Virtual info */
	self->_delete = _PtrSet_Delete;
	self->_print = _PtrSet_Print;
	self->_copy = NULL;
	self->_unionFunc = _PtrSet_Union;
	self->_intersectionFunc = _PtrSet_Intersection;
	self->_subtractionFunc = _PtrSet_Subtraction;
	_Set_Init( (Set*)self, 0, _PtrSet_CompareData, NULL, _PtrSet_DeleteData );
	
	/* PtrSet info */
	_PtrSet_Init( self );
}


PtrSet* _PtrSet_New(
		SizeT						_sizeOfSelf, 
		Type						type,
		Stg_Class_DeleteFunction*				_delete,
		Stg_Class_PrintFunction*				_print, 
		Stg_Class_CopyFunction*				_copy, 
		Set_UnionFunc*					_unionFunc, 
		Set_IntersectionFunc*				_intersectionFunc, 
		Set_SubtractionFunc*				_subtractionFunc, 
		Dictionary*					dictionary, 
		SizeT						elementSize, 
		BTree_compareFunction*				compareFunc, 
		BTree_dataCopyFunction*				dataCopyFunc, 
		BTree_dataDeleteFunction*			dataDeleteFunc )
{
	PtrSet*	self;
	
	/* allocate memory */
	assert( _sizeOfSelf >= sizeof(PtrSet) );
	self = (PtrSet*)_Set_New(
		_sizeOfSelf,
		type,
		_delete,
		_print, 
		_copy, 
		_unionFunc, 
		_intersectionFunc, 
		_subtractionFunc, 
		dictionary, 
		elementSize, 
		compareFunc, 
		dataCopyFunc, 
		dataDeleteFunc );
	
	/* general info */
	
	/* virtual info */
	
	/* PtrSet info */
	_PtrSet_Init( self );
	
	return self;
}


void _PtrSet_Init(
		PtrSet*						self )
{
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _PtrSet_Delete( void* ptrPtrSet ) {
	PtrSet*	self = (PtrSet*)ptrPtrSet;
	
	/* delete the class itself */
	
	/* delete parent */
	_Set_Delete( self );
}


void _PtrSet_Print( void* ptrPtrSet, Stream* stream ) {
	PtrSet*		self = (PtrSet*)ptrPtrSet;
	Stream*		myStream = Journal_Register( InfoStream_Type, "PtrSetStream" );

	/* print parent */
	_Set_Print( self, stream );
	
	/* general info */
	Journal_Printf( myStream, "PtrSet (ptr): (%p)\n", self );
	Journal_Printf( myStream, "\tdictionary (ptr): %p\n", self->dictionary );
	
	/* virtual info */
	
	/* PtrSet info */
}


void* _PtrSet_Union( void* ptrPtrSet, void* operand ) {
	PtrSet*	self = (PtrSet*)ptrPtrSet;
	PtrSet*	results;

	results = PtrSet_New( self->dictionary );

	BTree_ParseTree( self->_btree, _Set_BTreeUnion, results );
	BTree_ParseTree( ((PtrSet*)operand)->_btree, _Set_BTreeUnion, results );

	return results;
}


void* _PtrSet_Intersection( void* ptrPtrSet, void* operand ) {
	PtrSet*	self = (PtrSet*)ptrPtrSet;
	PtrSet*	results;
	void*	pack[2];

	results = PtrSet_New( self->dictionary );

	pack[0] = operand;
	pack[1] = results;
	BTree_ParseTree( self->_btree, _Set_BTreeIntersection, pack );

	return results;
}


void* _PtrSet_Subtraction( void* ptrPtrSet, void* operand ) {
	PtrSet*	self = (PtrSet*)ptrPtrSet;
	PtrSet*	results;
	void*	pack[2];

	results = PtrSet_New( self->dictionary );

	pack[0] = operand;
	pack[1] = results;
	BTree_ParseTree( self->_btree, _Set_BTreeSubtraction, pack );

	return results;
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

int _PtrSet_CompareData( void* left, void* right ) {
	if( (ArithPointer)left < (ArithPointer)right ) {
		return -1;
	}
	else if( (ArithPointer)left > (ArithPointer)right ) {
		return 1;
	}
	else {
		return 0;
	}
}


void _PtrSet_DeleteData( void* data ) {
}

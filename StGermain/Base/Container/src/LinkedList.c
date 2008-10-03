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
** $Id: List.c 2038 2004-11-15 07:11:01Z RaquibulHassan $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <Base/Foundation/Foundation.h>
#include <Base/IO/IO.h>

#include "types.h"
#include "LinkedListNode.h"
#include "LinkedList.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

const Type LinkedList_Type = "LinkedList";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/
LinkedList* _LinkedList_New(
			SizeT							_sizeOfSelf,
			Type							type,
			Stg_Class_DeleteFunction*			_delete,
			Stg_Class_PrintFunction*			_print,
			Stg_Class_CopyFunction*				_copy
			)
{
	LinkedList *self = NULL;

	self = (LinkedList*)_Stg_Class_New(
							_sizeOfSelf,
							type,
							_delete,
							_print,
							_copy);
	return self;
}

LinkedList* LinkedList_New( 
		LinkedList_compareFunction*		compareFunction,
		LinkedList_dataCopyFunction*	dataCopyFunction,
		LinkedList_dataPrintFunction*	dataPrintFunction,
		LinkedList_dataDeleteFunction*	dataDeleteFunction,
		Order listOrder )
{
	
	LinkedList* self;
	
	/* General info */
	assert ( compareFunction );
	
	self = _LinkedList_New( 
		   					sizeof(LinkedList),
							LinkedList_Type,
							_LinkedList_DeleteFunc,
							_LinkedList_PrintFunc,
							NULL);

	/* List info */
	self->compareFunction = compareFunction;
	self->dataCopyFunction = dataCopyFunction;
	self->dataPrintFunction = dataPrintFunction;
	self->dataDeleteFunction = dataDeleteFunction;
	self->listOrder = listOrder;
	
	/* Virtual functions */
	LinkedList_Init( self );
	return self;
}

void _LinkedList_Init( LinkedList* self ){
	/* General info */
	
	assert( self );
	_Stg_Class_Init ((Stg_Class*) self);
	
	self->head = NULL;
	self->nodeCount = 0;
	/* Dictionary info */
}

void LinkedList_Init( LinkedList *self )
{
	assert( self );
	_LinkedList_Init (self);
}

/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual Functions
*/
void _LinkedList_PrintFunc ( void *list, Stream *stream )
{
	LinkedList *self = NULL;
	
	self = (LinkedList*) list;
	assert( self );
	assert( stream );

	/* print parent */
	_Stg_Class_Print( (void*) self, stream );

	/* general info */
	Journal_Printf( stream, "LinkedList (ptr): (%p)\n", self );

	/* Virtual Info */

	/* LinkedList Info */
	Journal_Printf( stream, "\tNodeCount\t\t - %d\n", self->nodeCount );
	Journal_Printf( stream, "\tLinkedList Order\t - %s\n", (self->listOrder == LINKEDLIST_SORTED)?"SORTED":"UNSORTED" );
	
	Journal_Printf( stream, "\tLinkedList data\t - \n");
	if (self->dataPrintFunction)
		LinkedList_ParseList( self, (LinkedList_parseFunction*)self->dataPrintFunction, (void*)stream );
}

int LinkedList_DeleteAllNodes( LinkedList *list )
{
	LinkedList *self = NULL;
	LinkedListNode *curr = NULL, *temp = NULL;

	self = (LinkedList*)list;
	assert (self);

	curr = self->head;
	while (curr != NULL){
		temp = curr->next;

		if (self->dataDeleteFunction){
			self->dataDeleteFunction( curr->data );
		}
		else{
			Memory_Free(curr->data);
		}
		Memory_Free( curr ); /** Freeing the Node without calling the Stg_Class_Delete function, because LinkedList_Node does not inherit __Stg_Class */
		
		curr = temp;
		--self->nodeCount;
	}
	self->head = NULL;

	return 0;
}

void _LinkedList_DeleteFunc( void *list )
{
	LinkedList *self = NULL;
	
	self = (LinkedList*)list;
	assert (self);
	
	LinkedList_DeleteAllNodes( self );
	
	_Stg_Class_Delete( self );
}

/*----------------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/
void LinkedList_InsertNode ( LinkedList *list, void *newNodeData, SizeT dataSize)
{
	LinkedListNode *curr = NULL, *prev = NULL;
	LinkedListNode *newNode = NULL;
	
	assert (list);
	
	newNode = LinkedListNode_New();
	if(list->dataCopyFunction){
		list->dataCopyFunction(&newNode->data, newNodeData, dataSize);
	}
	else{
		newNode->data = newNodeData;
	}

	if( list->listOrder == LINKEDLIST_SORTED ){
		curr = list->head;
		prev = NULL;
		while(curr != NULL){
			if (list->compareFunction(newNodeData, curr->data) < 0){
				break;
			}
			prev = curr;
			curr = curr->next;
		}
		
		if (prev == NULL){
			newNode->next = list->head;
			list->head = newNode;
		}
		else {
			prev->next = newNode;
			newNode->next = curr;
		}
	}
	else{
		if (list->head == NULL){
			list->head = newNode;
		}
		else{
			newNode->next = list->head;
			list->head = newNode;
		}
	}
	++list->nodeCount;
}

LinkedListNode *LinkedList_FindNode( LinkedList *list, void *data )
{
	LinkedListNode *curr = NULL;

	assert (list);

	curr = list->head;

	while(curr != NULL){
		if (list->compareFunction(curr->data, data) == 0){
			return curr;
		}
		curr = curr->next;
	}
	return NULL;
}

int LinkedList_DeleteNode( LinkedList *list, void *nodeData )
{
	LinkedListNode *curr = NULL, *prev = NULL, *targetNode = NULL;
	

	assert (list);

	curr = list->head;
	prev = NULL;
	while(curr != NULL){
		if (curr->data && nodeData){
			if (list->compareFunction(curr->data, nodeData) == 0){
				targetNode = curr;
				break;
			}
		}
		prev = curr;
		curr = curr->next;
	}

	if (targetNode != NULL){
		if (list->dataDeleteFunction){
			list->dataDeleteFunction(targetNode->data);
		}
		else{
			Memory_Free(targetNode->data);
		}

		if(prev == NULL){
			list->head = targetNode->next;
		}
		else{
			prev->next = targetNode->next;
		}
		
		Memory_Free(targetNode);
		--list->nodeCount;
		return 1;
	}
	else{
		
	}

	return 0;
}

void LinkedList_ParseList( LinkedList *list, LinkedList_parseFunction *parseFunction, void *args )
{
	LinkedListNode *curr;

	assert(list);
	assert(parseFunction);

	curr = list->head;
	while(curr != NULL){
		parseFunction(curr->data, args);
		curr = curr->next;
	}
}

char* LinkedList_ReturnArrayFunc( LinkedList *list, SizeT dataSize )
{
	LinkedListNode *curr = NULL;
	char *array = NULL;
	int counter = 0;

	assert( list );
	
	array = Memory_Alloc_Array_Unnamed(char, list->nodeCount * dataSize );
	assert( array );
	
	curr = list->head;
	while(curr != NULL){
		memcpy(&array[(counter++)*dataSize], curr->data, dataSize);
		curr = curr->next;
	}

	return array;
}

void LinkedList_SetCompareFunction ( LinkedList *list, LinkedList_compareFunction *compareFunction )
{
	assert (list);
	assert (compareFunction);

	list->compareFunction = compareFunction;
}

void LinkedList_ParseListNode( LinkedListNode *root, LinkedList_parseFunction *parseFunction, void *args )
{
	LinkedListNode *curr;

	assert(root);
	assert(parseFunction);

	curr = root;
	while(curr != NULL){
		parseFunction(curr->data, args);
		curr = curr->next;
	}
}

void *LinkedList_FindNodeDataFunc( LinkedList *list, void *data )
{
	LinkedListNode *curr = NULL;

	assert (list);

	curr = list->head;

	while(curr != NULL){
		if (list->compareFunction(curr->data, data) == 0){
			return curr->data;
		}
		curr = curr->next;
	}
	return NULL;
}

void *LinkedList_ReturnNodeDataAtFunc( LinkedList *list, int index )
{
	LinkedListNode *curr = NULL;
	int nodeIndex = 0;

	assert (list);

	curr = list->head;

	while(curr != NULL){
		
		if (index == nodeIndex++){
			return curr->data;
		}
		else if (nodeIndex > index){
			break;
		}
		curr = curr->next;
	}
	return NULL;
}

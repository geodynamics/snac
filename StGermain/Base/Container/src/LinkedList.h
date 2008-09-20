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
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/** \file
** <b>Role:</b>
**	Binary Tree class for objects.
**
** <b>Assumptions:</b>
**	None
**
** <b>Comments:</b>
**	None
**
** $Id: LinkedList.h 2087 2005-2-25 02:28:44Z RaquibulHassan $
**
**/

#ifndef __LinkedList_h__
#define __LinkedList_h__

	/** Textual name for List class. */
	extern const Type LinkedList_Type;
	
	/** Virtual Function Pointers */
		/** This is a function type to be provided by the user for doing comparisons on node data */
	typedef int		(LinkedList_compareFunction)		( void *, void * );
		
		/** This is a function type to be provided by the user for copying node data, when a new node is being created */
	typedef void	(LinkedList_dataCopyFunction)		( void **, void *, SizeT );

		/** This is a function type to be provided by the user for printing the contents of a Node's data */
	typedef void	(LinkedList_dataPrintFunction)		( void *, void * );
		
		/** This is a function type to be provided by the user for deleting node data, when a new node is being deleted */
	typedef void	(LinkedList_dataDeleteFunction)		( void * );
	
		/** This is a function type to be provided by the user, which can be applied to each node on the tree later on.
		 * The first argument is the data stored inside a node and the second argument can be a single argument or a 
		 * struct of arguments that need to be passed to the user-supplied function */
	typedef void	(LinkedList_parseFunction)			( void *, void * );
	
	/** \def __List See __List */
	#define __LinkedList \
		/* General info */ \
		__Stg_Class \
		\
		/* Virtual info */ \
		\
		/* List info */ \
		LinkedListNode					*head; \
		LinkedList_compareFunction*		compareFunction; \
		LinkedList_dataCopyFunction*	dataCopyFunction; \
		LinkedList_dataPrintFunction*	dataPrintFunction; \
		LinkedList_dataDeleteFunction*	dataDeleteFunction; \
		Order							listOrder; \
		Index						nodeCount;

	struct LinkedList { __LinkedList };

	/** Constructor interface. */
	LinkedList* LinkedList_New( 
				LinkedList_compareFunction*		compareFunction,
				LinkedList_dataCopyFunction*	dataCopyFunction,
				LinkedList_dataPrintFunction*	dataPrintFunction,
				LinkedList_dataDeleteFunction*	dataDeleteFunction,
				Order listOrder);

	LinkedList* _LinkedList_New(
				SizeT							_sizeOfSelf,
				Type							type,
				Stg_Class_DeleteFunction*			_delete,
				Stg_Class_PrintFunction*			_print,
				Stg_Class_CopyFunction*				_copy
				);

	
	/** Init interface. */
	void LinkedList_Init( LinkedList* self );
	
	void _LinkedList_Init( LinkedList* self );
	
	/** Stg_Class_Delete interface. */
		/** Stg_Class delete function */
	void _LinkedList_DeleteFunc ( void *list );
	
	/** Print interface. */
		/** Stg_Class print function */
	void _LinkedList_PrintFunc ( void *list, Stream* stream );

	/** Public functions */
		/** Inserts a new node into the tree conserving the list's (Ascending) order */
	void LinkedList_InsertNode ( LinkedList *list, void *newNodeData, SizeT );
		
		/** Sets the compare function to be used by the tree */
	void LinkedList_SetCompareFunction ( LinkedList *list, LinkedList_compareFunction *compareFunction );
	
		/** Attempts to find a node in the list and returns the node if found*/
	LinkedListNode *LinkedList_FindNode( LinkedList *list, void *data );
		
	/** Attempts to find a node data in the list and returns the node data if found*/
	void *LinkedList_FindNodeDataFunc( LinkedList *list, void *data );
#define LinkedList_FindNodeData( list, data, type ) \
		(type*) LinkedList_FindNodeDataFunc( list, data )
	
	/** Attempts to find a node data in the list at a particular position and returns the node data if found*/
	void *LinkedList_ReturnNodeDataAtFunc( LinkedList *list, int index );
#define LinkedList_ReturnNodeDataAt( list, index, type ) \
		(type*) LinkedList_ReturnNodeDataAtFunc( list, index )
	
	/** Deletes a node from the list with the node-data being passed in as a parameter */
	int LinkedList_DeleteNode( LinkedList *list, void *nodeData );
	
	int LinkedList_DeleteAllNodes( LinkedList *list );
		
		/** Parses the list and executes a user-supplied function */
	void LinkedList_ParseList( LinkedList *list, LinkedList_parseFunction *parseFunction, void *args );
	
		/** Parses the list downwards from a given node in the list */
	void LinkedList_ParseListNode( LinkedListNode *root, LinkedList_parseFunction *parseFunction, void *args );

		/** Creates an array of all the nodes' data and returns it */
	char* LinkedList_ReturnArrayFunc( LinkedList *list, SizeT dataSize );

#define LinkedList_ReturnArray( list, type ) \
		(type*) LinkedList_ReturnArrayFunc( list, sizeof(type) )

	/** Private Functions */
	
	
#endif /* __LinkedList_h__ */


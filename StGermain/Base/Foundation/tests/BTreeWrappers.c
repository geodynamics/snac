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
** $Id: testLibStGermainDynamic.c 2742 2005-03-05 05:33:43Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdarg.h>
#include "Base/Foundation/Foundation.h"
#include "BTreeWrappers.h"
#include <stdio.h>
#include <stdlib.h>

BTree* BTree_New( 
	BTree_compareFunction*		compareFunction,
	BTree_dataCopyFunction*		dataCopyFunction,
	BTree_dataDeleteFunction*	dataDeleteFunction,
	BTree_dataPrintFunction*	dataPrintFunction,
	BTreeProperty				property)
{
	BTree* result = (BTree*)malloc( sizeof(BTree) );

	/* Manual fill data for this wrapper */
        result->_sizeOfSelf = sizeof(BTree);
	result->_deleteSelf = True;
	result->type = "BTree";
	result->_delete = _BTree_Delete;
	result->_print = NULL;

	result->numItems = 0;
	result->currentItem = 0;
	result->items = NULL;

	result->compareFunction = compareFunction;
	
	return result;
}

void _BTree_Delete( void* self ) {

	BTree* tree = (BTree*)self;
	int ii;
	
	for ( ii = 0; ii < tree-> currentItem; ++ii ) {
		if ( tree->items[ii] != NULL ) {
			free( tree->items[ii] );
			tree->items[ii] = NULL;
		}
	}
	
	free( tree );

}

int BTree_InsertNode ( BTree *tree, void *newNodeData, SizeT sizeOfData)
{
	if ( tree->currentItem == tree->numItems ) {
		tree->numItems++;
		tree->items = (BTreeNode**)realloc( tree->items, sizeof(BTreeNode*) * tree->numItems );
	}
	tree->items[tree->currentItem] = (BTreeNode*)malloc( sizeof(BTreeNode) );
	tree->items[tree->currentItem]->data = newNodeData;

	tree->currentItem++;
	
	return 1;
}
	
void BTree_SetCompareFunction ( BTree *tree, BTree_compareFunction *compareFunction )
{
	tree->compareFunction = compareFunction;
}
	
BTreeNode *BTree_FindNode( BTree *tree, void *data )
{
	int ii;

	if ( tree->numItems == 0 ) {
		return NULL;
	}

	for ( ii = 0; ii < tree->numItems; ++ii ) {
		if ( tree->items[ii] != NULL ) {
			if ( tree->compareFunction ( tree->items[ii]->data, data ) == 0 ) {
				return tree->items[ii];
			}
		}
	}
	return NULL;	
}
	
void BTree_DeleteNode( BTree *tree, BTreeNode *z )
{
	int ii;

	if ( z == NULL || tree->numItems == 0 ) {
		return;
	}

	for ( ii = 0; ii < tree->numItems; ++ii ) {
		if ( tree->items[ii] != NULL ) {
			if ( tree->compareFunction( tree->items[ii]->data, z->data ) == 0 ) {
				tree->items[ii] = NULL;
				free( z );
				return;
			}
		}
	}
}

void BTree_ParseTree( BTree *tree, BTree_parseFunction *parseFunction, void *args )
{
	int ii;

	for ( ii = 0; ii < tree->numItems; ++ii ) {
		if ( tree->items[ii] != NULL ) {
			parseFunction( tree->items[ii]->data, args );
		}
	}
}
	
void* BTree_GetData( BTreeNode *node )
{
	return node->data;
}

void BTree_Delete( void* tree ) {
	Stg_Class_Delete( tree );
}

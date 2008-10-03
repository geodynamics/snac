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
** $Id: UIntMap.c 3584 2006-05-16 11:11:07Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <mpi.h>

#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"

#include "types.h"
#include "BTreeNode.h"
#include "BTree.h"
#include "UIntMap.h"


typedef struct {
	unsigned	curItem;
	unsigned*	keys;
	unsigned*	vals;
} UIntMap_ParseStruct;


/* Textual name of this class */
const Type UIntMap_Type = "UIntMap";


/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

UIntMap* UIntMap_New() {
	return _UIntMap_New( sizeof(UIntMap), 
			     UIntMap_Type, 
			     _UIntMap_Delete, 
			     _UIntMap_Print, 
			     _UIntMap_Copy );
}

UIntMap* _UIntMap_New( UINTMAP_DEFARGS ) {
	UIntMap* self;
	
	/* Allocate memory */
	assert( sizeOfSelf >= sizeof(UIntMap) );
	self = (UIntMap*)_Stg_Class_New( STG_CLASS_PASSARGS );

	/* Virtual info */

	/* UIntMap info */
	_UIntMap_Init( self );

	return self;
}

void _UIntMap_Init( UIntMap* self ) {
	self->btree = BTree_New( UIntMap_DataCompare, UIntMap_DataCopy, UIntMap_DataDelete, NULL, 
				  BTREE_NO_DUPLICATES );
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _UIntMap_Delete( void* generator ) {
	UIntMap*	self = (UIntMap*)generator;

	FreeObject( self->btree );

	/* Delete the parent. */
	_Stg_Class_Delete( self );
}

void _UIntMap_Print( void* generator, Stream* stream ) {
	UIntMap*	self = (UIntMap*)generator;
	
	/* Set the Journal for printing informations */
	Stream* generatorStream;
	generatorStream = Journal_Register( InfoStream_Type, "UIntMapStream" );

	/* Print parent */
	Journal_Printf( stream, "UIntMap (ptr): (%p)\n", self );
	_Stg_Class_Print( self, stream );
}

void* _UIntMap_Copy( void* generator, void* destProc_I, Bool deep, Name nameExt, PtrMap* ptrMap ) {
#if 0
	UIntMap*	self = (UIntMap*)generator;
	UIntMap*	newUIntMap;
	PtrMap*	map = ptrMap;
	Bool	ownMap = False;

	/* Damn me for making copying so difficult... what was I thinking? */
	
	/* We need to create a map if it doesn't already exist. */
	if( !map ) {
		map = PtrMap_New( 10 );
		ownMap = True;
	}
	
	newUIntMap = (UIntMap*)_Mesh_Copy( self, destProc_I, deep, nameExt, map );
	
	/* Copy the virtual methods here. */

	/* Deep or shallow? */
	if( deep ) {
	}
	else {
	}
	
	/* If we own the map, get rid of it here. */
	if( ownMap ) Stg_Class_Delete( map );
	
	return (void*)newUIntMap;
#endif

	return NULL;
}

/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void UIntMap_Insert( void* map, unsigned key, unsigned val ) {
	UIntMap*	self = (UIntMap*)map;
	unsigned	data[2];

	assert( self );

	data[0] = key;
	data[1] = val;
	BTree_InsertNode( self->btree, data, 2 * sizeof(unsigned) );
	self->size = self->btree->nodeCount;
}

void UIntMap_Clear( void* map ) {
	UIntMap*	self = (UIntMap*)map;

	assert( self );

	FreeObject( self->btree );
	self->btree = BTree_New( UIntMap_DataCompare, UIntMap_DataCopy, UIntMap_DataDelete, NULL, 
				  BTREE_NO_DUPLICATES );
}

Bool UIntMap_HasKey( void* map, unsigned key ) {
	UIntMap*	self = (UIntMap*)map;

	assert( self );

	return (BTree_FindNode( self->btree, &key ) != NULL) ? True : False;
}

unsigned UIntMap_Map( void* map, unsigned key ) {
	UIntMap*	self = (UIntMap*)map;
	BTreeNode*	node;

	assert( self );

	node = BTree_FindNode( self->btree, &key );
	assert( node );

	return ((unsigned*)node->data)[1];
}

void UIntMap_GetItems( void* map, unsigned* nItems, unsigned** keys, unsigned** values ) {
	UIntMap*		self = (UIntMap*)map;
	UIntMap_ParseStruct	parseStruct;

	assert( self );

	parseStruct.curItem = 0;
	parseStruct.keys = Memory_Alloc_Array_Unnamed( unsigned, self->size );
	parseStruct.vals = Memory_Alloc_Array_Unnamed( unsigned, self->size );
	BTree_ParseTree( self->btree, UIntMap_ParseNode, &parseStruct );

	*nItems = self->size;
	*keys = parseStruct.keys;
	*values = parseStruct.vals;
}

/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

int UIntMap_DataCompare( void* left, void* right ) {
	if( ((unsigned*)left)[0] > ((unsigned*)right)[0] )
		return 1;
	else if( ((unsigned*)left)[0] < ((unsigned*)right)[0] )
		return -1;
	else
		return 0;
}

void UIntMap_DataCopy( void** dstData, void* data, SizeT size ) {
	*dstData = Memory_Alloc_Array_Unnamed( unsigned, 2 );
	((unsigned*)(*dstData))[0] = ((unsigned*)data)[0];
	((unsigned*)(*dstData))[1] = ((unsigned*)data)[1];
}

void UIntMap_DataDelete( void* data ) {
	Memory_Free( data );
}

void UIntMap_ParseNode( void* data, void* _parseStruct ) {
	UIntMap_ParseStruct*	parseStruct = (UIntMap_ParseStruct*)_parseStruct;

	assert( data );
	assert( parseStruct );

	parseStruct->keys[parseStruct->curItem] = ((unsigned*)data)[0];
	parseStruct->vals[parseStruct->curItem++] = ((unsigned*)data)[1];
}

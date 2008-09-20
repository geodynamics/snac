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
** $Id: NodeLayout.c 3574 2006-05-15 11:30:33Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"

#include "types.h"
#include "shortcuts.h"
#include "ElementLayout.h"
#include "NodeLayout.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


/* Textual name of this class */
const Type NodeLayout_Type = "NodeLayout";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

NodeLayout* _NodeLayout_New(
		SizeT						_sizeOfSelf, 
		Type						type,
		Stg_Class_DeleteFunction*				_delete,
		Stg_Class_PrintFunction*				_print,
		Stg_Class_CopyFunction*				_copy, 
		Stg_Component_DefaultConstructorFunction*	_defaultConstructor,
		Stg_Component_ConstructFunction*			_construct,
		Stg_Component_BuildFunction*				_build,
		Stg_Component_InitialiseFunction*			_initialise,
		Stg_Component_ExecuteFunction*				_execute,
		Stg_Component_DestroyFunction*				_destroy,
		Name									name,
		Bool									initFlag,
		NodeLayout_NodeCoordAtFunction*			        nodeCoordAt,
		NodeLayout_BuildNodeCoordsFunction*		        buildNodeCoords,
		NodeLayout_ElementNodeCountFunction*		        elementNodeCount,
		NodeLayout_BuildElementNodesFunction*		        buildElementNodes,
		NodeLayout_NodeElementCountFunction*		        nodeElementCount,
		NodeLayout_BuildNodeElementsFunction*		        buildNodeElements,
		NodeLayout_CalcNodeCountInDimFromElementCountFunction*  calcNodeCountInDimFromElementCount,
		Dictionary*					        dictionary,
		ElementLayout*					        elementLayout,
		Topology*					        topology )
{
	NodeLayout* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(NodeLayout) );
	
	self = (NodeLayout*)_Stg_Component_New( _sizeOfSelf, type, _delete, _print, _copy, _defaultConstructor, _construct, _build, 
			_initialise, _execute, _destroy, name, NON_GLOBAL );
	
	/* General info */
	self->dictionary = dictionary;
	
	/* Virtual info */
	self->nodeCoordAt = nodeCoordAt;
	self->buildNodeCoords = buildNodeCoords;
	self->elementNodeCount = elementNodeCount;
	self->buildElementNodes = buildElementNodes;
	self->nodeElementCount = nodeElementCount;
	self->buildNodeElements = buildNodeElements;
	self->calcNodeCountInDimFromElementCount = calcNodeCountInDimFromElementCount;
	
	/* NodeLayout info */
	if( initFlag ){
		_NodeLayout_Init( self, elementLayout, topology );
	}
	
	return self;
}

void _NodeLayout_Init(
		NodeLayout*					self,
		ElementLayout*					elementLayout,
		Topology*					topology )
{
	/* General and Virtual info should already be set */
	
	/* NodeLayout info */
	self->isConstructed = True;
	self->elementLayout = elementLayout;
	self->topology = topology;
	self->nodeCount = 0;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _NodeLayout_Delete( void* nodeLayout ) {
	NodeLayout* self = (NodeLayout*)nodeLayout;
	
	/* Stg_Class_Delete parent */
	_Stg_Component_Delete( self );
}


void _NodeLayout_Print( void* nodeLayout, Stream* stream ) {
	NodeLayout* self = (NodeLayout*)nodeLayout;
	
	/* Print parent */
	_Stg_Class_Print( self, stream );
	
	/* General info */
	Journal_Printf( stream, "NodeLayout (ptr): (%p)\n", self );
	
	/* Virtual info */
	
	/* NodeLayout info */
}


void* _NodeLayout_Copy( void* nodeLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	NodeLayout*	self = (NodeLayout*)nodeLayout;
	NodeLayout*	newNodeLayout;
	PtrMap*		map = ptrMap;
	Bool		ownMap = False;
	
	if( !map ) {
		map = PtrMap_New( 10 );
		ownMap = True;
	}
	
	newNodeLayout = (NodeLayout*)_Stg_Class_Copy( self, dest, deep, nameExt, map );
	
	/* Virtual methods */
	newNodeLayout->nodeCoordAt = self->nodeCoordAt;
	newNodeLayout->buildNodeCoords = self->buildNodeCoords;
	newNodeLayout->elementNodeCount = self->elementNodeCount;
	newNodeLayout->buildElementNodes = self->buildElementNodes;
	newNodeLayout->nodeElementCount = self->nodeElementCount;
	newNodeLayout->buildNodeElements = self->buildNodeElements;
	newNodeLayout->calcNodeCountInDimFromElementCount = self->calcNodeCountInDimFromElementCount;
	
	newNodeLayout->dictionary = self->dictionary;
	newNodeLayout->nodeCount = self->nodeCount;
	
	if( deep ) {
		newNodeLayout->elementLayout = (ElementLayout*)Stg_Class_Copy( self->elementLayout, NULL, deep, nameExt, map );
		newNodeLayout->topology = (Topology*)Stg_Class_Copy( self->topology, NULL, deep, nameExt, map );
	}
	else {
		newNodeLayout->elementLayout = self->elementLayout;
		newNodeLayout->topology = self->topology;
	}
	
	if( ownMap ) {
		Stg_Class_Delete( map );
	}
	
	return (void*)newNodeLayout;
}


Node_GlobalIndex NodeLayout_CalcNodeCountInDimFromElementCount( void* nodeLayout, Element_GlobalIndex elementCountInDim )
{
	NodeLayout* self = (NodeLayout*)nodeLayout;
	
	return self->calcNodeCountInDimFromElementCount( nodeLayout, elementCountInDim );
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

/* Note that I've tried to cut down on amount of memory allocing/reallocing in the case where each element has same
 * number of nodes. Main.PatrickSunter - 30 AUgust 2004 */
Node_GlobalIndex NodeLayout_BuildSubset( void* nodeLayout, Index elementCount, Index* elements, Node_GlobalIndex** nodes ) {
	NodeLayout*		self = (NodeLayout*)nodeLayout;
	Node_GlobalIndex	nodeCnt = 0;
	Node_GlobalIndex	prevENodeCnt = 0;
	Element_GlobalIndex     e_I;
	IndexSet*		set;
	Element_Nodes		eNodes;
	Element_GlobalIndex	eNodeCnt;
	
	set = IndexSet_New( self->nodeCount );
	eNodeCnt = self->elementNodeCount( self, elements[0] );
	eNodes = Memory_Alloc_Array_Unnamed( Node_GlobalIndex, eNodeCnt );
	prevENodeCnt = eNodeCnt;
	
	for( e_I = 0; e_I < elementCount; e_I++ ) {
		Element_GlobalIndex eNodeCnt = self->elementNodeCount( self, elements[e_I] );
		
		if( eNodeCnt ) {
			Node_GlobalIndex	n_I;

			if ( eNodeCnt > prevENodeCnt ) {
				eNodes = Memory_Realloc_Array( eNodes, Node_GlobalIndex, eNodeCnt );
			}
			
			self->buildElementNodes( self, elements[e_I], eNodes );
			for( n_I = 0; n_I < eNodeCnt; n_I++ )
				IndexSet_Add( set, eNodes[n_I] );
			
			prevENodeCnt = nodeCnt;
		}
	}
	
	Memory_Free( eNodes );
	IndexSet_GetMembers( set, &nodeCnt, nodes );
	Stg_Class_Delete( set );
	
	return nodeCnt;
}



/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

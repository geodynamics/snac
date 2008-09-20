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
** $Id: BodyNL.c 3883 2006-10-26 05:00:23Z KathleenHumble $
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
#include "BodyNL.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


/* Textual name of this class */
const Type BodyNL_Type = "BodyNL";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

BodyNL* BodyNL_DefaultNew( Name name )
{
	return (BodyNL*) _BodyNL_New( 
		sizeof(BodyNL), 
		BodyNL_Type, 
		_BodyNL_Delete, 
		_BodyNL_Print,
		NULL,
		(Stg_Component_DefaultConstructorFunction*)BodyNL_DefaultNew,
		_BodyNL_Construct,
		_BodyNL_Build,
		_BodyNL_Initialise,
		_BodyNL_Execute,
		_BodyNL_Destroy,
		name,
		False,
		_BodyNL_NodeCoordAt,
		_BodyNL_BuildNodeCoords,
		_BodyNL_ElementNodeCount,
		_BodyNL_BuildElementNodes,
		_BodyNL_NodeElementCount,
		_BodyNL_BuildNodeElements,
		_BodyNL_CalcNodeCountInDimFromElementCount,
		NULL,
		NULL,
		NULL );
}

BodyNL* BodyNL_New(
		Name						name,
		Dictionary*					dictionary,
		ElementLayout*					elementLayout,
		Topology*					topology )
{
	return _BodyNL_New( 
		sizeof(BodyNL), 
		BodyNL_Type, 
		_BodyNL_Delete, 
		_BodyNL_Print,
		NULL,
		(Stg_Component_DefaultConstructorFunction*)BodyNL_DefaultNew,
		_BodyNL_Construct,
		_BodyNL_Build,
		_BodyNL_Initialise,
		_BodyNL_Execute,
		_BodyNL_Destroy,
		name,
		True,
		_BodyNL_NodeCoordAt,
		_BodyNL_BuildNodeCoords,
		_BodyNL_ElementNodeCount,
		_BodyNL_BuildElementNodes,
		_BodyNL_NodeElementCount,
		_BodyNL_BuildNodeElements,
		_BodyNL_CalcNodeCountInDimFromElementCount,
		dictionary,
		elementLayout,
		topology );
}


void BodyNL_Init(
		BodyNL*						self,
		Name						name,
		Dictionary*					dictionary,
		ElementLayout*					elementLayout,
		Topology*					topology )
{
	/* General info */
	self->type = BodyNL_Type;
	self->_sizeOfSelf = sizeof(BodyNL);
	self->_deleteSelf = False;
	self->dictionary = dictionary;
	
	/* Virtual info */
	self->_delete = _BodyNL_Delete;
	self->_print = _BodyNL_Print;
	self->_copy = NULL;
	self->nodeCoordAt = _BodyNL_NodeCoordAt;
	self->buildNodeCoords = _BodyNL_BuildNodeCoords;
	self->elementNodeCount = _BodyNL_ElementNodeCount;
	self->buildElementNodes = _BodyNL_BuildElementNodes;
	self->nodeElementCount = _BodyNL_NodeElementCount;
	self->buildNodeElements = _BodyNL_BuildNodeElements;
	self->calcNodeCountInDimFromElementCount = _BodyNL_CalcNodeCountInDimFromElementCount;
	
	_Stg_Class_Init( (Stg_Class*)self );
	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );
	_Stg_Component_Init( (Stg_Component*)self );
	_NodeLayout_Init( (NodeLayout*)self, elementLayout, topology );
	
	/* BodyNL info */
	_BodyNL_Init( self );
}


BodyNL* _BodyNL_New(
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
		NodeLayout_NodeCoordAtFunction*			nodeCoordAt,
		NodeLayout_BuildNodeCoordsFunction*		buildNodeCoords,
		NodeLayout_ElementNodeCountFunction*		elementNodeCount,
		NodeLayout_BuildElementNodesFunction*		buildElementNodes,
		NodeLayout_NodeElementCountFunction*		nodeElementCount,
		NodeLayout_BuildNodeElementsFunction*		buildNodeElements,
		NodeLayout_CalcNodeCountInDimFromElementCountFunction*  calcNodeCountInDimFromElementCount,
		Dictionary*					dictionary,
		ElementLayout*					elementLayout,
		Topology*					topology )
{
	BodyNL* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(BodyNL) );
	self = (BodyNL*)_NodeLayout_New(
		_sizeOfSelf,
		type,
		_delete,
		_print,
		_copy,
		_defaultConstructor,
		_construct,
		_build,
		_initialise,
		_execute,
		_destroy,
		name,
		initFlag,
		nodeCoordAt,
		buildNodeCoords,
		elementNodeCount,
		buildElementNodes,
		nodeElementCount,
		buildNodeElements,
		calcNodeCountInDimFromElementCount,
		dictionary,
		elementLayout,
		topology );
	
	/* General info */
	
	/* Virtual info */
	
	/* BodyNL info */
	if( initFlag ){
		_BodyNL_Init( self );
	}
	
	return self;
}

void _BodyNL_Init(
		BodyNL*					self )
{
	/* General and Virtual info should already be set */
	
	/* BodyNL info */
	self->isConstructed = True;
	self->nodeCount = self->elementLayout->elementCount;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _BodyNL_Delete( void* bodyNL ) {
	BodyNL* self = (BodyNL*)bodyNL;
	
	/* Stg_Class_Delete parent */
	_NodeLayout_Delete( self );
}


void _BodyNL_Print( void* bodyNL, Stream* stream ) {
	BodyNL* self = (BodyNL*)bodyNL;
	
	/* Set the Journal for printing informations */
	Stream* bodyNLStream;
	bodyNLStream = Journal_Register( InfoStream_Type, "BodyNLStream" );

	/* Print parent */
	_NodeLayout_Print( self, stream );
	
	/* General info */
	Journal_Printf( stream, "BodyNL (ptr): (%p)\n", self );
	
	/* Virtual info */
	
	/* BodyNL info */
}

void _BodyNL_Construct( void* bodyNL, Stg_ComponentFactory *cf, void* data ){
	BodyNL *self = (BodyNL*)bodyNL;
	ElementLayout *elementLayout = NULL;
	Topology *topology = NULL;

	self->dictionary = cf->rootDict;
	
	elementLayout =  Stg_ComponentFactory_ConstructByKey(  cf,  self->name,  ElementLayout_Type, ElementLayout,  True, data ) ;	
	topology =  Stg_ComponentFactory_ConstructByKey(  cf,  self->name,  Topology_Type, Topology,  True, data ) ;
	
	_NodeLayout_Init( (NodeLayout*)self, elementLayout, topology );
	_BodyNL_Init( self );
}
	
void _BodyNL_Build( void* bodyNL, void* data ){
	
}
	
void _BodyNL_Initialise( void* bodyNL, void* data ){
	
}
	
void _BodyNL_Execute( void* bodyNL, void* data ){
	
}

void _BodyNL_Destroy( void* bodyNL, void* data ){
	
}

void _BodyNL_NodeCoordAt( void* bodyNL, Node_Index index, Coord coord ) {
	BodyNL*		self = (BodyNL*)bodyNL;
	ElementLayout*  eLayout = self->elementLayout;
	Geometry*       geometry = eLayout->geometry;
	Index*		indices = Memory_Alloc_Array( Index, eLayout->elementCornerCount, "BodyNL" );
	Index		i;
	
	eLayout->buildCornerIndices( eLayout, index, indices );
	coord[0] = 0.0;
	coord[1] = 0.0;
	coord[2] = 0.0;
	
	for( i = 0; i < eLayout->elementCornerCount; i++ ) {
		Coord corner;
		
		geometry->pointAt( geometry, indices[i], corner );
		coord[0] += corner[0];
		coord[1] += corner[1];
		coord[2] += corner[2];
	}
	
	coord[0] /= (double)eLayout->elementCornerCount;
	coord[1] /= (double)eLayout->elementCornerCount;
	coord[2] /= (double)eLayout->elementCornerCount;
}


void _BodyNL_BuildNodeCoords( void* bodyNL, Coord_List coords ) {
	BodyNL*		self = (BodyNL*)bodyNL;
	Index		n_I;
	
	for( n_I = 0; n_I < self->nodeCount; n_I++ )
		_BodyNL_NodeCoordAt( self, n_I, coords[n_I] );
}


Node_Index _BodyNL_ElementNodeCount( void* bodyNL, Element_GlobalIndex index ) {
	return 1;
}


void _BodyNL_BuildElementNodes( void* bodyNL, Element_GlobalIndex globalIndex, Element_Nodes nodes ) {
	nodes[0] = globalIndex;
}


Element_GlobalIndex _BodyNL_NodeElementCount( void* bodyNL, Node_GlobalIndex globalIndex ) {
	return 1;
}


void _BodyNL_BuildNodeElements( void* bodyNL, Node_GlobalIndex globalIndex, Node_Elements elements ) {
	elements[0] = globalIndex;
}

Node_GlobalIndex _BodyNL_CalcNodeCountInDimFromElementCount( void* nodeLayout, Element_GlobalIndex elementCountInDim )
{
	/* A body node layout has exactly as many nodes as elements in each dim */
	return elementCountInDim;
}

/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/


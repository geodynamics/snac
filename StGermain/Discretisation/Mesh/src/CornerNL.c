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
** $Id: CornerNL.c 3883 2006-10-26 05:00:23Z KathleenHumble $
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
#include "CornerNL.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


/* Textual name of this class */
const Type CornerNL_Type = "CornerNL";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

CornerNL* CornerNL_DefaultNew( Name name )
{
	return (CornerNL*)_CornerNL_New( 
		sizeof(CornerNL), 
		CornerNL_Type, 
		_CornerNL_Delete, 
		_CornerNL_Print,
		_CornerNL_Copy,
		(Stg_Component_DefaultConstructorFunction*)CornerNL_DefaultNew,
		_CornerNL_Construct,
		_CornerNL_Build,
		_CornerNL_Initialise,
		_CornerNL_Execute,
		_CornerNL_Destroy,
		name,
		False,
		_CornerNL_NodeCoordAt,
		_CornerNL_BuildNodeCoords,
		_CornerNL_ElementNodeCount,
		_CornerNL_BuildElementNodes,
		_CornerNL_NodeElementCount,
		_CornerNL_BuildNodeElements,
		_CornerNL_CalcNodeCountInDimFromElementCount,
		NULL,
		NULL,
		NULL );
}
	
CornerNL* CornerNL_New(
		Name						name,
		Dictionary*					dictionary,
		ElementLayout*					elementLayout,
		Topology*					topology )
{
	return _CornerNL_New( 
		sizeof(CornerNL), 
		CornerNL_Type, 
		_CornerNL_Delete, 
		_CornerNL_Print,
		_CornerNL_Copy,
		(Stg_Component_DefaultConstructorFunction*)CornerNL_DefaultNew,
		_CornerNL_Construct,
		_CornerNL_Build,
		_CornerNL_Initialise,
		_CornerNL_Execute,
		_CornerNL_Destroy,
		name,
		True,
		_CornerNL_NodeCoordAt,
		_CornerNL_BuildNodeCoords,
		_CornerNL_ElementNodeCount,
		_CornerNL_BuildElementNodes,
		_CornerNL_NodeElementCount,
		_CornerNL_BuildNodeElements,
		_CornerNL_CalcNodeCountInDimFromElementCount,
		dictionary,
		elementLayout,
		topology );
}

void CornerNL_Init(
		CornerNL*					self,
		Name						name,
		Dictionary*					dictionary,
		ElementLayout*					elementLayout,
		Topology*					topology )
{
	/* General info */
	self->type = CornerNL_Type;
	self->_sizeOfSelf = sizeof(CornerNL);
	self->_deleteSelf = False;
	self->dictionary = dictionary;
	
	/* Virtual info */
	self->_delete = _CornerNL_Delete;
	self->_print = _CornerNL_Print;
	self->_copy = _CornerNL_Copy;
	self->_defaultConstructor = (Stg_Component_DefaultConstructorFunction*)CornerNL_DefaultNew;
	self->_construct = _CornerNL_Construct;
	self->_build = _CornerNL_Build;
	self->_initialise = _CornerNL_Initialise;
	self->_execute = _CornerNL_Execute;
	self->_destroy = _CornerNL_Destroy;
	self->nodeCoordAt = _CornerNL_NodeCoordAt;
	self->buildNodeCoords = _CornerNL_BuildNodeCoords;
	self->elementNodeCount = _CornerNL_ElementNodeCount;
	self->buildElementNodes = _CornerNL_BuildElementNodes;
	self->nodeElementCount = _CornerNL_NodeElementCount;
	self->buildNodeElements = _CornerNL_BuildNodeElements;
	self->calcNodeCountInDimFromElementCount = _CornerNL_CalcNodeCountInDimFromElementCount;
	
	_Stg_Class_Init( (Stg_Class*)self );
	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );
	_Stg_Component_Init( (Stg_Component*)self );
	_NodeLayout_Init( (NodeLayout*)self, elementLayout, topology );
	
	/* CornerNL info */
	_CornerNL_Init( self );
}

CornerNL* _CornerNL_New(
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
		Topology*					topology)
{
	CornerNL* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(CornerNL) );
	self = (CornerNL*)_NodeLayout_New(
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
	
	/* CornerNL info */
	if( initFlag ){
		_CornerNL_Init( self );
	}
	
	return self;
}

void _CornerNL_Init(
		CornerNL*					self )
{
	/* General and Virtual info should already be set */
	
	/* CornerNL info */
	self->isConstructed = True;
	self->nodeCount = self->elementLayout->geometry->pointCount;
}

/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _CornerNL_Delete( void* cornerNL ) {
	CornerNL* self = (CornerNL*)cornerNL;
	
	/* Stg_Class_Delete parent */
	_NodeLayout_Delete( self );
}


void _CornerNL_Print( void* cornerNL, Stream* stream ) {
	CornerNL* self = (CornerNL*)cornerNL;
	
	/* Set the Journal for printing informations */
	Stream* cornerNLStream;
	cornerNLStream = Journal_Register( InfoStream_Type, "CornerNLStream" );

	/* Print parent */
	_NodeLayout_Print( self, stream );
	
	/* General info */
	Journal_Printf( stream, "CornerNL (ptr): (%p)\n", self );
	
	/* Virtual info */
	
	/* CornerNL info */
}
	
void _CornerNL_Construct( void* cornerNL, Stg_ComponentFactory* cf, void* data ){
	CornerNL *self = (CornerNL*)cornerNL;
	ElementLayout *elementLayout = NULL;
	Topology *topology = NULL;

	self->dictionary = cf->rootDict;
	
	elementLayout =  Stg_ComponentFactory_ConstructByKey(  cf,  self->name,  ElementLayout_Type, ElementLayout,  True, data ) ;	
	topology =  Stg_ComponentFactory_ConstructByKey(  cf,  self->name,  Topology_Type, Topology,  True, data ) ;
	
	_NodeLayout_Init( (NodeLayout*)self, elementLayout, topology );
	_CornerNL_Init( self );
}
	
void _CornerNL_Build( void* cornerNL, void* data ){
	
}
	
void _CornerNL_Initialise( void* cornerNL, void* data ){
	
}
	
void _CornerNL_Execute( void* cornerNL, void* data ){
	
}
	
void _CornerNL_Destroy( void* cornerNL, void* data ){
	
}

void* _CornerNL_Copy( void* cornerNL, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	CornerNL*	self = (CornerNL*)cornerNL;
	CornerNL*	newCornerNL;
	
	newCornerNL = (CornerNL*)_NodeLayout_Copy( self, dest, deep, nameExt, ptrMap );
	
	return (void*)newCornerNL;
}


void _CornerNL_NodeCoordAt( void* cornerNL, Node_Index index, Coord coord ) {
	CornerNL*       self = (CornerNL*)cornerNL;
	Geometry*       geometry = self->elementLayout->geometry;
	
	geometry->pointAt( geometry, index, coord );
}


void _CornerNL_BuildNodeCoords( void* cornerNL, Coord_List coords ) {
	CornerNL*	self = (CornerNL*)cornerNL;
	Geometry*       geometry = self->elementLayout->geometry;
	
	geometry->buildPoints( geometry, coords );
}


Node_Index _CornerNL_ElementNodeCount( void* cornerNL, Element_GlobalIndex index ) {
	CornerNL* self = (CornerNL*)cornerNL;
	
	return self->elementLayout->elementCornerCount;
}


void _CornerNL_BuildElementNodes( void* cornerNL, Element_GlobalIndex globalIndex, Element_Nodes nodes ) {
	CornerNL*       self = (CornerNL*)cornerNL;
	ElementLayout*  eLayout = self->elementLayout;
	
	eLayout->buildCornerIndices( eLayout, globalIndex, nodes );
}


Element_GlobalIndex _CornerNL_NodeElementCount( void* cornerNL, Node_GlobalIndex globalIndex ) {
	CornerNL*	self = (CornerNL*)cornerNL;
	ElementLayout*  eLayout = self->elementLayout;
	
	return eLayout->cornerElementCount( eLayout, globalIndex );
}


void _CornerNL_BuildNodeElements( void* cornerNL, Node_GlobalIndex globalIndex, Node_Elements elements ) {
	CornerNL*	self = (CornerNL*)cornerNL;
	ElementLayout*    eLayout = self->elementLayout;
	
	eLayout->buildCornerElements( eLayout, globalIndex, elements );
}


Node_GlobalIndex _CornerNL_CalcNodeCountInDimFromElementCount( void* nodeLayout, Element_GlobalIndex elementCountInDim )
{
	/* Corner node layout means we just add 1 to the element count to account for the 2 ends */
	return elementCountInDim+1;
}
/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

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
** $Id: ElementLayout.c 3817 2006-09-27 06:55:42Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"

#include "types.h"
#include "shortcuts.h"
#include "ElementLayout.h"

#include <assert.h>

/* Textual name of this class */
const Type ElementLayout_Type = "ElementLayout";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

ElementLayout* _ElementLayout_New(
		SizeT							_sizeOfSelf, 
		Type							type,
		Stg_Class_DeleteFunction*					_delete,
		Stg_Class_PrintFunction*					_print,
		Stg_Class_CopyFunction*					_copy,
		Stg_Component_DefaultConstructorFunction*	_defaultConstructor,
		Stg_Component_ConstructFunction*			_construct,
		Stg_Component_BuildFunction*				_build,
		Stg_Component_InitialiseFunction*			_initialise,
		Stg_Component_ExecuteFunction*				_execute,
		Stg_Component_DestroyFunction*				_destroy,
		Name									name,
		Bool									initFlag,
		ElementLayout_BuildFunction*  						build,
		ElementLayout_BuildCornerIndicesFunction*       	buildCornerIndices,
		ElementLayout_CornerElementCountFunction*       	cornerElementCount,
		ElementLayout_BuildCornerElementsFunction*      	buildCornerElements,
		ElementLayout_BuildEdgeIndicesFunction*			buildEdgeIndices,
		ElementLayout_EdgeElementCountFunction*			edgeElementCount,
		ElementLayout_BuildEdgeElementsFunction*		buildEdgeElements,
		ElementLayout_EdgeAtFunction*				edgeAt,
		ElementLayout_GetStaticMinAndMaxLocalCoordsFunction*	getStaticMinAndMaxLocalCoords,
		ElementLayout_GetStaticMinAndMaxGlobalCoordsFunction*	getStaticMinAndMaxGlobalCoords,
		ElementLayout_ElementWithPointFunction*			elementWithPoint,
		Dictionary*						dictionary,
		Geometry*						geometry,
		Topology*						topology )
{
	ElementLayout* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(ElementLayout) );
	self = (ElementLayout*)_Stg_Component_New( _sizeOfSelf, type, _delete, _print, _copy, _defaultConstructor, _construct, _build, 
			_initialise, _execute, _destroy, name, NON_GLOBAL );
	
	/* General info */
	self->dictionary = dictionary;
	
	/* Virtual info */
	self->build = build;
	self->buildCornerIndices = buildCornerIndices;
	self->cornerElementCount = cornerElementCount;
	self->buildCornerElements = buildCornerElements;
	self->buildEdgeIndices = buildEdgeIndices;
	self->edgeElementCount = edgeElementCount;
	self->buildEdgeElements = buildEdgeElements;
	self->edgeAt = edgeAt;
	self->getStaticMinAndMaxLocalCoords = getStaticMinAndMaxLocalCoords;
	self->getStaticMinAndMaxGlobalCoords = getStaticMinAndMaxGlobalCoords;
	self->elementWithPoint = elementWithPoint;
	
	/* ElementLayout info */
	if( initFlag ){
		_ElementLayout_Init( self, geometry, topology );
	}
	
	return self;
}

void _ElementLayout_Init(
		ElementLayout*					self,
		Geometry*					geometry,
		Topology*					topology )
{
	/* General and Virtual info should already be set */
	
	/* ElementLayout info */
	self->isConstructed = True;
	self->geometry = geometry;
	self->topology = topology;
	self->elementCount = 0;
	self->elementCornerCount = 0;
	self->cornerCount = 0;
	self->topo = NULL;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _ElementLayout_Delete( void* elementLayout ) {
	ElementLayout* self = (ElementLayout*)elementLayout;
	
	/* Stg_Class_Delete parent */
	_Stg_Component_Delete( self );
}


void _ElementLayout_Print( void* elementLayout, Stream* stream ) {
	ElementLayout* self = (ElementLayout*)elementLayout;
	
	/* Set the Journal for printing informations */
	Stream* elementLayoutStream;
	elementLayoutStream = Journal_Register( InfoStream_Type, "ElementLayoutStream" );

	/* Print parent */
	_Stg_Component_Print( self, stream );
	
	/* General info */
	Journal_Printf( stream, "ElementLayout (ptr): (%p)\n", self );
	
	/* Virtual info */
	
	/* ElementLayout info */
}


void* _ElementLayout_Copy( void* elementLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	ElementLayout*	self = (ElementLayout*)elementLayout;
	ElementLayout*	newElementLayout;
	PtrMap*		map = ptrMap;
	Bool		ownMap = False;
	
	if( !map ) {
		map = PtrMap_New( 10 );
		ownMap = True;
	}
	
	newElementLayout = (ElementLayout*)_Stg_Component_Copy( self, dest, deep, nameExt, map );
	
	/* Virtual methods */
	newElementLayout->build = self->build;
	newElementLayout->buildCornerIndices = self->buildCornerIndices;
	newElementLayout->cornerElementCount = self->cornerElementCount;
	newElementLayout->buildCornerElements = self->buildCornerElements;
	newElementLayout->buildEdgeIndices = self->buildEdgeIndices;
	newElementLayout->edgeElementCount = self->edgeElementCount;
	newElementLayout->buildEdgeElements = self->buildEdgeElements;
	newElementLayout->edgeAt = self->edgeAt;
	newElementLayout->elementWithPoint = self->elementWithPoint;
	
	newElementLayout->dictionary = self->dictionary;
	newElementLayout->elementCount = self->elementCount;
	newElementLayout->elementCornerCount = self->elementCornerCount;
	newElementLayout->cornerCount = self->cornerCount;
	newElementLayout->elementEdgeCount = self->elementEdgeCount;
	newElementLayout->edgeCount = self->edgeCount;
	
	if( deep ) {
		newElementLayout->geometry = (Geometry*)Stg_Class_Copy( self->geometry, NULL, deep, nameExt, map );
		newElementLayout->topology = (Topology*)Stg_Class_Copy( self->topology, NULL, deep, nameExt, map );
	}
	else {
		newElementLayout->geometry = self->geometry;
		newElementLayout->topology = self->topology;
	}
	
	if( ownMap ) {
		Stg_Class_Delete( map );
	}
	
	return (void*)newElementLayout;
}


void ElementLayout_Build( void* elementLayout, void* decomp ) {
	ElementLayout* self = (ElementLayout*)elementLayout;

	self->build( self, decomp );
}


void _ElementLayout_Build( void* elementLayout, void* decomp ) {
	ElementLayout* self = (ElementLayout*)elementLayout;

	/* Default implementation does nothing. */
	Build( self->geometry, NULL, False );
}

void ElementLayout_BuildCornerElements( void* elementLayout, Index corner, Element_GlobalIndex* elements ) {
	ElementLayout* self = (ElementLayout*)elementLayout;

	self->buildCornerElements( self, corner, elements );
}

void ElementLayout_BuildEdgeIndices( void* elementLayout, Element_GlobalIndex globalIndex, Index* edges ) {
	ElementLayout* self = (ElementLayout*)elementLayout;

	self->buildEdgeIndices( self, globalIndex, edges );
}

void ElementLayout_EdgeAt( void* elementLayout, Index index, Edge edge ) {
	ElementLayout* self = (ElementLayout*)elementLayout;

	self->edgeAt( self, index, edge );
}

Element_GlobalIndex ElementLayout_ElementWithPoint( void* elementLayout, void* decomp, Coord point, void* mesh, 
						    PartitionBoundaryStatus boundaryStatus, unsigned nHints, unsigned* hints ) {
	ElementLayout* self = (ElementLayout*)elementLayout;

	return self->elementWithPoint( self, decomp, point, mesh, boundaryStatus, nHints, hints );
}

Bool ElementLayout_GetStaticMinAndMaxLocalCoords( void* elementLayout, Coord min, Coord max ) {
	ElementLayout* self = (ElementLayout*)elementLayout;
	return self->getStaticMinAndMaxLocalCoords( self, min, max );
}

Bool _ElementLayout_GetStaticMinAndMaxLocalCoords( void* elementLayout, Coord min, Coord max ) {
	/* Default implementation: no static shortcut known:- return false. */
	return False;
}


Bool ElementLayout_GetStaticMinAndMaxGlobalCoords( void* elementLayout, Coord min, Coord max ) {
	ElementLayout* self = (ElementLayout*)elementLayout;
	return self->getStaticMinAndMaxGlobalCoords( self, min, max );
}

Bool _ElementLayout_GetStaticMinAndMaxGlobalCoords( void* elementLayout, Coord min, Coord max ) {
	/* Default implementation: no static shortcut known:- return false. */
	return False;
}

/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

Index ElementLayout_BuildEdgeSubset( void* elementLayout, Index elementCount, Index* elements, Index** edges ) {
	ElementLayout*		self = (ElementLayout*)elementLayout;
	Element_GlobalIndex	edgeCnt = 0;
	Element_GlobalIndex     element_I;
	Index*			eEdges;
	IndexSet*		set;
	
	set = IndexSet_New( self->edgeCount );
	eEdges = Memory_Alloc_Array( Element_GlobalIndex, self->elementEdgeCount, "ElementLayout" );
	
	for( element_I = 0; element_I < elementCount; element_I++ ) {
		Index   edge_I;
		
		self->buildEdgeIndices( self, elements[element_I], eEdges );
		for( edge_I = 0; edge_I < self->elementEdgeCount; edge_I++ )
			IndexSet_Add( set, eEdges[edge_I] );
	}
	
	Memory_Free( eEdges );
	
	IndexSet_GetMembers( set, &edgeCnt, edges );
	Stg_Class_Delete( set );
	
	return edgeCnt;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

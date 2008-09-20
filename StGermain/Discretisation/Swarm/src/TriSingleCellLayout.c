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
** $Id: TriSingleCellLayout.c 3885 2006-10-26 05:52:26Z KathleenHumble $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"
#include "Discretisation/Mesh/Mesh.h"

#include "types.h"
#include "shortcuts.h"
#include "CellLayout.h"
#include "TriSingleCellLayout.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

const Type TriSingleCellLayout_Type = "TriSingleCellLayout";

TriSingleCellLayout* TriSingleCellLayout_DefaultNew( Name name )
{
	return (TriSingleCellLayout*)_TriSingleCellLayout_New( sizeof(TriSingleCellLayout), TriSingleCellLayout_Type,
		_TriSingleCellLayout_Delete, _TriSingleCellLayout_Print, _TriSingleCellLayout_Copy,
		(Stg_Component_DefaultConstructorFunction*)TriSingleCellLayout_DefaultNew, _TriSingleCellLayout_Construct,
		_TriSingleCellLayout_Build, _TriSingleCellLayout_Initialise, _TriSingleCellLayout_Execute,
		_TriSingleCellLayout_Destroy, name, False,
		_TriSingleCellLayout_CellLocalCount, _TriSingleCellLayout_CellShadowCount,
		_TriSingleCellLayout_PointCount, _TriSingleCellLayout_InitialisePoints,
		_TriSingleCellLayout_MapElementIdToCellId, _TriSingleCellLayout_IsInCell,
		_TriSingleCellLayout_CellOf, _TriSingleCellLayout_GetShadowInfo,
		0, NULL );
}

TriSingleCellLayout* TriSingleCellLayout_New( Name name, int dim, Dictionary* dictionary ) { 
	return _TriSingleCellLayout_New( sizeof(TriSingleCellLayout), TriSingleCellLayout_Type, _TriSingleCellLayout_Delete,
		_TriSingleCellLayout_Print, _TriSingleCellLayout_Copy,
		(Stg_Component_DefaultConstructorFunction*)TriSingleCellLayout_DefaultNew, _TriSingleCellLayout_Construct,
		_TriSingleCellLayout_Build, _TriSingleCellLayout_Initialise, _TriSingleCellLayout_Execute,
		_TriSingleCellLayout_Destroy, name, True,
		_TriSingleCellLayout_CellLocalCount, _TriSingleCellLayout_CellShadowCount,
		_TriSingleCellLayout_PointCount, _TriSingleCellLayout_InitialisePoints,
		_TriSingleCellLayout_MapElementIdToCellId, _TriSingleCellLayout_IsInCell,
		_TriSingleCellLayout_CellOf, _TriSingleCellLayout_GetShadowInfo, dim, dictionary );
}

void TriSingleCellLayout_Init( TriSingleCellLayout* self, Name name, int dim, Dictionary* dictionary ) { 
	/* General info */
	self->type = TriSingleCellLayout_Type;
	self->_sizeOfSelf = sizeof(TriSingleCellLayout);
	self->_deleteSelf = False;
	self->dictionary = dictionary;
	
	/* Virtual info */
	self->_delete = _TriSingleCellLayout_Delete;
	self->_print = _TriSingleCellLayout_Print;
	self->_copy = _TriSingleCellLayout_Copy;
	self->_defaultConstructor = (Stg_Component_DefaultConstructorFunction*) TriSingleCellLayout_DefaultNew;
	self->_construct = _TriSingleCellLayout_Construct;
	self->_build = _TriSingleCellLayout_Build;
	self->_initialise = _TriSingleCellLayout_Initialise;
	self->_execute = _TriSingleCellLayout_Execute;
	self->_destroy = _TriSingleCellLayout_Destroy;
	self->_cellLocalCount = _TriSingleCellLayout_CellLocalCount,
	self->_cellShadowCount = _TriSingleCellLayout_CellShadowCount,
	self->_pointCount = _TriSingleCellLayout_PointCount,
	self->_initialisePoints = _TriSingleCellLayout_InitialisePoints,
	self->_mapElementIdToCellId = _TriSingleCellLayout_MapElementIdToCellId;
	self->_isInCell = _TriSingleCellLayout_IsInCell,
	self->_cellOf = _TriSingleCellLayout_CellOf,
	self->_getShadowInfo = _TriSingleCellLayout_GetShadowInfo;

	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );
	_CellLayout_Init( (CellLayout*)self );
	
	/* TriSingleCellLayout info */
	_TriSingleCellLayout_Init( self, dim );
}

TriSingleCellLayout* _TriSingleCellLayout_New( 
		SizeT					_sizeOfSelf,
		Type					type,
		Stg_Class_DeleteFunction*			_delete,
		Stg_Class_PrintFunction*			_print,
		Stg_Class_CopyFunction*			_copy, 
		Stg_Component_DefaultConstructorFunction*	_defaultConstructor,
		Stg_Component_ConstructFunction*			_construct,
		Stg_Component_BuildFunction*		_build,
		Stg_Component_InitialiseFunction*		_initialise,
		Stg_Component_ExecuteFunction*		_execute,
		Stg_Component_DestroyFunction*		_destroy,
		Name							name,
		Bool							initFlag,
		CellLayout_CellCountFunction*		_cellLocalCount,
		CellLayout_CellCountFunction*		_cellShadowCount,
		CellLayout_PointCountFunction*		_pointCount,
		CellLayout_InitialisePointsFunction*	_initialisePoints,
		CellLayout_MapElementIdToCellIdFunction*	_mapElementIdToCellId,		
		CellLayout_IsInCellFunction*		_isInCell, 
		CellLayout_CellOfFunction*		_cellOf,
		CellLayout_GetShadowInfoFunction*	_getShadowInfo,		
		int					dim, 
		Dictionary*				dictionary )
{
	TriSingleCellLayout* self;
	
	/* Allocate memory */
	self = (TriSingleCellLayout*)_CellLayout_New( 
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
		_cellLocalCount,
		_cellShadowCount,
		_pointCount,
		_initialisePoints,
		_mapElementIdToCellId,
		_isInCell,
		_cellOf,
		_getShadowInfo );
	
	/* General info */
	self->dictionary = dictionary;
	
	/* Virtual info */
	
	/* TriSingleCellLayout info */
	if( initFlag ){
		_TriSingleCellLayout_Init( self, dim );
	}
	
	return self;
}


void _TriSingleCellLayout_Init( TriSingleCellLayout* self, int dim ) { 
	/* General and Virtual info should already be set */
	
	/* SingleCellInfo info */
	self->isConstructed = True;
	self->dim = dim;
}


void _TriSingleCellLayout_Delete( void* triSingleCellLayout ) {
	TriSingleCellLayout* self = (TriSingleCellLayout*)triSingleCellLayout;
	
	/* Stg_Class_Delete parent class */
	_CellLayout_Delete( self );
}

void _TriSingleCellLayout_Print( void* triSingleCellLayout, Stream* stream ) {
	TriSingleCellLayout* self = (TriSingleCellLayout*)triSingleCellLayout;
	
	/* Set the Journal for printing informations */
	Stream* triSingleCellLayoutStream = stream;
	
	/* General info */
	Journal_Printf( triSingleCellLayoutStream, "TriSingleCellLayout (ptr): %p\n", self ); 
	
	/* Parent class info */
	_CellLayout_Print( self, stream );
	
	/* Virtual info */
	
	/* TriSingleCellLayout info */
	Journal_Printf( triSingleCellLayoutStream, "self->dim: %u", self->dim );
}


void* _TriSingleCellLayout_Copy( void* triSingleCellLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	TriSingleCellLayout*	self = (TriSingleCellLayout*)triSingleCellLayout;
	TriSingleCellLayout*	newTriSingleCellLayout;
	
	newTriSingleCellLayout = _CellLayout_Copy( self, dest, deep, nameExt, ptrMap );
	
	newTriSingleCellLayout->dictionary = self->dictionary;
	newTriSingleCellLayout->dim = self->dim;
	
	return (void*)newTriSingleCellLayout;
}
	
void _TriSingleCellLayout_Construct( void *triSingleCellLayout, Stg_ComponentFactory *cf, void* data ){
	TriSingleCellLayout *self = (TriSingleCellLayout*)triSingleCellLayout;
	Dimension_Index dim = 0;

	dim = Stg_ComponentFactory_GetRootDictUnsignedInt( cf, "dim", 0 );
	assert( dim );

	self->dictionary = cf->rootDict;
	
	_CellLayout_Init( (CellLayout*)self );
	_TriSingleCellLayout_Init( (TriSingleCellLayout*)self, dim );
}
	
void _TriSingleCellLayout_Build( void* triSingleCellLayout, void* data ){
	
}
	
void _TriSingleCellLayout_Initialise( void* triSingleCellLayout, void* data ){
	
}
	
void _TriSingleCellLayout_Execute( void* triSingleCellLayout, void* data ){
	
}
	
void _TriSingleCellLayout_Destroy( void* triSingleCellLayout, void* data ){
	
}

Cell_Index _TriSingleCellLayout_CellLocalCount( void* triSingleCellLayout ) {
	/* There is only one cell... */
	return 1;
}


Cell_Index _TriSingleCellLayout_CellShadowCount( void* triSingleCellLayout ) {
	/* No shadow cells */
	return 0;
}


Cell_PointIndex _TriSingleCellLayout_PointCount( void* triSingleCellLayout, Cell_Index cellIndex ) {
	TriSingleCellLayout* self = (TriSingleCellLayout*)triSingleCellLayout;
	
	switch( self->dim ) {
		case 1:
			return 2;
		case 2:
			return 3;
		case 3:
			return 4;
		default:
			assert( 0 );
	}
	return 0;
}


void _TriSingleCellLayout_InitialisePoints( 
		void*			triSingleCellLayout, 
		Cell_Index		cellIndex, 
		Cell_PointIndex		pointCount, 
		Cell_Points		points ) 
{
	TriSingleCellLayout* self = (TriSingleCellLayout*)triSingleCellLayout;
	
	switch( self->dim ) {
		case 1:
			assert( 0 );
		case 2:
			points[0] = Memory_Alloc( Coord, "points[0]" );
			points[1] = Memory_Alloc( Coord, "points[1]" );
			points[2] = Memory_Alloc( Coord, "points[2]" );
			
			(*points[0])[0] = 0.0f;
			(*points[0])[1] = 0.0f;
			(*points[1])[0] = 1.0f;
			(*points[1])[1] = 0.0f;
			(*points[2])[0] = 0.0f;
			(*points[2])[1] = 1.0f;
			break;
		case 3:
			assert( 0 );
		default:
			assert( 0 );
	}
}


Cell_Index _TriSingleCellLayout_MapElementIdToCellId( void* cellLayout, Element_DomainIndex element_dI ) {
	
	/* Always 0: see the header comment */
	return 0;
}


Bool _TriSingleCellLayout_IsInCell( void* triSingleCellLayout, Cell_Index cellIndex, void* particle ) {
	assert( 0 );
	return 0;
}


Cell_Index _TriSingleCellLayout_CellOf( void* triSingleCellLayout, void* particle ) {
	assert( 0 );
	return 0;
}


ShadowInfo* _TriSingleCellLayout_GetShadowInfo( void* triSingleCellLayout ) {
	//TriSingleCellLayout*      self = (TriSingleCellLayout*)triSingleCellLayout;

	/* TODO: this should return a shadow info with at least nbr info for my processors */
	Journal_Firewall( 0, Swarm_Warning, "Error: %s not implemented yet!\n", __func__ );
	return NULL;
}

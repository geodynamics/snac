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
** $Id: HexaEL.c 3883 2006-10-26 05:00:23Z KathleenHumble $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"

#include "types.h"
#include "shortcuts.h"
#include "ElementLayout.h"
#include "HexaEL.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "MeshDecomp.h"
#include "Decomp.h"
#include "Decomp_Sync.h"
#include "MeshTopology.h"
#include "HexaMD.h"
#include "MeshClass.h"


/* Textual name of this class */
const Type HexaEL_Type = "HexaEL";
const Name defaultHexaEL_IJK26TopologyName = "defaultHexaEL_IJK26TopologyName";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

HexaEL* HexaEL_DefaultNew( Name name )
{
	return (HexaEL*)_HexaEL_New( 
		sizeof(HexaEL), 
		HexaEL_Type, 
		_HexaEL_Delete, 
		_HexaEL_Print,
		_HexaEL_Copy,
		(Stg_Component_DefaultConstructorFunction*)HexaEL_DefaultNew,
		_HexaEL_Construct,
		_HexaEL_Build,
		_HexaEL_Initialise,
		_HexaEL_Execute,
		_HexaEL_Destroy,
		name,
		False,
		_ElementLayout_Build,
		_HexaEL_BuildCornerIndices,
		_HexaEL_CornerElementCount,
		_HexaEL_BuildCornerElements,
		_HexaEL_BuildEdgeIndices,
		_HexaEL_EdgeElementCount,
		_HexaEL_BuildEdgeElements,
		_HexaEL_EdgeAt,
		_ElementLayout_GetStaticMinAndMaxLocalCoords,
		_ElementLayout_GetStaticMinAndMaxGlobalCoords,
		_HexaEL_ElementWithPoint,
		0,
		NULL,
		NULL );
}

HexaEL* HexaEL_New(
	Name						name,
	Dimension_Index             dim,
	Dictionary*					dictionary,
	Geometry*					geometry )
{
	return _HexaEL_New( 
		sizeof(HexaEL), 
		HexaEL_Type, 
		_HexaEL_Delete, 
		_HexaEL_Print,
		_HexaEL_Copy,
		(Stg_Component_DefaultConstructorFunction*)HexaEL_DefaultNew,
		_HexaEL_Construct,
		_HexaEL_Build,
		_HexaEL_Initialise,
		_HexaEL_Execute,
		_HexaEL_Destroy,
		name,
		True,
		_ElementLayout_Build,
		_HexaEL_BuildCornerIndices,
		_HexaEL_CornerElementCount,
		_HexaEL_BuildCornerElements,
		_HexaEL_BuildEdgeIndices,
		_HexaEL_EdgeElementCount,
		_HexaEL_BuildEdgeElements,
		_HexaEL_EdgeAt,
		_ElementLayout_GetStaticMinAndMaxLocalCoords,
		_ElementLayout_GetStaticMinAndMaxGlobalCoords,
		_HexaEL_ElementWithPoint,
		dim,
		dictionary,
		geometry );
}

void HexaEL_Init(
	HexaEL*						self,
	Name						name,
	Dimension_Index             dim,
	Dictionary*					dictionary,
	Geometry*					geometry )
{
	/* General info */
	self->type = HexaEL_Type;
	self->_sizeOfSelf = sizeof(HexaEL);
	self->_deleteSelf = False;
	self->dictionary = dictionary;
	
	/* Virtual info */
	self->_delete = _HexaEL_Delete;
	self->_print = _HexaEL_Print;
	self->_copy = _HexaEL_Copy;
	self->_defaultConstructor = (Stg_Component_DefaultConstructorFunction*)HexaEL_DefaultNew;
	self->_construct = _HexaEL_Construct;
	self->_build = _HexaEL_Build;
	self->_initialise = _HexaEL_Initialise;
	self->_execute = _HexaEL_Execute;
	self->_destroy = _HexaEL_Destroy;
	self->build = _ElementLayout_Build;
	self->buildCornerIndices = _HexaEL_BuildCornerIndices;
	self->cornerElementCount = _HexaEL_CornerElementCount;
	self->buildCornerElements = _HexaEL_BuildCornerElements;
	self->buildEdgeIndices = _HexaEL_BuildEdgeIndices;
	self->edgeElementCount = _HexaEL_EdgeElementCount;
	self->buildEdgeElements = _HexaEL_BuildEdgeElements;
	self->edgeAt = _HexaEL_EdgeAt;
	self->getStaticMinAndMaxLocalCoords = _ElementLayout_GetStaticMinAndMaxLocalCoords;
	self->getStaticMinAndMaxGlobalCoords = _ElementLayout_GetStaticMinAndMaxGlobalCoords;
	self->elementWithPoint = _HexaEL_ElementWithPoint;

	_Stg_Class_Init( (Stg_Class*)self );
	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );
	_Stg_Component_Init( (Stg_Component*)self );
	_ElementLayout_Init( (ElementLayout*)self, geometry, (Topology*)IJK26Topology_New( defaultHexaEL_IJK26TopologyName, dictionary ) );
	self->topologyWasCreatedInternally = True;
	
	/* HexaEL info */
	_HexaEL_Init( self, dim, NULL, NULL );
}

HexaEL* _HexaEL_New(
	SizeT                                                   _sizeOfSelf, 
	Type                                                    type,
	Stg_Class_DeleteFunction*                               _delete,
	Stg_Class_PrintFunction*                                _print,
	Stg_Class_CopyFunction*                                 _copy, 
	Stg_Component_DefaultConstructorFunction*               _defaultConstructor,
	Stg_Component_ConstructFunction*                        _construct,
	Stg_Component_BuildFunction*                            _build,
	Stg_Component_InitialiseFunction*                       _initialise,
	Stg_Component_ExecuteFunction*                          _execute,
	Stg_Component_DestroyFunction*                          _destroy,
	Name                                                    name,
	Bool                                                    initFlag,
	ElementLayout_BuildFunction*                            build,		
	ElementLayout_BuildCornerIndicesFunction*               buildCornerIndices,
	ElementLayout_CornerElementCountFunction*               cornerElementCount,
	ElementLayout_BuildCornerElementsFunction*              buildCornerElements,
	ElementLayout_BuildEdgeIndicesFunction*                 buildEdgeIndices,
	ElementLayout_EdgeElementCountFunction*                 edgeElementCount,
	ElementLayout_BuildEdgeElementsFunction*                buildEdgeElements,
	ElementLayout_EdgeAtFunction*                           edgeAt,
	ElementLayout_GetStaticMinAndMaxLocalCoordsFunction*    getStaticMinAndMaxLocalCoords,
	ElementLayout_GetStaticMinAndMaxGlobalCoordsFunction*   getStaticMinAndMaxGlobalCoords,				
	ElementLayout_ElementWithPointFunction*                 elementWithPoint,
	Dimension_Index                                         dim,
	Dictionary*                                             dictionary,
	Geometry*                                               geometry )
{
	HexaEL* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(HexaEL) );
	self = (HexaEL*)_ElementLayout_New(
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
		build,
		buildCornerIndices,
		cornerElementCount,
		buildCornerElements,
		buildEdgeIndices,
		edgeElementCount,
		buildEdgeElements,
		edgeAt,
		getStaticMinAndMaxLocalCoords,
		getStaticMinAndMaxGlobalCoords,
		elementWithPoint,
		dictionary,
		geometry,
		(initFlag)?(Topology*)IJK26Topology_New( defaultHexaEL_IJK26TopologyName, dictionary ):NULL );
	
	/* General info */
	
	/* Virtual info */
	
	/* HexaEL info */
	if( initFlag ){
		_HexaEL_Init( self, dim, NULL, NULL );
		self->topologyWasCreatedInternally = True;
	}
	
	return self;
}

void _HexaEL_Init( HexaEL* self, Dimension_Index dim, IJK pointSize, IJK elementSize ) {
	/* General and Virtual info should already be set */
	
	/* HexaEL info */
	self->isConstructed = True;
	self->dim = dim;
	if (pointSize) {
		memcpy( self->pointSize, pointSize, sizeof(IJK) );
	}
	else {
		self->pointSize[ I_AXIS ] = Dictionary_GetUnsignedInt_WithDefault( self->dictionary, "meshSizeI", 2 );
		self->pointSize[ J_AXIS ] = Dictionary_GetUnsignedInt_WithDefault( self->dictionary, "meshSizeJ", 2 );
		self->pointSize[ K_AXIS ] = Dictionary_GetUnsignedInt_WithDefault( self->dictionary, "meshSizeK", 2 );
	}

	if ( dim <= 2 ) {
		self->pointSize[ K_AXIS ] = 1;
		if( dim == 1 )
			self->pointSize[J_AXIS] = 1;
	}
	assert( self->pointSize[0] * self->pointSize[1] * self->pointSize[2] );
	
	if ( elementSize ) {
		memcpy( self->elementSize, elementSize, sizeof(IJK) );
	}
	else {
		self->elementSize[0] = self->pointSize[0] - 1;
		self->elementSize[1] = self->pointSize[1] - 1;
		self->elementSize[2] = self->pointSize[2] - 1;
	}
	if ( dim <= 2 ) {
		self->elementSize[ K_AXIS ] = 1;
		if( dim == 1 )
			self->elementSize[J_AXIS] = 1;
	}
	self->elementCount = self->elementSize[0] * self->elementSize[1] * self->elementSize[2];
	assert( self->elementCount );
	
	self->cornerCount = self->geometry->pointCount;
	if( dim == 1 ) {
		self->elementCornerCount = 2;
		self->elementEdgeCount = 0;
		self->edgeCount = 0;
	}
	else if ( dim == 2 ) {
		self->elementCornerCount = 4;
		self->elementEdgeCount = 4;
		self->edgeCount = self->elementSize[0] * self->pointSize[1] + self->elementSize[1] * self->pointSize[0];
	}
	else {
		self->elementCornerCount = 8;
		self->elementEdgeCount = 12;
		self->edgeCount = (self->elementSize[0] * self->pointSize[1] + self->elementSize[1] * self->pointSize[0]) * 
			self->pointSize[2] + self->pointSize[0] * self->pointSize[1] * self->elementSize[2];
	}
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _HexaEL_Delete( void* hexaEL ) {
	HexaEL* self = (HexaEL*)hexaEL;
	

	/* modified by Patrick Sunter 18 May 2006 : in this component's construct it now grabs a topology from
	   the component factory : thus we shouldn't delete it as it may be shared. Only delete if its using
	   the default name and was thus constructed the old way */
	if ( self->topologyWasCreatedInternally == True ) {
		Stg_Class_Delete( self->topology );
		self->topology = NULL;
	}
	
	/* Stg_Class_Delete parent */
	_ElementLayout_Delete( self );
}


void _HexaEL_Print( void* hexaEL, Stream* stream ) {
	HexaEL* self = (HexaEL*)hexaEL;
	
	/* Set the Journal for printing informations */
	Stream* hexaELStream;
	hexaELStream = Journal_Register( InfoStream_Type, "HexaELStream" );

	/* Print parent */
	_ElementLayout_Print( self, stream );
	
	/* General info */
	Journal_Printf( stream, "HexaEL (ptr): (%p)\n", self );
	
	/* Virtual info */
	
	/* HexaEL info */
}


void* _HexaEL_Copy( void* hexaEL, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	HexaEL*	self = (HexaEL*)hexaEL;
	HexaEL*	newHexaEL;
	
	newHexaEL = (HexaEL*)_ElementLayout_Copy( self, dest, deep, nameExt, ptrMap );
	
	newHexaEL->pointSize[0] = self->pointSize[0];
	newHexaEL->pointSize[1] = self->pointSize[1];
	newHexaEL->pointSize[2] = self->pointSize[2];
	newHexaEL->elementSize[0] = self->elementSize[0];
	newHexaEL->elementSize[1] = self->elementSize[1];
	newHexaEL->elementSize[2] = self->elementSize[2];
	
	return (void*)newHexaEL;
}

void _HexaEL_Construct( void* hexaEL, Stg_ComponentFactory* cf, void* data ) {
	HexaEL*   self = (HexaEL*) hexaEL;
	IJK26Topology *topology = NULL;
	Geometry *geometry = NULL;
	Dimension_Index dim;

	self->dictionary = cf->rootDict;
	
	topology =  Stg_ComponentFactory_ConstructByKey(  cf,  self->name,  Topology_Type, IJK26Topology,  True, data  ) ;
	geometry =  Stg_ComponentFactory_ConstructByKey(  cf,  self->name,  Geometry_Type, Geometry,  True, data  ) ;

	dim = Dictionary_GetUnsignedInt( self->dictionary, "dim" );
	
	_ElementLayout_Init( (ElementLayout*)self, geometry, (Topology*)topology );
	_HexaEL_Init( (HexaEL*)self, dim, ((BlockGeometry*) geometry)->size, topology->size );
}
	
void _HexaEL_Build( void* hexaEL, void* data )
{
	
}
	
void _HexaEL_Initialise( void* hexaEL, void* data )
{
	
}
	
void _HexaEL_Execute( void* hexaEL, void* data )
{
	
}
	
void _HexaEL_Destroy( void* hexaEL, void* data )
{
	
}

#define HEL_P_3DTo1D_3( self, i, j, k )					\
	((k) * (self)->pointSize[0] * (self)->pointSize[1] + (j) * (self)->pointSize[0] + (i))

#define HEL_P_3DTo1D( self, ijk )			\
	HEL_P_3DTo1D_3( self, ijk[0], ijk[1], ijk[2] )


#define HEL_P_1DTo3D_3( self, index, i, j, k )				\
	*(i) = (index) % (self)->pointSize[0];				\
	*(j) = ((index) / (self)->pointSize[0]) % (self)->pointSize[1]; \
	*(k) = (index) / ((self)->pointSize[0] * (self)->pointSize[1])

#define HEL_P_1DTo3D( self, index, ijk )				\
	HEL_P_1DTo3D_3( self, index, &(ijk)[0], &(ijk)[1], &(ijk)[2] )


#define HEL_E_3DTo1D_3( self, i, j, k )					\
	((k) * (self)->elementSize[0] * (self)->elementSize[1] + (j) * (self)->elementSize[0] + (i))


#define HEL_E_1DTo3D_3( self, index, i, j, k )				\
	*(i) = (index) % (self)->elementSize[0];			\
	*(j) = ((index) / (self)->elementSize[0]) % (self)->elementSize[1]; \
	*(k) = (index) / ((self)->elementSize[0] * (self)->elementSize[1])

#define HEL_E_1DTo3D( self, index, ijk )				\
	HEL_E_1DTo3D_3( self, index, &(ijk)[0], &(ijk)[1], &(ijk)[2] )

#define HEL_P_2DTo1D_2( self, i, j )		\
	((j) * (self)->pointSize[0] + (i))

#define HEL_P_2DTo1D( self, ij )		\
	HEL_P_2DTo1D_2( self, ij[0], ij[1] )


#define HEL_P_1DTo2D_2( self, index, i, j)				\
	*(i) = (index) % (self)->pointSize[0];				\
	*(j) = ((index) / (self)->pointSize[0]) % (self)->pointSize[1]

#define HEL_P_1DTo2D( self, index, ij )				\
	HEL_P_1DTo2D_2( self, index, &(ij)[0], &(ij)[1] )


#define HEL_E_2DTo1D_2( self, i, j )		\
	((j) * (self)->elementSize[0] + (i))


#define HEL_E_1DTo2D_2( self, index, i, j )				\
	*(i) = (index) % (self)->elementSize[0];			\
	*(j) = ((index) / (self)->elementSize[0]) % (self)->elementSize[1]

#define HEL_E_1DTo2D( self, index, ij )				\
	HEL_E_1DTo2D_2( self, index, &(ij)[0], &(ij)[1] )


void _HexaEL_BuildCornerIndices( void* hexaEL, Element_GlobalIndex globalIndex, Index* points ) {
	HexaEL*		self = (HexaEL*)hexaEL;
	IJK		ijk;
	
	HEL_E_1DTo3D( self, globalIndex, ijk );
	
	points[0] = HEL_P_3DTo1D_3( self, ijk[0],       ijk[1],		ijk[2] );
	points[1] = HEL_P_3DTo1D_3( self, ijk[0] + 1,	ijk[1],		ijk[2] );
	points[3] = HEL_P_3DTo1D_3( self, ijk[0],       ijk[1] + 1,	ijk[2] );
	points[2] = HEL_P_3DTo1D_3( self, ijk[0] + 1,	ijk[1] + 1,	ijk[2] );
	if ( self->dim == 3 ) {
		points[4] = HEL_P_3DTo1D_3( self, ijk[0],       ijk[1],		ijk[2] + 1 );
		points[5] = HEL_P_3DTo1D_3( self, ijk[0] + 1,	ijk[1],		ijk[2] + 1 );
		points[7] = HEL_P_3DTo1D_3( self, ijk[0],       ijk[1] + 1,	ijk[2] + 1 );
		points[6] = HEL_P_3DTo1D_3( self, ijk[0] + 1,	ijk[1] + 1,	ijk[2] + 1 );
	}
}

Element_GlobalIndex _HexaEL_CornerElementCount( void* hexaEL, Index corner ) {
	HexaEL*		self = (HexaEL*)hexaEL;
	return ( self->dim == 1 ? 2 : self->dim == 2 ? 4 : 8 );
}

void _HexaEL_BuildCornerElements( void* hexaEL, Index corner, Element_GlobalIndex* elements ) {
	HexaEL*		self = (HexaEL*)hexaEL;

	self->buildCornerElements = (self->dim == 1 ? _HexaEL_BuildCornerElements1D : 
				     self->dim == 2 ? _HexaEL_BuildCornerElements2D : _HexaEL_BuildCornerElements3D );

	ElementLayout_BuildCornerElements( self, corner, elements );
}

void _HexaEL_BuildCornerElements1D( void* hexaEL, Index corner, Element_GlobalIndex* elements ) {
	HexaEL*		self = (HexaEL*)hexaEL;

	if( corner > 0 )
		elements[0] = corner;
	else
		elements[0] = self->elementCount;

	if( corner < self->elementCount - 1 )
		elements[1] = corner + 1;
	else
		elements[1] = self->elementCount;
}

void _HexaEL_BuildCornerElements2D( void* hexaEL, Index corner, Element_GlobalIndex* elements ) {
	HexaEL*		self = (HexaEL*)hexaEL;
	IJK		ij;
	
	HEL_P_1DTo2D( self, corner, ij );
	
	if( ij[0] > 0 && ij[1] > 0 )
		elements[0] = HEL_E_2DTo1D_2( self, ij[0] - 1, ij[1] - 1 );
	else
		elements[0] = self->elementCount;
	
	if( ij[0] < self->elementSize[0] && ij[1] > 0 )
		elements[1] = HEL_E_2DTo1D_2( self, ij[0], ij[1] - 1 );
	else
		elements[1] = self->elementCount;
	
	if( ij[0] > 0 && ij[1] < self->elementSize[1] )
		elements[2] = HEL_E_2DTo1D_2( self, ij[0] - 1, ij[1] );
	else
		elements[2] = self->elementCount;
	
	if( ij[0] < self->elementSize[0] && ij[1] < self->elementSize[1] )
		elements[3] = HEL_E_2DTo1D_2( self, ij[0], ij[1] );
	else
		elements[3] = self->elementCount;
}

void _HexaEL_BuildCornerElements3D( void* hexaEL, Index corner, Element_GlobalIndex* elements ) {
	HexaEL*		self = (HexaEL*)hexaEL;
	IJK		ijk;
	
	HEL_P_1DTo3D( self, corner, ijk );
	
	if( ijk[0] > 0 && ijk[1] > 0 && ijk[2] > 0 )
		elements[0] = HEL_E_3DTo1D_3( self, ijk[0] - 1, ijk[1] - 1, ijk[2] - 1 );
	else
		elements[0] = self->elementCount;
	
	if( ijk[0] < self->elementSize[0] && ijk[1] > 0 && ijk[2] > 0 )
		elements[1] = HEL_E_3DTo1D_3( self, ijk[0], ijk[1] - 1, ijk[2] - 1 );
	else
		elements[1] = self->elementCount;
	
	if( ijk[0] > 0 && ijk[1] < self->elementSize[1] && ijk[2] > 0 )
		elements[2] = HEL_E_3DTo1D_3( self, ijk[0] - 1, ijk[1], ijk[2] - 1 );
	else
		elements[2] = self->elementCount;
	
	if( ijk[0] < self->elementSize[0] && ijk[1] < self->elementSize[1] && ijk[2] > 0 )
		elements[3] = HEL_E_3DTo1D_3( self, ijk[0], ijk[1], ijk[2] - 1 );
	else
		elements[3] = self->elementCount;
	
	if( ijk[0] > 0 && ijk[1] > 0 && ijk[2] < self->elementSize[2] )
		elements[4] = HEL_E_3DTo1D_3( self, ijk[0] - 1, ijk[1] - 1, ijk[2] );
	else
		elements[4] = self->elementCount;
	
	if( ijk[0] < self->elementSize[0] && ijk[1] > 0 && ijk[2] < self->elementSize[2] )
		elements[5] = HEL_E_3DTo1D_3( self, ijk[0], ijk[1] - 1, ijk[2] );
	else
		elements[5] = self->elementCount;
	
	if( ijk[0] > 0 && ijk[1] < self->elementSize[1] && ijk[2] < self->elementSize[2] )
		elements[6] = HEL_E_3DTo1D_3( self, ijk[0] - 1, ijk[1], ijk[2] );
	else
		elements[6] = self->elementCount;
	
	if( ijk[0] < self->elementSize[0] && ijk[1] < self->elementSize[1] && ijk[2] < self->elementSize[2] )
		elements[7] = HEL_E_3DTo1D_3( self, ijk[0], ijk[1], ijk[2] );
	else
		elements[7] = self->elementCount;
}

void _HexaEL_BuildEdgeIndices( void* hexaEL, Element_GlobalIndex globalIndex, Index* edges ) {
	HexaEL*		self = (HexaEL*)hexaEL;

	self->buildEdgeIndices = (self->dim == 1 ? _HexaEL_BuildEdgeIndices1D : 
				  self->dim == 2 ? _HexaEL_BuildEdgeIndices2D : _HexaEL_BuildEdgeIndices3D );

	ElementLayout_BuildEdgeIndices( self, globalIndex, edges );
}

void _HexaEL_BuildEdgeIndices1D( void* hexaEL, Element_GlobalIndex globalIndex, Index* edges ) {
	abort();
}

void _HexaEL_BuildEdgeIndices2D( void* hexaEL, Element_GlobalIndex globalIndex, Index* edges ) {
	HexaEL*		self = (HexaEL*)hexaEL;
	IJK		ij;
	
	HEL_E_1DTo2D( self, globalIndex, ij );
	
	edges[0] = (self->elementSize[0] + self->pointSize[0]) * ij[1];
	edges[0] += ij[0];
	
	edges[1] = edges[0] + self->elementSize[0];
	
	edges[2] = edges[1] + 1;
	
	edges[3] = edges[2] + self->elementSize[0];
}

void _HexaEL_BuildEdgeIndices3D( void* hexaEL, Element_GlobalIndex globalIndex, Index* edges ) {
	HexaEL*		self = (HexaEL*)hexaEL;
	IJK		ijk;
	
	HEL_E_1DTo3D( self, globalIndex, ijk );
	
	edges[0] = (self->elementSize[0] * self->pointSize[1] + self->elementSize[1] * self->pointSize[0] + 
		    self->pointSize[0] * self->pointSize[1]) * ijk[2];
	edges[0] += (self->elementSize[0] + self->pointSize[0]) * ijk[1];
	edges[0] += ijk[0];
	
	edges[1] = edges[0] + self->elementSize[0];
	
	edges[2] = edges[1] + 1;
	
	edges[3] = edges[2] + self->elementSize[0];
	
	edges[4] = (self->elementSize[0] * self->pointSize[1] + self->elementSize[1] * self->pointSize[0] + 
		    self->pointSize[0] * self->pointSize[1]) * ijk[2] + 
		self->elementSize[0] * self->pointSize[1] + self->elementSize[1] * self->pointSize[0];
	edges[4] += ijk[1] * self->pointSize[0] + ijk[0];
	
	edges[5] = edges[4] + 1;
	
	edges[6] = edges[4] + self->pointSize[0];
	
	edges[7] = edges[6] + 1;

	edges[8] = (self->elementSize[0] * self->pointSize[1] + self->elementSize[1] * self->pointSize[0] + 
		    self->pointSize[0] * self->pointSize[1]) * (ijk[2] + 1);
	edges[8] += (self->elementSize[0] + self->pointSize[0]) * ijk[1];
	edges[8] += ijk[0];
	
	edges[9] = edges[8] + self->elementSize[0];
	
	edges[10] = edges[9] + 1;
	
	edges[11] = edges[10] + self->elementSize[0];
}


Element_GlobalIndex _HexaEL_EdgeElementCount( void* hexaEL, Index edge ) {
	HexaEL*		self = (HexaEL*)hexaEL;

	return (self->dim == 1 ? 0 : self->dim == 2 ? 2 : 4 );
}


void _HexaEL_BuildEdgeElements( void* hexaEL, Index edge, Element_GlobalIndex* elements ) {
	/*HexaEL*		self = (HexaEL*)hexaEL; */
	/* TODO */
	assert( 0 );
}

void _HexaEL_EdgeAt( void* hexaEL, Index index, Edge edge ) {
	HexaEL*		self = (HexaEL*)hexaEL;

	self->edgeAt = (self->dim == 1 ? _HexaEL_EdgeAt1D : 
			self->dim == 2 ? _HexaEL_EdgeAt2D : _HexaEL_EdgeAt3D );
	ElementLayout_EdgeAt( self, index, edge );
}

void _HexaEL_EdgeAt1D( void* hexaEL, Index index, Edge edge ) {
	abort();
}

void _HexaEL_EdgeAt2D( void* hexaEL, Index index, Edge edge ) {
	HexaEL*		self = (HexaEL*)hexaEL;
	
	if( index < self->edgeCount ) {
		IJK     ij;
		Index   base;
		
		base = self->elementSize[0] + self->pointSize[0];
		
		ij[1] = index / base;
		ij[0] = index % base;
		
		if( ij[0] < self->elementSize[0] ) {
			edge[1] = HEL_P_2DTo1D_2( self, ij[0] + 1, ij[1] );
		}
		else {
			ij[0] -= self->elementSize[0];
			edge[1] = HEL_P_2DTo1D_2( self, ij[0], ij[1] + 1 );
		}
		
		edge[0] = HEL_P_2DTo1D( self, ij );
	}
	else {
		edge[0] = self->geometry->pointCount;
		edge[1] = self->geometry->pointCount;
	}
}


void _HexaEL_EdgeAt3D( void* hexaEL, Index index, Edge edge ) {
	HexaEL*		self = (HexaEL*)hexaEL;
	
	if( index < self->edgeCount ) {
		IJK     ijk, base;
		Index   mod;
		
		base[0] = self->elementSize[0] + self->pointSize[0];
		base[1] = base[0] * self->elementSize[1] + self->elementSize[0];
		base[2] = base[1] + self->pointSize[0] * self->pointSize[1];
		
		ijk[2] = index / base[2];
		mod = index % base[2];
		
		if( mod < base[1] ) {
			ijk[1] = mod / base[0];
			ijk[0] = mod % base[0];
			
			if( ijk[0] < self->elementSize[0] ) {
				edge[1] = HEL_P_3DTo1D_3( self, ijk[0] + 1, ijk[1], ijk[2] );
			}
			else {
				ijk[0] -= self->elementSize[0];
				edge[1] = HEL_P_3DTo1D_3( self, ijk[0], ijk[1] + 1, ijk[2] );
			}
		}
		else {
			ijk[1] = (mod - base[1]) / self->pointSize[0];
			ijk[0] = (mod - base[1]) % self->pointSize[0];
			
			edge[1] = HEL_P_3DTo1D_3( self, ijk[0], ijk[1], ijk[2] + 1 );
		}
		
		edge[0] = HEL_P_3DTo1D( self, ijk );
	}
	else {
		edge[0] = self->geometry->pointCount;
		edge[1] = self->geometry->pointCount;
	}
}

Element_DomainIndex _HexaEL_ElementWithPoint( void* hexaEL, void* decomp, Coord point, void* mesh, 
					      PartitionBoundaryStatus boundaryStatus, unsigned nHints, unsigned* hints )
{		
	HexaEL*			self = (HexaEL*)hexaEL;

	self->elementWithPoint = (self->dim == 1 ? _HexaEL_ElementWithPoint1D : 
				  self->dim == 2 ? _HexaEL_ElementWithPoint2D : _HexaEL_ElementWithPoint3D );

	return ElementLayout_ElementWithPoint( self, decomp, point, mesh, boundaryStatus, nHints, hints );
}


Bool _HexaEL_Equiv( double var, double val ) {
	static const double	fac = 1e-13;

	return (var > val - fac && var < val + fac);
}


void _HexaEL_TriBarycenter( Coord tri[3], const Coord pnt, Coord dst ) {
	double	a = tri[0][0] - tri[2][0];
	double	b = tri[1][0] - tri[2][0];
	double	c = tri[2][0] - pnt[0];
	double	d = tri[0][1] - tri[2][1];
	double	e = tri[1][1] - tri[2][1];
	double	f = tri[2][1] - pnt[1];

	dst[0] = (b * f - c * e) / (a * e - b * d);
	if( _HexaEL_Equiv( dst[0], 0.0 ) ) dst[0] = 0.0;
	else if( _HexaEL_Equiv( dst[0], 1.0 ) ) dst[0] = 1.0;

	dst[1] = (a * f - c * d) / (b * d - a * e);
	if( _HexaEL_Equiv( dst[1], 0.0 ) ) dst[1] = 0.0;
	else if( _HexaEL_Equiv( dst[1], 1.0 ) ) dst[1] = 1.0;

	dst[2] = 1.0 - dst[0] - dst[1];
	if( _HexaEL_Equiv( dst[2], 0.0 ) ) dst[2] = 0.0;
	else if( _HexaEL_Equiv( dst[2], 1.0 ) ) dst[2] = 1.0;
}


void _HexaEL_TetBarycenter( Coord tet[4], const Coord pnt, double* dst ) {
	double	x0 = tet[0][0], x1 = tet[1][0], x2 = tet[2][0], x3 = tet[3][0];
	double	y0 = tet[0][1], y1 = tet[1][1], y2 = tet[2][1], y3 = tet[3][1];
	double	z0 = tet[0][2], z1 = tet[1][2], z2 = tet[2][2], z3 = tet[3][2];
	double	px = pnt[0], py = pnt[1], pz = pnt[2];
	double	den = 1.0 / (x1*(y0*(z3 - z2) + y2*(z0 - z3) + y3*(z2 - z0)) + 
			     x0*(y2*(z3 - z1) + y1*(z2 - z3) + y3*(z1 - z2)) + 
			     x2*(y1*(z3 - z0) + y0*(z1 - z3) + y3*(z0 - z1)) + 
			     x3*(y0*(z2 - z1) + y1*(z0 - z2) + y2*(z1 - z0)));

	dst[1] = -(x0*(py*(z3 - z2) + y2*(pz - z3) + y3*(z2 - pz)) + 
		   px*(y2*(z3 - z0) + y0*(z2 - z3) + y3*(z0 - z2)) + 
		   x2*(y0*(z3 - pz) + py*(z0 - z3) + y3*(pz - z0)) + 
		   x3*(py*(z2 - z0) + y0*(pz - z2) + y2*(z0 - pz))) * den;
	if( _HexaEL_Equiv( dst[1], 0.0 ) ) dst[1] = 0.0;
	else if( _HexaEL_Equiv( dst[1], 1.0 ) ) dst[1] = 1.0;

	dst[2] = (x0*(py*(z3 - z1) + y1*(pz - z3) + y3*(z1 - pz)) + 
		  px*(y1*(z3 - z0) + y0*(z1 - z3) + y3*(z0 - z1)) + 
		  x1*(y0*(z3 - pz) + py*(z0 - z3) + y3*(pz - z0)) + 
		  x3*(py*(z1 - z0) + y0*(pz - z1) + y1*(z0 - pz))) * den;
	if( _HexaEL_Equiv( dst[2], 0.0 ) ) dst[2] = 0.0;
	else if( _HexaEL_Equiv( dst[2], 1.0 ) ) dst[2] = 1.0;

	dst[3] = -(x0*(py*(z2 - z1) + y1*(pz - z2) + y2*(z1 - pz)) + 
		   px*(y1*(z2 - z0) + y0*(z1 - z2) + y2*(z0 - z1)) + 
		   x1*(y0*(z2 - pz) + py*(z0 - z2) + y2*(pz - z0)) + 
		   x2*(py*(z1 - z0) + y0*(pz - z1) + y1*(z0 - pz))) * den;
	if( _HexaEL_Equiv( dst[3], 0.0 ) ) dst[3] = 0.0;
	else if( _HexaEL_Equiv( dst[3], 1.0 ) ) dst[3] = 1.0;

	dst[0] = 1.0 - dst[1] - dst[2] - dst[3];
	if( _HexaEL_Equiv( dst[0], 0.0 ) ) dst[0] = 0.0;
	else if( _HexaEL_Equiv( dst[0], 1.0 ) ) dst[0] = 1.0;
}


Bool _HexaEL_FindTriBarycenter( const Coord crds[4], const Coord pnt, double* bcs, unsigned* dstInds, 
				PartitionBoundaryStatus bndStatus, MeshTopology* topo, unsigned gElInd )
{
	const unsigned	nTris = 2;
	const unsigned	inds[2][3] = {{0, 1, 2}, {1, 3, 2}};
	Coord		tri[3];
	unsigned	tri_i;

	for( tri_i = 0; tri_i < nTris; tri_i++ ) {
		unsigned	ind_i;

		/* Copy coordinate. */
		for( ind_i = 0; ind_i < 3; ind_i++ ) {
			dstInds[ind_i] = inds[tri_i][ind_i];
			memcpy( tri[ind_i], crds[inds[tri_i][ind_i]], sizeof(Coord) );
		}

		/* Clac the barycenter. */
		_HexaEL_TriBarycenter( tri, pnt, bcs );

		/* Check for completeness. */
		for( ind_i = 0; ind_i < 3; ind_i++ ) {
			if( bcs[ind_i] < 0.0 || bcs[ind_i] > 1.0 )
				break;
		}
		if( ind_i == 3 ) {
			if( topo && topo->domains && bndStatus == EXCLUSIVE_UPPER_BOUNDARY ) {
				Decomp*		decomp;
				unsigned	telInd;
				unsigned	dElInd = Decomp_Sync_GlobalToDomain( topo->domains[MT_FACE], gElInd );

				assert( dElInd < MeshTopology_GetDomainSize( topo, MT_FACE ) );

				/* Check boundary ownership. */
				if( bcs[0] == 0.0 || bcs[0] == -0.0 ) {
					if( bcs[1] == 0.0 || bcs[1] == -0.0 ) {
						decomp = topo->domains[MT_VERTEX]->decomp;
						telInd = topo->incEls[MT_FACE][MT_VERTEX][dElInd][inds[tri_i][2]];
					}
					else if( bcs[2] == 0.0 || bcs[2] == -0.0 ) {
						decomp = topo->domains[MT_VERTEX]->decomp;
						telInd = topo->incEls[MT_FACE][MT_VERTEX][dElInd][inds[tri_i][1]];
					}
					else {
						decomp = topo->domains[MT_EDGE]->decomp;
						if( tri_i == 0 )
							return True;
						else if( tri_i == 1 )
							telInd = topo->incEls[MT_FACE][MT_EDGE][dElInd][1];
					}
					return telInd < decomp->nLocals;
				}
				else if( bcs[1] == 0.0 || bcs[1] == -0.0 ) {
					if( bcs[2] == 0.0 || bcs[2] == -0.0 ) {
						decomp = topo->domains[MT_VERTEX]->decomp;
						telInd = topo->incEls[MT_FACE][MT_VERTEX][dElInd][inds[tri_i][0]];
					}
					else {
						decomp = topo->domains[MT_EDGE]->decomp;
						if( tri_i == 0 )
							telInd = topo->incEls[MT_FACE][MT_EDGE][dElInd][2];
						else if( tri_i == 1 )
							return True;
					}
					return telInd < decomp->nLocals;
				}
				else if( bcs[2] == 0.0 || bcs[2] == -0.0 ) {
					decomp = topo->domains[MT_EDGE]->decomp;
					if( tri_i == 0 )
						telInd = topo->incEls[MT_FACE][MT_EDGE][dElInd][0];
					else if( tri_i == 1 )
						telInd = topo->incEls[MT_FACE][MT_EDGE][dElInd][3];
					return telInd < decomp->nLocals;
				}
			}

			return True;
		}
	}

	return False;
}


Bool _HexaEL_FindTetBarycenter( const Coord crds[8], const Coord pnt, double* bcs, unsigned* dstInds, 
				PartitionBoundaryStatus bndStatus, MeshTopology* topo, unsigned gElInd )
{
	const unsigned	nTets = 10;
	const unsigned	inds[10][4] = {{0, 1, 2, 4}, 
				       {1, 2, 3, 7}, 
				       {1, 4, 5, 7}, 
				       {2, 4, 6, 7}, 
				       {1, 2, 4, 7}, 
				       {0, 1, 3, 5}, 
				       {0, 4, 5, 6}, 
				       {0, 2, 3, 6}, 
				       {3, 5, 6, 7}, 
				       {0, 3, 5, 6}};
	Coord		tet[4];
	int		tet_i;

	for( tet_i = 0; tet_i < nTets; tet_i++ ) {
		int	ind_i;

		/* Copy coordinates to the correct order. */
		for( ind_i = 0; ind_i < 4; ind_i++ ) {
			dstInds[ind_i] = inds[tet_i][ind_i];
			memcpy( tet[ind_i], crds[inds[tet_i][ind_i]], sizeof(Coord) );
		}

		/* Calc barycenter. */
		_HexaEL_TetBarycenter( tet, pnt, bcs );

		/* Is this the right tetrahedron? */
		for( ind_i = 0; ind_i < 4; ind_i++ ) {
			if( bcs[ind_i] < 0.0 || bcs[ind_i] > 1.0 )
				break;
		}
		if( ind_i == 4 ){
			if( topo && topo->domains && bndStatus == EXCLUSIVE_UPPER_BOUNDARY ) {
				Decomp*		decomp;
				unsigned	telInd;
				unsigned	dElInd = Decomp_Sync_GlobalToDomain( topo->domains[MT_VOLUME], gElInd );

				assert( dElInd < MeshTopology_GetDomainSize( topo, MT_VOLUME ) );

				/* Check boundary ownership. */
				if( bcs[0] == 0.0 || bcs[0] == -0.0 ) {
					if( bcs[1] == 0.0 || bcs[1] == -0.0 ) {
						if( bcs[2] == 0.0 || bcs[2] == -0.0 ) {
							decomp = topo->domains[MT_VERTEX]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_VERTEX][dElInd][inds[tet_i][3]];
						}
						else if( bcs[3] == 0.0 || bcs[3] == -0.0 ) {
							decomp = topo->domains[MT_VERTEX]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_VERTEX][dElInd][inds[tet_i][2]];
						}
						else {
							if( tet_i == 0 ) {
								decomp = topo->domains[MT_FACE]->decomp;
								telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][4];
							}
							else if( tet_i == 1 ) {
								decomp = topo->domains[MT_EDGE]->decomp;
								telInd = topo->incEls[MT_VOLUME][MT_EDGE][dElInd][11];
							}
							else if( tet_i == 2 ) {
								decomp = topo->domains[MT_EDGE]->decomp;
								telInd = topo->incEls[MT_VOLUME][MT_EDGE][dElInd][7];
							}
							else if( tet_i == 3 ) {
								decomp = topo->domains[MT_EDGE]->decomp;
								telInd = topo->incEls[MT_VOLUME][MT_EDGE][dElInd][5];
							}
							else if( tet_i == 4 ) {
								decomp = topo->domains[MT_FACE]->decomp;
								telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][1];
							}
							else if( tet_i == 5 ) {
								decomp = topo->domains[MT_FACE]->decomp;
								telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][5];
							}
							else if( tet_i == 6 ) {
								decomp = topo->domains[MT_FACE]->decomp;
								telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][1];
							}
							else if( tet_i == 7 ) {
								decomp = topo->domains[MT_FACE]->decomp;
								telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][3];
							}
							else if( tet_i == 8 ) {
								decomp = topo->domains[MT_EDGE]->decomp;
								telInd = topo->incEls[MT_VOLUME][MT_EDGE][dElInd][5];
							}
							else {
								decomp = topo->domains[MT_FACE]->decomp;
								telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][1];
							}
						}
					}
					else if( bcs[2] == 0.0 || bcs[2] == -0.0 ) {
						if( bcs[3] == 0.0 || bcs[3] == -0.0 ) {
							decomp = topo->domains[MT_VERTEX]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_VERTEX][dElInd][inds[tet_i][1]];
						}
						else {
							if( tet_i == 0 ) {
								decomp = topo->domains[MT_FACE]->decomp;
								telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][2];
							}
							else if( tet_i == 1 ) {
								decomp = topo->domains[MT_FACE]->decomp;
								telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][3];
							}
							else if( tet_i == 2 ) {
								decomp = topo->domains[MT_FACE]->decomp;
								telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][1];
							}
							else if( tet_i == 3 ) {
								decomp = topo->domains[MT_FACE]->decomp;
								telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][1];
							}
							else if( tet_i == 4 ) {
								decomp = topo->domains[MT_FACE]->decomp;
								telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][3];
							}
							else if( tet_i == 5 ) {
								decomp = topo->domains[MT_EDGE]->decomp;
								telInd = topo->incEls[MT_VOLUME][MT_EDGE][dElInd][9];
							}
							else if( tet_i == 6 ) {
								decomp = topo->domains[MT_EDGE]->decomp;
								telInd = topo->incEls[MT_VOLUME][MT_EDGE][dElInd][6];
							}
							else if( tet_i == 7 ) {
								decomp = topo->domains[MT_EDGE]->decomp;
								telInd = topo->incEls[MT_VOLUME][MT_EDGE][dElInd][10];
							}
							else if( tet_i == 8 ) {
								decomp = topo->domains[MT_EDGE]->decomp;
								telInd = topo->incEls[MT_VOLUME][MT_EDGE][dElInd][7];
							}
							else {
								decomp = topo->domains[MT_FACE]->decomp;
								telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][3];
							}
						}
					}
					else if( bcs[3] == 0.0 || bcs[3] == -0.0 ) {
						if( tet_i == 0 ) {
							decomp = topo->domains[MT_FACE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][0];
						}
						else if( tet_i == 1 ) {
							decomp = topo->domains[MT_EDGE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_EDGE][dElInd][1];
						}
						else if( tet_i == 2 ) {
							decomp = topo->domains[MT_EDGE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_EDGE][dElInd][4];
						}
						else if( tet_i == 3 ) {
							decomp = topo->domains[MT_EDGE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_EDGE][dElInd][6];
						}
						else if( tet_i == 4 ) {
							decomp = topo->domains[MT_FACE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][4];
						}
						else if( tet_i == 5 ) {
							decomp = topo->domains[MT_EDGE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_EDGE][dElInd][3];
						}
						else if( tet_i == 6 ) {
							decomp = topo->domains[MT_EDGE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_EDGE][dElInd][4];
						}
						else if( tet_i == 7 ) {
							decomp = topo->domains[MT_EDGE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_EDGE][dElInd][1];
						}
						else if( tet_i == 8 ) {
							decomp = topo->domains[MT_FACE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][1];
						}
						else {
							decomp = topo->domains[MT_FACE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][5];
						}
					}
					else {
						if( tet_i == 0 )
							return True;
						else if( tet_i == 1 ) {
							decomp = topo->domains[MT_FACE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][3];
						}
						else if( tet_i == 2 ) {
							decomp = topo->domains[MT_FACE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][1];
						}
						else if( tet_i == 3 ) {
							decomp = topo->domains[MT_FACE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][1];
						}
						else if( tet_i == 4 )
							return True;
						else if( tet_i == 5 ) {
							decomp = topo->domains[MT_FACE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][5];
						}
						else if( tet_i == 6 ) {
							decomp = topo->domains[MT_FACE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][1];
						}
						else if( tet_i == 7 ) {
							decomp = topo->domains[MT_FACE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][3];
						}
						else if( tet_i == 8 ) {
							decomp = topo->domains[MT_FACE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][1];
						}
						else
							return True;
					}

					return telInd < decomp->nLocals;
				}
				else if( bcs[1] == 0.0 || bcs[1] == -0.0 ) {
					if( bcs[2] == 0.0 || bcs[2] == -0.0 ) {
						if( bcs[3] == 0.0 || bcs[3] == -0.0 ) {
							decomp = topo->domains[MT_VERTEX]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_VERTEX][dElInd][inds[tet_i][0]];
						}
						else {
							if( tet_i == 0 ) {
								decomp = topo->domains[MT_EDGE]->decomp;
								telInd = topo->incEls[MT_VOLUME][MT_EDGE][dElInd][8];
							}
							else if( tet_i == 1 ) {
								decomp = topo->domains[MT_FACE]->decomp;
								telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][5];
							}
							else if( tet_i == 2 ) {
								decomp = topo->domains[MT_FACE]->decomp;
								telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][5];
							}
							else if( tet_i == 3 ) {
								decomp = topo->domains[MT_FACE]->decomp;
								telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][3];
							}
							else if( tet_i == 4 ) {
								decomp = topo->domains[MT_FACE]->decomp;
								telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][5];
							}
							else if( tet_i == 5 ) {
								decomp = topo->domains[MT_FACE]->decomp;
								telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][2];
							}
							else if( tet_i == 6 ) {
								decomp = topo->domains[MT_FACE]->decomp;
								telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][4];
							}
							else if( tet_i == 7 ) {
								decomp = topo->domains[MT_FACE]->decomp;
								telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][4];
							}
							else if( tet_i == 8 ) {
								decomp = topo->domains[MT_EDGE]->decomp;
								telInd = topo->incEls[MT_VOLUME][MT_EDGE][dElInd][7];
							}
							else {
								decomp = topo->domains[MT_FACE]->decomp;
								telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][4];
							}
						}
					}
					else if( bcs[3] == 0.0 || bcs[3] == -0.0 ) {
						if( tet_i == 0 ) {
							decomp = topo->domains[MT_EDGE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_EDGE][dElInd][2];
						}
						else if( tet_i == 1 ) {
							decomp = topo->domains[MT_EDGE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_EDGE][dElInd][3];
						}
						else if( tet_i == 2 ) {
							decomp = topo->domains[MT_EDGE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_EDGE][dElInd][9];
						}
						else if( tet_i == 3 ) {
							decomp = topo->domains[MT_EDGE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_EDGE][dElInd][10];
						}
						else if( tet_i == 4 ) {
							decomp = topo->domains[MT_FACE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][2];
						}
						else if( tet_i == 5 ) {
							decomp = topo->domains[MT_FACE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][0];
						}
						else if( tet_i == 6 ) {
							decomp = topo->domains[MT_FACE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][2];
						}
						else if( tet_i == 7 ) {
							decomp = topo->domains[MT_FACE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][0];
						}
						else if( tet_i == 8 ) {
							decomp = topo->domains[MT_FACE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][3];
						}
						else {
							decomp = topo->domains[MT_FACE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][2];
						}
					}
					else {
						if( tet_i == 0 ) {
							decomp = topo->domains[MT_FACE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][4];
						}
						else if( tet_i == 1 ) {
							decomp = topo->domains[MT_FACE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][5];
						}
						else if( tet_i == 2 ) {
							decomp = topo->domains[MT_FACE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][5];
						}
						else if( tet_i == 3 ) {
							decomp = topo->domains[MT_FACE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][3];
						}
						else if( tet_i == 4 )
							return True;
						else if( tet_i == 5 )
							return True;
						else if( tet_i == 6 )
							return True;
						else if( tet_i == 7 )
							return True;
						else if( tet_i == 8 ) {
							decomp = topo->domains[MT_FACE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][3];
						}
						else
							return True;
					}

					return telInd < decomp->nLocals;
				}
				else if( bcs[2] == 0.0 || bcs[2] == -0.0 ) {
					if( bcs[3] == 0.0 || bcs[3] == -0.0 ) {
						if( tet_i == 0 ) {
							decomp = topo->domains[MT_EDGE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_EDGE][dElInd][0];
						}
						else if( tet_i == 1 ) {
							decomp = topo->domains[MT_FACE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][0];
						}
						else if( tet_i == 2 ) {
							decomp = topo->domains[MT_FACE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][2];
						}
						else if( tet_i == 3 ) {
							decomp = topo->domains[MT_FACE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][4];
						}
						else if( tet_i == 4 ) {
							decomp = topo->domains[MT_FACE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][0];
						}
						else if( tet_i == 5 ) {
							decomp = topo->domains[MT_EDGE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_EDGE][dElInd][0];
						}
						else if( tet_i == 6 ) {
							decomp = topo->domains[MT_EDGE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_EDGE][dElInd][8];
						}
						else if( tet_i == 7 ) {
							decomp = topo->domains[MT_EDGE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_EDGE][dElInd][2];
						}
						else if( tet_i == 8 ) {
							decomp = topo->domains[MT_FACE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][5];
						}
						else {
							decomp = topo->domains[MT_FACE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][0];
						}
					}
					else {
						if( tet_i == 0 ) {
							decomp = topo->domains[MT_FACE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][2];
						}
						else if( tet_i == 1 )
							return True;
						else if( tet_i == 2 )
							return True;
						else if( tet_i == 3 )
							return True;
						else if( tet_i == 4 )
							return True;
						else if( tet_i == 5 ) {
							decomp = topo->domains[MT_FACE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][2];
						}
						else if( tet_i == 6 ) {
							decomp = topo->domains[MT_FACE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][4];
						}
						else if( tet_i == 7 ) {
							decomp = topo->domains[MT_FACE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][4];
						}
						else if( tet_i == 8 ) {
							decomp = topo->domains[MT_FACE]->decomp;
							telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][5];
						}
						else
							return True;
					}

					return telInd < decomp->nLocals;
				}
				else if( bcs[3] == 0.0 || bcs[3] == -0.0 ) {
					if( tet_i == 0 ) {
						decomp = topo->domains[MT_FACE]->decomp;
						telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][0];
					}
					else if( tet_i == 1 ) {
						decomp = topo->domains[MT_FACE]->decomp;
						telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][0];
					}
					else if( tet_i == 2 ) {
						decomp = topo->domains[MT_FACE]->decomp;
						telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][2];
					}
					else if( tet_i == 3 ) {
						decomp = topo->domains[MT_FACE]->decomp;
						telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][4];
					}
					else if( tet_i == 4 )
						return True;
					else if( tet_i == 5 ) {
						decomp = topo->domains[MT_FACE]->decomp;
						telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][0];
					}
					else if( tet_i == 6 ) {
						decomp = topo->domains[MT_FACE]->decomp;
						telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][2];
					}
					else if( tet_i == 7 ) {
						decomp = topo->domains[MT_FACE]->decomp;
						telInd = topo->incEls[MT_VOLUME][MT_FACE][dElInd][0];
					}
					else if( tet_i == 8 )
						return True;
					else
						return True;

					return telInd < decomp->nLocals;
				}
			}

			return True;
		}
	}

	return False;
}

Element_DomainIndex _HexaEL_ElementWithPoint1D( void* hexaEL, void* _decomp, Coord point, void* _mesh, 
						PartitionBoundaryStatus boundaryStatus, unsigned nHints, unsigned* hints )
{
	abort();
}

Element_DomainIndex _HexaEL_ElementWithPoint2D( void* hexaEL, void* _decomp, Coord point, void* _mesh, 
						PartitionBoundaryStatus boundaryStatus, unsigned nHints, unsigned* hints )
{
	HexaEL*		self = (HexaEL*)hexaEL;
	HexaMD*		decomp = (HexaMD*)_decomp;
	Geometry*       geometry = self->geometry;
	Mesh*		mesh = (Mesh*)_mesh;
	IJK		ij;
	Coord		crds[4];
	unsigned	inds[3];
	Coord		bc;
	unsigned	nEls = nHints ? nHints : decomp->elementDomainCount;
	unsigned	e_i;

	for( e_i = 0; e_i < nEls; e_i++ ) {
		unsigned	elInd = nHints ? hints[e_i] : e_i;

		if( elInd >= decomp->elementDomainCount ) {
			continue;
		}

		/* Convert global point to 2D. */
		if( mesh )
			elInd = Mesh_ElementMapDomainToGlobal( mesh, elInd );
		else
			elInd = decomp->elementMapDomainToGlobal( decomp, elInd );
		HEL_E_1DTo2D( self, elInd, ij );

		/* Collect points. */
		geometry->pointAt( geometry, 
				   HEL_P_2DTo1D_2( self, ij[0], ij[1] ), 
				   crds[0] );
		geometry->pointAt( geometry, 
				   HEL_P_2DTo1D_2( self, ij[0] + 1, ij[1] ), 
				   crds[1] );
		geometry->pointAt( geometry, 
				   HEL_P_2DTo1D_2( self, ij[0], ij[1] + 1 ), 
				   crds[2] );
		geometry->pointAt( geometry, 
				   HEL_P_2DTo1D_2( self, ij[0] + 1, ij[1] + 1 ), 
				   crds[3] );
		if( _HexaEL_FindTriBarycenter( (const Coord*)crds, point, bc, inds, 
					       boundaryStatus, self->topo, elInd ) )
		{
			return decomp->elementMapGlobalToDomain( decomp, elInd );
		}
	}

	return decomp->elementDomainCount;
}


Element_DomainIndex _HexaEL_ElementWithPoint3D( void* hexaEL, void* _decomp, Coord point, void* _mesh, 
						PartitionBoundaryStatus boundaryStatus, unsigned nHints, unsigned* hints )
{
	HexaEL*		self = (HexaEL*)hexaEL;
	HexaMD*		decomp = (HexaMD*)_decomp;
	Geometry*       geometry = self->geometry;
	Mesh*		mesh = (Mesh*)_mesh;
	IJK		ijk;
	Coord		crds[8];
	unsigned	inds[4];
	double		bc[4];
	unsigned	nEls = nHints ? nHints : decomp->elementDomainCount;
	unsigned	e_i;

	for( e_i = 0; e_i < nEls; e_i++ ) {
		unsigned	elInd = nHints ? hints[e_i] : e_i;

		/* Safety check. */
		if( elInd >= decomp->elementDomainCount )
			continue;

		/* Convert global point to 3D. */
		if( mesh )
			elInd = Mesh_ElementMapDomainToGlobal( mesh, elInd );
		else
			elInd = decomp->elementMapDomainToGlobal( decomp, elInd );
		HEL_E_1DTo3D( self, elInd, ijk );

		/* Collect the coordinates. */
		geometry->pointAt( geometry, 
				   HEL_P_3DTo1D_3( self, ijk[0], ijk[1], ijk[2] ), 
				   crds[0] );
		geometry->pointAt( geometry, 
				   HEL_P_3DTo1D_3( self, ijk[0] + 1, ijk[1], ijk[2] ), 
				   crds[1] );
		geometry->pointAt( geometry, 
				   HEL_P_3DTo1D_3( self, ijk[0], ijk[1] + 1, ijk[2] ), 
				   crds[2] );
		geometry->pointAt( geometry, 
				   HEL_P_3DTo1D_3( self, ijk[0] + 1, ijk[1] + 1, ijk[2] ), 
				   crds[3] );
		geometry->pointAt( geometry, 
				   HEL_P_3DTo1D_3( self, ijk[0], ijk[1], ijk[2] + 1 ), 
				   crds[4] );
		geometry->pointAt( geometry, 
				   HEL_P_3DTo1D_3( self, ijk[0] + 1, ijk[1], ijk[2] + 1 ), 
				   crds[5] );
		geometry->pointAt( geometry, 
				   HEL_P_3DTo1D_3( self, ijk[0], ijk[1] + 1, ijk[2] + 1 ), 
				   crds[6] );
		geometry->pointAt( geometry, 
				   HEL_P_3DTo1D_3( self, ijk[0] + 1, ijk[1] + 1, ijk[2] + 1 ), 
				   crds[7] );

		/* Find the barycenter. */
		if( _HexaEL_FindTetBarycenter( (const Coord*)crds, point, bc, inds, 
					       boundaryStatus, self->topo, elInd ) )
		{
			return decomp->elementMapGlobalToDomain( decomp, elInd );
		}
	}

	return decomp->elementDomainCount;
}

#if 0
Element_DomainIndex _HexaEL_ElementWithPoint3D( void* hexaEL, void* decomp, Coord point,
						PartitionBoundaryStatus boundaryStatus, unsigned nHints, unsigned* hints )
{		
	HexaEL*			self = (HexaEL*)hexaEL;
	HexaMD*			hexaDecomp = (HexaMD*)decomp;
	Element_GlobalIndex     e_I;
	Plane			planes[6];
	unsigned		nEls = nHints ? nHints : hexaDecomp->elementDomainCount;
	
	for( e_I = 0; e_I < nEls; e_I++ ) {
		unsigned	elInd = nHints ? hints[e_I] : e_I;
		Index		i;

		if( elInd >= hexaDecomp->elementDomainCount ) continue;
		
		_HexaEL_BuildElementPlanes( self, elInd, planes );
		
		for( i = 0; i < 6; i++ )
			if( Plane_DistanceToPoint( planes[i], point ) > 0.0 )
				break;
		
		if( i == 6 )
			return elInd;
	}
	
	return hexaDecomp->elementDomainCount;
}
#endif


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

void _HexaEL_BuildElementPlanes( void* hexaEL, Element_GlobalIndex globalIndex, Plane_List planes ) {
	HexaEL*		self = (HexaEL*)hexaEL;
	Geometry*   geometry = self->geometry;
	Coord		a, b, c, axisA, axisB;
	IJK		ijk;
	
	HEL_E_1DTo3D( self, globalIndex, ijk );
	
	geometry->pointAt( geometry, HEL_P_3DTo1D_3( self, ijk[0], ijk[1], ijk[2] ), a );
	geometry->pointAt( geometry, HEL_P_3DTo1D_3( self, ijk[0] + 1, ijk[1], ijk[2] ), b );
	geometry->pointAt( geometry, HEL_P_3DTo1D_3( self, ijk[0], ijk[1] + 1, ijk[2] ), c );
	Vector_Sub( axisA, b, a );
	Vector_Sub( axisB, c, a );
	Plane_CalcFromVec( planes[0], axisB, axisA, a );

	geometry->pointAt( geometry, HEL_P_3DTo1D_3( self, ijk[0], ijk[1], ijk[2] + 1 ), b );
	Vector_Sub( axisA, b, a );
	Plane_CalcFromVec( planes[1], axisA, axisB, a );
	
	geometry->pointAt( geometry, HEL_P_3DTo1D_3( self, ijk[0] + 1, ijk[1], ijk[2] ), b );
	Vector_Sub( axisB, b, a );
	Plane_CalcFromVec( planes[2], axisB, axisA, a );
	
	geometry->pointAt( geometry, HEL_P_3DTo1D_3( self, ijk[0] + 1, ijk[1] + 1, ijk[2] + 1 ), a );
	geometry->pointAt( geometry, HEL_P_3DTo1D_3( self, ijk[0], ijk[1] + 1, ijk[2] + 1 ), b );
	geometry->pointAt( geometry, HEL_P_3DTo1D_3( self, ijk[0] + 1, ijk[1], ijk[2] + 1 ), c );
	Vector_Sub( axisA, b, a );
	Vector_Sub( axisB, c, a );
	Plane_CalcFromVec( planes[3], axisA, axisB, a );
	
	geometry->pointAt( geometry, HEL_P_3DTo1D_3( self, ijk[0] + 1, ijk[1] + 1, ijk[2] ), b );
	Vector_Sub( axisA, b, a );
	Plane_CalcFromVec( planes[4], axisB, axisA, a );
	
	geometry->pointAt( geometry, HEL_P_3DTo1D_3( self, ijk[0], ijk[1] + 1, ijk[2] + 1 ), c );
	Vector_Sub( axisB, c, a );
	Plane_CalcFromVec( planes[5], axisA, axisB, a );

#ifndef NDEBUG
	geometry->pointAt( geometry, HEL_P_3DTo1D_3( self, ijk[0] + 1, ijk[1] + 1, ijk[2] ), b );
	assert( Plane_DistanceToPoint( planes[0], b ) > -1e-9 && Plane_DistanceToPoint( planes[0], b ) < 1e-9 );
	geometry->pointAt( geometry, HEL_P_3DTo1D_3( self, ijk[0], ijk[1] + 1, ijk[2] + 1 ), b );
	assert( Plane_DistanceToPoint( planes[1], b ) > -1e-9 && Plane_DistanceToPoint( planes[1], b ) < 1e-9 );
	geometry->pointAt( geometry, HEL_P_3DTo1D_3( self, ijk[0] + 1, ijk[1], ijk[2] + 1 ), b );
	assert( Plane_DistanceToPoint( planes[2], b ) > -1e-9 && Plane_DistanceToPoint( planes[2], b ) < 1e-9 );
	geometry->pointAt( geometry, HEL_P_3DTo1D_3( self, ijk[0], ijk[1], ijk[2] + 1 ), b );
	assert( Plane_DistanceToPoint( planes[3], b ) > -1e-9 && Plane_DistanceToPoint( planes[3], b ) < 1e-9 );
	geometry->pointAt( geometry, HEL_P_3DTo1D_3( self, ijk[0] + 1, ijk[1], ijk[2] ), b );
	assert( Plane_DistanceToPoint( planes[4], b ) > -1e-9 && Plane_DistanceToPoint( planes[4], b ) < 1e-9 );
	geometry->pointAt( geometry, HEL_P_3DTo1D_3( self, ijk[0], ijk[1] + 1, ijk[2] ), b );
	assert( Plane_DistanceToPoint( planes[5], b ) > -1e-9 && Plane_DistanceToPoint( planes[5], b ) < 1e-9 );
#endif
}

void _HexaEL_BuildElementLines( void* hexaEL, Element_GlobalIndex globalIndex, Stg_Line_List lines ) {
	HexaEL*		self = (HexaEL*)hexaEL;
	Geometry*       geometry = self->geometry;
	Coord		a, b;
	IJK		ij;
	
	HEL_E_1DTo2D( self, globalIndex, ij );
	
	geometry->pointAt( geometry, HEL_P_2DTo1D_2( self, ij[0], ij[1] ), a );
	geometry->pointAt( geometry, HEL_P_2DTo1D_2( self, ij[0] + 1, ij[1] ), b );
	Stg_Line_CalcFromPoints( lines[0], a, b );
	
	geometry->pointAt( geometry, HEL_P_2DTo1D_2( self, ij[0], ij[1] + 1 ), b );
	Stg_Line_CalcFromPoints( lines[1], b, a );
	
	geometry->pointAt( geometry, HEL_P_2DTo1D_2( self, ij[0] + 1, ij[1] + 1 ), a );
	geometry->pointAt( geometry, HEL_P_2DTo1D_2( self, ij[0], ij[1] + 1 ), b );
	Stg_Line_CalcFromPoints( lines[2], a, b );
	
	geometry->pointAt( geometry, HEL_P_2DTo1D_2( self, ij[0] + 1, ij[1] ), b );
	Stg_Line_CalcFromPoints( lines[3], b, a );
}

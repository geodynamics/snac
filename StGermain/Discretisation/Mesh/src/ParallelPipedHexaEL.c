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
** $Id: ParallelPipedHexaEL.c 3883 2006-10-26 05:00:23Z KathleenHumble $
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
#include "ParallelPipedHexaEL.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "MeshDecomp.h"
#include "HexaMD.h"


/* Textual name of this class */
const Type ParallelPipedHexaEL_Type = "ParallelPipedHexaEL";
const Name PPHexaEL_IJK26TopologyName = "defaultIJK26Topology";
const Name PPHexaEL_BlockGeometryName = "defaultBlockGeometry";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

ParallelPipedHexaEL* ParallelPipedHexaEL_DefaultNew( Name name )
{
	return (ParallelPipedHexaEL*)_ParallelPipedHexaEL_New(
		sizeof(ParallelPipedHexaEL), 
		ParallelPipedHexaEL_Type, 
		_ParallelPipedHexaEL_Delete, 
		_ParallelPipedHexaEL_Print,
		_ParallelPipedHexaEL_Copy,
		(Stg_Component_DefaultConstructorFunction*)ParallelPipedHexaEL_DefaultNew,
		_ParallelPipedHexaEL_Construct,
		_ParallelPipedHexaEL_Build,
		_ParallelPipedHexaEL_Initialise,
		_ParallelPipedHexaEL_Execute,
		_ParallelPipedHexaEL_Destroy,
		name,
		False,
		_ParallelPipedHexaEL_Build,
		_HexaEL_BuildCornerIndices,
		_HexaEL_CornerElementCount,
		_HexaEL_BuildCornerElements,
		_HexaEL_BuildEdgeIndices,
		_HexaEL_EdgeElementCount,
		_HexaEL_BuildEdgeElements,
		_HexaEL_EdgeAt,
		_ParallelPipedHexaEL_GetStaticMinAndMaxLocalCoords,
		_ParallelPipedHexaEL_GetStaticMinAndMaxGlobalCoords,
		_ParallelPipedHexaEL_ElementWithPoint,
		0,
		NULL );
}

ParallelPipedHexaEL* ParallelPipedHexaEL_New(
		Name						name,
		Dimension_Index             dim,
		Dictionary*					dictionary )
{
	return _ParallelPipedHexaEL_New( 
		sizeof(ParallelPipedHexaEL), 
		ParallelPipedHexaEL_Type, 
		_ParallelPipedHexaEL_Delete, 
		_ParallelPipedHexaEL_Print,
		_ParallelPipedHexaEL_Copy,
		(Stg_Component_DefaultConstructorFunction*)ParallelPipedHexaEL_DefaultNew,
		_ParallelPipedHexaEL_Construct,
		_ParallelPipedHexaEL_Build,
		_ParallelPipedHexaEL_Initialise,
		_ParallelPipedHexaEL_Execute,
		_ParallelPipedHexaEL_Destroy,
		name,
		True,
		_ParallelPipedHexaEL_Build,
		_HexaEL_BuildCornerIndices,
		_HexaEL_CornerElementCount,
		_HexaEL_BuildCornerElements,
		_HexaEL_BuildEdgeIndices,
		_HexaEL_EdgeElementCount,
		_HexaEL_BuildEdgeElements,
		_HexaEL_EdgeAt,
		_ParallelPipedHexaEL_GetStaticMinAndMaxLocalCoords,
		_ParallelPipedHexaEL_GetStaticMinAndMaxGlobalCoords,
		_ParallelPipedHexaEL_ElementWithPoint,
		dim,
		dictionary );
}

void ParallelPipedHexaEL_Init(
		ParallelPipedHexaEL*				self,
		Name						name,
		Dimension_Index             dim,
		Dictionary*					dictionary )
{
	/* General info */
	self->type = ParallelPipedHexaEL_Type;
	self->_sizeOfSelf = sizeof(ParallelPipedHexaEL);
	self->_deleteSelf = False;
	self->dictionary = dictionary;
	
	/* Virtual info */
	self->_delete = _ParallelPipedHexaEL_Delete;
	self->_print = _ParallelPipedHexaEL_Print;
	self->_copy = _ParallelPipedHexaEL_Copy;
	self->_defaultConstructor = (Stg_Component_DefaultConstructorFunction*)ParallelPipedHexaEL_DefaultNew;
	self->_construct = _ParallelPipedHexaEL_Construct;
	self->_build = _ParallelPipedHexaEL_Stg_ComponentBuild;
	self->_initialise = _ParallelPipedHexaEL_Initialise;
	self->_execute = _ParallelPipedHexaEL_Execute;
	self->_destroy = _ParallelPipedHexaEL_Destroy;
	self->build = _ParallelPipedHexaEL_Build;
	self->buildCornerIndices = _HexaEL_BuildCornerIndices;
	self->cornerElementCount = _HexaEL_CornerElementCount;
	self->buildCornerElements = _HexaEL_BuildCornerElements;
	self->buildEdgeIndices = _HexaEL_BuildEdgeIndices;
	self->edgeElementCount = _HexaEL_EdgeElementCount;
	self->buildEdgeElements = _HexaEL_BuildEdgeElements;
	self->edgeAt = _HexaEL_EdgeAt;
	self->getStaticMinAndMaxLocalCoords = _ParallelPipedHexaEL_GetStaticMinAndMaxLocalCoords;
	self->getStaticMinAndMaxGlobalCoords = _ParallelPipedHexaEL_GetStaticMinAndMaxGlobalCoords;
	self->elementWithPoint = _ParallelPipedHexaEL_ElementWithPoint;

	_Stg_Class_Init( (Stg_Class*)self );
	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );
	_Stg_Component_Init( (Stg_Component*)self );

	_ElementLayout_Init(
		(ElementLayout*)self,
		(Geometry*)BlockGeometry_New( PPHexaEL_BlockGeometryName, dictionary ),
		(Topology*)IJK26Topology_New( PPHexaEL_IJK26TopologyName, dictionary ) );
	self->topologyWasCreatedInternally = True;
	self->geometryWasCreatedInternally = True;
	_HexaEL_Init( (HexaEL*)self, dim, NULL, NULL );
	
	/* ParallelPipedHexaEL info */
	_ParallelPipedHexaEL_Init( self );
}

ParallelPipedHexaEL* _ParallelPipedHexaEL_New(
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
		Dictionary*                                             dictionary )
{
	ParallelPipedHexaEL* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(ParallelPipedHexaEL) );
	self = (ParallelPipedHexaEL*)_HexaEL_New(
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
		dim,
		dictionary,
		(initFlag)?(Geometry*)BlockGeometry_New( PPHexaEL_BlockGeometryName, dictionary ):NULL );
	
	/* General info */
	
	/* Virtual info */
	
	/* ParallelPipedHexaEL info */
	if( initFlag ){
		
		self->geometryWasCreatedInternally = True;
		_ParallelPipedHexaEL_Init( self );
	}
	
	return self;
}

void _ParallelPipedHexaEL_Init( ParallelPipedHexaEL* self )
{
	Dimension_Index	  dim_I = 0;
	Index             topSizeInDim = 0;
	double            geomSizeInDim = 0;

	/* General and Virtual info should already be set */
	
	/* Since we are using a block geometry, check that the width's are sane for
	the given dimensionality of the Topology */
	for ( dim_I = 0; dim_I < 3 ; dim_I++ ) {
		topSizeInDim = ((IJK26Topology*)self->topology)->size[dim_I];
		geomSizeInDim = ((BlockGeometry*)self->geometry)->max[dim_I] - ((BlockGeometry*)self->geometry)->min[dim_I]; 

		/* for 2D we don't need to check the 3rd axis, which by convention is K */
		if ( (dim_I == K_AXIS) && (self->dim == 2) ) continue;
		
		if ( (geomSizeInDim < 1e-99) ) {
			Stream*      errorStream = Journal_Register( Error_Type, self->type );
			Journal_Printf( errorStream, "Error - in %s(), for %s \"%s\": number of elements requested "
				"in %c direction using IJK26Topology \"%s\" is %u, but the width of "
				"that dimension in provided BlockGeometry \"%s\" is %.2f - %.2f = 0, "
				"thus the element search & location functions will fail.",
				__func__, self->type, self->name,
				IJKTopology_DimNumToDimLetter[dim_I], self->topology->name, topSizeInDim,
				self->geometry->name, ((BlockGeometry*)self->geometry)->max[dim_I],
				((BlockGeometry*)self->geometry)->min[dim_I] );
			Journal_Firewall( 0, errorStream, "Exiting.\n" );	
		}
	}
	
	/* ParallelPipedHexaEL info */
	self->isConstructed = True;

	/* Initialise everything else to zero, until build phase */
	for ( dim_I = 0; dim_I < 3 ; dim_I++ ) {
		self->minLocalThisPartition[dim_I] = 0;
		self->maxLocalThisPartition[dim_I] = 0;
		self->minDomainThisPartition[dim_I] = 0;
		self->maxDomainThisPartition[dim_I] = 0;
		self->elementLengthEachDim[dim_I] = 0;
	} 
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _ParallelPipedHexaEL_Delete( void* parallelPipedHexaEL ) {
	ParallelPipedHexaEL* self = (ParallelPipedHexaEL*)parallelPipedHexaEL;
	
	/* Only delete our geometry ptr if it's a default one created using the old init method,
		not the XML componentFactory way : otherwise we may double-delete memory */
	if( ( True == self->geometryWasCreatedInternally ) ) {
		Stg_Class_Delete( self->geometry );
		self->geometry = NULL;
	}
	
	/* The parent will check about deleting the topology */
	
	/* Stg_Class_Delete parent */
	_HexaEL_Delete( self );
}


void _ParallelPipedHexaEL_Print( void* parallelPipedHexaEL, Stream* stream ) {
	ParallelPipedHexaEL* self = (ParallelPipedHexaEL*)parallelPipedHexaEL;
	
	/* Set the Journal for printing informations */
	Stream* parallelPipedHexaELStream;
	parallelPipedHexaELStream = Journal_Register( InfoStream_Type, "ParallelPipedHexaELStream" );

	/* Print parent */
	_HexaEL_Print( self, stream );
	
	/* General info */
	Journal_Printf( stream, "ParallelPipedHexaEL (ptr): (%p)\n", self );
	
	/* Virtual info */
	
	/* ParallelPipedHexaEL info */
}


void* _ParallelPipedHexaEL_Copy( void* ppHexaEL, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	ParallelPipedHexaEL*	self = (ParallelPipedHexaEL*)ppHexaEL;
	ParallelPipedHexaEL*	newParallelPipedHexaEL;
	Index			idx_I;
	
	newParallelPipedHexaEL = (ParallelPipedHexaEL*)_HexaEL_Copy( self, dest, deep, nameExt, ptrMap );
	
	for( idx_I = 0; idx_I < 3; idx_I++ ) {
		newParallelPipedHexaEL->minLocalThisPartition[idx_I] = self->minLocalThisPartition[idx_I];
		newParallelPipedHexaEL->maxLocalThisPartition[idx_I] = self->maxLocalThisPartition[idx_I];
		newParallelPipedHexaEL->minDomainThisPartition[idx_I] = self->minDomainThisPartition[idx_I];
		newParallelPipedHexaEL->maxDomainThisPartition[idx_I] = self->maxDomainThisPartition[idx_I];
		newParallelPipedHexaEL->elementLengthEachDim[idx_I] = self->elementLengthEachDim[idx_I];
	}
	newParallelPipedHexaEL->dim = self->dim;
	
	return (void*)newParallelPipedHexaEL;
}

void _ParallelPipedHexaEL_Construct( void* ppHexaEL, Stg_ComponentFactory* cf, void* data ){
	ParallelPipedHexaEL *self = (ParallelPipedHexaEL*)ppHexaEL;
	IJK26Topology *topology = NULL;
	BlockGeometry *geometry = NULL;
	Dimension_Index dim;

	/* Note: we can't just use the HexaEL parent construct since we want to assert that the input geometry
	is a block geometry this time */

	self->dictionary = cf->rootDict;
	
	topology =  Stg_ComponentFactory_ConstructByKey(  cf,  self->name, Topology_Type, IJK26Topology,  True, data  ) ;
	geometry =  Stg_ComponentFactory_ConstructByKey(  cf,  self->name, Geometry_Type, BlockGeometry,  True, data  ) ;

	dim = Dictionary_GetUnsignedInt( self->dictionary, "dim" );
	
	_ElementLayout_Init( (ElementLayout*)self, (Geometry*)geometry, (Topology*)topology );
	_HexaEL_Init( (HexaEL*)self, dim, geometry->size, topology->size );
	_ParallelPipedHexaEL_Init( self );
}
	
void _ParallelPipedHexaEL_Stg_ComponentBuild( void* ppHexaEL, void* data ){
	
}
	
void _ParallelPipedHexaEL_Initialise( void* ppHexaEL, void* data ){
	
}
	
void _ParallelPipedHexaEL_Execute( void* ppHexaEL, void* data ){
	
}
	
void _ParallelPipedHexaEL_Destroy( void* ppHexaEL, void* data ){
	
}

void _ParallelPipedHexaEL_Build( void* parallelPipedHexaEL, void* decomp ) {
	ParallelPipedHexaEL_UpdateGeometryPartitionInfo( parallelPipedHexaEL, decomp );
}


void ParallelPipedHexaEL_UpdateGeometryPartitionInfo( void* parallelPipedHexaEL, void* decomp ) {
	ParallelPipedHexaEL*	self = (ParallelPipedHexaEL*)parallelPipedHexaEL;
	BlockGeometry*		geometry = (BlockGeometry*)self->geometry;
	HexaMD*			hexaDecomp = (HexaMD*) decomp;
	Dimension_Index		dim_I;
	Partition_Index		myRank = hexaDecomp->rank;
	Element_Index*		elementLocal3DCounts = hexaDecomp->elementLocal3DCounts[myRank];

	Journal_Firewall( 
			Stg_Class_IsInstance( hexaDecomp, HexaMD_Type ), 
			Journal_Register( Error_Type, self->type ),
			"Error in func %s: hexaDecomp %s '%s' passed in, is not an instance of type '%s'.\n", 
			__func__, hexaDecomp->type, hexaDecomp->name, HexaMD_Type );

	/* Calculate dimensions of each element */
	for ( dim_I = 0; dim_I < self->dim; dim_I++ ) {
		self->elementLengthEachDim[dim_I] = (geometry->max[dim_I] - geometry->min[dim_I]) / self->elementSize[dim_I];
	}	

	/* calculate bounds of local geometry */
	for ( dim_I = 0; dim_I < self->dim; dim_I++ ) {
		self->minLocalThisPartition[dim_I] = geometry->min[dim_I]
			+ hexaDecomp->_elementOffsets[myRank][dim_I] * self->elementLengthEachDim[dim_I];

		self->maxLocalThisPartition[dim_I] = self->minLocalThisPartition[dim_I]
			+ elementLocal3DCounts[dim_I] * self->elementLengthEachDim[dim_I];
	}	

	/* calculate bounds of domain geometry */
	for ( dim_I = 0; dim_I < self->dim; dim_I++ ) {
		self->minDomainThisPartition[dim_I] = self->minLocalThisPartition[dim_I];
		if ( hexaDecomp->_elementOffsets[myRank][dim_I] != 0) {	
			self->minDomainThisPartition[dim_I] -= hexaDecomp->shadowDepth * self->elementLengthEachDim[dim_I];
		}		
		self->maxDomainThisPartition[dim_I] = self->maxLocalThisPartition[dim_I];
		if ( hexaDecomp->_elementOffsets[myRank][dim_I] + elementLocal3DCounts[dim_I] != hexaDecomp->elementGlobal3DCounts[dim_I] )
		{	
			self->maxDomainThisPartition[dim_I] += hexaDecomp->shadowDepth * self->elementLengthEachDim[dim_I];
		}
	}	
}


Bool _ParallelPipedHexaEL_GetStaticMinAndMaxLocalCoords( void* parallelPipedHexaEL, Coord min, Coord max ) {
	ParallelPipedHexaEL*	self = (ParallelPipedHexaEL*)parallelPipedHexaEL;
	Dimension_Index		dim_I = 0;

	memset( min, 0, sizeof( Coord ) );
	memset( max, 0, sizeof( Coord ) );
	for ( dim_I=0; dim_I < self->dim ; dim_I++ ) {
		min[dim_I] = self->minLocalThisPartition[dim_I];
		max[dim_I] = self->maxLocalThisPartition[dim_I];
	}	

	return True;
}


Bool _ParallelPipedHexaEL_GetStaticMinAndMaxGlobalCoords( void* parallelPipedHexaEL, Coord min, Coord max ) {
	ParallelPipedHexaEL*	self = (ParallelPipedHexaEL*)parallelPipedHexaEL;
	BlockGeometry*		bGeometry = (BlockGeometry*)self->geometry;
	Dimension_Index		dim_I = 0;
	
	memset( min, 0, sizeof( Coord ) );
	memset( max, 0, sizeof( Coord ) );
	for ( dim_I=0; dim_I < self->dim ; dim_I++ ) {
		min[dim_I] = bGeometry->min[dim_I];
		max[dim_I] = bGeometry->max[dim_I];
	}

	return True;
}


Element_DomainIndex _ParallelPipedHexaEL_ElementWithPoint( void* parallelPipedHexaEL, void* decomp, Coord point, void* mesh, 
							   PartitionBoundaryStatus boundaryStatus, unsigned nHints, unsigned* hints )
{
	ParallelPipedHexaEL*	self = (ParallelPipedHexaEL*)parallelPipedHexaEL;
	BlockGeometry*		geometry = (BlockGeometry*)self->geometry;
	HexaMD*			hexaDecomp = (HexaMD*)decomp;
	int			localElementIJK[3] = { 0, 0, 0 }; /* Note: has to be int for shadow calculations */
	Dimension_Index		dim_I;
	Coord			adjPoint;
	Coord			normalisedLocalCoord;
	Coord			normalisedDomainCoord;
	Element_DomainIndex	elementWithPoint;
	Bool			withinLocalGeometry = True;
	Partition_Index		myRank = hexaDecomp->rank;
	Element_Index*		elementLocal3DCounts = hexaDecomp->elementLocal3DCounts[myRank];

	Journal_DFirewall( (hexaDecomp->type == HexaMD_Type ), Mesh_Error, "Error: %s doesn't know how to "
			"handle decomps other than %s type.\n", __func__, HexaMD_Type );
	

	
	/* see if the element is within the global geometry */
	for ( dim_I = 0; dim_I < self->dim; dim_I++ ) {
		if ( ( point[dim_I] < ((BlockGeometry*)self->geometry)->min[dim_I] )
			|| ( point[dim_I] > ((BlockGeometry*)self->geometry)->max[dim_I] ) )
		{
			return hexaDecomp->elementDomainCount;
		}	
	}

	/* see if the element is within the local geometry */
	for ( dim_I = 0; dim_I < self->dim; dim_I++ ) {
		if ( ( point[dim_I] < self->minLocalThisPartition[dim_I] ) 
			|| ( point[dim_I] >= self->maxLocalThisPartition[dim_I] ) )
		{
			/* Testing the upper boundary */
			if ( ( boundaryStatus == INCLUSIVE_UPPER_BOUNDARY ) &&
				( point[dim_I] == self->maxLocalThisPartition[dim_I] ) )
			{ 	
				/* Inclusive upper boundaries: consider upper boundary as owned by last row
				of elements on local processor */
				continue; 
			}	
			else if ( ( self->maxLocalThisPartition[dim_I] == ((BlockGeometry*)self->geometry)->max[dim_I] ) &&
				( point[dim_I] == ((BlockGeometry*)self->geometry)->max[dim_I] ) )
			{
				/* Exception for the last processor: the right hand boundary _is_ owned by
				the last processor */
				continue;
			}
			else {
				withinLocalGeometry = False;
				break;
			}
		}	
	}

	if ( False == withinLocalGeometry ) {
		if ( hexaDecomp->elementShadowCount > 0 ) {
			/* Test if the element is still within the full domain geometry */
			for (; dim_I < self->dim; dim_I++ ) {
				if ( ( point[dim_I] < self->minDomainThisPartition[dim_I] ) ||
					( point[dim_I] >= self->maxDomainThisPartition[dim_I] ))
				{
					if ( ( boundaryStatus == INCLUSIVE_UPPER_BOUNDARY )
						&& ( point[dim_I] == self->maxDomainThisPartition[dim_I] ) )
					{
						/* Allow boundary shadow elements if inclusive upper boundary */
						continue;
					}
					else if ( ( self->maxDomainThisPartition[dim_I] == ((BlockGeometry*)self->geometry)->max[dim_I] ) &&
						( point[dim_I] == ((BlockGeometry*)self->geometry)->max[dim_I] ) )
					{
						/* Exception for the last processor: the right hand boundary _is_ owned by
						the last processor */
						continue;
					}

					/* We now know element is outside domain, so last chance is if its in periodic space */
					if ( ( True == ((IJKTopology*)self->topology)->isPeriodic[dim_I] ) && ( hexaDecomp->shadowDepth > 0 ) )
					{
						/* Tests for if the point is within the periodic shadow elements */
						/* Lower -> upper periodic test */
						if ( ( hexaDecomp->_elementOffsets[myRank][dim_I] == 0 ) &&
							( point[dim_I] >= ((BlockGeometry*)self->geometry)->max[dim_I] -
								(self->elementLengthEachDim[dim_I] * hexaDecomp->shadowDepth ) ) ) 
						{
							continue;
						}
						/* Upper -> lower periodic test */
						else if ( ( (hexaDecomp->_elementOffsets[myRank][dim_I] + hexaDecomp->elementLocal3DCounts[myRank][dim_I]) ==
								hexaDecomp->elementGlobal3DCounts[dim_I] ) )
						{
							if ( point[dim_I] < ((BlockGeometry*)self->geometry)->min[dim_I] +
								(self->elementLengthEachDim[dim_I] * hexaDecomp->shadowDepth ) )
							{
								continue;
							}
							else if ( ( boundaryStatus == INCLUSIVE_UPPER_BOUNDARY )
								&& ( point[dim_I] == ((BlockGeometry*)self->geometry)->min[dim_I] +
									(self->elementLengthEachDim[dim_I] * hexaDecomp->shadowDepth ) ) )
							{
								continue;
							}	
							else {
								return hexaDecomp->elementDomainCount;
							}	
						}	
						else {
							/* Element isn't even within full domain, or periodic part 
							-> return and notify of this */
							return hexaDecomp->elementDomainCount;
						}
					}	
					else {	
						/* Element isn't even within full domain, or periodic part 
						-> return and notify of this */
						return hexaDecomp->elementDomainCount;
					}	
				}
			}
		}
		else {
			return hexaDecomp->elementDomainCount;
		}
	}

	/* So now know point is either within local or shadow space */

	for ( dim_I = 0; dim_I < self->dim; dim_I++ ) {
		adjPoint[dim_I] = point[dim_I];
	}

	/* Check if the point is in periodic shadow space. If so, adjust it to its non-periodic corresponding
	point, so the rest of the algorithm works as usual */
	for ( dim_I = 0; dim_I < self->dim; dim_I++ ) {
		if ( point[dim_I] > self->maxDomainThisPartition[dim_I] ) {	
			adjPoint[dim_I] = self->maxDomainThisPartition[dim_I] + ( geometry->max[dim_I] - point[dim_I] );
		}	
		else if ( point[dim_I] < self->minDomainThisPartition[dim_I] ) {	
			adjPoint[dim_I] = self->minDomainThisPartition[dim_I] - ( point[dim_I] - geometry->min[dim_I] ) ;
		}	
	}	

	/* Now normalise the search point relative to the bottom corner of the local processor's mesh */
	for ( dim_I = 0; dim_I < self->dim; dim_I++ ) {
		normalisedLocalCoord[dim_I] = adjPoint[dim_I] - self->minLocalThisPartition[dim_I];
	}

	/* Calculate 3D tuple of local element number based on dividing points in each dim */
	for ( dim_I = 0; dim_I < self->dim; dim_I++ ) {
		localElementIJK[dim_I] = floor( normalisedLocalCoord[dim_I] / self->elementLengthEachDim[dim_I] ); 
		if ( adjPoint[dim_I] == geometry->max[dim_I] ) {
			/* special case: on the very upper end of the domain, set equal to last element (regardless if 
			shadow or normal) */
			localElementIJK[dim_I]--;
		}
		else if ( (True == withinLocalGeometry) && ( localElementIJK[dim_I] == elementLocal3DCounts[dim_I] ) ) {
			/* This case following is to handle 2 special situations:
			1) When INCLUSIVE_UPPER_BOUNDARY chosen, deals with points that are exactly on the upper end of the
			local space (other than those on max geometry considered above). 
			2) points just below the edge of the local space border sometimes get calculated as above due to
			floating-point rounding in the above calculation. So, we need to manually adjust this. */
			localElementIJK[dim_I]--;
		}	
	}
	
	if ( True == withinLocalGeometry ) {
		/* local elements: just convert element 3D tuple -> 1D index */
		Dimension_3DTo1D( localElementIJK, elementLocal3DCounts, &elementWithPoint );
	}
	else {
		Index			adjustment = 0;
		Element_DomainIJK	domainElementIJK = {0,0,0};
		Index			domainElementWithPoint = 0;
		Coord			normalisationPoint;
		
		/* shadow elements: more work to do */
		/* Now normalise the search point relative to the bottom corner of the local processor's mesh */
		for ( dim_I = 0; dim_I < self->dim; dim_I++ ) {
			normalisationPoint[dim_I] = self->minDomainThisPartition[dim_I];
			/* Test for periodic and renormalise */
			if ( ( True == ((IJKTopology*)self->topology)->isPeriodic[dim_I] ) && ( hexaDecomp->shadowDepth > 0 ) 
				&& ( (hexaDecomp->_elementOffsets[myRank][dim_I] + hexaDecomp->elementLocal3DCounts[myRank][dim_I]) ==
								hexaDecomp->elementGlobal3DCounts[dim_I] ) )
			{	
				normalisationPoint[dim_I] -= hexaDecomp->shadowDepth * self->elementLengthEachDim[dim_I];
			}

			normalisedDomainCoord[dim_I] = adjPoint[dim_I] - normalisationPoint[dim_I];
		}
		/* Find 3D tuple of shadow element */
		for ( dim_I = 0; dim_I < self->dim; dim_I++ ) {
			domainElementIJK[dim_I] = floor( normalisedDomainCoord[dim_I] / self->elementLengthEachDim[dim_I] ); 

			if ( ( True == ((IJKTopology*)self->topology)->isPeriodic[dim_I] ) && ( hexaDecomp->shadowDepth > 0 ) ) 
			{	/* Periodic special cases */	
				if ( ( (hexaDecomp->_elementOffsets[myRank][dim_I] + hexaDecomp->elementLocal3DCounts[myRank][dim_I]) ==
								hexaDecomp->elementGlobal3DCounts[dim_I] ) 
					&& ( point[dim_I] == geometry->min[dim_I] ) ) 
				{
					/* A special case for points on lower boundary of periodic shadow space: the adjPoint system
					 * doesn't really work for them since they get confused with the lower shadow space section,
					 * so we need to manually adjust here. */
					domainElementIJK[dim_I]--;
				}
				else if ( ( hexaDecomp->_elementOffsets[myRank][dim_I] == 0 )
					&& ( point[dim_I] == self->maxDomainThisPartition[dim_I] ) )
				{
					/* When periodicity is enabled, for the lowest processor, points on the max domain don't get
					adjusted by the case below, because of the shadow elements at the "top" influencing the
					domainElementIJK calculation. Adjust here. */
					domainElementIJK[dim_I]--;
				}
			}

			if ( domainElementIJK[dim_I] == hexaDecomp->elementDomain3DCounts[dim_I] ) {
				/* The following is to handle 3 special cases:
				1) When the point is on the upper boundary of the entire domain.
				2) When INCLUSIVE_UPPER_BOUNDARY chosen, deals with points that are exactly on the upper end of the
				domain space.
				3) points just below the edge of the domain space border sometimes get calculated as above due to
				floating-point rounding in the above calculation. So, we need to manually adjust this. */
				domainElementIJK[dim_I]--;
			}	
		}	
		/* Convert 3D tuple -> 1D index */
		Dimension_3DTo1D( domainElementIJK, hexaDecomp->elementDomain3DCounts, &domainElementWithPoint );

		/* Now: calculate how many "holes" for local elements we will subtract */
		if ( self->dim == 2 ) {
			if ( localElementIJK[J_AXIS] < 0 ) {
				adjustment = 0;
			}
			else if ( domainElementIJK[J_AXIS] > localElementIJK[J_AXIS] ) {
				adjustment = hexaDecomp->elementLocalCount;
			}
			else {
				Index adjustRows = localElementIJK[J_AXIS];
				if ( adjPoint[I_AXIS] >= self->maxLocalThisPartition[I_AXIS] ) {
					adjustRows++;
				}
				adjustment = adjustRows * hexaDecomp->elementLocal3DCounts[myRank][I_AXIS];
			}
		}
		else {

			/* First consider the Z plane */
			if ( localElementIJK[K_AXIS] < 0 ) {
				/* If we are below the lowest I-J plane of the local block, no adjustments */
				adjustment = 0;
			}
			else if ( domainElementIJK[K_AXIS] > localElementIJK[K_AXIS] ) {
				/* If we are above the max I-J plane of the local block, adjust by the entire local block */
				adjustment = hexaDecomp->elementLocalCount;
			}
			else {
				Index	adjustZSlices;
				Index	localIJ_SliceCount;

				/* Ok, we are in one of the I-J planes of the local block */

				/* First, adjust for any complete blocks below us */
				adjustZSlices = localElementIJK[K_AXIS];
				localIJ_SliceCount = ( elementLocal3DCounts[I_AXIS] * elementLocal3DCounts[J_AXIS] );
				adjustment = adjustZSlices * localIJ_SliceCount;
				
				/* Ok, now adjust for any local elements within the current slice */
				if ( localElementIJK[J_AXIS] < 0 ) {
					/* We are below the lowest J row: nothing more to adjust */
					adjustment += 0;
				}
				else if ( domainElementIJK[J_AXIS] > localElementIJK[J_AXIS] ) {
					/* We are above the last K row: add another full slice to adjust by */
					adjustment += localIJ_SliceCount;
				}
				else {
					Index adjustRows;

					/* We are somewhere next to a K Row: calculate how many to add on */
					adjustRows = localElementIJK[J_AXIS];

					/* If we're on the RHS of one, need to add one more row */
					if ( adjPoint[I_AXIS] >= self->maxLocalThisPartition[I_AXIS] ) {
						adjustRows++;
					}
					adjustment += adjustRows * elementLocal3DCounts[I_AXIS];
				}
			}	
		}

		/* OK: calculate proper domain index: local index, plus the "raw" domain index, then subtract
		  adjustment for local element positions */
		elementWithPoint = hexaDecomp->elementLocalCount + domainElementWithPoint - adjustment;
	}
	
	return elementWithPoint;
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

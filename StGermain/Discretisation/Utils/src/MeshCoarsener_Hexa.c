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
** $Id: MeshCoarsener_Hexa.c 2453 2004-12-21 04:49:34Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"
#include "Discretisation/Mesh/Mesh.h"

#include "types.h"
#include "MeshCoarsener_Hexa.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


/* Textual name of this class */
const Type MeshCoarsener_Hexa_Type = "MeshCoarsener_Hexa";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

MeshCoarsener_Hexa* MeshCoarsener_Hexa_DefaultNew( Name name )
{
	return _MeshCoarsener_Hexa_New(
		sizeof(MeshCoarsener_Hexa), 
		MeshCoarsener_Hexa_Type, 
		_MeshCoarsener_Hexa_Delete, 
		_MeshCoarsener_Hexa_Print,
		NULL,
		(Stg_Component_DefaultConstructorFunction*)MeshCoarsener_Hexa_DefaultNew,
		_MeshCoarsener_Hexa_Construct,
		_MeshCoarsener_Hexa_Build,
		_MeshCoarsener_Hexa_Initialise,
		_MeshCoarsener_Hexa_Execute,
		_MeshCoarsener_Hexa_Destroy,
		name,
		False,
		NULL );
}

MeshCoarsener_Hexa* MeshCoarsener_Hexa_New_Param(
		Name						name,
		Dictionary*					dictionary )
{
	return _MeshCoarsener_Hexa_New( 
		sizeof(MeshCoarsener_Hexa), 
		MeshCoarsener_Hexa_Type, 
		_MeshCoarsener_Hexa_Delete, 
		_MeshCoarsener_Hexa_Print,
		NULL,
		(Stg_Component_DefaultConstructorFunction*)MeshCoarsener_Hexa_DefaultNew,
		_MeshCoarsener_Hexa_Construct,
		_MeshCoarsener_Hexa_Build,
		_MeshCoarsener_Hexa_Initialise,
		_MeshCoarsener_Hexa_Execute,
		_MeshCoarsener_Hexa_Destroy,
		name,
		True,
		dictionary );
}


void MeshCoarsener_Hexa_Init(
		MeshCoarsener_Hexa*				self,
		Name						name,
		Dictionary*					dictionary )
{
	/* General info */
	self->type = MeshCoarsener_Hexa_Type;
	self->_sizeOfSelf = sizeof(MeshCoarsener_Hexa);
	self->_deleteSelf = False;
	self->dictionary = dictionary;
	
	/* Virtual info */
	self->_delete = _MeshCoarsener_Hexa_Delete;
	self->_print = _MeshCoarsener_Hexa_Print;
	self->_copy = NULL;
	self->_defaultConstructor = (Stg_Component_DefaultConstructorFunction*) MeshCoarsener_Hexa_DefaultNew;
	self->_construct = _MeshCoarsener_Hexa_Construct;
	self->_build = _MeshCoarsener_Hexa_Build;
	self->_initialise = _MeshCoarsener_Hexa_Initialise;
	self->_execute = _MeshCoarsener_Hexa_Execute;
	self->_destroy = _MeshCoarsener_Hexa_Destroy;
	
	_Stg_Class_Init( (Stg_Class*)self );
	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );
	_Stg_Component_Init( (Stg_Component*)self );
	
	/* MeshCoarsener_Hexa info */
	_MeshCoarsener_Hexa_Init( self );
}


MeshCoarsener_Hexa* _MeshCoarsener_Hexa_New(
		SizeT						_sizeOfSelf, 
		Type						type,
		Stg_Class_DeleteFunction*				_delete,
		Stg_Class_PrintFunction*				_print,
		Stg_Class_CopyFunction*				_copy, 
		Stg_Component_DefaultConstructorFunction*	_defaultConstructor,
		Stg_Component_ConstructFunction*			_construct,
		Stg_Component_BuildFunction*		_build,
		Stg_Component_InitialiseFunction*		_initialise,
		Stg_Component_ExecuteFunction*		_execute,
		Stg_Component_DestroyFunction*		_destroy,
		Name							name,
		Bool						initFlag,
		Dictionary*					dictionary )
{
	MeshCoarsener_Hexa* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(MeshCoarsener_Hexa) );
	self = (MeshCoarsener_Hexa*)_Stg_Component_New( _sizeOfSelf, type, _delete, _print, _copy, _defaultConstructor, _construct, _build, 
			_initialise, _execute, _destroy, name, NON_GLOBAL );
	
	/* General info */
	self->dictionary = dictionary;
	
	/* Virtual info */
	
	/* MeshCoarsener_Hexa info */
	if( initFlag ){
		_MeshCoarsener_Hexa_Init( self );
	}
	
	return self;
}

void _MeshCoarsener_Hexa_Init(
		MeshCoarsener_Hexa*				self )
{
	/* General and Virtual info should already be set */
	self->isConstructed = True;
	/* MeshCoarsener_Hexa info */
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _MeshCoarsener_Hexa_Delete( void* meshCoarsener ) {
	MeshCoarsener_Hexa* self = (MeshCoarsener_Hexa*)meshCoarsener;
	
	/* Stg_Class_Delete parent */
	_Stg_Class_Delete( self );
}


void _MeshCoarsener_Hexa_Print( void* meshCoarsener, Stream* stream ) {
	MeshCoarsener_Hexa* self = (MeshCoarsener_Hexa*)meshCoarsener;
	
	/* Set the Journal for printing informations */
	Stream* meshCoarsenerStream;
	meshCoarsenerStream = Journal_Register( InfoStream_Type, "MeshCoarsener_HexaStream" );

	/* Print parent */
	_Stg_Class_Print( self, stream );
	
	/* General info */
	Journal_Printf( stream, "MeshCoarsener_Hexa (ptr): (%p)\n", self );
	
	/* Virtual info */
	
	/* MeshCoarsener_Hexa info */
}

void _MeshCoarsener_Hexa_Construct( void* meshCoarsener, Stg_ComponentFactory *cf, void* data ){
	
}
	
void _MeshCoarsener_Hexa_Build( void* meshCoarsener, void *data ){
	
}
	
void _MeshCoarsener_Hexa_Initialise( void* meshCoarsener, void *data ){
	
}
	
void _MeshCoarsener_Hexa_Execute( void* meshCoarsener, void *data ){
	
}

void _MeshCoarsener_Hexa_Destroy( void* meshCoarsener, void *data ){

}

/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void MeshCoarsener_Hexa_Coarsen( void* meshCoarsener, 
				 void* hexaMesh, 
				 unsigned level, 
				 IndexMap** nodeFineToCoarse, 
				 IndexMap** nodeCoarseToFine )
{
	MeshCoarsener_Hexa*	self = (MeshCoarsener_Hexa*)meshCoarsener;
	Mesh*			mesh = (Mesh*)hexaMesh;
	HexaMD*			decomp = (HexaMD*)mesh->layout->decomp; /* TODO: assumption; need to rectify */
	Partition_Index		proc_I;
	Index			idx_I;
	Element_LocalIJK	eBase;
	unsigned		levelMax[3];
	Element_GlobalIJK	oldElement3DCounts;
	Node_GlobalIJK		nodeGlobal3DCounts;
	IJK26Topology*		elementTopology;
	BlockGeometry*		elementGeometry;
	HexaEL*			elementLayout;
	IJK6Topology*		nodeTopology;
	NodeLayout*		nodeLayout;
	
	/*
	** There are a bunch of requirements for this version, here they are...
	*/
	
	/* Because of potentially deforming geometry we must do this before the build phase */
	assert( !mesh->isBuilt );
	
	/* If the mesh is decomposed, we must make sure each local mesh is the same size */
	for( proc_I = 1; proc_I < decomp->procsInUse; proc_I++ ) {
		for( idx_I = 0; idx_I < 3; idx_I++ ) {
			assert( decomp->nodeLocal3DCounts[proc_I][idx_I] == decomp->nodeLocal3DCounts[0][idx_I] );
		}
	}
	
	/* Ensure the level is below the level maximum (maximum is where the mesh currently is) */
	for( idx_I = 0; idx_I < 3; idx_I++ ) {
		_MeshCoarsener_Hexa_CalcElementBaseAndLevel( self, 
							     decomp->elementLocal3DCounts[0][idx_I], 
							     &levelMax[idx_I], 
							     &eBase[idx_I] );
		assert( level <= levelMax[idx_I] );
	}
	
	/* If we want level 0, just return without altering the mesh */
	if( level == 0 ) {
		return;
	}
	
	
	/*
	** Perform the resize, note that this is a bad way of doing it, will change later
	*/
	
	/* Calculate the new element sizes */
	for( idx_I = 0; idx_I < 3; idx_I++ ) {
		nodeGlobal3DCounts[idx_I] = (Node_LocalIndex)pow( 2.0, (double)(levelMax[idx_I] - level) ) * eBase[idx_I];
		
		if( decomp->partitionedAxis[idx_I] ) {
			nodeGlobal3DCounts[idx_I] *= decomp->partition3DCounts[idx_I];
		}
		
		nodeGlobal3DCounts[idx_I]++;
	}
	
	/* Store old elements sizes for later use */
	oldElement3DCounts[0] = decomp->elementLocal3DCounts[decomp->rank][0];
	oldElement3DCounts[1] = decomp->elementLocal3DCounts[decomp->rank][1];
	oldElement3DCounts[2] = decomp->elementLocal3DCounts[decomp->rank][2];
	
	/* Go in and change what's needed (gotta fix this crap) */
	elementTopology = (IJK26Topology*)mesh->layout->elementLayout->topology;
	elementTopology->size[0] = nodeGlobal3DCounts[0] - 1;
	elementTopology->size[1] = nodeGlobal3DCounts[1] - 1;
	elementTopology->size[2] = nodeGlobal3DCounts[2] - 1;
	
	elementGeometry = (BlockGeometry*)mesh->layout->elementLayout->geometry;
	elementGeometry->size[0] = nodeGlobal3DCounts[0];
	elementGeometry->size[1] = nodeGlobal3DCounts[1];
	elementGeometry->size[2] = nodeGlobal3DCounts[2];
	
	elementLayout = (HexaEL*)mesh->layout->elementLayout;
	elementLayout->pointSize[0] = nodeGlobal3DCounts[0];
	elementLayout->pointSize[1] = nodeGlobal3DCounts[1];
	elementLayout->pointSize[2] = nodeGlobal3DCounts[2];
	elementLayout->elementSize[0] = nodeGlobal3DCounts[0] - 1;
	elementLayout->elementSize[1] = nodeGlobal3DCounts[1] - 1;
	elementLayout->elementSize[2] = nodeGlobal3DCounts[2] - 1;
	elementLayout->elementCount = (nodeGlobal3DCounts[0] - 1) * (nodeGlobal3DCounts[1] - 1) * (nodeGlobal3DCounts[2] - 1);
	
	nodeTopology = (IJK6Topology*)mesh->layout->nodeLayout->topology;
	nodeTopology->size[0] = nodeGlobal3DCounts[0];
	nodeTopology->size[1] = nodeGlobal3DCounts[1];
	nodeTopology->size[2] = nodeGlobal3DCounts[2];
	
	nodeLayout = mesh->layout->nodeLayout;
	nodeLayout->nodeCount = nodeGlobal3DCounts[0] * nodeGlobal3DCounts[1] * nodeGlobal3DCounts[2];

	_HexaMD_Destroy( decomp, NULL );
	HexaMD_Init( decomp, "HexaMD",
		     decomp->dictionary, 
		     decomp->communicator, 
		     (ElementLayout*)elementLayout, 
		     nodeLayout, 
		     decomp->numPartitionedDims );
	
	/* Build maps if necessary, note that the current algorithm is quite inefficient */
	if( nodeFineToCoarse ) {
		IndexMap*		map;
		unsigned		factor[3];
		Index			lNode_I, lNode_J, lNode_K;
		Index			mappedNode_I, mappedNode_J, mappedNode_K;
		
		factor[0] = oldElement3DCounts[0] / decomp->elementLocal3DCounts[decomp->rank][0];
		factor[1] = oldElement3DCounts[1] / decomp->elementLocal3DCounts[decomp->rank][1];
		factor[2] = oldElement3DCounts[2] / decomp->elementLocal3DCounts[decomp->rank][2];
		
		map = IndexMap_New();
		
		mappedNode_K = 0;
		for( lNode_K = 0; lNode_K < oldElement3DCounts[2] + 1; lNode_K++ ) {
			mappedNode_J = 0;
			for( lNode_J = 0; lNode_J < oldElement3DCounts[1] + 1; lNode_J++ ) {
				mappedNode_I = 0;
				for( lNode_I = 0; lNode_I < oldElement3DCounts[0] + 1; lNode_I++ ) {
					Index	nodeIdx = lNode_K * (oldElement3DCounts[0] + 1) * (oldElement3DCounts[1] + 1) +
							lNode_J * (oldElement3DCounts[0] + 1) +
							lNode_I;
					
					if( lNode_K % factor[2] == 0 && lNode_J % factor[1] == 0 && lNode_I % factor[0] == 0 ) {
						Index	mappedNodeIdx = mappedNode_K * (decomp->elementLocal3DCounts[decomp->rank][0] + 1) * (decomp->elementLocal3DCounts[decomp->rank][1] + 1) +
								mappedNode_J * (decomp->elementLocal3DCounts[decomp->rank][0] + 1) +
								mappedNode_I;
						
						IndexMap_Append( map, nodeIdx, mappedNodeIdx );
						
						mappedNode_I++;
					}
					else {
						IndexMap_Append( map, nodeIdx, decomp->nodeGlobalCount );
					}
				}
				
				if( lNode_J % factor[1] == 0 ) {
					mappedNode_J++;
				}
			}
			
			if( lNode_K % factor[2] == 0 ) {
				mappedNode_K++;
			}
		}
		
		*nodeFineToCoarse = map;
	}
	
	if( nodeCoarseToFine ) {
		IndexMap*		map;
		unsigned		factor[3];
		Index			lNode_I, lNode_J, lNode_K;
		Index			mappedNode_I, mappedNode_J, mappedNode_K;
		
		factor[0] = oldElement3DCounts[0] / decomp->elementLocal3DCounts[decomp->rank][0];
		factor[1] = oldElement3DCounts[1] / decomp->elementLocal3DCounts[decomp->rank][1];
		factor[2] = oldElement3DCounts[2] / decomp->elementLocal3DCounts[decomp->rank][2];
		
		map = IndexMap_New();
		
		mappedNode_K = 0;
		for( lNode_K = 0; lNode_K < oldElement3DCounts[2] + 1; lNode_K++ ) {
			mappedNode_J = 0;
			for( lNode_J = 0; lNode_J < oldElement3DCounts[1] + 1; lNode_J++ ) {
				mappedNode_I = 0;
				for( lNode_I = 0; lNode_I < oldElement3DCounts[0] + 1; lNode_I++ ) {
					Index	nodeIdx = lNode_K * (oldElement3DCounts[0] + 1) * (oldElement3DCounts[1] + 1) +
							lNode_J * (oldElement3DCounts[0] + 1) +
							lNode_I;
					
					if( lNode_K % factor[2] == 0 && lNode_J % factor[1] == 0 && lNode_I % factor[0] == 0 ) {
						Index	mappedNodeIdx = mappedNode_K * (decomp->elementLocal3DCounts[decomp->rank][0] + 1) * (decomp->elementLocal3DCounts[decomp->rank][1] + 1) +
								mappedNode_J * (decomp->elementLocal3DCounts[decomp->rank][0] + 1) +
								mappedNode_I;
						
						IndexMap_Append( map, mappedNodeIdx, nodeIdx );
						
						mappedNode_I++;
					}
				}
				
				if( lNode_J % factor[1] == 0 ) {
					mappedNode_J++;
				}
			}
			
			if( lNode_K % factor[2] == 0 ) {
				mappedNode_K++;
			}
		}
		
		*nodeCoarseToFine = map;
	}
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

void _MeshCoarsener_Hexa_CalcElementBaseAndLevel( void* meshCoarsener, Element_Index size, unsigned* level, Element_Index* base ) {
	/*MeshCoarsener_Hexa*	self = (MeshCoarsener_Hexa*)meshCoarsener;*/
	
	/* Calculate the level and the base */
	*level = 0;
	*base = size;
	do {
		if( ((*base) % 2) > 0 ) {
			break;
		}
		else {
			(*base) /= 2;
		}
		
		(*level)++;
	} while( 1 );
}


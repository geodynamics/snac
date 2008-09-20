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
** $Id: MeshDecomp.c 3574 2006-05-15 11:30:33Z PatrickSunter $
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
#include "MeshDecomp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


/* Textual name of this class */
const Type MeshDecomp_Type = "MeshDecomp";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

MeshDecomp* _MeshDecomp_New(
		SizeT                                           _sizeOfSelf, 
		Type                                            type,
		Stg_Class_DeleteFunction*                       _delete,
		Stg_Class_PrintFunction*                        _print,
		Stg_Class_CopyFunction*                         _copy, 
		Stg_Component_DefaultConstructorFunction*       _defaultConstructor,
		Stg_Component_ConstructFunction*                _construct,
		Stg_Component_BuildFunction*                    _build,
		Stg_Component_InitialiseFunction*               _initialise,
		Stg_Component_ExecuteFunction*                  _execute,
		Stg_Component_DestroyFunction*                  _destroy,
		Name                                            name,
		Bool                                            initFlag,
		MeshDecomp_Node_MapLocalToGlobalFunction*       nodeMapLocalToGlobal,
		MeshDecomp_Node_MapDomainToGlobalFunction*      nodeMapDomainToGlobal,
		MeshDecomp_Node_MapShadowToGlobalFunction*      nodeMapShadowToGlobal,
		MeshDecomp_Node_MapGlobalToLocalFunction*       nodeMapGlobalToLocal,
		MeshDecomp_Node_MapGlobalToDomainFunction*      nodeMapGlobalToDomain,
		MeshDecomp_Node_MapGlobalToShadowFunction*      nodeMapGlobalToShadow,
		MeshDecomp_Element_MapLocalToGlobalFunction*	elementMapLocalToGlobal,
		MeshDecomp_Element_MapDomainToGlobalFunction*	elementMapDomainToGlobal,
		MeshDecomp_Element_MapShadowToGlobalFunction*	elementMapShadowToGlobal,
		MeshDecomp_Element_MapGlobalToLocalFunction*	elementMapGlobalToLocal,
		MeshDecomp_Element_MapGlobalToDomainFunction*	elementMapGlobalToDomain,
		MeshDecomp_Element_MapGlobalToShadowFunction*	elementMapGlobalToShadow,
		MeshDecomp_Shadow_ProcCountFunction*            shadowProcCount,
		MeshDecomp_Shadow_BuildProcsFunction*           shadowBuildProcs,
		MeshDecomp_Shadow_ProcElementCountFunction*     shadowProcElementCount,
		MeshDecomp_Proc_WithElementFunction*            procWithElement,
		Dictionary*                                     dictionary,
		MPI_Comm                                        communicator,
		ElementLayout*                                  elementLayout,
		NodeLayout*                                     nodeLayout )
{
	MeshDecomp* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(MeshDecomp) );
	self = (MeshDecomp*)_Stg_Component_New( _sizeOfSelf, type, _delete, _print, _copy, _defaultConstructor, _construct, _build, 
			_initialise, _execute, _destroy, name, NON_GLOBAL );
	
	/* General info */
	self->dictionary = dictionary;
	
	/* Virtual info */
	self->nodeMapLocalToGlobal = nodeMapLocalToGlobal;
	self->nodeMapDomainToGlobal = nodeMapDomainToGlobal;
	self->nodeMapShadowToGlobal = nodeMapShadowToGlobal;
	self->nodeMapGlobalToLocal = nodeMapGlobalToLocal;
	self->nodeMapGlobalToDomain = nodeMapGlobalToDomain;
	self->nodeMapGlobalToShadow = nodeMapGlobalToShadow;
	self->elementMapLocalToGlobal = elementMapLocalToGlobal;
	self->elementMapDomainToGlobal = elementMapDomainToGlobal;
	self->elementMapShadowToGlobal = elementMapShadowToGlobal;
	self->elementMapGlobalToLocal = elementMapGlobalToLocal;
	self->elementMapGlobalToDomain = elementMapGlobalToDomain;
	self->elementMapGlobalToShadow = elementMapGlobalToShadow;
	self->shadowProcCount = shadowProcCount;
	self->shadowBuildProcs = shadowBuildProcs;
	self->shadowProcElementCount = shadowProcElementCount;
	self->procWithElement = procWithElement;

	if( initFlag ){
		_MeshDecomp_Init( self, communicator, elementLayout, nodeLayout, NULL );
	}
	
	return self;
}

void _MeshDecomp_Init(
		MeshDecomp*					self,
		MPI_Comm					communicator,
		ElementLayout*					elementLayout,
		NodeLayout*					nodeLayout,
		Stg_ObjectList*             pointer_Register )
{
	/* General and Virtual info should already be set */
	
	/* MeshDecomp info */
	char*   storageStr;
	
	self->isConstructed = True;
	self->communicator = communicator;
	
	MPI_Comm_rank( self->communicator, (int*)&self->rank );
	MPI_Comm_size( self->communicator, (int*)&self->nproc );
	self->procsInUse = 0;
	
	self->allowUnusedCPUs = Dictionary_GetBool_WithDefault( self->dictionary, "allowUnusedCPUs", False ) ;
	self->allowPartitionOnNode = Dictionary_GetBool_WithDefault( self->dictionary, "allowPartitionOnNode", True );
	self->allowPartitionOnElement = Dictionary_GetBool_WithDefault( self->dictionary, "allowPartitionOnElement", False );

	assert( self->allowPartitionOnNode || self->allowPartitionOnElement );

	self->allowUnbalancing = Dictionary_GetBool_WithDefault( self->dictionary, "allowUnbalancing", False );

	self->procTopology = NULL;
	
	/* Determine IndexSet storage type */
	storageStr = Dictionary_GetString_WithDefault( self->dictionary, "selfStorage", "storeNeighbours" );
	if( !strcmp( storageStr, "storeAll" ) ) {
		self->storage = StoreAll;
	}
	else if( !strcmp( storageStr, "storeSelf" ) ) {
		self->storage = StoreSelf;
	}
	else { /* !strcmp( storageStr, "storeNeighbours" ) */
		self->storage = StoreNeighbours;
	}
	
	self->elementLayout = elementLayout;
	self->nodeLayout = nodeLayout;
	self->localElementSets = NULL;
	self->shadowElementSets = NULL;
	self->localNodeSets = NULL;
	self->shadowNodeSets = NULL;
	
	self->nodeGlobalCount = 0;
	self->nodeLocalCount = 0;
	self->nodeShadowCount = 0;
	self->nodeDomainCount = 0;
	
	self->elementGlobalCount = 0;
	self->elementLocalCount = 0;
	self->elementShadowCount = 0;
	self->elementDomainCount = 0;

	/* TODO - Hack: Registering pointer to node domain count */
	if (pointer_Register) {
		Name nodeDomainCountName;

		Stg_asprintf( &nodeDomainCountName, "%s-nodeDomainCount", self->name );  /* Need to free memory somewhere! */
		Stg_ObjectList_GlobalPointerAppend( pointer_Register, &self->nodeDomainCount, nodeDomainCountName );
	}
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _MeshDecomp_Delete( void* meshDecomp ) {
	MeshDecomp* self = (MeshDecomp*)meshDecomp;
	
	if( self->procTopology ) {
		Stg_Class_Delete( self->procTopology );
		self->procTopology = NULL;
	}
	
	if( self->localElementSets ) {
		Partition_Index proc_I;
		for( proc_I = 0; proc_I < self->procsInUse; proc_I++ )
			if( self->localElementSets[proc_I] )
				Stg_Class_Delete( self->localElementSets[proc_I] );
		Memory_Free( self->localElementSets );
		self->localElementSets = NULL;
	}
	
	if( self->shadowElementSets ) {
		Partition_Index proc_I;	
		for( proc_I = 0; proc_I < self->procsInUse; proc_I++ )
			if( self->shadowElementSets[proc_I] )
				Stg_Class_Delete( self->shadowElementSets[proc_I] );
		Memory_Free( self->shadowElementSets );
		self->shadowElementSets = NULL;
	}
	
	if( self->localNodeSets ) {
		Partition_Index proc_I;
		for( proc_I = 0; proc_I < self->procsInUse; proc_I++ )
			if( self->localNodeSets[proc_I] )
				Stg_Class_Delete( self->localNodeSets[proc_I] );
		Memory_Free( self->localNodeSets );
		self->localNodeSets = NULL;
	}
	
	if( self->shadowNodeSets ) {
		Partition_Index proc_I;
		for( proc_I = 0; proc_I < self->procsInUse; proc_I++ )
			if( self->shadowNodeSets[proc_I] )
				Stg_Class_Delete( self->shadowNodeSets[proc_I] );
		Memory_Free( self->shadowNodeSets );
		self->shadowNodeSets = NULL;
	}
	
	/* Stg_Class_Delete parent */
	_Stg_Component_Delete( self );
}


void _MeshDecomp_Print( void* meshDecomp, Stream* stream ) {
	MeshDecomp* self = (MeshDecomp*)meshDecomp;
	
	/* Set the Journal for printing informations */
	Stream* meshDecompStream;
	meshDecompStream = Journal_Register( InfoStream_Type, "MeshDecompStream" );

	/* Print parent */
	_Stg_Component_Print( self, stream );
	
	/* General info */
	Journal_Printf( stream, "MeshDecomp (ptr): (%p)\n", self );
	
	/* Virtual info */
	
	/* MeshDecomp info */
}


void* _MeshDecomp_Copy( void* meshDecomp, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	MeshDecomp*	self = (MeshDecomp*)meshDecomp;
	MeshDecomp*	newMeshDecomp;
	PtrMap*		map = ptrMap;
	Bool		ownMap = False;
	
	if( !map ) {
		map = PtrMap_New( 10 );
		ownMap = True;
	}
	
	newMeshDecomp = (MeshDecomp*)_Stg_Component_Copy( self, dest, deep, nameExt, map );
	
	/* Virtual methods */
	newMeshDecomp->nodeMapLocalToGlobal = self->nodeMapLocalToGlobal;
	newMeshDecomp->nodeMapDomainToGlobal = self->nodeMapDomainToGlobal;
	newMeshDecomp->nodeMapShadowToGlobal = self->nodeMapShadowToGlobal;
	newMeshDecomp->nodeMapGlobalToLocal = self->nodeMapGlobalToLocal;
	newMeshDecomp->nodeMapGlobalToDomain = self->nodeMapGlobalToDomain;
	newMeshDecomp->nodeMapGlobalToShadow = self->nodeMapGlobalToShadow;
	newMeshDecomp->elementMapLocalToGlobal = self->elementMapLocalToGlobal;
	newMeshDecomp->elementMapDomainToGlobal = self->elementMapDomainToGlobal;
	newMeshDecomp->elementMapShadowToGlobal = self->elementMapShadowToGlobal;
	newMeshDecomp->elementMapGlobalToLocal = self->elementMapGlobalToLocal;
	newMeshDecomp->elementMapGlobalToDomain = self->elementMapGlobalToDomain;
	newMeshDecomp->elementMapGlobalToShadow = self->elementMapGlobalToShadow;
	newMeshDecomp->shadowProcCount = self->shadowProcCount;
	newMeshDecomp->shadowBuildProcs = self->shadowBuildProcs;
	newMeshDecomp->shadowProcElementCount = self->shadowProcElementCount;
	newMeshDecomp->procWithElement = self->procWithElement;
	
	newMeshDecomp->dictionary = self->dictionary;
	newMeshDecomp->communicator = self->communicator;
	newMeshDecomp->rank = self->rank;
	newMeshDecomp->nproc = self->nproc;
	newMeshDecomp->procsInUse = self->procsInUse;
	newMeshDecomp->allowUnusedCPUs = self->allowUnusedCPUs;
	newMeshDecomp->allowPartitionOnNode = self->allowPartitionOnNode;
	newMeshDecomp->allowPartitionOnElement = self->allowPartitionOnElement;
	newMeshDecomp->allowUnbalancing = self->allowUnbalancing;
	newMeshDecomp->storage = self->storage;
	newMeshDecomp->shadowDepth = self->shadowDepth;
	newMeshDecomp->nodeGlobalCount = self->nodeGlobalCount;
	newMeshDecomp->nodeLocalCount = self->nodeLocalCount;
	newMeshDecomp->nodeShadowCount = self->nodeShadowCount;
	newMeshDecomp->nodeDomainCount = self->nodeDomainCount;
	newMeshDecomp->elementGlobalCount = self->elementGlobalCount;
	newMeshDecomp->elementLocalCount = self->elementLocalCount;
	newMeshDecomp->elementShadowCount = self->elementShadowCount;
	newMeshDecomp->elementDomainCount = self->elementDomainCount;
	
	if( deep ) {
		newMeshDecomp->procTopology = (Topology*)Stg_Class_Copy( self->procTopology, NULL, deep, nameExt, map );
		newMeshDecomp->elementLayout = (ElementLayout*)Stg_Class_Copy( self->elementLayout, NULL, deep, nameExt, map );
		newMeshDecomp->nodeLayout = (NodeLayout*)Stg_Class_Copy( self->nodeLayout, NULL, deep, nameExt, map );
		
		if( (newMeshDecomp->localNodeSets = PtrMap_Find( map, self->localNodeSets )) == NULL && self->localNodeSets ) {
			Partition_Index		proc_I;
			
			newMeshDecomp->localNodeSets = Memory_Alloc_Array( IndexSet*, newMeshDecomp->procsInUse, "MeshDecomp->localNodeSets" );
			for( proc_I = 0; proc_I < newMeshDecomp->procsInUse; proc_I++ ) {
				newMeshDecomp->localNodeSets[proc_I] = (IndexSet*)Stg_Class_Copy( self->localNodeSets[proc_I], NULL, deep, nameExt, map );
			}
		}
		
		if( (newMeshDecomp->shadowNodeSets = PtrMap_Find( map, self->shadowNodeSets )) == NULL && self->shadowNodeSets ) {
			Partition_Index		proc_I;
			
			newMeshDecomp->shadowNodeSets = Memory_Alloc_Array( IndexSet*, newMeshDecomp->procsInUse, "MeshDecomp->shadowNodeSets" );
			for( proc_I = 0; proc_I < newMeshDecomp->procsInUse; proc_I++ ) {
				newMeshDecomp->shadowNodeSets[proc_I] = (IndexSet*)Stg_Class_Copy( self->shadowNodeSets[proc_I], NULL, deep, nameExt, map );
			}
		}
		
		if( (newMeshDecomp->localElementSets = PtrMap_Find( map, self->localElementSets )) == NULL && self->localElementSets ) {
			Partition_Index		proc_I;
			
			newMeshDecomp->localElementSets = Memory_Alloc_Array( IndexSet*, newMeshDecomp->procsInUse, "MeshDecomp->localElementSets" );
			for( proc_I = 0; proc_I < newMeshDecomp->procsInUse; proc_I++ ) {
				newMeshDecomp->localElementSets[proc_I] = (IndexSet*)Stg_Class_Copy( self->localElementSets[proc_I], NULL, deep, nameExt, map );
			}
		}
		
		if( (newMeshDecomp->shadowElementSets = PtrMap_Find( map, self->shadowElementSets )) == NULL && self->shadowElementSets ) {
			Partition_Index		proc_I;
			
			newMeshDecomp->shadowElementSets = Memory_Alloc_Array( IndexSet*, newMeshDecomp->procsInUse, "MeshDecomp->shadowElementSets" );
			for( proc_I = 0; proc_I < newMeshDecomp->procsInUse; proc_I++ ) {
				newMeshDecomp->shadowElementSets[proc_I] = (IndexSet*)Stg_Class_Copy( self->shadowElementSets[proc_I], NULL, deep, nameExt, map );
			}
		}
	}
	else {
		newMeshDecomp->procTopology = self->procTopology;
		newMeshDecomp->elementLayout = self->elementLayout;
		newMeshDecomp->nodeLayout = self->nodeLayout;
		newMeshDecomp->localNodeSets = self->localNodeSets;
		newMeshDecomp->shadowNodeSets = self->shadowNodeSets;
		newMeshDecomp->localElementSets = self->localElementSets;
		newMeshDecomp->shadowElementSets = self->shadowElementSets;
	}
	
	if( ownMap ) {
		Stg_Class_Delete( map );
	}
	
	return (void*)newMeshDecomp;
}


/* Shadow virtual functions */

Element_ShadowIndex _MeshDecomp_Shadow_ProcElementCount(void* meshDecomp, Partition_Index rank) {
	assert( 0 );
	return 0;
}


/* Processor virtual functions */
Partition_Index _MeshDecomp_Proc_WithElement( void* meshDecomp, Element_GlobalIndex globalIndex ) {
	MeshDecomp*     self = (MeshDecomp*)meshDecomp;
	Partition_Index proc_I;
	
	for( proc_I = 0; proc_I < self->procsInUse; proc_I++ ) {
		if( self->localElementSets[proc_I] && IndexSet_IsMember( self->localElementSets[proc_I], globalIndex ) )
			return proc_I;
	}
	
	return self->nproc;
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/


Node_GlobalIndex* MeshDecomp_BuildNodeLocalToGlobalMap( void* meshDecomp ) {
	MeshDecomp*	self = (MeshDecomp*)meshDecomp;
	Node_LocalIndex lNodeCnt;
	Node_GlobalIndex* nodeL2G;
	IndexSet* myLocalNodeSet;
	
	if ( self->rank >= self->procsInUse ) return NULL;

	myLocalNodeSet = self->localNodeSets[self->rank];
	
	lNodeCnt = IndexSet_UpdateMembersCount( myLocalNodeSet );
	nodeL2G = Memory_Alloc_Array( Node_GlobalIndex, lNodeCnt, "nodeL2G" );
	IndexSet_GetMembers2( myLocalNodeSet, nodeL2G );
	return nodeL2G;
}


Node_GlobalIndex* MeshDecomp_BuildNodeDomainToGlobalMap( void* meshDecomp ) {
	MeshDecomp*	self = (MeshDecomp*)meshDecomp;
	Node_LocalIndex lNodeCnt;
	Node_GlobalIndex* nodeD2G;
	Node_ShadowIndex sNodeCnt;
	Node_GlobalIndex* tempNodeS2G;
	IndexSet* myLocalNodeSet;
	
	if ( self->rank >= self->procsInUse ) return NULL;

	myLocalNodeSet = self->localNodeSets[self->rank];
	lNodeCnt = IndexSet_UpdateMembersCount( myLocalNodeSet );
	nodeD2G = Memory_Alloc_Array( Node_GlobalIndex, lNodeCnt, "nodeD2G" );
	IndexSet_GetMembers2( myLocalNodeSet, nodeD2G );

	if ( self->nodeShadowCount > 0 ) {
		IndexSet* myShadowNodeSet = self->shadowNodeSets[self->rank];
		IndexSet_GetMembers( myShadowNodeSet, &sNodeCnt, &tempNodeS2G );
		/* combine the two */
		nodeD2G = Memory_Realloc_Array( nodeD2G, Node_GlobalIndex, lNodeCnt + sNodeCnt );
		memcpy( &nodeD2G[lNodeCnt], tempNodeS2G, sNodeCnt * sizeof(Node_GlobalIndex) );
		Memory_Free( tempNodeS2G );
	}

	return nodeD2G;
}


Node_LocalIndex* MeshDecomp_BuildNodeGlobalToLocalMap( void* meshDecomp ) {
	MeshDecomp*		self = (MeshDecomp*)meshDecomp;
	Node_GlobalIndex	gNode_I = 0;
	Node_LocalIndex		lNode_I = 0;
	Node_LocalIndex* nodeG2L;
	
	if ( self->rank >= self->procsInUse ) return NULL;

	nodeG2L = Memory_Alloc_Array( Node_LocalIndex, self->nodeGlobalCount, "nodeG2L" );
	
	for( gNode_I = 0; gNode_I < self->nodeGlobalCount; gNode_I++ ) {
		if ( IndexSet_IsMember( self->localNodeSets[self->rank], gNode_I ) ) {
			nodeG2L[gNode_I] = lNode_I++;
		}	
		else {
			nodeG2L[gNode_I] = MD_N_Invalid( self );
		}	
	}	

	return nodeG2L;
}


Node_DomainIndex* MeshDecomp_BuildNodeGlobalToDomainMap( void* meshDecomp ) {
	MeshDecomp*		self = (MeshDecomp*)meshDecomp;
	Node_GlobalIndex	gNode_I = 0;
	Node_DomainIndex	lNode_I = 0;
	Node_DomainIndex	sNode_I = self->nodeLocalCount;
	IndexSet*			localSet  = NULL;
	IndexSet*			shadowSet = NULL;
	Node_DomainIndex*	nodeG2D;
	
	if ( self->rank >= self->procsInUse ) return NULL;

	nodeG2D = Memory_Alloc_Array( Node_DomainIndex, self->nodeGlobalCount, "nodeG2D" );
	
	localSet = self->localNodeSets[self->rank];
	if ( 0 != self->nodeShadowCount ) {
		shadowSet = self->shadowNodeSets[self->rank];
	}
	
	for( gNode_I = 0; gNode_I < self->nodeGlobalCount; gNode_I++ ) {
		if ( IndexSet_IsMember( localSet, gNode_I ) ) {
			nodeG2D[gNode_I] = lNode_I++;
		}	
		else if ( (0 != self->nodeShadowCount) && IndexSet_IsMember( shadowSet, gNode_I ) ) {
			nodeG2D[gNode_I] = sNode_I++;
		}
		else {
			nodeG2D[gNode_I] = MD_N_Invalid( self );
		}	
	}	

	return nodeG2D;
}


Element_GlobalIndex* MeshDecomp_BuildElementLocalToGlobalMap( void* meshDecomp ) {
	MeshDecomp*		self = (MeshDecomp*)meshDecomp;
	Element_LocalIndex		lElementCnt;
	Element_GlobalIndex*	elementL2G;
	IndexSet* myLocalElementSet;
	
	if ( self->rank >= self->procsInUse ) return NULL;
	myLocalElementSet = self->localElementSets[self->rank];
	
	lElementCnt = IndexSet_UpdateMembersCount( myLocalElementSet );
	elementL2G = Memory_Alloc_Array( Element_GlobalIndex, lElementCnt, "elementL2G" );
	IndexSet_GetMembers2( myLocalElementSet, elementL2G );
	return elementL2G;
}	


Element_GlobalIndex* MeshDecomp_BuildElementDomainToGlobalMap( void* meshDecomp ) {
	MeshDecomp*	self = (MeshDecomp*)meshDecomp;
	Element_LocalIndex lElementCnt;
	Element_ShadowIndex sElementCnt;
	Element_DomainIndex* elementD2G;
	Element_DomainIndex* tempElementS2G;
	IndexSet* myLocalElementSet = self->localElementSets[self->rank];
	
	if ( self->rank >= self->procsInUse ) return NULL;
	myLocalElementSet = self->localElementSets[self->rank];

	lElementCnt = IndexSet_UpdateMembersCount( myLocalElementSet );
	elementD2G = Memory_Alloc_Array( Element_GlobalIndex, lElementCnt, "elementD2G" );
	IndexSet_GetMembers2( myLocalElementSet, elementD2G );

	if ( self->elementShadowCount > 0 ) {
		IndexSet_GetMembers( self->shadowElementSets[self->rank], &sElementCnt, &tempElementS2G );
		/* combine the two */
		elementD2G = Memory_Realloc_Array( elementD2G, Element_GlobalIndex, lElementCnt + sElementCnt );
		memcpy( &elementD2G[lElementCnt], tempElementS2G, sElementCnt * sizeof(Element_GlobalIndex) );
		Memory_Free( tempElementS2G );
	}

	return elementD2G;
}


Element_LocalIndex* MeshDecomp_BuildElementGlobalToLocalMap( void* meshDecomp ) {
	MeshDecomp*		self = (MeshDecomp*)meshDecomp;
	Element_GlobalIndex	gEl_I = 0;
	Element_LocalIndex	lEl_I = 0;
	Element_LocalIndex* elementG2L;
	
	if ( self->rank >= self->procsInUse ) return NULL;

	elementG2L = Memory_Alloc_Array( Element_LocalIndex, self->elementGlobalCount, "elementG2L" );
		
	for( gEl_I = 0; gEl_I < self->elementGlobalCount; gEl_I++ ) {
		if( IndexSet_IsMember( self->localElementSets[self->rank], gEl_I ) ) {
			elementG2L[gEl_I] = lEl_I++;
		}
		else {
			elementG2L[gEl_I] = MD_E_Invalid( self );
		}	
	}	

	return elementG2L;
}


Element_DomainIndex* MeshDecomp_BuildElementGlobalToDomainMap( void* meshDecomp ) {
	MeshDecomp*		self = (MeshDecomp*)meshDecomp;
	Element_GlobalIndex	gElement_I = 0;
	Element_LocalIndex		lElement_I = 0;
	Element_LocalIndex		sElement_I = self->elementLocalCount;
	IndexSet*			localSet  = NULL;
	IndexSet*			shadowSet = NULL;
	Element_DomainIndex* elementG2D;
	
	if ( self->rank >= self->procsInUse ) return NULL;

	elementG2D = Memory_Alloc_Array( Element_DomainIndex, self->elementGlobalCount, "elementG2D" );
	
	localSet = self->localElementSets[self->rank];
	if ( 0 != self->elementShadowCount ) {
		sElement_I = self->elementLocalCount;
		shadowSet = self->shadowElementSets[self->rank];
	}

	for( gElement_I = 0; gElement_I < self->elementGlobalCount; gElement_I++ ) {
		if ( IndexSet_IsMember( localSet, gElement_I ) ) {
			elementG2D[gElement_I] = lElement_I++;
		}	
		else if ( ( 0 != self->elementShadowCount ) && ( IndexSet_IsMember( shadowSet, gElement_I ) ) ) {
			elementG2D[gElement_I] = sElement_I++;
		}
		else {
			elementG2D[gElement_I] = MD_E_Invalid( self );
		}	
	}	

	return elementG2D;
}


/* *** On-the-fly conversion functions *** */

Node_GlobalIndex _MeshDecomp_Node_LocalToGlobal1D( void* meshDecomp, Node_LocalIndex localIndex ) {
	MeshDecomp* self = (MeshDecomp*)meshDecomp;
	return IndexSet_GetIndexOfNthMember( self->localNodeSets[self->rank], localIndex );
}


Node_GlobalIndex _MeshDecomp_Node_ShadowToGlobal1D( void* meshDecomp, Node_ShadowIndex shadowIndex ) {
	MeshDecomp* self = (MeshDecomp*)meshDecomp;
	if (self->nodeShadowCount == 0)
		return MD_N_Invalid(self);
	else
		return IndexSet_GetIndexOfNthMember( self->shadowNodeSets[self->rank], shadowIndex );
}


Node_GlobalIndex _MeshDecomp_Node_DomainToGlobal1D( void* meshDecomp, Node_DomainIndex domainIndex ) {
	MeshDecomp* self = (MeshDecomp*)meshDecomp;
	if ( domainIndex < self->nodeLocalCount ) {
		return self->nodeMapLocalToGlobal( self, domainIndex );
	}
	else if ( domainIndex < self->nodeDomainCount ) {
		return  self->nodeMapShadowToGlobal( self, domainIndex - self->nodeLocalCount );
	}
	else return MD_N_Invalid( self );
}


Node_LocalIndex _MeshDecomp_Node_GlobalToLocal1D( void* meshDecomp, Node_GlobalIndex globalIndex ) {
	MeshDecomp*		self = (MeshDecomp*)meshDecomp;
	
	if (!IndexSet_IsMember( self->localNodeSets[self->rank], globalIndex ) ) {
		return MD_N_Invalid( self );
	}
	else {
		int		lNode_I = 0;
		IndexSet_Index	index;
		
		/* since we now know this global index corresponds to a local node, just 
		find out how many other local nodes there are before it.
		
		THIS IS THE SLOWEST FUNCTION IN THE WORLD YET DISCOVERED 
		 */
		for( index = 0; index < globalIndex; index++ ) {
			if( IndexSet_IsMember( self->localNodeSets[self->rank], index ) ) {
				lNode_I++;
			}
		}	
		
		return lNode_I;
	}
}


Node_ShadowIndex _MeshDecomp_Node_GlobalToShadow1D( void* meshDecomp, Node_GlobalIndex globalIndex ) {
	MeshDecomp* self = (MeshDecomp*)meshDecomp;
	
	if ( (self->nodeShadowCount == 0) || !IndexSet_IsMember( self->shadowNodeSets[self->rank], globalIndex ) ) 
	{	
		return MD_N_Invalid( self );
	}	
	else {
		IndexSet_Index	index;
		Node_ShadowIndex	sNode_I = 0;
		
		/* since we now know this global index corresponds to a shadow node, just 
		find out how many other shadow nodes there are before it. */
		for( index = 0; index < globalIndex; index++ ) {
			if( IndexSet_IsMember( self->shadowNodeSets[self->rank], index ) ) {
				sNode_I++;
			}
		}	
		
		return sNode_I;
	}
	
	return MD_N_Invalid( self );
}


Node_DomainIndex _MeshDecomp_Node_GlobalToDomain1D( void* meshDecomp, Node_GlobalIndex globalIndex ) {
	MeshDecomp* self = (MeshDecomp*)meshDecomp;
	Node_LocalIndex lNode_I;
	Node_ShadowIndex sNode_I;

	if ( (lNode_I = self->nodeMapGlobalToLocal( self, globalIndex)) != MD_N_Invalid(self) ) {
		return lNode_I;
	}
	else if ( (sNode_I = self->nodeMapGlobalToShadow( self, globalIndex)) != MD_N_Invalid(self) ) {
		return self->nodeLocalCount + sNode_I;
	}
	
	return MD_N_Invalid( self );
}


Element_GlobalIndex _MeshDecomp_Element_LocalToGlobal1D( void* meshDecomp, Element_LocalIndex localIndex ) {
	MeshDecomp* self = (MeshDecomp*)meshDecomp;
	return IndexSet_GetIndexOfNthMember( self->localElementSets[self->rank], localIndex );
}


Element_GlobalIndex _MeshDecomp_Element_ShadowToGlobal1D( void* meshDecomp, Element_ShadowIndex shadowIndex ) {
	MeshDecomp* self = (MeshDecomp*)meshDecomp;
	if (self->elementShadowCount == 0)
		return MD_E_Invalid(self);
	else
		return IndexSet_GetIndexOfNthMember( self->shadowElementSets[self->rank], shadowIndex );
}


Element_GlobalIndex _MeshDecomp_Element_DomainToGlobal1D( void* meshDecomp, Element_DomainIndex domainIndex ) {
	MeshDecomp* self = (MeshDecomp*)meshDecomp;
	if ( domainIndex < self->elementLocalCount ) {
		return self->elementMapLocalToGlobal( self, domainIndex );
	}
	else if ( domainIndex < self->elementDomainCount ) {
		return  self->elementMapShadowToGlobal( self, domainIndex - self->elementLocalCount );
	}
	else return MD_E_Invalid( self );
}


Element_LocalIndex _MeshDecomp_Element_GlobalToLocal1D( void* meshDecomp, Element_GlobalIndex globalIndex ) {
	MeshDecomp*		self = (MeshDecomp*)meshDecomp;
	
	if (!IndexSet_IsMember( self->localElementSets[self->rank], globalIndex ) ) {
		return MD_E_Invalid( self );
	}
	else {
		int		lElement_I = 0;
		IndexSet_Index	index;
		
		/* since we now know this global index corresponds to a local element, just 
		find out how many other local elements there are before it. */
		for( index = 0; index < globalIndex; index++ ) {
			if( IndexSet_IsMember( self->localElementSets[self->rank], index ) ) {
				lElement_I++;
			}
		}	
		
		return lElement_I;
	}
}


Element_ShadowIndex _MeshDecomp_Element_GlobalToShadow1D( void* meshDecomp, Element_GlobalIndex globalIndex ) {
	MeshDecomp* self = (MeshDecomp*)meshDecomp;
	
	if ( (self->elementShadowCount == 0) || !IndexSet_IsMember( self->shadowElementSets[self->rank], globalIndex ) ) 
	{	
		return MD_E_Invalid( self );
	}	
	else {
		IndexSet_Index	index;
		Element_ShadowIndex	sElement_I = 0;
		
		/* since we now know this global index corresponds to a shadow element, just 
		find out how many other shadow elements there are before it. */
		for( index = 0; index < globalIndex; index++ ) {
			if( IndexSet_IsMember( self->shadowElementSets[self->rank], index ) ) {
				sElement_I++;
			}
		}	
		
		return sElement_I;
	}
	
	return MD_E_Invalid( self );
}


Element_DomainIndex _MeshDecomp_Element_GlobalToDomain1D( void* meshDecomp, Element_GlobalIndex globalIndex ) {
	MeshDecomp* self = (MeshDecomp*)meshDecomp;
	Element_LocalIndex lElement_I;
	Element_ShadowIndex sElement_I;

	if ( (lElement_I = self->elementMapGlobalToLocal( self, globalIndex)) != MD_E_Invalid(self) ) {
		return lElement_I;
	}
	else if ( (sElement_I = self->elementMapGlobalToShadow( self, globalIndex)) != MD_E_Invalid(self) ) {
		return self->elementLocalCount + sElement_I;
	}
	
	return MD_E_Invalid( self );
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/
void _MeshDecomp_DecomposeNodes( void* meshDecomp ) {
	MeshDecomp*		self = (MeshDecomp*)meshDecomp;
	Partition_Index		procCnt;
	Partition_Index*	procs;
	Partition_Index		proc_I;
	
	self->localNodeSets = Memory_Alloc_Array( IndexSet*, self->procsInUse, "MeshDecomp->localNodeSets" );
	for( proc_I = 0; proc_I < self->procsInUse; proc_I++ )
		self->localNodeSets[proc_I] = NULL;
	
	if( self->storage == StoreAll ) {
		procCnt = self->procsInUse;
		procs = Memory_Alloc_Array( Partition_Index, procCnt, "procs" );
		for( proc_I = 0; proc_I < self->procsInUse; proc_I++ )
			procs[proc_I] = proc_I;
	}
	else if( self->storage == StoreSelf ) {
		procCnt = 1;
		procs = Memory_Alloc_Array( Partition_Index, procCnt, "procs" );
		procs[proc_I] = self->rank;
	}
	else /* self->storage == StoreNeighbours */{
		procCnt = self->shadowProcCount( self ) + 1;
		procs = Memory_Alloc_Array( Partition_Index, procCnt, "procs" );
		procs[0] = self->rank;
		self->shadowBuildProcs( self, &procs[1] );
	}
	
	for( proc_I = 0; proc_I < procCnt; proc_I++ ) {
		Index*  indices = NULL;
		Index   indexCnt;
		
		self->localNodeSets[procs[proc_I]] = IndexSet_New( self->nodeGlobalCount );
		IndexSet_GetMembers( self->localElementSets[procs[proc_I]], &indexCnt, &indices );
		
		if( indexCnt ) {
			Node_GlobalIndex	nodeCnt;
			Node_GlobalIndex*       nodes;
			Node_GlobalIndex	n_I;
			
			nodeCnt = NodeLayout_BuildSubset( self->nodeLayout, indexCnt, indices, &nodes );
			
			if( nodeCnt ) {
				for( n_I = 0; n_I < nodeCnt; n_I++ )
					IndexSet_Add( self->localNodeSets[procs[proc_I]], nodes[n_I] );
				
				Memory_Free( nodes );
			}
			else {
				Stg_Class_Delete( self->localNodeSets[procs[proc_I]] );
				self->localNodeSets[procs[proc_I]] = NULL;
			}
			
			Memory_Free( indices );
		}
	}
	
	if( procs )
		Memory_Free( procs );
}


/* Note this func only works for shadow depth 1 - see the firewall below */
void _MeshDecomp_BuildShadowSets(
		void*					meshDecomp,
		Partition_Index				proc,
		Element_GlobalIndex			edgeElementCount,
		Element_GlobalIndex*			edgeElements )
{
	MeshDecomp*		self = (MeshDecomp*)meshDecomp;
	Topology*		topology = self->elementLayout->topology;
	Index			indexCnt;
	Index*			indices;
	Node_ShadowIndex	nodeCnt, sNode_I;
	Node_ShadowIndex*       nodes;
	Element_LocalIndex      lElement_I;
	Partition_Index		proc_I;
	Stream*                 error = Journal_Register( Error_Type, self->type );
	
	Journal_Firewall( self->shadowDepth > 0, error, "Error: %s() called when shadow depth set to 0.\n",
		__func__ );

	Journal_Firewall( self->shadowDepth == 1, error, "Error: %s() called when shadow depth > 1. This function "
		"for arbitrary mesh types uses Topology_BuildNeighbours() to calculate shadow nodes, and "
		"thus currently only works for shadow depth 1.\n", __func__ );
	
	/* We can only build shadow information for procs that have local element/node info, check
	   that 'storage' is set to an appropriate value */
	   
	/* If user has requested self storage only, then we can only build shadow
	   info for the current proc. Check that the user's hasn't requested it for
	   another proc. */
	if ( self->storage == StoreSelf ) {
		Journal_Firewall( proc == self->rank, error, "Error- in %s: Cannot build shadow sets "
			"for proc %d, since you have only requested 'StoreSelf' storage - thus "
			"cannot build info for processors other than this one's rank of %d.\n",
			__func__, proc, self->rank );
	}
	
	/* If user has requested self+neighbour storage only, then check that the given
	   processor to build shadow info for is actually on one our neighbours */
	if( self->storage == StoreNeighbours && proc != self->rank ) {
		NeighbourIndex  nbrCnt = self->procTopology->neighbourCount( self->procTopology, self->rank );
		
		if( nbrCnt ) {
			NeighbourIndex*		nbrs = Memory_Alloc_Array( NeighbourIndex, nbrCnt, "nbrs" );
			
			self->procTopology->buildNeighbours( self->procTopology, self->rank, nbrs );
			for( proc_I = 0; proc_I < nbrCnt; proc_I++ ) {
				if( nbrs[proc_I] == proc )
					break;
			}
			
			Memory_Free( nbrs );
			
			if( proc_I == nbrCnt )
				Journal_Firewall( 0, error, "Error- in %s: Cannot build shadow sets "
					"for proc %d, since you have requested 'StoreNeighbours' storage but "
					"requested proc is not among the current processor %d's %d neighbours.\n",
					__func__, proc, nbrCnt, self->rank );
		}
		else
			Journal_Firewall( 0, error, "Error- in %s: Cannot build shadow sets "
				"for proc %d, since you have requested 'StoreNeighbours' storage but "
				"the current processor %d has no neighbours.\n",
				__func__, proc, self->rank );
	}
	
	/* If the shadow set arrays have not yet been allocated, do it now */
	if( !self->shadowElementSets ) {
		self->shadowElementSets = Memory_Alloc_Array( IndexSet*, self->procsInUse, "MeshDecomp->shadowElementSets" );
		for( proc_I = 0; proc_I < self->procsInUse; proc_I++ )
			self->shadowElementSets[proc_I] = NULL;
	}
	
	if( !self->shadowNodeSets ) {
		self->shadowNodeSets = Memory_Alloc_Array( IndexSet*, self->procsInUse, "MeshDecomp->shadowNodeSets" );
		for( proc_I = 0; proc_I < self->procsInUse; proc_I++ )
			self->shadowNodeSets[proc_I] = NULL;
	}
	
	/* If the IndexSets for this proc have not been constructed, do it now */
	if( !self->shadowElementSets[proc] )
		self->shadowElementSets[proc] = IndexSet_New( self->elementGlobalCount );
	
	if( !self->shadowNodeSets[proc] )
		self->shadowNodeSets[proc] = IndexSet_New( self->nodeGlobalCount );
	
	/* Build shadow elements for this proc */
	for( lElement_I = 0; lElement_I < edgeElementCount; lElement_I++ ) {
		Element_GlobalIndex     gElement_I = edgeElements[lElement_I];
		Element_NeighbourIndex  eNbrCnt = topology->neighbourCount( topology, gElement_I );
		
		if( eNbrCnt ) {
			Element_NeighbourIndex		eNbr_I;
			Element_NeighbourIndex*		eNbrs = Memory_Alloc_Array( Element_NeighbourIndex, eNbrCnt, "eNbrs" );
			
			/* Note: using the neighbour only allows shadow depth of 1 */
			topology->buildNeighbours( topology, gElement_I, eNbrs );
			
			for( eNbr_I = 0; eNbr_I < eNbrCnt; eNbr_I++ ) {
				/* First check if this neigbour position is a real element. */
				if ( eNbrs[eNbr_I] < topology->indexCount ) {
					/* Now, only add the element to the shadow set if its on the Neigbour processor. */
					if( !IndexSet_IsMember( self->localElementSets[proc], eNbrs[eNbr_I] ) )
						IndexSet_Add( self->shadowElementSets[proc], eNbrs[eNbr_I] );
				}		
			}
			
			Memory_Free( eNbrs );
		}
	}
	
	IndexSet_GetMembers( self->shadowElementSets[proc], &indexCnt, &indices );
	nodeCnt = NodeLayout_BuildSubset( self->nodeLayout, indexCnt, indices, &nodes );
	
	for( sNode_I = 0; sNode_I < nodeCnt; sNode_I++ ) {
		if( !IndexSet_IsMember( self->localNodeSets[proc], nodes[sNode_I] ) )
			IndexSet_Add( self->shadowNodeSets[proc], nodes[sNode_I] );
	}
	
	Memory_Free( nodes );
}

/* Serial Functions - Reassign function pointers for superfast serial implementations */
void MeshDecomp_OverrideMethodsIfSerial( void* meshDecomp ) {
	MeshDecomp*		self = (MeshDecomp*)meshDecomp;

	/* if I am not a serial decomposition - then return */
	if ( self->nproc > 1 )
		return;

	self->nodeMapLocalToGlobal = _MeshDecomp_Node_MapLocalToGlobal_Serial;
	self->nodeMapDomainToGlobal = _MeshDecomp_Node_MapDomainToGlobal_Serial;
	self->nodeMapShadowToGlobal = _MeshDecomp_Node_MapShadowToGlobal_Serial;
	self->nodeMapGlobalToLocal = _MeshDecomp_Node_MapGlobalToLocal_Serial;
	self->nodeMapGlobalToDomain = _MeshDecomp_Node_MapGlobalToDomain_Serial;
	self->nodeMapGlobalToShadow = _MeshDecomp_Node_MapGlobalToShadow_Serial;

	self->elementMapLocalToGlobal = _MeshDecomp_Element_MapLocalToGlobal_Serial;
	self->elementMapDomainToGlobal = _MeshDecomp_Element_MapDomainToGlobal_Serial;
	self->elementMapShadowToGlobal = _MeshDecomp_Element_MapGlobalToShadow_Serial;
	self->elementMapGlobalToLocal = _MeshDecomp_Element_MapGlobalToLocal_Serial;
	self->elementMapGlobalToDomain = _MeshDecomp_Element_MapGlobalToDomain_Serial;
	self->elementMapGlobalToShadow = _MeshDecomp_Element_MapGlobalToShadow_Serial;

	self->shadowProcCount = _MeshDecomp_Shadow_ProcCount_Serial;
	self->shadowBuildProcs = _MeshDecomp_Shadow_BuildProcs_Serial;
	self->shadowProcElementCount = _MeshDecomp_Shadow_ProcElementCount_Serial;
	self->procWithElement = _MeshDecomp_Proc_WithElement_Serial;
}

Node_GlobalIndex _MeshDecomp_Node_MapLocalToGlobal_Serial( void* meshDecomp, Node_LocalIndex localIndex ) {
	return localIndex;
}

Node_GlobalIndex _MeshDecomp_Node_MapDomainToGlobal_Serial( void* meshDecomp, Node_DomainIndex domainIndex ) {
	return domainIndex;
}
Node_GlobalIndex _MeshDecomp_Node_MapShadowToGlobal_Serial( void* meshDecomp, Node_ShadowIndex shadowIndex ) {
	MeshDecomp*		self = (MeshDecomp*)meshDecomp;
	/* There should not be shadow nodes in serial */
	return MD_N_Invalid( self );
}

Node_LocalIndex _MeshDecomp_Node_MapGlobalToLocal_Serial( void* meshDecomp, Node_GlobalIndex globalIndex ) {
	return globalIndex;
}

Node_DomainIndex _MeshDecomp_Node_MapGlobalToDomain_Serial( void* meshDecomp, Node_GlobalIndex globalIndex ) {
	return globalIndex;
}

Node_ShadowIndex _MeshDecomp_Node_MapGlobalToShadow_Serial( void* meshDecomp, Node_GlobalIndex globalIndex ) {
	MeshDecomp*		self = (MeshDecomp*)meshDecomp;
	/* There should not be shadow nodes in serial */
	return MD_N_Invalid( self );	
}

Element_GlobalIndex _MeshDecomp_Element_MapLocalToGlobal_Serial( void* meshDecomp, Element_LocalIndex localIndex ) {
	return localIndex;
}

Element_GlobalIndex _MeshDecomp_Element_MapDomainToGlobal_Serial( void* meshDecomp, Element_DomainIndex domainIndex ) {
	return domainIndex;
}

Element_GlobalIndex _MeshDecomp_Element_MapShadowToGlobal_Serial( void* meshDecomp, Element_DomainIndex domainIndex ) {
	return domainIndex;
}

Element_LocalIndex _MeshDecomp_Element_MapGlobalToLocal_Serial( void* meshDecomp, Element_GlobalIndex globalIndex ) {
	return globalIndex;
}

Element_DomainIndex _MeshDecomp_Element_MapGlobalToDomain_Serial( void* meshDecomp, Element_GlobalIndex globalIndex ) {
	return globalIndex;
}

Element_ShadowIndex _MeshDecomp_Element_MapGlobalToShadow_Serial( void* meshDecomp, Element_GlobalIndex globalIndex ) {
	MeshDecomp*		self = (MeshDecomp*)meshDecomp;
	/* There should not be shadow elements in serial */
	return MD_E_Invalid( self );
}
Partition_Index _MeshDecomp_Shadow_ProcCount_Serial( void* meshDecomp ) {
	return 0; /* I think that this is right - not sure though */
}
void _MeshDecomp_Shadow_BuildProcs_Serial( void* meshDecomp, Partition_Index* procs ) {}
Element_ShadowIndex _MeshDecomp_Shadow_ProcElementCount_Serial( void* meshDecomp, Partition_Index rank ) {
	return 0; /* There are no shadow elements in serial */
}
Partition_Index _MeshDecomp_Proc_WithElement_Serial( void* meshDecomp, Element_GlobalIndex globalIndex ) {
	return 0; /* The only processor is Procesor 0 - this function is easy */
}

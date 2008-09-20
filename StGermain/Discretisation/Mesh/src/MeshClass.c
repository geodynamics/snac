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
** $Id: MeshClass.c 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"

#include "types.h"
#include "shortcuts.h"
#include "MeshTopology.h"
#include "MeshClass.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ElementLayout.h"
#include "NodeLayout.h"
#include "MeshDecomp.h"
#include "MeshLayout.h"
#include "ShadowInfo.h"
#include "MeshGeometry.h"
#include "HexaEL.h"
#include "ParallelPipedHexaEL.h"

/* Textual name of this class */
const Type Mesh_Type = "Mesh";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

Mesh* Mesh_DefaultNew( Name name )
{
	return _Mesh_New( 
		sizeof(Mesh), 
		Mesh_Type, 
		_Mesh_Delete, 
		_Mesh_Print, 
		_Mesh_Copy, 
		(void* (*)(Name))Mesh_DefaultNew,
		_Mesh_Construct,
		_Mesh_Build, 
		_Mesh_Initialise, 
		_Mesh_Execute,
		_Mesh_Destroy,
		name,
		False,
		_Mesh_Node_IsLocal1D,
		_Mesh_Node_IsShadow1D,
		_Mesh_Element_IsLocal1D,
		_Mesh_Element_IsShadow1D,
		NULL, 
		0,
		0, 
		NULL, 
		NULL );
}

Mesh* Mesh_New(
	Name						name,
	void*						layout,
	SizeT						_nodeSize,
	SizeT						_elementSize,
	void*						extensionMgr_Register,
	Dictionary*					dictionary )
{
	return _Mesh_New( 
		sizeof(Mesh), 
		Mesh_Type, 
		_Mesh_Delete, 
		_Mesh_Print, 
		_Mesh_Copy, 
		(void* (*)(Name))Mesh_DefaultNew,
		_Mesh_Construct,
		_Mesh_Build, 
		_Mesh_Initialise, 
		_Mesh_Execute,
		_Mesh_Destroy,
		name,
		True,
		_Mesh_Node_IsLocal1D,
		_Mesh_Node_IsShadow1D,
		_Mesh_Element_IsLocal1D,
		_Mesh_Element_IsShadow1D,
		layout, 
		_nodeSize,
		_elementSize, 
		extensionMgr_Register, 
		dictionary );
}


void Mesh_Init(
	Mesh*						self,
	Name						name,
	void*						layout,
	SizeT						_nodeSize,
	SizeT						_elementSize,
	void*						extensionMgr_Register,
	Dictionary*					dictionary )
{
	/* General info */
	self->type = Mesh_Type;
	self->_sizeOfSelf = sizeof(Mesh);
	self->_deleteSelf = False;
	self->dictionary = dictionary;
	
	/* Virtual info */
	self->_delete = _Mesh_Delete;
	self->_print = _Mesh_Print;
	self->_copy = _Mesh_Copy;
	self->_defaultConstructor = (void* (*)(Name))Mesh_DefaultNew;
	self->_construct = _Mesh_Construct,
		self->_build = _Mesh_Build;
	self->_initialise = _Mesh_Initialise;
	self->_execute = _Mesh_Execute;
	self->_destroy = _Mesh_Destroy,
		self->nodeIsLocal = _Mesh_Node_IsLocal1D;
	self->nodeIsShadow = _Mesh_Node_IsShadow1D;
	self->elementIsLocal = _Mesh_Element_IsLocal1D;
	self->elementIsShadow = _Mesh_Element_IsShadow1D;
	
	_Stg_Class_Init( (Stg_Class*)self );
	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );
	_Stg_Component_Init( (Stg_Component*)self );
	/* Mesh info */
	_Mesh_Init( self, layout, _nodeSize, _elementSize, extensionMgr_Register );
}


Mesh* _Mesh_New(
	SizeT						_sizeOfSelf, 
	Type						type,
	Stg_Class_DeleteFunction*				_delete,
	Stg_Class_PrintFunction*				_print, 
	Stg_Class_CopyFunction*				_copy, 
	Stg_Component_DefaultConstructorFunction*	_defaultConstructor,
	Stg_Component_ConstructFunction*			_construct,
	Stg_Component_BuildFunction*			_build,
	Stg_Component_InitialiseFunction*			_initialise,
	Stg_Component_ExecuteFunction*			_execute,
	Stg_Component_DestroyFunction*			_destProc_Iroy,
	Name								name,
	Bool								initFlag,
	Mesh_Node_IsLocalFunction*			_nodeIsLocal,
	Mesh_Node_IsShadowFunction*			_nodeIsShadow,
	Mesh_Element_IsLocalFunction*			_elementIsLocal,
	Mesh_Element_IsShadowFunction*			_elementIsShadow,
	void*						layout,
	SizeT						nodeSize,
	SizeT						elementSize, 
	void*						extensionMgr_Register,
	Dictionary*					dictionary )
{
	Mesh* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(Mesh) );
	self = (Mesh*)_Stg_Component_New( _sizeOfSelf, type, _delete, _print, _copy, _defaultConstructor, _construct, _build, 
					  _initialise, _execute, _destProc_Iroy, name, NON_GLOBAL );
	
	/* General info */
	self->dictionary = dictionary;
	
	/* Virtual info */
	self->nodeIsLocal = _nodeIsLocal;
	self->nodeIsShadow = _nodeIsShadow;
	self->elementIsLocal = _elementIsLocal;
	self->elementIsShadow = _elementIsShadow;
	
	/* Mesh info */
	if( initFlag ){
		_Mesh_Init( self, layout, nodeSize, elementSize, extensionMgr_Register );
	}
	
	return self;
}

void _Mesh_Init(
	Mesh*						self,
	void*						layout,
	SizeT						_nodeSize,
	SizeT						_elementSize,
	void*						extensionMgr_Register )
{
	/* General and Virtual info should already be set */
	
	/* Mesh info */
	self->isConstructed = True;
	self->layout = (MeshLayout*)layout;
	self->nodeExtensionMgr = ExtensionManager_New_OfStruct( "node", _nodeSize );
	self->elementExtensionMgr = ExtensionManager_New_OfStruct( "element", _elementSize );
	ExtensionManager_Register_Add( extensionMgr_Register, self->nodeExtensionMgr );
	ExtensionManager_Register_Add( extensionMgr_Register, self->elementExtensionMgr );
	self->extensionMgr_Register = extensionMgr_Register;
	self->debug = Journal_Register( Debug_Type, self->type );

	/* New topology stuff. */
	self->topo = MeshTopology_New( "" );
	self->useTopo = False;
	self->generator = NULL;
	self->info = ExtensionManager_New_OfExistingObject( "mesh_info", self );
	
	/* Nodes */
	self->nodeLocalCount = 0;
	self->nodeDomainCount = 0;
	self->nodeShadowCount = 0;
	self->nodeGlobalCount = 0;
	self->nodeL2G = 0;
	self->nodeD2G = 0;
	self->nodeG2L = 0;
	self->nodeG2D = 0;
	self->nodeNeighbourTbl = 0;
	self->nodeNeighbourCountTbl = 0;
	self->nodeElementTbl = 0;
	self->nodeElementCountTbl = 0;
	/* By default, build the local to global table for speed. The global to local tables are
	   potentially memory-hungry for large parallel runs, so don't build them by default */
	self->buildNodeLocalToGlobalMap = Dictionary_Entry_Value_AsBool(
		Dictionary_GetDefault( self->dictionary, "buildNodeLocalToGlobalMap",
				       Dictionary_Entry_Value_FromBool( True ) ) );
	self->buildNodeDomainToGlobalMap = Dictionary_Entry_Value_AsBool(
		Dictionary_GetDefault( self->dictionary, "buildNodeDomainToGlobalMap",
				       Dictionary_Entry_Value_FromBool( True ) ) );
	self->buildNodeGlobalToLocalMap = Dictionary_Entry_Value_AsBool(
		Dictionary_GetDefault( self->dictionary, "buildNodeGlobalToLocalMap",
				       Dictionary_Entry_Value_FromBool( False ) ) );
	self->buildNodeGlobalToDomainMap = Dictionary_Entry_Value_AsBool(
		Dictionary_GetDefault( self->dictionary, "buildNodeGlobalToDomainMap",
				       Dictionary_Entry_Value_FromBool( True ) ) );
	self->buildNodeNeighbourTbl = Dictionary_Entry_Value_AsBool(
		Dictionary_GetDefault( self->dictionary, "buildNodeNeighbourTbl",
				       Dictionary_Entry_Value_FromBool( True ) ) );
	self->buildNodeElementTbl = Dictionary_Entry_Value_AsBool(
		Dictionary_GetDefault( self->dictionary, "buildNodeElementTbl",
				       Dictionary_Entry_Value_FromBool( False ) ) );
	self->ownsNodeLocalToGlobalMap = True;
	self->ownsNodeDomainToGlobalMap = True;
	self->ownsNodeGlobalToDomainMap = True;
	self->ownsNodeGlobalToLocalMap = True;
	self->ownsNodeNeighbourTbl = True;
	self->ownsNodeNeighbourCountTbl = True;
	self->ownsNodeElementTbl = True;
	self->ownsNodeElementCountTbl = True;
	
	/* Element */
	self->elementDomainCount = 0;
	self->elementLocalCount = 0;
	self->elementShadowCount = 0;
	self->elementGlobalCount = 0;
	self->elementL2G = 0;
	self->elementD2G = 0;
	self->elementG2L = 0;
	self->elementG2D = 0;
	self->elementNodeTbl = 0;
	self->elementNodeCountTbl = 0;
	self->elementNeighbourTbl = 0;
	self->elementNeighbourCountTbl = 0;
	self->buildElementLocalToGlobalMap = Dictionary_Entry_Value_AsBool(
		Dictionary_GetDefault( self->dictionary, "buildElementLocalToGlobalMap",
				       Dictionary_Entry_Value_FromBool( True ) ) );
	self->buildElementDomainToGlobalMap = Dictionary_Entry_Value_AsBool(
		Dictionary_GetDefault( self->dictionary, "buildElementDomainToGlobalMap",
				       Dictionary_Entry_Value_FromBool( True ) ) );
	self->buildElementGlobalToLocalMap = Dictionary_Entry_Value_AsBool(
		Dictionary_GetDefault( self->dictionary, "buildElementGlobalToLocalMap",
				       Dictionary_Entry_Value_FromBool( False ) ) );
	self->buildElementGlobalToDomainMap = Dictionary_Entry_Value_AsBool(
		Dictionary_GetDefault( self->dictionary, "buildElementGlobalToDomainMap",
				       Dictionary_Entry_Value_FromBool( False ) ) );
	self->buildElementNodeTbl = Dictionary_Entry_Value_AsBool(
		Dictionary_GetDefault( self->dictionary, "buildElementNodeTbl",
				       Dictionary_Entry_Value_FromBool( False ) ) );
	self->buildElementNeighbourTbl = Dictionary_Entry_Value_AsBool(
		Dictionary_GetDefault( self->dictionary, "buildElementNeighbourTbl",
				       Dictionary_Entry_Value_FromBool( False ) ) );
	self->buildTemporaryGlobalTables = Dictionary_Entry_Value_AsBool(		
		Dictionary_GetDefault( self->dictionary, "buildTemporaryGlobalTables",
				       Dictionary_Entry_Value_FromBool( True ) ) );
	self->ownsElementLocalToGlobalMap = True;
	self->ownsElementDomainToGlobalMap = True;
	self->ownsElementGlobalToDomainMap = True;
	self->ownsElementGlobalToLocalMap = True;
	self->ownsElementNodeTbl = True;
	self->ownsElementNodeCountTbl = True;
	self->ownsElementNeighbourTbl = True;
	self->ownsElementNeighbourCountTbl = True;
	
	/* sync info */
	self->procNbrInfo = Memory_Alloc( ProcNbrInfo, "Mesh->procNbrInfo" );
	self->procNbrInfo->procNbrCnt = 0;
	self->procNbrInfo->procNbrTbl = NULL;
	self->elementShadowInfo = Memory_Alloc( ShadowInfo, "Mesh->elementShadowInfo" );
	self->elementShadowInfo->procShadowedCnt = NULL;
	self->elementShadowInfo->procShadowedTbl = NULL;
	self->elementShadowInfo->procShadowCnt = NULL;
	self->elementShadowInfo->procShadowTbl = NULL;
	self->nodeShadowInfo = Memory_Alloc( ShadowInfo, "Mesh->nodeShadowInfo" );
	self->nodeShadowInfo->procShadowedCnt = NULL;
	self->nodeShadowInfo->procShadowedTbl = NULL;
	self->nodeShadowInfo->procShadowCnt = NULL;
	self->nodeShadowInfo->procShadowTbl = NULL;
	
	self->syncRequestTbl = NULL;
	self->syncElementTbl = NULL;
	self->syncNodeCoordTbl = NULL;
	self->syncNodeTbl = NULL;
	self->syncElementSendTbl = NULL;
	self->syncNodeCoordTbl = NULL;
	self->syncNodeTbl = NULL;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _Mesh_Construct( void* mesh, Stg_ComponentFactory* cf, void* data ) {
	Mesh* self = (Mesh*)mesh;
	Dictionary *componentDict = NULL;
	Dictionary *thisComponentDict = NULL;
	MeshLayout *meshLayout = NULL;
	void *extensionManager_Register = NULL;
	unsigned int elementSize = 0;
	unsigned int nodeSize = 0;

	self->dictionary = cf->rootDict;
	componentDict = cf->componentDict;
	assert( componentDict );
	thisComponentDict = Dictionary_Entry_Value_AsDictionary( Dictionary_Get( componentDict, self->name ) );
	assert( thisComponentDict );
	
	meshLayout =  Stg_ComponentFactory_ConstructByKey(  cf,  self->name,  MeshLayout_Type, MeshLayout,  True, data ) ;
	
	extensionManager_Register = Stg_ObjectList_Get( cf->registerRegister, "ExtensionManager_Register" );
	assert( extensionManager_Register );
	

	elementSize = Dictionary_GetUnsignedInt_WithDefault( thisComponentDict, "ElementSize", 8 );
	nodeSize = Dictionary_GetUnsignedInt_WithDefault( thisComponentDict, "NodeSize", 0 );

	self->nSpaceDims = Stg_ComponentFactory_GetRootDictUnsignedInt( cf, "dim", 0 );

	_Mesh_Init( (Mesh*)self, meshLayout, nodeSize, elementSize, extensionManager_Register );
}

void _Mesh_Destroy( void* mesh, void* data )
{
	
}

void _Mesh_Delete( void* mesh ) {
	Mesh* self = (Mesh*)mesh;

	FreeObject( self->topo );
	Memory_Free( self->nodeCoord );

	Mesh_DestroyTables( self );
	
	/* Stg_Class_Delete the mesh itself */
	if( self->element ) {
		ExtensionManager_Free( self->elementExtensionMgr, self->element );
	}
	if( self->node ) {
		ExtensionManager_Free( self->nodeExtensionMgr, self->node );
	}
	
	/* Stg_Class_Delete parent */
	_Stg_Component_Delete( self );
}


void Mesh_DestroyTables( void* mesh ) {
	Mesh* self = (Mesh*)mesh;
	
	/* Because remeshing may have the same connectivity, we may want to pass this onto the new mesh, hence "owns" flags */
	if( self->ownsElementNeighbourTbl && self->elementNeighbourTbl ) {
		Memory_Free( self->elementNeighbourTbl );
	}
	if( self->ownsElementNeighbourCountTbl && self->elementNeighbourCountTbl ) {
		Memory_Free( self->elementNeighbourCountTbl );
	}
	if( self->ownsElementNodeTbl && self->elementNodeTbl ) {
		Memory_Free( self->elementNodeTbl );
	}
	if( self->ownsElementNodeCountTbl && self->elementNodeCountTbl ) {
		Memory_Free( self->elementNodeCountTbl );
	}
	if( self->ownsElementGlobalToDomainMap && self->elementG2D ) {
		Memory_Free( self->elementG2D );
	}
	/* Note: G2D is same ptr as G2L if both active and no shadowing */
	if( self->ownsElementGlobalToLocalMap && self->elementG2L && 
	    !(self->elementG2D && (self->nodeShadowCount == 0) ) )
	{
		Memory_Free( self->elementG2L );
	}
	if( self->ownsElementDomainToGlobalMap && self->elementD2G ) {
		Memory_Free( self->elementD2G );
	}
	/* L2G is same ptr as D2G if both requested */
	if( self->ownsElementLocalToGlobalMap && self->elementL2G && !self->elementD2G ) {
		Memory_Free( self->elementL2G );
	}
	if( self->ownsNodeElementTbl && self->nodeElementTbl ) {
		Memory_Free( self->nodeElementTbl );
	}
	if( self->ownsNodeElementCountTbl && self->nodeElementCountTbl ) {
		Memory_Free( self->nodeElementCountTbl );
	}
	if( self->ownsNodeNeighbourTbl && self->nodeNeighbourTbl ) {
		Memory_Free( self->nodeNeighbourTbl );
	}
	if( self->ownsNodeNeighbourCountTbl && self->nodeNeighbourCountTbl ) {
		Memory_Free( self->nodeNeighbourCountTbl );
	}
	if( self->ownsNodeGlobalToDomainMap && self->nodeG2D ) {
		Memory_Free( self->nodeG2D );
	}
	/* G2D is same ptr as G2L if both active and no shadowing */
	if( self->ownsNodeGlobalToLocalMap && self->nodeG2L && 
	    !(self->nodeG2D && (self->nodeShadowCount == 0) ) )
	{	
		Memory_Free( self->nodeG2L );
	}
	if( self->ownsElementDomainToGlobalMap && self->nodeD2G ) {
		Memory_Free( self->nodeD2G );
	}
	/* L2G is same ptr as D2G if both requested */
	if( self->ownsElementLocalToGlobalMap && self->nodeL2G && !self->nodeD2G ) {
		Memory_Free( self->nodeL2G );
	}
	
	/* sync info */
	if( self->procNbrInfo->procNbrTbl ) {
		Memory_Free( self->procNbrInfo->procNbrTbl );
	}
	
	if( self->elementShadowInfo->procShadowedCnt ) {
		Memory_Free( self->elementShadowInfo->procShadowedCnt );
	}
	if( self->elementShadowInfo->procShadowedTbl ) {
		NeighbourIndex			nbr_I;
		
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			if( self->elementShadowInfo->procShadowedTbl[nbr_I] ) {
				Memory_Free( self->elementShadowInfo->procShadowedTbl[nbr_I] );
			}
		}
		Memory_Free( self->elementShadowInfo->procShadowedTbl );
	}
	if( self->elementShadowInfo->procShadowCnt ) {
		Memory_Free( self->elementShadowInfo->procShadowCnt );
	}
	if( self->elementShadowInfo->procShadowTbl ) {
		NeighbourIndex			nbr_I;
		
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			if( self->elementShadowInfo->procShadowTbl[nbr_I] ) {
				Memory_Free( self->elementShadowInfo->procShadowTbl[nbr_I] );
			}
		}
		Memory_Free( self->elementShadowInfo->procShadowTbl );
	}
	
	if( self->nodeShadowInfo->procShadowedCnt ) {
		Memory_Free( self->nodeShadowInfo->procShadowedCnt );
	}
	if( self->nodeShadowInfo->procShadowedTbl ) {
		NeighbourIndex			nbr_I;
		
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			if( self->nodeShadowInfo->procShadowedTbl[nbr_I] ) {
				Memory_Free( self->nodeShadowInfo->procShadowedTbl[nbr_I] );
			}
		}
		Memory_Free( self->nodeShadowInfo->procShadowedTbl );
	}
	if( self->nodeShadowInfo->procShadowCnt ) {
		Memory_Free( self->nodeShadowInfo->procShadowCnt );
	}
	if( self->nodeShadowInfo->procShadowTbl ) {
		NeighbourIndex			nbr_I;
		
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			if( self->nodeShadowInfo->procShadowTbl[nbr_I] ) {
				Memory_Free( self->nodeShadowInfo->procShadowTbl[nbr_I] );
			}
		}
		Memory_Free( self->nodeShadowInfo->procShadowTbl );
	}
	
	if( self->syncRequestTbl ) {
		NeighbourIndex		nbr_I;
		
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			if( self->syncRequestTbl[nbr_I] )
				Memory_Free( self->syncRequestTbl[nbr_I] );
		}
		Memory_Free( self->syncRequestTbl );
	}
	if( self->syncElementTbl ) {
		NeighbourIndex		nbr_I;
		
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			if( self->syncElementTbl[nbr_I] )
				Memory_Free( self->syncElementTbl[nbr_I] );
		}
		Memory_Free( self->syncElementTbl );
	}
	if( self->syncNodeCoordTbl ) {
		NeighbourIndex		nbr_I;
		
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			if( self->syncNodeCoordTbl[nbr_I] )
				Memory_Free( self->syncNodeCoordTbl[nbr_I] );
		}
		Memory_Free( self->syncNodeCoordTbl );
	}
	if( self->syncNodeTbl ) {
		NeighbourIndex		nbr_I;
		
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			if( self->syncNodeTbl[nbr_I] )
				Memory_Free( self->syncNodeTbl[nbr_I] );
		}
		Memory_Free( self->syncNodeTbl );
	}
	
	if( self->syncElementSendTbl ) {
		Memory_Free( self->syncElementSendTbl );
	}
	if( self->syncNodeCoordSendTbl ) {
		Memory_Free( self->syncNodeCoordSendTbl );
	}
	if( self->syncNodeTbl ) {
		Memory_Free( self->syncNodeSendTbl );
	}
	Memory_Free( self->elementShadowInfo );
	Memory_Free( self->nodeShadowInfo );
	Memory_Free( self->procNbrInfo );
}


void _Mesh_Print( void* mesh, Stream* stream ) {
	Mesh* self = (Mesh*)mesh;
	
	/* Set the Journal for printing informations */
	Stream* meshStream;
	meshStream = Journal_Register( InfoStream_Type, "MeshStream" );

	/* Print parent */
	Journal_Printf( stream, "Mesh (ptr): (%p)\n", self );
	_Stg_Component_Print( self, stream );
	
	/* General info */
	Journal_Printf( stream, "\tdictionary (ptr): %p\n", self->dictionary );
	
	/* Virtual info */
	
	/* Mesh info */
	Journal_Printf( stream, "\tlayout (ptr): %p\n", self->layout );
	Print( self->nodeExtensionMgr, stream );
	Print( self->elementExtensionMgr, stream );
	
	/* Nodes */
	Journal_Printf( stream, "\tnode (ptr): %p\n", self->node );
	Journal_Printf( stream, "\tnodeLocalCount: %d\n", self->nodeLocalCount );
	Journal_Printf( stream, "\tnodeDomainCount: %d\n", self->nodeDomainCount );
	Journal_Printf( stream, "\tnodeShadowCount: %d\n", self->nodeShadowCount );
	Journal_Printf( stream, "\tnodeGlobalCount: %d\n", self->nodeGlobalCount );
	_Mesh_PrintCoords( self, stream );

	Journal_Printf( stream, "\townsNodeLocalToGlobalMap: %u\n", self->ownsNodeLocalToGlobalMap );
	_Mesh_PrintTable( stream, self->nodeL2G, "nodeL2G", self->nodeLocalCount );
	Journal_Printf( stream, "\townsNodeDomainToGlobalMap: %u\n", self->ownsNodeDomainToGlobalMap );
	_Mesh_PrintTable( stream, self->nodeD2G, "nodeD2G", self->nodeDomainCount );
	Journal_Printf( stream, "\townsNodeGlobalToLocalMap: %u\n", self->ownsNodeGlobalToLocalMap );
	_Mesh_PrintTable( stream, self->nodeG2L, "nodeG2L", self->nodeGlobalCount );
	Journal_Printf( stream, "\townsNodeGlobalToDomainMap: %u\n", self->ownsNodeGlobalToDomainMap );
	_Mesh_PrintTable( stream, self->nodeG2D, "nodeG2D", self->nodeGlobalCount );

	Journal_Printf( stream, "\townsNodeNeighbourCountTbl: %u\n", self->ownsNodeNeighbourCountTbl );
	Journal_Printf( stream, "\tnodeNeighbourCountTbl (ptr): %p\n", self->nodeNeighbourCountTbl );
	Journal_Printf( stream, "\townsNodeNeighbourTbl: %u\n", self->ownsNodeNeighbourTbl );
	_Mesh_Print2DTable( stream, self->nodeNeighbourTbl, self->nodeNeighbourCountTbl, "nodeNeighbourTbl",
			    self->nodeDomainCount );

	Journal_Printf( stream, "\townsNodeElementCountTbl: %u\n", self->ownsNodeElementCountTbl );
	Journal_Printf( stream, "\tnodeElementCountTbl (ptr): %p\n", self->nodeElementCountTbl );
	Journal_Printf( stream, "\townsNodeElementTbl: %u\n", self->ownsNodeElementTbl );
	_Mesh_Print2DTable( stream, self->nodeElementTbl, self->nodeElementCountTbl, "nodeElementTbl",
			    self->nodeDomainCount );
	
	/* Element */
	Journal_Printf( stream, "\telement (ptr): %p\n", self->element );
	Journal_Printf( stream, "\telementLocalCount: %d\n", self->elementLocalCount );
	Journal_Printf( stream, "\telementDomainCount: %d\n", self->elementDomainCount );
	Journal_Printf( stream, "\telementShadowCount: %d\n", self->elementShadowCount );
	Journal_Printf( stream, "\telementGlobalCount: %d\n", self->elementGlobalCount );

	Journal_Printf( stream, "\townsElementLocalToGlobalMap: %u\n", self->ownsElementLocalToGlobalMap );
	_Mesh_PrintTable( stream, self->elementL2G, "elementL2G", self->elementLocalCount );
	Journal_Printf( stream, "\townsElementDomainToGlobalMap: %u\n", self->ownsElementDomainToGlobalMap );
	_Mesh_PrintTable( stream, self->elementD2G, "elementD2G", self->elementDomainCount );
	Journal_Printf( stream, "\townsElementGlobalToLocalMap: %u\n", self->ownsElementGlobalToLocalMap );
	_Mesh_PrintTable( stream, self->elementG2L, "elementG2L", self->elementGlobalCount );
	Journal_Printf( stream, "\townsElementGlobalToDomainMap: %u\n", self->ownsElementGlobalToDomainMap );
	_Mesh_PrintTable( stream, self->elementG2D, "elementG2D", self->elementGlobalCount );

	Journal_Printf( stream, "\townsElementNeighbourCountTbl: %u\n", self->ownsElementNeighbourCountTbl );
	Journal_Printf( stream, "\telementNeighbourCountTbl (ptr): %p\n", self->elementNeighbourCountTbl );
	Journal_Printf( stream, "\townsElementNeighbourTbl: %u\n", self->ownsElementNeighbourTbl );
	_Mesh_Print2DTable( stream, self->elementNeighbourTbl, self->elementNeighbourCountTbl, "elementNeighbourTbl",
			    self->elementDomainCount );
	Journal_Printf( stream, "\townsElementNodeCountTbl: %u\n", self->ownsElementNodeCountTbl );
	Journal_Printf( stream, "\telementNodeCountTbl (ptr): %p\n", self->elementNodeCountTbl );
	Journal_Printf( stream, "\townsElementNodeTbl: %u\n", self->ownsElementNodeTbl );
	_Mesh_Print2DTable( stream, self->elementNodeTbl, self->elementNodeCountTbl, "elementNodeTbl",
			    self->elementDomainCount );
	
	/* sync info */
	Journal_Printf( stream, "\tprocNbrCnt: %d\n", self->procNbrInfo->procNbrCnt );
	Journal_Printf( stream, "\tprocNbrTbl (ptr): %p\n", self->procNbrInfo->procNbrTbl );
	if( self->procNbrInfo->procNbrTbl ) {
		NeighbourIndex		nbr_I;
		
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			Journal_Printf( stream, "\t\tprocNbrTbl[%d]: %d\n", nbr_I, self->procNbrInfo->procNbrTbl[nbr_I] );
		}
	}
	
	Journal_Printf( stream, "\telementShadowInfo->procShadowedCnt (ptr): %p\n", self->elementShadowInfo->procShadowedCnt );
	if( self->elementShadowInfo->procShadowedCnt ) {
		NeighbourIndex		nbr_I;
		
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			Journal_Printf( stream, "\t\telementShadowInfo->procShadowedCnt[%d]: %d\n", nbr_I, 
					self->elementShadowInfo->procShadowedCnt[nbr_I] );
		}
	}
	Journal_Printf( stream, "\telementShadowInfo->procShadowedTbl (ptr): %p\n", self->elementShadowInfo->procShadowedTbl );
	if( self->elementShadowInfo->procShadowedTbl ) {
		NeighbourIndex		nbr_I;
		
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			Journal_Printf( stream, "\t\telementShadowInfo->procShadowedTbl[%d] (ptr): %p\n", nbr_I, 
					self->elementShadowInfo->procShadowedTbl[nbr_I] );
			if( self->elementShadowInfo->procShadowedTbl[nbr_I] ) {
				Element_LocalIndex		element_lI;
				
				for( element_lI = 0; element_lI < self->elementShadowInfo->procShadowedCnt[nbr_I]; element_lI++ ) {
					Journal_Printf( stream, "\t\t\telementShadowInfo->procShadowedTbl[%d][%d]: %d\n", nbr_I, element_lI, 
							self->elementShadowInfo->procShadowedTbl[nbr_I][element_lI] );
				}
			}
		}
	}
	
	Journal_Printf( stream, "\telementShadowInfo->procShadowCnt (ptr): %p\n", self->elementShadowInfo->procShadowCnt );
	if( self->elementShadowInfo->procShadowCnt ) {
		NeighbourIndex		nbr_I;
		
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			Journal_Printf( stream, "\t\telementShadowInfo->procShadowCnt[%d]: %d\n", nbr_I, 
					self->elementShadowInfo->procShadowCnt[nbr_I] );
		}
	}
	Journal_Printf( stream, "\telementShadowInfo->procShadowTbl (ptr): %p\n", self->elementShadowInfo->procShadowTbl );
	if( self->elementShadowInfo->procShadowTbl ) {
		NeighbourIndex		nbr_I;
		
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			Journal_Printf( stream, "\t\telementShadowInfo->procShadowTbl[%d] (ptr): %p\n", nbr_I, 
					self->elementShadowInfo->procShadowTbl[nbr_I] );
			if( self->elementShadowInfo->procShadowTbl[nbr_I] ) {
				Element_ShadowIndex		element_sI;
				
				for( element_sI = 0; element_sI < self->elementShadowInfo->procShadowCnt[nbr_I]; element_sI++ ) {
					Journal_Printf( stream, "\t\t\telementShadowInfo->procShadowTbl[%d][%d]: %d\n", nbr_I, element_sI, 
							self->elementShadowInfo->procShadowTbl[nbr_I][element_sI] );
				}
			}
		}
	}
	
	Journal_Printf( stream, "\tnodeShadowInfo->procShadowedCnt (ptr): %p\n", self->nodeShadowInfo->procShadowedCnt );
	if( self->nodeShadowInfo->procShadowedCnt ) {
		NeighbourIndex		nbr_I;
		
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			Journal_Printf( stream, "\t\tnodeShadowInfo->procShadowedCnt[%d]: %d\n", nbr_I, 
					self->nodeShadowInfo->procShadowedCnt[nbr_I] );
		}
	}
	Journal_Printf( stream, "\tnodeShadowInfo->procShadowedTbl (ptr): %p\n", self->nodeShadowInfo->procShadowedTbl );
	if( self->nodeShadowInfo->procShadowedTbl ) {
		NeighbourIndex		nbr_I;
		
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			Journal_Printf( stream, "\t\tnodeShadowInfo->procShadowedTbl[%d] (ptr): %p\n", nbr_I,
					self->nodeShadowInfo->procShadowedTbl[nbr_I] );
			if( self->nodeShadowInfo->procShadowedTbl[nbr_I] ) {
				Node_LocalIndex		node_lI;
				
				for( node_lI = 0; node_lI < self->nodeShadowInfo->procShadowedCnt[nbr_I]; node_lI++ ) {
					Journal_Printf( stream, "\t\t\tnodeShadowInfo->procShadowedTbl[%d][%d]: %d\n", nbr_I, node_lI, 
							self->nodeShadowInfo->procShadowedTbl[nbr_I][node_lI] );
				}
			}
		}
	}
	
	Journal_Printf( stream, "\tnodeShadowInfo->procShadowCnt (ptr): %p\n", self->nodeShadowInfo->procShadowCnt );
	if( self->nodeShadowInfo->procShadowCnt ) {
		NeighbourIndex		nbr_I;
		
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			Journal_Printf( stream, "\t\tnodeShadowInfo->procShadowCnt[%d]: %d\n", nbr_I, 
					self->nodeShadowInfo->procShadowCnt[nbr_I] );
		}
	}
	Journal_Printf( stream, "\tnodeShadowInfo->procShadowTbl (ptr): %p\n", self->nodeShadowInfo->procShadowTbl );
	if( self->nodeShadowInfo->procShadowTbl ) {
		NeighbourIndex		nbr_I;
		
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			Journal_Printf( stream, "\t\tnodeShadowInfo->procShadowTbl[%d] (ptr): %p\n", nbr_I,
					self->nodeShadowInfo->procShadowTbl[nbr_I] );
			if( self->nodeShadowInfo->procShadowTbl[nbr_I] ) {
				Node_ShadowIndex		node_sI;
				
				for( node_sI = 0; node_sI < self->nodeShadowInfo->procShadowCnt[nbr_I]; node_sI++ ) {
					Journal_Printf( stream, "\t\t\tnodeShadowInfo->procShadowTbl[%d][%d]: %d\n", nbr_I, node_sI, 
							self->nodeShadowInfo->procShadowTbl[nbr_I][node_sI] );
				}
			}
		}
	}
}


void _Mesh_PrintCoords( Mesh* self, Stream* meshStream )
{
	Index	itemCount = self->nodeDomainCount;
	Index	 item_I;

	Journal_Printf( meshStream, "\tnodeCoord (ptr): %p\n", self->nodeCoord );
	Journal_Printf( meshStream, "\t%s[0-%u]:   ", "nodeCoord", itemCount );
	if( self->nodeCoord ) {
		Coord* coordPtr = NULL;

		for( item_I = 0; item_I < itemCount; item_I++ ) {
			coordPtr = &self->nodeCoord[item_I];
			Journal_Printf( meshStream, "(%.2f,%.2f,%.2f), ", (*coordPtr)[0], (*coordPtr)[1],
					(*coordPtr)[2] );
		}
		Journal_Printf( meshStream, "\n" );
	}
	else {
		Journal_Printf( meshStream, "(null)\n");
	}
}


void _Mesh_PrintTable( Stream* meshStream, Index* table, const Name tableName, Index itemCount )
{
	Index item_I;

	Journal_Printf( meshStream, "\t%s (ptr): %p\n", tableName, table );
	Journal_Printf( meshStream, "\t%s[0-%u]:   ", tableName, itemCount );
	if( table ) {
		for( item_I = 0; item_I < itemCount; item_I++ ) {
			Journal_Printf( meshStream, "%.3u ", table[item_I] );
		}
		Journal_Printf( meshStream, "\n" );
	}
	else {
		Journal_Printf( meshStream, "(null)\n");
	}
}


void _Mesh_Print2DTable( Stream* meshStream, Index** table, Index* tableEntryCounts, const Name tableName, Index entryCount )
{
	Index entry_I;
	Index item_I;
	Index itemCount;
	
	Journal_Printf( meshStream, "\t%s (ptr): %p\n", tableName, table );
	if( table && tableEntryCounts ) {
		Journal_Printf( meshStream, "\t%s[0-%u]:\n", tableName, entryCount );
		for( entry_I = 0; entry_I < entryCount; entry_I++ ) {
			itemCount = tableEntryCounts[entry_I];

			Journal_Printf( meshStream, "\t\t%s[%u][0-%u]: ", tableName, entry_I, itemCount );

			for( item_I = 0; item_I < itemCount; item_I++ ) {
				Journal_Printf( meshStream, "%.3u ", table[entry_I][item_I] );
			}
			Journal_Printf( meshStream, "\n" );
		}
	}
}


void* _Mesh_Copy( void* mesh, void* destProc_I, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	Mesh*		self = (Mesh*)mesh;
	Mesh*		newMesh;
	PtrMap*		map = ptrMap;
	Bool		ownMap = False;
	
	if( !map ) {
		map = PtrMap_New( 10 );
		ownMap = True;
	}
	
	newMesh = (Mesh*)_Stg_Component_Copy( self, destProc_I, deep, nameExt, map );
	
	/* Virtual methods */
	newMesh->nodeIsLocal = self->nodeIsLocal;
	newMesh->nodeIsShadow = self->nodeIsShadow;
	newMesh->elementIsLocal = self->elementIsLocal;
	newMesh->elementIsShadow = self->elementIsShadow;
	
	newMesh->dictionary = self->dictionary;
	newMesh->nodeLocalCount = self->nodeLocalCount;
	newMesh->nodeDomainCount = self->nodeDomainCount;
	newMesh->nodeShadowCount = self->nodeShadowCount;
	newMesh->nodeGlobalCount = self->nodeGlobalCount;
	newMesh->buildNodeLocalToGlobalMap = self->buildNodeLocalToGlobalMap;
	newMesh->buildNodeDomainToGlobalMap = self->buildNodeDomainToGlobalMap;
	newMesh->buildNodeGlobalToLocalMap = self->buildNodeGlobalToLocalMap;
	newMesh->buildNodeGlobalToDomainMap = self->buildNodeGlobalToDomainMap;
	newMesh->ownsNodeLocalToGlobalMap = self->ownsNodeLocalToGlobalMap;
	newMesh->ownsNodeDomainToGlobalMap = self->ownsNodeDomainToGlobalMap;
	newMesh->ownsNodeGlobalToLocalMap = self->ownsNodeGlobalToLocalMap;
	newMesh->ownsNodeGlobalToDomainMap = self->ownsNodeGlobalToDomainMap;
	newMesh->buildNodeNeighbourTbl = self->buildNodeNeighbourTbl;
	newMesh->buildNodeElementTbl = self->buildNodeElementTbl;
	newMesh->ownsNodeNeighbourTbl = self->ownsNodeNeighbourTbl;
	newMesh->ownsNodeNeighbourCountTbl = self->ownsNodeNeighbourCountTbl;
	newMesh->ownsNodeElementTbl = self->ownsNodeElementTbl;
	newMesh->ownsNodeElementCountTbl = self->ownsNodeElementCountTbl;
	newMesh->elementLocalCount = self->elementLocalCount;
	newMesh->elementDomainCount = self->elementDomainCount;
	newMesh->elementShadowCount = self->elementShadowCount;
	newMesh->elementGlobalCount = self->elementGlobalCount;
	newMesh->buildElementLocalToGlobalMap = self->buildElementLocalToGlobalMap;
	newMesh->buildElementDomainToGlobalMap = self->buildElementDomainToGlobalMap;
	newMesh->buildElementGlobalToLocalMap = self->buildElementGlobalToLocalMap;
	newMesh->buildElementGlobalToDomainMap = self->buildElementGlobalToDomainMap;
	newMesh->ownsElementLocalToGlobalMap = self->ownsElementLocalToGlobalMap;
	newMesh->ownsElementDomainToGlobalMap = self->ownsElementDomainToGlobalMap;
	newMesh->ownsElementGlobalToLocalMap = self->ownsElementGlobalToLocalMap;
	newMesh->ownsElementGlobalToDomainMap = self->ownsElementGlobalToDomainMap;
	newMesh->buildElementNeighbourTbl = self->buildElementNeighbourTbl;
	newMesh->buildElementNodeTbl = self->buildElementNodeTbl;
	newMesh->ownsElementNeighbourTbl = self->ownsElementNeighbourTbl;
	newMesh->ownsElementNeighbourCountTbl = self->ownsElementNeighbourCountTbl;
	newMesh->ownsElementNodeTbl = self->ownsElementNodeTbl;
	newMesh->ownsElementNodeCountTbl = self->ownsElementNodeCountTbl;
	newMesh->buildTemporaryGlobalTables = self->buildTemporaryGlobalTables;
	
	if( deep ) {
		newMesh->layout = (MeshLayout*)Stg_Class_Copy( self->layout, NULL, deep, nameExt, map );
		newMesh->nodeExtensionMgr = (ExtensionManager*)Stg_Class_Copy( self->nodeExtensionMgr, NULL, deep, nameExt, map );
		newMesh->elementExtensionMgr = (ExtensionManager*)Stg_Class_Copy( self->elementExtensionMgr, NULL, deep, nameExt, map );
		
		/* TODO: Temporary */
		newMesh->node = NULL;
		newMesh->nodeCoord = NULL;
		newMesh->nodeL2G = NULL;
		newMesh->nodeD2G = NULL;
		newMesh->nodeG2L = NULL;
		newMesh->nodeG2D = NULL;
		newMesh->nodeNeighbourTbl = NULL;
		newMesh->nodeNeighbourCountTbl = NULL;
		newMesh->nodeElementTbl = NULL;
		newMesh->nodeElementCountTbl = NULL;
		newMesh->element = NULL;
		newMesh->elementL2G = NULL;
		newMesh->elementD2G = NULL;
		newMesh->elementG2L = NULL;
		newMesh->elementG2D = NULL;
		newMesh->elementNeighbourTbl = NULL;
		newMesh->elementNeighbourCountTbl = NULL;
		newMesh->elementNodeTbl = NULL;
		newMesh->elementNodeCountTbl = NULL;
		newMesh->syncRequestTbl = NULL;
		newMesh->syncElementTbl = NULL;
		newMesh->syncNodeCoordTbl = NULL;
		newMesh->syncNodeTbl = NULL;
		newMesh->syncElementSendTbl = NULL;
		newMesh->syncNodeCoordSendTbl = NULL;
		newMesh->syncNodeSendTbl = NULL;
		
		if( (newMesh->procNbrInfo = PtrMap_Find( map, self->procNbrInfo )) == NULL && self->procNbrInfo ) {
			newMesh->procNbrInfo = Memory_Alloc( ProcNbrInfo, "Mesh->procNbrInfo" );
			memcpy( newMesh->procNbrInfo, self->procNbrInfo, sizeof(ProcNbrInfo) );
		}
		
		if( (newMesh->nodeShadowInfo = PtrMap_Find( map, self->nodeShadowInfo )) == NULL && self->nodeShadowInfo ) {
			newMesh->nodeShadowInfo = Memory_Alloc( ShadowInfo, "Mesh->nodeShadowInfo" );
			memcpy( newMesh->nodeShadowInfo, self->nodeShadowInfo, sizeof(ShadowInfo) );
		}
		
		if( (newMesh->elementShadowInfo = PtrMap_Find( map, self->elementShadowInfo )) == NULL && self->elementShadowInfo ) {
			newMesh->elementShadowInfo = Memory_Alloc( ShadowInfo, "Mesh->elementShadowInfo" );
			memcpy( newMesh->elementShadowInfo, self->elementShadowInfo, sizeof(ShadowInfo) );
		}
		
		/*
		** TODO: copy everything else.  We can leave it off now though, as we know it won't be used
		**	 until copying after the build phase.
		*/
	}
	else {
		newMesh->layout = self->layout;
		newMesh->nodeExtensionMgr = self->nodeExtensionMgr;
		newMesh->elementExtensionMgr = self->elementExtensionMgr;
	}
	
	if( ownMap ) {
		Stg_Class_Delete( map );
	}
	
	return (void*)newMesh;
}


void _Mesh_Build( void* mesh, void* data ) {
	Mesh* self = (Mesh*) mesh;

	/* Run the generator if we have one. */
	if( self->generator )
		Build( self->generator, data, False );

	/* Build Layout */
	Build( self->layout, data, False );

	/* KIND OF HACK: Add the topology to the element layout. */
	self->layout->elementLayout->topo = self->topo;
	
	if ( self->layout->decomp->rank >= self->layout->decomp->procsInUse ) return;

	/* Node counts */
	self->nodeLocalCount = self->layout->decomp->nodeLocalCount;
	self->nodeDomainCount = self->layout->decomp->nodeDomainCount;
	self->nodeShadowCount = self->layout->decomp->nodeShadowCount;
	self->nodeGlobalCount = self->layout->decomp->nodeGlobalCount;

	/* Element counts */
	self->elementDomainCount = self->layout->decomp->elementDomainCount;
	self->elementLocalCount = self->layout->decomp->elementLocalCount;
	self->elementShadowCount = self->layout->decomp->elementShadowCount;
	self->elementGlobalCount = self->layout->decomp->elementGlobalCount;
	
	_Mesh_BuildNodes( self );

	/* this func checks the individual if statements inside it. */
	_Mesh_BuildNodeMaps( self );

	if ( self->buildNodeNeighbourTbl )
		_Mesh_BuildNodeNeighbours( self );
	if ( self->buildNodeElementTbl ) 
		_Mesh_BuildNodeElements( self );
	
	
	/* TODO: should we add the option to not build elements for FD 
	   schemes? will have to think about repercussions */

	_Mesh_BuildElements( self );
	/* Build element tables */
	/* this func checks the individual if statements inside it. */
	_Mesh_BuildElementMaps( self );
	
	if ( self->buildElementNeighbourTbl )
		_Mesh_BuildElementNeighbours( self );
	if ( self->buildElementNodeTbl ) 
		_Mesh_BuildElementNodes( self );
	
	/* build sync tables (note that they will only build if there is shadowDepth > 0 and
	   procsInUse > 1) */
	_Mesh_BuildSyncTables( self );

	ElementLayout_Build( self->layout->elementLayout, self->layout->decomp );

	_Mesh_InitialNodeGeometries( self );
}


void _Mesh_BuildNodes( void* mesh ) {
	Mesh*		self = (Mesh*) mesh;
	
	/* Allocate node array (local node array appended by shadow node arrays) */
	self->node = (Node_List)ExtensionManager_Malloc( self->nodeExtensionMgr, self->nodeDomainCount );
}


void _Mesh_BuildElements( void* mesh ) {
 	Mesh*		self = (Mesh*)mesh;
	
	/* Allocate node array (local node array appended by shadow node arrays) */
	self->element = (Element_List)ExtensionManager_Malloc( self->elementExtensionMgr, self->elementDomainCount );
}


void _Mesh_BuildNodeMaps( void* mesh ) {
	Mesh*		self = (Mesh*) mesh;
	MeshDecomp*     decomp = self->layout->decomp;

	if( self->buildNodeDomainToGlobalMap ) {
		self->nodeD2G = MeshDecomp_BuildNodeDomainToGlobalMap( decomp );
	}
	if( self->buildNodeLocalToGlobalMap ) {
		/* if domain to global built, share pointer */
		if ( self->nodeD2G ) {
			self->nodeL2G = self->nodeD2G;
		}
		else {
			self->nodeL2G = MeshDecomp_BuildNodeLocalToGlobalMap( decomp );
		}	
	}
	if( self->buildNodeGlobalToDomainMap ) {
		self->nodeG2D = MeshDecomp_BuildNodeGlobalToDomainMap( decomp );
	}
	if( self->buildNodeGlobalToLocalMap ) {
		/* if we have build nodeG2D, and no shadowing is active, then
		   nodeG2L will be exactly the same as it - just share the 
		   pointer */
		if ( self->nodeG2D && (0 == self->nodeShadowCount) ) {
			self->nodeG2L = self->nodeG2D;
		}
		else {	
			self->nodeG2L = MeshDecomp_BuildNodeGlobalToLocalMap( decomp );
		}	
	}
}


void _Mesh_BuildNodeNeighbours( void* mesh ) {
	Mesh*		self = (Mesh*)mesh;
	MeshDecomp*     decomp = self->layout->decomp;
	Topology*       topology = self->layout->nodeLayout->topology;
	Node_Index      dNode_I;
	Node_NeighbourIndex neighbourIndex;
	Node_GlobalIndex* nodeD2G = self->nodeD2G;
	Bool nodeD2GBuiltTemporarily = False;
	Bool nodeG2DBuiltTemporarily = False;

	if ( !nodeD2G ) {
		nodeD2G = MeshDecomp_BuildNodeDomainToGlobalMap( decomp );
		nodeD2GBuiltTemporarily = True;
	}

	if ( (self->buildTemporaryGlobalTables == True) && (self->nodeG2D == 0) ) {
		self->nodeG2D = MeshDecomp_BuildNodeGlobalToDomainMap( self->layout->decomp );
		nodeG2DBuiltTemporarily = True;
	}

	self->nodeNeighbourCountTbl = Memory_Alloc_Array( Node_NeighbourIndex, self->nodeDomainCount,
							  "Mesh->nodeNeighbourCountTbl" );
	
	/* Build complex 2D array indices */
	for( dNode_I = 0; dNode_I < self->nodeDomainCount; dNode_I++ ) {

		self->nodeNeighbourCountTbl[dNode_I] = topology->neighbourCount( topology, nodeD2G[dNode_I] );	
	}
	
	self->nodeNeighbourTbl = Memory_Alloc_2DComplex( Node_DomainIndex, self->nodeDomainCount, self->nodeNeighbourCountTbl,
							 "Mesh->nodeNeighbourTbl" );
	
	/* Build neighbour list */
	for( dNode_I = 0; dNode_I < self->nodeDomainCount; dNode_I++ ) {
		
		/* Build neighbour info for global index */
		topology->buildNeighbours( topology, nodeD2G[dNode_I], self->nodeNeighbourTbl[dNode_I] );
		
		/* self->nodeNeighbourTbl currently stores global indices... map to node array indices */
		/* TODO: should change nLayout to store in local indices */
		for( neighbourIndex = 0; neighbourIndex < self->nodeNeighbourCountTbl[dNode_I]; neighbourIndex++ ) {
			self->nodeNeighbourTbl[dNode_I][neighbourIndex] = Mesh_NodeMapGlobalToDomain( self, 
												      self->nodeNeighbourTbl[dNode_I][neighbourIndex] );
		}
	}
	
	if ( nodeD2GBuiltTemporarily == True) {
		Memory_Free( nodeD2G );
		self->nodeD2G = NULL;
	}	
	if ( nodeG2DBuiltTemporarily ) {
		Memory_Free( self->nodeG2D );
		self->nodeG2D = NULL;
	}
}


void _Mesh_BuildNodeElements( void* mesh ) {
	Mesh*		self = (Mesh*)mesh;
	MeshDecomp*     decomp = self->layout->decomp;
	NodeLayout*     nLayout = self->layout->nodeLayout;
	Node_DomainIndex	dNode_I;
	Node_GlobalIndex*	nodeD2G = self->nodeD2G;
	Bool				nodeD2GBuiltTemporarily = False;
	Bool				elementG2DBuiltTemporarily = False;

	Node_ElementIndex elementIndex;

	if ( !nodeD2G ) {
		nodeD2G = MeshDecomp_BuildNodeDomainToGlobalMap( decomp );
		nodeD2GBuiltTemporarily = True;
	}

	if	( (self->buildTemporaryGlobalTables == True) && (self->elementG2D == 0) ) 
	{
		self->elementG2D = MeshDecomp_BuildElementGlobalToDomainMap( self->layout->decomp );
		elementG2DBuiltTemporarily = True;
	}
	
	self->nodeElementCountTbl = Memory_Alloc_Array( Node_ElementIndex, self->nodeDomainCount, "Mesh->nodeElementCountTbl" );

	/* Build counts of nodes per element */
	for( dNode_I = 0; dNode_I < self->nodeDomainCount; dNode_I++ ) {
		self->nodeElementCountTbl[dNode_I] = nLayout->nodeElementCount( nLayout, nodeD2G[dNode_I] );
	}

	self->nodeElementTbl = (Node_ElementsList)Memory_Alloc_2DComplex( Element_DomainIndex,
									  self->nodeDomainCount, self->nodeElementCountTbl, "Mesh->nodeElementTbl" );
	
	/* Build element list */
	for( dNode_I = 0; dNode_I < self->nodeDomainCount; dNode_I++ ) {
		/* Build element info for global index */

		nLayout->buildNodeElements( nLayout, nodeD2G[dNode_I], self->nodeElementTbl[dNode_I] );
		
		/* self->nodeElementTbl currently stores global indices... map to node array indices */
		for( elementIndex = 0; elementIndex < self->nodeElementCountTbl[dNode_I]; elementIndex++ ) {
			self->nodeElementTbl[dNode_I][elementIndex] = Mesh_ElementMapGlobalToDomain( self, 
												     self->nodeElementTbl[dNode_I][elementIndex] );
		}
	}
	
	if ( nodeD2GBuiltTemporarily ) {
		Memory_Free( nodeD2G );
		self->nodeD2G = NULL;
	}
	
	if ( elementG2DBuiltTemporarily == True ) {
		Memory_Free( self->elementG2D );
		self->elementG2D = NULL;
	}
}


void _Mesh_BuildElementMaps( void* mesh ) {
	Mesh*		self = (Mesh*) mesh;
	MeshDecomp*  decomp = self->layout->decomp;

	if( self->buildElementDomainToGlobalMap ) {
		self->elementD2G = MeshDecomp_BuildElementDomainToGlobalMap( decomp );
	}
	if( self->buildElementLocalToGlobalMap ) {
		/* if domain to global built, share pointer */
		if ( self->elementD2G ) {
			self->elementL2G = self->elementD2G;
		}
		else {
			self->elementL2G = MeshDecomp_BuildElementLocalToGlobalMap( decomp );
		}	
	}
	if( self->buildElementGlobalToDomainMap ) {
		self->elementG2D = MeshDecomp_BuildElementGlobalToDomainMap( decomp );
	}
	if( self->buildElementGlobalToLocalMap ) {
		/* if we have build elementG2D, and no shadowing is active, then
		   elementG2L will be exactly the same as it - just share the 
		   pointer */
		if ( self->elementG2D && (0 == self->elementShadowCount) ) {
			self->elementG2L = self->elementG2D;
		}
		else {	
			self->elementG2L = MeshDecomp_BuildElementGlobalToLocalMap( decomp );
		}	
	}
}


void _Mesh_BuildElementNeighbours( void* mesh ) {
	Mesh*		self = (Mesh*)mesh;
	MeshDecomp*     decomp = self->layout->decomp;
	Topology*       topology = self->layout->elementLayout->topology;
	Element_Index      dElement_I;
	Element_NeighbourIndex neighbourIndex;
	Element_GlobalIndex* elementD2G = self->elementD2G;
	Bool elementD2GBuiltTemporarily = False;
	Bool elementG2DBuiltTemporarily = False;

	if ( !elementD2G ) {
		elementD2G = MeshDecomp_BuildElementDomainToGlobalMap( decomp );
		elementD2GBuiltTemporarily = True;
	}

	if	( (self->buildTemporaryGlobalTables == True) && (self->elementG2D == 0) ) 
	{
		self->elementG2D = MeshDecomp_BuildElementGlobalToDomainMap( self->layout->decomp );
		elementG2DBuiltTemporarily = True;
	}
	self->elementNeighbourCountTbl = Memory_Alloc_Array( Element_NeighbourIndex, self->elementDomainCount,
							     "Mesh->elementNeighbourCountTbl" );
	/* Build complex 2D array indices */
	for( dElement_I = 0; dElement_I < self->elementDomainCount; dElement_I++ ) {
		self->elementNeighbourCountTbl[dElement_I] = topology->neighbourCount( topology, elementD2G[dElement_I] );	
	}
	
	self->elementNeighbourTbl = Memory_Alloc_2DComplex( Element_DomainIndex, self->elementDomainCount,
							    self->elementNeighbourCountTbl, "Mesh->elementNeighbourTbl" );
	
	/* Build neighbour list */
	for( dElement_I = 0; dElement_I < self->elementDomainCount; dElement_I++ ) {
		
		/* Build neighbour info for global index */
		topology->buildNeighbours( topology, elementD2G[dElement_I], self->elementNeighbourTbl[dElement_I] );
		
		/* self->elementNeighbourTbl currently stores global indices... map to domain array indices */
		/* TODO: should change eLayout to store in local indices */
		for( neighbourIndex = 0; neighbourIndex < self->elementNeighbourCountTbl[dElement_I]; neighbourIndex++ ) {
			self->elementNeighbourTbl[dElement_I][neighbourIndex] = Mesh_ElementMapGlobalToDomain( self, 
													       self->elementNeighbourTbl[dElement_I][neighbourIndex] );
		}
	}
	
	if ( elementD2GBuiltTemporarily == True) {
		Memory_Free( elementD2G );
		self->elementD2G = NULL;
	}	

	if ( elementG2DBuiltTemporarily == True ) {
		Memory_Free( self->elementG2D );
		self->elementG2D = NULL;
	}
}


void _Mesh_BuildElementNodes( void* mesh ) {
	Mesh*			self = (Mesh*)mesh;
	MeshDecomp*		decomp = self->layout->decomp;
	NodeLayout*		nLayout = self->layout->nodeLayout;
	Element_DomainIndex	dElement_I;
	Element_GlobalIndex*	elementD2G = self->elementD2G;
	Bool			elementD2GBuiltTemporarily = False;
	Bool			nodeG2DBuiltTemporarily = False;

	Element_NodeIndex nodeIndex;

	if ( !elementD2G ) {
		elementD2G = MeshDecomp_BuildElementDomainToGlobalMap( decomp );
		elementD2GBuiltTemporarily = True;
	}
	
	if ( (self->buildTemporaryGlobalTables == True) && (self->nodeG2D == 0) ) {
		self->nodeG2D = MeshDecomp_BuildNodeGlobalToDomainMap( self->layout->decomp );
		nodeG2DBuiltTemporarily = True;
	}

	self->elementNodeCountTbl = Memory_Alloc_Array( Element_NodeIndex, self->elementDomainCount, "Mesh->elementNodeCountTbl" );

	/* Build counts of nodes per element */
	for( dElement_I = 0; dElement_I < self->elementDomainCount; dElement_I++ ) {
		self->elementNodeCountTbl[dElement_I] = nLayout->elementNodeCount( nLayout, elementD2G[dElement_I] );
	}

	self->elementNodeTbl = Memory_Alloc_2DComplex( Node_DomainIndex,
						       self->elementDomainCount, self->elementNodeCountTbl, "Mesh->elementNodeTbl" );
	
	/* Build element list */
	for( dElement_I = 0; dElement_I < self->elementDomainCount; dElement_I++ ) {
		/* Build element info for global index */

		nLayout->buildElementNodes( nLayout, elementD2G[dElement_I], self->elementNodeTbl[dElement_I] );
		
		/* self->elementNodesTbl currently stores global indices... map to element array indices */
		for( nodeIndex = 0; nodeIndex < self->elementNodeCountTbl[dElement_I]; nodeIndex++ ) {
			self->elementNodeTbl[dElement_I][nodeIndex] = Mesh_NodeMapGlobalToDomain( self, 
												  self->elementNodeTbl[dElement_I][nodeIndex] );
		}
	}
	
	if ( elementD2GBuiltTemporarily ) {
		Memory_Free( elementD2G );
		self->elementD2G = NULL;
	}

	if ( nodeG2DBuiltTemporarily ) {
		Memory_Free( self->nodeG2D );
		self->nodeG2D = NULL;
	}
}


void _Mesh_Initialise( void* mesh, void* data ) {
	
}


void _Mesh_Execute( void* mesh, void* data ) {
	/* Do nothing */
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/


Node_GlobalIndex Mesh_NodeMapLocalToGlobal( void* mesh, Node_LocalIndex localIndex ) {
	Mesh* self = (Mesh*)mesh;
	
	if ( localIndex >= self->nodeLocalCount )
		return Mesh_Node_Invalid( self );

	if( self->nodeL2G ) {
		return self->nodeL2G[localIndex];
	}
	else {
		MeshDecomp*     decomp = self->layout->decomp;
		return decomp->nodeMapLocalToGlobal( decomp, localIndex );
	}
}


Node_GlobalIndex Mesh_NodeMapShadowToGlobal( void* mesh, Node_ShadowIndex shadowIndex ) {
	Mesh* self = (Mesh*)mesh;
	
	if ( shadowIndex >= self->nodeShadowCount ) {
		return Mesh_Node_Invalid( self );
	}	
	else if (self->nodeD2G ) {
		return self->nodeD2G[self->nodeLocalCount+shadowIndex];
	}
	else {
		MeshDecomp*     decomp = self->layout->decomp;
		return decomp->nodeMapShadowToGlobal( self, shadowIndex );
	}
}


Node_GlobalIndex Mesh_NodeMapDomainToGlobal( void* mesh, Node_DomainIndex domainIndex ) {
	Mesh* self = (Mesh*)mesh;
	
	if ( domainIndex >= self->nodeDomainCount ) {
		return Mesh_Element_Invalid( self );
	}	
	else if (self->nodeD2G ) {
		return self->nodeD2G[domainIndex];
	}
	else {
		MeshDecomp*     decomp = self->layout->decomp;
		return decomp->nodeMapDomainToGlobal( decomp, domainIndex );
	}
}


Node_LocalIndex Mesh_NodeMapGlobalToLocal( void* mesh, Node_GlobalIndex globalIndex ) {
	Mesh*		self = (Mesh*)mesh;
	
	if ( globalIndex >= self->nodeGlobalCount ) 
	{	
		return Mesh_Node_Invalid( self );
	}	
	else if( self->nodeG2L ) {
		return self->nodeG2L[globalIndex];
	}
	else {
		MeshDecomp*     decomp = self->layout->decomp;
		return decomp->nodeMapGlobalToLocal( decomp, globalIndex);
	}
}


Node_DomainIndex Mesh_NodeMapGlobalToDomain( void* mesh, Node_GlobalIndex globalIndex ) {
	Mesh* self = (Mesh*)mesh;
	
	if( globalIndex >= self->nodeGlobalCount ) {
		return Mesh_Node_Invalid( self );
	}	
	else if ( self->nodeG2D ) {
		return self->nodeG2D[globalIndex];
	}
	else {
		MeshDecomp*     decomp = self->layout->decomp;
		return decomp->nodeMapGlobalToDomain( decomp, globalIndex);
	}
}


Node_ShadowIndex Mesh_NodeMapGlobalToShadow( void* mesh, Node_GlobalIndex globalIndex ) {
	Mesh* self = (Mesh*)mesh;
	
	if ( ( globalIndex >= self->nodeGlobalCount ) || 
	     (0 == self->nodeShadowCount ) ) 
	{	
		return Mesh_Node_Invalid( self );
	}	
	else if ( self->nodeG2D ) {
		Node_DomainIndex dNode_I;
		dNode_I = self->nodeG2D[globalIndex];
		if ( (dNode_I != MD_N_Invalid( self )) && (dNode_I >= self->nodeLocalCount ) ) {
			return dNode_I - self->nodeLocalCount;
		}
	}
	else {
		MeshDecomp*     decomp = self->layout->decomp;
		return decomp->nodeMapGlobalToShadow( decomp, globalIndex );
	}
	
	return Mesh_Node_Invalid( self );
}


#define M_N_IsL1D( self, localIndex )				\
	((localIndex) < (self)->nodeLocalCount ? True : False)

Bool _Mesh_Node_IsLocal1D( void* mesh, Node_LocalIndex localIndex ) {
	return M_N_IsL1D( (Mesh*)mesh, localIndex );
}


#define M_N_IsS1D( self, shadowIndex )					\
	((shadowIndex) < (self)->nodeShadowCount ? True : False)

Bool _Mesh_Node_IsShadow1D( void* mesh, Node_ShadowIndex shadowIndex ) {
	return M_N_IsS1D( (Mesh*)mesh, shadowIndex );
}


/* Element virtual functions */

Element_GlobalIndex Mesh_ElementMapLocalToGlobal( void* mesh, Element_LocalIndex localIndex ) {
	Mesh* self = (Mesh*)mesh;
	
	if ( localIndex >= self->elementLocalCount )
		return Mesh_Element_Invalid( self );

	if( self->elementL2G ) {
		return self->elementL2G[localIndex];
	}
	else {
		MeshDecomp*     decomp = self->layout->decomp;
		return decomp->elementMapLocalToGlobal( decomp, localIndex );
	}
}


Element_GlobalIndex Mesh_ElementMapShadowToGlobal( void* mesh, Element_ShadowIndex shadowIndex ) {
	Mesh* self = (Mesh*)mesh;
	
	if ( shadowIndex >= self->elementShadowCount ) {
		return Mesh_Element_Invalid( self );
	}	
	else if (self->elementD2G ) {
		return self->elementD2G[self->elementLocalCount+shadowIndex];
	}
	else {
		MeshDecomp*     decomp = self->layout->decomp;
		return decomp->elementMapShadowToGlobal( self, shadowIndex );
	}
}


Element_GlobalIndex Mesh_ElementMapDomainToGlobal( void* mesh, Element_DomainIndex domainIndex ) {
	Mesh* self = (Mesh*)mesh;
	
	if ( domainIndex >= self->elementDomainCount ) {
		return Mesh_Element_Invalid( self );
	}	
	else if (self->elementD2G ) {
		return self->elementD2G[domainIndex];
	}
	else {
		MeshDecomp*     decomp = self->layout->decomp;
		return decomp->elementMapDomainToGlobal( decomp, domainIndex );
	}
}


Element_LocalIndex Mesh_ElementMapGlobalToLocal( void* mesh, Element_GlobalIndex globalIndex ) {
	Mesh*		self = (Mesh*)mesh;
	
	if ( globalIndex >= self->elementGlobalCount ) 
	{	
		return Mesh_Element_Invalid( self );
	}	
	else if( self->elementG2L ) {
		return self->elementG2L[globalIndex];
	}
	else {
		MeshDecomp*     decomp = self->layout->decomp;
		return decomp->elementMapGlobalToLocal( decomp, globalIndex);
	}
}


Element_DomainIndex Mesh_ElementMapGlobalToDomain( void* mesh, Element_GlobalIndex globalIndex ) {
	Mesh* self = (Mesh*)mesh;
	
	if( globalIndex >= self->elementGlobalCount ) {
		return Mesh_Element_Invalid( self );
	}	
	else if ( self->elementG2D ) {
		return self->elementG2D[globalIndex];
	}
	else {
		MeshDecomp*     decomp = self->layout->decomp;
		return decomp->elementMapGlobalToDomain( decomp, globalIndex);
	}
}


Element_ShadowIndex Mesh_ElementMapGlobalToShadow( void* mesh, Element_GlobalIndex globalIndex ) {
	Mesh* self = (Mesh*)mesh;
	
	if ( ( globalIndex >= self->elementGlobalCount ) || 
	     (0 == self->elementShadowCount ) ) 
	{	
		return Mesh_Element_Invalid( self );
	}	
	else if ( self->elementG2D ) {
		Element_DomainIndex dElement_I;
		dElement_I = self->elementG2D[globalIndex];
		if ( (dElement_I != MD_E_Invalid( self )) && (dElement_I >= self->elementLocalCount ) ) {
			return dElement_I - self->elementLocalCount;
		}
	}
	else {
		MeshDecomp*     decomp = self->layout->decomp;
		return decomp->elementMapGlobalToShadow( decomp, globalIndex );
	}
	
	return Mesh_Element_Invalid( self );
}


#define M_E_IsL1D( self, localIndex )					\
	(((localIndex) < (self)->nodeLocalCount) ? True : False)

Bool _Mesh_Element_IsLocal1D( void* mesh, Element_LocalIndex localIndex ) {
	return M_E_IsL1D( (Mesh*)mesh, localIndex );
}


#define M_E_IsS1D( self, shadowIndex )					\
	(((shadowIndex) < (self)->nodeShadowCount) ? True : False)

Bool _Mesh_Element_IsShadow1D( void* mesh, Element_ShadowIndex shadowIndex ) {
	return M_E_IsS1D( (Mesh*)mesh, shadowIndex );
}




double* Mesh_CoordAtFunc( Mesh* self, Node_DomainIndex node_dI ) {
	return Mesh_CoordAtMacro( self, node_dI );
}

Node* Mesh_NodeAtFunc( Mesh* self, Node_DomainIndex node_dI ) {
	return Mesh_NodeAtMacro( self, node_dI );
}

Element* Mesh_ElementAtFunc( Mesh* self, Element_DomainIndex element_dI ) {
	return Mesh_ElementAtMacro( self, element_dI );
}


void Mesh_GetNodeCoordPtrsOfElement( void* mesh, Element_DomainIndex element_dI, Coord** nodeCoordPtrs ) {
	Mesh*			self = (Mesh*)mesh;
	Node_ElementLocalIndex	node_elLocalI = 0;
	Node_ElementLocalIndex	currElNodeCount = 0;

	currElNodeCount = self->elementNodeCountTbl[element_dI];
	
	for ( node_elLocalI=0; node_elLocalI < currElNodeCount; node_elLocalI++ ) {
		nodeCoordPtrs[node_elLocalI] = &( Mesh_CoordAt( self, self->elementNodeTbl[element_dI][node_elLocalI] ));
	}
}


void Mesh_PrintNodeCoordsOfElement( void* mesh, Element_DomainIndex element_dI, Stream* stream ) {
	Mesh*			self = (Mesh*)mesh;
	Node_ElementLocalIndex	node_elLocalI = 0;
	Node_ElementLocalIndex	currElementNodeCount = 0;
	Coord**			coordPtrs = Memory_Alloc_Array_Unnamed( Coord*, self->elementNodeCountTbl[element_dI] );
	Coord*			coordPtr = NULL;

	currElementNodeCount = self->elementNodeCountTbl[element_dI];
	Mesh_GetNodeCoordPtrsOfElement( self, element_dI, coordPtrs );

	for ( node_elLocalI = 0; node_elLocalI < currElementNodeCount; node_elLocalI++ ) {
		coordPtr = coordPtrs[node_elLocalI];
		Journal_Printf( stream, "(%.2f,%.2f,%.2f), ", (*coordPtr)[0], (*coordPtr)[1],
				(*coordPtr)[2] );
	}
	Journal_Printf( stream, "\n" );
	Memory_Free( coordPtrs );
}


unsigned Mesh_ClosestNode( void* _self, Coord crd ) {
	Mesh*		self = (Mesh*)_self;
	Bool		done;
	Coord*		nodeCrds = self->nodeCoord;
	unsigned	curNode;
	unsigned	nDims = self->nSpaceDims;
	double		dist, tmp;
	unsigned	d_i;

	/* Must have neighbours enabled. */
	assert( self->buildNodeNeighbourTbl && self->nodeNeighbourTbl );

	/* Begin somewhere in the middle. */
	curNode = self->nodeDomainCount / 2;

	/* Calc distance squared to current node. */
	tmp = nodeCrds[curNode][0] - crd[0];
	dist = tmp * tmp;
	for( d_i = 1; d_i < nDims; d_i++ ) {
		tmp = nodeCrds[curNode][d_i] - crd[d_i];
		dist += tmp * tmp;
	}

	/* Loop until we've found closest local node. */
	do {
		unsigned	nNbrs = self->nodeNeighbourCountTbl[curNode];
		unsigned*	nbrs = self->nodeNeighbourTbl[curNode];
		unsigned	nbr_i;

		/* Assume we'll be done after this loop. */
		done = True;

		/* Compare to neighbours. */
		for( nbr_i = 0; nbr_i < nNbrs; nbr_i++ ) {
			double	nbrDist;

			/* Skip invalid neighbours. */
			if( nbrs[nbr_i] >= self->nodeDomainCount )
				continue;

			tmp = nodeCrds[nbrs[nbr_i]][0] - crd[0];
			nbrDist = tmp * tmp;
			for( d_i = 1; d_i < nDims; d_i++ ) {
				tmp = nodeCrds[nbrs[nbr_i]][d_i] - crd[d_i];
				nbrDist += tmp * tmp;
			}

			if( nbrDist < dist ) {
				curNode = nbrs[nbr_i];
				dist = nbrDist;
				done = False;
			}
		}
	}
	while( !done );

	return curNode;
}


double Mesh_NodeSeparation( Mesh* self ) {
	double		sep = HUGE_VAL;
	unsigned	e_i;

	assert( self );

	for( e_i = 0; e_i < self->elementDomainCount; e_i++ ) {
		unsigned	en_i;

		for( en_i = 0; en_i < self->elementNodeCountTbl[e_i]; en_i++ ) {
			unsigned	nIndI = self->elementNodeTbl[e_i][en_i];
			unsigned	en_j;

			for( en_j = en_i + 1; en_j < self->elementNodeCountTbl[e_i]; en_j++ ) {
				unsigned	nIndJ = self->elementNodeTbl[e_i][en_j];
				Coord		tmp;
				double		curSep;

				Vector_Sub( tmp, self->nodeCoord[nIndI], self->nodeCoord[nIndJ] );
				curSep = Vector_Mag( tmp );
				if( curSep < sep ) sep = curSep;
			}
		}
	}

	return sep;
}


unsigned Mesh_ElementWithPoint( void* mesh, Coord pnt, PartitionBoundaryStatus bndStat ) {
	Mesh*		self = (Mesh*)mesh;
	ElementLayout*	eLyt = self->layout->elementLayout;

	/* If using ParallelPipedHexaEL, just run it as it's bloody quick. */
	if( eLyt->type == ParallelPipedHexaEL_Type )
		return eLyt->elementWithPoint( eLyt, self->layout->decomp, pnt, self, bndStat, 0, NULL );
	else if( self->nodeNeighbourTbl ) {
		unsigned	closeNode;
		unsigned	elInd;

		/* If node neighbour table is built, use it to speed things up. */
		closeNode = Mesh_ClosestNode( self, pnt );
		elInd = eLyt->elementWithPoint( eLyt, self->layout->decomp, pnt, self, bndStat, 
						self->nodeElementCountTbl[closeNode], self->nodeElementTbl[closeNode] );
		if( elInd >= self->elementDomainCount )
			elInd = eLyt->elementWithPoint( eLyt, self->layout->decomp, pnt, self, bndStat, 0, NULL );

		return elInd;
	}
	else
		return eLyt->elementWithPoint( eLyt, self->layout->decomp, pnt, self, bndStat, 0, NULL );
}


#define tagElementExt	300
#define tagNodeCoord	301
#define tagNodeExt	302

void Mesh_Sync( void* mesh ) {
	Mesh*			self = (Mesh*)mesh;
	Neighbour_Index		nbr_I;
	
	assert( self );

#if 0
	if( self->layout->decomp->shadowDepth == 0 || self->layout->decomp->procsInUse == 1 ) {
		printf( "Warning: attempting to sync a mesh under inappropriate circumstances (either\
			shadowDepth = 0 or only one CPU in use)." );
		return;
	}
#endif
	
	
	/*
	 * Initiate non-blocking receives for all data.
	 */
	
	for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
		NeighbourIndex			srcProc_I = self->procNbrInfo->procNbrTbl[nbr_I];
		MPI_Comm			comm = self->layout->decomp->communicator;
		SizeT				size;
		
		if( self->elementShadowInfo->procShadowCnt[nbr_I] ) {
			unsigned			finalSize = self->elementExtensionMgr->finalSize;
			
			size = finalSize * self->elementShadowInfo->procShadowCnt[nbr_I];
			MPI_Irecv( self->syncElementTbl[nbr_I], size, MPI_BYTE, srcProc_I, tagElementExt, comm, 
				   &self->syncRequestTbl[nbr_I][0] );
		}
		
		if( self->nodeShadowInfo->procShadowCnt[nbr_I] ) {
			unsigned				finalSize = self->nodeExtensionMgr->finalSize;
			
			/* receive the node coords */
			size = sizeof(Coord) * self->nodeShadowInfo->procShadowCnt[nbr_I];
			MPI_Irecv( self->syncNodeCoordTbl[nbr_I], size, MPI_BYTE, srcProc_I, tagNodeCoord, comm,
				   &self->syncRequestTbl[nbr_I][1] );
			
			/* recieve the nodes */
			size = finalSize * self->nodeShadowInfo->procShadowCnt[nbr_I];
			MPI_Irecv( self->syncNodeTbl[nbr_I], size, MPI_BYTE, srcProc_I, tagNodeExt, comm,
				   &self->syncRequestTbl[nbr_I][2] );
		}
	}

	
	/*
	 * Send node coordinates, node extension info and element extension info to neighbouring procs
	 */
	 
	for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
		PartitionIndex			destProc_I = self->procNbrInfo->procNbrTbl[nbr_I];
		MPI_Comm			comm = self->layout->decomp->communicator;
		SizeT				size;
		
		/* send element extensions */
		if( self->elementShadowInfo->procShadowedCnt[nbr_I] ) {
			Element_LocalIndex		element_lI;
			unsigned			finalSize = self->elementExtensionMgr->finalSize;
			
			for( element_lI = 0; element_lI < self->elementShadowInfo->procShadowedCnt[nbr_I]; element_lI++ ) {
				Element_LocalIndex			ncIdx;
				
				ncIdx =	self->elementShadowInfo->procShadowedTbl[nbr_I][element_lI];
				
				memcpy( (unsigned char*)self->syncElementSendTbl + finalSize * element_lI, 
					(unsigned char*)self->element + finalSize * ncIdx, finalSize );
			}

			size = finalSize * self->elementShadowInfo->procShadowedCnt[nbr_I];
			MPI_Send( self->syncElementSendTbl, size, MPI_BYTE, destProc_I, tagElementExt, comm );
		}

		/* move the node coords intended for nbr_I into the array */
		if( self->nodeShadowInfo->procShadowedCnt[nbr_I] ) {
			Node_LocalIndex			node_lI;
			unsigned			finalSize = self->nodeExtensionMgr->finalSize;
			
			for( node_lI = 0; node_lI < self->nodeShadowInfo->procShadowedCnt[nbr_I]; node_lI++ ) {
				Node_LocalIndex			ncIdx;
				
				ncIdx =	self->nodeShadowInfo->procShadowedTbl[nbr_I][node_lI];
				
				self->syncNodeCoordSendTbl[node_lI][0] = self->nodeCoord[ncIdx][0];
				self->syncNodeCoordSendTbl[node_lI][1] = self->nodeCoord[ncIdx][1];
				self->syncNodeCoordSendTbl[node_lI][2] = self->nodeCoord[ncIdx][2];
				
				memcpy( (unsigned char*)self->syncNodeSendTbl + finalSize * node_lI, 
					(unsigned char*)self->node + finalSize * ncIdx, finalSize );
			}
			
			size = sizeof(Coord) * self->nodeShadowInfo->procShadowedCnt[nbr_I];
			MPI_Send( self->syncNodeCoordSendTbl, size, MPI_BYTE, destProc_I, tagNodeCoord, comm );
			
			size = finalSize * self->nodeShadowInfo->procShadowedCnt[nbr_I];
			MPI_Send( self->syncNodeSendTbl, size, MPI_BYTE, destProc_I, tagNodeExt, comm );
		}
	}
	
	
	/*
	 * Receive node coordinates, node extension, blah blah blah
	 */
	
	for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
		MPI_Status			status;
		
		/* grab the element extension */
		if( self->elementShadowInfo->procShadowCnt[nbr_I] ) {
			Element_ShadowIndex			element_sI;
			unsigned				finalSize = self->elementExtensionMgr->finalSize;
			
			/* recieve the elements */
			MPI_Wait( &self->syncRequestTbl[nbr_I][0], &status );
			
			for( element_sI = 0; element_sI < self->elementShadowInfo->procShadowCnt[nbr_I]; element_sI++ ) {
				Element_ShadowIndex			ncIdx;
				
				ncIdx = self->elementShadowInfo->procShadowTbl[nbr_I][element_sI];
				
				memcpy( (unsigned char*)self->element + finalSize * ncIdx,
					(unsigned char*)self->syncElementTbl[nbr_I] + finalSize * element_sI, finalSize );
			}
		}
		
		if( self->nodeShadowInfo->procShadowCnt[nbr_I] ) {
			Node_ShadowIndex			node_sI;
			unsigned				finalSize = self->nodeExtensionMgr->finalSize;
			
			/* receive the node coords */
			MPI_Wait( &self->syncRequestTbl[nbr_I][1], &status );
			
			/* recieve the nodes */
			MPI_Wait( &self->syncRequestTbl[nbr_I][2], &status );
			
			for( node_sI = 0; node_sI < self->nodeShadowInfo->procShadowCnt[nbr_I]; node_sI++ ) {
				Node_ShadowIndex			ncIdx;
				
				ncIdx = self->nodeShadowInfo->procShadowTbl[nbr_I][node_sI];
				
				self->nodeCoord[ncIdx][0] = self->syncNodeCoordTbl[nbr_I][node_sI][0];
				self->nodeCoord[ncIdx][1] = self->syncNodeCoordTbl[nbr_I][node_sI][1];
				self->nodeCoord[ncIdx][2] = self->syncNodeCoordTbl[nbr_I][node_sI][2];
				
				memcpy( (unsigned char*)self->node + finalSize * ncIdx,
					(unsigned char*)self->syncNodeTbl[nbr_I] + finalSize * node_sI, finalSize );
			}
		}
	}
}



/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

void _Mesh_InitialNodeGeometries( void* mesh ) {
	Mesh*			self = (Mesh*)mesh;
	NodeLayout*		nLayout = self->layout->nodeLayout;
	Node_DomainIndex	index;
	
	/* If we're using the new generation methods, skip this bit. */
	if( !self->nodeCoord ) {
		self->nodeCoord = Memory_Alloc_Array( Coord, self->nodeDomainCount, "Mesh->nodeCoord" );
		for( index = 0;  index < self->nodeDomainCount; index++ ) {
			Node_GlobalIndex globalIndex;
		
			globalIndex = Mesh_NodeMapDomainToGlobal( self, index );
			nLayout->nodeCoordAt( nLayout, globalIndex, self->nodeCoord[index] );
		}
	}
	else {
		Coord*		newCoords;
		unsigned	n_i;

		newCoords = Memory_Alloc_Array( Coord, self->nodeDomainCount, "Mesh->nodeCoord" );
		for( n_i = 0; n_i < MeshTopology_GetDomainSize( self->topo, MT_VERTEX ); n_i++ ) {
			unsigned	dInd;

			dInd = MeshTopology_DomainToGlobal( self->topo, MT_VERTEX, n_i );
			dInd = Mesh_NodeMapGlobalToDomain( self, dInd );
			memcpy( newCoords[dInd], self->nodeCoord[n_i], sizeof(Coord) );
		}
		FreeArray( self->nodeCoord );
		self->nodeCoord = newCoords;
	}

	/* After we collect initial node geometries, we need to flag the element geometry so. */
	if( !strcmp( self->layout->elementLayout->geometry->type, MeshGeometry_Type ) ) {
		((MeshGeometry*)self->layout->elementLayout->geometry)->useMesh = True;
	}
}


void _Mesh_BuildSyncTables( void* mesh ) {
	Mesh*			self = (Mesh*)mesh;
	
	assert( self );
	
	if( self->layout->decomp->shadowDepth > 0 && self->layout->decomp->procsInUse > 1 ) {
		MeshDecomp*		decomp = self->layout->decomp;
		MPI_Comm		comm = decomp->communicator;
		PartitionIndex		myRank = decomp->rank;
		Topology*		procTop = decomp->procTopology;
		NeighbourIndex		nbrCnt;
		NeighbourIndex*		tempNbrArray = NULL;
		NeighbourIndex		nbr_I;
		NeighbourIndex		prevNbr_I;
		Element_ShadowIndex*	elementTbl;
		Element_ShadowIndex	element_sI;
		Element_LocalIndex	element_lI;
		Node_ShadowIndex*	nodeTbl;
		Node_ShadowIndex	node_sI;
		Node_LocalIndex		node_lI;
		Index                   maxElementCnt;
                Index                   maxNodeCnt;
		MPI_Request*		request;
		Bool			nodeG2LBuiltTemporarily = False;
		Bool			nodeG2DBuiltTemporarily = False;
		Bool			elementG2LBuiltTemporarily = False;
		Bool			elementG2DBuiltTemporarily = False;
		Stream*                 errorStream = Journal_Register( Error_Type, self->type );

		Journal_DPrintf( self->debug, "In %s(), Proc %d:\n", __func__, myRank );
		Stream_Indent( self->debug );

		if (self->buildTemporaryGlobalTables == True) {
			if (self->nodeG2L == NULL) {
				self->nodeG2L = MeshDecomp_BuildNodeGlobalToLocalMap( self->layout->decomp );
				nodeG2LBuiltTemporarily = True;
			}	
			if (self->nodeG2D == NULL) {
				self->nodeG2D = MeshDecomp_BuildNodeGlobalToDomainMap( self->layout->decomp );
				nodeG2DBuiltTemporarily = True;
			}	
			if (self->elementG2L == NULL) {
				self->elementG2L = MeshDecomp_BuildElementGlobalToLocalMap( self->layout->decomp );
				elementG2LBuiltTemporarily = True;
			}	
			if (self->elementG2D == NULL) {
				self->elementG2D = MeshDecomp_BuildElementGlobalToDomainMap( self->layout->decomp );
				elementG2DBuiltTemporarily = True;
			}	
		}

		/* Build the set of valid proc neighbours */
		/* modified by PatrickSunter, 16 May 2006 :
		   For periodic problems the nbrs aren't necessarily unique, so we need to check
		   to avoid adding the same one twice.
		   
		   Also, previously this part used to check for but silently ignore
		   nbrs pointing to inactive processors, but this is no good as it has to
		   be the responsibility of the meshDecomp to adjust its own procTopology if there are
		   inactive processors, and return adjusted nbrs accordingly - since otherwise things 
		   such as periodicity may not work.
		*/
		nbrCnt = procTop->neighbourCount( procTop, myRank );
		tempNbrArray = Memory_Alloc_Array( NeighbourIndex, nbrCnt, "tempNbrArray" );
		/* Just allocate the procNbrTbl to be the same size, even if there might be duplicates:
		   doesn't hurt to have a few blank spots in the array so long as count is correct */
		self->procNbrInfo->procNbrTbl = Memory_Alloc_Array( NeighbourIndex, nbrCnt, "Mesh->procNbrTbl" );
		procTop->buildNeighbours( procTop, myRank, tempNbrArray );

#if DEBUG
		if ( 0 == strcmp( IJK26Topology_Type, procTop->type ) ) {
			Journal_DPrintf( self->debug, "Calculated my neighbour processor are:\n" );
			Stream_Indent( self->debug );
			((IJK26Topology*)procTop)->dynamicSizes = False;
			IJK26Topology_PrintNeighboursOfIndex( (IJK26Topology*)procTop, myRank, self->debug );
			((IJK26Topology*)procTop)->dynamicSizes = True;
			Stream_UnIndent( self->debug );
		}
#endif
		Stream_Indent( self->debug );
		self->procNbrInfo->procNbrCnt = 0;
		/* Ok, now search through the temp list, and only add the unique Ids to our real list */
		for( nbr_I = 0; nbr_I < nbrCnt; nbr_I++ ) {

			Journal_Firewall( tempNbrArray[nbr_I] < decomp->procsInUse, errorStream,
					  "Error - in %s() on proc %u: nbr %u in my ProcNbrInfo mapped "
					  "to processor ID %u, which is >= calculated "
					  "procs in use %u! Check the decomp info.\n",
					  __func__, myRank, nbr_I, tempNbrArray[nbr_I], decomp->procsInUse );
			
			/* Check the current candidate in temp list for equality against prev acceptances */
			for ( prevNbr_I = 0; prevNbr_I < self->procNbrInfo->procNbrCnt; prevNbr_I++ ) {
				if ( self->procNbrInfo->procNbrTbl[prevNbr_I] == tempNbrArray[nbr_I] ) 
				{	
					Journal_DPrintfL( self->debug, 2, "In %s() on proc %u: nbr %u in my "
							  "ProcNbrInfo mapped to proc ID %u, but have already added that "
							  "processor to unique nbr list - ignoring duplicate.\n",
							  __func__, myRank, nbr_I, tempNbrArray[nbr_I] );
					break;	
				}	
			}
			/* Only if we successfully checked each prev should we accept the new one - otherwise
			   let it get over-written by not incrementing the count */
			if ( prevNbr_I == self->procNbrInfo->procNbrCnt ) {
				self->procNbrInfo->procNbrTbl[self->procNbrInfo->procNbrCnt] = tempNbrArray[nbr_I];
				self->procNbrInfo->procNbrCnt++;
			}
		}
		Stream_UnIndent( self->debug );
		Memory_Free( tempNbrArray );

#if DEBUG
		Journal_DPrintf( self->debug, "Calculated my unique list of %u nbr procs is:\n\t{",
				 self->procNbrInfo->procNbrCnt );
		for ( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			Journal_DPrintf( self->debug, "%u, ", self->procNbrInfo->procNbrTbl[nbr_I] );
		}
		Journal_DPrintf( self->debug, "}\n" );
#endif

		self->nodeShadowInfo->procNbrInfo = self->procNbrInfo;
		self->elementShadowInfo->procNbrInfo = self->procNbrInfo;
		
		/*
		 * Calculate where my shadow elements are going to come from.
		 */
		
		/* TODO: FIX TAGS */
		/* TODO: see if some of this can be reduced into re-usable funcs */

		/* allocate some work space */
		request = Memory_Alloc_Array( MPI_Request , self->procNbrInfo->procNbrCnt, "MPI_Request_procNbrCnt" );
		assert( request );
		
		/* convert my shadow element indices to global element indices */
		elementTbl = Memory_Alloc_Array(Element_ShadowIndex, self->elementShadowCount, "Element_ShadowIndex_ElementShadowCount" );
		assert( elementTbl );
		for( element_sI = 0; element_sI < self->elementShadowCount; element_sI++ ) {
			elementTbl[element_sI] = Mesh_ElementMapShadowToGlobal( self, element_sI );
		}
		
		/* prepare my 'shadowed tables' */
		self->elementShadowInfo->procShadowedCnt = Memory_Alloc_Array( Element_LocalIndex,
									       self->procNbrInfo->procNbrCnt, 
									       "Mesh->elementShadowInfo->procShadowedCnt" );
		self->elementShadowInfo->procShadowedTbl = Memory_Alloc_Array( Element_LocalIndex*,
									       self->procNbrInfo->procNbrCnt,
									       "Mesh->elementShadowInfo->procShadowedTbl" );
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			self->elementShadowInfo->procShadowedCnt[nbr_I] = 0;
			self->elementShadowInfo->procShadowedTbl[nbr_I] = NULL;
		}
		
		/* prepare non-blocking recieve for size */
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			PartitionIndex			srcProc_I = self->procNbrInfo->procNbrTbl[nbr_I];
			
			MPI_Irecv( &self->elementShadowInfo->procShadowedCnt[nbr_I], 1, MPI_UNSIGNED, srcProc_I, 100, comm,
				   &request[nbr_I] );
		}
		
		/* send my shadow element count to each neighbour */
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			PartitionIndex			dst = self->procNbrInfo->procNbrTbl[nbr_I];
			
			MPI_Send( &self->elementShadowCount, 1, MPI_UNSIGNED, dst, 100, comm );
		}
		
		/* retrieve the size and prepare tables */
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			MPI_Status		status;
			
			MPI_Wait( &request[nbr_I], &status );
			self->elementShadowInfo->procShadowedTbl[nbr_I] = 
				Memory_Alloc_Array(
					Element_LocalIndex,
					self->elementShadowInfo->procShadowedCnt[nbr_I],
					"self->elementShadowInfo->procShadowedTbl[nbr_I]" );
			assert( self->elementShadowInfo->procShadowedTbl[nbr_I] );
		}
		
		/* prepare non-bloking recieve for elements */
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			PartitionIndex			srcProc_I = self->procNbrInfo->procNbrTbl[nbr_I];
			
			MPI_Irecv( self->elementShadowInfo->procShadowedTbl[nbr_I],
				   self->elementShadowInfo->procShadowedCnt[nbr_I], MPI_UNSIGNED, srcProc_I, 
				   101, comm, &request[nbr_I] );
		}
		
		/* send my shadow elements to each neighbour */
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			PartitionIndex			dst = self->procNbrInfo->procNbrTbl[nbr_I];
			
			MPI_Send( elementTbl, self->elementShadowCount, MPI_UNSIGNED, dst, 101, comm );
		}
		
		/* recieve elements from all neighbours one at a time and process them */
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			Element_GlobalIndex		element_gI;
			MPI_Status			status;
			IndexSet*			localElementSet;
			
			MPI_Wait( &request[nbr_I], &status );
			
			localElementSet = IndexSet_New( self->elementLocalCount );
			assert( localElementSet );
			for( element_gI = 0; element_gI < self->elementShadowInfo->procShadowedCnt[nbr_I]; element_gI++ ) {
				Element_LocalIndex		elementIdx;
				
				elementIdx = Mesh_ElementMapGlobalToLocal( self, 
									   self->elementShadowInfo->procShadowedTbl[nbr_I][element_gI] );
				if( elementIdx < self->elementLocalCount ) {
					IndexSet_Add( localElementSet, elementIdx );
				}
			}
			
			Memory_Free( self->elementShadowInfo->procShadowedTbl[nbr_I] );
			IndexSet_GetMembers( localElementSet, &self->elementShadowInfo->procShadowedCnt[nbr_I], 
					     &self->elementShadowInfo->procShadowedTbl[nbr_I] );
		}
		
		/* setup tables to store feedback */
		self->elementShadowInfo->procShadowCnt = Memory_Alloc_Array( Element_DomainIndex, self->procNbrInfo->procNbrCnt,
									     "Mesh->elementShadowInfo->procShadowCnt" );
		self->elementShadowInfo->procShadowTbl = Memory_Alloc_Array( Element_DomainIndex*, self->procNbrInfo->procNbrCnt,
									     "Mesh->elementShadowInfo->procShadowTbl" );
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			self->elementShadowInfo->procShadowCnt[nbr_I] = 0;
			self->elementShadowInfo->procShadowTbl[nbr_I] = NULL;
		}
		
		/* prepare non-blocking recieve for size */
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			PartitionIndex			srcProc_I = self->procNbrInfo->procNbrTbl[nbr_I];
			
			MPI_Irecv( &self->elementShadowInfo->procShadowCnt[nbr_I], 1, MPI_UNSIGNED, srcProc_I, 200,
				   comm, &request[nbr_I] );
		}
		
		/* send the size */
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			PartitionIndex			dst = self->procNbrInfo->procNbrTbl[nbr_I];
			
			MPI_Send( &self->elementShadowInfo->procShadowedCnt[nbr_I], 1, MPI_UNSIGNED, dst, 200, comm );
		}
		
		/* recieve the feedback size */
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			MPI_Status			status;
			
			MPI_Wait( &request[nbr_I], &status );
			self->elementShadowInfo->procShadowTbl[nbr_I] = Memory_Alloc_Array( Element_DomainIndex,
											    self->elementShadowInfo->procShadowCnt[nbr_I], "Mesh->elementShadowInfo->procShadowTbl[][]" );
		}
		
		/* prepare non-blocking recieve for feedback */
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			PartitionIndex			srcProc_I = self->procNbrInfo->procNbrTbl[nbr_I];
			
			MPI_Irecv( self->elementShadowInfo->procShadowTbl[nbr_I],
				   self->elementShadowInfo->procShadowCnt[nbr_I], MPI_UNSIGNED, 
				   srcProc_I, 201, comm, &request[nbr_I] );
		}
		
		/* convert the shadowed local indices to global indices and send back to the appropriate proc */
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			PartitionIndex			destProc_I = self->procNbrInfo->procNbrTbl[nbr_I];
			Element_LocalIndex		elementCnt;
			Element_GlobalIndex*		globalElementTbl;
			
			elementCnt = self->elementShadowInfo->procShadowedCnt[nbr_I];
			globalElementTbl = Memory_Alloc_Array(Element_GlobalIndex, elementCnt, "Element_GlobalIndex_ElementCnt" );
			assert( globalElementTbl );
			for( element_lI = 0; element_lI < elementCnt; element_lI++ ) {
				globalElementTbl[element_lI] = Mesh_ElementMapLocalToGlobal( self,
											     self->elementShadowInfo->procShadowedTbl[nbr_I][element_lI] );
			}
			
			MPI_Send( globalElementTbl, elementCnt, MPI_UNSIGNED, destProc_I, 201, comm );
			
			Memory_Free( globalElementTbl );
		}
		
		/* recieve the feedback, convert to domain indices and store */
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			MPI_Status			status;
			Element_DomainIndex		element_dI;
			
			MPI_Wait( &request[nbr_I], &status );
			
			for( element_dI = 0; element_dI < self->elementShadowInfo->procShadowCnt[nbr_I]; element_dI++ ) {
				Element_DomainIndex		elementIdx;
				
				elementIdx = Mesh_ElementMapGlobalToDomain( self,
									    self->elementShadowInfo->procShadowTbl[nbr_I][element_dI] );
				assert( elementIdx < self->elementDomainCount );
				self->elementShadowInfo->procShadowTbl[nbr_I][element_dI] = elementIdx;
			}
		}
		
		
		/*
		 * Use the calculated shadow elements and shadowed elements to build the same tables for nodes.
		 * On second thought, just repeat everything above.
		 */
		
		/* convert my shadow node indices to global node indices */
		nodeTbl = Memory_Alloc_Array(Node_ShadowIndex, self->nodeShadowCount, "Node_ShadowIndex_nodeShadowCount" );
		assert( nodeTbl );
		for( node_sI = 0; node_sI < self->nodeShadowCount; node_sI++ ) {
			nodeTbl[node_sI] = Mesh_NodeMapShadowToGlobal( self, node_sI );
		}
		
		/* prepare my 'shadowed tables' */
		self->nodeShadowInfo->procShadowedCnt = Memory_Alloc_Array( Node_LocalIndex, self->procNbrInfo->procNbrCnt, 
									    "Mesh->nodeShadowInfo->procShadowedCnt" );
		self->nodeShadowInfo->procShadowedTbl = Memory_Alloc_Array( Node_LocalIndex*, self->procNbrInfo->procNbrCnt,
									    "Mesh->nodeShadowInfo->procShadowedTbl" );
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			self->nodeShadowInfo->procShadowedCnt[nbr_I] = 0;
			self->nodeShadowInfo->procShadowedTbl[nbr_I] = NULL;
		}
		
		/* prepare non-blocking recieve for size */
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			PartitionIndex			srcProc_I = self->procNbrInfo->procNbrTbl[nbr_I];
			
			MPI_Irecv( &self->nodeShadowInfo->procShadowedCnt[nbr_I], 1, MPI_UNSIGNED, srcProc_I,
				   100, comm, &request[nbr_I] );
		}
		
		/* send my shadow node count to each neighbour */
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			PartitionIndex			dst = self->procNbrInfo->procNbrTbl[nbr_I];
			
			MPI_Send( &self->nodeShadowCount, 1, MPI_UNSIGNED, dst, 100, comm );
		}
		
		/* retrieve the size and prepare tables */
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			MPI_Status		status;
			
			MPI_Wait( &request[nbr_I], &status );
			self->nodeShadowInfo->procShadowedTbl[nbr_I] = 
				Memory_Alloc_Array(
					Node_GlobalIndex,
					self->nodeShadowInfo->procShadowedCnt[nbr_I],
					"self->nodeShadowInfo->procShadowedTbl[nbr_I]" );
			assert( self->nodeShadowInfo->procShadowedTbl[nbr_I] );
		}
		
		/* prepare non-bloking recieve for nodes */
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			PartitionIndex			srcProc_I = self->procNbrInfo->procNbrTbl[nbr_I];
			
			MPI_Irecv( self->nodeShadowInfo->procShadowedTbl[nbr_I],
				   self->nodeShadowInfo->procShadowedCnt[nbr_I], MPI_UNSIGNED, srcProc_I, 
				   101, comm, &request[nbr_I] );
		}
		
		/* send my shadow nodes to each neighbour */
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			PartitionIndex			dst = self->procNbrInfo->procNbrTbl[nbr_I];
			
			MPI_Send( nodeTbl, self->nodeShadowCount, MPI_UNSIGNED, dst, 101, comm );
		}
		
		/* recieve nodes from all neighbours one at a time and process them */
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			Node_GlobalIndex		node_gI;
			MPI_Status			status;
			IndexSet*			localNodeSet;
			
			MPI_Wait( &request[nbr_I], &status );
			
			localNodeSet = IndexSet_New( self->nodeLocalCount );
			assert( localNodeSet );
			for( node_gI = 0; node_gI < self->nodeShadowInfo->procShadowedCnt[nbr_I]; node_gI++ ) {
				Node_LocalIndex		nodeIdx;
				
				nodeIdx = Mesh_NodeMapGlobalToLocal( self, 
								     self->nodeShadowInfo->procShadowedTbl[nbr_I][node_gI] );
				if( nodeIdx < self->nodeLocalCount ) {
					IndexSet_Add( localNodeSet, nodeIdx );
				}
			}
			
			Memory_Free( self->nodeShadowInfo->procShadowedTbl[nbr_I] );
			IndexSet_GetMembers( localNodeSet, &self->nodeShadowInfo->procShadowedCnt[nbr_I], 
					     &self->nodeShadowInfo->procShadowedTbl[nbr_I] );
		}
		
		/* setup tables to store feedback */
		self->nodeShadowInfo->procShadowCnt = Memory_Alloc_Array( Node_DomainIndex, self->procNbrInfo->procNbrCnt,
									  "Mesh->nodeShadowInfo->procShadowCnt" );
		self->nodeShadowInfo->procShadowTbl = Memory_Alloc_Array( Node_DomainIndex*, self->procNbrInfo->procNbrCnt,
									  "Mesh->nodeShadowInfo->procShadowTbl" );
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			self->nodeShadowInfo->procShadowCnt[nbr_I] = 0;
			self->nodeShadowInfo->procShadowTbl[nbr_I] = NULL;
		}
		
		/* prepare non-blocking recieve for size */
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			PartitionIndex			srcProc_I = self->procNbrInfo->procNbrTbl[nbr_I];
			
			MPI_Irecv( &self->nodeShadowInfo->procShadowCnt[nbr_I], 1, MPI_UNSIGNED,
				   srcProc_I, 200, comm, &request[nbr_I] );
		}
		
		/* send the size */
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			PartitionIndex			dst = self->procNbrInfo->procNbrTbl[nbr_I];
			
			MPI_Send( &self->nodeShadowInfo->procShadowedCnt[nbr_I], 1, MPI_UNSIGNED, dst, 200, comm );
		}
		
		/* recieve the feedback size */
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			MPI_Status			status;
			
			MPI_Wait( &request[nbr_I], &status );
			self->nodeShadowInfo->procShadowTbl[nbr_I] = Memory_Alloc_Array( Node_DomainIndex,
											 self->nodeShadowInfo->procShadowCnt[nbr_I], "Mesh->nodeShadowInfo->procShadowTbl[][]" );
		}
		
		/* prepare non-blocking recieve for feedback */
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			PartitionIndex			srcProc_I = self->procNbrInfo->procNbrTbl[nbr_I];
			
			MPI_Irecv( self->nodeShadowInfo->procShadowTbl[nbr_I],
				   self->nodeShadowInfo->procShadowCnt[nbr_I], MPI_UNSIGNED, 
				   srcProc_I, 201, comm, &request[nbr_I] );
		}
		
		/* convert the shadowed local indices to global indices and send back to the appropriate proc */
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			PartitionIndex			destProc_I = self->procNbrInfo->procNbrTbl[nbr_I];
			Node_LocalIndex		nodeCnt;
			Node_GlobalIndex*		globalNodeTbl;
			
			nodeCnt = self->nodeShadowInfo->procShadowedCnt[nbr_I];
			globalNodeTbl = Memory_Alloc_Array( Node_GlobalIndex, nodeCnt, "Node_GlobalIndex_nodeCnt" );
			assert( globalNodeTbl );
			for( node_lI = 0; node_lI < nodeCnt; node_lI++ ) {
				globalNodeTbl[node_lI] = Mesh_NodeMapLocalToGlobal( self,
										    self->nodeShadowInfo->procShadowedTbl[nbr_I][node_lI] );
			}
			
			MPI_Send( globalNodeTbl, nodeCnt, MPI_UNSIGNED, destProc_I, 201, comm );
			
			Memory_Free( globalNodeTbl );
		}
		
		/* recieve the feedback, convert to domain indices and store */
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			MPI_Status			status;
			Node_DomainIndex		node_dI;
			
			MPI_Wait( &request[nbr_I], &status );
			
			for( node_dI = 0; node_dI < self->nodeShadowInfo->procShadowCnt[nbr_I]; node_dI++ ) {
				Node_DomainIndex		nodeIdx;
				
				nodeIdx = Mesh_NodeMapGlobalToDomain( self, self->nodeShadowInfo->procShadowTbl[nbr_I][node_dI] );
				assert( nodeIdx < self->nodeDomainCount );
				self->nodeShadowInfo->procShadowTbl[nbr_I][node_dI] = nodeIdx;
			}
		}
		if ( True == nodeG2LBuiltTemporarily ) {
			Memory_Free( self->nodeG2L );
			self->nodeG2L = NULL;
		}
		if ( True == nodeG2DBuiltTemporarily ) {
			Memory_Free( self->nodeG2D );
			self->nodeG2D = NULL;
		}
		if ( True == elementG2LBuiltTemporarily ) {
			Memory_Free( self->elementG2L );
			self->elementG2L = NULL;
		}
		if ( True == elementG2DBuiltTemporarily ) {
			Memory_Free( self->elementG2D );
			self->elementG2D = NULL;
		}
		
		if( request )
			Memory_Free( request );
		
		
		/*
		 * Set up the tables needed for sending and receiving when Mesh_Sync is called.
		 */
		
		self->syncRequestTbl = Memory_Alloc_Array( MPI_Request*, self->procNbrInfo->procNbrCnt, "Mesh->syncRequestTbl" );
		self->syncElementTbl = Memory_Alloc_Array( Element*, self->procNbrInfo->procNbrCnt, "Mesh->syncElementTbl" );
		self->syncNodeCoordTbl = Memory_Alloc_Array( Coord*, self->procNbrInfo->procNbrCnt, "Mesh->syncNodeCoordTbl" );
		self->syncNodeTbl = Memory_Alloc_Array( Node*, self->procNbrInfo->procNbrCnt, "Mesh->syncNodeTbl" );
		for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
			self->syncRequestTbl[nbr_I] = Memory_Alloc_Array( MPI_Request, 3, "Mesh->syncRequestTbl[...]" );
			self->syncElementTbl[nbr_I] = Memory_Alloc_Array_Bytes( self->elementExtensionMgr->finalSize, 
										self->elementShadowInfo->procShadowCnt[nbr_I], "Element", "Mesh->syncElementTbl[...]" );
			self->syncNodeCoordTbl[nbr_I] = Memory_Alloc_Array( Coord,
									    self->nodeShadowInfo->procShadowCnt[nbr_I], 
									    "Mesh->syncNodeCoordTbl[...]" );
			self->syncNodeTbl[nbr_I] = Memory_Alloc_Array_Bytes( self->nodeExtensionMgr->finalSize, 
									     self->nodeShadowInfo->procShadowCnt[nbr_I], "Node", "Mesh->syncNodeTbl[...]" );
		}
		
		/* send tables */
		maxElementCnt = 0;
                maxNodeCnt = 0;
                for( nbr_I = 0; nbr_I < self->procNbrInfo->procNbrCnt; nbr_I++ ) {
                        if( self->elementShadowInfo->procShadowedCnt[nbr_I] > maxElementCnt )
                                maxElementCnt = self->elementShadowInfo->procShadowedCnt[nbr_I];
                        if( self->elementShadowInfo->procShadowCnt[nbr_I] > maxElementCnt )
                                maxElementCnt = self->elementShadowInfo->procShadowCnt[nbr_I];

                        if( self->nodeShadowInfo->procShadowedCnt[nbr_I] > maxNodeCnt )
                                maxNodeCnt = self->nodeShadowInfo->procShadowedCnt[nbr_I];
                        if( self->nodeShadowInfo->procShadowCnt[nbr_I] > maxNodeCnt )
                                maxNodeCnt = self->nodeShadowInfo->procShadowCnt[nbr_I];
                }

                self->syncElementSendTbl = Memory_Alloc_Array_Bytes( self->elementExtensionMgr->finalSize, maxElementCnt,
								     "Element", "Mesh->syncElementSendTbl" );
                self->syncNodeCoordSendTbl = Memory_Alloc_Array( Coord, maxNodeCnt, "Mesh->syncNodeCoordSendTbl" );
                self->syncNodeSendTbl = Memory_Alloc_Array_Bytes( self->nodeExtensionMgr->finalSize, maxNodeCnt, 
								  "Node", "Mesh->syncNodeSendTbl" );
		Stream_UnIndent( self->debug );
	}
	else {
		/* Cancel everything to zero */
		self->procNbrInfo->procNbrCnt = 0;
		self->procNbrInfo->procNbrTbl = NULL;
		self->elementShadowInfo->procShadowedCnt = NULL;
		self->elementShadowInfo->procShadowedTbl = NULL;
		self->elementShadowInfo->procShadowCnt = NULL;
		self->elementShadowInfo->procShadowTbl = NULL;
		self->nodeShadowInfo->procShadowedCnt = NULL;
		self->nodeShadowInfo->procShadowedTbl = NULL;
		self->nodeShadowInfo->procShadowCnt = NULL;
		self->nodeShadowInfo->procShadowTbl = NULL;
		self->syncElementTbl = NULL;
		self->syncNodeCoordTbl = NULL;
		self->syncNodeTbl = NULL;
	}
}


IndexSet* Mesh_CreateIndexSetFromShape( void* mesh, void* _shape ) {
	Mesh*             self              = Stg_CheckType( mesh, Mesh );
	Stg_Shape*        shape             = Stg_CheckType( _shape, Stg_Shape );
	Node_DomainIndex  dNode_I;
	Node_DomainIndex  nodeDomainCount;
	IndexSet*         indexSet;
	double*           coord;

	nodeDomainCount = self->nodeDomainCount;
	indexSet = IndexSet_New( nodeDomainCount );
	for( dNode_I = 0; dNode_I < nodeDomainCount; dNode_I++ ) {
		coord = Mesh_CoordAt( self, dNode_I );
		if ( Stg_Shape_IsCoordInside( shape, coord ) ) {
			IndexSet_Add( indexSet, dNode_I );
		}
	}

	return indexSet;
}

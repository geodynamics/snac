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
** $Id: HMesh.c 3584 2006-05-16 11:11:07Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"

#include "types.h"
#include "shortcuts.h"
#include "MeshClass.h"
#include "HMesh.h"


#define HMESH_WELD_RESOLUTION	1e-5;


/* Textual name of this class */
const Type HMesh_Type = "HMesh";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

HMesh* HMesh_DefaultNew( Name name ) {
	return _HMesh_New( sizeof(HMesh), 
			   HMesh_Type, 
			   _HMesh_Delete, 
			   _HMesh_Print, 
			   _HMesh_Copy, 
			   (void*)HMesh_DefaultNew,
			   _HMesh_Construct,
			   _HMesh_Build, 
			   _HMesh_Initialise, 
			   _HMesh_Execute,
			   _HMesh_Destroy,
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

HMesh* HMesh_New( Name		name,
		  void*		layout,
		  SizeT		_nodeSize,
		  SizeT		_elementSize,
		  void*		extension_Register,
		  Dictionary*	dictionary )
{
	return _HMesh_New( sizeof(HMesh), 
			   HMesh_Type, 
			   _HMesh_Delete, 
			   _HMesh_Print, 
			   _HMesh_Copy, 
			   (void*)HMesh_DefaultNew,
			   _HMesh_Construct,
			   _HMesh_Build, 
			   _HMesh_Initialise, 
			   _HMesh_Execute,
			   _HMesh_Destroy,
			   name,
			   True,
			   _Mesh_Node_IsLocal1D,
			   _Mesh_Node_IsShadow1D,
			   _Mesh_Element_IsLocal1D,
			   _Mesh_Element_IsShadow1D,
			   layout, 
			   _nodeSize,
			   _elementSize, 
			   extension_Register, 
			   dictionary );
}

void HMesh_Init( HMesh*		self,
		 Name		name,
		 void*		layout,
		 SizeT		_nodeSize,
		 SizeT		_elementSize,
		 void*		extensionMgr_Register,
		 Dictionary*	dictionary )
{
	/* General info */
	self->type = HMesh_Type;
	self->_sizeOfSelf = sizeof(HMesh);
	self->_deleteSelf = False;
	self->dictionary = dictionary;
	
	/* Virtual info */
	self->_delete = _HMesh_Delete;
	self->_print = _HMesh_Print;
	self->_copy = _HMesh_Copy;
	self->_defaultConstructor = (void*)HMesh_DefaultNew;
	self->_construct = _HMesh_Construct;
	self->_build = _HMesh_Build;
	self->_initialise = _HMesh_Initialise;
	self->_execute = _HMesh_Execute;
	self->_destroy = _HMesh_Destroy;
	self->nodeIsLocal = _Mesh_Node_IsLocal1D;
	self->nodeIsShadow = _Mesh_Node_IsShadow1D;
	self->elementIsLocal = _Mesh_Element_IsLocal1D;
	self->elementIsShadow = _Mesh_Element_IsShadow1D;
	
	_Stg_Class_Init( (Stg_Class*)self );
	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );
	_Stg_Component_Init( (Stg_Component*)self );
	_Mesh_Init( (Mesh*)self, layout, _nodeSize, _elementSize, extensionMgr_Register );
	_HMesh_Init( self );
}

HMesh* _HMesh_New( SizeT					_sizeOfSelf, 
		   Type						type,
		   Stg_Class_DeleteFunction*			_delete,
		   Stg_Class_PrintFunction*			_print,
		   Stg_Class_CopyFunction*			_copy, 
		   Stg_Component_DefaultConstructorFunction*	_defaultConstructor,
		   Stg_Component_ConstructFunction*		_construct,
		   Stg_Component_BuildFunction*			_build,
		   Stg_Component_InitialiseFunction*		_initialise,
		   Stg_Component_ExecuteFunction*		_execute,
		   Stg_Component_DestroyFunction*		_destroy,
		   Name						name,
		   Bool						initFlag,
		   Mesh_Node_IsLocalFunction*			nodeIsLocal,
		   Mesh_Node_IsShadowFunction*			nodeIsShadow,
		   Mesh_Element_IsLocalFunction*		elementIsLocal,
		   Mesh_Element_IsShadowFunction*		elementIsShadow,
		   void*					layout,
		   SizeT					_nodeSize,
		   SizeT					_elementSize, 
		   void*					extension_Register,
		   Dictionary*					dictionary )
{
	HMesh* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(HMesh) );
	self = (HMesh*)_Mesh_New( _sizeOfSelf, type, _delete, _print, _copy, 
				  _defaultConstructor, _construct, 
				  _build, _initialise, _execute, _destroy, 
				  name, NON_GLOBAL, 
				  nodeIsLocal, nodeIsShadow, elementIsLocal, elementIsShadow, 
				  layout, _nodeSize, _elementSize, 
				  extension_Register, dictionary );
	
	/* Virtual info */
	
	/* HMesh info */
	if( initFlag ) _HMesh_Init( self );

	return self;
}

void _HMesh_Init( HMesh* self ) {
	/* At this point, _Mesh_Init will have been called. */
	self->nLevels = 0;
	self->levels = NULL;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _HMesh_Construct( void* mesh, Stg_ComponentFactory* cf, void* data ) {
	HMesh*		self = (HMesh*)mesh;
	Dictionary*	dict;
	unsigned	nLevels;
	char*		meshName;
	Mesh*		baseMesh;
	unsigned	baseLevel;

	/* As far as I can tell, we don't really want to construct the super-class. */
	/*_Mesh_Construct( mesh, cf );*/

	/* Sort out my own initialisation. */
	_HMesh_Init( (HMesh*)self );

	/* Shortcuts. */
	dict = cf->componentDict;

	/* Read the base mesh from the dictionary. */
	meshName = Stg_ComponentFactory_GetString( cf, self->name, "base-mesh", NULL );
	if( meshName && strcmp( meshName, "" ) ) {
		baseMesh = Stg_ComponentFactory_ConstructByName( cf, meshName, Mesh, True, data );
	}
	else
		baseMesh = NULL;

	/* Read the number of levels expected. */
	nLevels = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, "levels", 1 );

	/* Read the base mesh's level. */
	baseLevel = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, "base-level", 0 );
	assert( baseLevel < nLevels );

	/* Set everything up. */
	HMesh_SetNLevels( self, nLevels, baseLevel );
	if( baseMesh )
		HMesh_SetMesh( self, baseLevel, baseMesh, False );
}

void _HMesh_Destroy( void* mesh, void* data ) {
	/* What the hell is supposed to go in here anyway? I was under the impression that *_Delete takes
	   care of all the destruction around here... anyway, destroy the super-class. */
	_Mesh_Destroy( mesh, data );
}

void _HMesh_Delete( void* mesh ) {
	HMesh*	self = (HMesh*)mesh;

	/* Delete the parent. */
	_Mesh_Delete( self );
}

void _HMesh_Print( void* mesh, Stream* stream ) {
	HMesh*	self = (HMesh*)mesh;
	
	/* Set the Journal for printing informations */
	Stream* meshStream;
	meshStream = Journal_Register( InfoStream_Type, "HMeshStream" );

	/* Print parent */
	Journal_Printf( stream, "HMesh (ptr): (%p)\n", self );
	_Mesh_Print( self, stream );
}

void* _HMesh_Copy( void* mesh, void* destProc_I, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	HMesh*	self = (HMesh*)mesh;
	HMesh*	newHMesh;
	PtrMap*	map = ptrMap;
	Bool	ownMap = False;

	/* Damn me for making copying so difficult... what was I thinking? */
	
	/* We need to create a map if it doesn't already exist. */
	if( !map ) {
		map = PtrMap_New( 10 );
		ownMap = True;
	}
	
	newHMesh = (HMesh*)_Mesh_Copy( self, destProc_I, deep, nameExt, map );
	
	/* Copy the virtual methods here. */

	/* Deep or shallow? */
	if( deep ) {
	}
	else {
	}
	
	/* If we own the map, get rid of it here. */
	if( ownMap ) Stg_Class_Delete( map );
	
	return (void*)newHMesh;
}

void _HMesh_Build( void* mesh, void* data ) {
	HMesh*		self = (HMesh*)mesh;
	unsigned	l_i;

	assert( self );

	/* Build parent mesh. */
	_Mesh_Build( mesh, data );

	/* Build each level. */
	for( l_i = 0; l_i < self->nLevels; l_i++ )
		Build( self->levels[l_i].mesh, data, False );
}

void _HMesh_Initialise( void* mesh, void* data ) {
	HMesh*		self = (HMesh*)mesh;
	unsigned	l_i;

	assert( self );

	/* Initalise the parent mesh. */
	_Mesh_Initialise( mesh, data );

	/* Initialise all levels, we'll need each mesh's data. */
	for( l_i = 0; l_i < self->nLevels; l_i++ )
		Initialise( self->levels[l_i].mesh, data, False );

	/* Here is where we construct our topographical mesh, the composite of all levels. 
	   These routines also take care of building the mappings bewtween levels and to the
	   topographical mesh. Essentially, this is where the bulk of the work is done. */
	HMesh_BuildNodes( self );
	HMesh_BuildElements( self );
}

void _HMesh_Execute( void* mesh, void* data ) {
	/* Execute parent mesh. */
	_Mesh_Execute( mesh, data );
}

/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void HMesh_SetNLevels( HMesh* self, unsigned nLevels, unsigned baseLevel ) {
	assert( self );
	assert( !nLevels || baseLevel < nLevels );

	/* If this is being set after a previous construction, kill everything. */
	if( self->nLevels ) {
		unsigned	l_i;

		for( l_i = 0; l_i < self->nLevels; l_i++ )
			HMesh_DeleteLevel( self->levels + l_i );
		KillArray( self->levels );
	}

	/* Set the levels and resize everything. */
	self->nLevels = nLevels;
	self->baseLevel = baseLevel;
	if( self->nLevels ) {
		self->levels = Memory_Alloc_Array( HMesh_Level, self->nLevels, "HMesh::levels" );
		memset( self->levels, 0, self->nLevels * sizeof(HMesh_Level) );
	}
}

void HMesh_SetMesh( HMesh* self, unsigned level, Mesh* mesh, Bool own ) {
	assert( self );
	assert( level < self->nLevels );

	/* Make sure the level we're looking at is empty. */
	HMesh_DeleteLevel( self->levels + level );

	/* Store the new mesh. */
	self->levels[level].mesh = mesh;
	self->levels[level].ownMesh = own;
}

Mesh* HMesh_GenMesh( HMesh* self, unsigned level ) {
	char*	name;

	assert( self );
	assert( level < self->nLevels );

	/* Make sure the level we're looking at is empty. */
	HMesh_DeleteLevel( self->levels + level );

	/* Create a new mesh for this level. */
	name = HMesh_GenName( self, level );
	self->levels[level].mesh = Mesh_DefaultNew( name );
	self->levels[level].ownMesh = True;
	Memory_Free( name );

	return self->levels[level].mesh;
}

Mesh* HMesh_GetMesh( HMesh* self, unsigned level ) {
	assert( self );
	assert( level < self->nLevels );
	assert( self->levels );

	return self->levels[level].mesh;
}

Mesh* HMesh_GetBaseMesh( HMesh* self ) {
	assert( self );
	assert( self->levels );

	return self->levels[self->baseLevel].mesh;
}

/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

char* HMesh_GenName( HMesh* self, unsigned level ) {
	unsigned	len;
	unsigned	width;
	char*		name;

	assert( self );
	assert( level < self->nLevels );

	/* Calculate the length of the new name. */
	len = self->name ? strlen(self->name) : 0;
	width = self->nLevels ? log(self->nLevels) + 1 : 0;
	len += width + 2;

	/* Construct the new name. */
	name = Memory_Alloc_Array( char, len, "HMesh::<mesh name>" );
	sprintf( name, "%s_%0*d", self->name ? self->name : "", width, level );

	return name;
}

void HMesh_BuildNodes( HMesh* self ) {
	unsigned	l_i;

	assert( self );

	/* Make sure nodes aren't already set, if they are kill'em. */
	/* TODO */

	/* Higher levels require some additional crap to allow node welding. */
	for( l_i = 0; l_i < self->nLevels; l_i++ ) {
		HMesh_Level*	level = self->levels + l_i;
		HMesh_Level*	prevLevel = self->levels + l_i - 1;
		Mesh*		mesh = level->mesh;
		Mesh*		prevMesh = prevLevel->mesh;
		unsigned	n_i;

		/* The coarsest level is a bit of an exception, we can just copy his 
		   nodal values directly, no messing about. */
		if( l_i == 0 ) {
			self->nodeLocalCount = mesh->nodeLocalCount;
			self->nodeShadowCount = mesh->nodeShadowCount;
			self->nodeDomainCount = mesh->nodeDomainCount;
			self->nodeCoord = Memory_Alloc_Array( Coord, self->nodeDomainCount, "HMesh::nodeCoord" );
			memcpy( self->nodeCoord, mesh->nodeCoord, self->nodeDomainCount * sizeof(Coord) );
			level->nodeUp = Memory_Alloc_Array( unsigned, mesh->nodeDomainCount, "HMesh_InterMap::upToLow" );
			level->nodeGlobals = Memory_Alloc_Array( unsigned, mesh->nodeDomainCount, "HMesh_Level::nodeGlobals" );
			for( n_i = 0; n_i < prevMesh->nodeDomainCount; n_i++ ) {
				level->nodeUp[n_i] = -1;
				level->nodeGlobals[n_i] = n_i;
			}
			continue;
		}

		/* Allocate space on this level for the mappings. */
		level->nodeUp = Memory_Alloc_Array( unsigned, mesh->nodeDomainCount, "HMesh_Level::nodeUp" );
		level->nodeDown = Memory_Alloc_Array( unsigned, mesh->nodeDomainCount, "HMesh_Level::nodeDown" );
		level->nodeGlobals = Memory_Alloc_Array( unsigned, mesh->nodeDomainCount, "HMesh_Level::nodeGlobals" );

		/* Initialise the up map to invalid values. */
		for( n_i = 0; n_i < prevMesh->nodeDomainCount; n_i++ )
			level->nodeUp[n_i] = -1;

		/* Search the previous level for overlapping nodes (i.e. nodes that 
		   are within a certain radius) and use their storage instead of creating new space. */
		for( n_i = 0; n_i < mesh->nodeDomainCount; n_i++ ) {
			unsigned	weld;

			if( HMesh_FindWeld( self, l_i, mesh->nodeCoord[n_i], &weld ) ) {
				/* If we found a node to weld to, do it. */
				level->nodeDown[n_i] = weld;
				prevLevel->nodeUp[weld] = n_i;
				level->nodeGlobals[n_i] = prevLevel->nodeGlobals[weld];
			}
			else {
				/* If we didn't find one, add a new node. */
				self->nodeDomainCount++;
				self->nodeCoord = Memory_Realloc_Array( self->nodeCoord, Coord, self->nodeDomainCount );
				memcpy( self->nodeCoord + self->nodeDomainCount - 1, 
					mesh->nodeCoord + n_i, 
					sizeof(Coord) );
				level->nodeDown[n_i] = -1;
				level->nodeGlobals[n_i] = self->nodeDomainCount - 1;
			}
		}
	}
}

void HMesh_BuildElements( HMesh* self ) {
	unsigned	l_i;

	assert( self );

	/* To construct this level's down elements we can check which down element each of 
	   this element's nodes falls in. If there are more than one, this element spans 
	   many lower elements, meaning something sinister is afoot. */
	for( l_i = 0; l_i < self->nLevels; l_i++ ) {
		HMesh_Level*	level = self->levels + l_i;
		HMesh_Level*	prevLevel;
		Mesh*		mesh = level->mesh;
		Mesh*		prevMesh;
		unsigned	e_i;

		/* We can skip the coarsest level, as it has no down map. */
		if( l_i == 0 ) continue;

		/* Shortcuts. */
		prevLevel = self->levels + l_i - 1;
		prevMesh = prevLevel->mesh;

		/* Allocate space on this level for the mappings. */
		level->elDown = Memory_Alloc_Array( unsigned, mesh->elementDomainCount, "HMesh_Level::elDown" );

		/* Search all the elements. */
		for( e_i = 0; e_i < mesh->elementDomainCount; e_i++ ) {
			unsigned	elInd;
			unsigned	en_i;

			/* Search all the elements' nodes. */
			for( en_i = 0; en_i < mesh->elementNodeCountTbl[e_i]; en_i++ ) {
				unsigned	nInd = mesh->elementNodeTbl[e_i][en_i];
				unsigned	curElInd;

				/* They all have to be in the same coarser element. */
				curElInd = Mesh_ElementWithPoint( prevMesh, mesh->nodeCoord[nInd], 
								  INCLUSIVE_UPPER_BOUNDARY );
				if( en_i == 0 ) elInd = curElInd;
				else if( curElInd != elInd ) {
					fprintf( stderr, "*** ERROR: Overlapping element.\n" );
					abort();
				}
			}

			/* Success! */
			level->elDown[e_i] = elInd;
		}
	}

	/* Now that we have the down mapping, we can construct the up mapping... */
	for( l_i = 0; l_i < self->nLevels; l_i++ ) {
		HMesh_Level*	level = self->levels + l_i;
		HMesh_Level*	nextLevel;
		Mesh*		mesh = level->mesh;
		Mesh*		nextMesh;
		unsigned	e_i;

		/* We can skip the finest level, as it has no up map. */
		if( l_i == self->nLevels - 1 ) continue;

		/* Shortcuts. */
		nextLevel = self->levels + l_i + 1;
		nextMesh = nextLevel->mesh;

		/* Allocate space on this level for the mappings. */
		level->nElsUp = Memory_Alloc_Array( unsigned, mesh->elementDomainCount, "HMesh_Level::nElsUp" );
		level->elUp = Memory_Alloc_Array( unsigned*, mesh->elementDomainCount, "HMesh_Level::elUp" );

		/* Search the upper mesh's elements. */
		for( e_i = 0; e_i < nextMesh->elementDomainCount; e_i++ ) {
			unsigned	elInd = nextLevel->elDown[e_i];

			/* Resize our element list. */
			level->nElsUp[elInd]++;
			if( level->elUp[elInd] ) {
				level->elUp[elInd] = Memory_Realloc_Array( level->elUp[elInd], unsigned, level->nElsUp[elInd] );
			}
			else {
				level->elUp[elInd] = Memory_Alloc_Array( unsigned, 1, "HMesh_Level::elUp[]" );
			}

			/* Add the up element. */
			level->elUp[elInd][level->nElsUp[elInd] - 1] = e_i;
		}

		/* Use the up mapping to determine how many elements we have left unrefined on this level. */
		for( e_i = 0; e_i < mesh->elementDomainCount; e_i++ )
			if( !level->elUp[e_i] ) self->elementDomainCount++;
	}
}

Bool HMesh_FindWeld( HMesh* self, unsigned level, Coord crd, unsigned* weld ) {
	unsigned	node;
	Mesh*		mesh;
	Coord		tmp;

	assert( self );
	assert( level > 0 && level < self->nLevels );

	/* If there is no lower mesh, then what the hell are we doing here? */
	mesh = self->levels[level - 1].mesh;
	assert( mesh );

	/* Find the closest node from lower mesh, check if it's close enough */
	node = Mesh_ClosestNode( mesh, crd );
	Vector_Sub( tmp, crd, mesh->nodeCoord[node] );
	if( Vector_Mag( tmp ) < self->levels[level].weldEpsilon ) {
		*weld = node;
		return True;
	}

	return False;
}

void HMesh_CalcWeldEpsilons( HMesh* self ) {
	unsigned	l_i;
	double		lsep, sep;

	assert( self );

	/* If we don't have enough levels, skip the lot. */
	if( self->nLevels <= 1 ) return;

	/* Calculate the nodal separation for the coarsest mesh before
	   entering the loop; kind of an optimisation. */
	lsep = Mesh_NodeSeparation( self->levels[0].mesh );
	for( l_i = 1; l_i < self->nLevels; l_i++ ) {
		Mesh*	upper = self->levels[l_i].mesh;
		double	usep;

		/* Get the minimum distance between any two nodes of each mesh. */
		usep = Mesh_NodeSeparation( upper );
		sep = usep < lsep ? usep : lsep;

		/* Factor in the weld resolution. */
		self->levels[l_i].weldEpsilon = sep * HMESH_WELD_RESOLUTION;

		/* Flip the separations. */
		lsep = usep;
	}
}

void HMesh_DeleteLevel( HMesh_Level* level ) {
	assert( level );

	if( level->mesh && level->ownMesh ) Stg_Class_Delete( level->mesh );
	else level->mesh = NULL;
	KillArray( level->nodeUp );
	KillArray( level->nodeDown );
	KillArray( level->nodeGlobals );
	KillArray2D( level->mesh->elementDomainCount, level->elUp );
	KillArray( level->elDown );
}

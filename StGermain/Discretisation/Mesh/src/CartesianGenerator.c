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
** $Id: CartesianGenerator.c 3584 2006-05-16 11:11:07Z PatrickSunter $
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
#include "Grid.h"
#include "CommTopology.h"
#include "Decomp.h"
#include "Decomp_Sync.h"
#include "MeshTopology.h"
#include "MeshClass.h"
#include "MeshGenerator.h"
#include "CartesianGenerator.h"

#include "MeshDecomp.h"
#include "HexaMD.h"


/* Textual name of this class */
const Type CartesianGenerator_Type = "CartesianGenerator";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

CartesianGenerator* CartesianGenerator_New( Name name ) {
	return _CartesianGenerator_New( sizeof(CartesianGenerator), 
					CartesianGenerator_Type, 
					_CartesianGenerator_Delete, 
					_CartesianGenerator_Print, 
					_CartesianGenerator_Copy, 
					(void* (*)(Name))_CartesianGenerator_New, 
					_CartesianGenerator_Construct, 
					_CartesianGenerator_Build, 
					_CartesianGenerator_Initialise, 
					_CartesianGenerator_Execute, 
					_CartesianGenerator_Destroy, 
					name, 
					NON_GLOBAL, 
					CartesianGenerator_Generate );
}

CartesianGenerator* _CartesianGenerator_New( CARTESIANGENERATOR_DEFARGS ) {
	CartesianGenerator* self;
	
	/* Allocate memory */
	assert( sizeOfSelf >= sizeof(CartesianGenerator) );
	self = (CartesianGenerator*)_MeshGenerator_New( MESHGENERATOR_PASSARGS );

	/* Virtual info */

	/* CartesianGenerator info */
	_CartesianGenerator_Init( self );

	return self;
}

void _CartesianGenerator_Init( CartesianGenerator* self ) {
	self->shadowDepth = 1;
	self->maxDecompDims = 0;
	self->minDecomp = NULL;
	self->maxDecomp = NULL;
	self->crdMin = NULL;
	self->crdMax = NULL;

	self->topoGrid = NULL;
	self->procGrid = NULL;
	self->origin = NULL;
	self->range = NULL;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _CartesianGenerator_Delete( void* generator ) {
	CartesianGenerator*	self = (CartesianGenerator*)generator;

	CartesianGenerator_Destruct( self );

	/* Delete the parent. */
	_MeshGenerator_Delete( self );
}

void _CartesianGenerator_Print( void* generator, Stream* stream ) {
	CartesianGenerator*	self = (CartesianGenerator*)generator;
	
	/* Set the Journal for printing informations */
	Stream* generatorStream;
	generatorStream = Journal_Register( InfoStream_Type, "CartesianGeneratorStream" );

	/* Print parent */
	Journal_Printf( stream, "CartesianGenerator (ptr): (%p)\n", self );
	_MeshGenerator_Print( self, stream );
}

void* _CartesianGenerator_Copy( void* generator, void* destProc_I, Bool deep, Name nameExt, PtrMap* ptrMap ) {
#if 0
	CartesianGenerator*	self = (CartesianGenerator*)generator;
	CartesianGenerator*	newCartesianGenerator;
	PtrMap*	map = ptrMap;
	Bool	ownMap = False;

	/* Damn me for making copying so difficult... what was I thinking? */
	
	/* We need to create a map if it doesn't already exist. */
	if( !map ) {
		map = PtrMap_New( 10 );
		ownMap = True;
	}
	
	newCartesianGenerator = (CartesianGenerator*)_Mesh_Copy( self, destProc_I, deep, nameExt, map );
	
	/* Copy the virtual methods here. */

	/* Deep or shallow? */
	if( deep ) {
	}
	else {
	}
	
	/* If we own the map, get rid of it here. */
	if( ownMap ) Stg_Class_Delete( map );
	
	return (void*)newCartesianGenerator;
#endif

	return NULL;
}

void _CartesianGenerator_Construct( void* generator, Stg_ComponentFactory* cf, void* data ) {
	CartesianGenerator*	self = (CartesianGenerator*)generator;
	Dictionary*		dict;
	Dictionary_Entry_Value*	sizeList;
	Dictionary_Entry_Value*	minList;
	Dictionary_Entry_Value*	maxList;
	unsigned		nDims;
	double*			crdMin;
	double*			crdMax;
	unsigned*		size;
	unsigned		rank;
	unsigned		d_i;

	HexaMD*			decomp;

	assert( self );
	assert( cf );

	/* Call parent construct. */
	_MeshGenerator_Construct( self, cf, data );

	/* Check if we're using the antiquated version. */
	decomp = Stg_ComponentFactory_ConstructByKey( cf, self->name, "decomp", HexaMD, False, data );
	if( decomp ) {
		/* Read the number of dimensions. */
		dict = cf->rootDict;
		nDims = Dictionary_GetDouble( dict, "dim" );

		/* Allocate for values. */
		crdMin = Memory_Alloc_Array_Unnamed( double, nDims );
		crdMax = Memory_Alloc_Array_Unnamed( double, nDims );
		size = Memory_Alloc_Array_Unnamed( unsigned, nDims );

		/* Read size and coords. */
		size[0] = Dictionary_GetUnsignedInt( dict, "elementResI" );
		crdMin[0] = Dictionary_GetDouble( dict, "minX" );
		crdMax[0] = Dictionary_GetDouble( dict, "maxX" );
		if( nDims >= 2 ) {
			size[1] = Dictionary_GetUnsignedInt( dict, "elementResJ" );
			crdMin[1] = Dictionary_GetDouble( dict, "minY" );
			crdMax[1] = Dictionary_GetDouble( dict, "maxY" );

			if( nDims >= 3 ) {
				size[2] = Dictionary_GetUnsignedInt( dict, "elementResK" );
				crdMin[2] = Dictionary_GetDouble( dict, "minZ" );
				crdMax[2] = Dictionary_GetDouble( dict, "maxZ" );
			}
		}

		/* Read shadow depth. */
		self->shadowDepth = Dictionary_GetUnsignedInt( dict, "shadowDepth" );

		/* Setup. */
		CartesianGenerator_SetTopologyParams( self, nDims, size, 0, NULL, NULL );
		CartesianGenerator_SetGeometryParams( self, crdMin, crdMax );

		/* Free stuff. */
		FreeArray( size );
		FreeArray( crdMin );
		FreeArray( crdMax );

		/* Make sure we're the same decomp as old mesh. */
		Grid_SetSizes( self->procGrid, decomp->partition3DCounts );
		MPI_Comm_rank( self->comm, (int*)&rank );
		for( d_i = 0; d_i < nDims; d_i++ ) {
			self->origin[d_i] = decomp->_elementOffsets[rank][d_i];
			self->range[d_i] = decomp->elementLocal3DCounts[rank][d_i];
		}
	}
	else {
		/* Rip out the components structure as a dictionary. */
		dict = Dictionary_Entry_Value_AsDictionary( Dictionary_Get( cf->componentDict, self->name ) );

		/* Read the sizes. */
		sizeList = Dictionary_Get( dict, "size" );
		assert( sizeList );
		nDims = Dictionary_Entry_Value_GetCount( sizeList );
		assert( nDims );
		size = Memory_Alloc_Array_Unnamed( unsigned, nDims );
		for( d_i = 0; d_i < nDims; d_i++ )
			size[d_i] = Dictionary_Entry_Value_AsUnsignedInt( Dictionary_Entry_Value_GetElement( sizeList, d_i ) );

		/* Initial setup. */
		CartesianGenerator_SetTopologyParams( self, nDims, size, 0, NULL, NULL );

		/* Read geometry. */
		minList = Dictionary_Get( dict, "coordMin" );
		maxList = Dictionary_Get( dict, "coordMax" );
		if( minList && maxList ) {
			assert( Dictionary_Entry_Value_GetCount( sizeList ) == nDims );
			assert( Dictionary_Entry_Value_GetCount( sizeList ) == nDims );
			crdMin = Memory_Alloc_Array_Unnamed( double, nDims );
			crdMax = Memory_Alloc_Array_Unnamed( double, nDims );
			for( d_i = 0; d_i < nDims; d_i++ ) {
				crdMin[d_i] = Dictionary_Entry_Value_AsDouble( Dictionary_Entry_Value_GetElement( minList, d_i ) );
				crdMax[d_i] = Dictionary_Entry_Value_AsDouble( Dictionary_Entry_Value_GetElement( maxList, d_i ) );
			}

			/* Initial setup. */
			CartesianGenerator_SetGeometryParams( self, crdMin, crdMax );

			/* Free coordinate arrays. */
			FreeArray( crdMin );
			FreeArray( crdMax );
		}

		/* Free stuff. */
		FreeArray( size );
	}
}

void _CartesianGenerator_Build( void* generator, void* data ) {
	_MeshGenerator_Build( generator, data );
}

void _CartesianGenerator_Initialise( void* generator, void* data ) {
}

void _CartesianGenerator_Execute( void* generator, void* data ) {
}

void _CartesianGenerator_Destroy( void* generator, void* data ) {
}

/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void CartesianGenerator_SetTopologyParams( void* generator, unsigned nDims, unsigned* sizes, 
					   unsigned maxDecompDims, unsigned* minDecomp, unsigned* maxDecomp )
{
	CartesianGenerator*	self = (CartesianGenerator*)generator;

	/* Sanity check. */
	assert( self );
	assert( !nDims || sizes );
	assert( nDims <= 3 );

	/* Kill everything we have, topologically. */
	KillObject( self->topoGrid );
	KillObject( self->procGrid );
	KillArray( self->origin );
	KillArray( self->range );

	/* Set the parameters. */
	self->topoGrid = Grid_New();
	Grid_SetNDims( self->topoGrid, nDims );
	Grid_SetSizes( self->topoGrid, sizes );

	/* Allocate and set restrictions. */
	self->minDecomp = Memory_Alloc_Array( unsigned, nDims, "CartesianGenerator::minDecomp" );
	self->maxDecomp = Memory_Alloc_Array( unsigned, nDims, "CartesianGenerator::maxDecomp" );

	if( minDecomp )
		memcpy( self->minDecomp, minDecomp, nDims * sizeof(unsigned) );
	else
		memset( self->minDecomp, 0, nDims * sizeof(unsigned) );

	if( maxDecomp )
		memcpy( self->maxDecomp, maxDecomp, nDims * sizeof(unsigned) );
	else
		memset( self->maxDecomp, 0, nDims * sizeof(unsigned) );

	self->maxDecompDims = maxDecompDims;

	/* As soon as we know the topology, we can decompose. */
	CartesianGenerator_BuildDecomp( self );
}

void CartesianGenerator_SetGeometryParams( void* generator, double* min, double* max ) {
	CartesianGenerator*	self = (CartesianGenerator*)generator;

	/* Sanity check. */
	assert( self );
	assert( !self->topoGrid->nDims || (min && max) );

	/* Kill everything we have, geometrically. */
	KillArray( self->crdMin );
	KillArray( self->crdMax );

	/* Set the parameters. */
	if( self->topoGrid->nDims ) {
		self->crdMin = Memory_Alloc_Array( double, self->topoGrid->nDims, "CartesianGenerator::min" );
		self->crdMax = Memory_Alloc_Array( double, self->topoGrid->nDims, "CartesianGenerator::max" );
		memcpy( self->crdMin, min, self->topoGrid->nDims * sizeof(double) );
		memcpy( self->crdMax, max, self->topoGrid->nDims * sizeof(double) );
	}
}

void CartesianGenerator_Generate( void* generator, void* _mesh ) {
	CartesianGenerator*	self = (CartesianGenerator*)generator;
	Mesh*			mesh = (Mesh*)_mesh;
	unsigned*		gSize;
	unsigned*		lSize;
	unsigned*		offs;
	double*			min;
	double*			max;
	unsigned		d_i;

	/* Sanity check. */
	assert( self );
	assert( !self->topoGrid || mesh );

	/* If we havn't been given anything, don't do anything. */
	if( self->topoGrid ) {
		/* Fill topological values. */
		MeshTopology_SetNDims( mesh->topo, self->topoGrid->nDims );
		CartesianGenerator_GenTopo( self, mesh->topo );

		/* Fill geometric values. */
		CartesianGenerator_GenGeom( self, mesh );
	}
	else {
		MeshTopology_SetNDims( mesh->topo, 0 );
	}

	/* Add an extension to the mesh and fill with cartesian information. */
	ExtensionManager_AddArray( mesh->info, "cartesianGlobalSize", sizeof(unsigned), self->topoGrid->nDims );
	ExtensionManager_AddArray( mesh->info, "cartesianLocalSize", sizeof(unsigned), self->topoGrid->nDims );
	ExtensionManager_AddArray( mesh->info, "cartesianLocalOffset", sizeof(unsigned), self->topoGrid->nDims );
	ExtensionManager_AddArray( mesh->info, "cartesianMinCoord", sizeof(double), self->topoGrid->nDims );
	ExtensionManager_AddArray( mesh->info, "cartesianMaxCoord", sizeof(double), self->topoGrid->nDims );
	gSize = (unsigned*)ExtensionManager_Get( mesh->info, mesh, 
						 ExtensionManager_GetHandle( mesh->info, "cartesianGlobalSize" ) );
	lSize = (unsigned*)ExtensionManager_Get( mesh->info, mesh, 
						 ExtensionManager_GetHandle( mesh->info, "cartesianLocalSize" ) );
	offs = (unsigned*)ExtensionManager_Get( mesh->info, mesh, 
						ExtensionManager_GetHandle( mesh->info, "cartesianLocalOffset" ) );
	min = (double*)ExtensionManager_Get( mesh->info, mesh, 
					     ExtensionManager_GetHandle( mesh->info, "cartesianMinCoord" ) );
	max = (double*)ExtensionManager_Get( mesh->info, mesh, 
					     ExtensionManager_GetHandle( mesh->info, "cartesianMaxCoord" ) );
	for( d_i = 0; d_i < self->topoGrid->nDims; d_i++ ) {
		gSize[d_i] = self->topoGrid->sizes[d_i];
		lSize[d_i] = self->range[d_i];
		offs[d_i] = self->origin[d_i];
		min[d_i] = self->crdMin[d_i];
		max[d_i] = self->crdMax[d_i];
	}
}

/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

void CartesianGenerator_BuildDecomp( CartesianGenerator* self ) {
	unsigned	nProcs, rank;
	unsigned	nPos = 0;
	unsigned**	posNSubDomains = NULL;
	unsigned*	tmpSubDomains;
	double		bestRatio;
	unsigned	bestPos;
	unsigned	p_i, d_i;

	/* Sanity check. */
	assert( self );

	/* Collect information regarding parallel configuration. */
	MPI_Comm_size( self->comm, (int*)&nProcs );
	MPI_Comm_rank( self->comm, (int*)&rank );

	/* Allocate for possible sub-domains. */
	tmpSubDomains = Memory_Alloc_Array( unsigned, self->topoGrid->nDims, "" );

	/* Build a list of all acceptable decompositions. */
	CartesianGenerator_RecurseDecomps( self, 0, nProcs, 
					   tmpSubDomains, &nPos, &posNSubDomains );
	assert( nPos );

	/* Free unneeded memory. */
	FreeArray( tmpSubDomains );

	/* Now check for the best ratio. */
	bestRatio = HUGE_VAL;
	bestPos = -1;
	for( p_i = 0; p_i < nPos; p_i++ ) {
		double		curRatio = 0.0;
		unsigned	decompDims = 0;

		/* If decomposed in more dimensions than allowed, skip. */
		for( d_i = 0; d_i < self->topoGrid->nDims; d_i++ )
			decompDims += (posNSubDomains[p_i][d_i] > 1) ? 1 : 0;
		if( self->maxDecompDims && decompDims > self->maxDecompDims )
			continue;

		/* Evaluate ratios. */
		for( d_i = 0; d_i < self->topoGrid->nDims; d_i++ ) {
			double		nI = (double)self->topoGrid->sizes[d_i] / (double)posNSubDomains[p_i][d_i];
			unsigned	d_j;

			for( d_j = d_i + 1; d_j < self->topoGrid->nDims; d_j++ ) {
				double	nJ = (double)self->topoGrid->sizes[d_j] / (double)posNSubDomains[p_i][d_j];

				curRatio += (nI > nJ) ? nI / nJ : nJ / nI;
			}
		}

		/* Better than best? */
		if( curRatio < bestRatio ) {
			bestRatio = curRatio;
			bestPos = p_i;
		}
	}
	assert( bestPos != -1 );

	/* Allocate for results. */
	self->origin = Memory_Alloc_Array( unsigned, self->topoGrid->nDims, "CartesianGenerator::origin" );
	self->range = Memory_Alloc_Array( unsigned, self->topoGrid->nDims, "CartesianGenerator::range" );

	/* Build a sub-domain grid. */
	self->procGrid = Grid_New();
	Grid_SetNDims( self->procGrid, self->topoGrid->nDims );
	Grid_SetSizes( self->procGrid, posNSubDomains[bestPos] );

	/* Free unneeded space. */
	FreeArray( posNSubDomains );

	/* Lift the rank to a parameterised offset. */
	Grid_Lift( self->procGrid, rank, self->origin );
	for( d_i = 0; d_i < self->topoGrid->nDims; d_i++ ) {
		unsigned	base = self->topoGrid->sizes[d_i] / self->procGrid->sizes[d_i];
		unsigned	mod = self->topoGrid->sizes[d_i] % self->procGrid->sizes[d_i];
		unsigned	origin = self->origin[d_i];

		self->origin[d_i] *= base;
		self->range[d_i] = base;
		if( origin < mod ) {
			self->origin[d_i] += origin;
			self->range[d_i]++;
		}
		else
			self->origin[d_i] += mod;
	}
}

void CartesianGenerator_RecurseDecomps( CartesianGenerator* self, 
					unsigned dim, unsigned max, 
					unsigned* nSubDomains, 
					unsigned* nPos, unsigned*** posNSubDomains )
{
	unsigned	nProcs;
	unsigned	nSDs = 1;
	unsigned	d_i;

	MPI_Comm_size( self->comm, (int*)&nProcs );

	/* If we're over the limit, return immediately. */
	for( d_i = 0; d_i < dim; d_i++ )
		nSDs *= nSubDomains[d_i];
	if( nSDs > nProcs )
		return;

	/* Where are we up to? */
	if( dim == self->topoGrid->nDims ) {
		/* If this covers all processors, store it. */
		if( nSDs == nProcs ) {
			/* If we havn't already allocated do it now. */
			if( !*posNSubDomains ) {
				*nPos = 1;
				*posNSubDomains = Memory_Alloc_2DArray_Unnamed( unsigned, 2, self->topoGrid->nDims );
			}
			else {
				/* Reallocate the arrays. */
				(*nPos)++;
				if( *nPos != 2 ) {
					*posNSubDomains = Memory_Realloc_2DArray( *posNSubDomains, unsigned, 
										  *nPos, self->topoGrid->nDims );
				}
			}

			/* Store status. */
			memcpy( (*posNSubDomains)[(*nPos) - 1], nSubDomains, self->topoGrid->nDims * sizeof(unsigned) );
		}
	}
	else {
		unsigned	p_i;

		/* Loop over all remaining */
		for( p_i = 0; p_i < max; p_i++ ) {
			/* Don't try and decompose more than this dimension allows. */
			if( p_i >= self->topoGrid->sizes[dim] || 
			    (self->maxDecomp[dim] && p_i >= self->maxDecomp[dim]) )
			{
				break;
			}

			/* If we have a minimum decomp, skip until we reach it. */
			if( self->minDecomp[dim] && p_i < self->minDecomp[dim] - 1 )
				continue;

			/* Set the number of sub-domains. */
			nSubDomains[dim] = p_i + 1;

			/* Try this combination. */
			CartesianGenerator_RecurseDecomps( self, dim + 1, max - nSDs + 1, nSubDomains, 
							   nPos, posNSubDomains );
		}
	}
}

void CartesianGenerator_GenTopo( CartesianGenerator* self, MeshTopology* topo ) {
	Grid***		grids;
	unsigned	d_i;

	assert( self );
	assert( topo );

	MeshTopology_SetNDims( topo, self->topoGrid->nDims );

	/* Build additional grids for use in numbering. */
	grids = Memory_Alloc_2DArray_Unnamed( Grid*, topo->nTDims, topo->nTDims );
	for( d_i = 0; d_i < topo->nTDims; d_i++ )
		memset( grids[d_i], 0, topo->nTDims * sizeof(Grid*) );

	grids[topo->nDims][0] = self->topoGrid;

	grids[0][0] = Grid_New();
	Grid_SetNDims( grids[0][0], topo->nDims );
	for( d_i = 0; d_i < topo->nDims; d_i++ )
		self->topoGrid->sizes[d_i]++;
	Grid_SetSizes( grids[0][0], self->topoGrid->sizes );
	for( d_i = 0; d_i < topo->nDims; d_i++ )
		self->topoGrid->sizes[d_i]--;

	grids[1][0] = Grid_New();
	Grid_SetNDims( grids[1][0], topo->nDims );
	grids[0][0]->sizes[0]--;
	Grid_SetSizes( grids[1][0], grids[0][0]->sizes );
	grids[0][0]->sizes[0]++;

	if( topo->nDims >= 2 ) {
		grids[1][1] = Grid_New();
		Grid_SetNDims( grids[1][1], topo->nDims );
		grids[0][0]->sizes[1]--;
		Grid_SetSizes( grids[1][1], grids[0][0]->sizes );
		grids[0][0]->sizes[1]++;

		if( topo->nDims >= 3 ) {
			grids[1][2] = Grid_New();
			Grid_SetNDims( grids[1][2], topo->nDims );
			grids[0][0]->sizes[2]--;
			Grid_SetSizes( grids[1][2], grids[0][0]->sizes );
			grids[0][0]->sizes[2]++;

			grids[2][0] = Grid_New();
			Grid_SetNDims( grids[2][0], topo->nDims );
			grids[0][0]->sizes[0]--;
			grids[0][0]->sizes[1]--;
			Grid_SetSizes( grids[2][0], grids[0][0]->sizes );
			grids[0][0]->sizes[0]++;
			grids[0][0]->sizes[1]++;

			grids[2][1] = Grid_New();
			Grid_SetNDims( grids[2][1], topo->nDims );
			grids[0][0]->sizes[0]--;
			grids[0][0]->sizes[2]--;
			Grid_SetSizes( grids[2][1], grids[0][0]->sizes );
			grids[0][0]->sizes[0]++;
			grids[0][0]->sizes[2]++;

			grids[2][2] = Grid_New();
			Grid_SetNDims( grids[2][2], topo->nDims );
			grids[0][0]->sizes[1]--;
			grids[0][0]->sizes[2]--;
			Grid_SetSizes( grids[2][2], grids[0][0]->sizes );
			grids[0][0]->sizes[1]++;
			grids[0][0]->sizes[2]++;
		}
	}

	/* Generate topological elements. */
	CartesianGenerator_GenElements( self, topo, grids );
	CartesianGenerator_GenVertices( self, topo, grids );
	if( topo->nDims >= 2 ) {
		CartesianGenerator_GenEdges( self, topo, grids );
		if( topo->nDims >= 3 )
			CartesianGenerator_GenFaces( self, topo, grids );
	}

	/* Generate topological incidence. */
	CartesianGenerator_GenEdgeVertexInc( self, topo, grids );
	if( topo->nDims >= 2 ) {
		CartesianGenerator_GenFaceEdgeInc( self, topo, grids );
		if( topo->nDims >= 3 )
			CartesianGenerator_GenVolumeFaceInc( self, topo, grids );
	}

	/* Set the shadow depth and correct incidence. */
	if( self->shadowDepth && topo->domains[MT_VERTEX]->commTopo->nProcs > 1 ) {
		/* Build enough incidence to set shadow depth. */
		CartesianGenerator_GenElementVertexInc( self, topo, grids );
		if( topo->nDims >= 3 )
			MeshTopology_Cascade( topo, MT_VOLUME, MT_EDGE );
		MeshTopology_Invert( topo, MT_VERTEX, topo->nDims );
		MeshTopology_Invert( topo, MT_VERTEX, MT_EDGE );
		MeshTopology_Neighbourhood( topo, MT_VERTEX );
		MeshTopology_Neighbourhood( topo, topo->nDims );

		MeshTopology_SetShadowDepth( topo, self->shadowDepth );

		/* Kill up relations and neighbours. */
		for( d_i = 0; d_i < topo->nDims; d_i++ ) {
			unsigned	d_j;

			for( d_j = d_i; d_j < topo->nTDims; d_j++ ) {
				KillArray( topo->nIncEls[d_i][d_j] );
				KillArray( topo->incEls[d_i][d_j] );
			}
		}
		KillArray( topo->nIncEls[topo->nDims][topo->nDims] );
		KillArray( topo->incEls[topo->nDims][topo->nDims] );

		/* Kill cascaded relations. */
		for( d_i = 2; d_i < topo->nTDims; d_i++ ) {
			unsigned	d_j;

			for( d_j = 0; d_j < d_i - 1; d_j++ ) {
				KillArray( topo->nIncEls[d_i][d_j] );
				KillArray( topo->incEls[d_i][d_j] );
			}
		}
	}

	/* Complete all relations. */
	/*MeshTopology_Complete( topo );*/

	/* Free allocated grids. */
	grids[topo->nDims][0] = NULL;
	for( d_i = 0; d_i < topo->nDims; d_i++ ) {
		unsigned	d_j;

		for( d_j = 0; d_j < topo->nTDims; d_j++ )
			FreeObject( grids[d_i][d_j] );
	}
	FreeArray( grids );
}

void CartesianGenerator_GenElements( CartesianGenerator* self, MeshTopology* topo, Grid*** grids ) {
	Grid*		grid;
	unsigned	nEls;
	unsigned*	els;
	unsigned*	dimInds;
	unsigned	d_i, e_i;

	assert( self );
	assert( topo );
	assert( grids );

	grid = Grid_New();
	Grid_SetNDims( grid, self->topoGrid->nDims );
	Grid_SetSizes( grid, self->range );

	nEls = grid->sizes[0];
	for( d_i = 1; d_i < grid->nDims; d_i++ )
		nEls *= grid->sizes[d_i];
	els = Memory_Alloc_Array_Unnamed( unsigned, nEls );

	dimInds = Memory_Alloc_Array_Unnamed( unsigned, self->topoGrid->nDims );

	for( e_i = 0; e_i < nEls; e_i++ ) {
		Grid_Lift( grid, e_i, dimInds );
		for( d_i = 0; d_i < grid->nDims; d_i++ )
			dimInds[d_i] += self->origin[d_i];
		els[e_i] = Grid_Project( self->topoGrid, dimInds );
	}

	MeshTopology_SetElements( topo, self->topoGrid->nDims, nEls, els );

	FreeArray( dimInds );
	FreeArray( els );
	FreeObject( grid );
}

void CartesianGenerator_GenVertices( CartesianGenerator* self, MeshTopology* topo, Grid*** grids ) {
	Grid*		globalGrid;
	Grid*		grid;
	unsigned	nEls;
	unsigned*	els;
	unsigned*	dimInds;
	unsigned	d_i, e_i;

	assert( self );
	assert( topo );
	assert( grids );

	globalGrid = Grid_New();
	Grid_SetNDims( globalGrid, self->topoGrid->nDims );
	for( d_i = 0; d_i < globalGrid->nDims; d_i++ )
		self->topoGrid->sizes[d_i]++;
	Grid_SetSizes( globalGrid, self->topoGrid->sizes );
	for( d_i = 0; d_i < globalGrid->nDims; d_i++ )
		self->topoGrid->sizes[d_i]--;

	grid = Grid_New();
	Grid_SetNDims( grid, self->topoGrid->nDims );
	for( d_i = 0; d_i < grid->nDims; d_i++ )
		self->range[d_i]++;
	Grid_SetSizes( grid, self->range );
	for( d_i = 0; d_i < grid->nDims; d_i++ )
		self->range[d_i]--;

	nEls = grid->sizes[0];
	for( d_i = 1; d_i < grid->nDims; d_i++ )
		nEls *= grid->sizes[d_i];
	els = Memory_Alloc_Array_Unnamed( unsigned, nEls );

	dimInds = Memory_Alloc_Array_Unnamed( unsigned, self->topoGrid->nDims );

	for( e_i = 0; e_i < nEls; e_i++ ) {
		Grid_Lift( grid, e_i, dimInds );
		for( d_i = 0; d_i < grid->nDims; d_i++ )
			dimInds[d_i] += self->origin[d_i];
		els[e_i] = Grid_Project( globalGrid, dimInds );
	}

	MeshTopology_SetElements( topo, MT_VERTEX, nEls, els );

	FreeArray( dimInds );
	FreeArray( els );
	FreeObject( grid );
	FreeObject( globalGrid );
}

void CartesianGenerator_GenEdges( CartesianGenerator* self, MeshTopology* topo, Grid*** grids ) {
	assert( self );
	assert( topo );
	assert( self->topoGrid->nDims >= 2 );
	assert( self->topoGrid->nDims <= 3 );

	if( self->topoGrid->nDims == 2 )
		CartesianGenerator_GenEdges2D( self, topo, grids );
	else
		CartesianGenerator_GenEdges3D( self, topo, grids );
}

void CartesianGenerator_GenEdges2D( CartesianGenerator* self, MeshTopology* topo, Grid*** grids ) {
	Grid*		globalGrid;
	unsigned	nGlobalEls;
	Grid*		grid;
	unsigned	nEls[2];
	unsigned*	els;
	unsigned*	dimInds;
	unsigned	d_i, e_i;

	assert( self );
	assert( topo );
	assert( grids );
	assert( self->topoGrid->nDims == 2 );

	globalGrid = Grid_New();
	Grid_SetNDims( globalGrid, self->topoGrid->nDims );
	self->topoGrid->sizes[1]++;
	Grid_SetSizes( globalGrid, self->topoGrid->sizes );
	self->topoGrid->sizes[1]--;

	grid = Grid_New();
	Grid_SetNDims( grid, self->topoGrid->nDims );
	self->range[1]++;
	Grid_SetSizes( grid, self->range );
	self->range[1]--;

	nEls[0] = grid->sizes[0];
	nGlobalEls = globalGrid->sizes[0];
	for( d_i = 1; d_i < grid->nDims; d_i++ ) {
		nEls[0] *= grid->sizes[d_i];
		nGlobalEls *= globalGrid->sizes[d_i];
	}
	els = Memory_Alloc_Array_Unnamed( unsigned, nEls[0] );

	dimInds = Memory_Alloc_Array_Unnamed( unsigned, self->topoGrid->nDims );

	for( e_i = 0; e_i < nEls[0]; e_i++ ) {
		Grid_Lift( grid, e_i, dimInds );
		for( d_i = 0; d_i < grid->nDims; d_i++ )
			dimInds[d_i] += self->origin[d_i];
		els[e_i] = Grid_Project( globalGrid, dimInds );
	}

	self->topoGrid->sizes[0]++;
	Grid_SetSizes( globalGrid, self->topoGrid->sizes );
	self->topoGrid->sizes[0]--;

	self->range[0]++;
	Grid_SetSizes( grid, self->range );
	self->range[0]--;

	nEls[1] = grid->sizes[0];
	for( d_i = 1; d_i < grid->nDims; d_i++ )
		nEls[1] *= grid->sizes[d_i];
	els = Memory_Realloc_Array( els, unsigned, nEls[0] + nEls[1] );

	for( e_i = 0; e_i < nEls[1]; e_i++ ) {
		Grid_Lift( grid, e_i, dimInds );
		for( d_i = 0; d_i < grid->nDims; d_i++ )
			dimInds[d_i] += self->origin[d_i];
		els[nEls[0] + e_i] = nGlobalEls + Grid_Project( globalGrid, dimInds );
	}

	MeshTopology_SetElements( topo, MT_EDGE, nEls[0] + nEls[1], els );

	FreeArray( dimInds );
	FreeArray( els );
	FreeObject( grid );
	FreeObject( globalGrid );
}

void CartesianGenerator_GenEdges3D( CartesianGenerator* self, MeshTopology* topo, Grid*** grids ) {
	Grid*		globalGrid;
	unsigned	nGlobalEls[2];
	Grid*		grid;
	unsigned	nEls[3];
	unsigned*	els;
	unsigned*	dimInds;
	unsigned	d_i, e_i;

	assert( self );
	assert( topo );
	assert( grids );
	assert( self->topoGrid->nDims == 3 );

	globalGrid = Grid_New();
	Grid_SetNDims( globalGrid, self->topoGrid->nDims );
	self->topoGrid->sizes[1]++;
	self->topoGrid->sizes[2]++;
	Grid_SetSizes( globalGrid, self->topoGrid->sizes );
	self->topoGrid->sizes[1]--;
	self->topoGrid->sizes[2]--;

	grid = Grid_New();
	Grid_SetNDims( grid, self->topoGrid->nDims );
	self->range[1]++;
	self->range[2]++;
	Grid_SetSizes( grid, self->range );
	self->range[1]--;
	self->range[2]--;

	nEls[0] = grid->sizes[0];
	nGlobalEls[0] = globalGrid->sizes[0];
	for( d_i = 1; d_i < grid->nDims; d_i++ ) {
		nEls[0] *= grid->sizes[d_i];
		nGlobalEls[0] *= globalGrid->sizes[d_i];
	}
	els = Memory_Alloc_Array_Unnamed( unsigned, nEls[0] );

	dimInds = Memory_Alloc_Array_Unnamed( unsigned, self->topoGrid->nDims );

	for( e_i = 0; e_i < nEls[0]; e_i++ ) {
		Grid_Lift( grid, e_i, dimInds );
		for( d_i = 0; d_i < grid->nDims; d_i++ )
			dimInds[d_i] += self->origin[d_i];
		els[e_i] = Grid_Project( globalGrid, dimInds );
	}

	self->topoGrid->sizes[0]++;
	self->topoGrid->sizes[2]++;
	Grid_SetSizes( globalGrid, self->topoGrid->sizes );
	self->topoGrid->sizes[0]--;
	self->topoGrid->sizes[2]--;

	self->range[0]++;
	self->range[2]++;
	Grid_SetSizes( grid, self->range );
	self->range[0]--;
	self->range[2]--;

	nEls[1] = grid->sizes[0];
	nGlobalEls[1] = globalGrid->sizes[0];
	for( d_i = 1; d_i < grid->nDims; d_i++ ) {
		nEls[1] *= grid->sizes[d_i];
		nGlobalEls[1] *= globalGrid->sizes[d_i];
	}
	els = Memory_Realloc_Array( els, unsigned, nEls[0] + nEls[1] );

	for( e_i = 0; e_i < nEls[1]; e_i++ ) {
		Grid_Lift( grid, e_i, dimInds );
		for( d_i = 0; d_i < grid->nDims; d_i++ )
			dimInds[d_i] += self->origin[d_i];
		els[nEls[0] + e_i] = nGlobalEls[0] + Grid_Project( globalGrid, dimInds );
	}

	self->topoGrid->sizes[0]++;
	self->topoGrid->sizes[1]++;
	Grid_SetSizes( globalGrid, self->topoGrid->sizes );
	self->topoGrid->sizes[0]--;
	self->topoGrid->sizes[1]--;

	self->range[0]++;
	self->range[1]++;
	Grid_SetSizes( grid, self->range );
	self->range[0]--;
	self->range[1]--;

	nEls[2] = grid->sizes[0];
	for( d_i = 1; d_i < grid->nDims; d_i++ )
		nEls[2] *= grid->sizes[d_i];
	els = Memory_Realloc_Array( els, unsigned, nEls[0] + nEls[1] + nEls[2] );

	for( e_i = 0; e_i < nEls[2]; e_i++ ) {
		Grid_Lift( grid, e_i, dimInds );
		for( d_i = 0; d_i < grid->nDims; d_i++ )
			dimInds[d_i] += self->origin[d_i];
		els[nEls[0] + nEls[1] + e_i] = nGlobalEls[0] + nGlobalEls[1] + Grid_Project( globalGrid, dimInds );
	}

	MeshTopology_SetElements( topo, MT_EDGE, nEls[0] + nEls[1] + nEls[2], els );

	FreeArray( dimInds );
	FreeArray( els );
	FreeObject( grid );
	FreeObject( globalGrid );
}

void CartesianGenerator_GenFaces( CartesianGenerator* self, MeshTopology* topo, Grid*** grids ) {
	Grid*		globalGrid;
	unsigned	nGlobalEls[2];
	Grid*		grid;
	unsigned	nEls[3];
	unsigned*	els;
	unsigned*	dimInds;
	unsigned	d_i, e_i;

	assert( self );
	assert( topo );
	assert( grids );
	assert( self->topoGrid->nDims == 3 );

	globalGrid = Grid_New();
	Grid_SetNDims( globalGrid, self->topoGrid->nDims );
	self->topoGrid->sizes[2]++;
	Grid_SetSizes( globalGrid, self->topoGrid->sizes );
	self->topoGrid->sizes[2]--;

	grid = Grid_New();
	Grid_SetNDims( grid, self->topoGrid->nDims );
	self->range[2]++;
	Grid_SetSizes( grid, self->range );
	self->range[2]--;

	nEls[0] = grid->sizes[0];
	nGlobalEls[0] = globalGrid->sizes[0];
	for( d_i = 1; d_i < grid->nDims; d_i++ ) {
		nEls[0] *= grid->sizes[d_i];
		nGlobalEls[0] *= globalGrid->sizes[d_i];
	}
	els = Memory_Alloc_Array_Unnamed( unsigned, nEls[0] );

	dimInds = Memory_Alloc_Array_Unnamed( unsigned, self->topoGrid->nDims );

	for( e_i = 0; e_i < nEls[0]; e_i++ ) {
		Grid_Lift( grid, e_i, dimInds );
		for( d_i = 0; d_i < grid->nDims; d_i++ )
			dimInds[d_i] += self->origin[d_i];
		els[e_i] = Grid_Project( globalGrid, dimInds );
	}

	self->topoGrid->sizes[1]++;
	Grid_SetSizes( globalGrid, self->topoGrid->sizes );
	self->topoGrid->sizes[1]--;

	self->range[1]++;
	Grid_SetSizes( grid, self->range );
	self->range[1]--;

	nEls[1] = grid->sizes[0];
	nGlobalEls[1] = globalGrid->sizes[0];
	for( d_i = 1; d_i < grid->nDims; d_i++ ) {
		nEls[1] *= grid->sizes[d_i];
		nGlobalEls[1] *= globalGrid->sizes[d_i];
	}
	els = Memory_Realloc_Array( els, unsigned, nEls[0] + nEls[1] );

	for( e_i = 0; e_i < nEls[1]; e_i++ ) {
		Grid_Lift( grid, e_i, dimInds );
		for( d_i = 0; d_i < grid->nDims; d_i++ )
			dimInds[d_i] += self->origin[d_i];
		els[nEls[0] + e_i] = nGlobalEls[0] + Grid_Project( globalGrid, dimInds );
	}

	self->topoGrid->sizes[0]++;
	Grid_SetSizes( globalGrid, self->topoGrid->sizes );
	self->topoGrid->sizes[0]--;

	self->range[0]++;
	Grid_SetSizes( grid, self->range );
	self->range[0]--;

	nEls[2] = grid->sizes[0];
	for( d_i = 1; d_i < grid->nDims; d_i++ )
		nEls[2] *= grid->sizes[d_i];
	els = Memory_Realloc_Array( els, unsigned, nEls[0] + nEls[1] + nEls[2] );

	for( e_i = 0; e_i < nEls[2]; e_i++ ) {
		Grid_Lift( grid, e_i, dimInds );
		for( d_i = 0; d_i < grid->nDims; d_i++ )
			dimInds[d_i] += self->origin[d_i];
		els[nEls[0] + nEls[1] + e_i] = nGlobalEls[0] + nGlobalEls[1] + Grid_Project( globalGrid, dimInds );
	}

	MeshTopology_SetElements( topo, MT_FACE, nEls[0] + nEls[1] + nEls[2], els );

	FreeArray( dimInds );
	FreeArray( els );
	FreeObject( grid );
	FreeObject( globalGrid );
}

void CartesianGenerator_GenElementVertexInc( CartesianGenerator* self, MeshTopology* topo, Grid*** grids ) {
	unsigned*	nIncEls;
	unsigned**	incEls;
	unsigned*	dimInds;
	unsigned	vertsPerEl;
	unsigned	e_i;

	assert( self );
	assert( topo );
	assert( grids );

	vertsPerEl = (topo->nDims == 1) ? 2 : (topo->nDims == 2) ? 4 : 8;

	nIncEls = Memory_Alloc_Array_Unnamed( unsigned, topo->nDomainEls[topo->nDims] );
	incEls = Memory_Alloc_2DArray_Unnamed( unsigned, topo->nDomainEls[topo->nDims], vertsPerEl );
	dimInds = Memory_Alloc_Array_Unnamed( unsigned, topo->nDims );
	for( e_i = 0; e_i < topo->nDomainEls[topo->nDims]; e_i++ ) {
		unsigned	gInd = Decomp_Sync_DomainToGlobal( topo->domains[topo->nDims], e_i );

		nIncEls[e_i] = vertsPerEl;
		Grid_Lift( grids[topo->nDims][0], gInd, dimInds );

		incEls[e_i][0] = Grid_Project( grids[0][0], dimInds );

		dimInds[0]++;
		incEls[e_i][1] = Grid_Project( grids[0][0], dimInds );
		dimInds[0]--;

		if( topo->nDims >= 2 ) {
			dimInds[1]++;
			incEls[e_i][2] = Grid_Project( grids[0][0], dimInds );

			dimInds[0]++;
			incEls[e_i][3] = Grid_Project( grids[0][0], dimInds );
			dimInds[0]--;
			dimInds[1]--;

			if( topo->nDims >= 3 ) {
				dimInds[2]++;
				incEls[e_i][4] = Grid_Project( grids[0][0], dimInds );

				dimInds[0]++;
				incEls[e_i][5] = Grid_Project( grids[0][0], dimInds );
				dimInds[0]--;

				dimInds[1]++;
				incEls[e_i][6] = Grid_Project( grids[0][0], dimInds );

				dimInds[0]++;
				incEls[e_i][7] = Grid_Project( grids[0][0], dimInds );
				dimInds[0]--;
				dimInds[1]--;
				dimInds[2]--;
			}
		}
	}

	CartesianGenerator_MapToDomain( self, topo->domains[MT_VERTEX], topo->nDomainEls[topo->nDims], nIncEls, incEls );
	MeshTopology_SetIncidence( topo, topo->nDims, MT_VERTEX, nIncEls, incEls );
	FreeArray( nIncEls );
	FreeArray( incEls );
	FreeArray( dimInds );
}

void CartesianGenerator_GenVolumeFaceInc( CartesianGenerator* self, MeshTopology* topo, Grid*** grids ) {
	unsigned*	nIncEls;
	unsigned**	incEls;
	unsigned*	dimInds;
	unsigned	e_i;

	assert( self );
	assert( topo );
	assert( grids );
	assert( topo->nDims >= 3 );

	nIncEls = Memory_Alloc_Array_Unnamed( unsigned, topo->nDomainEls[topo->nDims] );
	incEls = Memory_Alloc_2DArray_Unnamed( unsigned, topo->nDomainEls[topo->nDims], 6 );
	dimInds = Memory_Alloc_Array_Unnamed( unsigned, topo->nDims );
	for( e_i = 0; e_i < topo->nDomainEls[MT_VOLUME]; e_i++ ) {
		unsigned	gInd = Decomp_Sync_DomainToGlobal( topo->domains[MT_VOLUME], e_i );

		nIncEls[e_i] = 6;
		Grid_Lift( grids[topo->nDims][0], gInd, dimInds );

		incEls[e_i][0] = Grid_Project( grids[2][0], dimInds );

		dimInds[2]++;
		incEls[e_i][1] = Grid_Project( grids[2][0], dimInds );
		dimInds[2]--;

		incEls[e_i][2] = Grid_Project( grids[2][1], dimInds ) + grids[2][0]->nPoints;

		dimInds[1]++;
		incEls[e_i][3] = Grid_Project( grids[2][1], dimInds ) + grids[2][0]->nPoints;
		dimInds[1]--;

		incEls[e_i][4] = Grid_Project( grids[2][2], dimInds ) + grids[2][0]->nPoints + grids[2][1]->nPoints;

		dimInds[0]++;
		incEls[e_i][5] = Grid_Project( grids[2][2], dimInds ) + grids[2][0]->nPoints + grids[2][1]->nPoints;
		dimInds[0]--;
	}

	CartesianGenerator_MapToDomain( self, topo->domains[MT_FACE], topo->nDomainEls[MT_VOLUME], nIncEls, incEls );
	MeshTopology_SetIncidence( topo, MT_VOLUME, MT_FACE, nIncEls, incEls );
	FreeArray( nIncEls );
	FreeArray( incEls );
	FreeArray( dimInds );
}

void CartesianGenerator_GenFaceEdgeInc( CartesianGenerator* self, MeshTopology* topo, Grid*** grids ) {
	unsigned*	nIncEls;
	unsigned**	incEls;
	unsigned*	dimInds;
	unsigned	e_i;

	assert( self );
	assert( topo );
	assert( grids );
	assert( topo->nDims >= 2 );

	nIncEls = Memory_Alloc_Array_Unnamed( unsigned, topo->nDomainEls[MT_FACE] );
	incEls = Memory_Alloc_2DArray_Unnamed( unsigned, topo->nDomainEls[MT_FACE], 4 );
	dimInds = Memory_Alloc_Array_Unnamed( unsigned, topo->nDims );
	for( e_i = 0; e_i < topo->nDomainEls[MT_FACE]; e_i++ ) {
		unsigned	gInd = Decomp_Sync_DomainToGlobal( topo->domains[MT_FACE], e_i );

		nIncEls[e_i] = 4;

		if( gInd < grids[2][0]->nPoints ) {
			Grid_Lift( grids[2][0], gInd, dimInds );

			incEls[e_i][0] = Grid_Project( grids[1][0], dimInds );

			dimInds[1]++;
			incEls[e_i][1] = Grid_Project( grids[1][0], dimInds );
			dimInds[1]--;

			incEls[e_i][2] = Grid_Project( grids[1][1], dimInds ) + grids[1][0]->nPoints;

			dimInds[0]++;
			incEls[e_i][3] = Grid_Project( grids[1][1], dimInds ) + grids[1][0]->nPoints;
			dimInds[0]--;
		}
		else if( gInd < grids[2][0]->nPoints + grids[2][1]->nPoints ) {
			assert( topo->nDims >= 3 );

			Grid_Lift( grids[2][1], gInd - grids[2][0]->nPoints, dimInds );

			incEls[e_i][0] = Grid_Project( grids[1][0], dimInds );

			dimInds[2]++;
			incEls[e_i][1] = Grid_Project( grids[1][0], dimInds );
			dimInds[2]--;

			incEls[e_i][2] = Grid_Project( grids[1][2], dimInds ) + grids[1][0]->nPoints + grids[1][1]->nPoints;

			dimInds[0]++;
			incEls[e_i][3] = Grid_Project( grids[1][2], dimInds ) + grids[1][0]->nPoints + grids[1][1]->nPoints;
			dimInds[0]--;
		}
		else {
			assert( gInd < grids[2][0]->nPoints + grids[2][1]->nPoints + grids[2][2]->nPoints );
			assert( topo->nDims >= 3 );

			Grid_Lift( grids[2][2], gInd - grids[2][0]->nPoints - grids[2][1]->nPoints, dimInds );

			incEls[e_i][0] = Grid_Project( grids[1][1], dimInds ) + grids[1][0]->nPoints;

			dimInds[2]++;
			incEls[e_i][1] = Grid_Project( grids[1][1], dimInds ) + grids[1][0]->nPoints;
			dimInds[2]--;

			incEls[e_i][2] = Grid_Project( grids[1][2], dimInds ) + grids[1][0]->nPoints + grids[1][1]->nPoints;

			dimInds[1]++;
			incEls[e_i][3] = Grid_Project( grids[1][2], dimInds ) + grids[1][0]->nPoints + grids[1][1]->nPoints;
			dimInds[1]--;
		}
	}

	CartesianGenerator_MapToDomain( self, topo->domains[MT_EDGE], topo->nDomainEls[MT_FACE], nIncEls, incEls );
	MeshTopology_SetIncidence( topo, MT_FACE, MT_EDGE, nIncEls, incEls );
	FreeArray( nIncEls );
	FreeArray( incEls );
	FreeArray( dimInds );
}

void CartesianGenerator_GenEdgeVertexInc( CartesianGenerator* self, MeshTopology* topo, Grid*** grids ) {
	unsigned*	nIncEls;
	unsigned**	incEls;
	unsigned*	dimInds;
	unsigned	e_i;

	assert( self );
	assert( topo );
	assert( grids );

	nIncEls = Memory_Alloc_Array_Unnamed( unsigned, topo->nDomainEls[MT_EDGE] );
	incEls = Memory_Alloc_2DArray_Unnamed( unsigned, topo->nDomainEls[MT_EDGE], 2 );
	dimInds = Memory_Alloc_Array_Unnamed( unsigned, topo->nDims );
	for( e_i = 0; e_i < topo->nDomainEls[MT_EDGE]; e_i++ ) {
		unsigned	gInd = Decomp_Sync_DomainToGlobal( topo->domains[MT_EDGE], e_i );

		nIncEls[e_i] = 2;

		if( gInd < grids[1][0]->nPoints ) {
			Grid_Lift( grids[1][0], gInd, dimInds );

			incEls[e_i][0] = Grid_Project( grids[0][0], dimInds );

			dimInds[0]++;
			incEls[e_i][1] = Grid_Project( grids[0][0], dimInds );
			dimInds[0]--;
		}
		else if( gInd < grids[1][0]->nPoints + grids[1][1]->nPoints ) {
			assert( topo->nDims >= 2 );

			Grid_Lift( grids[1][1], gInd - grids[1][0]->nPoints, dimInds );

			incEls[e_i][0] = Grid_Project( grids[0][0], dimInds );

			dimInds[1]++;
			incEls[e_i][1] = Grid_Project( grids[0][0], dimInds );
			dimInds[1]--;
		}
		else {
			assert( gInd < grids[1][0]->nPoints + grids[1][1]->nPoints + grids[1][2]->nPoints );
			assert( topo->nDims >= 3 );

			Grid_Lift( grids[1][2], gInd - grids[1][0]->nPoints - grids[1][1]->nPoints, dimInds );

			incEls[e_i][0] = Grid_Project( grids[0][0], dimInds );

			dimInds[2]++;
			incEls[e_i][1] = Grid_Project( grids[0][0], dimInds );
			dimInds[2]--;
		}
	}

	CartesianGenerator_MapToDomain( self, topo->domains[MT_VERTEX], topo->nDomainEls[MT_EDGE], nIncEls, incEls );
	MeshTopology_SetIncidence( topo, MT_EDGE, MT_VERTEX, nIncEls, incEls );
	FreeArray( nIncEls );
	FreeArray( incEls );
	FreeArray( dimInds );
}

void CartesianGenerator_MapToDomain( CartesianGenerator* self, Decomp_Sync* sync, 
				     unsigned size, unsigned* nIncEls, unsigned** incEls )
{
	unsigned	e_i;

	assert( self );
	assert( sync );
	assert( nIncEls );
	assert( incEls );

	for( e_i = 0; e_i < size; e_i++ ) {
		unsigned	inc_i;

		for( inc_i = 0; inc_i < nIncEls[e_i]; inc_i++ )
			incEls[e_i][inc_i] = Decomp_Sync_GlobalToDomain( sync, incEls[e_i][inc_i] );
	}
}

void CartesianGenerator_GenGeom( CartesianGenerator* self, Mesh* mesh ) {
	Grid*		grid;
	unsigned*	inds;
	double*		steps;
	unsigned	n_i, d_i;

	assert( self );
	assert( mesh );

	/* Build grid and space for indices. */
	grid = Grid_New();
	Grid_SetNDims( grid, mesh->topo->nDims );
	for( d_i = 0; d_i < mesh->topo->nDims; d_i++ )
		self->topoGrid->sizes[d_i]++;
	Grid_SetSizes( grid, self->topoGrid->sizes );
	for( d_i = 0; d_i < mesh->topo->nDims; d_i++ )
		self->topoGrid->sizes[d_i]--;
	inds = Memory_Alloc_Array_Unnamed( unsigned, mesh->topo->nDims );

	/* Calculate steps. */
	steps = Memory_Alloc_Array_Unnamed( double, mesh->topo->nDims );
	for( d_i = 0; d_i < mesh->topo->nDims; d_i++ )
		steps[d_i] = self->crdMax[d_i] - self->crdMin[d_i];

	/* Allocate for coordinates. */
	mesh->nodeCoord = Memory_Alloc_Array( Coord, MeshTopology_GetDomainSize( mesh->topo, MT_VERTEX ), 
					      "Mesh::nodeCoord" );

	/* Loop over domain nodes. */
	for( n_i = 0; n_i < MeshTopology_GetDomainSize( mesh->topo, MT_VERTEX ); n_i++ ) {
		unsigned	gNode;

		gNode = MeshTopology_DomainToGlobal( mesh->topo, MT_VERTEX, n_i );
		Grid_Lift( grid, gNode, inds );

		/* Calculate coordinate. */
		for( d_i = 0; d_i < mesh->topo->nDims; d_i++ ) {
			mesh->nodeCoord[n_i][d_i] = self->crdMin[d_i] + 
				((double)inds[d_i] / (double)self->topoGrid->sizes[d_i]) * steps[d_i];
		}
	}

	/* Free resources. */
	FreeArray( inds );
	FreeArray( steps );
	FreeObject( grid );
}

void CartesianGenerator_Destruct( CartesianGenerator* self ) {
	assert( self );

	FreeObject( self->topoGrid );
	FreeArray( self->crdMin );
	FreeArray( self->crdMax );
	FreeObject( self->procGrid );
	FreeArray( self->origin );
	FreeArray( self->range );
}

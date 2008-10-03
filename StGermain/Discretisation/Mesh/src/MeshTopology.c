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
** $Id: MeshTopology.c 3584 2006-05-16 11:11:07Z PatrickSunter $
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
#include "CommTopology.h"
#include "Decomp.h"
#include "Decomp_Sync.h"
#include "MeshTopology.h"


/* Textual name of this class */
const Type MeshTopology_Type = "MeshTopology";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

MeshTopology* MeshTopology_New( Name name ) {
	return _MeshTopology_New( sizeof(MeshTopology), 
				  MeshTopology_Type, 
				  _MeshTopology_Delete, 
				  _MeshTopology_Print, 
				  _MeshTopology_Copy, 
				  (void* (*)(Name))_MeshTopology_New, 
				  _MeshTopology_Construct, 
				  _MeshTopology_Build, 
				  _MeshTopology_Initialise, 
				  _MeshTopology_Execute, 
				  _MeshTopology_Destroy, 
				  name, 
				  NON_GLOBAL );
}

MeshTopology* _MeshTopology_New( MESHTOPOLOGY_DEFARGS ) {
	MeshTopology* self;
	
	/* Allocate memory */
	assert( sizeOfSelf >= sizeof(MeshTopology) );
	self = (MeshTopology*)_Stg_Component_New( STG_COMPONENT_PASSARGS );

	/* Virtual info */

	/* MeshTopology info */
	_MeshTopology_Init( self );

	return self;
}

void _MeshTopology_Init( MeshTopology* self ) {
	self->nDims = 0;
	self->nTDims = 0;

	self->domains = NULL;
	self->nRentals = NULL;
	self->rentals = NULL;
	self->nDomainEls = NULL;
	self->shadowDepth = 0;

	self->nIncEls = NULL;
	self->incEls = NULL;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _MeshTopology_Delete( void* topo ) {
	MeshTopology*	self = (MeshTopology*)topo;

	MeshTopology_Destruct( self );

	/* Delete the parent. */
	_Stg_Component_Delete( self );
}

void _MeshTopology_Print( void* topo, Stream* stream ) {
	MeshTopology*	self = (MeshTopology*)topo;
	
	/* Set the Journal for printing informations */
	Stream* topoStream;
	topoStream = Journal_Register( InfoStream_Type, "MeshTopologyStream" );

	/* Print parent */
	Journal_Printf( stream, "MeshTopology (ptr): (%p)\n", self );
	_Stg_Component_Print( self, stream );
}

void* _MeshTopology_Copy( void* topo, void* destProc_I, Bool deep, Name nameExt, PtrMap* ptrMap ) {
#if 0
	MeshTopology*	self = (MeshTopology*)topo;
	MeshTopology*	newMeshTopology;
	PtrMap*	map = ptrMap;
	Bool	ownMap = False;

	/* Damn me for making copying so difficult... what was I thinking? */
	
	/* We need to create a map if it doesn't already exist. */
	if( !map ) {
		map = PtrMap_New( 10 );
		ownMap = True;
	}
	
	newMeshTopology = (MeshTopology*)_Mesh_Copy( self, destProc_I, deep, nameExt, map );
	
	/* Copy the virtual methods here. */

	/* Deep or shallow? */
	if( deep ) {
	}
	else {
	}
	
	/* If we own the map, get rid of it here. */
	if( ownMap ) Stg_Class_Delete( map );
	
	return (void*)newMeshTopology;
#endif

	return NULL;
}

void _MeshTopology_Construct( void* topo, Stg_ComponentFactory* cf, void* data ) {
}

void _MeshTopology_Build( void* topo, void* data ) {
}

void _MeshTopology_Initialise( void* topo, void* data ) {
}

void _MeshTopology_Execute( void* topo, void* data ) {
}

void _MeshTopology_Destroy( void* topo, void* data ) {
}

/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void MeshTopology_SetNDims( void* topo, unsigned nDims ) {
	MeshTopology*	self = (MeshTopology*)topo;

	assert( self );
	assert( nDims > 0 );

	/* If we're changing dimensions, kill everything and begin again. */
	MeshTopology_Destruct( self );

	/* Set and allocate. */
	self->nDims = nDims;
	self->nTDims = nDims + 1;
	if( nDims ) {
		unsigned	 d_i;

		self->domains = Memory_Alloc_Array( Decomp_Sync*, self->nTDims, "MeshTopology::domains" );
		self->nDomainEls = Memory_Alloc_Array( unsigned, self->nTDims, "MeshTopology::nDomainEls" );
		self->nIncEls = Memory_Alloc_2DArray( unsigned*, self->nTDims, self->nTDims, "MeshTopology::nIncEls" );
		self->incEls = Memory_Alloc_2DArray( unsigned**, self->nTDims, self->nTDims, "MeshTopology::incEls" );
		for( d_i = 0; d_i < self->nTDims; d_i++ ) {
			unsigned	d_j;

			self->domains[d_i] = Decomp_Sync_New( "" );
			self->nDomainEls[d_i] = 0;
			Decomp_Sync_SetDecomp( self->domains[d_i], Decomp_New( "" ) );

			for( d_j = 0; d_j < self->nTDims; d_j++ ) {
				self->nIncEls[d_i][d_j] = NULL;
				self->incEls[d_i][d_j] = NULL;
			}
		}
	}
}

void MeshTopology_SetElements( void* topo, MeshTopology_Dim dim, unsigned nEls, unsigned* els ) {
	MeshTopology*	self = (MeshTopology*)topo;

	assert( self );
	assert( dim < self->nTDims );
	assert( !nEls || els );

	/* Decompose elements. */
	Decomp_Sync_Decompose( self->domains[dim], nEls, els );

	/* Set the number of domain elements. */	
	self->nDomainEls[dim] = self->domains[dim]->decomp->nLocals + self->domains[dim]->nRemotes;
}

void MeshTopology_SetIncidence( void* topo, MeshTopology_Dim fromDim, MeshTopology_Dim toDim, 
				unsigned* nIncEls, unsigned** incEls )
{
	MeshTopology*	self = (MeshTopology*)topo;
	unsigned	size;
	unsigned	e_i;

	assert( self );
	assert( fromDim < self->nTDims );
	assert( toDim < self->nTDims );
	assert( nIncEls );
	assert( incEls );
	assert( Decomp_Sync_GetDomainSize( self->domains[fromDim] ) );
	assert( Decomp_Sync_GetDomainSize( self->domains[toDim] ) );

	/* Clear existing incidence. */
	KillArray( self->incEls[fromDim][toDim] );
	KillArray( self->nIncEls[fromDim][toDim] );

	/* Allocate for incoming data. */
	size = Decomp_Sync_GetDomainSize( self->domains[fromDim] );
	self->nIncEls[fromDim][toDim] = Memory_Alloc_Array( unsigned, size, "MeshTopology::nIncEls[][]" );
	self->incEls[fromDim][toDim] = Memory_Alloc_2DComplex( unsigned, size, nIncEls, "MeshTopology::incEls[][]" );

	/* Copy the lot. */
	memcpy( self->nIncEls[fromDim][toDim], nIncEls, size * sizeof(unsigned) );
	for( e_i = 0; e_i < size; e_i++ ) {
		memcpy( self->incEls[fromDim][toDim][e_i], incEls[e_i], 
			self->nIncEls[fromDim][toDim][e_i] * sizeof(unsigned) );
	}
}

void MeshTopology_Complete( void* topo ) {
	MeshTopology*	self = (MeshTopology*)topo;
	unsigned	d_i, d_j;

	assert( self );

	/* Complete downwards. */
	for( d_i = 0; d_i < self->nTDims - 1; d_i++ ) {
		for( d_j = d_i + 2; d_j < self->nTDims; d_j++ ) {
			if( !self->nIncEls[d_j][d_i] )
				MeshTopology_Cascade( self, d_j, d_i );
		}
	}

	/* Invert missing up relations. */
	for( d_i = 0; d_i < self->nTDims - 1; d_i++ ) {
		for( d_j = d_i + 1; d_j < self->nTDims; d_j++ ) {
			if( !self->nIncEls[d_i][d_j] )
				MeshTopology_Invert( self, d_i, d_j );
		}
	}

	/* Build neighbourhoods. */
	for( d_i = 0; d_i < self->nTDims; d_i++ ) {
		if( !self->nIncEls[d_i][d_i] )
			MeshTopology_Neighbourhood( self, d_i );
	}
}

void MeshTopology_SetShadowDepth( void* topo, unsigned depth ) {
	MeshTopology*	self = (MeshTopology*)topo;
	RangeSet***	shadows;
	unsigned**	incSizes;
	unsigned***	shadowInc;

	assert( self );
	assert( depth );

	/* Need to store shadow information for each processor in each topological dimension. */
	self->shadowDepth = depth;

	/* If there are no neighbouring processors, forget about it. */
	if( !self->domains[MT_VERTEX]->commTopo->nInc )
		return;

	shadows = (RangeSet***)Memory_Alloc_2DArray_Unnamed( RangeSet**, self->nTDims, 
							     self->domains[MT_VERTEX]->commTopo->nInc );
	MeshTopology_BuildTopShadows( self, shadows + self->nDims );
	MeshTopology_BuildAllShadows( self, shadows );
	MeshTopology_BuildShadowInc( self, shadows, &incSizes, &shadowInc );

	/* Send to neighbours. */
	MeshTopology_SendRecvShadows( self, shadows, incSizes, shadowInc );
}

void MeshTopology_Invert( void* topo, MeshTopology_Dim fromDim, MeshTopology_Dim toDim ) {
	MeshTopology*	self = (MeshTopology*)topo;
	unsigned	fromSize, toSize;
	unsigned*	invNIncEls;
	unsigned**	invIncEls;
	unsigned*	nIncEls;
	unsigned**	incEls;
	unsigned	e_i;

	/* Sanity check. */
	assert( self );
	assert( fromDim < self->nTDims );
	assert( toDim < self->nTDims );
	assert( Decomp_Sync_GetDomainSize( self->domains[fromDim] ) );
	assert( Decomp_Sync_GetDomainSize( self->domains[toDim] ) );
	assert( !self->nIncEls[fromDim][toDim] && !self->incEls[fromDim][toDim] );
	assert( self->nIncEls[toDim][fromDim] && self->incEls[toDim][fromDim] );

	/* Shortcuts. */
	fromSize = Decomp_Sync_GetDomainSize( self->domains[fromDim] );
	toSize = Decomp_Sync_GetDomainSize( self->domains[toDim] );
	invNIncEls = self->nIncEls[toDim][fromDim];
	invIncEls = self->incEls[toDim][fromDim];

	/* Allocate some tables. */
	nIncEls = Memory_Alloc_Array( unsigned, fromSize, "MeshTopology::nIncEls[][]" );
	memset( nIncEls, 0, fromSize * sizeof(unsigned) );

	/* Two phase process: count the numbers then allocate and do it again. */
	for( e_i = 0; e_i < toSize; e_i++ ) {
		unsigned	inc_i;

		for( inc_i = 0; inc_i < invNIncEls[e_i]; inc_i++ ) {
			unsigned	elInd = invIncEls[e_i][inc_i];

			nIncEls[elInd]++;
		}
	}

	/* Build up the tables. */
	incEls = Memory_Alloc_2DComplex( unsigned, fromSize, nIncEls, "MeshTopology::incEls[][]" );
	memset( nIncEls, 0, fromSize * sizeof(unsigned) );
	for( e_i = 0; e_i < toSize; e_i++ ) {
		unsigned	inc_i;

		for( inc_i = 0; inc_i < invNIncEls[e_i]; inc_i++ ) {
			unsigned	elInd = invIncEls[e_i][inc_i];

			incEls[elInd][nIncEls[elInd]++] = e_i;
		}
	}

	/* Transfer to permanent storage. */
	self->nIncEls[fromDim][toDim] = nIncEls;
	self->incEls[fromDim][toDim] = incEls;
}

void MeshTopology_Cascade( void* topo, MeshTopology_Dim fromDim, MeshTopology_Dim toDim ) {
	MeshTopology*	self = (MeshTopology*)topo;
	unsigned	maxInc = 0;
	unsigned*	nIncEls;
	unsigned**	incEls;
	unsigned	e_i;

	assert( self );
	assert( fromDim < self->nTDims );
	assert( toDim < self->nTDims );
	assert( fromDim > toDim + 1 );

	/* Determine maximum incidence for any element. */
	for( e_i = 0; e_i < self->nDomainEls[fromDim]; e_i++ ) {
		unsigned	curInc = 0;
		unsigned	inc_i;

		for( inc_i = 0; inc_i < self->nIncEls[fromDim][fromDim - 1][e_i]; inc_i++ ) {
			unsigned	incEl = self->incEls[fromDim][fromDim - 1][e_i][inc_i];

			curInc += self->nIncEls[fromDim - 1][toDim][incEl];
		}

		if( curInc > maxInc )
			maxInc = curInc;
	}

	/* Allocate. */
	nIncEls = Memory_Alloc_Array_Unnamed( unsigned, self->nDomainEls[fromDim] );
	incEls = Memory_Alloc_2DArray_Unnamed( unsigned, self->nDomainEls[fromDim], maxInc );
	memset( nIncEls, 0, self->nDomainEls[fromDim] * sizeof(unsigned) );

	/* Determine actual incidence. */
	for( e_i = 0; e_i < self->nDomainEls[fromDim]; e_i++ ) {
		unsigned	inc_i;

		for( inc_i = 0; inc_i < self->nIncEls[fromDim][fromDim - 1][e_i]; inc_i++ ) {
			unsigned	incEl = self->incEls[fromDim][fromDim - 1][e_i][inc_i];
			unsigned	inc_j;

			for( inc_j = 0; inc_j < self->nIncEls[fromDim - 1][toDim][incEl]; inc_j++ ) {
				unsigned	target = self->incEls[fromDim - 1][toDim][incEl][inc_j];
				unsigned	e_j;

				for( e_j = 0; e_j < nIncEls[e_i]; e_j++ ) {
					if( incEls[e_i][e_j] == target )
						break;
				}
				if( e_j == nIncEls[e_i] )
					incEls[e_i][nIncEls[e_i]++] = target;
			}
		}
	}

	/* Set incidence and kill temporary arrays. */
	MeshTopology_SetIncidence( self, fromDim, toDim, nIncEls, incEls );
	FreeArray( nIncEls );
	FreeArray( incEls );
}

void MeshTopology_Neighbourhood( void* topo, MeshTopology_Dim dim ) {
	MeshTopology*		self = (MeshTopology*)topo;
	unsigned		size;
	MeshTopology_Dim	toDim;
	unsigned*		nNbrs;
	unsigned**		nbrs;
	unsigned		e_i;

	assert( self );
	assert( dim < self->nTDims );
	assert( Decomp_Sync_GetDomainSize( self->domains[dim] ) );

	/* Vertex neighbours search upwards, all others search downwards. */
	size = Decomp_Sync_GetDomainSize( self->domains[dim] );
	toDim = (dim == MT_VERTEX) ? MT_EDGE : MT_VERTEX;

	/* Allocate some space for neighbour counts. */
	nNbrs = Memory_Alloc_Array_Unnamed( unsigned, size ); 

	/* Calculate maximum neighbours for each element. */
	for( e_i = 0; e_i < size; e_i++ ) {
		unsigned	nNodes = self->nIncEls[dim][toDim][e_i];
		unsigned*	nodes = self->incEls[dim][toDim][e_i];
		unsigned	n_i;

		nNbrs[e_i] = 0;
		for( n_i = 0; n_i < nNodes; n_i++ ) {
			unsigned	nCurNbrs = self->nIncEls[toDim][dim][nodes[n_i]];

			nNbrs[e_i] += nCurNbrs - 1;
		}
	}

	/* Allocate for maximum neighbours and clear neighbour counts. */
	nbrs = Memory_Alloc_2DComplex_Unnamed( unsigned, size, nNbrs );
	memset( nNbrs, 0, size * sizeof(unsigned) );

	/* Build neighbours for each element of dimension 'dim'. */
	for( e_i = 0; e_i < size; e_i++ ) {
		unsigned	nNodes = self->nIncEls[dim][toDim][e_i];
		unsigned*	nodes = self->incEls[dim][toDim][e_i];
		unsigned	n_i;

		/* Build a set of unique element nodes' elements. */
		for( n_i = 0; n_i < nNodes; n_i++ ) {
			unsigned	nCurNbrs = self->nIncEls[toDim][dim][nodes[n_i]];
			unsigned	e_j;

			for( e_j = 0; e_j < nCurNbrs; e_j++ ) {
				unsigned	curNbr = self->incEls[toDim][dim][nodes[n_i]][e_j];
				unsigned	nbr_i;

				/* Don't add current element. */
				if( curNbr == e_i )
					continue;

				/* Ensure is unique. */
				for( nbr_i = 0; nbr_i < nNbrs[e_i]; nbr_i++ ) {
					if( nbrs[e_i][nbr_i] == curNbr )
						break;
				}
				if( nbr_i == nNbrs[e_i] )
					nbrs[e_i][nNbrs[e_i]++] = curNbr;
			}
		}
	}

	/* Transfer. */
	MeshTopology_SetIncidence( self, dim, dim, nNbrs, nbrs );
	FreeArray( nNbrs );
	FreeArray( nbrs );
}

unsigned MeshTopology_GetLocalSize( void* meshTopology, MeshTopology_Dim dim ) {
	MeshTopology*	self = (MeshTopology*)meshTopology;

	assert( self );
	assert( dim < self->nTDims );
	assert( self->domains );
	assert( self->domains[dim] );
	assert( self->domains[dim]->decomp );

	return self->domains[dim]->decomp->nLocals;
}

unsigned MeshTopology_GetShadowSize( void* meshTopology, MeshTopology_Dim dim ) {
	MeshTopology*	self = (MeshTopology*)meshTopology;

	assert( self );
	assert( dim < self->nTDims );
	assert( self->domains );
	assert( self->domains[dim] );

	return self->domains[dim]->nRemotes;
}

unsigned MeshTopology_GetDomainSize( void* meshTopology, MeshTopology_Dim dim ) {
	MeshTopology*	self = (MeshTopology*)meshTopology;

	assert( self );
	assert( dim < self->nTDims );
	assert( self->nDomainEls );

	return self->nDomainEls[dim];
}

unsigned MeshTopology_DomainToGlobal( void* meshTopology, MeshTopology_Dim dim, unsigned domain ) {
	MeshTopology*	self = (MeshTopology*)meshTopology;

	assert( self );
	assert( dim < self->nTDims );
	assert( self->domains );
	assert( self->domains[dim] );

	return Decomp_Sync_DomainToGlobal( self->domains[dim], domain );
}

unsigned MeshTopology_GlobalToDomain( void* meshTopology, MeshTopology_Dim dim, unsigned global ) {
	MeshTopology*	self = (MeshTopology*)meshTopology;

	assert( self );
	assert( dim < self->nTDims );
	assert( self->domains );
	assert( self->domains[dim] );

	return Decomp_Sync_GlobalToDomain( self->domains[dim], global );
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

void MeshTopology_BuildTopShadows( void* meshTopology, RangeSet*** shadows ) {
	MeshTopology*	self = (MeshTopology*)meshTopology;
	CommTopology*	commTopo;
	unsigned	nIncRanks;
	unsigned*	incRanks;
	void**		shdSets;
	Decomp_Sync*	sync;
	unsigned	p_i, s_i;

	assert( self );
	assert( shadows );

	/* Extract communicator incidence. */
	commTopo = self->domains[MT_VERTEX]->commTopo;
	CommTopology_GetIncidence( commTopo, commTopo->rank, &nIncRanks, &incRanks );

	/* Build some sets, initially index sets. */
	shdSets = (void**)Memory_Alloc_Array_Unnamed( IndexSet*, nIncRanks );
	for( p_i = 0; p_i < nIncRanks; p_i++ )
		shdSets[p_i] = (void*)IndexSet_New( Decomp_Sync_GetDomainSize( self->domains[MT_VERTEX] ) );

	/* Store shared vertices. */
	sync = self->domains[0];
	for( s_i = 0; s_i < sync->nShared; s_i++ ) {
		for( p_i = 0; p_i < sync->nSharers[s_i]; p_i++ )
			IndexSet_Add( shdSets[sync->sharers[s_i][p_i]], sync->shared[s_i] );
	}

	/* Build range sets of shadowed elements. */
	for( p_i = 0; p_i < nIncRanks; p_i++ ) {
		unsigned	nEdges;
		unsigned*	edges;
		unsigned	nShdEls;
		unsigned*	shdEls;
		unsigned	e_i;

		IndexSet_GetMembers( shdSets[p_i], &nEdges, &edges );
		FreeObject( shdSets[p_i] );
		shdSets[p_i] = (void*)IndexSet_New( MeshTopology_GetLocalSize( self, self->nDims ) );

		/* Build initial set of elements. */
		for( e_i = 0; e_i < nEdges; e_i++ ) {
			unsigned	nIncEls = self->nIncEls[MT_VERTEX][self->nDims][edges[e_i]];
			unsigned*	incEls = self->incEls[MT_VERTEX][self->nDims][edges[e_i]];
			unsigned	inc_i;

			for( inc_i = 0; inc_i < nIncEls; inc_i++ ) {
				if( incEls[inc_i] >= MeshTopology_GetLocalSize( self, self->nDims ) )
					continue;
				IndexSet_Add( shdSets[p_i], incEls[inc_i] );
			}
		}

		/* Free edge set. */
		FreeArray( edges );

		/* Expand the elements to satisfy shadow depth. */
		if( self->shadowDepth > 1 ) {
			IndexSet*	bndSet;
			unsigned	d_i;

			bndSet = IndexSet_DeepCopy( shdSets[p_i] );

			for( d_i = 1; d_i < self->shadowDepth; d_i++ )
				MeshTopology_ExpandShadows( self, (IndexSet*)shdSets[p_i], bndSet );

			FreeObject( bndSet );
		}

		/* Get members and convert to global indices. */
		IndexSet_GetMembers( shdSets[p_i], &nShdEls, &shdEls );
		FreeObject( shdSets[p_i] );
		for( e_i = 0; e_i < nShdEls; e_i++ )
			shdEls[e_i] = Decomp_Sync_DomainToGlobal( self->domains[self->nDims], shdEls[e_i] );

		/* Create range set. */
		shdSets[p_i] = (void*)RangeSet_New();
		RangeSet_SetIndices( shdSets[p_i], nShdEls, shdEls );
		FreeArray( shdEls );
	}

	/* Store result. */
	*shadows = (RangeSet**)shdSets;
}

void MeshTopology_ExpandShadows( void* meshTopology, IndexSet* shadows, IndexSet* boundary ) {
	MeshTopology*	self = (MeshTopology*)meshTopology;
	unsigned	nLocals = MeshTopology_GetLocalSize( self, self->nDims );
	unsigned	nBndEls;
	unsigned*	bndEls;
	unsigned	b_i;

	assert( self );
	assert( shadows );
	assert( boundary );

	IndexSet_GetMembers( boundary, &nBndEls, &bndEls );
	FreeObject( boundary );
	boundary = IndexSet_New( nLocals );

	for( b_i = 0; b_i < nBndEls; b_i++ ) {
		unsigned	nNbrs = self->nIncEls[self->nDims][self->nDims][bndEls[b_i]];
		unsigned*	nbrs = self->incEls[self->nDims][self->nDims][bndEls[b_i]];
		unsigned	n_i;

		for( n_i = 0; n_i < nNbrs; n_i++ ) {
			if( nbrs[n_i] >= nLocals || IndexSet_IsMember( shadows, nbrs[n_i] ) )
				continue;
			IndexSet_Add( boundary, nbrs[n_i] );
		}
	}
	FreeArray( bndEls );

	IndexSet_GetMembers( boundary, &nBndEls, &bndEls );
	for( b_i = 0; b_i < nBndEls; b_i++ )
		IndexSet_Add( shadows, bndEls[b_i] );
	FreeArray( bndEls );
}

void MeshTopology_BuildAllShadows( void* meshTopology, RangeSet*** shadows ) {
	MeshTopology*	self = (MeshTopology*)meshTopology;
	CommTopology*	commTopo;
	unsigned	nIncRanks;
	unsigned*	incRanks;
	unsigned	p_i;

	assert( self );
	assert( shadows );

	/* Extract communicator incidence. */
	commTopo = self->domains[MT_VERTEX]->commTopo;
	CommTopology_GetIncidence( commTopo, commTopo->rank, &nIncRanks, &incRanks );

	for( p_i = 0; p_i < nIncRanks; p_i++ ) {
		unsigned	nElInds;
		unsigned*	elInds;
		unsigned	d_i, e_i;

		/* Convert range set of top level to indices. */
		RangeSet_GetIndices( shadows[self->nDims][p_i], &nElInds, &elInds );
		for( e_i = 0; e_i < nElInds; e_i++ )
			elInds[e_i] = Decomp_Sync_GlobalToDomain( self->domains[self->nDims], elInds[e_i] );

		/* */
		for( d_i = 0; d_i < self->nDims; d_i++ ) {
			IndexSet*	iSet;
			unsigned	nInds;
			unsigned*	inds;

			/* If there are no elements in this dimension or we have no incidence relations between
			   these levels, skip it. */
			if( MeshTopology_GetLocalSize( self, d_i ) == 0 || 
			    !self->nIncEls[self->nDims][d_i] || !self->incEls[self->nDims][d_i] )
			{
				shadows[d_i][p_i] = NULL;
				continue;
			}

			iSet = IndexSet_New( MeshTopology_GetLocalSize( self, d_i ) );

			for( e_i = 0; e_i < nElInds; e_i++ ) {
				unsigned	nIncEls;
				unsigned*	incEls;
				unsigned	inc_i;

				nIncEls = self->nIncEls[self->nDims][d_i][elInds[e_i]];
				incEls = self->incEls[self->nDims][d_i][elInds[e_i]];
				for( inc_i = 0; inc_i < nIncEls; inc_i++ ) {
					if( incEls[inc_i] >= MeshTopology_GetLocalSize( self, d_i ) )
						continue;
					IndexSet_Add( iSet, incEls[inc_i] );
				}
			}

			/* Get members and convert to global indices. */
			IndexSet_GetMembers( iSet, &nInds, &inds );
			FreeObject( iSet );
			for( e_i = 0; e_i < nInds; e_i++ )
				inds[e_i] = Decomp_Sync_DomainToGlobal( self->domains[d_i], inds[e_i] );

			/* Create shadow set. */
			shadows[d_i][p_i] = RangeSet_New();
			RangeSet_SetIndices( shadows[d_i][p_i], nInds, inds );
			FreeArray( inds );
		}
	}
}

void MeshTopology_BuildShadowInc( void* meshTopology, RangeSet*** shadows, 
				  unsigned*** incDataSizes, unsigned**** incData )
{
	MeshTopology*	self = (MeshTopology*)meshTopology;
	CommTopology*	commTopo;
	unsigned	nIncRanks;
	unsigned*	incRanks;
	unsigned	p_i;

	assert( self );
	assert( shadows );
	assert( incDataSizes );
	assert( incData );

	/* Extract communicator incidence. */
	commTopo = self->domains[MT_VERTEX]->commTopo;
	CommTopology_GetIncidence( commTopo, commTopo->rank, &nIncRanks, &incRanks );

	/* Allocate for the incidence data sizes. */
	*incDataSizes = Memory_Alloc_2DArray_Unnamed( unsigned, self->nTDims, nIncRanks );
	*incData = Memory_Alloc_2DArray_Unnamed( unsigned*, self->nTDims, nIncRanks );

	for( p_i = 0; p_i < nIncRanks; p_i++ ) {
		unsigned	d_i;

		for( d_i = 1; d_i < self->nTDims; d_i++ ) {
			unsigned	nEls;
			unsigned*	els;
			unsigned	offs = 0;
			unsigned	e_i;

			/* If we have no shadows for this dimension, skip. */
			if( !shadows[d_i][p_i] ) {
				(*incDataSizes)[d_i][p_i] = 0;
				(*incData)[d_i][p_i] = NULL;
				continue;
			}

			/* Extract the indices. */
			RangeSet_GetIndices( shadows[d_i][p_i], &nEls, &els );
			for( e_i = 0; e_i < nEls; e_i++ )
				els[e_i] = Decomp_Sync_GlobalToDomain( self->domains[d_i], els[e_i] );

			/* Sum the data size for this dimension. */
			(*incDataSizes)[d_i][p_i] = 1;
			for( e_i = 0; e_i < nEls; e_i++ )
				(*incDataSizes)[d_i][p_i] += self->nIncEls[d_i][d_i - 1][els[e_i]] + 2;

			/* Allocate for incidence data. */
			(*incData)[d_i][p_i] = Memory_Alloc_Array_Unnamed( unsigned, (*incDataSizes)[d_i][p_i] );

			/* Store data. */
			(*incData)[d_i][p_i][offs++] = nEls;
			for( e_i = 0; e_i < nEls; e_i++ ) {
				unsigned	nIncEls = self->nIncEls[d_i][d_i - 1][els[e_i]];
				unsigned*	incEls = self->incEls[d_i][d_i - 1][els[e_i]];
				unsigned	gElInd = Decomp_Sync_DomainToGlobal( self->domains[d_i], els[e_i] );
				unsigned	inc_i;

				(*incData)[d_i][p_i][offs++] = gElInd;
				(*incData)[d_i][p_i][offs++] = nIncEls;
				for( inc_i = 0; inc_i < nIncEls; inc_i++ )
					(*incData)[d_i][p_i][offs++] = Decomp_Sync_DomainToGlobal( self->domains[d_i - 1], 
												   incEls[inc_i] );
			}

			/* Free the indices. */
			FreeArray( els );
		}
	}
}

void MeshTopology_SendRecvShadows( void* meshTopology, RangeSet*** shadows, 
				   unsigned** incDataSizes, unsigned*** incData )
{
	MeshTopology*	self = (MeshTopology*)meshTopology;
	CommTopology*	commTopo;
	unsigned	nIncRanks;
	unsigned*	nSrcBytes;
	Stg_Byte**	srcBytes;
	unsigned*	nDstBytes;
	Stg_Byte**	dstBytes;
	unsigned**	dataSize;
	unsigned***	data;
	unsigned	p_i, d_i;

	assert( self );
	assert( shadows );
	assert( incDataSizes );
	assert( incData );

	/* Shortcuts. */
	commTopo = self->domains[MT_VERTEX]->commTopo;
	nIncRanks = commTopo->nInc;

	/* Convert downward incidence relations to global indices. */
	for( d_i = 1; d_i < self->nTDims; d_i++ ) {
		unsigned	e_i;

		for( e_i = 0; e_i < self->nDomainEls[d_i]; e_i++ ) {
			unsigned	nIncEls = self->nIncEls[d_i][d_i - 1][e_i];
			unsigned*	incEls = self->incEls[d_i][d_i - 1][e_i];
			unsigned	inc_i;

			for( inc_i = 0; inc_i < nIncEls; inc_i++ )
				incEls[inc_i] = Decomp_Sync_DomainToGlobal( self->domains[d_i - 1], incEls[inc_i] );
		}
	}

	/* Pickle all the range sets and pipe them off to our neighbours. */
	nSrcBytes = Memory_Alloc_Array_Unnamed( unsigned, nIncRanks );
	srcBytes = Memory_Alloc_Array_Unnamed( Stg_Byte*, nIncRanks );
	for( d_i = 0; d_i < self->nTDims; d_i++ ) {
		for( p_i = 0; p_i < nIncRanks; p_i++ )
			RangeSet_Pickle( shadows[d_i][p_i], nSrcBytes + p_i, srcBytes + p_i );
		CommTopology_Alltoall( commTopo, nSrcBytes, (void**)srcBytes, 
				       &nDstBytes, (void***)&dstBytes, sizeof(Stg_Byte) );
		for( p_i = 0; p_i < nIncRanks; p_i++ ) {
			FreeArray( srcBytes[p_i] );
			RangeSet_Unpickle( shadows[d_i][p_i], nDstBytes[p_i], dstBytes[p_i] );
		}
		FreeArray( nDstBytes );
		FreeArray( dstBytes );

		/* Take the union of all other procs' shadow elements. */
		for( p_i = 1; p_i < nIncRanks; p_i++ ) {
			RangeSet_Union( shadows[d_i][0], shadows[d_i][p_i] );
			FreeObject( shadows[d_i][p_i] );
		}
	}

	/* Send and recieve all the incidence relations. */
	dataSize = Memory_Alloc_Array_Unnamed( unsigned*, self->nTDims );
	data = Memory_Alloc_Array_Unnamed( unsigned**, self->nTDims );
	for( d_i = 1; d_i < self->nTDims; d_i++ ) {
		/* Communicate. */
		CommTopology_Alltoall( commTopo, incDataSizes[d_i], (void**)incData[d_i], 
				       dataSize + d_i, (void***)(data + d_i), sizeof(unsigned) );

		/* Free old incidence relations. */
		for( p_i = 0; p_i < nIncRanks; p_i++ )
			FreeArray( incData[d_i][p_i] );
	}
	FreeArray( incData );
	FreeArray( incDataSizes );

	/* Update remote elements. */
	for( d_i = 0; d_i < self->nTDims; d_i++ ) {
		unsigned	nRemotes;
		unsigned*	remotes;

		/* Set the remote elements. */
		RangeSet_GetIndices( shadows[d_i][0], &nRemotes, &remotes );
		Decomp_Sync_SetRemotes( self->domains[d_i], nRemotes, remotes );
		self->nDomainEls[d_i] = self->domains[d_i]->decomp->nLocals + nRemotes;
		FreeArray( remotes );

		/* Free range set. */
		FreeObject( shadows[d_i][0] );
	}
	FreeArray( nSrcBytes );
	FreeArray( srcBytes );
	FreeArray( shadows );

	/* Update incidence. */
	for( d_i = 1; d_i < self->nTDims; d_i++ ) {
		unsigned**	oldInc;
		unsigned	e_i;

		/* Resize old incidence relations to include new remote sizes. */
		self->nIncEls[d_i][d_i - 1] = Memory_Realloc_Array( self->nIncEls[d_i][d_i - 1], 
								    unsigned, self->nDomainEls[d_i] );

		/* Insert each other procs' incidence relations. */
		for( p_i = 0; p_i < nIncRanks; p_i++ ) {
			unsigned*	curData = data[d_i][p_i];
			unsigned	nEls = curData[0];

			curData++;
			for( e_i = 0; e_i < nEls; e_i++ ) {
				unsigned	dInd = Decomp_Sync_GlobalToDomain( self->domains[d_i], curData[0] );

				self->nIncEls[d_i][d_i - 1][dInd] = curData[1];
				curData += curData[1] + 2;
			}
		}

		/* Resize incidence relations. */
		oldInc = self->incEls[d_i][d_i - 1];
		self->incEls[d_i][d_i - 1] = Memory_Alloc_2DComplex( unsigned, self->nDomainEls[d_i], self->nIncEls[d_i][d_i - 1], 
								     "MeshTopology::incEls[][]" );
		for( e_i = 0; e_i < MeshTopology_GetLocalSize( self, d_i ); e_i++ ) {
			memcpy( self->incEls[d_i][d_i - 1][e_i], oldInc[e_i], 
				self->nIncEls[d_i][d_i - 1][e_i] * sizeof(unsigned) );
		}
		FreeArray( oldInc );

		/* Copy new relations. */
		for( p_i = 0; p_i < nIncRanks; p_i++ ) {
			unsigned*	curData = data[d_i][p_i];
			unsigned	nEls = curData[0];

			curData++;
			for( e_i = 0; e_i < nEls; e_i++ ) {
				unsigned	dInd = Decomp_Sync_GlobalToDomain( self->domains[d_i], curData[0] );
				unsigned	inc_i;

				for( inc_i = 0; inc_i < curData[1]; inc_i++ )
					self->incEls[d_i][d_i - 1][dInd][inc_i] = curData[2 + inc_i];
				curData += curData[1] + 2;
			}
		}

		/* Free data and sizes. */
		FreeArray( data[d_i] );
		FreeArray( dataSize[d_i] );
	}
	FreeArray( data );
	FreeArray( dataSize );

	/* Convert downward incidence relations to back to domain indices. */
	for( d_i = 1; d_i < self->nTDims; d_i++ ) {
		unsigned	e_i;

		for( e_i = 0; e_i < self->nDomainEls[d_i]; e_i++ ) {
			unsigned	nIncEls = self->nIncEls[d_i][d_i - 1][e_i];
			unsigned*	incEls = self->incEls[d_i][d_i - 1][e_i];
			unsigned	inc_i;

			for( inc_i = 0; inc_i < nIncEls; inc_i++ )
				incEls[inc_i] = Decomp_Sync_GlobalToDomain( self->domains[d_i - 1], incEls[inc_i] );
		}
	}
}

void MeshTopology_Destruct( MeshTopology* self ) {
	unsigned	d_i;

	assert( self );

	KillArray( self->nRentals );
	KillArray( self->rentals );
	KillArray( self->nDomainEls );
	self->shadowDepth = 0;

	for( d_i = 0; d_i < self->nTDims; d_i++ ) {
		unsigned	d_j;

		FreeObject( self->domains[d_i]->decomp );
		FreeObject( self->domains[d_i] );

		for( d_j = 0; d_j < self->nTDims; d_j++ ) {
			KillArray( self->incEls[d_i][d_j] );
			KillArray( self->nIncEls[d_i][d_j] );
		}
	}
	KillArray( self->incEls );
	KillArray( self->nIncEls );
	KillArray( self->domains );
}

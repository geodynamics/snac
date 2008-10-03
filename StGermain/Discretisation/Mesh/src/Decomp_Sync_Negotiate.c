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
** $Id: Decomp_Sync_Negotiate.c 3584 2006-05-16 11:11:07Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <mpi.h>

#include "Base/Base.h"

#include "types.h"
#include "shortcuts.h"
#include "CommTopology.h"
#include "Decomp.h"
#include "Decomp_Sync.h"
#include "Decomp_Sync_Negotiate.h"


/* Textual name of this class */
const Type Decomp_Sync_Negotiate_Type = "Decomp_Sync_Negotiate";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

Decomp_Sync_Negotiate* Decomp_Sync_Negotiate_New( Name name ) {
	return _Decomp_Sync_Negotiate_New( sizeof(Decomp_Sync_Negotiate), 
					   Decomp_Sync_Negotiate_Type, 
					   _Decomp_Sync_Negotiate_Delete, 
					   _Decomp_Sync_Negotiate_Print, 
					   _Decomp_Sync_Negotiate_Copy, 
					   (void* (*)(Name))_Decomp_Sync_Negotiate_New, 
					   _Decomp_Sync_Negotiate_Construct, 
					   _Decomp_Sync_Negotiate_Build, 
					   _Decomp_Sync_Negotiate_Initialise, 
					   _Decomp_Sync_Negotiate_Execute, 
					   _Decomp_Sync_Negotiate_Destroy, 
					   name, 
					   NON_GLOBAL, 
					   Decomp_Sync_Negotiate_Select );
}

Decomp_Sync_Negotiate* _Decomp_Sync_Negotiate_New( DECOMP_SYNC_NEGOTIATE_DEFARGS ) {
	Decomp_Sync_Negotiate* self;
	
	/* Allocate memory */
	assert( sizeOfSelf >= sizeof(Decomp_Sync_Negotiate) );
	self = (Decomp_Sync_Negotiate*)_Stg_Component_New( STG_COMPONENT_PASSARGS );

	/* Virtual info */
	self->selectFunc = selectFunc;

	/* Decomp_Sync_Negotiate info */
	_Decomp_Sync_Negotiate_Init( self );

	return self;
}

void _Decomp_Sync_Negotiate_Init( Decomp_Sync_Negotiate* self ) {
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _Decomp_Sync_Negotiate_Delete( void* negotiate ) {
	Decomp_Sync_Negotiate*	self = (Decomp_Sync_Negotiate*)negotiate;

	/* Delete the parent. */
	_Stg_Component_Delete( self );
}

void _Decomp_Sync_Negotiate_Print( void* negotiate, Stream* stream ) {
	Decomp_Sync_Negotiate*	self = (Decomp_Sync_Negotiate*)negotiate;
	
	/* Set the Journal for printing informations */
	Stream* negotiateStream;
	negotiateStream = Journal_Register( InfoStream_Type, "Decomp_Sync_NegotiateStream" );

	/* Print parent */
	Journal_Printf( stream, "Decomp_Sync_Negotiate (ptr): (%p)\n", self );
	_Stg_Component_Print( self, stream );
}

void* _Decomp_Sync_Negotiate_Copy( void* negotiate, void* destProc_I, Bool deep, Name nameExt, PtrMap* ptrMap ) {
#if 0
	Decomp_Sync_Negotiate*	self = (Decomp_Sync_Negotiate*)negotiate;
	Decomp_Sync_Negotiate*	newDecomp_Sync_Negotiate;
	PtrMap*	map = ptrMap;
	Bool	ownMap = False;

	/* Damn me for making copying so difficult... what was I thinking? */
	
	/* We need to create a map if it doesn't already exist. */
	if( !map ) {
		map = PtrMap_New( 10 );
		ownMap = True;
	}
	
	newDecomp_Sync_Negotiate = (Decomp_Sync_Negotiate*)_Mesh_Copy( self, destProc_I, deep, nameExt, map );
	
	/* Copy the virtual methods here. */

	/* Deep or shallow? */
	if( deep ) {
	}
	else {
	}
	
	/* If we own the map, get rid of it here. */
	if( ownMap ) Stg_Class_Delete( map );
	
	return (void*)newDecomp_Sync_Negotiate;
#endif

	return NULL;
}

void _Decomp_Sync_Negotiate_Construct( void* negotiate, Stg_ComponentFactory* cf, void* data ) {
}

void _Decomp_Sync_Negotiate_Build( void* negotiate, void* data ) {
}

void _Decomp_Sync_Negotiate_Initialise( void* negotiate, void* data ) {
}

void _Decomp_Sync_Negotiate_Execute( void* negotiate, void* data ) {
}

void _Decomp_Sync_Negotiate_Destroy( void* negotiate, void* data ) {
}

void Decomp_Sync_Negotiate_Select( void* negotiate, Decomp_Sync* sync ) {
	Decomp_Sync_Negotiate*	self = (Decomp_Sync_Negotiate*)negotiate;
	CommTopology*		commTopo;
	unsigned		rank;
	unsigned		nInc;
	unsigned*		inc;
	unsigned*		nFound;
	unsigned**		found;
	unsigned*		nUniques;
	unsigned**		uniques;
	unsigned		mostUniques;
	unsigned**		srcs;
	unsigned		p_i;

	assert( self );
	assert( sync );
	assert( sync->commTopo );

	/* Shortcuts. */
	commTopo = sync->commTopo;
	MPI_Comm_rank( commTopo->comm, (int*)&rank );

	/* Locate remote indices. */
	Decomp_Sync_Negotiate_RemoteSearch( self, sync, &nFound, &found );

	/* Get processor incidence. */
	CommTopology_GetIncidence( commTopo, rank, &nInc, &inc );

	if( nInc ) {
		/* Set up the source arrays. */
		sync->nSrcs = Memory_Alloc_Array( unsigned, nInc, "Decomp_Sync::nSrcs" );
		srcs = Memory_Alloc_Array_Unnamed( unsigned*, nInc );
		memset( sync->nSrcs, 0, nInc * sizeof(unsigned) );
		memset( srcs, 0, nInc * sizeof(unsigned*) );

		/* Select source procs from which to retrieve required nodes such that communications are minimized. */
		nUniques = Memory_Alloc_Array_Unnamed( unsigned, nInc );
		uniques = Memory_Alloc_2DComplex_Unnamed( unsigned, nInc, nFound );
		memset( nUniques, 0, nInc * sizeof(unsigned) );

		do {
			unsigned	mostProc = 0;

			/* Loop over procs and find unique nodes per proc. ie. the most nodes not already covered by
			   an existing source. */
			mostUniques = 0;
			for( p_i = 0; p_i < nInc; p_i++ ) {
				unsigned	fnd_i;

				/* Clear the number of uniques. */
				nUniques[p_i] = 0;

				/* If there are no founds for this proc or we've already sourced it, then skip. */
				if( nFound[p_i] == 0 || sync->nSrcs[p_i] > 0 )
					continue;

				/* Hunt down unique global indices. */
				for( fnd_i = 0; fnd_i < nFound[p_i]; fnd_i++ ) {
					unsigned	p_j;

					for( p_j = 0; p_j < nInc; p_j++ ) {
						unsigned	src_i;

						for( src_i = 0; src_i < sync->nSrcs[p_i]; src_i++ ) {
							if( srcs[p_i][src_i] == found[p_i][fnd_i] )
								break;
						}
						if( src_i < sync->nSrcs[p_i] )
							break;
					}
					if( p_j == nInc )
						uniques[p_i][nUniques[p_i]++] = found[p_i][fnd_i];
				}

				/* Determine which proc has the most uniques and store. */
				if( nUniques[p_i] > mostUniques ) {
					mostUniques = nUniques[p_i];
					mostProc = p_i;
				}
			}

			/* Store result. */
			if( mostUniques ) {
				sync->nSrcs[mostProc] = mostUniques;
				if( mostUniques ) {
					srcs[mostProc] = Memory_Alloc_Array_Unnamed( unsigned, mostUniques );
					memcpy( srcs[mostProc], uniques[mostProc], mostUniques * sizeof(unsigned) );
				}
			}
		}
		while( mostUniques );

		/* Proper allocation. */
		sync->srcs = Memory_Alloc_2DComplex( unsigned, nInc, sync->nSrcs, "Decomp_Sync::srcs" );
		for( p_i = 0; p_i < nInc; p_i++ ) {
			if( sync->nSrcs[p_i] )
				memcpy( sync->srcs[p_i], srcs[p_i], sync->nSrcs[p_i] * sizeof(unsigned) );
		}

		/* Get rid of all the resources so far. */
		FreeArray2D( nInc, srcs );
		FreeArray( nUniques );
		FreeArray( uniques );

		/* Communicate back to all procs our selection.  Note that even though most procs will not need to send
		   anything here, we still need to let them know that. */
		CommTopology_Alltoall( commTopo, 
				       sync->nSrcs, (void**)sync->srcs, 
				       &sync->nSnks, (void***)&sync->snks, 
				       sizeof(unsigned) );

		/* Calculate net values for source and sink. */
		for( p_i = 0; p_i < nInc; p_i++ ) {
			sync->netSrcs += sync->nSrcs[p_i];
			sync->netSnks += sync->nSnks[p_i];
		}
	}

	/* Return incidence. */
	CommTopology_ReturnIncidence( commTopo, rank, &nInc, &inc );
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

void Decomp_Sync_Negotiate_RemoteSearch( Decomp_Sync_Negotiate* self, Decomp_Sync* sync, 
					 unsigned** nRemFound, unsigned*** remFound )
{
	unsigned*	nFound;
	unsigned**	found;
	RangeSet*	rSet;
	unsigned	nBytes;
	Stg_Byte*	bytes;
	unsigned*	remNBytes;
	unsigned**	remBytes;
	RangeSet**	remSets;
	unsigned	p_i;

	assert( self );
	assert( sync );
	assert( nRemFound );
	assert( remFound );

	if( sync->commTopo->nInc ) {
		/* Build a range set of our remotes and pickle. */
		rSet = RangeSet_New();
		RangeSet_SetIndices( rSet, sync->nRemotes, sync->remotes );
		RangeSet_Pickle( rSet, &nBytes, &bytes );

		/* Broadcast our remotes to neighbours. */
		CommTopology_Allgather( sync->commTopo, 
					nBytes, bytes, 
					&remNBytes, (void***)&remBytes, 
					sizeof(Stg_Byte) );

		/* Free bytes. */
		FreeArray( bytes );

		/* Unpickle range sets. */
		remSets = Memory_Alloc_Array_Unnamed( RangeSet*, sync->commTopo->nInc );
		for( p_i = 0; p_i < sync->commTopo->nInc; p_i++ ) {
			remSets[p_i] = RangeSet_New();
			RangeSet_Unpickle( remSets[p_i], remNBytes[p_i], (void*)remBytes[p_i] );
		}

		/* Free remote bytes. */
		FreeArray( remNBytes );
		FreeArray( remBytes );

		/* Replace our remote range set with a local one, then intersect remote sets. */
		RangeSet_SetIndices( rSet, sync->decomp->nLocals, sync->decomp->locals );
		nFound = Memory_Alloc_Array_Unnamed( unsigned, sync->commTopo->nInc );
		found = Memory_Alloc_Array_Unnamed( unsigned*, sync->commTopo->nInc );
		for( p_i = 0; p_i < sync->commTopo->nInc; p_i++ ) {
			RangeSet_Intersection( remSets[p_i], rSet );
			RangeSet_GetIndices( remSets[p_i], nFound + p_i, found + p_i );
			FreeObject( remSets[p_i] );
		}

		/* Free remote set array. */
		FreeArray( remSets );

		/* Send back all the ones we found and receive from all others all our requireds they found. */
		CommTopology_Alltoall( sync->commTopo, 
				       nFound, (void**)found, 
				       nRemFound, (void***)remFound, 
				       sizeof(unsigned) );

		/* Release some memory. */
		FreeArray( nFound );
		FreeArray2D( sync->commTopo->nInc, found );
	}
	else {
		*nRemFound = 0;
		*remFound = NULL;
	}
}

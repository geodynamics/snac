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
** $Id: Decomp_Sync_Claim.c 3584 2006-05-16 11:11:07Z PatrickSunter $
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
#include "Decomp_Sync_Claim.h"


/* Textual name of this class */
const Type Decomp_Sync_Claim_Type = "Decomp_Sync_Claim";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

Decomp_Sync_Claim* Decomp_Sync_Claim_New( Name name ) {
	return _Decomp_Sync_Claim_New( sizeof(Decomp_Sync_Claim), 
				       Decomp_Sync_Claim_Type, 
				       _Decomp_Sync_Claim_Delete, 
				       _Decomp_Sync_Claim_Print, 
				       _Decomp_Sync_Claim_Copy, 
				       (void* (*)(Name))_Decomp_Sync_Claim_New, 
				       _Decomp_Sync_Claim_Construct, 
				       _Decomp_Sync_Claim_Build, 
				       _Decomp_Sync_Claim_Initialise, 
				       _Decomp_Sync_Claim_Execute, 
				       _Decomp_Sync_Claim_Destroy, 
				       name, 
				       NON_GLOBAL, 
				       Decomp_Sync_Claim_Select );
}

Decomp_Sync_Claim* _Decomp_Sync_Claim_New( DECOMP_SYNC_CLAIM_DEFARGS ) {
	Decomp_Sync_Claim* self;
	
	/* Allocate memory */
	assert( sizeOfSelf >= sizeof(Decomp_Sync_Claim) );
	self = (Decomp_Sync_Claim*)_Stg_Component_New( STG_COMPONENT_PASSARGS );

	/* Virtual info */
	self->selectFunc = selectFunc;

	/* Decomp_Sync_Claim info */
	_Decomp_Sync_Claim_Init( self );

	return self;
}

void _Decomp_Sync_Claim_Init( Decomp_Sync_Claim* self ) {
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _Decomp_Sync_Claim_Delete( void* claim ) {
	Decomp_Sync_Claim*	self = (Decomp_Sync_Claim*)claim;

	/* Delete the parent. */
	_Stg_Component_Delete( self );
}

void _Decomp_Sync_Claim_Print( void* claim, Stream* stream ) {
	Decomp_Sync_Claim*	self = (Decomp_Sync_Claim*)claim;
	
	/* Set the Journal for printing informations */
	Stream* claimStream;
	claimStream = Journal_Register( InfoStream_Type, "Decomp_Sync_ClaimStream" );

	/* Print parent */
	Journal_Printf( stream, "Decomp_Sync_Claim (ptr): (%p)\n", self );
	_Stg_Component_Print( self, stream );
}

void* _Decomp_Sync_Claim_Copy( void* claim, void* destProc_I, Bool deep, Name nameExt, PtrMap* ptrMap ) {
#if 0
	Decomp_Sync_Claim*	self = (Decomp_Sync_Claim*)claim;
	Decomp_Sync_Claim*	newDecomp_Sync_Claim;
	PtrMap*	map = ptrMap;
	Bool	ownMap = False;

	/* Damn me for making copying so difficult... what was I thinking? */
	
	/* We need to create a map if it doesn't already exist. */
	if( !map ) {
		map = PtrMap_New( 10 );
		ownMap = True;
	}
	
	newDecomp_Sync_Claim = (Decomp_Sync_Claim*)_Mesh_Copy( self, destProc_I, deep, nameExt, map );
	
	/* Copy the virtual methods here. */

	/* Deep or shallow? */
	if( deep ) {
	}
	else {
	}
	
	/* If we own the map, get rid of it here. */
	if( ownMap ) Stg_Class_Delete( map );
	
	return (void*)newDecomp_Sync_Claim;
#endif

	return NULL;
}

void _Decomp_Sync_Claim_Construct( void* claim, Stg_ComponentFactory* cf, void* data ) {
}

void _Decomp_Sync_Claim_Build( void* claim, void* data ) {
}

void _Decomp_Sync_Claim_Initialise( void* claim, void* data ) {
}

void _Decomp_Sync_Claim_Execute( void* claim, void* data ) {
}

void _Decomp_Sync_Claim_Destroy( void* claim, void* data ) {
}

void Decomp_Sync_Claim_Select( void* claim, Decomp_Sync* sync, unsigned nRequired, unsigned* required, 
			       unsigned* nLocals, unsigned** locals )
{
	Decomp_Sync_Claim*	self = (Decomp_Sync_Claim*)claim;
	RangeSet*	lSet;

	assert( self );
	assert( !nRequired || required );

	/* Build a range set of local indices. */
	lSet = RangeSet_New();
	RangeSet_SetIndices( lSet, nRequired, required );

	/* Determine ownership. */
	Decomp_Sync_Claim_ClaimOwnership( self, sync->commTopo, nRequired, required, lSet, sync->isects, 
					  nLocals, locals, &sync->nRemotes, &sync->remotes );

	/* Free the local set. */
	FreeObject( lSet );
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

void Decomp_Sync_Claim_ClaimOwnership( Decomp_Sync_Claim* self, CommTopology* topo, unsigned nRequired, unsigned* required, 
				       RangeSet* lSet, RangeSet** isects, 
				       unsigned* nLocals, unsigned** locals, unsigned* nRemotes, unsigned** remotes )
{
	unsigned	rank, nProcs;
	unsigned	nInc;
	unsigned*	inc;
	unsigned	nBytes;
	Stg_Byte*		bytes;
	RangeSet*	tmpClaimed;
	unsigned	tag = 6669;
	unsigned	p_i, p_j;

	/* Get basic MPI info. */
	MPI_Comm_rank( topo->comm, (int*)&rank );
	MPI_Comm_size( topo->comm, (int*)&nProcs );

	/* Extract our neighbouring processors. */
	CommTopology_GetIncidence( topo, rank, &nInc, &inc );

	/* Figure out where info is coming from and going to. Note that the incidence
	   is always ordered from lowest to highest rank. */
	tmpClaimed = RangeSet_New();
	for( p_i = 0; p_i < nInc; p_i++ ) {
		MPI_Status	status;

		if( inc[p_i] > rank )
			break;

		/* Receive from neighbour which indices it has taken. */
		MPI_Recv( &nBytes, 1, MPI_UNSIGNED, inc[p_i], tag, topo->comm, &status );
		bytes = Memory_Alloc_Array_Unnamed( Stg_Byte, nBytes );
		MPI_Recv( bytes, nBytes, MPI_BYTE, inc[p_i], tag, topo->comm, &status );
		RangeSet_Unpickle( tmpClaimed, nBytes, bytes );
		FreeArray( bytes );

		/* Subtract from our claimed set. */
		RangeSet_Subtraction( lSet, tmpClaimed );
	}
	FreeObject( tmpClaimed );

	/* Extract our ownership. */
	Decomp_Sync_Claim_BuildIndices( self, nRequired, required, lSet, 
					nLocals, locals, nRemotes, remotes );

	/* Update remaining neighbours as to which indices we've taken. */
	for( p_j = p_i; p_j < nInc; p_j++ ) {
		RangeSet*	intersect;

		intersect = RangeSet_DeepCopy( isects[p_j] );
		RangeSet_Intersection( intersect, lSet );
		RangeSet_Pickle( intersect, &nBytes, &bytes );
		MPI_Send( &nBytes, 1, MPI_UNSIGNED, inc[p_j], tag, topo->comm );
		MPI_Send( bytes, nBytes, MPI_BYTE, inc[p_j], tag, topo->comm );
		FreeArray( bytes );
	}

	/* Return incidence. */
	CommTopology_ReturnIncidence( topo, rank, &nInc, &inc );
}

void Decomp_Sync_Claim_BuildIndices( Decomp_Sync_Claim* self, unsigned nRequired, unsigned* required, RangeSet* claimed, 
				     unsigned* nLocals, unsigned** locals, unsigned* nRemotes, unsigned** remotes )
{
	RangeSet*	rSet;

	/* Extract indices. */
	RangeSet_GetIndices( claimed, nLocals, locals );

	/* Build a set of remotes. */
	rSet = RangeSet_New();
	RangeSet_SetIndices( rSet, nRequired, required );
	RangeSet_Subtraction( rSet, claimed );
	RangeSet_GetIndices( rSet, nRemotes, remotes );

	/* Destroy rental set. */
	FreeObject( rSet );
}

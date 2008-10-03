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
** $Id: Decomp.c 3584 2006-05-16 11:11:07Z PatrickSunter $
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
#include "Decomp_Sync.h"
#include "Decomp.h"


/* Textual name of this class */
const Type Decomp_Type = "Decomp";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

Decomp* Decomp_New( Name name ) {
	return _Decomp_New( sizeof(Decomp), 
			    Decomp_Type, 
			    _Decomp_Delete, 
			    _Decomp_Print, 
			    _Decomp_Copy, 
			    (void* (*)(Name))_Decomp_New, 
			    _Decomp_Construct, 
			    _Decomp_Build, 
			    _Decomp_Initialise, 
			    _Decomp_Execute, 
			    _Decomp_Destroy, 
			    name, 
			    NON_GLOBAL );
}

Decomp* _Decomp_New( DECOMP_DEFARGS ) {
	Decomp* self;
	
	/* Allocate memory */
	assert( sizeOfSelf >= sizeof(Decomp) );
	self = (Decomp*)_Stg_Component_New( STG_COMPONENT_PASSARGS );

	/* Virtual info */

	/* Decomp info */
	_Decomp_Init( self );

	return self;
}

void _Decomp_Init( Decomp* self ) {
	self->comm = MPI_COMM_WORLD;

	self->nGlobals = 0;
	self->nLocals = 0;
	self->locals = NULL;

	self->glMap = NULL;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _Decomp_Delete( void* decomp ) {
	Decomp*	self = (Decomp*)decomp;

	Decomp_Destruct( self );

	/* Delete the parent. */
	_Stg_Component_Delete( self );
}

void _Decomp_Print( void* decomp, Stream* stream ) {
	Decomp*	self = (Decomp*)decomp;
	
	/* Set the Journal for printing informations */
	Stream* decompStream;
	decompStream = Journal_Register( InfoStream_Type, "DecompStream" );

	/* Print parent */
	Journal_Printf( stream, "Decomp (ptr): (%p)\n", self );
	_Stg_Component_Print( self, stream );
}

void* _Decomp_Copy( void* decomp, void* destProc_I, Bool deep, Name nameExt, PtrMap* ptrMap ) {
#if 0
	Decomp*	self = (Decomp*)decomp;
	Decomp*	newDecomp;
	PtrMap*	map = ptrMap;
	Bool	ownMap = False;

	/* Damn me for making copying so difficult... what was I thinking? */
	
	/* We need to create a map if it doesn't already exist. */
	if( !map ) {
		map = PtrMap_New( 10 );
		ownMap = True;
	}
	
	newDecomp = (Decomp*)_Mesh_Copy( self, destProc_I, deep, nameExt, map );
	
	/* Copy the virtual methods here. */

	/* Deep or shallow? */
	if( deep ) {
	}
	else {
	}
	
	/* If we own the map, get rid of it here. */
	if( ownMap ) Stg_Class_Delete( map );
	
	return (void*)newDecomp;
#endif

	return NULL;
}

void _Decomp_Construct( void* decomp, Stg_ComponentFactory* cf, void* data ) {
}

void _Decomp_Build( void* decomp, void* data ) {
}

void _Decomp_Initialise( void* decomp, void* data ) {
}

void _Decomp_Execute( void* decomp, void* data ) {
}

void _Decomp_Destroy( void* decomp, void* data ) {
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

#define DECOMP_OK		0x0000
#define DECOMP_OVERLAPPING	0x0001
#define DECOMP_INCOMPLETE	0x0002

void Decomp_SetComm( void* decomp, MPI_Comm comm ) {
	Decomp*	self = (Decomp*)decomp;

	assert( self );

	Decomp_Destruct( self );

	self->comm = comm;
}

void Decomp_SetLocals( void* decomp, unsigned nLocals, unsigned* locals ) {
	Decomp*		self = (Decomp*)decomp;
	unsigned	status;

	assert( self );
	assert( !nLocals || locals );

	Decomp_DestructLocals( self );

	/* Store local information. */
	self->nLocals = nLocals;
	self->locals = Memory_Alloc_Array( unsigned, nLocals, "Decomp::locals" );
	memcpy( self->locals, locals, nLocals * sizeof(unsigned) );

	/* Validate the domain. */
	Decomp_ValidateDomain( self, &status );
	assert( status == DECOMP_OK );

	/* Build global to local map. */
	Decomp_BuildGLMap( self );
}

void Decomp_AddSync( void* decomp, Decomp_Sync* sync ) {
	Decomp*		self = (Decomp*)decomp;

	assert( self );
	assert( sync );

	if( !self->nSyncs )
		self->syncs = Memory_Alloc_Array( Decomp_Sync*, ++self->nSyncs, "Decomp::syncs" );
	else
		self->syncs = Memory_Realloc_Array( self->syncs, Decomp_Sync*, ++self->nSyncs );
	self->syncs[self->nSyncs - 1] = sync;
}

void Decomp_RemoveSync( void* decomp, Decomp_Sync* sync ) {
	Decomp*		self = (Decomp*)decomp;
	unsigned	s_i = 0;

	assert( self );
	assert( sync );

	while( self->syncs[s_i++] != sync && s_i < self->nSyncs );
	assert( s_i <= self->nSyncs );
	for( ; s_i < self->nSyncs; s_i++ )
		self->syncs[s_i - 1] = self->syncs[s_i];

	if( --self->nSyncs == 0 ) {
		KillArray( self->syncs );
	}
	else
		self->syncs = Memory_Realloc_Array( self->syncs, Decomp_Sync*, self->nSyncs );
}

Bool Decomp_IsLocal( void* decomp, unsigned global ) {
	Decomp*	self = (Decomp*)decomp;

	assert( self );
	assert( global < self->nGlobals );
	assert( self->glMap );

	return UIntMap_HasKey( self->glMap, global );
}

unsigned Decomp_LocalToGlobal( void* decomp, unsigned local ) {
	Decomp*	self = (Decomp*)decomp;

	assert( self );
	assert( local < self->nLocals );
	assert( self->locals );

	return self->locals[local];
}

unsigned Decomp_GlobalToLocal( void* decomp, unsigned global ) {
	Decomp*	self = (Decomp*)decomp;

	assert( self );
	assert( global < self->nGlobals );
	assert( self->glMap );

	return UIntMap_Map( self->glMap, global );
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

void Decomp_BuildGLMap( Decomp* self ) {
	UIntMap*	map;
	unsigned	l_i;

	FreeObject( self->glMap );

	map = UIntMap_New();
	for( l_i = 0; l_i < self->nLocals; l_i++ )
		UIntMap_Insert( map, self->locals[l_i], l_i );
	self->glMap = map;
}

void Decomp_ValidateDomain( Decomp* self, unsigned* status ) {
	unsigned	rank, nProcs;
	unsigned	nBytes;
	Stg_Byte*		bytes;
	unsigned	tag = 6669;
	RangeSet*	lSet;
	RangeSet*	gSet;
	unsigned	netInds;

	assert( self );

	/* Clear the status. */
	*status = DECOMP_OK;

	/* Get basic MPI info. */
	MPI_Comm_rank( self->comm, (int*)&rank );
	MPI_Comm_size( self->comm, (int*)&nProcs );

	/* Create a local range set. */
	lSet = RangeSet_New();
	RangeSet_SetIndices( lSet, self->nLocals, self->locals );

	/* Create a global range set. */
	gSet = RangeSet_New();

	/* Receive from previous rank. */
	if( rank > 0 ) {
		MPI_Status	mpiStatus;

		MPI_Recv( &nBytes, 1, MPI_UNSIGNED, rank - 1, tag, self->comm, &mpiStatus );
		bytes = Memory_Alloc_Array_Unnamed( Stg_Byte, nBytes );
		MPI_Recv( bytes, nBytes, MPI_BYTE, rank - 1, tag, self->comm, &mpiStatus );
		RangeSet_Unpickle( gSet, nBytes, bytes );
		FreeArray( bytes );
	}

	/* Combine sets. */
	RangeSet_Union( gSet, lSet );

	if( rank < nProcs - 1 ) {
		RangeSet_Pickle( gSet, &nBytes, &bytes );
		MPI_Send( &nBytes, 1, MPI_UNSIGNED, rank + 1, tag, self->comm );
		MPI_Send( bytes, nBytes, MPI_BYTE, rank + 1, tag, self->comm );
		FreeArray( bytes );
	}
	else {
		self->nGlobals = RangeSet_GetNIndices( gSet );
		if( RangeSet_GetNRanges( gSet ) > 1 )
			*status |= DECOMP_INCOMPLETE;
	}

	/* Transfer global count to all. */
	MPI_Bcast( &self->nGlobals, 1, MPI_UNSIGNED, nProcs - 1, self->comm );

	/* Check for overlap. */
	MPI_Allreduce( &lSet->nInds, &netInds, 1, MPI_UNSIGNED, MPI_SUM, self->comm );
	if( netInds != self->nGlobals )
		*status |= DECOMP_OVERLAPPING;

	/* Free the sets. */
	FreeObject( lSet );
	FreeObject( gSet );
}

void Decomp_Destruct( Decomp* self ) {
	Decomp_DestructLocals( self );
}

void Decomp_DestructLocals( Decomp* self ) {
	Decomp_DestructSyncs( self );
	self->nLocals = 0;
	KillArray( self->locals );
	KillObject( self->glMap );
}

void Decomp_DestructSyncs( Decomp* self ) {
	unsigned	s_i;
	unsigned	tmpNSyncs;
	Decomp_Sync**	tmpSyncs;

	tmpNSyncs = self->nSyncs;
	tmpSyncs = Memory_Alloc_Array_Unnamed( Decomp_Sync*, tmpNSyncs );
	memcpy( tmpSyncs, self->syncs, tmpNSyncs * sizeof(Decomp_Sync*) );
	for( s_i = 0; s_i < tmpNSyncs; s_i++ )
		Decomp_Sync_SetDecomp( tmpSyncs[s_i], NULL );
}

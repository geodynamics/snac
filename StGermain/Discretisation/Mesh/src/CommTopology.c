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
** $Id: CommTopology.c 3584 2006-05-16 11:11:07Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <mpi.h>

#include "Base/Base.h"
#include "Discretisation/Geometry/Geometry.h"

#include "types.h"
#include "shortcuts.h"
#include "CommTopology.h"


/* Textual name of this class */
const Type CommTopology_Type = "CommTopology";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

CommTopology* CommTopology_New( Name name ) {
	return _CommTopology_New( sizeof(CommTopology), 
				  CommTopology_Type, 
				  _CommTopology_Delete, 
				  _CommTopology_Print, 
				  _CommTopology_Copy, 
				  (void* (*)(Name))_CommTopology_New, 
				  _CommTopology_Construct, 
				  _CommTopology_Build, 
				  _CommTopology_Initialise, 
				  _CommTopology_Execute, 
				  _CommTopology_Destroy, 
				  name, 
				  NON_GLOBAL );
}

CommTopology* _CommTopology_New( COMMTOPOLOGY_DEFARGS ) {
	CommTopology* self;
	
	/* Allocate memory */
	assert( sizeOfSelf >= sizeof(CommTopology) );
	self = (CommTopology*)_Stg_Component_New( STG_COMPONENT_PASSARGS );

	/* Virtual info */

	/* CommTopology info */
	_CommTopology_Init( self );

	return self;
}

void _CommTopology_Init( CommTopology* self ) {
	self->comm = MPI_COMM_WORLD;
	MPI_Comm_size( self->comm, (int*)&self->nProcs );
	MPI_Comm_rank( self->comm, (int*)&self->rank );
	self->nInc = 0;
	self->inc = NULL;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _CommTopology_Delete( void* generator ) {
	CommTopology*	self = (CommTopology*)generator;

	CommTopology_Destruct( self );

	/* Delete the parent. */
	_Stg_Component_Delete( self );
}

void _CommTopology_Print( void* generator, Stream* stream ) {
	CommTopology*	self = (CommTopology*)generator;
	
	/* Set the Journal for printing informations */
	Stream* generatorStream;
	generatorStream = Journal_Register( InfoStream_Type, "CommTopologyStream" );

	/* Print parent */
	Journal_Printf( stream, "CommTopology (ptr): (%p)\n", self );
	_Stg_Component_Print( self, stream );
}

void* _CommTopology_Copy( void* generator, void* destProc_I, Bool deep, Name nameExt, PtrMap* ptrMap ) {
#if 0
	CommTopology*	self = (CommTopology*)generator;
	CommTopology*	newCommTopology;
	PtrMap*	map = ptrMap;
	Bool	ownMap = False;

	/* Damn me for making copying so difficult... what was I thinking? */
	
	/* We need to create a map if it doesn't already exist. */
	if( !map ) {
		map = PtrMap_New( 10 );
		ownMap = True;
	}
	
	newCommTopology = (CommTopology*)_Mesh_Copy( self, destProc_I, deep, nameExt, map );
	
	/* Copy the virtual methods here. */

	/* Deep or shallow? */
	if( deep ) {
	}
	else {
	}
	
	/* If we own the map, get rid of it here. */
	if( ownMap ) Stg_Class_Delete( map );
	
	return (void*)newCommTopology;
#endif

	return NULL;
}

void _CommTopology_Construct( void* generator, Stg_ComponentFactory* cf, void* data ) {
}

void _CommTopology_Build( void* generator, void* data ) {
}

void _CommTopology_Initialise( void* generator, void* data ) {
}

void _CommTopology_Execute( void* generator, void* data ) {
}

void _CommTopology_Destroy( void* generator, void* data ) {
}

/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void CommTopology_SetComm( void* commTopology, MPI_Comm comm ) {
	CommTopology*	self = (CommTopology*)commTopology;

	assert( self );

	CommTopology_Destruct( self );
	self->comm = comm;
	MPI_Comm_size( comm, (int*)&self->nProcs );
	MPI_Comm_rank( comm, (int*)&self->rank );
}


void CommTopology_SetIncidence( void* commTopology, unsigned nInc, unsigned* inc ) {
	CommTopology*	self = (CommTopology*)commTopology;

	assert( self );
	assert( !nInc || inc );
#ifndef NDEBUG
	{
		unsigned	inc_i;

		for( inc_i = 0; inc_i < nInc; inc_i++ ) {
			assert( inc[inc_i] < self->nProcs );
			assert( inc[inc_i] != self->rank );
		}
	}
#endif

	self->nInc = nInc;
	if( nInc ) {
		if( !self->inc )
			self->inc = Memory_Alloc_Array( unsigned, nInc, "CommTopology::inc" );
		else
			self->inc = Memory_Realloc_Array( self->inc, unsigned, nInc );
		memcpy( self->inc, inc, nInc * sizeof(unsigned) );
		qsort( self->inc, nInc, sizeof(unsigned), CommTopology_CmpRanks );
	}
	else {
		KillArray( self->inc );
	}
}


void CommTopology_GetIncidence( void* commTopology, unsigned proc, 
				unsigned* nInc, unsigned** inc )
{
	CommTopology*	self = (CommTopology*)commTopology;

	assert( self );
	assert( proc < self->nProcs );
	assert( nInc && inc );

	if( proc == self->rank ) {
		*nInc = self->nInc;
		*inc = self->inc;
	}
	else {
		fprintf( stderr, "*** Error: Processor topology does not yet support retrieving\n" );
		fprintf( stderr, "***        incidence from other processors.\n" );
		abort();
	}
}


void CommTopology_ReturnIncidence( void* commTopology, unsigned proc, 
				   unsigned* nInc, unsigned** inc )
{
	CommTopology*	self = (CommTopology*)commTopology;

	assert( self );
	assert( proc < self->nProcs );
	assert( nInc && inc );

	if( proc != self->rank )
		KillArray( *inc );
	else
		*inc = NULL;
	*nInc = 0;
}


void CommTopology_Allgather( void* commTopology, 
			     unsigned srcSize, void* srcArray, 
			     unsigned** dstSizes, void*** dstArrays, 
			     unsigned itemSize )
{
	CommTopology*	self = (CommTopology*)commTopology;
	unsigned	tag = 669;
	MPI_Status	status;
	unsigned*	nbrSizes;
	unsigned*	tmpSizes;
	Stg_Byte**	nbrArrays;
	unsigned	p_i;

	assert( self );
	assert( !srcSize || srcArray );
	assert( dstSizes );
	assert( dstArrays );

	/* Skip this if we have no neighbours. */
	if( !self->nInc ) {
		*dstSizes = NULL;
		*dstArrays = NULL;
		return;
	}

	/* Need space for neighbour sizes. */
	nbrSizes = Memory_Alloc_Array_Unnamed( unsigned, self->nInc );
	tmpSizes = Memory_Alloc_Array_Unnamed( unsigned, self->nInc );

	/* Send/recv with each neighbour.  There won't be deadlocks because the neighbouring
	   ranks are ordered from lowest to highest. Start with sizes. */
	for( p_i = 0; p_i < self->nInc; p_i++ ) {
		unsigned	nbr = self->inc[p_i];

		MPI_Sendrecv( &srcSize, 1, MPI_UNSIGNED, nbr, tag, 
			      nbrSizes + p_i, 1, MPI_UNSIGNED, nbr, tag, 
			      self->comm, &status );

		tmpSizes[p_i] = nbrSizes[p_i] * itemSize;
	}

	/* Allocate space for results. */
	nbrArrays = Memory_Alloc_2DComplex_Unnamed( Stg_Byte, self->nInc, tmpSizes );

	/* Transfer arrays. */
	for( p_i = 0; p_i < self->nInc; p_i++ ) {
		unsigned	nbr = self->inc[p_i];

		MPI_Sendrecv( srcArray, srcSize * itemSize, MPI_BYTE, nbr, tag, 
			      nbrArrays[p_i], tmpSizes[p_i], MPI_BYTE, nbr, tag, 
			      self->comm, &status );
	}

	/* Free temporary sizes. */
	FreeArray( tmpSizes );

	/* Store results. */
	*dstSizes = nbrSizes;
	*dstArrays = (void**)nbrArrays;
}


void CommTopology_Alltoall( void* commTopology, 
			    unsigned* srcSizes, void** srcArrays, 
			    unsigned** dstSizes, void*** dstArrays, 
			    unsigned itemSize )
{
	CommTopology*	self = (CommTopology*)commTopology;
	unsigned	tag = 669;
	MPI_Status	status;
	unsigned*	nbrSizes;
	unsigned*	tmpSizes;
	Stg_Byte**	nbrArrays;
	unsigned	p_i;

	assert( self );
	assert( !srcSizes || srcArrays );
	assert( dstSizes );
	assert( dstArrays );

	/* Skip this if we have no neighbours. */
	if( !self->nInc ) {
		*dstSizes = NULL;
		*dstArrays = NULL;
		return;
	}

	/* Need space for neighbour sizes. */
	nbrSizes = Memory_Alloc_Array_Unnamed( unsigned, self->nInc );
	tmpSizes = Memory_Alloc_Array_Unnamed( unsigned, self->nInc );

	/* Send/recv with each neighbour.  There won't be deadlocks because the neighbouring
	   ranks are ordered from lowest to highest. Start with sizes. */
	for( p_i = 0; p_i < self->nInc; p_i++ ) {
		unsigned	nbr = self->inc[p_i];

		MPI_Sendrecv( srcSizes + p_i, 1, MPI_UNSIGNED, nbr, tag, 
			      nbrSizes + p_i, 1, MPI_UNSIGNED, nbr, tag, 
			      self->comm, &status );

		tmpSizes[p_i] = nbrSizes[p_i] * itemSize;
	}

	/* Allocate space for results. */
	nbrArrays = Memory_Alloc_2DComplex_Unnamed( Stg_Byte, self->nInc, tmpSizes );

	/* Transfer arrays. */
	for( p_i = 0; p_i < self->nInc; p_i++ ) {
		unsigned	nbr = self->inc[p_i];

		MPI_Sendrecv( srcArrays[p_i], srcSizes[p_i] * itemSize, MPI_BYTE, nbr, tag, 
			      nbrArrays[p_i], tmpSizes[p_i], MPI_BYTE, nbr, tag, 
			      self->comm, &status );
	}

	/* Free temporary sizes. */
	FreeArray( tmpSizes );

	/* Store results. */
	*dstSizes = nbrSizes;
	*dstArrays = (void**)nbrArrays;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

int CommTopology_CmpRanks( const void* rank0, const void* rank1 ) {
	if( *((unsigned*)rank0) < *((unsigned*)rank1) )
		return -1;
	else if( *((unsigned*)rank0) > *((unsigned*)rank1) )
		return 1;
	else
		return 0;
}

void CommTopology_Destruct( CommTopology* self ) {
	KillArray( self->inc );
	self->nInc = 0;
}

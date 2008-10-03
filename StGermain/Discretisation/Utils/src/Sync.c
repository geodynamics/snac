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
** $Id: Sync.c 2453 2004-12-21 04:49:34Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"
#include "Discretisation/Mesh/Mesh.h"

#include "types.h"
#include "Sync.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


/* Textual name of this class */
const Type Sync_Type = "Sync";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

void* Sync_DefaultNew( Name name ) {
	return _Sync_New( sizeof(Sync), 
			  Sync_Type, 
			  _Sync_Delete, 
			  _Sync_Print,
			  NULL,
			  Sync_DefaultNew,
			  _Sync_Construct,
			  _Sync_Build,
			  _Sync_Initialise,
			  _Sync_Execute,
			  _Sync_Destroy,
			  name,
			  False,
			  NULL );
}


Sync* Sync_New_Param( Dictionary* dictionary, Name name ) {
	return _Sync_New( sizeof(Sync), 
			  Sync_Type, 
			  _Sync_Delete, 
			  _Sync_Print,
			  NULL,
			  Sync_DefaultNew,
			  _Sync_Construct,
			  _Sync_Build,
			  _Sync_Initialise,
			  _Sync_Execute,
			  _Sync_Destroy,
			  name,
			  True,
			  dictionary );
}


void Sync_Init( Sync* self,
		Dictionary* dictionary, Name name )
{
	/* General info */
	self->type = Sync_Type;
	self->_sizeOfSelf = sizeof(Sync);
	self->_deleteSelf = False;
	self->dictionary = dictionary;
	
	/* Virtual info */
	self->_delete = _Sync_Delete;
	self->_print = _Sync_Print;
	self->_copy = NULL;
	self->_defaultConstructor = Sync_DefaultNew;
	self->_construct = _Sync_Construct;
	self->_build = _Sync_Build;
	self->_initialise = _Sync_Initialise;
	self->_execute = _Sync_Execute;
	self->_destroy = _Sync_Destroy;
	
	_Stg_Class_Init( (Stg_Class*)self );
	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );
	_Stg_Component_Init( (Stg_Component*)self );
	
	/* Sync info */
	_Sync_Init( self );
}


Sync* _Sync_New( SizeT						_sizeOfSelf, 
		 Type						type,
		 Stg_Class_DeleteFunction*			_delete,
		 Stg_Class_PrintFunction*			_print,
		 Stg_Class_CopyFunction*			_copy, 
		 Stg_Component_DefaultConstructorFunction*	_defaultConstructor,
		 Stg_Component_ConstructFunction*		_construct,
		 Stg_Component_BuildFunction*			_build,
		 Stg_Component_InitialiseFunction*		_initialise,
		 Stg_Component_ExecuteFunction*			_execute,
		 Stg_Component_DestroyFunction*			_destroy,
		 Name						name,
		 Bool						initFlag,
		 Dictionary*					dictionary )
{
	Sync* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(Sync) );
	self = (Sync*)_Stg_Component_New( 
		_sizeOfSelf, 
		type, 
		_delete, 
		_print, 
		_copy, 
		_defaultConstructor, 
		_construct, 
		_build, 
		_initialise, 
		_execute, 
		_destroy, 
		name, 
		NON_GLOBAL );
	
	/* General info */
	self->dictionary = dictionary;
	
	/* Virtual info */
	
	/* Sync info */
	if( initFlag ){
		_Sync_Init( self );
	}
	
	return self;
}


void _Sync_Init( Sync* self ) {
	/* General and Virtual info should already be set */
	self->isConstructed = True;
	/* Sync info */
	
	self->comm = MPI_COMM_WORLD;
	self->nProcs = 0;
	self->rank = 0;
	
	self->select = _Sync_Select_MinimizeComms;

	self->nGlobal = 0;
	self->nLocal = 0;
	self->local = NULL;
	self->nShared = 0;
	self->shared = NULL;
	self->nShadow = 0;
	self->shadow = NULL;
	self->nDomain = 0;
	self->domain = NULL;

	self->globalMap = NULL;
	
	self->netSource = 0;
	self->nSource = NULL;
	self->source = NULL;
	
	self->netSink = 0;
	self->nSink = NULL;
	self->sink = NULL;
	
	self->sourceDisps = NULL;
	self->sourceSizes = NULL;
	self->sourceOffs = NULL;
	self->sourceArray = NULL;
	self->sinkDisps = NULL;
	self->sinkSizes = NULL;
	self->sinkOffs = NULL;
	self->sinkArray = NULL;

	self->itemSize = 0;
	self->arrayType = Sync_ArrayType_None;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _Sync_Delete( void* sync ) {
	Sync* self = (Sync*)sync;

	_Sync_Free( self );

	/* Stg_Class_Delete parent */
	_Stg_Class_Delete( self );
}


void _Sync_Print( void* sync, Stream* stream ) {
	Sync* self = (Sync*)sync;
	
	/* Print parent */
	_Stg_Class_Print( self, stream );
	
	/* General info */
	Journal_Printf( stream, "Sync (ptr): (%p)\n", self );
	
	/* Virtual info */
	
	/* Sync info */
}


void _Sync_Construct( void* sync, Stg_ComponentFactory *cf, void* data ){
}

	
void _Sync_Build( void* sync, void *data ){
}

	
void _Sync_Initialise( void* sync, void *data ) {
}

	
void _Sync_Execute( void* sync, void *data ){
}


void _Sync_Destroy( void* sync, void *data ){
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void Sync_Negotiate( void* sync, 
		     unsigned nGlobal, 
		     unsigned* local, unsigned nLocal, 
		     unsigned* shared, unsigned nShared, 
		     unsigned* shadow, unsigned nShadow, 
		     MPI_Comm comm )
{
	Sync*		self = (Sync*)sync;
	unsigned*	remNFound;
	unsigned**	remFound;
	unsigned	proc_i;

	assert( sync );
	assert( nGlobal );
	assert( !nLocal || local );
	assert( !nShared || shared );
	assert( !nShadow || shadow );


	/*
	** Check for existing allocated arrays and free them.
	*/

	_Sync_Free( self );


	/*
	** Before anything else, store the parameters and build the global map.
	*/

	self->nGlobal = nGlobal;
	self->nLocal = nLocal;
	self->nShared = nShared;
	self->nShadow = nShadow;
	self->nDomain = nLocal + nShadow;
	if( self->nDomain ) {
		self->domain = Memory_Alloc_Array( unsigned, self->nDomain, "Sync" );
	}
	if( nLocal ) {
		self->local = self->domain;
		memcpy( self->local, local, sizeof(unsigned) * nLocal );
	}
	if( nShadow ) {
		self->shadow = self->domain + nLocal;
		memcpy( self->shadow, shadow, sizeof(unsigned) * nShadow );
	}
	if( nShared ) {
		self->shared = Memory_Alloc_Array( unsigned, nShared, "Sync" );
		memcpy( self->shared, shared, sizeof(unsigned) * nShared );
	}

	/* And build the global map. */
	_Sync_BuildGlobalMap( self );


	/*
	** Collect basic MPI information.
	*/

	self->comm = comm;
	CHK_MPI( MPI_Comm_rank( comm, &self->rank ) );
	CHK_MPI( MPI_Comm_size( comm, &self->nProcs ) );

	if( self->nProcs == 1 ) {
		self->nSource = Memory_Alloc_Array( unsigned, 1, "Sync" );
		self->nSink = Memory_Alloc_Array( unsigned, 1, "Sync" );
		self->nSource[0] = 0;
		self->nSink[0] = 0;
		return;
	}


	/*
	** For each other proc build a set of local that correspond to remotes they will send here.  There will
	** probably be mainly empty sets.  Send back to respective procs.
	*/

	{
		unsigned*	remNReq;
		unsigned**	remReq;
		unsigned*	nFound;
		unsigned**	found;
		IndexSet*	globalSet;

		/* Send our required to all others and receive from all others. */
		MPI_ArrayAllgather( nShadow, sizeof(unsigned), shadow, &remNReq, (void***)&remReq, comm );

		/* Prepare to locate remote requireds. */
		nFound = Memory_Alloc_Array( unsigned, self->nProcs, "Sync" );
		found = Memory_Alloc_Array( unsigned*, self->nProcs, "Sync" );
		globalSet = IndexSet_New( nGlobal );

		/* Begin searching... */
		for( proc_i = 0; proc_i < self->nProcs; proc_i++ ) {
			unsigned	remReq_i;

			if( proc_i == self->rank ) {
				nFound[proc_i] = 0;
				found[proc_i] = NULL;
				continue;
			}

			IndexSet_RemoveAll( globalSet );

			for( remReq_i = 0; remReq_i < remNReq[proc_i]; remReq_i++ ) {
				unsigned	loc_i;

				for( loc_i = 0; loc_i < nLocal; loc_i++ ) {
					if( local[loc_i] == remReq[proc_i][remReq_i] ) {
						IndexSet_Add( globalSet, remReq[proc_i][remReq_i] );
						break;
					}
				}
			}

			IndexSet_GetMembers( globalSet, &nFound[proc_i], &found[proc_i] );
		}

		/* Release some memory. */
		FreeArray( remNReq );
		FreeArray( remReq );
		Stg_Class_Delete( globalSet );

		/* Send back all the ones we found and receive from all others all our requireds they found. */
		MPI_Array2DAlltoall( nFound, sizeof(unsigned), (void**)found, &remNFound, (void***)&remFound, comm );

		/* Release some memory. */
		FreeArray( nFound );
		FreeArray2D( self->nProcs, found );
	}


	/*
	** Execute an algorithm (potentially user supplied) to determine where the requested nodes will
	** come from.
	*/

	self->select( self, remNFound, remFound );

	/* We can now free up some more memory. */
	FreeArray( remNFound );
	FreeArray( remFound );


	/*
	** Communicate back to all procs our selection.  Note that even though most procs will not need to send
	** anything here, we still need to let them know that.
	*/

	/* Allocate for the sinks. */
	self->nSink = Memory_Alloc_Array( unsigned, self->nProcs, "Sync" );
	
	CHK_MPI( MPI_Alltoall( self->nSource, 1, MPI_UNSIGNED, 
			       self->nSink, 1, MPI_UNSIGNED, comm ) );
	MPI_Array2DAlltoall( self->nSource, sizeof(unsigned), (void**)self->source, 
			     &self->nSink, (void***)&self->sink, comm );


	/*
	** Calculate net values for source and sink.
	*/

	self->netSource = 0;
	self->netSink = 0;
	for( proc_i = 0; proc_i < self->nProcs; proc_i++ ) {
		self->netSource += self->nSource[proc_i];
		self->netSink += self->nSink[proc_i];
	}


	/*
	** Complete the mappings, ie. map sink and source arrays from global indices to domain indices.
	*/

	{
		unsigned	snk_i, src_i;

		for( proc_i = 0; proc_i < self->nProcs; proc_i++ ) {
			for( snk_i = 0; snk_i < self->nSink[proc_i]; snk_i++ ) {
				self->sink[proc_i][snk_i] = self->globalMap[self->sink[proc_i][snk_i]];
			}
			for( src_i = 0; src_i < self->nSource[proc_i]; src_i++ ) {
				self->source[proc_i][src_i] = self->globalMap[self->source[proc_i][src_i]];
			}
		}
	}
}


unsigned Sync_GetNGlobal( void* sync ) {
	assert( sync );
	return ((Sync*)sync)->nGlobal;
}


unsigned Sync_GetNLocal( void* sync ) {
	assert( sync );
	return ((Sync*)sync)->nLocal;
}


unsigned Sync_GetNShadow( void* sync ) {
	assert( sync );
	return ((Sync*)sync)->nShadow;
}


unsigned Sync_GetNDomain( void* sync ) {
	assert( sync );
	return ((Sync*)sync)->nDomain;
}


unsigned Sync_MapLocal( void* sync, unsigned lInd ) {
	Sync*	self = (Sync*)sync;

	assert( self );
	assert( !self->nLocal || self->local );
	assert( lInd < self->nLocal );

	return self->local[lInd];
}


unsigned Sync_MapShadow( void* sync, unsigned sInd ) {
	Sync*	self = (Sync*)sync;

	assert( self );
	assert( !self->nShadow || self->shadow );
	assert( sInd < self->nShadow );

	return self->shadow[sInd];
}


unsigned Sync_MapDomain( void* sync, unsigned dInd ) {
	Sync*	self = (Sync*)sync;

	assert( self );
	assert( !self->nDomain || self->domain );
	assert( dInd < self->nDomain );

	return self->domain[dInd];
}


unsigned Sync_MapGlobal( void* sync, unsigned gInd ) {
	Sync*	self = (Sync*)sync;

	assert( self );
	assert( self->globalMap );
	assert( gInd < self->nGlobal );

	return self->globalMap[gInd];
}


Bool Sync_IsLocal( void* sync, unsigned gInd ) {
	Sync*	self = (Sync*)sync;

	assert( self );
	assert( self->globalMap );
	assert( gInd < self->nGlobal );

	return (self->globalMap[gInd] < self->nLocal) ? True : False;
}


Bool Sync_IsShadow( void* sync, unsigned gInd ) {
	Sync*	self = (Sync*)sync;

	assert( self );
	assert( self->globalMap );
	assert( gInd < self->nGlobal );

	return (self->globalMap[gInd] >= self->nLocal && 
		self->globalMap[gInd] < self->nDomain) ? True : False;
}


void Sync_SetDomainArray( void* sync, 
			  size_t itemSize, size_t stride, void* array )
{
	Sync*		self = (Sync*)sync;
	unsigned	proc_i;

	assert( self );
	assert( itemSize );
	assert( stride );
	assert( array );


	/* If needed, empty the current array content. */
	KillArray( self->sourceOffs );
	KillArray( self->sourceDisps );
	KillArray( self->sourceSizes );
	KillArray( self->sinkOffs );
	KillArray( self->sinkDisps );
	KillArray( self->sinkSizes );
	KillArray( self->sourceArray );
	KillArray( self->sinkArray );


	/*
	** Build a displacement list from the local array we will get when sending such that we can pack values into
	** a contiguous array.  While we're at it, construct an array of sizes, proc wise.
	*/

	if( self->netSource > 0 ) {
		unsigned*	srcOffs;
		unsigned*	srcSizes;
		unsigned*	srcDisps;
		unsigned	srcInd;

		/* Alloc memory. */
		srcDisps = Memory_Alloc_Array( unsigned, self->nProcs, "Sync" );
		srcSizes = Memory_Alloc_Array( unsigned, self->nProcs, "Sync" );
		srcOffs = Memory_Alloc_Array( unsigned, self->netSource, "Sync" );

		srcInd = 0;
		for( proc_i = 0; proc_i < self->nProcs; proc_i++ ) {
			unsigned	src_i;

			srcSizes[proc_i] = 0;
			for( src_i = 0; src_i < self->nSource[proc_i]; src_i++ ) {
				/* Calc the diplacement. */
				srcOffs[srcInd] = self->source[proc_i][src_i] * stride;

				/* Add to the size. */
				srcSizes[proc_i] += itemSize;

				srcInd++;
			}
		}

		/* Calc the displacements. */
		srcDisps[0] = 0;
		for( proc_i = 1; proc_i < self->nProcs; proc_i++ ) {
			srcDisps[proc_i] = srcDisps[proc_i - 1] + srcSizes[proc_i - 1];
		}

		/* Store arrays. */
		self->sourceOffs = srcOffs;
		self->sourceDisps = srcDisps;
		self->sourceSizes = srcSizes;
	}
	else {
		self->sourceOffs = NULL;
		self->sourceDisps = Memory_Alloc_Array( unsigned, self->nProcs, "Sync" );
		self->sourceSizes = Memory_Alloc_Array( unsigned, self->nProcs, "Sync" );
		memset( self->sourceDisps, 0, sizeof(unsigned) * self->nProcs );
		memset( self->sourceSizes, 0, sizeof(unsigned) * self->nProcs );
	}


	/*
	** Do the same for the sink items.
	*/

	if( self->netSink > 0 ) {
		unsigned*	snkOffs;
		unsigned*	snkSizes;
		unsigned*	snkDisps;
		unsigned	snkInd;

		/* Alloc memory. */
		snkDisps = Memory_Alloc_Array( unsigned, self->nProcs, "Sync" );
		snkSizes = Memory_Alloc_Array( unsigned, self->nProcs, "Sync" );
		snkOffs = Memory_Alloc_Array( unsigned, self->netSink, "Sync" );

		snkInd = 0;
		for( proc_i = 0; proc_i < self->nProcs; proc_i++ ) {
			unsigned	snk_i;

			snkSizes[proc_i] = 0;
			for( snk_i = 0; snk_i < self->nSink[proc_i]; snk_i++ ) {
				/* Calc the diplacement. */
				snkOffs[snkInd] = self->sink[proc_i][snk_i] * stride;

				/* Set the size. */
				snkSizes[proc_i] += itemSize;

				snkInd++;
			}
		}

		/* Calc the displacements. */
		snkDisps[0] = 0;
		for( proc_i = 1; proc_i < self->nProcs; proc_i++ ) {
			snkDisps[proc_i] = snkDisps[proc_i - 1] + snkSizes[proc_i - 1];
		}

		/* Store arrays. */
		self->sinkOffs = snkOffs;
		self->sinkDisps = snkDisps;
		self->sinkSizes = snkSizes;
	}
	else {
		self->sinkOffs = NULL;
		self->sinkDisps = Memory_Alloc_Array( unsigned, self->nProcs, "Sync" );
		self->sinkSizes = Memory_Alloc_Array( unsigned, self->nProcs, "Sync" );
		memset( self->sinkSizes, 0, sizeof(unsigned) * self->nProcs );
		memset( self->sinkDisps, 0, sizeof(unsigned) * self->nProcs );
	}


	/*
	** Allocate source and sink arrays so we can compile values into contiguous chunks.
	*/

	self->sourceArray = self->netSource ? Memory_Alloc_Array( unsigned char, self->netSource * itemSize, "Sync" ) : NULL;
	self->sinkArray = self->netSink ? Memory_Alloc_Array( unsigned char, self->netSink * itemSize, "Sync" ) : NULL;


	/*
	** Fill in any remaing values.
	*/

	self->itemSize = itemSize;
	self->arrayType = Sync_ArrayType_Domain;
	self->arrayInfo.array = array;
}


void Sync_SetSplitArrays( void* sync, 
			  size_t itemSize, 
			  size_t srcStride, void* srcArray, 
			  size_t snkStride, void* snkArray )
{
	Sync*		self = (Sync*)sync;
	unsigned	proc_i;

	assert( self );
	assert( itemSize );
	assert( srcStride );
	assert( srcArray );
	assert( snkStride );
	assert( snkArray );

	/* If needed, empty the current array content. */
	KillArray( self->sourceOffs );
	KillArray( self->sourceDisps );
	KillArray( self->sourceSizes );
	KillArray( self->sinkOffs );
	KillArray( self->sinkDisps );
	KillArray( self->sinkSizes );
	KillArray( self->sourceArray );
	KillArray( self->sinkArray );


	/*
	** Build a displacement list from the local array we will get when sending such that we can pack values into
	** a contiguous array.  While we're at it, construct an array of sizes, proc wise.
	*/

	if( self->netSource > 0 ) {
		unsigned*	srcOffs;
		unsigned*	srcSizes;
		unsigned*	srcDisps;
		unsigned	srcInd;

		/* Alloc memory. */
		srcDisps = Memory_Alloc_Array( unsigned, self->nProcs, "Sync" );
		srcSizes = Memory_Alloc_Array( unsigned, self->nProcs, "Sync" );
		srcOffs = Memory_Alloc_Array( unsigned, self->netSource, "Sync" );

		srcInd = 0;
		for( proc_i = 0; proc_i < self->nProcs; proc_i++ ) {
			unsigned	src_i;

			srcSizes[proc_i] = 0;
			for( src_i = 0; src_i < self->nSource[proc_i]; src_i++ ) {
				/* Calc the diplacement. */
				srcOffs[srcInd] = self->source[proc_i][src_i] * srcStride;

				/* Add to the size. */
				srcSizes[proc_i] += itemSize;

				srcInd++;
			}
		}

		/* Calc the displacements. */
		srcDisps[0] = 0;
		for( proc_i = 1; proc_i < self->nProcs; proc_i++ ) {
			srcDisps[proc_i] = srcDisps[proc_i - 1] + srcSizes[proc_i - 1];
		}

		/* Store arrays. */
		self->sourceOffs = srcOffs;
		self->sourceDisps = srcDisps;
		self->sourceSizes = srcSizes;
	}
	else {
		self->sourceOffs = NULL;
		self->sourceDisps = Memory_Alloc_Array( unsigned, self->nProcs, "Sync" );
		self->sourceSizes = Memory_Alloc_Array( unsigned, self->nProcs, "Sync" );
		memset( self->sourceDisps, 0, sizeof(unsigned) * self->nProcs );
		memset( self->sourceSizes, 0, sizeof(unsigned) * self->nProcs );
	}


	/*
	** Do the same for the sink items.
	*/

	if( self->netSink > 0 ) {
		unsigned*	snkOffs;
		unsigned*	snkSizes;
		unsigned*	snkDisps;
		unsigned	snkInd;

		/* Alloc memory. */
		snkDisps = Memory_Alloc_Array( unsigned, self->nProcs, "Sync" );
		snkSizes = Memory_Alloc_Array( unsigned, self->nProcs, "Sync" );
		snkOffs = Memory_Alloc_Array( unsigned, self->netSink, "Sync" );

		snkInd = 0;
		for( proc_i = 0; proc_i < self->nProcs; proc_i++ ) {
			unsigned	snk_i;

			snkSizes[proc_i] = 0;
			for( snk_i = 0; snk_i < self->nSink[proc_i]; snk_i++ ) {
				/* Calc the diplacement. */
				snkOffs[snkInd] = self->sink[proc_i][snk_i] * snkStride;

				/* Set the size. */
				snkSizes[proc_i] += itemSize;

				snkInd++;
			}
		}

		/* Calc the displacements. */
		snkDisps[0] = 0;
		for( proc_i = 1; proc_i < self->nProcs; proc_i++ ) {
			snkDisps[proc_i] = snkDisps[proc_i - 1] + snkSizes[proc_i - 1];
		}

		/* Store arrays. */
		self->sinkOffs = snkOffs;
		self->sinkDisps = snkDisps;
		self->sinkSizes = snkSizes;
	}
	else {
		self->sinkOffs = NULL;
		self->sinkDisps = Memory_Alloc_Array( unsigned, self->nProcs, "Sync" );
		self->sinkSizes = Memory_Alloc_Array( unsigned, self->nProcs, "Sync" );
		memset( self->sinkSizes, 0, sizeof(unsigned) * self->nProcs );
		memset( self->sinkDisps, 0, sizeof(unsigned) * self->nProcs );
	}


	/*
	** Allocate source and sink arrays so we can compile values into contiguous chunks.
	*/

	self->sourceArray = self->netSource ? Memory_Alloc_Array( unsigned char, self->netSource * itemSize, "Sync" ) : NULL;
	self->sinkArray = self->netSink ? Memory_Alloc_Array( unsigned char, self->netSink * itemSize, "Sync" ) : NULL;


	/*
	** Fill in any remaing values.
	*/

	self->itemSize = itemSize;
	self->arrayType = Sync_ArrayType_Split;
	self->arrayInfo.splitArray.srcArray = srcArray;
	self->arrayInfo.splitArray.snkArray = snkArray;
}


void Sync_SendRecv( void* sync ) {
	Sync*		self = (Sync*)sync;
	unsigned	snk_i, src_i;

	assert( self );
	assert( self->arrayType != Sync_ArrayType_None );

	/*
	** Pack from the local array into a contiguous array.
	*/

	if( self->netSink > 0 ) {
		void*	array;

		/* Select the array to use. */
		array = (self->arrayType == Sync_ArrayType_Domain) ? self->arrayInfo.array : self->arrayInfo.splitArray.srcArray;
		assert( array );
		
		/* Pack the values. */
		for( snk_i = 0; snk_i < self->netSink; snk_i++ ) {
			memcpy( 
				(void*)((ArithPointer)self->sinkArray + (ArithPointer)(snk_i * self->itemSize)), 
				(void*)((ArithPointer)array + (ArithPointer)self->sinkOffs[snk_i]), 
				self->itemSize );
		}
	}


	/*
	** Execute the sync system.
	*/

	MPI_Alltoallv( self->sinkArray, (int*)self->sinkSizes, (int*)self->sinkDisps, MPI_BYTE, 
		       self->sourceArray, (int*)self->sourceSizes, (int*)self->sourceDisps, MPI_BYTE, 
		       self->comm );


	/*
	** Unpack the received values.
	*/

	{
		unsigned	offs;
		void*		array;

		/* Select an array and offset accordingly. */
		if( self->arrayType == Sync_ArrayType_Domain ) {
			array = self->arrayInfo.array;
			offs = 0;
		}
		else {
			array = self->arrayInfo.splitArray.snkArray;
			offs = self->nLocal * self->itemSize;
		}

		for( src_i = 0; src_i < self->netSource; src_i++ ) {
			memcpy( 
				(void*)((ArithPointer)array + (ArithPointer)self->sourceOffs[src_i] - (ArithPointer)offs), 
				(void*)((ArithPointer)self->sourceArray + (ArithPointer)(src_i * self->itemSize)),
				self->itemSize );
		}
	}
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

void _Sync_Free( void* sync ) {
	Sync*	self = (Sync*)sync;

	assert( self );

	/*
	** Kill any arrays that were allocated by Sync functions.
	*/

	KillArray( self->domain );
	self->local = NULL;
	self->shadow = NULL;

	KillArray( self->globalMap );

	KillArray( self->nSource );
	KillArray2D( self->nProcs, self->source );

	KillArray( self->nSink );
	KillArray( self->sink );

	KillArray( self->sourceDisps );
	KillArray( self->sourceSizes );
	KillArray( self->sourceOffs );
	KillArray( self->sourceArray );
	KillArray( self->sinkDisps );
	KillArray( self->sinkSizes );
	KillArray( self->sinkOffs );
	KillArray( self->sinkArray );
}


void _Sync_Select_MinimizeComms( void* sync, 
				 unsigned* nFound, unsigned** found )
{
	Sync*		self = (Sync*)sync;
	unsigned*	nUniques;
	unsigned**	uniques;
	IndexSet*	globalSet;
	unsigned	mostUniques;
	unsigned	proc_i;

	Bool _Sync_IsUnique( Sync* self, unsigned found );

	assert( self );
	assert( nFound );
	assert( found );


	/*
	** Set up the source arrays.
	*/
	
	self->nSource = Memory_Alloc_Array( unsigned, self->nProcs, "Sync" );
	self->source = Memory_Alloc_Array( unsigned*, self->nProcs, "Sync" );
	for( proc_i = 0; proc_i < self->nProcs; proc_i++ ) {
		self->nSource[proc_i] = 0;
		self->source[proc_i] = NULL;
	}
	
	
	/*
	** Select source procs from which to retrieve required nodes such that communications are minimized.
	*/
	
	nUniques = Memory_Alloc_Array( unsigned, self->nProcs, "Sync" );
	uniques = Memory_Alloc_Array( unsigned*, self->nProcs, "Sync" );
	for( proc_i = 0; proc_i < self->nProcs; proc_i++ ) {
		uniques[proc_i] = NULL;
	}
	
	globalSet = IndexSet_New( self->nGlobal );
	
	do {
		/* Loop over procs and find unique nodes per proc. ie. the most nodes not already covered by
		   an existing source. */
		for( proc_i = 0; proc_i < self->nProcs; proc_i++ ) {
			unsigned	fnd_i;
			
			/* If there are no founds for this proc or we've already sourced it, then skip. */
			if( nFound[proc_i] == 0 || self->nSource[proc_i] > 0 ) {
				nUniques[proc_i] = 0;
				continue;
			}
			
			IndexSet_RemoveAll( globalSet );
			
			for( fnd_i = 0; fnd_i < nFound[proc_i]; fnd_i++ ) {
				if( _Sync_IsUnique( sync, found[proc_i][fnd_i] ) ) {
					IndexSet_Add( globalSet, found[proc_i][fnd_i] );
				}
			}
			
			IndexSet_GetMembers( globalSet, &nUniques[proc_i], &uniques[proc_i] );
		}

		/* Determine which proc has the most uniques and store. */
		{
			unsigned	mostProc = 0;
			
			mostUniques = 0;
			for( proc_i = 0; proc_i < self->nProcs; proc_i++ ) {
				if( nUniques[proc_i] > mostUniques ) {
					mostUniques = nUniques[proc_i];
					mostProc = proc_i;
				}
			}
			
			/* Store result. */
			if( mostUniques ) {
				self->nSource[mostProc] = mostUniques;
				self->source[mostProc] = uniques[mostProc];
				uniques[mostProc] = NULL;
			}
		}
		
		/* Clobber the allocated 'uniques' arrays. */
		for( proc_i = 0; proc_i < self->nProcs; proc_i++ )
			FreeArray( uniques[proc_i] );
	}
	while( mostUniques );

	/* Get rid of all the resources. */
	FreeArray( nUniques );
	FreeArray( uniques );
	Stg_Class_Delete( globalSet );
}


Bool _Sync_IsUnique( Sync* self, unsigned found ) {
	unsigned	proc_i;
	
	for( proc_i = 0; proc_i < self->nProcs; proc_i++ ) {
		unsigned	src_i;
		
		for( src_i = 0; src_i < self->nSource[proc_i]; src_i++ ) {
			if( self->source[proc_i][src_i] == found ) {
				return False;
			}
		}
	}
	
	return True;
}


void _Sync_BuildGlobalMap( void* sync ) {
	Sync*		self = (Sync*)sync;
	unsigned	glo_i, dom_i;

	assert( self );
	assert( !self->globalMap );
	assert( self->nGlobal );


	/*
	** Construct a map from global space to domain space.
	*/

	self->globalMap = Memory_Alloc_Array( unsigned, self->nGlobal, "Sync" );
	for( glo_i = 0; glo_i < self->nGlobal; glo_i++ ) {
		self->globalMap[glo_i] = (unsigned)-1;
	}

	for( dom_i = 0; dom_i < self->nDomain; dom_i++ ) {
		self->globalMap[self->domain[dom_i]] = dom_i;
	}
}


void _Sync_MapAll( void* sync ) {
	Sync*		self = (Sync*)sync;
	unsigned	proc_i, snk_i, src_i;

	assert( self );
	assert( self->globalMap );
	assert( !self->nLocal || self->local );


	/*
	** Map the global indices stored in the sink array to local indices.
	*/

	/* Ensure the global map is built. */
	for( proc_i = 0; proc_i < self->nProcs; proc_i++ ) {
		for( snk_i = 0; snk_i < self->nSink[proc_i]; snk_i++ ) {
			self->sink[proc_i][snk_i] = self->globalMap[self->sink[proc_i][snk_i]];
		}
		for( src_i = 0; src_i < self->nSource[proc_i]; src_i++ ) {
			self->source[proc_i][src_i] = self->globalMap[self->source[proc_i][src_i]];
		}
	}
}


/*--------------------------------------------------------------------------------------------------------------------------
** MPI Helper Functions.
*/

void MPI_ArrayGather1D( unsigned size, size_t itemSize, void* array, 
			unsigned** dstSizes, void** dstArray, 
			unsigned root, MPI_Comm comm )
{
   	int	nProcs;
	int	rank;
	

	/* Get basic MPI info. */
	CHK_MPI( MPI_Comm_size( comm, &nProcs ) );
	CHK_MPI( MPI_Comm_rank( comm, &rank ) );
	
	
	/*
	** Send a 1D array of arbitrary length to root process in supplied communicator.  This means we also
	** need to receive arrays of arbitrary length from all others.  As the array from this proc is already stored
	** elsewhere we will remove it from the received array, setting its length to zero.
	*/
	
	{
		unsigned*	tmpSizes = NULL;
		void*		tmpArray1D = NULL;
		unsigned*	disps = NULL;
		unsigned	proc_i;
		
		/* Receive size/s. */
		*dstSizes = Memory_Alloc_Array_Unnamed( unsigned, nProcs );
		CHK_MPI( MPI_Gather( &size, 1, MPI_UNSIGNED, *dstSizes, 1, MPI_UNSIGNED, root, comm ) );
		
		/* Factor in 'itemSize'. */
		tmpSizes = Memory_Alloc_Array_Unnamed( unsigned, nProcs );
		for( proc_i = 0; proc_i < nProcs; proc_i++ ) {
			tmpSizes[proc_i] = (*dstSizes)[proc_i] * itemSize;
		}
		
		/* Allocate space for the coming arrays and build a displacement list. */
		{
			unsigned	netSize;
		
			disps = Memory_Alloc_Array_Unnamed( unsigned, nProcs );
		
			disps[0] = 0;
			netSize = tmpSizes[0];
			for( proc_i = 1; proc_i < nProcs; proc_i++ ) {
				disps[proc_i] = disps[proc_i - 1] + tmpSizes[proc_i - 1];
				netSize += tmpSizes[proc_i];
			}
		
			if( netSize ) {
				tmpArray1D = Memory_Alloc_Array_Bytes_Unnamed( sizeof(byte_t), netSize, "unknown" );
			}
			else {
				tmpArray1D = NULL;
			}
		}

		/* Send/receive array/s. */
		CHK_MPI( MPI_Gatherv( array, size * itemSize, MPI_BYTE, 
				      tmpArray1D, (int*)tmpSizes, (int*)disps, MPI_BYTE, 
				      root, comm ) );
		
		/* Free the displacements and temp-sizes. */
		Memory_Free( disps );
		Memory_Free( tmpSizes );
		
		/* Store the 1D array. */
		*dstArray = tmpArray1D;
	}
}


void MPI_ArrayAllgather( unsigned		size, 
					size_t		itemSize, 
					void*		array, 
					unsigned**	dstSizes, 
					void***		dstArray, 
					MPI_Comm		comm )
{
	int	nProcs;
	int	rank;
	
	
/*
	Journal_Firewall( itemSize > 0 && comm, "Corrupt arguments to MPI_ArrayAllgather.\n" );
*/
	
	
	/* Get basic MPI info. */
	CHK_MPI( MPI_Comm_size( comm, &nProcs ) );
	CHK_MPI( MPI_Comm_rank( comm, &rank ) );
	
	
	/*
	** Send a 1D array of arbitrary length to all other processes in the supplied communicator.  This means we also
	** need to receive arrays of arbitrary length from all others.  As the array from this proc is already stored
	** elsewhere we will remove it from the received array, setting its length to zero.
	*/
	
	{
		unsigned*	tmpSizes;
		void*	tmpArray1D;
		unsigned*	disps;
		unsigned	proc_i;
		
		/* Send/receive size/s. */
		*dstSizes = Memory_Alloc_Array_Unnamed( unsigned, nProcs );
		CHK_MPI( MPI_Allgather( &size, 1, MPI_UNSIGNED, *dstSizes, 1, MPI_UNSIGNED, comm ) );
		
		/* Factor in 'itemSize'. */
		tmpSizes = Memory_Alloc_Array_Unnamed( unsigned, nProcs );
		for( proc_i = 0; proc_i < nProcs; proc_i++ ) {
			tmpSizes[proc_i] = (*dstSizes)[proc_i] * itemSize;
		}
		
		/* Allocate space for the coming arrays and build a displacement list. */
		{
			unsigned	netSize;
			
			disps = Memory_Alloc_Array_Unnamed( unsigned, nProcs );
			
			disps[0] = 0;
			netSize = tmpSizes[0];
			for( proc_i = 1; proc_i < nProcs; proc_i++ ) {
				disps[proc_i] = disps[proc_i - 1] + tmpSizes[proc_i - 1];
				netSize += tmpSizes[proc_i];
			}
			
			if( netSize ) {
				tmpArray1D = Memory_Alloc_Array_Bytes_Unnamed( sizeof(byte_t), netSize, "unknown" );
			}
			else {
				tmpArray1D = NULL;
			}
		}
		
		/* Send/receive array/s. */
		CHK_MPI( MPI_Allgatherv( array, 
							size * itemSize, 
							MPI_BYTE, 
							tmpArray1D, 
							(int*)tmpSizes, 
							(int*)disps, 
							MPI_BYTE, 
							comm ) );
		
		/* Free the displacements and temp-sizes. */
		Memory_Free( disps );
		Memory_Free( tmpSizes );
		
		/* Unpack the 1D array into the 2D destination. */
		Array_1DTo2D( nProcs, *dstSizes, tmpArray1D, dstArray, sizeof(unsigned) );
		
		/* Free resources. */
		if( tmpArray1D ) {
			Memory_Free( tmpArray1D );
		}
	}
}


void MPI_Array2DAlltoall( unsigned*	sizes, 
					 size_t		itemSize, 
					 void**		array, 
					 unsigned**	dstSizes, 
					 void***		dstArray, 
					 MPI_Comm		comm )
{
	int	nProcs;
	int	rank;
	
	
/*
	Journal_Firewall( itemSize > 0 && comm, "Corrupt arguments to MPI_ArrayAllgather.\n" );
*/
	
	
	/* Get basic MPI info. */
	CHK_MPI( MPI_Comm_size( comm, &nProcs ) );
	CHK_MPI( MPI_Comm_rank( comm, &rank ) );
	
	
	/*
	** Blah, blah, sick of comments.
	*/
	
	{
		unsigned*	tmpDstSizes;
		unsigned*	tmpDstArray1D;
		unsigned*	dstDisps;
		unsigned	proc_i;
		
		/* Send/receive sizes. */
		*dstSizes = Memory_Alloc_Array_Unnamed( unsigned, nProcs );
		CHK_MPI( MPI_Alltoall( sizes, 1, MPI_UNSIGNED, *dstSizes, 1, MPI_UNSIGNED, comm ) );

		/* Copy sizes into a new array and modify to include 'itemSize'. */
		tmpDstSizes = Memory_Alloc_Array_Unnamed( unsigned, nProcs );
		
		for( proc_i = 0; proc_i < nProcs; proc_i++ ) {
			tmpDstSizes[proc_i] = (*dstSizes)[proc_i] * itemSize;
		}
		
		/* Allocate space for the coming arrays and build a displacement list. */
		{
			unsigned	netSize;
			
			dstDisps = Memory_Alloc_Array_Unnamed( unsigned, nProcs );
			
			dstDisps[0] = 0;
			netSize = tmpDstSizes[0];
			for( proc_i = 1; proc_i < nProcs; proc_i++ ) {
				dstDisps[proc_i] = dstDisps[proc_i - 1] + tmpDstSizes[proc_i - 1];
				netSize += tmpDstSizes[proc_i];
			}
			
			if( netSize ) {
				tmpDstArray1D = Memory_Alloc_Array_Bytes_Unnamed( sizeof(byte_t), netSize, "unknown" );
			}
			else {
				tmpDstArray1D = NULL;
			}
		}
		
		/* Pack the supplied 2D array into a 1D array and send/receive. */
		{
			unsigned*	tmpSizes;
			void*	tmpSrcArray1D;
			unsigned*	disps;
			
			Array_2DTo1D( nProcs, sizes, array, &tmpSrcArray1D, sizeof(unsigned), &disps );
			
			/* Generate a temporary set of sizes to include 'itemSize'. Modify 'dists' while we're at it. */
			tmpSizes = Memory_Alloc_Array_Unnamed( unsigned, nProcs );
			for( proc_i = 0; proc_i < nProcs; proc_i++ ) {
				disps[proc_i] *= itemSize;
				tmpSizes[proc_i] = sizes[proc_i] * itemSize;
			}
			
			/* Send/recv. */
			CHK_MPI( MPI_Alltoallv( 
				tmpSrcArray1D, 
				(int*)tmpSizes, 
				(int*)disps, 
				MPI_BYTE, 
				tmpDstArray1D, 
				(int*)tmpDstSizes, 
				(int*)dstDisps, 
				MPI_BYTE, 
				comm ) );
			
			/* Free memory. */
			Memory_Free( tmpSizes );
			Memory_Free( disps );
			Memory_Free( dstDisps );
			Memory_Free( tmpDstSizes );
			
			if( tmpSrcArray1D ) {
				Memory_Free( tmpSrcArray1D );
			}
		}
		
		/* Unpack the 1D array into the 2D destination. */
		Array_1DTo2D( nProcs, *dstSizes, tmpDstArray1D, dstArray, sizeof(unsigned) );
		
		/* Free resources. */
		if( tmpDstArray1D ) {
			Memory_Free( tmpDstArray1D );
		}
	}
}

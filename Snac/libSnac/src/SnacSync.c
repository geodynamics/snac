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
** This file may be distributed under the terms of the VPAC Public License
** as defined by VPAC of Australia and appearing in the file
** LICENSE.VPL included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** $Id: SnacSync.c 2453 2004-12-21 04:49:34Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>

#include "types.h"
#include "SnacSync.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


/* Useful typedefs. */
typedef unsigned char	Snac_Byte;


/* Textual name of this class */
const Type SnacSync_Type = "SnacSync";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

void* SnacSync_DefaultNew( Name name )
{
	return _SnacSync_New(
		sizeof(SnacSync), 
		SnacSync_Type, 
		_SnacSync_Delete, 
		_SnacSync_Print,
		NULL,
		SnacSync_DefaultNew,
		_SnacSync_Construct,
		_SnacSync_Build,
		_SnacSync_Initialise,
		_SnacSync_Execute,
		_SnacSync_Destroy,
		name,
		False,
		NULL );
}


SnacSync* SnacSync_New_Param(
		Dictionary*	dictionary,
		Name			name )
{
	return _SnacSync_New( 
		sizeof(SnacSync), 
		SnacSync_Type, 
		_SnacSync_Delete, 
		_SnacSync_Print,
		NULL,
		SnacSync_DefaultNew,
		_SnacSync_Construct,
		_SnacSync_Build,
		_SnacSync_Initialise,
		_SnacSync_Execute,
		_SnacSync_Destroy,
		name,
		True,
		dictionary );
}


void SnacSync_Init(
		SnacSync*		self,
		Dictionary*	dictionary,
		Name			name )
{
	/* General info */
	self->type = SnacSync_Type;
	self->_sizeOfSelf = sizeof(SnacSync);
	self->_deleteSelf = False;
	self->dictionary = dictionary;
	
	/* Virtual info */
	self->_delete = _SnacSync_Delete;
	self->_print = _SnacSync_Print;
	self->_copy = NULL;
	self->_defaultConstructor = SnacSync_DefaultNew;
	self->_construct = _SnacSync_Construct;
	self->_build = _SnacSync_Build;
	self->_initialise = _SnacSync_Initialise;
	self->_execute = _SnacSync_Execute;
	self->_destroy = _SnacSync_Destroy;
	
	_Stg_Class_Init( (Stg_Class*)self );
	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );
	_Stg_Component_Init( (Stg_Component*)self );
	
	/* SnacSync info */
	_SnacSync_Init( self );
}


SnacSync* _SnacSync_New(
		SizeT							_sizeOfSelf, 
		Type								type,
		Stg_Class_DeleteFunction*			_delete,
		Stg_Class_PrintFunction*				_print,
		Stg_Class_CopyFunction*				_copy, 
		Stg_Component_DefaultConstructorFunction*	_defaultConstructor,
		Stg_Component_ConstructFunction*			_construct,
		Stg_Component_BuildFunction*				_build,
		Stg_Component_InitialiseFunction*			_initialise,
		Stg_Component_ExecuteFunction*			_execute,
		Stg_Component_DestroyFunction*			_destroy,
		Name								name,
		Bool								initFlag,
		Dictionary*						dictionary )
{
	SnacSync* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(SnacSync) );
	self = (SnacSync*)_Stg_Component_New( 
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
	
	/* SnacSync info */
	if( initFlag ){
		_SnacSync_Init( self );
	}
	
	return self;
}

void _SnacSync_Init( SnacSync* self )
{
	/* General and Virtual info should already be set */
	self->isConstructed = True;
	/* SnacSync info */
	
	self->comm = MPI_COMM_WORLD;
	self->nProcs = 0;
	self->rank = 0;
	
	self->select = _SnacSync_Select_MinimizeComms;
	
	self->netSource = 0;
	self->nSource = NULL;
	self->source = NULL;
	
	self->netSink = 0;
	self->nSink = NULL;
	self->sink = NULL;
	
	self->sourceDisps = NULL;
	self->sourceSizes = NULL;
	self->sinkDisps = NULL;
	self->sinkSizes = NULL;
	
	self->sourceOffs = NULL;
	self->sinkOffs = NULL;
	self->itemSize = 0;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

#define CHK_MPI( mpiFunc ) \
	if( mpiFunc != MPI_SUCCESS ) \
		fprintf( stderr, "Warning: MPI function returned failure...\n" )


void _SnacSync_Delete( void* sync ) {
	SnacSync* self = (SnacSync*)sync;
	
	/* Stg_Class_Delete parent */
	_Stg_Class_Delete( self );
	
	FreeArray( self->nSource );
	FreeArray2D( self->nProcs, self->source );
	
	FreeArray( self->nSink );
	FreeArray2D( self->nProcs, self->sink );
	
	FreeArray( self->sourceDisps );
	FreeArray( self->sourceSizes );
	FreeArray( self->sinkDisps );
	FreeArray( self->sinkSizes );
	
	FreeArray( self->sourceOffs );
	FreeArray( self->sinkOffs );
}


void _SnacSync_Print( void* sync, Stream* stream ) {
	SnacSync* self = (SnacSync*)sync;
	
	/* Set the Journal for printing informations */
	Stream* syncStream;
	syncStream = Journal_Register( InfoStream_Type, "SnacSyncStream" );

	/* Print parent */
	_Stg_Class_Print( self, stream );
	
	/* General info */
	Journal_Printf( stream, "SnacSync (ptr): (%p)\n", self );
	
	/* Virtual info */
	
	/* SnacSync info */
}


void _SnacSync_Construct( void* sync, Stg_ComponentFactory *cf, void* data ){
}

	
void _SnacSync_Build( void* sync, void *data ){
}

	
void _SnacSync_Initialise( void* sync, void *data ) {
}

	
void _SnacSync_Execute( void* sync, void *data ){
}


void _SnacSync_Destroy( void* sync, void *data ){
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void SnacSync_Negotiate( void*		sync, 
				 unsigned		nGlobals, 
				 unsigned		nLocals, 
				 unsigned*	locals, 
				 unsigned		nRequired, 
				 unsigned*	required, 
				 MPI_Comm		comm )
{
	SnacSync*		self = (SnacSync*)sync;
	unsigned*		remNFound;
	unsigned**	remFound;
	unsigned		proc_i;

	
/*
	Journal_Firewall( nGlobals > 0, "'SnacSync' class initialised with no globals.\n" );
	
	if( nLocals ) {
		Journal_Firewall( locals, "Corrupt local information in 'sync' class.\n" );
	}
	
	if( nRequired ) {
		Journal_Firewall( required, "Corrupt remote information in 'sync' class.\n" );
	}
	
	Journal_Firewall( self->comm, "No communicator associated with 'sync' class." );
*/
	
	
	/*
	** Check for existing allocated arrays and free them.
	*/
	
	_SnacSync_FreePrivateArrays( self );
	
	
	/*
	** Collect basic MPI information.
	*/
	
	self->comm = comm;
	CHK_MPI( MPI_Comm_rank( comm, &self->rank ) );
	CHK_MPI( MPI_Comm_size( comm, &self->nProcs ) );
	
	if( self->nProcs == 1 ) {
		self->nSource = Memory_Alloc_Array( unsigned, 1, "SnacSync" );
		self->nSource[0] = 0;
		
		self->nSink = Memory_Alloc_Array( unsigned, 1, "SnacSync" );
		self->nSink[0] = 0;
		
		return;
	}
		
	
	/*
	** For each other proc build a set of locals that correspond to remotes they will send here.  There will
	** probably be mainly empty sets.  Send back to respective procs.
	*/
	
	{
		unsigned*		remNReq;
		unsigned**	remReq;
		unsigned*		nFound;
		unsigned**	found;
		IndexSet*		globals;
		
		/* Send our required to all others and receive from all others. */
		MPI_ArrayAllgather( nRequired, sizeof(unsigned), required, &remNReq, (void***)&remReq, comm );
		
		/* Prepare to locate remote requireds. */
		nFound = Memory_Alloc_Array( unsigned, self->nProcs, "SnacSync" );
		found = Memory_Alloc_Array( unsigned*, self->nProcs, "SnacSync" );
		globals = IndexSet_New( nGlobals );
		
		/* Begin searching... */
		for( proc_i = 0; proc_i < self->nProcs; proc_i++ ) {
			unsigned	remReq_i;
			
			if( proc_i == self->rank ) {
				nFound[proc_i] = 0;
				found[proc_i] = NULL;
				continue;
			}
			
			IndexSet_RemoveAll( globals );
			
			for( remReq_i = 0; remReq_i < remNReq[proc_i]; remReq_i++ ) {
				unsigned	local_i;
				
				for( local_i = 0; local_i < nLocals; local_i++ ) {
					if( locals[local_i] == remReq[proc_i][remReq_i] ) {
						IndexSet_Add( globals, remReq[proc_i][remReq_i] );
						break;
					}
				}
			}
			
			IndexSet_GetMembers( globals, &nFound[proc_i], &found[proc_i] );
		}
		
		/* Release some memory. */
		FreeArray( remNReq );
		FreeArray( remReq );
		Stg_Class_Delete( globals );
		
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
	
	self->select( self, nGlobals, remNFound, remFound );
	
	/* We can now free up some more memory. */
	FreeArray( remNFound );
	FreeArray( remFound );
	
	
	/*
	** Communicate back to all procs our selection.  Note that even though most procs will not need to send
	** anything here, we still need to let them know that.
	*/
	
	/* Allocate for the sinks. */
	self->nSink = Memory_Alloc_Array( unsigned, self->nProcs, "SnacSync" );
	
	CHK_MPI( MPI_Alltoall( self->nSource, 1, MPI_UNSIGNED, self->nSink, 1, MPI_UNSIGNED, comm ) );
	MPI_Array2DAlltoall( self->nSource, sizeof(unsigned), (void**)self->source, &self->nSink, (void***)&self->sink, comm );
	
	
	/*
	** Calculate net values for source and sink.
	*/
	
	self->netSource = 0;
	self->netSink = 0;
	for( proc_i = 0; proc_i < self->nProcs; proc_i++ ) {
		self->netSource += self->nSource[proc_i];
		self->netSink += self->nSink[proc_i];
	}
}


void SnacSync_SendRecvInitialise( void*	sync, 
						size_t	itemSize, 
						unsigned*	sinkOffsets, 
						unsigned	sinkStride, 
						unsigned*	sourceOffsets, 
						unsigned	sourceStride )
{
	SnacSync*	self = (SnacSync*)sync;
	
	
	/*
	** Build a displacement list from the local array we will get when sending such that we can pack values into
	** a contiguous array.  While we're at it, construct an array of sizes, proc wise.
	*/
	
	if( self->netSource > 0 ) {
		unsigned*	srcOffs;
		unsigned*	srcSizes;
		unsigned*	srcDisps;
		unsigned	srcInd = 0;
		unsigned	proc_i;
		
		/* Alloc memory. */
		srcDisps = Memory_Alloc_Array( unsigned, self->nProcs, "SnacSync" );
		srcSizes = Memory_Alloc_Array( unsigned, self->nProcs, "SnacSync" );
		srcOffs = Memory_Alloc_Array( unsigned, self->netSource, "SnacSync" );
		
		for( proc_i = 0; proc_i < self->nProcs; proc_i++ ) {
			unsigned	src_i;
			
			srcSizes[proc_i] = 0;
			
			for( src_i = 0; src_i < self->nSource[proc_i]; src_i++ ) {
				/* Calc the diplacement. */
				srcOffs[srcInd] = sourceOffsets[srcInd] * sourceStride;
				
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
		self->sourceDisps = Memory_Alloc_Array( unsigned, self->nProcs, "SnacSync" );
		self->sourceSizes = Memory_Alloc_Array( unsigned, self->nProcs, "SnacSync" );
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
		unsigned	snkInd = 0;
		unsigned	proc_i;
		
		/* Alloc memory. */
		snkDisps = Memory_Alloc_Array( unsigned, self->nProcs, "SnacSync" );
		snkSizes = Memory_Alloc_Array( unsigned, self->nProcs, "SnacSync" );
		snkOffs = Memory_Alloc_Array( unsigned, self->netSink, "SnacSync" );
		
		for( proc_i = 0; proc_i < self->nProcs; proc_i++ ) {
			unsigned	snk_i;
			
			snkSizes[proc_i] = 0;
			
			for( snk_i = 0; snk_i < self->nSink[proc_i]; snk_i++ ) {
				/* Calc the diplacement. */
				snkOffs[snkInd] = sinkOffsets[snkInd] * sinkStride;
				
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
		self->sinkDisps = Memory_Alloc_Array( unsigned, self->nProcs, "SnacSync" );
		self->sinkSizes = Memory_Alloc_Array( unsigned, self->nProcs, "SnacSync" );
		memset( self->sinkSizes, 0, sizeof(unsigned) * self->nProcs );
		memset( self->sinkDisps, 0, sizeof(unsigned) * self->nProcs );
	}
	
	
	/*
	** Fill in any remaing values.
	*/
	
	self->itemSize = itemSize;
}


void SnacSync_SendRecv( void*	sync, 
				void*	srcArray, 
				void*	dstArray )
{
	SnacSync*	self = (SnacSync*)sync;
	Snac_Byte*	sendBuf;
	Snac_Byte*	recvBuf;
	
	
	if( !self->sourceDisps || !self->sourceSizes || 
	    !self->sinkDisps || !self->sinkSizes )
	{
		assert( 0 ); /* TODO: Convert to journal. */
	}
	
	
	/*
	** Pack from the local array into a contiguous array.
	*/
	
	if( self->netSink > 0 ){
		unsigned	snk_i;
		
		/* Allocate the send buffer. */
		sendBuf = Memory_Alloc_Array( Snac_Byte, self->netSink * self->itemSize, "SnacSync" );
		
		/* Pack the values. */
		for( snk_i = 0; snk_i < self->netSink; snk_i++ ) {
			memcpy( sendBuf + snk_i * self->itemSize, (Snac_Byte*)srcArray + self->sinkOffs[snk_i], self->itemSize );
		}
	}
	else {
		sendBuf = NULL;
	}
	
	
	/*
	** Prepare a buffer to receive to.
	*/
	
	if( self->netSource > 0 ) {
		/* Allocate the send buffer. */
		recvBuf = Memory_Alloc_Array( Snac_Byte, self->netSource * self->itemSize, "SnacSync" );
	}
	else {
		recvBuf = NULL;
	}
	
	
	/*
	** Execute the sync system.
	*/
	
	MPI_Alltoallv( sendBuf, (int*)self->sinkSizes, (int*)self->sinkDisps, MPI_BYTE, 
				recvBuf, (int*)self->sourceSizes, (int*)self->sourceDisps, MPI_BYTE, 
				self->comm );
	
	/* Free the send buffer. */
	FreeArray( sendBuf );
	
	
	/*
	** Unpack the received values.
	*/
	
	{
		unsigned	src_i;
		
		for( src_i = 0; src_i < self->netSource; src_i++ ) {
			memcpy( (Snac_Byte*)dstArray + self->sourceOffs[src_i], recvBuf + src_i * self->itemSize, self->itemSize );
		}
	}
	
	/* Free teh recv buffer. */
	FreeArray( recvBuf );
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

void _SnacSync_FreePrivateArrays( void* sync ) {
	SnacSync*	self = (SnacSync*)sync;
	
	
	/*
	** Kill any arrays that were allocated by SnacSync functions.
	*/
	
	KillArray( self->nSource );
	KillArray2D( self->nProcs, self->source );
	
	KillArray( self->nSink );
	KillArray2D( self->nProcs, self->sink );
	
	KillArray( self->sourceDisps );
	KillArray( self->sourceSizes );
	KillArray( self->sinkDisps );
	KillArray( self->sinkSizes );
	
	KillArray( self->sourceOffs );
	KillArray( self->sinkOffs );
}


Bool _SnacSync_IsUnique( SnacSync* self, unsigned found ) {
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


void _SnacSync_Select_MinimizeComms( void* sync, unsigned nGlobals, unsigned* nFound, unsigned** found ) {
	SnacSync*		self = (SnacSync*)sync;
	unsigned*		nUniques;
	unsigned**	uniques;
	IndexSet*		globals;
	unsigned		mostUniques;
	unsigned		proc_i;
	
	
/*
	Journal_Firewall( sync && nFound && found, "Corrupt arguments to 'SnacSync_Select'.\n" );
*/
	
	
	/*
	** Set up the source arrays.
	*/
	
	self->nSource = Memory_Alloc_Array( unsigned, self->nProcs, "SnacSync" );
	self->source = Memory_Alloc_Array( unsigned*, self->nProcs, "SnacSync" );
	for( proc_i = 0; proc_i < self->nProcs; proc_i++ ) {
		self->nSource[proc_i] = 0;
		self->source[proc_i] = NULL;
	}
	
	
	/*
	** Select source procs from which to retrieve required nodes such that communications are minimized.
	*/
	
	nUniques = Memory_Alloc_Array( unsigned, self->nProcs, "SnacSync" );
	uniques = Memory_Alloc_Array( unsigned*, self->nProcs, "SnacSync" );
	for( proc_i = 0; proc_i < self->nProcs; proc_i++ ) {
		uniques[proc_i] = NULL;
	}
	
	globals = IndexSet_New( nGlobals );
	
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
			
			IndexSet_RemoveAll( globals );
			
			for( fnd_i = 0; fnd_i < nFound[proc_i]; fnd_i++ ) {
				if( _SnacSync_IsUnique( sync, found[proc_i][fnd_i] ) ) {
					IndexSet_Add( globals, found[proc_i][fnd_i] );
				}
			}
			
			IndexSet_GetMembers( globals, &nUniques[proc_i], &uniques[proc_i] );
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
	}
	while( mostUniques );
	
	/* Get rid of all the resources. */
	FreeArray( nUniques );
	FreeArray2D( self->nProcs, uniques );
	FreeObject( globals );
}


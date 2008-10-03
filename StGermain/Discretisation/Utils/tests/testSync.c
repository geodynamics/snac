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
** $Id: testSyncVC.c 3056 2005-06-28 06:06:29Z RaquibulHassan $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"
#include "Discretisation/Mesh/Mesh.h"
#include "Discretisation/Utils/Utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define NLOCALS	10


int main( int argc, char *argv[] ) {
	int		rank;
	int		nProcs;
	int		watch;
	Stream*		stream;
	unsigned	nGlobals;
	unsigned	locals[NLOCALS];
	unsigned	nRequired;
	unsigned*	required;
	unsigned*	array;

	void initArray( unsigned** array, unsigned nRequired, unsigned nLocals, unsigned rank );
	void dumpSync( const char* title, Sync* sync );
	
	
	/*
	** Initialise MPI and StGermain, get world info.
	*/
	
	MPI_Init( &argc, &argv );
	MPI_Comm_size( MPI_COMM_WORLD, &nProcs );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	
	Base_Init( &argc, &argv );
	
	DiscretisationGeometry_Init( &argc, &argv );
	DiscretisationShape_Init( &argc, &argv );
	DiscretisationMesh_Init( &argc, &argv );
	DiscretisationUtils_Init( &argc, &argv );
	MPI_Barrier( MPI_COMM_WORLD ); /* Ensures copyright info always come first in output */

	/* Setup a stream. */
	stream = Journal_Register( Info_Type, "myStream" );
	
	/* Which process are we going to watch? */
	watch = (argc >= 2) ? atoi( argv[1] ) : 0;

	/* Set up globals. */
	nGlobals = NLOCALS * nProcs;
	
	
	/*
	** Test 1.
	**	Here we will set up a system with no dependancies on other procs.
	*/
	
	{
		Sync*		sync;
		
		sync = Sync_New( "someName" );

		/* Set up local values. */
		{
			unsigned	local_i;
			
			for( local_i = 0; local_i < NLOCALS; local_i++ ) {
				locals[local_i] = rank * NLOCALS + local_i;
			}
		}
		
		/* No dependancies. */
		nRequired = 0;
		required = NULL;

		/* Setup basic stuff. */
		initArray( &array, nRequired, NLOCALS, rank );

		/* Perform initialisation. */
		Sync_Negotiate( sync, 
				nGlobals, 
				locals, NLOCALS, 
				NULL, 0, 
				required, nRequired, 
				MPI_COMM_WORLD );
		
		/* Prepare send/recv. */
		Sync_SetDomainArray( sync, 
				     sizeof(unsigned), 
				     sizeof(unsigned), array );

		/* Execute. */
		Sync_SendRecv( sync );
		
		/* Check the results. */
		dumpSync( "Test 1:", sync );
		
		Stg_Class_Delete( sync );
	}

	/*
	** Test 2.
	**	Here we will set up a system with some dependancies on other procs.
	*/
	
	{
		Sync*	sync;
		
		sync = Sync_New( "someName" );
		
		/* Set up local values. */
		{
			unsigned	local_i;
			
			for( local_i = 0; local_i < NLOCALS; local_i++ ) {
				locals[local_i] = rank * NLOCALS + local_i;
			}
		}
		
		/* Dependancies. */
		{
			unsigned	req_i;
			
			nRequired = 2;
			required = Memory_Alloc_Array( unsigned, nRequired, "testSync" );
			for( req_i = nRequired; req_i > 0; req_i-- ) {
				required[nRequired - req_i] = ((rank + 1) % nProcs) * NLOCALS + req_i - 1;
			}
		}

		/* Setup basic stuff. */
		initArray( &array, nRequired, NLOCALS, rank );

		/* Perform initialisation. */
		Sync_Negotiate( sync, 
				nGlobals, 
				locals, NLOCALS, 
				NULL, 0, 
				required, nRequired, 
				MPI_COMM_WORLD );
		
		/* Prepare send/recv. */
		Sync_SetDomainArray( sync, 
				     sizeof(unsigned), 
				     sizeof(unsigned), array );

		/* Free the required array. */
		FreeArray( required );

		/* Execute. */
		Sync_SendRecv( sync );
		
		/* Check the results. */
		dumpSync( "Test 2:", sync );
		
		Stg_Class_Delete( sync );
	}
	

	/*
	** Test 3.
	**	Each proc is entirely dependant on another.
	*/
	
	{
		Sync*	sync;
		
		sync = Sync_New( "someName" );
		
		/* Set up local values. */
		{
			unsigned	local_i;
			
			for( local_i = 0; local_i < NLOCALS; local_i++ ) {
				locals[local_i] = rank * NLOCALS + local_i;
			}
		}
		
		/* Dependancies. */
		{
			unsigned	req_i;
			
			nRequired = NLOCALS;
			required = Memory_Alloc_Array( unsigned, nRequired, "testSync" );
			for( req_i = 0; req_i < nRequired; req_i++ ) {
				required[req_i] = ((rank + 1) % nProcs) * NLOCALS + req_i;
			}
		}

		/* Setup basic stuff. */
		initArray( &array, nRequired, NLOCALS, rank );

		/* Perform initialisation. */
		Sync_Negotiate( sync, 
				nGlobals, 
				locals, NLOCALS, 
				NULL, 0, 
				required, nRequired, 
				MPI_COMM_WORLD );
		
		/* Prepare send/recv. */
		Sync_SetDomainArray( sync, 
				     sizeof(unsigned), 
				     sizeof(unsigned), array );
		
		/* Free the required array. */
		FreeArray( required );
		
		/* Execute. */
		Sync_SendRecv( sync );
		
		/* Check the results. */
		dumpSync( "Test 3:", sync );
		
		Stg_Class_Delete( sync );
	}
	
	
	/*
	** Shut everything down.
	*/
	
	DiscretisationUtils_Finalise();
	DiscretisationMesh_Finalise();
	DiscretisationShape_Finalise();
	DiscretisationGeometry_Finalise();
	
	Base_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return EXIT_SUCCESS;
}


void initArray( unsigned** array, unsigned nRequired, unsigned nLocals, unsigned rank ) {
   unsigned	val_i;

   *array = Memory_Alloc_Array( unsigned, nLocals + nRequired, "testSync" );
   for( val_i = 0; val_i < nLocals; val_i++ ) {
      (*array)[val_i] = rank * nLocals + val_i;
   }
}


void buildSinkMap( Sync* sync, unsigned** map ) {
	if( sync->netSink ) {
		unsigned*	tmpMap;
		unsigned	snkInd = 0;
		unsigned	proc_i;
		
		tmpMap = Memory_Alloc_Array( unsigned, sync->netSink, "testSync" );
		
		for( proc_i = 0; proc_i < sync->nProcs; proc_i++ ) {
			unsigned	snk_i;

			for( snk_i = 0; snk_i < sync->nSink[proc_i]; snk_i++ ) {
				tmpMap[snkInd] = sync->sink[proc_i][snk_i] % 10;
				snkInd++;
			}
		}

		*map = tmpMap;
	}
	else {
		*map = NULL;
	}
}


void buildSourceMap( Sync* sync, unsigned** map ) {
	if( sync->netSource ) {
		unsigned*	tmpMap;
		unsigned	src_i;
		
		tmpMap = Memory_Alloc_Array( unsigned, sync->netSource, "testSync" );
		
		for( src_i = 0; src_i < sync->netSource; src_i++ ) {
			tmpMap[src_i] = src_i;
		}
		
		*map = tmpMap;
	}
	else {
		*map = NULL;
	}
}


/*
** A whole bunch of printing tools.
*/

typedef struct {
	unsigned	col;
	unsigned	indent;
	unsigned	tabWidth;
	
	unsigned	master;
	unsigned	nProcs;
	unsigned	rank;
	MPI_Comm	comm;
} PrintContext;


void expandTabs( PrintContext* ctx, char* str, const char* tabStr ) {
	char*	curPos;
	char*	res;
	
	curPos = str;
	do {
		res = strpbrk( curPos, tabStr );
		if( res ) {
			unsigned	nextTab;
			unsigned	tab_i;
			
			*res = '\0';
			ctx->col += strlen( curPos );
			printf( "%s", curPos );
			
			/* Calculate the next tab stop. */
			nextTab = ctx->tabWidth - ctx->col % ctx->tabWidth;
			ctx->col += nextTab;
			for( tab_i = 0; tab_i < nextTab; tab_i++ ) {
				printf( " " );
			}
			
			curPos = res + 1;
		}
	}
	while( res );
	
	/* Print the last section. */
	ctx->col += strlen( curPos );
	printf( "%s", curPos );
}


void print( PrintContext* ctx, const char* fmt, ... ) {
	va_list	args;
	char*	tmpFmt;
	unsigned	maxStrSize;
	
	char*	tabCode = "\xFF";
	char*	crCode = "\xFE";
	
	
	if( !fmt ) {
		return;
	}
	
	/* Start the variable argument list. */
	va_start( args, fmt );
	
	
	/*
	** If we need to indent, then do so.
	*/
	
	if( ctx->col < ctx->indent * ctx->tabWidth ) {
		unsigned	indSize = ctx->indent * ctx->tabWidth - ctx->col;
		unsigned	tab_i;
		
		for( tab_i = 0; tab_i < indSize; tab_i++ ) {
			printf( " " );
		}
		
		ctx->col = ctx->indent * ctx->tabWidth;
	}
	
	
	/*
	** Process the format string looking for tabs and newlines.  Any we find we'll encode so that c formatted print
	** won't modify it.  We want to mess with it.
	*/
	
	{
		unsigned	chr_i;
		unsigned	len = strlen( fmt ) + 1;
		unsigned	curChr = 0;
		
		/* Destination format buffer. */
		tmpFmt = Memory_Alloc_Array( char, len, "print" );
		
		for( chr_i = 0; chr_i < len; chr_i++ ) {
			if( fmt[chr_i] == '\t' ) {
				tmpFmt[curChr] = tabCode[0];
			}
			else if( fmt[chr_i] == '\n' ) {
				tmpFmt[curChr] = crCode[0];
			}
			else {
				tmpFmt[curChr] = fmt[chr_i];
			}
			
			curChr++;
		}
		
		/* TODO: Need to implement a system of counting how long the string is going to be.  Will look for '%' format
		   discriptors in the format string and use the maximum number of digits for that type.  For the moment, just
		   hard code a maximum. */
		maxStrSize = 1024;
	}
	
	
	/*
	** Dump the string to a new buffer and translate back while printing.
	*/
	
	{
		char*	tmpStr;
		char*	res;
		char*	curPos;
		
		tmpStr = Memory_Alloc_Array( char, maxStrSize, "print" );
		
		/* Pass to vsprintf. */
		vsprintf( tmpStr, tmpFmt, args );
		
		/* Free destination buffer. */
		FreeArray( tmpFmt );
		
		/* Tokenize on carriage returns and tab stops respectively. */
		curPos = tmpStr;
		do {
			res = strpbrk( curPos, crCode );
			if( res ) {
				/* Search for tab stops. */
				*res = '\0';
				expandTabs( ctx, curPos, tabCode );
				
				/* Print carriage return. */
				ctx->col = 0;
				printf( "\n" );
				
				curPos = res + 1;
			}
		}
		while( res );
		
		/* Search for tabs in the last section. */
		expandTabs( ctx, curPos, tabCode );
		
		/* Free memory. */
		FreeArray( tmpStr );
	}
	
	/* End the argument list. */
	va_end( args );
}


void printArray_unsigned( PrintContext* ctx, 
					 const char*	title, 
					 unsigned		size, 
					 unsigned*	array )
{
	if( title ) {
		print( ctx, title );
	}
	print( ctx, "[]:\n" );
	ctx->indent++;
	
	if( size > 0 && array ) {
		unsigned	item_i;
		
		for( item_i = 0; item_i < size; item_i++ ) {
			print( ctx, "[%d]:\t%d\n", item_i, array[item_i] );
		}
	}
	else {
		print( ctx, "empty\n" );
	}
	
	ctx->indent--;
}


void printArray2D_unsigned( PrintContext*	ctx, 
					   const char*		title, 
					   unsigned		nBlocks, 
					   unsigned*		sizes, 
					   unsigned**		array )
{
	if( title ) {
		print( ctx, title );
	}
	print( ctx, "[][]:\n" );
	ctx->indent++;
	
	if( nBlocks > 0 && array && sizes ) {
		unsigned	block_i;
		char*	title;
		unsigned	maxTitleLen;
		
		maxTitleLen = (unsigned)floor( log10( (double)nBlocks ) ) + 4;
		title = Memory_Alloc_Array( char, maxTitleLen, "printArray2D_unsigned" );
		
		for( block_i = 0; block_i < nBlocks; block_i++ ) {
			sprintf( title, "[%d]", block_i );
			printArray_unsigned( ctx, title, sizes[block_i], array[block_i] );
		}
		
		FreeArray( title );
	}
	else {
		print( ctx, "empty\n" );
	}
	
	ctx->indent--;
}


const unsigned	tagBeginPrinting = 1021;

void printPreBarrier( PrintContext* ctx ) {
	/* We need to ensure that if we're running in parallel each proc prints in order.  We'll do this by waiting for a 
	   go-ahead from the previous proc. */
	if( ctx->rank > 0 ) {
		MPI_Status	status;
		unsigned		buf;
		
		MPI_Recv( &buf, 1, MPI_UNSIGNED, ctx->rank - 1, tagBeginPrinting, ctx->comm, &status );
	}
}


void printPostBarrier( PrintContext* ctx ) {
	/* Before signalling, make sure our output is flushed. */
	fflush( stdout );
	
	/* Signal the next proc to begin printing. */
	if( ctx->nProcs > 1 && ctx->rank < ctx->nProcs - 1 ) {
		unsigned		buf;
		
		MPI_Send( &buf, 1, MPI_UNSIGNED, ctx->rank + 1, tagBeginPrinting, ctx->comm );
	}
	
	/* This will make sure proc 0 doesn't start printing again until all others are done. */
	MPI_Barrier( ctx->comm );
}


void dumpSync( const char* title, Sync* sync ) {
	PrintContext	ctx;
	
	/* Set up the context. */
	ctx.indent = 0;
	ctx.col = 0;
	ctx.tabWidth = 5;
	ctx.nProcs = sync->nProcs;
	ctx.rank = sync->rank;
	ctx.comm = sync->comm;
	
	/* Dump. */
	if( title ) {
		print( &ctx, title );
		print( &ctx, "\n" );
	}
	ctx.indent++;
	print( &ctx, "Sync on proc %d:\n", sync->rank );
	ctx.indent++;
	printArray_unsigned( &ctx, "nSource", sync->nProcs, sync->nSource );
	printArray2D_unsigned( &ctx, "source", sync->nProcs, sync->nSource, sync->source );
	printArray_unsigned( &ctx, "nSink", sync->nProcs, sync->nSink );
	printArray2D_unsigned( &ctx, "sink", sync->nProcs, sync->nSink, sync->sink );
	print( &ctx, "\n" );
}


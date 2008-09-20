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
** $Id: testCommTopology.c 2136 2004-09-30 02:47:13Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <mpi.h>

#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"


Bool testConstruct( unsigned rank, unsigned nProcs, unsigned watch ) {
	RangeSet*	set;

	set = RangeSet_New();
	FreeObject( set );

	return True;
}


Bool testIndices( unsigned rank, unsigned nProcs, unsigned watch ) {
	Bool		result = True;
	unsigned	nInds = 100;
	unsigned	inds[100];
	RangeSet*	set;
	unsigned	nDstInds;
	unsigned*	dstInds;
	unsigned	i;

	for( i = 0; i < nInds; i++ )
		inds[i] = i;

	set = RangeSet_New();

	RangeSet_SetIndices( set, nInds, inds );
	if( rank == watch ) {
		if( set->nInds != nInds || 
		    set->nRanges != 1 )
		{
			result = False;
			goto done;
		}
	}

	RangeSet_GetIndices( set, &nDstInds, &dstInds );
	if( rank == watch ) {
		unsigned	ind_i;

		if( nDstInds != nInds ) {
			FreeArray( dstInds );
		}

		for( ind_i = 0; ind_i < nDstInds; ind_i++ ) {
			if( dstInds[ind_i] != inds[ind_i] )
				break;
		}
		if( ind_i < nDstInds ) {
			result = False;
			goto done;
		}
	}

done:
	FreeArray( dstInds );
	FreeObject(set );

	return result;
}


Bool testRanges( unsigned rank, unsigned nProcs, unsigned watch ) {
	Bool		result = True;
	unsigned	nInds = 100;
	unsigned	inds[100];
	RangeSet*	set;
	unsigned	i;

	for( i = 0; i < nInds; i++ )
		inds[i] = (i/10)*10 + i;

	set = RangeSet_New();

	RangeSet_SetIndices( set, nInds, inds );
	if( rank == watch ) {
		if( set->nInds != nInds || 
		    set->nRanges != nInds/10 )
		{
			result = False;
			goto done;
		}

		for( i = 0; i < nInds; i++ ) {
			if( set->ranges[i/10].begin != (i/10)*20 || 
			    set->ranges[i/10].end != (i/10)*20 + 10 || 
			    set->ranges[i/10].step != 1 )
			{
				result = False;
				goto done;
			}
		}
	}

done:
	FreeObject( set );

	return result;
}


Bool testUnion( unsigned rank, unsigned nProcs, unsigned watch ) {
	unsigned	nInds = 100;
	unsigned	inds[100];
	RangeSet*	set0;
	RangeSet*	set1;
	unsigned	i;

	for( i = 0; i < nInds; i++ )
		inds[i] = (i/10)*10 + i;
	set0 = RangeSet_New();
	RangeSet_SetIndices( set0, nInds, inds );

	for( i = 0; i < nInds; i++ )
		inds[i] = (i/10)*10 + 100 + i;
	set1 = RangeSet_New();
	RangeSet_SetIndices( set1, nInds, inds );

	RangeSet_Union( set0, set1 );
	FreeObject( set1 );
	if( rank == watch ) {
		if( RangeSet_GetNIndices( set0 ) != nInds + nInds / 2 || 
		    RangeSet_GetNRanges( set0 ) != nInds / 10 + nInds / 20 )
		{
			FreeObject( set0 );
			return False;
		}
	}

	FreeObject( set0 );

	return True;
}


Bool testIntersection( unsigned rank, unsigned nProcs, unsigned watch ) {
	unsigned	nInds = 100;
	unsigned	inds[100];
	RangeSet*	set0;
	RangeSet*	set1;
	unsigned	i;

	for( i = 0; i < nInds; i++ )
		inds[i] = (i/10)*10 + i;
	set0 = RangeSet_New();
	RangeSet_SetIndices( set0, nInds, inds );

	for( i = 0; i < nInds; i++ )
		inds[i] = (i/10)*10 + 100 + i;
	set1 = RangeSet_New();
	RangeSet_SetIndices( set1, nInds, inds );

	RangeSet_Intersection( set0, set1 );
	FreeObject( set1 );
	if( rank == watch ) {
		if( RangeSet_GetNIndices( set0 ) != nInds / 2 || 
		    RangeSet_GetNRanges( set0 ) != nInds / 20 )
		{
			FreeObject( set0 );
			return False;
		}
	}

	FreeObject( set0 );

	return True;
}


Bool testSubtraction( unsigned rank, unsigned nProcs, unsigned watch ) {
	unsigned	nInds = 100;
	unsigned	inds[100];
	RangeSet*	set0;
	RangeSet*	set1;
	unsigned	i;

	for( i = 0; i < nInds; i++ )
		inds[i] = (i/10)*10 + i;
	set0 = RangeSet_New();
	RangeSet_SetIndices( set0, nInds, inds );

	for( i = 0; i < nInds; i++ )
		inds[i] = (i/10)*10 + 100 + i;
	set1 = RangeSet_New();
	RangeSet_SetIndices( set1, nInds, inds );

	RangeSet_Subtraction( set0, set1 );
	FreeObject( set1 );
	if( rank == watch ) {
		if( RangeSet_GetNIndices( set0 ) != nInds / 2 || 
		    RangeSet_GetNRanges( set0 ) != nInds / 20 )
		{
			FreeObject( set0 );
			return False;
		}
	}

	FreeObject( set0 );

	return True;
}


Bool testPickle( unsigned rank, unsigned nProcs, unsigned watch ) {
	unsigned	nInds = 100;
	unsigned	inds[100];
	RangeSet*	set;
	unsigned	nBytes;
	Stg_Byte*		bytes;
	unsigned	i;

	for( i = 0; i < nInds; i++ )
		inds[i] = (i/10)*10 + i;
	set = RangeSet_New();
	RangeSet_SetIndices( set, nInds, inds );

	RangeSet_Pickle( set, &nBytes, &bytes );
	RangeSet_Clear( set );
	RangeSet_Unpickle( set, nBytes, bytes );

	if( rank == watch ) {
		if( set->nInds != nInds || 
		    set->nRanges != nInds/10 )
		{
			FreeObject( set );
			return False;
		}

		for( i = 0; i < nInds; i++ ) {
			if( set->ranges[i/10].begin != (i/10)*20 || 
			    set->ranges[i/10].end != (i/10)*20 + 10 || 
			    set->ranges[i/10].step != 1 )
			{
				FreeObject( set );
				return False;
			}
		}
	}

	FreeObject( set );

	return True;
}


#define nTests	7

TestSuite_Test	tests[nTests] = {{"construct", testConstruct, 10}, 
				 {"set indices", testIndices, 10}, 
				 {"ranges", testRanges, 10 }, 
				 {"union", testUnion, 10}, 
				 {"intersection", testIntersection, 10}, 
				 {"subtraction", testSubtraction, 10}, 
				 {"pickle", testPickle, 10}};


int main( int argc, char* argv[] ) {
	TestSuite*	suite;

	/* Initialise MPI, get world info. */
	MPI_Init( &argc, &argv );

	/* Initialise StGermain. */
	BaseFoundation_Init( &argc, &argv );
	BaseIO_Init( &argc, &argv );
	BaseContainer_Init( &argc, &argv );

	/* Create the test suite. */
	suite = TestSuite_New();
	TestSuite_SetProcToWatch( suite, (argc >= 2) ? atoi( argv[1] ) : 0 );
	TestSuite_SetTests( suite, nTests, tests );

	/* Run the tests. */
	TestSuite_Run( suite );

	/* Destroy test suites. */
	FreeObject( suite );

	/* Finalise StGermain. */
	BaseContainer_Finalise();
	BaseIO_Finalise();
	BaseFoundation_Finalise();

	/* Close off MPI */
	MPI_Finalize();

	return MPI_SUCCESS;
}

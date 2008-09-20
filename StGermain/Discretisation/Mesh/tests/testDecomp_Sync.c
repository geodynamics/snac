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
** $Id: testDecomp_Sync.c 2136 2004-09-30 02:47:13Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <mpi.h>

#include "Base/Base.h"
#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Mesh/Mesh.h"


Bool testRemotes( unsigned rank, unsigned nProcs, unsigned watch );
Bool testSnkSrc( unsigned rank, unsigned nProcs, unsigned watch );
Bool testArrays( unsigned rank, unsigned nProcs, unsigned watch );
Bool testClaim( unsigned rank, unsigned nProcs, unsigned watch );

Decomp* buildDecomp( unsigned rank, unsigned nProcs, unsigned nLocals );
void buildRequired( unsigned rank, unsigned nProcs, 
		    unsigned* nRequired, unsigned** required );


#define nTests	4

TestSuite*	suite;
TestSuite_Test	tests[nTests] = {{"set remotes", testRemotes}, 
				 {"sink/sources", testSnkSrc}, 
				 {"arrays", testArrays}, 
				 {"claim", testClaim}};


Bool testRemotes( unsigned rank, unsigned nProcs, unsigned watch ) {
	Decomp*		decomp;
	Decomp_Sync*	sync;
	unsigned	nLocals = 100;
	unsigned	remPerSide = nLocals / 10;
	unsigned	nRemotes = remPerSide * ((rank > 0 && rank < nProcs - 1) ? 2 : 1) * ((nProcs > 1) ? 1 : 0);
	unsigned*	remotes;
	unsigned	r_i;

	decomp = buildDecomp( rank, nProcs, nLocals );
	sync = Decomp_Sync_New( "" );
	Decomp_Sync_SetDecomp( sync, decomp );

	if( nRemotes ) {
		remotes = Memory_Alloc_Array_Unnamed( unsigned, nRemotes );
		if( rank > 0 ) {
			for( r_i = 0; r_i < remPerSide; r_i++ )
				remotes[r_i] = rank * nLocals - remPerSide + r_i;
		}
		if( rank < nProcs - 1 ) {
			for( r_i = 0; r_i < remPerSide; r_i++ ) {
				unsigned	ind = r_i + ((rank > 0) ? remPerSide : 0);

				remotes[ind] = (rank + 1) * nLocals + r_i;
			}
		}
	}
	else
		remotes = NULL;

	Decomp_Sync_SetRemotes( sync, nRemotes, remotes );

	if( rank == watch ) {
		if( sync->nRemotes != nRemotes ) {
			FreeArray( remotes );
			FreeObject( sync );
			FreeObject( decomp );
			return False;
		}

		if( rank > 0 ) {
			for( r_i = 0; r_i < remPerSide; r_i++ ) {
				if( sync->remotes[r_i] != remotes[r_i] ) {
					FreeArray( remotes );
					FreeObject( sync );
					FreeObject( decomp );
					return False;
				}
			}
		}
		if( rank < nProcs - 1 ) {
			for( r_i = 0; r_i < remPerSide; r_i++ ) {
				unsigned	ind = r_i + ((rank > 0) ? remPerSide : 0);

				if( sync->remotes[ind] != remotes[ind] ) {
					FreeArray( remotes );
					FreeObject( sync );
					FreeObject( decomp );
					return False;
				}
			}
		}
	}

	FreeArray( remotes );
	FreeObject( sync );
	FreeObject( decomp );

	return True;
}

Bool testSnkSrc( unsigned rank, unsigned nProcs, unsigned watch ) {
	Decomp*		decomp;
	Decomp_Sync*	sync;
	unsigned	nLocals = 100;
	unsigned	remPerSide = nLocals / 10;
	unsigned	nRemotes = remPerSide * ((rank > 0 && rank < nProcs - 1) ? 2 : 1) * ((nProcs > 1) ? 1 : 0);
	unsigned*	remotes;
	unsigned	r_i;

	decomp = buildDecomp( rank, nProcs, nLocals );
	sync = Decomp_Sync_New( "" );
	Decomp_Sync_SetDecomp( sync, decomp );

	if( nRemotes ) {
		remotes = Memory_Alloc_Array_Unnamed( unsigned, nRemotes );
		if( rank > 0 ) {
			for( r_i = 0; r_i < remPerSide; r_i++ )
				remotes[r_i] = rank * nLocals - remPerSide + r_i;
		}
		if( rank < nProcs - 1 ) {
			for( r_i = 0; r_i < remPerSide; r_i++ ) {
				unsigned	ind = r_i + ((rank > 0) ? remPerSide : 0);

				remotes[ind] = (rank + 1) * nLocals + r_i;
			}
		}
	}
	else
		remotes = NULL;

	Decomp_Sync_SetRemotes( sync, nRemotes, remotes );

	if( rank == watch ) {
		if( sync->netSnks != nRemotes || sync->netSrcs != nRemotes ) {
			FreeArray( remotes );
			FreeObject( sync );
			FreeObject( decomp );
			return False;
		}
	}

	FreeArray( remotes );
	FreeObject( sync );
	FreeObject( decomp );

	return True;
}

typedef struct {
	int	one;
	int	two;
	int	three;
} theStruct;

Bool testArrays( unsigned rank, unsigned nProcs, unsigned watch ) {
	Decomp*		decomp;
	Decomp_Sync*	sync;
	unsigned	nLocals = 100;
	unsigned	remPerSide = nLocals / 10;
	unsigned	nRemotes = remPerSide * ((rank > 0 && rank < nProcs - 1) ? 2 : 1) * ((nProcs > 1) ? 1 : 0);
	unsigned*	remotes;
	int*		intLocals;
	int*		intRemotes;
	theStruct*	structLocals;
	theStruct*	structRemotes;
	unsigned	r_i;

	decomp = buildDecomp( rank, nProcs, nLocals );
	sync = Decomp_Sync_New( "" );
	Decomp_Sync_SetDecomp( sync, decomp );

	if( nRemotes ) {
		remotes = Memory_Alloc_Array_Unnamed( unsigned, nRemotes );
		if( rank > 0 ) {
			for( r_i = 0; r_i < remPerSide; r_i++ )
				remotes[r_i] = rank * nLocals - remPerSide + r_i;
		}
		if( rank < nProcs - 1 ) {
			for( r_i = 0; r_i < remPerSide; r_i++ ) {
				unsigned	ind = r_i + ((rank > 0) ? remPerSide : 0);

				remotes[ind] = (rank + 1) * nLocals + r_i;
			}
		}
	}
	else
		remotes = NULL;

	if( nLocals ) {
		intLocals = Memory_Alloc_Array_Unnamed( int, nLocals );
		structLocals = Memory_Alloc_Array_Unnamed( theStruct, nLocals );
	}
	else {
		intLocals = NULL;
		structLocals = NULL;
	}
	if( nRemotes ) {
		intRemotes = Memory_Alloc_Array_Unnamed( int, nRemotes );
		structRemotes = Memory_Alloc_Array_Unnamed( theStruct, nRemotes );
	}
	else {
		intRemotes = NULL;
		structRemotes = NULL;
	}
	for( r_i = 0; r_i < nLocals; r_i++ ) {
		intLocals[r_i] = rank;
		structLocals[r_i].one = -rank;
		structLocals[r_i].two = rank;
		structLocals[r_i].three = -rank;
	}
	for( r_i = 0; r_i < nRemotes; r_i++ ) {
		intRemotes[r_i] = rank;
		structRemotes[r_i].one = -rank;
		structRemotes[r_i].two = rank;
		structRemotes[r_i].three = -rank;
	}

	Decomp_Sync_SetRemotes( sync, nRemotes, remotes );
	Decomp_Sync_AddArray( sync, intLocals, intRemotes, sizeof(int), sizeof(int), sizeof(int) );
	Decomp_Sync_AddArray( sync, &structLocals[0].two, &structRemotes[0].two, 
			      sizeof(theStruct), sizeof(theStruct), sizeof(int) );
	Decomp_Sync_Sync( sync );

	FreeArray( remotes );
	FreeObject( sync );
	FreeObject( decomp );

	if( rank == watch ) {
		for( r_i = 0; r_i < nLocals; r_i++ ) {
			if( intLocals[r_i] != rank || structLocals[r_i].two != rank || 
			    structLocals[r_i].one != -rank || structLocals[r_i].three != -rank )
			{
				break;
			}
		}
		if( r_i < nLocals ) {
			FreeArray( intLocals );
			FreeArray( intRemotes );
			FreeArray( structLocals );
			FreeArray( structRemotes );
			return False;
		}

		if( rank > 0 ) {
			for( r_i = 0; r_i < remPerSide; r_i++ ) {
				if( intRemotes[r_i] != rank - 1 || structRemotes[r_i].two != rank - 1 || 
				    structRemotes[r_i].one != -rank || structRemotes[r_i].three != -rank )
				{
					break;
				}
			}
			if( r_i < remPerSide ) {
				FreeArray( intLocals );
				FreeArray( intRemotes );
				FreeArray( structLocals );
				FreeArray( structRemotes );
				return False;
			}
		}
		if( rank < nProcs - 1 ) {
			for( r_i = 0; r_i < remPerSide; r_i++ ) {
				unsigned	ind = r_i + ((rank > 0) ? remPerSide : 0);

				if( intRemotes[ind] != rank + 1 || structRemotes[ind].two != rank + 1 || 
				    structRemotes[ind].one != -rank || structRemotes[ind].three != -rank )
				{
					break;
				}
			}
			if( r_i < remPerSide ) {
				FreeArray( intLocals );
				FreeArray( intRemotes );
				FreeArray( structLocals );
				FreeArray( structRemotes );
				return False;
			}
		}
	}

	FreeArray( intLocals );
	FreeArray( intRemotes );
	FreeArray( structLocals );
	FreeArray( structRemotes );

	return True;
}

Bool testClaim( unsigned rank, unsigned nProcs, unsigned watch ) {
	Decomp*		decomp;
	Decomp_Sync*	sync;
	unsigned	nRequired;
	unsigned*	required;

	buildRequired( rank, nProcs, &nRequired, &required );
	decomp = Decomp_New( "" );
	sync = Decomp_Sync_New( "" );
	Decomp_Sync_SetDecomp( sync, decomp );
	Decomp_Sync_Decompose( sync, nRequired, required );

	if( rank == watch ) {
	}

	FreeArray( required );
	FreeObject( sync );
	FreeObject( decomp );

	return True;
}


Decomp* buildDecomp( unsigned rank, unsigned nProcs, unsigned nLocals ) {
	Decomp*		decomp = Decomp_New( "" );
	unsigned*	locals;
	unsigned	l_i;

	locals = Memory_Alloc_Array_Unnamed( unsigned, nLocals );
	for( l_i = 0; l_i < nLocals; l_i++ )
		locals[l_i] = rank * nLocals + l_i;
	Decomp_SetLocals( decomp, nLocals, locals );

	FreeArray( locals );

	return decomp;
}

void buildRequired( unsigned rank, unsigned nProcs, 
		    unsigned* nRequired, unsigned** required )
{
	unsigned	start;
	unsigned	r_i;

	*nRequired = 100;
	*required = Memory_Alloc_Array_Unnamed( unsigned, *nRequired );
	start = rank * (*nRequired - 10);

	for( r_i = 0; r_i < *nRequired; r_i++ )
		(*required)[r_i] = start + r_i;
}


int main( int argc, char* argv[] ) {
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

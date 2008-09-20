/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003,
**	Steve Quenette, 110 Victoria Street, Melbourne, Victoria, 3053, Australia.
**	Californian Institute of Technology, 1200 East California Boulevard, Pasadena, California, 91125, USA.
**	University of Texas, 1 University Station, Austin, Texas, 78712, USA.
**
** Authors:
**	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
**	Stevan M. Quenette, Visitor in Geophysics, Caltech.
**	Luc Lavier, Research Scientist, The University of Texas. (luc@utig.ug.utexas.edu)
**	Luc Lavier, Research Scientist, Caltech.
**
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2, or (at your option) any
** later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**
** $Id$
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>

#include "units.h"
#include "types.h"
#include "shortcuts.h"
#include "Parallel.h"

#include <string.h>

/* Rank neighbour relative location information. Utilises same indexing as elment-neighbours (ijk26)...
   L = left, R = right, B = bottom, T = top, A = away (back), C = close (front), NC = neighbour count */
#define LBA	 0
#define LB	 9
#define LBC	17
#define LA	 3
#define L	12
#define LC	20
#define LTA	 6
#define LT	14
#define LTC	23
#define BA	 1
#define B	10
#define BC	18
#define A	 4
#define C	21
#define TA	 7
#define T	15
#define TC	24
#define RBA	 2
#define RB	11
#define RBC	19
#define RA	 5
#define R	13
#define RC	22
#define RTA	 8
#define RT	16
#define RTC	25
#define NC	26

typedef IndexSet* IndexSetPtr;
typedef IndexSetPtr BorderIndexSets[NC];
typedef IndexSet* (*CreateIndexSetFunction) ( Mesh* mesh, BorderIndexSets tmpSet );
static IndexSet* CreateLBA(	Mesh* mesh, BorderIndexSets tmpSet );
static IndexSet* CreateLB(	Mesh* mesh, BorderIndexSets tmpSet );
static IndexSet* CreateLBC(	Mesh* mesh, BorderIndexSets tmpSet );
static IndexSet* CreateLA(	Mesh* mesh, BorderIndexSets tmpSet );
static IndexSet* CreateL(	Mesh* mesh, BorderIndexSets tmpSet );
static IndexSet* CreateLC(	Mesh* mesh, BorderIndexSets tmpSet );
static IndexSet* CreateLTA(	Mesh* mesh, BorderIndexSets tmpSet );
static IndexSet* CreateLT(	Mesh* mesh, BorderIndexSets tmpSet );
static IndexSet* CreateLTC(	Mesh* mesh, BorderIndexSets tmpSet );
static IndexSet* CreateBA(	Mesh* mesh, BorderIndexSets tmpSet );
static IndexSet* CreateB(	Mesh* mesh, BorderIndexSets tmpSet );
static IndexSet* CreateBC(	Mesh* mesh, BorderIndexSets tmpSet );
static IndexSet* CreateA(	Mesh* mesh, BorderIndexSets tmpSet );
static IndexSet* CreateC(	Mesh* mesh, BorderIndexSets tmpSet );
static IndexSet* CreateTA(	Mesh* mesh, BorderIndexSets tmpSet );
static IndexSet* CreateT(	Mesh* mesh, BorderIndexSets tmpSet );
static IndexSet* CreateTC(	Mesh* mesh, BorderIndexSets tmpSet );
static IndexSet* CreateRBA(	Mesh* mesh, BorderIndexSets tmpSet );
static IndexSet* CreateRB(	Mesh* mesh, BorderIndexSets tmpSet );
static IndexSet* CreateRBC(	Mesh* mesh, BorderIndexSets tmpSet );
static IndexSet* CreateRA(	Mesh* mesh, BorderIndexSets tmpSet );
static IndexSet* CreateR(	Mesh* mesh, BorderIndexSets tmpSet );
static IndexSet* CreateRC(	Mesh* mesh, BorderIndexSets tmpSet );
static IndexSet* CreateRTA(	Mesh* mesh, BorderIndexSets tmpSet );
static IndexSet* CreateRT(	Mesh* mesh, BorderIndexSets tmpSet );
static IndexSet* CreateRTC(	Mesh* mesh, BorderIndexSets tmpSet );

static const CreateIndexSetFunction CreateIndexSet[3][3][3] = {
	{ /* I = 0 */
		{ CreateLBA,	CreateLB,	CreateLBC	},	/* J = 0 */
		{ CreateLA,	CreateL,	CreateLC	},	/* J = 1 */
		{ CreateLTA,	CreateLT,	CreateLTC	}	/* J = 2 */
	},
	{ /* I = 1 */
		{ CreateBA,	CreateB,	CreateBC	},	/* J = 0 */
		{ CreateA,	0,		CreateC		},	/* J = 1 */
		{ CreateTA,	CreateT,	CreateTC	}	/* J = 2 */
	},
	{ /* I = 2 */
		{ CreateRBA,	CreateRB,	CreateRBC	},	/* J = 0 */
		{ CreateRA,	CreateR,	CreateRC	},	/* J = 1 */
		{ CreateRTA,	CreateRT,	CreateRTC	}	/* J = 2 */
	}
};

/* Textual name of this class */
const Type Snac_Parallel_Type = "Snac_Parallel";


Snac_Parallel* Snac_Parallel_New( MPI_Comm communicator, Mesh* mesh ) {
	return _Snac_Parallel_New( 
		sizeof(Snac_Parallel), 
		Snac_Parallel_Type, 
		_Snac_Parallel_Delete, 
		Snac_Parallel_Print, 
		NULL, 
		communicator,
		mesh );
}


void Snac_Parallel_Init( Snac_Parallel* self, MPI_Comm communicator, Mesh* mesh ) {
	/* General info */
	self->type = Snac_Parallel_Type;
	self->_sizeOfSelf = sizeof( Snac_Parallel );
	self->_deleteSelf = False;

	/* Virtual info */
	self->_delete = _Snac_Parallel_Delete;
	self->_print = Snac_Parallel_Print;
	self->_copy = NULL;
	_Stg_Class_Init( (Stg_Class*)self );

	/* Snac_Parallel info */
	_Snac_Parallel_Init( self, communicator, mesh );
}


Snac_Parallel* _Snac_Parallel_New(
		SizeT					_sizeOfSelf,
		Type					type,
		Stg_Class_DeleteFunction*			_delete,
		Stg_Class_PrintFunction*			_print,
		Stg_Class_CopyFunction*			_copy, 
		MPI_Comm				communicator, 
		Mesh*					mesh )
{
	Snac_Parallel* self;

	/* Allocate memory */
	self = (Snac_Parallel*)_Stg_Class_New( _sizeOfSelf, type, _delete, _print, _copy );

	/* General info */

	/* Virtual info */

	/* Snac_Parallel info */
	_Snac_Parallel_Init( self, communicator, mesh );

	return self;
}

void _Snac_Parallel_Init( Snac_Parallel* self, MPI_Comm communicator, Mesh* mesh ) {
	if( self->rank == 0 ) Journal_Printf( Journal_Register( DebugStream_Type, AbstractContext_Type ), "In: %s\n", __func__ );

	self->debug = Journal_Register( DebugStream_Type, self->type );
	self->info = Journal_Register( InfoStream_Type, self->type );
	
	self->communicator = communicator;
	self->mesh = mesh;

	/* Ensure the decomosition only occurs across nodes */
	Journal_Firewall( 
		!(self->mesh->layout->decomp->allowPartitionOnElement && self->mesh->isBuilt),
		Journal_Register( ErrorStream_Type, self->type ), 
		"Error: Mesh already built using illegal partition on element.\n" );
	if( self->mesh->layout->decomp->allowPartitionOnElement == True ) {
		if( !self->mesh->isBuilt ) {
			Journal_Printf( 
				Journal_Register( ErrorStream_Type, self->type ), 
				"Warning: Setting Mesh to partition exclusively on node.\n" );
			self->mesh->layout->decomp->allowPartitionOnElement = False;
			self->mesh->layout->decomp->allowPartitionOnNode = True;
		}
	}
	
	self->rnCount = 0;
	self->rn = 0;
	self->decompDimCount = 0;
	self->decompRankCount[0] = 0;
	self->decompRankCount[1] = 0;
	self->decompRankCount[2] = 0;
	self->boundarySet = 0;
	self->boundaryArray = 0;
	self->boundaryCount = 0;
	self->boundaryForce = 0;
	self->boundaryForceRemote = 0;
	self->boundaryIMass = 0;
	self->boundaryIMassRemote = 0;
}


void _Snac_Parallel_Delete( void* parallel ) {
	Snac_Parallel*	self = (Snac_Parallel*)parallel;
	PartitionIndex	rn_I;
	
	if( self->rank == 0 ) Journal_Printf( self->debug, "In: %s\n", __func__ );

	if( self->boundarySet ) {
		for( rn_I = 0; rn_I < self->rnCount; rn_I++ ) {
			if( self->boundarySet[rn_I] ) {
				Stg_Class_Delete( self->boundarySet[rn_I] );
			}
		}
		Memory_Free( self->boundarySet );
	}
	
	if( self->boundaryForce ) {
		for( rn_I = 0; rn_I < self->rnCount; rn_I++ ) {
			if( self->boundaryForce[rn_I] ) {
				Memory_Free( self->boundaryForce[rn_I] );
			}
		}
		Memory_Free( self->boundaryForce );
	}
	
	if( self->boundaryForceRemote ) {
		for( rn_I = 0; rn_I < self->rnCount; rn_I++ ) {
			if( self->boundaryForceRemote[rn_I] ) {
				Memory_Free( self->boundaryForceRemote[rn_I] );
			}
		}
		Memory_Free( self->boundaryForceRemote );
	}
	
	if( self->boundaryIMass ) {
		for( rn_I = 0; rn_I < self->rnCount; rn_I++ ) {
			if( self->boundaryIMass[rn_I] ) {
				Memory_Free( self->boundaryIMass[rn_I] );
			}
		}
		Memory_Free( self->boundaryIMass );
	}
	
	if( self->boundaryIMassRemote ) {
		for( rn_I = 0; rn_I < self->rnCount; rn_I++ ) {
			if( self->boundaryIMassRemote[rn_I] ) {
				Memory_Free( self->boundaryIMassRemote[rn_I] );
			}
		}
		Memory_Free( self->boundaryIMassRemote );
	}
	
	Memory_Free( self->rn );
	
	/* Stg_Class_Delete parent class */
	_Stg_Class_Delete( parallel );
}



void Snac_Parallel_Print( void* parallel, Stream* stream ) {
	Snac_Parallel* self = (Snac_Parallel*)parallel;

	/* General info */
	Journal_Printf( stream, "Snac_Parallel (%p):\n", self );

	/* Virtual info */

	/* Snac_Parallel info */

	/* Parent class info */
	_Stg_Class_Print( parallel, stream );
}


void Snac_Parallel_Build( void* parallel ) {
	Snac_Parallel*	self = (Snac_Parallel*)parallel;
	int 		nproc;
	int		rank;
	Partition_Index	rn_I;
	IJK		ijk;
	Mesh*		mesh = self->mesh;
	HexaMD*		decomp = (HexaMD*)mesh->layout->decomp;
	IJK26Topology*	topology = (IJK26Topology*)decomp->procTopology;
	BorderIndexSets tmpSet;
	
	Journal_Printf( self->debug, "In: %s\n", __func__ );

	/* Create a local copy of essential decomposition info. Create base arrays. */
	MPI_Comm_size( self->communicator, &nproc );
	MPI_Comm_rank( self->communicator, &rank );
	self->nproc = nproc;
	self->rank = rank;
	self->rnCount = _HexaMD_Shadow_ProcCount( (HexaMD*)self->mesh->layout->decomp );
	self->rn = Memory_Alloc_Array( Partition_Index, self->rnCount,  "rn" );
	self->decompDimCount = decomp->numPartitionedDims;
	self->decompRankCount[0] = decomp->partition3DCounts[0];
	self->decompRankCount[1] = decomp->partition3DCounts[1];
	self->decompRankCount[2] = decomp->partition3DCounts[2];
	self->boundarySet = Memory_Alloc_Array( IndexSet*, self->rnCount, "boundarySet" );
	self->boundaryArray = Memory_Alloc_Array( Index*, self->rnCount,  "boundaryArray" );
	self->boundaryCount = Memory_Alloc_Array( IndexSet_Index, self->rnCount,  "boundaryCount" );
	self->boundaryForce = Memory_Alloc_Array( Force*, self->rnCount, "boundaryForce" );
	self->boundaryForceRemote = Memory_Alloc_Array( Force*, self->rnCount, "boundaryForceRemote" );
	self->boundaryIMass = Memory_Alloc_Array( Mass*, self->rnCount, "boundaryIMass" );
	self->boundaryIMassRemote = Memory_Alloc_Array( Mass*, self->rnCount, "boundaryIMassRemote" );
	Journal_Printf( 
		self->debug, 
		"Decomposition rank counts: %u %u %u\n", 
		self->decompRankCount[0],  
		self->decompRankCount[1],
		self->decompRankCount[2] );
	Journal_Printf( self->debug, "Neighbour count: %u\n", self->rnCount );
	
	_HexaMD_Shadow_BuildProcs( decomp, self->rn );
	#ifdef DEBUG 
	{
		Partition_Index rn_I;
		
		Journal_Printf( self->debug, "Neighbours: { " );
		for( rn_I = 0; rn_I < self->rnCount; rn_I++ ) {
			Journal_Printf( self->debug, "%u ", self->rn[rn_I] );
		}
		Journal_Printf( self->debug, "}\n" );
	}
	#endif
	Stream_Flush( self->debug );
	MPI_Barrier( self->communicator );
	
	/* Work out the set of nodes shared by each neighbour */
	IJK_1DTo3D( topology, self->rank, ijk );
	memset( tmpSet, 0, sizeof(BorderIndexSets) );
	for( rn_I = 0; rn_I < self->rnCount; rn_I++ ) {
		IJK		rnIjk;
		IJK		d; /* difference => position in CreateIndexSet table */
		Index		i;
		
		IJK_1DTo3D( topology, self->rn[rn_I], rnIjk );
		for( i = 0; i < 3; i++ ) {
			d[i] = rnIjk[i] - ijk[i] + 1;
		}
		
		self->boundarySet[rn_I] = CreateIndexSet[d[0]][d[1]][d[2]]( mesh, tmpSet );
		IndexSet_GetMembers( self->boundarySet[rn_I], &self->boundaryCount[rn_I], &self->boundaryArray[rn_I] );
		self->boundaryForce[rn_I] = Memory_Alloc_Array( Force, self->boundaryCount[rn_I], "boundaryForce" );
		self->boundaryForceRemote[rn_I] = Memory_Alloc_Array( Force, self->boundaryCount[rn_I], "boundaryForceRemote" );
		self->boundaryIMass[rn_I] = Memory_Alloc_Array( Mass, self->boundaryCount[rn_I], "boundaryIMassRemote" );
		self->boundaryIMassRemote[rn_I] = Memory_Alloc_Array( Mass, self->boundaryCount[rn_I], "boundaryIMassRemote" );
	}
	#ifdef DEBUG 
	{
		Partition_Index rn_I;
		
		Journal_Printf( self->debug, "Number of nodes I share with neighbour: { " );
		for( rn_I = 0; rn_I < self->rnCount; rn_I++ ) {
			Journal_Printf( self->debug, "%u ", self->boundaryCount[rn_I] );
		}
		Journal_Printf( self->debug, "}\n" );
	}
	#endif
}

static IndexSet* CreateLBA( Mesh* mesh, BorderIndexSets tmpSet ) {
	if( !tmpSet[LBA] ) {
		if( tmpSet[LB] ) {
			tmpSet[LBA] = IndexSet_Duplicate( CreateLB( mesh, tmpSet ) );
			IndexSet_Merge_AND( tmpSet[LBA],  CreateA( mesh, tmpSet ) );
		}
		else { /* else if tmpSet[BA] or neither */
			tmpSet[LBA] = IndexSet_Duplicate( CreateBA( mesh, tmpSet ) );
			IndexSet_Merge_AND( tmpSet[LBA],  CreateL( mesh, tmpSet ) );
		}
	}
	return tmpSet[LBA];
}

static IndexSet* CreateLB( Mesh* mesh, BorderIndexSets tmpSet ) {
	if( !tmpSet[LB] ) {
		tmpSet[LB] = IndexSet_Duplicate( CreateL( mesh, tmpSet ) );
		IndexSet_Merge_AND( tmpSet[LB],  CreateB( mesh, tmpSet ) );
	}
	return tmpSet[LB];
}

static IndexSet* CreateLBC( Mesh* mesh, BorderIndexSets tmpSet ) {
	if( !tmpSet[LBC] ) {
		if( tmpSet[LB] ) {
			tmpSet[LBC] = IndexSet_Duplicate( CreateLB( mesh, tmpSet ) );
			IndexSet_Merge_AND( tmpSet[LBC],  CreateC( mesh, tmpSet ) );
		}
		else { /* else if tmpSet[BC] or neither */
			tmpSet[LBC] = IndexSet_Duplicate( CreateBC( mesh, tmpSet ) );
			IndexSet_Merge_AND( tmpSet[LBC],  CreateL( mesh, tmpSet ) );
		}
	}
	return tmpSet[LBC];
}

static IndexSet* CreateLA( Mesh* mesh, BorderIndexSets tmpSet ) {
	if( !tmpSet[LA] ) {
		tmpSet[LA] = IndexSet_Duplicate( CreateL( mesh, tmpSet ) );
		IndexSet_Merge_AND( tmpSet[LA],  CreateA( mesh, tmpSet ) );
	}
	return tmpSet[LA];
}

static IndexSet* CreateL( Mesh* mesh, BorderIndexSets tmpSet ) {
	if( !tmpSet[L] ) {
		tmpSet[L] = RegularMeshUtils_CreateLocalLeftSet( mesh );
	}
	return tmpSet[L];
}

static IndexSet* CreateLC( Mesh* mesh, BorderIndexSets tmpSet ) {
	if( !tmpSet[LC] ) {
		tmpSet[LC] = IndexSet_Duplicate( CreateL( mesh, tmpSet ) );
		IndexSet_Merge_AND( tmpSet[LC],  CreateC( mesh, tmpSet ) );
	}
	return tmpSet[LC];
}

static IndexSet* CreateLTA( Mesh* mesh, BorderIndexSets tmpSet ) {
	if( !tmpSet[LTA] ) {
		if( tmpSet[LT] ) {
			tmpSet[LTA] = IndexSet_Duplicate( CreateLT( mesh, tmpSet ) );
			IndexSet_Merge_AND( tmpSet[LTA],  CreateA( mesh, tmpSet ) );
		}
		else { /* else if tmpSet[TA] or neither */
			tmpSet[LTA] = IndexSet_Duplicate( CreateTA( mesh, tmpSet ) );
			IndexSet_Merge_AND( tmpSet[LTA],  CreateL( mesh, tmpSet ) );
		}
	}
	return tmpSet[LTA];
}

static IndexSet* CreateLT( Mesh* mesh, BorderIndexSets tmpSet ) {
	if( !tmpSet[LT] ) {
		tmpSet[LT] = IndexSet_Duplicate( CreateL( mesh, tmpSet ) );
		IndexSet_Merge_AND( tmpSet[LT],  CreateT( mesh, tmpSet ) );
	}
	return tmpSet[LT];
}

static IndexSet* CreateLTC( Mesh* mesh, BorderIndexSets tmpSet ) {
	if( !tmpSet[LTC] ) {
		if( tmpSet[LT] ) {
			tmpSet[LTC] = IndexSet_Duplicate( CreateLT( mesh, tmpSet ) );
			IndexSet_Merge_AND( tmpSet[LTC],  CreateC( mesh, tmpSet ) );
		}
		else { /* else if tmpSet[TC] or neither */
			tmpSet[LTC] = IndexSet_Duplicate( CreateTC( mesh, tmpSet ) );
			IndexSet_Merge_AND( tmpSet[LTC],  CreateL( mesh, tmpSet ) );
		}
	}
	return tmpSet[LTC];
}

static IndexSet* CreateBA( Mesh* mesh, BorderIndexSets tmpSet ) {
	if( !tmpSet[BA] ) {
		tmpSet[BA] = IndexSet_Duplicate( CreateB( mesh, tmpSet ) );
		IndexSet_Merge_AND( tmpSet[BA],  CreateA( mesh, tmpSet ) );
	}
	return tmpSet[BA];
}

static IndexSet* CreateB( Mesh* mesh, BorderIndexSets tmpSet ) {
	if( !tmpSet[B] ) {
		tmpSet[B] = RegularMeshUtils_CreateLocalBottomSet( mesh );
	}
	return tmpSet[B];
}

static IndexSet* CreateBC( Mesh* mesh, BorderIndexSets tmpSet ) {
	if( !tmpSet[BC] ) {
		tmpSet[BC] = IndexSet_Duplicate( CreateB( mesh, tmpSet ) );
		IndexSet_Merge_AND( tmpSet[BC],  CreateC( mesh, tmpSet ) );
	}
	return tmpSet[BC];
}

static IndexSet* CreateA( Mesh* mesh, BorderIndexSets tmpSet ) {
	if( !tmpSet[A] ) {
		tmpSet[A] = RegularMeshUtils_CreateLocalBackSet( mesh );
	}
	return tmpSet[A];
}

static IndexSet* CreateC( Mesh* mesh, BorderIndexSets tmpSet ) {
	if( !tmpSet[C] ) {
		tmpSet[C] = RegularMeshUtils_CreateLocalFrontSet( mesh );
	}
	return tmpSet[C];
}

static IndexSet* CreateTA( Mesh* mesh, BorderIndexSets tmpSet ) {
	if( !tmpSet[TA] ) {
		tmpSet[TA] = IndexSet_Duplicate( CreateT( mesh, tmpSet ) );
		IndexSet_Merge_AND( tmpSet[TA],  CreateA( mesh, tmpSet ) );
	}
	return tmpSet[TA];
}

static IndexSet* CreateT( Mesh* mesh, BorderIndexSets tmpSet ) {
	if( !tmpSet[T] ) {
		tmpSet[T] = RegularMeshUtils_CreateLocalTopSet( mesh );
	}
	return tmpSet[T];
}

static IndexSet* CreateTC( Mesh* mesh, BorderIndexSets tmpSet ) {
	if( !tmpSet[TC] ) {
		tmpSet[TC] = IndexSet_Duplicate( CreateT( mesh, tmpSet ) );
		IndexSet_Merge_AND( tmpSet[TC],  CreateC( mesh, tmpSet ) );
	}
	return tmpSet[TC];
}

static IndexSet* CreateRBA( Mesh* mesh, BorderIndexSets tmpSet ) {
	if( !tmpSet[RBA] ) {
		if( tmpSet[RB] ) {
			tmpSet[RBA] = IndexSet_Duplicate( CreateRB( mesh, tmpSet ) );
			IndexSet_Merge_AND( tmpSet[RBA],  CreateA( mesh, tmpSet ) );
		}
		else { /* else if tmpSet[BA] or neither */
			tmpSet[RBA] = IndexSet_Duplicate( CreateBA( mesh, tmpSet ) );
			IndexSet_Merge_AND( tmpSet[RBA],  CreateR( mesh, tmpSet ) );
		}
	}
	return tmpSet[RBA];
}

static IndexSet* CreateRB( Mesh* mesh, BorderIndexSets tmpSet ) {
	if( !tmpSet[RB] ) {
		tmpSet[RB] = IndexSet_Duplicate( CreateR( mesh, tmpSet ) );
		IndexSet_Merge_AND( tmpSet[RB],  CreateB( mesh, tmpSet ) );
	}
	return tmpSet[RB];
}

static IndexSet* CreateRBC( Mesh* mesh, BorderIndexSets tmpSet ) {
	if( !tmpSet[RBC] ) {
		if( tmpSet[RB] ) {
			tmpSet[RBC] = IndexSet_Duplicate( CreateRB( mesh, tmpSet ) );
			IndexSet_Merge_AND( tmpSet[RBC],  CreateC( mesh, tmpSet ) );
		}
		else { /* else if tmpSet[BC] or neither */
			tmpSet[RBC] = IndexSet_Duplicate( CreateBC( mesh, tmpSet ) );
			IndexSet_Merge_AND( tmpSet[RBC],  CreateR( mesh, tmpSet ) );
		}
	}
	return tmpSet[RBC];
}

static IndexSet* CreateRA( Mesh* mesh, BorderIndexSets tmpSet ) {
	if( !tmpSet[RA] ) {
		tmpSet[RA] = IndexSet_Duplicate( CreateR( mesh, tmpSet ) );
		IndexSet_Merge_AND( tmpSet[RA],  CreateA( mesh, tmpSet ) );
	}
	return tmpSet[RA];
}

static IndexSet* CreateR( Mesh* mesh, BorderIndexSets tmpSet ) {
	if( !tmpSet[R] ) {
		tmpSet[R] = RegularMeshUtils_CreateLocalRightSet( mesh );
	}
	return tmpSet[R];
}

static IndexSet* CreateRC( Mesh* mesh, BorderIndexSets tmpSet ) {
	if( !tmpSet[RC] ) {
		tmpSet[RC] = IndexSet_Duplicate( CreateR( mesh, tmpSet ) );
		IndexSet_Merge_AND( tmpSet[RC],  CreateC( mesh, tmpSet ) );
	}
	return tmpSet[RC];
}

static IndexSet* CreateRTA( Mesh* mesh, BorderIndexSets tmpSet ) {
	if( !tmpSet[RTA] ) {
		if( tmpSet[RT] ) {
			tmpSet[RTA] = IndexSet_Duplicate( CreateRT( mesh, tmpSet ) );
			IndexSet_Merge_AND( tmpSet[RTA],  CreateA( mesh, tmpSet ) );
		}
		else { /* else if tmpSet[TA] or neither */
			tmpSet[RTA] = IndexSet_Duplicate( CreateTA( mesh, tmpSet ) );
			IndexSet_Merge_AND( tmpSet[RTA],  CreateR( mesh, tmpSet ) );
		}
	}
	return tmpSet[RTA];
}

static IndexSet* CreateRT( Mesh* mesh, BorderIndexSets tmpSet ) {
	if( !tmpSet[RT] ) {
		tmpSet[RT] = IndexSet_Duplicate( CreateR( mesh, tmpSet ) );
		IndexSet_Merge_AND( tmpSet[RT],  CreateT( mesh, tmpSet ) );
	}
	return tmpSet[RT];
}

static IndexSet* CreateRTC( Mesh* mesh, BorderIndexSets tmpSet ) {
	if( !tmpSet[RTC] ) {
		if( tmpSet[RT] ) {
			tmpSet[RTC] = IndexSet_Duplicate( CreateRT( mesh, tmpSet ) );
			IndexSet_Merge_AND( tmpSet[RTC],  CreateC( mesh, tmpSet ) );
		}
		else { /* else if tmpSet[TC] or neither */
			tmpSet[RTC] = IndexSet_Duplicate( CreateTC( mesh, tmpSet ) );
			IndexSet_Merge_AND( tmpSet[RTC],  CreateR( mesh, tmpSet ) );
		}
	}
	return tmpSet[RTC];
}

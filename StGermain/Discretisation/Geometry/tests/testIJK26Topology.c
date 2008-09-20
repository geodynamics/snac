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
** $Id: testIJK26Topology.c 3583 2006-05-16 10:06:22Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"

#include <stdio.h>

void Test_TestTopologyOfSize( Dictionary* dictionary, IJK ijkSize, Bool periodic[3], Stream* stream ) {	
	IJK26Topology*	topology;
	Index		i,j,k;
	Index elNum;
	Index           sizeI, sizeJ, sizeK;

	sizeI = ijkSize[I_AXIS];
	sizeJ = ijkSize[J_AXIS];
	sizeK = ijkSize[K_AXIS];

	topology = IJK26Topology_New_All( "Topology", NULL, ijkSize, periodic );
	Journal_Printf( stream, "topology:{ " );
	for ( i=0; i < 3; i++ ) {
		Journal_Printf( stream, "%d, ", topology->size[i] );
	}
	Journal_Printf( stream, "}" );

	Journal_Printf( stream, " - isPeriodic = { " );
	for ( i=0; i < 3; i++ ) {
		Journal_Printf( stream, "%s, ", StG_BoolToStringMap[topology->isPeriodic[i]] );
	}
	Journal_Printf( stream, "}\n" );

	Stream_Indent( stream );

	for ( k=0; k < sizeK; k++ ) {
		for ( j=0; j < sizeJ; j++ ) {
			for ( i=0; i < sizeI; i++ ) {
				IJK_3DTo1D_3( topology, i, j, k, &elNum );
				Journal_Printf( stream, "Item at (%d,%d,%d)->%d:\n", 
					i, j, k, elNum );
				Stream_Indent( stream );
				Journal_Printf( stream, "Dyn: " );
				topology->dynamicSizes = True;
				IJK26Topology_PrintNeighboursOfIndex( topology, elNum, stream );
				Journal_Printf( stream, "Full:\n" );
				Stream_Indent( stream );
				topology->dynamicSizes = False;
				IJK26Topology_PrintNeighboursOfIndex( topology, elNum, stream );
				Stream_UnIndent( stream );
				Stream_UnIndent( stream );
			}
		}	
	}
	Journal_Printf( stream, "\n" );

	Stream_UnIndent( stream );

	Stg_Class_Delete( topology );
}	


int main( int argc, char* argv[] ) {
	Dictionary*	dictionary;
	MPI_Comm	CommWorld;
	int		rank;
	int		numProcessors;
	Bool            periodic[3] = { False, False, False };
	unsigned int    ii, jj, kk;
	Stream*         stream;
	IJK             ijkSize;
	
	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size( CommWorld, &numProcessors );
	MPI_Comm_rank( CommWorld, &rank );
	
	Base_Init( &argc, &argv );
	
	DiscretisationGeometry_Init( &argc, &argv );
	MPI_Barrier( CommWorld ); /* Ensures copyright info always come first in output */
	
	stream = Journal_Register( Info_Type, argv[0] );
	Stream_Enable( stream, True );

	dictionary = Dictionary_New();
	Dictionary_Add( dictionary, "meshSizeI", Dictionary_Entry_Value_FromUnsignedInt( 1 ) );
	Dictionary_Add( dictionary, "meshSizeJ", Dictionary_Entry_Value_FromUnsignedInt( 1 ) );
	Dictionary_Add( dictionary, "meshSizeK", Dictionary_Entry_Value_FromUnsignedInt( 1 ) );

	Journal_Printf( stream, "+++++++++++++ 1D Tests +++++++++++++\n\n" );
	ijkSize[I_AXIS] = 1;
	ijkSize[J_AXIS] = 1;
	ijkSize[K_AXIS] = 1;
	periodic[I_AXIS] = False;
	periodic[J_AXIS] = False;
	periodic[K_AXIS] = False;
	for ( ijkSize[I_AXIS] = 1; ijkSize[I_AXIS] <= 3; ijkSize[I_AXIS]++ ) {
		for ( ii=False; ii < 2; ii++ ) {
			periodic[I_AXIS] = ii;
			Test_TestTopologyOfSize( dictionary, ijkSize, periodic, stream );
		}	
	}
	ijkSize[I_AXIS] = 1;
	periodic[I_AXIS] = True;	/* Leave this on just to make sure it has no effect on J axis */
	for ( ijkSize[J_AXIS] = 2; ijkSize[J_AXIS] <= 3; ijkSize[J_AXIS]++ ) {
		for ( ii=False; ii < 2; ii++ ) {
			periodic[J_AXIS] = ii;
			Test_TestTopologyOfSize( dictionary, ijkSize, periodic, stream );
		}	
	}

	Journal_Printf( stream, "+++++++++++++ 2D Tests +++++++++++++\n\n" );
	ijkSize[I_AXIS] = 1;
	ijkSize[J_AXIS] = 1;
	ijkSize[K_AXIS] = 1;
	periodic[I_AXIS] = False;
	periodic[J_AXIS] = False;
	periodic[K_AXIS] = False;
	for ( ijkSize[J_AXIS] = 2; ijkSize[J_AXIS] <= 3; ijkSize[J_AXIS]++ ) {
		for ( ijkSize[I_AXIS] = 2; ijkSize[I_AXIS] <= 3; ijkSize[I_AXIS]++ ) {
			for ( jj=False; jj < 2; jj++ ) {
				periodic[J_AXIS] = jj;
				for ( ii=False; ii < 2; ii++ ) {
					periodic[I_AXIS] = ii;
					Test_TestTopologyOfSize( dictionary, ijkSize, periodic, stream );
				}
			}
		}
	}

	Journal_Printf( stream, "+++++++++++++ 3D Tests +++++++++++++\n\n" );
	ijkSize[I_AXIS] = 3;
	ijkSize[J_AXIS] = 3;
	ijkSize[K_AXIS] = 3;
	periodic[I_AXIS] = False;
	periodic[J_AXIS] = False;
	periodic[K_AXIS] = False;
	for ( kk=False; kk < 2; kk++ ) {
		periodic[K_AXIS] = kk;

		for ( jj=False; jj < 2; jj++ ) {
			periodic[J_AXIS] = jj;

			for ( ii=False; ii < 2; ii++ ) {
				periodic[I_AXIS] = ii;
				Test_TestTopologyOfSize( dictionary, ijkSize, periodic, stream );
			}
		}
	}	

	Stg_Class_Delete( dictionary );
		
	DiscretisationGeometry_Finalise();
	
	Base_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0;
}

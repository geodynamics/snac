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
** Role:
**	Tests StrainRate function for values of element volume.
**
** $Id: testStress-tetra-strain-4-4-4.c 3243 2006-10-12 09:04:00Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "TestStress.h"

#include "stdio.h"
#include "stdlib.h"

#define TOL 1.0e-16

struct _Node {
	struct { __Snac_Node };
};

struct _Element {
	struct { __Snac_Element };
};

int main( int argc, char* argv[] ) {
	MPI_Comm			CommWorld;
	int				rank;
	int				numProcessors;
	int				procToWatch;
	Dictionary*			dictionary;
	Snac_Context*			snacContext;
	Element_LocalIndex		elementIndex;
	double				minLengthScale;
	double				speedOfSound;
#if 0
	Node_LocalIndex		nodeIndex;
	Mass				mass;
	Mass				inertialMass;
	Force				force;
	Force				balance;
#endif
	Stream*				stream;
	Index elementNode_lI;

	Dictionary_Entry_Value* velocityBCs;
	Dictionary_Entry_Value* vcList;
	Dictionary_Entry_Value* varStruct;
	Dictionary_Entry_Value* eachVarList;
	Dictionary_Entry_Value* vxStruct;

	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	Snac_Init( &argc, &argv );
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size( CommWorld, &numProcessors );
	MPI_Comm_rank( CommWorld, &rank );

	stream = Journal_Register (InfoStream_Type, "myStream");

	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}
	if( rank == procToWatch ) printf( "Watching rank: %i\n", rank );

	/* Read input */
	dictionary = Dictionary_New();
	dictionary->add( dictionary, "rank", Dictionary_Entry_Value_FromUnsignedInt( rank ) );
	dictionary->add( dictionary, "numProcessors", Dictionary_Entry_Value_FromUnsignedInt( numProcessors ) );
	dictionary->add( dictionary, "meshSizeI", Dictionary_Entry_Value_FromUnsignedInt( 3 ) );
	dictionary->add( dictionary, "meshSizeJ", Dictionary_Entry_Value_FromUnsignedInt( 3 ) );
	dictionary->add( dictionary, "meshSizeK", Dictionary_Entry_Value_FromUnsignedInt( 3 ) );
	dictionary->add( dictionary, "minX", Dictionary_Entry_Value_FromDouble( 0.0f ) );
	dictionary->add( dictionary, "minY", Dictionary_Entry_Value_FromDouble( -1.0f ) );
	dictionary->add( dictionary, "minZ", Dictionary_Entry_Value_FromDouble( 0.0f ) );
	dictionary->add( dictionary, "maxX", Dictionary_Entry_Value_FromDouble( 1.0f ) );
	dictionary->add( dictionary, "maxY", Dictionary_Entry_Value_FromDouble( 0.0f ) );
	dictionary->add( dictionary, "maxZ", Dictionary_Entry_Value_FromDouble( 1.0f ) );
	dictionary->add( dictionary, "dtType", Dictionary_Entry_Value_FromString( "constant" ) );
	dictionary->add( dictionary, "timeStep", Dictionary_Entry_Value_FromDouble( 1.0 ) );
	dictionary->add( dictionary, "topo_kappa", Dictionary_Entry_Value_FromDouble( 0 ) );
	dictionary->add( dictionary, "alpha", Dictionary_Entry_Value_FromDouble( 0 ) );
	dictionary->add( dictionary, "forceCalcType", Dictionary_Entry_Value_FromString( "complete" ) );

	velocityBCs = Dictionary_Entry_Value_NewStruct();
	vcList = Dictionary_Entry_Value_NewList();
	/* right wall */
	varStruct = Dictionary_Entry_Value_NewStruct();
	eachVarList = Dictionary_Entry_Value_NewList();
	vxStruct = Dictionary_Entry_Value_NewStruct();
	Dictionary_Entry_Value_AddMember( vxStruct, "name", Dictionary_Entry_Value_FromString( "vx" ) );
	Dictionary_Entry_Value_AddMember( vxStruct, "type", Dictionary_Entry_Value_FromString( "double" ) );
	Dictionary_Entry_Value_AddMember( vxStruct, "value", Dictionary_Entry_Value_FromDouble( 0.01 ) );
	Dictionary_Entry_Value_AddElement( eachVarList, vxStruct );
	Dictionary_Entry_Value_AddMember( varStruct, "type", Dictionary_Entry_Value_FromString( "WallVC" ) );
	Dictionary_Entry_Value_AddMember( varStruct, "wall", Dictionary_Entry_Value_FromString( "right" ) );
	Dictionary_Entry_Value_AddMember( varStruct, "variables", eachVarList );
	Dictionary_Entry_Value_AddElement( vcList, varStruct );
	/* left wall */
	varStruct = Dictionary_Entry_Value_NewStruct();
	eachVarList = Dictionary_Entry_Value_NewList();
	vxStruct = Dictionary_Entry_Value_NewStruct();
	Dictionary_Entry_Value_AddMember( vxStruct, "name", Dictionary_Entry_Value_FromString( "vx" ) );
	Dictionary_Entry_Value_AddMember( vxStruct, "type", Dictionary_Entry_Value_FromString( "double" ) );
	Dictionary_Entry_Value_AddMember( vxStruct, "value", Dictionary_Entry_Value_FromDouble( 0.0 ) );
	Dictionary_Entry_Value_AddElement( eachVarList, vxStruct );
	Dictionary_Entry_Value_AddMember( varStruct, "type", Dictionary_Entry_Value_FromString( "WallVC" ) );
	Dictionary_Entry_Value_AddMember( varStruct, "wall", Dictionary_Entry_Value_FromString( "left" ) );
	Dictionary_Entry_Value_AddMember( varStruct, "variables", eachVarList );
	Dictionary_Entry_Value_AddElement( vcList, varStruct );
	/* add to dictionary */
	Dictionary_Entry_Value_AddMember( velocityBCs, "vcList", vcList );
	Dictionary_Add( dictionary, "velocityBCs", velocityBCs );

	/* Build the context */
	snacContext = Snac_Context_New( 0.0f, 10.0f, sizeof(Snac_Node), sizeof(Snac_Element), CommWorld, dictionary );

	/* Construction phase -----------------------------------------------------------------------------------------------*/
	Stg_Component_Construct( snacContext, 0 /* dummy */, &snacContext, True );
	
	/* Building phase ---------------------------------------------------------------------------------------------------*/
	Stg_Component_Build( snacContext, 0 /* dummy */, False );
	
	/* Initialisaton phase ----------------------------------------------------------------------------------------------*/
	Stg_Component_Initialise( snacContext, 0 /* dummy */, False );

	/* Check the coordinates of the nodes. */
	for( elementIndex = 0; elementIndex < snacContext->mesh->elementLocalCount; elementIndex ++ ) {
		printf( "Element: %u, Coords: ", elementIndex );
		for( elementNode_lI = 0; elementNode_lI < 8; elementNode_lI++ ) {
			printf( "(%g %g %g) ",
					Snac_Element_NodeCoord( snacContext, elementIndex, elementNode_lI )[0],
					Snac_Element_NodeCoord( snacContext, elementIndex, elementNode_lI )[1],
					Snac_Element_NodeCoord( snacContext, elementIndex, elementNode_lI )[2] );
		}
		printf( "\n" );
	}
#if 0
	/* Generate a simple velocity field: "right" face is pulled at 0.01 m/sec while "left" face is fixed. */
	for( elementIndex = 0; elementIndex < snacContext->mesh->elementLocalCount; elementIndex ++ ) {
		for( elementNode_lI = 0; elementNode_lI < 8; elementNode_lI++ )
			for( dimI = 0; dimI < 3; dimI++ ) {
				if( Snac_Element_NodeCoord( snacContext, 0, elementNode_lI )[0] == 0.0 )
					Snac_Element_Node_P( snacContext, 0, elementNode_lI )->velocity[dimI] = 0.0;
				else
					Snac_Element_Node_P( snacContext, 0, elementNode_lI )->velocity[dimI] = ((dimI==0)?0.01:0.0);
		}
	}
#endif
	/* Check the velocity field. */
	for( elementIndex = 0; elementIndex < snacContext->mesh->elementLocalCount; elementIndex ++ ) {
		printf( "Element: %u, Vel: ",elementIndex );
		for( elementNode_lI = 0; elementNode_lI < 8; elementNode_lI++ ) {
			printf( "(%g %g %g) ",
					Snac_Element_Node_P( snacContext, elementIndex, elementNode_lI )->velocity[0],
					Snac_Element_Node_P( snacContext, elementIndex, elementNode_lI )->velocity[1],
					Snac_Element_Node_P( snacContext, elementIndex, elementNode_lI )->velocity[2] );
		}
		printf( "\n" );
	}

	/* Update all the elements, and in the process work out this processor's minLengthScale */
	elementIndex = 0;
	Snac_UpdateElementMomentum( (Context*)snacContext, elementIndex, &minLengthScale );
	for( elementIndex = 1; elementIndex < snacContext->mesh->elementLocalCount; elementIndex++ ) {
		double elementMinLengthScale;

		Snac_UpdateElementMomentum( (Context*)snacContext, elementIndex, &elementMinLengthScale );

		if( elementMinLengthScale < minLengthScale ) {
			minLengthScale = elementMinLengthScale;
		}
	}

	/* Find the global minLengthScale, and then calculate the new dt and speed of sound, assuming frac=0.5,
	 * strain_inert=1.0e-5f & vbc_max=3.0e-10f */
	{
		double localMinLengthScale = minLengthScale;

		MPI_Allreduce( &localMinLengthScale, &minLengthScale, 1, MPI_DOUBLE, MPI_MIN, CommWorld );

		/* snacContext->dt = minLengthScale * 0.5f * 1.0e-5f / 3.0e-10f; */
		speedOfSound = minLengthScale * 0.5f / snacContext->dt;
	}
	printf( "Dt: %g\n", snacContext->dt );

	/* For each element, calculate strain-rate and then stress */
	for( elementIndex = 0; elementIndex < snacContext->mesh->elementLocalCount; elementIndex++ ) {
		Snac_StrainRate( (Context*)snacContext, elementIndex );
		SnacTest_Stress( (Context*)snacContext, elementIndex );
	}

	/* Test 1: strain[0][0] should be equal to 0.02 within the tolerance range. */
	{
		Tetrahedra_Index	tetra_I;
		for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
			if( fabs( Snac_Element_At( snacContext, 1 )->tetra[tetra_I].strain[0][0] - 0.02 ) > TOL ) {
				printf( "Element: 1, strain test 1 for tetrahedron %u faild: |%g - 0.02| > %g\n",
						tetra_I, Snac_Element_At( snacContext, 1 )->tetra[tetra_I].strain[0][0], TOL );
			}
		}
	}
	/* Test 2: The RMS of strain elements (except strain[0][0])
	   should be equal to 0 within the tolerance range. */
	{
		Tetrahedra_Index	tetra_I;
		for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
			if( sqrt( pow( Snac_Element_At( snacContext, 1 )->tetra[tetra_I].strain[1][1], 2.0 ) +
					  pow( Snac_Element_At( snacContext, 1 )->tetra[tetra_I].strain[2][2], 2.0 ) +
					  pow( Snac_Element_At( snacContext, 1 )->tetra[tetra_I].strain[0][1], 2.0 ) +
					  pow( Snac_Element_At( snacContext, 1 )->tetra[tetra_I].strain[0][2], 2.0 ) +
					  pow( Snac_Element_At( snacContext, 1 )->tetra[tetra_I].strain[1][2], 2.0 ) ) > TOL ) {
				printf( "Element: 1, strain test 2 for tetrahedron %u faild: RMS %g > %g\n",
						tetra_I,
						sqrt( pow( Snac_Element_At( snacContext, 1 )->tetra[tetra_I].strain[1][1], 2.0 ) +
							  pow( Snac_Element_At( snacContext, 1 )->tetra[tetra_I].strain[2][2], 2.0 ) +
							  pow( Snac_Element_At( snacContext, 1 )->tetra[tetra_I].strain[0][1], 2.0 ) +
							  pow( Snac_Element_At( snacContext, 1 )->tetra[tetra_I].strain[0][2], 2.0 ) +
							  pow( Snac_Element_At( snacContext, 1 )->tetra[tetra_I].strain[1][2], 2.0 ) ),
						TOL );
			}
		}

	}
#if 0
	/* For each node, calculate the mass, force, veloity and coordinates */
	for( nodeIndex = 0; nodeIndex < snacContext->mesh->nodeLocalCount; nodeIndex++ ) {
		Snac_Force( (Context*)snacContext, nodeIndex, speedOfSound, &mass, &inertialMass, &force, &balance );
		Snac_UpdateNodeMomentum( (Context*)snacContext, nodeIndex, inertialMass, force );
	}

	/* Check the updated coordinates of the nodes. */
	printf( "Element: 0, Updated Coords: " );
	for( elementNode_lI = 0; elementNode_lI < 8; elementNode_lI++ ) {
		printf( "(%g %g %g) ",
				Snac_Element_NodeCoord( snacContext, 0, elementNode_lI )[0],
				Snac_Element_NodeCoord( snacContext, 0, elementNode_lI )[1],
				Snac_Element_NodeCoord( snacContext, 0, elementNode_lI )[2] );
	}
	printf( "\n" );
#endif

	/* Stg_Class_Delete stuff */
	Stg_Class_Delete( snacContext );
	Stg_Class_Delete( dictionary );

	/* Close off MPI */
	MPI_Finalize();

	return 0; /* success */
}

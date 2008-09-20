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
** $Id: testStrainRate-strainRate-4-4-4.c 3243 2006-10-12 09:04:00Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"

#include "stdio.h"
#include "stdlib.h"

#define TOL 1.0e-09

struct _Node {
	struct { __Snac_Node };
};

struct _Element {
	struct { __Snac_Element };
};

int main( int argc, char* argv[] ) {
	MPI_Comm		CommWorld;
	int			rank;
	int			numProcessors;
	int			procToWatch;
	Dictionary*		dictionary;
	Snac_Context*		snacContext;
	Element_Index		elementIndex;
	double			minLengthScale;
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
	elementIndex = 1;
	printf( "Element: %u, Coords: ", elementIndex );
	for( elementNode_lI = 0; elementNode_lI < 8; elementNode_lI++ ) {
		printf( "(%g %g %g) ",
				Snac_Element_NodeCoord( snacContext, elementIndex, elementNode_lI )[0],
				Snac_Element_NodeCoord( snacContext, elementIndex, elementNode_lI )[1],
				Snac_Element_NodeCoord( snacContext, elementIndex, elementNode_lI )[2] );
	}
	printf( "\n" );
#if 0
	/* Generate a simple velocity field: "right" face is pulled at 0.01 m/sec while "left" face is fixed. */
	for( elementNode_lI = 0; elementNode_lI < 8; elementNode_lI++ )
		for( dimI = 0; dimI < 3; dimI++ ) {
			if( Snac_Element_NodeCoord( snacContext, 0, elementNode_lI )[0] == 0.0 )
				Snac_Element_Node_P( snacContext, 0, elementNode_lI )->velocity[dimI] = 0.0;
			else
				Snac_Element_Node_P( snacContext, 0, elementNode_lI )->velocity[dimI] = ((dimI==0)?0.01:0.0);
		}
#endif
	/* Check the velocity field. */
	elementIndex = 1;
	printf( "Element: %u, Vel: ",elementIndex );
	for( elementNode_lI = 0; elementNode_lI < 8; elementNode_lI++ ) {
		printf( "(%g %g %g) ",
				Snac_Element_Node_P( snacContext, elementIndex, elementNode_lI )->velocity[0],
				Snac_Element_Node_P( snacContext, elementIndex, elementNode_lI )->velocity[1],
				Snac_Element_Node_P( snacContext, elementIndex, elementNode_lI )->velocity[2] );
	}
	printf( "\n" );

	/* Let SNAC compute strain rates. */
	elementIndex = 1;
	Snac_UpdateElementMomentum( (Context*)snacContext, elementIndex, &minLengthScale );
	Snac_StrainRate( (Context*)snacContext, elementIndex );

	/* Test1: The second invariant of strain tensor of element 1 should be equal to 0
	   within tolerance range. Can be less accurate than strain rates because
	   the computation of the second invariant is prone to large round-off error. */
	if( Snac_Element_At( snacContext, elementIndex )->strainRate > TOL )
		printf( "Element: %u, strainRate computation failed: %e > TOL (%e)\n", elementIndex, Snac_Element_At( snacContext, elementIndex )->strainRate, TOL );

	/* Stg_Class_Delete stuff */
	Stg_Class_Delete( snacContext );
	Stg_Class_Delete( dictionary );

	/* Close off MPI */
	MPI_Finalize();

	return 0; /* success */
}

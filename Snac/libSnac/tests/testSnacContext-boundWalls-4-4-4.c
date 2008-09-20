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
**	Tests StrainRate function for values of element strainRate.
**
** $Id: testSnacContext-boundWalls-4-4-4.c 2934 2005-05-12 06:14:18Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"

#include "stdio.h"
#include "stdlib.h"


/* Final definition of a node, element and particle */
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
	Dictionary_Entry_Value*		tmpVal0;
	Dictionary_Entry_Value*		tmpVal1;
	Snac_Context*			snacContext;

	
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
	Dictionary_Add( dictionary, "rank", Dictionary_Entry_Value_FromUnsignedInt( rank ) );
	Dictionary_Add( dictionary, "numProcessors", Dictionary_Entry_Value_FromUnsignedInt( numProcessors ) );
	Dictionary_Add( dictionary, "meshSizeI", Dictionary_Entry_Value_FromUnsignedInt( 4 ) );
	Dictionary_Add( dictionary, "meshSizeJ", Dictionary_Entry_Value_FromUnsignedInt( 4 ) );
	Dictionary_Add( dictionary, "meshSizeK", Dictionary_Entry_Value_FromUnsignedInt( 4 ) );
	Dictionary_Add( dictionary, "minX", Dictionary_Entry_Value_FromDouble( 0.0f ) );
	Dictionary_Add( dictionary, "minY", Dictionary_Entry_Value_FromDouble( -300.0f ) );
	Dictionary_Add( dictionary, "minZ", Dictionary_Entry_Value_FromDouble( 0.0f ) );
	Dictionary_Add( dictionary, "maxX", Dictionary_Entry_Value_FromDouble( 300.0f ) );
	Dictionary_Add( dictionary, "maxY", Dictionary_Entry_Value_FromDouble( 0.0f ) );
	Dictionary_Add( dictionary, "maxZ", Dictionary_Entry_Value_FromDouble( 300.0f ) );
	Dictionary_Add( dictionary, "Snac_InitialVelocityType", Dictionary_Entry_Value_FromString( "constant" ) );
	Dictionary_Add( dictionary, "Snac_InitialVelocityX", Dictionary_Entry_Value_FromDouble( 3.0e-10f ) );
	Dictionary_Add( dictionary, "Snac_InitialVelocityY", Dictionary_Entry_Value_FromDouble( 3.0e-10f ) );
	Dictionary_Add( dictionary, "Snac_InitialVelocityZ", Dictionary_Entry_Value_FromDouble( 6.0e-10f ) );

	/* Left wall dicitonary definition */
	tmpVal1 = Dictionary_Entry_Value_NewList();
	tmpVal0 = Dictionary_Entry_Value_NewStruct();
	Dictionary_Entry_Value_AddMember( tmpVal0, "name", Dictionary_Entry_Value_FromString( "vx" ) );
	Dictionary_Entry_Value_AddMember( tmpVal0, "type", Dictionary_Entry_Value_FromString( "double" ) );
	Dictionary_Entry_Value_AddMember( tmpVal0, "value", Dictionary_Entry_Value_FromDouble( 0.0f ) );
	Dictionary_Entry_Value_AddElement( tmpVal1, tmpVal0 );
	tmpVal0 = Dictionary_Entry_Value_NewStruct();
	Dictionary_Entry_Value_AddMember( tmpVal0, "wall", Dictionary_Entry_Value_FromString( "left" ) );
	Dictionary_Entry_Value_AddMember( tmpVal0, "variables", tmpVal1 );
	Dictionary_Add( dictionary, "leftwall", tmpVal0 );

	/* right wall dictionary definition */
	tmpVal1 = Dictionary_Entry_Value_NewList();
	tmpVal0 = Dictionary_Entry_Value_NewStruct();
	Dictionary_Entry_Value_AddMember( tmpVal0, "name", Dictionary_Entry_Value_FromString( "vy" ) );
	Dictionary_Entry_Value_AddMember( tmpVal0, "type", Dictionary_Entry_Value_FromString( "double" ) );
	Dictionary_Entry_Value_AddMember( tmpVal0, "value", Dictionary_Entry_Value_FromDouble( 0.0f ) );
	Dictionary_Entry_Value_AddElement( tmpVal1, tmpVal0 );
	tmpVal0 = Dictionary_Entry_Value_NewStruct();
	Dictionary_Entry_Value_AddMember( tmpVal0, "wall", Dictionary_Entry_Value_FromString( "right" ) );
	Dictionary_Entry_Value_AddMember( tmpVal0, "variables", tmpVal1 );
	Dictionary_Add( dictionary, "rightwall", tmpVal0 );

	/* back wall dictionary definition */
	tmpVal1 = Dictionary_Entry_Value_NewList();
	tmpVal0 = Dictionary_Entry_Value_NewStruct();
	Dictionary_Entry_Value_AddMember( tmpVal0, "name", Dictionary_Entry_Value_FromString( "vy" ) );
	Dictionary_Entry_Value_AddMember( tmpVal0, "type", Dictionary_Entry_Value_FromString( "double" ) );
	Dictionary_Entry_Value_AddMember( tmpVal0, "value", Dictionary_Entry_Value_FromDouble( 0.0f ) );
	Dictionary_Entry_Value_AddElement( tmpVal1, tmpVal0 );
	tmpVal0 = Dictionary_Entry_Value_NewStruct();
	Dictionary_Entry_Value_AddMember( tmpVal0, "wall", Dictionary_Entry_Value_FromString( "back" ) );
	Dictionary_Entry_Value_AddMember( tmpVal0, "variables", tmpVal1 );
	Dictionary_Add( dictionary, "backwall", tmpVal0 );

	/* front wall dictionary definition */
	tmpVal1 = Dictionary_Entry_Value_NewList();
	tmpVal0 = Dictionary_Entry_Value_NewStruct();
	Dictionary_Entry_Value_AddMember( tmpVal0, "name", Dictionary_Entry_Value_FromString( "vy" ) );
	Dictionary_Entry_Value_AddMember( tmpVal0, "type", Dictionary_Entry_Value_FromString( "double" ) );
	Dictionary_Entry_Value_AddMember( tmpVal0, "value", Dictionary_Entry_Value_FromDouble( 0.0f ) );
	Dictionary_Entry_Value_AddElement( tmpVal1, tmpVal0 );
	tmpVal0 = Dictionary_Entry_Value_NewStruct();
	Dictionary_Entry_Value_AddMember( tmpVal0, "wall", Dictionary_Entry_Value_FromString( "front" ) );
	Dictionary_Entry_Value_AddMember( tmpVal0, "variables", tmpVal1 );
	Dictionary_Add( dictionary, "frontwall", tmpVal0 );


	/* Build the context */
	snacContext = Snac_Context_New( 0.0f, 10.0f, sizeof(Snac_Node), sizeof(Snac_Element), CommWorld, dictionary );
	

	/* Initialise the context */
	AbstractContext_Setup( snacContext );

	/* Step the context solver */
	AbstractContext_Step( snacContext, snacContext->dt );
	
	if( rank == procToWatch ) {
		Element_LocalIndex 	element_lI;
		Variable_Index     dofSet_I, dofSet_count;
		VariableCondition_Index	vc_I, vcCount;
		Node_LocalIndex	node_lI, node_count;
		
		printf( "Dofs:\n" );
		dofSet_count = snacContext->variable_Register->dofCount;
		for( dofSet_I = 0; dofSet_I < dofSet_count; dofSet_I++ ) {
			Dof_Index		dof_I, dof_count;
			
			dof_count = snacContext->mesh->dof->countTbl[dofSet_I];
			for( dof_I = 0; dof_I < dof_count; dof_I++ ) {
				printf( "\tDof set: %u, Dof: { #: %u, name: \"%s\", offset: %lu }\n", dofSet_I, dof_I, snacContext->mesh->dof->infoTbl[dofSet_I][dof_I].name, snacContext->mesh->dof->infoTbl[dofSet_I][dof_I].offset );
			}
		}
		
		printf( "BoundaryConditions:\n" );
		printf( "\tLeft wall: \n" );
		vcCount = snacContext->bcLeftWall->valueCount;
		for( vc_I = 0; vc_I < vcCount; vc_I++ ) {
			printf( "\t\tboundary condition[%u]: %g\n", vc_I, snacContext->bcLeftWall->valueTbl[vc_I].asDouble );
		}
		printf( "\t\tnodes with boundary conditions (node#:dof#:bc#): " );
		node_count = snacContext->mesh->nodeLocalCount;
		for( node_lI = 0; node_lI < node_count; node_lI++ ) {
			Dof_Index		dof_I;
			DofLayout_Set_Index	dofSet_I;
			
			dofSet_I = DofLayout_SetOfNode( snacContext->mesh->dof, node_lI );
			for( dof_I = 0; dof_I < snacContext->mesh->dof->countTbl[dofSet_I]; dof_I++ ) {
				if( VariableCondition_GetIndex( snacContext->bcLeftWall, node_lI, dof_I ) != (unsigned)-1 ) {
					printf( "%u:%u:%u ", node_lI, dof_I, VariableCondition_GetIndex( snacContext->bcLeftWall, node_lI, dof_I ) );
				}
			}
		}
		printf( "\n" );
		printf( "\tRight wall: \n" );
		vcCount = snacContext->bcRightWall->count;
		for( vc_I = 0; vc_I < vcCount; vc_I++ ) {
			printf( "\t\tboundary condition[%u]: %g\n", vc_I, snacContext->bcRightWall->valueTbl[vc_I].asDouble );
		}
		printf( "\t\tnodes with boundary conditions (node#:dof#:bc#): " );
		node_count = snacContext->mesh->nodeLocalCount;
		for( node_lI = 0; node_lI < node_count; node_lI++ ) {
			Dof_Index		dof_I;
			DofLayout_Set_Index	dofSet_I;
			
			dofSet_I = DofLayout_SetOfNode( snacContext->mesh->dof, node_lI );
			for( dof_I = 0; dof_I < snacContext->mesh->dof->countTbl[dofSet_I]; dof_I++ ) {
				if( VariableCondition_GetIndex( snacContext->bcRightWall, node_lI, dof_I ) != (unsigned)-1 ) {
					printf( "%u:%u:%u ", node_lI, dof_I, VariableCondition_GetIndex( snacContext->bcRightWall, node_lI, dof_I ) );
				}
			}
		}
		printf( "\n" );
		printf( "\tBack wall: \n" );
		vcCount = snacContext->bcBackWall->count;
		for( vc_I = 0; vc_I < vcCount; vc_I++ ) {
			printf( "\t\tboundary condition[%u]: %g\n", vc_I, snacContext->bcBackWall->valueTbl[vc_I].asDouble );
		}
		printf( "\t\tnodes with boundary conditions (node#:dof#:bc#): " );
		node_count = snacContext->mesh->nodeLocalCount;
		for( node_lI = 0; node_lI < node_count; node_lI++ ) {
			Dof_Index		dof_I;
			DofLayout_Set_Index	dofSet_I;
			
			dofSet_I = DofLayout_SetOfNode( snacContext->mesh->dof, node_lI );
			for( dof_I = 0; dof_I < snacContext->mesh->dof->countTbl[dofSet_I]; dof_I++ ) {
				if( VariableCondition_GetIndex( snacContext->bcBackWall, node_lI, dof_I ) != (unsigned)-1 ) {
					printf( "%u:%u:%u ", node_lI, dof_I, VariableCondition_GetIndex( snacContext->bcBackWall, node_lI, dof_I ) );
				}
			}
		}
		printf( "\n" );
		printf( "\tFront wall: \n" );
		vcCount = snacContext->bcFrontWall->count;
		for( vc_I = 0; vc_I < vcCount; vc_I++ ) {
			printf( "\t\tboundary condition[%u]: %g\n", vc_I, snacContext->bcFrontWall->valueTbl[vc_I].asDouble );
		}
		printf( "\t\tnodes with boundary conditions (node#:dof#:bc#): " );
		node_count = snacContext->mesh->nodeLocalCount;
		for( node_lI = 0; node_lI < node_count; node_lI++ ) {
			Dof_Index		dof_I;
			DofLayout_Set_Index	dofSet_I;
			
			dofSet_I = DofLayout_SetOfNode( snacContext->mesh->dof, node_lI );
			for( dof_I = 0; dof_I < snacContext->mesh->dof->countTbl[dofSet_I]; dof_I++ ) {
				if( VariableCondition_GetIndex( snacContext->bcFrontWall, node_lI, dof_I ) != (unsigned)-1 ) {
					printf( "%u:%u:%u ", node_lI, dof_I, VariableCondition_GetIndex( snacContext->bcFrontWall, node_lI, dof_I ) );
				}
			}
		}
		printf( "\n" );

		
		printf( "dt: %g\n", snacContext->dt );
		printf( "element[0]->tetra[0].strainRate: (%12g %12g %12g) (%12g %12g %12g)\n", Snac_Element_At( snacContext, 0 )->tetra[0].strainRate[0][0], Snac_Element_At( snacContext, 0 )->tetra[0].strainRate[1][1], Snac_Element_At( snacContext, 0 )->tetra[0].strainRate[2][2], Snac_Element_At( snacContext, 0 )->tetra[0].strainRate[0][1], Snac_Element_At( snacContext, 0 )->tetra[0].strainRate[0][2], Snac_Element_At( snacContext, 0 )->tetra[0].strainRate[1][2] );
		printf( "element[0]->tetra[1].strainRate: (%12g %12g %12g) (%12g %12g %12g)\n", Snac_Element_At( snacContext, 0 )->tetra[1].strainRate[0][0], Snac_Element_At( snacContext, 0 )->tetra[1].strainRate[1][1], Snac_Element_At( snacContext, 0 )->tetra[1].strainRate[2][2], Snac_Element_At( snacContext, 0 )->tetra[1].strainRate[0][1], Snac_Element_At( snacContext, 0 )->tetra[1].strainRate[0][2], Snac_Element_At( snacContext, 0 )->tetra[1].strainRate[1][2] );
		printf( "element[0]->tetra[2].strainRate: (%12g %12g %12g) (%12g %12g %12g)\n", Snac_Element_At( snacContext, 0 )->tetra[2].strainRate[0][0], Snac_Element_At( snacContext, 0 )->tetra[2].strainRate[1][1], Snac_Element_At( snacContext, 0 )->tetra[2].strainRate[2][2], Snac_Element_At( snacContext, 0 )->tetra[2].strainRate[0][1], Snac_Element_At( snacContext, 0 )->tetra[2].strainRate[0][2], Snac_Element_At( snacContext, 0 )->tetra[2].strainRate[1][2] );
		printf( "element[0]->tetra[3].strainRate: (%12g %12g %12g) (%12g %12g %12g)\n", Snac_Element_At( snacContext, 0 )->tetra[3].strainRate[0][0], Snac_Element_At( snacContext, 0 )->tetra[3].strainRate[1][1], Snac_Element_At( snacContext, 0 )->tetra[3].strainRate[2][2], Snac_Element_At( snacContext, 0 )->tetra[3].strainRate[0][1], Snac_Element_At( snacContext, 0 )->tetra[3].strainRate[0][2], Snac_Element_At( snacContext, 0 )->tetra[3].strainRate[1][2] );
		printf( "element[0]->tetra[4].strainRate: (%12g %12g %12g) (%12g %12g %12g)\n", Snac_Element_At( snacContext, 0 )->tetra[4].strainRate[0][0], Snac_Element_At( snacContext, 0 )->tetra[4].strainRate[1][1], Snac_Element_At( snacContext, 0 )->tetra[4].strainRate[2][2], Snac_Element_At( snacContext, 0 )->tetra[4].strainRate[0][1], Snac_Element_At( snacContext, 0 )->tetra[4].strainRate[0][2], Snac_Element_At( snacContext, 0 )->tetra[4].strainRate[1][2] );
		printf( "element[0]->tetra[5].strainRate: (%12g %12g %12g) (%12g %12g %12g)\n", Snac_Element_At( snacContext, 0 )->tetra[5].strainRate[0][0], Snac_Element_At( snacContext, 0 )->tetra[5].strainRate[1][1], Snac_Element_At( snacContext, 0 )->tetra[5].strainRate[2][2], Snac_Element_At( snacContext, 0 )->tetra[5].strainRate[0][1], Snac_Element_At( snacContext, 0 )->tetra[5].strainRate[0][2], Snac_Element_At( snacContext, 0 )->tetra[5].strainRate[1][2] );
		printf( "element[0]->tetra[6].strainRate: (%12g %12g %12g) (%12g %12g %12g)\n", Snac_Element_At( snacContext, 0 )->tetra[6].strainRate[0][0], Snac_Element_At( snacContext, 0 )->tetra[6].strainRate[1][1], Snac_Element_At( snacContext, 0 )->tetra[6].strainRate[2][2], Snac_Element_At( snacContext, 0 )->tetra[6].strainRate[0][1], Snac_Element_At( snacContext, 0 )->tetra[6].strainRate[0][2], Snac_Element_At( snacContext, 0 )->tetra[6].strainRate[1][2] );
		printf( "element[0]->tetra[7].strainRate: (%12g %12g %12g) (%12g %12g %12g)\n", Snac_Element_At( snacContext, 0 )->tetra[7].strainRate[0][0], Snac_Element_At( snacContext, 0 )->tetra[7].strainRate[1][1], Snac_Element_At( snacContext, 0 )->tetra[7].strainRate[2][2], Snac_Element_At( snacContext, 0 )->tetra[7].strainRate[0][1], Snac_Element_At( snacContext, 0 )->tetra[7].strainRate[0][2], Snac_Element_At( snacContext, 0 )->tetra[7].strainRate[1][2] );
		printf( "element[0]->tetra[8].strainRate: (%12g %12g %12g) (%12g %12g %12g)\n", Snac_Element_At( snacContext, 0 )->tetra[8].strainRate[0][0], Snac_Element_At( snacContext, 0 )->tetra[8].strainRate[1][1], Snac_Element_At( snacContext, 0 )->tetra[8].strainRate[2][2], Snac_Element_At( snacContext, 0 )->tetra[8].strainRate[0][1], Snac_Element_At( snacContext, 0 )->tetra[8].strainRate[0][2], Snac_Element_At( snacContext, 0 )->tetra[8].strainRate[1][2] );
		printf( "element[0]->tetra[9].strainRate: (%12g %12g %12g) (%12g %12g %12g)\n", Snac_Element_At( snacContext, 0 )->tetra[9].strainRate[0][0], Snac_Element_At( snacContext, 0 )->tetra[9].strainRate[1][1], Snac_Element_At( snacContext, 0 )->tetra[9].strainRate[2][2], Snac_Element_At( snacContext, 0 )->tetra[9].strainRate[0][1], Snac_Element_At( snacContext, 0 )->tetra[9].strainRate[0][2], Snac_Element_At( snacContext, 0 )->tetra[9].strainRate[1][2] );
		for( element_lI = 1; element_lI < snacContext->mesh->elementLocalCount; element_lI++ ) {
			Tetrahedra_Index tetra_I;
			
			for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
				if( 
					Snac_Element_At( snacContext, element_lI )->tetra[tetra_I].strainRate[0][0] !=
					Snac_Element_At( snacContext, 0         )->tetra[tetra_I].strainRate[0][0] ||
					Snac_Element_At( snacContext, element_lI )->tetra[tetra_I].strainRate[1][1] !=
					Snac_Element_At( snacContext, 0         )->tetra[tetra_I].strainRate[1][1] ||
					Snac_Element_At( snacContext, element_lI )->tetra[tetra_I].strainRate[2][2] !=
					Snac_Element_At( snacContext, 0         )->tetra[tetra_I].strainRate[2][2] ||
					Snac_Element_At( snacContext, element_lI )->tetra[tetra_I].strainRate[0][1] !=
					Snac_Element_At( snacContext, 0         )->tetra[tetra_I].strainRate[0][1] ||
					Snac_Element_At( snacContext, element_lI )->tetra[tetra_I].strainRate[0][2] !=
					Snac_Element_At( snacContext, 0         )->tetra[tetra_I].strainRate[0][2] ||
					Snac_Element_At( snacContext, element_lI )->tetra[tetra_I].strainRate[1][2] !=
					Snac_Element_At( snacContext, 0         )->tetra[tetra_I].strainRate[1][2] )
				{
					printf( "element[%u]->tetra[%u].strainRate: (%12g %12g %12g) (%12g %12g %12g)\n", element_lI, tetra_I, Snac_Element_At( snacContext, element_lI )->tetra[0].strainRate[0][0], Snac_Element_At( snacContext, element_lI )->tetra[0].strainRate[1][1], Snac_Element_At( snacContext, element_lI )->tetra[0].strainRate[2][2], Snac_Element_At( snacContext, element_lI )->tetra[0].strainRate[0][1], Snac_Element_At( snacContext, element_lI )->tetra[0].strainRate[0][2], Snac_Element_At( snacContext, element_lI )->tetra[0].strainRate[1][2] );
				}
			}
		}
	}

	/* Stg_Class_Delete stuff */
	Stg_Class_Delete( snacContext );
	Stg_Class_Delete( dictionary );

	/* Close off MPI */
	MPI_Finalize();

	return 0; /* success */
}

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
**	Tests Tetrahedra_SurfaceArea function
**
** $Id: testTetrahedra-face-area-4-4-4.c 3243 2006-10-12 09:04:00Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"

#include "stdio.h"
#include "stdlib.h"


struct _Node {
	struct { __Snac_Node };
};

struct _Element {
	struct { __Snac_Element };
};

int main( int argc, char* argv[] ) {
	MPI_Comm CommWorld;
	int rank;
	int numProcessors;
	int procToWatch;
	Dictionary* dictionary;
	Snac_Context* snacContext;
	Tetrahedra_Index tetraIndex;
	Area area0[Tetrahedra_Count][Tetrahedra_Surface_Count];
	Element_Index elementIndex;
	
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
	dictionary->add( dictionary, "meshSizeI", Dictionary_Entry_Value_FromUnsignedInt( 4 ) );
	dictionary->add( dictionary, "meshSizeJ", Dictionary_Entry_Value_FromUnsignedInt( 4 ) );
	dictionary->add( dictionary, "meshSizeK", Dictionary_Entry_Value_FromUnsignedInt( 4 ) );
	dictionary->add( dictionary, "minX", Dictionary_Entry_Value_FromDouble( 0.0f ) );
	dictionary->add( dictionary, "minY", Dictionary_Entry_Value_FromDouble( -300.0f ) );
	dictionary->add( dictionary, "minZ", Dictionary_Entry_Value_FromDouble( 0.0f ) );
	dictionary->add( dictionary, "maxX", Dictionary_Entry_Value_FromDouble( 300.0f ) );
	dictionary->add( dictionary, "maxY", Dictionary_Entry_Value_FromDouble( 0.0f ) );
	dictionary->add( dictionary, "maxZ", Dictionary_Entry_Value_FromDouble( 300.0f ) );

	/* Build the context */
	snacContext = Snac_Context_New( 0.0f, 10.0f, sizeof(Snac_Node), sizeof(Snac_Element), CommWorld, dictionary );
	
	/* Construction phase -----------------------------------------------------------------------------------------------*/
	Stg_Component_Construct( snacContext, 0 /* dummy */, &snacContext, True );
	
	/* Building phase ---------------------------------------------------------------------------------------------------*/
	Stg_Component_Build( snacContext, 0 /* dummy */, False );
	
	/* Initialisaton phase ----------------------------------------------------------------------------------------------*/
	Stg_Component_Initialise( snacContext, 0 /* dummy */, False );
	

	/* Work out the first element's tetrahedra values, and print them. */
	printf( "Element: 0, Coords: (%g %g %g), (%g %g %g), (%g %g %g), (%g %g %g), (%g %g %g) (%g %g %g) (%g %g %g) (%g %g %g)\n",
		Snac_Element_NodeCoord( snacContext, 0, 0 )[0],
		Snac_Element_NodeCoord( snacContext, 0, 0 )[1],
		Snac_Element_NodeCoord( snacContext, 0, 0 )[2],
		Snac_Element_NodeCoord( snacContext, 0, 1 )[0],
		Snac_Element_NodeCoord( snacContext, 0, 1 )[1],
		Snac_Element_NodeCoord( snacContext, 0, 1 )[2],
		Snac_Element_NodeCoord( snacContext, 0, 3 )[0],
		Snac_Element_NodeCoord( snacContext, 0, 3 )[1],
		Snac_Element_NodeCoord( snacContext, 0, 3 )[2],
		Snac_Element_NodeCoord( snacContext, 0, 2 )[0],
		Snac_Element_NodeCoord( snacContext, 0, 2 )[1],
		Snac_Element_NodeCoord( snacContext, 0, 2 )[2],
		Snac_Element_NodeCoord( snacContext, 0, 4 )[0],
		Snac_Element_NodeCoord( snacContext, 0, 4 )[1],
		Snac_Element_NodeCoord( snacContext, 0, 4 )[2],
		Snac_Element_NodeCoord( snacContext, 0, 5 )[0],
		Snac_Element_NodeCoord( snacContext, 0, 5 )[1],
		Snac_Element_NodeCoord( snacContext, 0, 5 )[2],
		Snac_Element_NodeCoord( snacContext, 0, 7 )[0],
		Snac_Element_NodeCoord( snacContext, 0, 7 )[1],
		Snac_Element_NodeCoord( snacContext, 0, 7 )[2],
		Snac_Element_NodeCoord( snacContext, 0, 6 )[0],
		Snac_Element_NodeCoord( snacContext, 0, 6 )[1],
		Snac_Element_NodeCoord( snacContext, 0, 6 )[2] );
	for( tetraIndex = 0; tetraIndex < Tetrahedra_Count; tetraIndex++ ) {
		Tetrahedra_Surface_Index faceIndex;

		for( faceIndex = 0; faceIndex < Tetrahedra_Surface_Count; faceIndex++ ) {
			area0[tetraIndex][faceIndex] = Tetrahedra_SurfaceArea( 
				Snac_Element_NodeCoord( snacContext, 0, TetraToNode[tetraIndex][FaceToNode[faceIndex][0]] ),
				Snac_Element_NodeCoord( snacContext, 0, TetraToNode[tetraIndex][FaceToNode[faceIndex][1]] ),
				Snac_Element_NodeCoord( snacContext, 0, TetraToNode[tetraIndex][FaceToNode[faceIndex][2]] ) );
			printf( "Element: 0, Tetrahedra: %u, Face: %u, Area: %g\n", tetraIndex, faceIndex, 
				area0[tetraIndex][faceIndex] );
		}

	}


	/* For each element, compare to the first element's */
	for( elementIndex = 1; elementIndex < snacContext->mesh->elementLocalCount; elementIndex++ ) {
		Bool error;
		
		/* For each tetrahedra of the element */
		for( tetraIndex = 0, error = False; tetraIndex < Tetrahedra_Count; tetraIndex++ ) {
			Tetrahedra_Surface_Index faceIndex;

			for( faceIndex = 0; faceIndex < Tetrahedra_Surface_Count; faceIndex++ ) {
				Area area;
				
				area = Tetrahedra_SurfaceArea( 
					Snac_Element_NodeCoord( snacContext, elementIndex, 
						TetraToNode[tetraIndex][FaceToNode[faceIndex][0]] ),
					Snac_Element_NodeCoord( snacContext, elementIndex, 
						TetraToNode[tetraIndex][FaceToNode[faceIndex][1]] ),
					Snac_Element_NodeCoord( snacContext, elementIndex, 
						TetraToNode[tetraIndex][FaceToNode[faceIndex][2]] ) );
			
				if( area != area0[tetraIndex][faceIndex] ) {
					printf( "Element %u: Tetrahedra: %u, Face: %u, Area: %g (difference: %g) \n", 
						elementIndex, tetraIndex, faceIndex, area, area0[tetraIndex][faceIndex] - area );
					error = True;
				}
			}
		}
		
		if( error ) {
			printf( "Element: 0, Coords: (%g %g %g), (%g %g %g), (%g %g %g), (%g %g %g), (%g %g %g) (%g %g %g) (%g %g %g) (%g %g %g)\n",
				Snac_Element_NodeCoord( snacContext, elementIndex, 0 )[0],
				Snac_Element_NodeCoord( snacContext, elementIndex, 0 )[1],
				Snac_Element_NodeCoord( snacContext, elementIndex, 0 )[2],
				Snac_Element_NodeCoord( snacContext, elementIndex, 1 )[0],
				Snac_Element_NodeCoord( snacContext, elementIndex, 1 )[1],
				Snac_Element_NodeCoord( snacContext, elementIndex, 1 )[2],
				Snac_Element_NodeCoord( snacContext, elementIndex, 2 )[0],
				Snac_Element_NodeCoord( snacContext, elementIndex, 2 )[1],
				Snac_Element_NodeCoord( snacContext, elementIndex, 2 )[2],
				Snac_Element_NodeCoord( snacContext, elementIndex, 3 )[0],
				Snac_Element_NodeCoord( snacContext, elementIndex, 3 )[1],
				Snac_Element_NodeCoord( snacContext, elementIndex, 3 )[2],
				Snac_Element_NodeCoord( snacContext, elementIndex, 4 )[0],
				Snac_Element_NodeCoord( snacContext, elementIndex, 4 )[1],
				Snac_Element_NodeCoord( snacContext, elementIndex, 4 )[2],
				Snac_Element_NodeCoord( snacContext, elementIndex, 5 )[0],
				Snac_Element_NodeCoord( snacContext, elementIndex, 5 )[1],
				Snac_Element_NodeCoord( snacContext, elementIndex, 5 )[2],
				Snac_Element_NodeCoord( snacContext, elementIndex, 6 )[0],
				Snac_Element_NodeCoord( snacContext, elementIndex, 6 )[1],
				Snac_Element_NodeCoord( snacContext, elementIndex, 6 )[2],
				Snac_Element_NodeCoord( snacContext, elementIndex, 7 )[0],
				Snac_Element_NodeCoord( snacContext, elementIndex, 7 )[1],
				Snac_Element_NodeCoord( snacContext, elementIndex, 7 )[2] );
		}
		else {
			printf( "Element %u: has same values as element 0.\n", elementIndex );
		}		
	}
	
	/* Stg_Class_Delete stuff */
	Stg_Class_Delete( snacContext );
	Stg_Class_Delete( dictionary );

	/* Close off MPI */
	MPI_Finalize();

	return 0; /* success */
}

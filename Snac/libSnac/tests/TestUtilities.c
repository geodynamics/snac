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
**	Set up and Tear down code common to many tests.
**
** $Id: TestUtilities.c 3243 2006-10-12 09:04:00Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "TestUtilities.h"
#include "TestStress.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


MPI_Comm			CommWorld;
int				rank;
int				numProcessors;
int				procToWatch;
Dictionary*			dictionary;
Snac_Context*			snacContext;
XML_IO_Handler*			ioHandler;
char*				filename;


void SnacTest_SetUp( int argc, char* argv[] ) {
		/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size( CommWorld, &numProcessors );
	MPI_Comm_rank( CommWorld, &rank );

	Snac_Init( &argc, &argv );

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

	ioHandler = XML_IO_Handler_New();
	IO_Handler_ReadAllFromCommandLine( ioHandler, argc, argv, dictionary );
#if 0
	if (argc < 2) {
		fprintf( stderr, "Error: No input file specified using first argument. Exiting.\n" );
		exit( EXIT_FAILURE );
	}
	filename = strdup( argv[1] );

	if ( False == IO_Handler_ReadAllFromFile( ioHandler, filename, dictionary ) )
	{
		fprintf( stderr, "Error: Snac couldn't find specified input file %s. Exiting.\n", filename );
		exit( EXIT_FAILURE );
	}
#endif

	/* Build the context */
	snacContext = Snac_Context_New( 0.0f, 10.0f, sizeof(Snac_Node), sizeof(Snac_Element), CommWorld, dictionary );

	/* Construction phase -----------------------------------------------------------------------------------------------*/
	Stg_Component_Construct( snacContext, 0 /* dummy */, &snacContext, True );
	
	/* Building phase ---------------------------------------------------------------------------------------------------*/
	Stg_Component_Build( snacContext, 0 /* dummy */, False );
	
	/* Initialisaton phase ----------------------------------------------------------------------------------------------*/
	Stg_Component_Initialise( snacContext, 0 /* dummy */, False );

}


void SnacTest_TearDown( void ) {
	/* Stg_Class_Delete stuff */
	Stg_Class_Delete( snacContext );
	free( filename );
	Stg_Class_Delete( ioHandler );
	Stg_Class_Delete( dictionary );

	/* Close off MPI */
	MPI_Finalize();
}

void SnacTest_PrintElementZeroInfo( void ) {
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
	printf( "Element: 0, Vel: (%g %g %g), (%g %g %g), (%g %g %g), (%g %g %g), (%g %g %g) (%g %g %g) (%g %g %g) (%g %g %g)\n",
		Snac_Element_Node_P( snacContext, 0, 0 )->velocity[0],
		Snac_Element_Node_P( snacContext, 0, 0 )->velocity[1],
		Snac_Element_Node_P( snacContext, 0, 0 )->velocity[2],
		Snac_Element_Node_P( snacContext, 0, 1 )->velocity[0],
		Snac_Element_Node_P( snacContext, 0, 1 )->velocity[1],
		Snac_Element_Node_P( snacContext, 0, 1 )->velocity[2],
		Snac_Element_Node_P( snacContext, 0, 3 )->velocity[0],
		Snac_Element_Node_P( snacContext, 0, 3 )->velocity[1],
		Snac_Element_Node_P( snacContext, 0, 3 )->velocity[2],
		Snac_Element_Node_P( snacContext, 0, 2 )->velocity[0],
		Snac_Element_Node_P( snacContext, 0, 2 )->velocity[1],
		Snac_Element_Node_P( snacContext, 0, 2 )->velocity[2],
		Snac_Element_Node_P( snacContext, 0, 4 )->velocity[0],
		Snac_Element_Node_P( snacContext, 0, 4 )->velocity[1],
		Snac_Element_Node_P( snacContext, 0, 4 )->velocity[2],
		Snac_Element_Node_P( snacContext, 0, 5 )->velocity[0],
		Snac_Element_Node_P( snacContext, 0, 5 )->velocity[1],
		Snac_Element_Node_P( snacContext, 0, 5 )->velocity[2],
		Snac_Element_Node_P( snacContext, 0, 7 )->velocity[0],
		Snac_Element_Node_P( snacContext, 0, 7 )->velocity[1],
		Snac_Element_Node_P( snacContext, 0, 7 )->velocity[2],
		Snac_Element_Node_P( snacContext, 0, 6 )->velocity[0],
		Snac_Element_Node_P( snacContext, 0, 6 )->velocity[1],
		Snac_Element_Node_P( snacContext, 0, 6 )->velocity[2] );
}


/** Update all the elements, and in the process work out this processor's minLengthScale */
void SnacTest_UpdateElementsNodes( double* speedOfSound ) {
	double				minLengthScale;
	Element_LocalIndex		elementIndex = 0;

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

		//snacContext->dt = minLengthScale * 0.5f * 1.0e-5f / 3.0e-10f;
		*speedOfSound = minLengthScale * 0.5f / snacContext->dt;
	}
	printf( "Global minLengthScale: %g\n", minLengthScale );
	printf( "Dt: %g\n", snacContext->dt );
	printf( "Speed of sound: %g\n", *speedOfSound );

	/* For each element, calculate strain-rate and then stress */
	for( elementIndex = 0; elementIndex < snacContext->mesh->elementLocalCount; elementIndex++ ) {
		Snac_StrainRate( (Context*)snacContext, elementIndex );
		SnacTest_Stress( (Context*)snacContext, elementIndex );
	}
}

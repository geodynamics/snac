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
**	Instantiates the Snac solver/modeller.
**
** Assumptions:
**
** Comments:
**	Snac is an evolution of the FLAC methodology and code.
**	Snac is 3D.
**
** $Id: main.c 3243 2006-10-12 09:04:00Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "StGermain/StGermain.h"
#include "StGermain/FD/FD.h"
#include "Snac/Snac.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* We are not extending on Snac at all, so just let the compiler know that a node, element and particle are the Snac equivalent. */
struct _Node {
	struct { __Snac_Node };
};

struct _Element {
	struct { __Snac_Element };
};

/* Main */
int main( int argc, char* argv[] ) {
	MPI_Comm			CommWorld;
	int				rank;
	int				numProcessors;
	int				procToWatch;
	Dictionary*			dictionary;
	Dictionary*			componentDict;
	XML_IO_Handler*			ioHandler;
	char*				filename;
	Snac_Context*			snacContext;
	int				tmp;

	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size( CommWorld, &numProcessors );
	MPI_Comm_rank( CommWorld, &rank );
	if( argc >= 3 ) {
		procToWatch = atoi( argv[2] );
	}
	else {
		procToWatch = 0;
	}
	if( rank == procToWatch ) printf( "Watching rank: %i\n", rank );
	
	if (!Snac_Init( &argc, &argv )) {
		fprintf(stderr, "Error initialising StGermain, exiting.\n" );
		exit(EXIT_FAILURE);
	}
	
	/* Snac's init message */
	tmp = Stream_GetPrintingRank( Journal_Register( InfoStream_Type, "Context" ) );
	Stream_SetPrintingRank( Journal_Register( InfoStream_Type, "Context" ), 0 );
	Journal_Printf( /* DO NOT CHANGE OR REMOVE */
		Journal_Register( InfoStream_Type, "Context" ), 
		"Snac. Copyright (C) 2003-2005 Caltech, VPAC & University of Texas.\n" );
	Stream_Flush( Journal_Register( InfoStream_Type, "Context" ) );
	Stream_SetPrintingRank( Journal_Register( InfoStream_Type, "Context" ), tmp );
	MPI_Barrier( CommWorld ); /* Ensures copyright info always come first in output */
	
	
	/* Create the dictionary, and some fixed values */
	dictionary = Dictionary_New();
	Dictionary_Add( dictionary, "rank", Dictionary_Entry_Value_FromUnsignedInt( rank ) );
	Dictionary_Add( dictionary, "numProcessors", Dictionary_Entry_Value_FromUnsignedInt( numProcessors ) );
	
	/* Read input */
	ioHandler = XML_IO_Handler_New();
	if( argc >= 2 ) {
		filename = strdup( argv[1] );
	}
	else {
		filename = strdup( "input.xml" );
	}
	if ( False == IO_Handler_ReadAllFromFile( ioHandler, filename, dictionary ) )
	{
		fprintf( stderr, "Error: Snac couldn't find specified input file %s. Exiting.\n", filename );
		exit( EXIT_FAILURE );
	}
	Journal_ReadFromDictionary( dictionary );

	snacContext = Snac_Context_New( 0.0f, 0.0f, sizeof(Snac_Node), sizeof(Snac_Element), CommWorld, dictionary );
	if( rank == procToWatch ) Dictionary_PrintConcise( dictionary, snacContext->verbose );


	/* Construction phase -----------------------------------------------------------------------------------------------*/
	Stg_Component_Construct( snacContext, 0 /* dummy */, &snacContext, True );
	
	/* Building phase ---------------------------------------------------------------------------------------------------*/
	Stg_Component_Build( snacContext, 0 /* dummy */, False );
	
	/* Initialisaton phase ----------------------------------------------------------------------------------------------*/
	Stg_Component_Initialise( snacContext, 0 /* dummy */, False );
	if( rank == procToWatch ) Context_PrintConcise( snacContext, snacContext->verbose );
	
	/* Step the context solver */
	Stg_Component_Execute( snacContext, 0 /* dummy */, False );
	
	/* Stg_Class_Delete stuff */
	Stg_Component_Destroy( snacContext, 0 /* dummy */, False );
	Stg_Class_Delete( snacContext );
	free( filename );
	Stg_Class_Delete( ioHandler );
	
	Stg_Class_Delete( dictionary );
	
	/* Close off frameworks */
	Snac_Finalise();
	MPI_Finalize();
	
	return 0; /* success */
}

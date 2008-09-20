/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003, Victorian Partnership for Advanced Computing (VPAC) Ltd, 110 Victoria Street, Melbourne, 3053, Australia.
**
** Authors:
**	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
**	David May, PhD Student Monash University, VPAC. (davidm@vpac.org)
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
** Role:
**	Reads an XML file (potentially with includes) and then outputs as a single file
**
** $Id: main.c 190 2005-06-20 04:56:57Z RobertTurnbull $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <mpi.h>

#include "Base/Base.h"

void SingleRegister( void* c ) {}

int main( int argc, char* argv[] ) 
{
	Dictionary*			dictionary;
	XML_IO_Handler*			ioHandler;
	Stream*				stream;
	Stream*				error;
	Dictionary_Entry_Value*		pluginList;

	Index plugin_I, count;


	MPI_Init( &argc, &argv );
	if( !Base_Init( &argc, &argv ) ) {
		fprintf( stderr, "Error initialising StGermain, exiting.\n" );
		exit( EXIT_FAILURE );
	}
	stream = Journal_Register( Info_Type, __FILE__ );
	error = Journal_Register( Info_Type, __FILE__ );
	
	dictionary = Dictionary_New();

	/* Read input */
	ioHandler = XML_IO_Handler_New();
	IO_Handler_ReadAllFromCommandLine( ioHandler, argc, argv, dictionary );

	Stream_Enable( stream, True );
	Stream_Enable( error, True );

	pluginList = Dictionary_Get( dictionary, "plugins" );
	count = Dictionary_Entry_Value_GetCount( pluginList );
	for ( plugin_I = 0; plugin_I < count; ++plugin_I ) {
		Journal_Printf( stream, "%s\n", Dictionary_Entry_Value_AsString( Dictionary_Entry_Value_GetElement( pluginList, plugin_I ) ) );
	}

	Stg_Class_Delete( dictionary );
	Stg_Class_Delete( ioHandler );

	Base_Finalise();
	MPI_Finalize();

	return 0; /* success */
}

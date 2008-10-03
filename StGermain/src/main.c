/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003-2006, Victorian Partnership for Advanced Computing (VPAC) Ltd, 110 Victoria Street,
**	Melbourne, 3053, Australia.
**
** Primary Contributing Organisations:
**	Victorian Partnership for Advanced Computing Ltd, Computational Software Development - http://csd.vpac.org
**	Australian Computational Earth Systems Simulator - http://www.access.edu.au
**	Monash Cluster Computing - http://www.mcc.monash.edu.au
**	Computational Infrastructure for Geodynamics - http://www.geodynamics.org
**
** Contributors:
**	Patrick D. Sunter, Software Engineer, VPAC. (pds@vpac.org)
**	Robert Turnbull, Research Assistant, Monash University. (robert.turnbull@sci.monash.edu.au)
**	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
**	David May, PhD Student, Monash University (david.may@sci.monash.edu.au)
**	Louis Moresi, Associate Professor, Monash University. (louis.moresi@sci.monash.edu.au)
**	Luke J. Hodkinson, Computational Engineer, VPAC. (lhodkins@vpac.org)
**	Alan H. Lo, Computational Engineer, VPAC. (alan@vpac.org)
**	Raquibul Hassan, Computational Engineer, VPAC. (raq@vpac.org)
**	Julian Giordani, Research Assistant, Monash University. (julian.giordani@sci.monash.edu.au)
**	Vincent Lemiale, Postdoctoral Fellow, Monash University. (vincent.lemiale@sci.monash.edu.au)
**	Kent Humphries, Software Engineer, VPAC. (kenth@vpac.org)
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
** $Id: main.c 532 2006-04-04 00:21:59Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifdef HAVE_PYTHON
	#include <Python.h>
#endif
#include <mpi.h>
//EP_APPLICATIONS_FINALISE defined in StGermain.h
#include <StGermain/StGermain.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const Type StGermain_Type = "StGermain";

int main( int argc, char* argv[] ) 
{
	/* StGermain standard bits & pieces */
	MPI_Comm			CommWorld;
	int				rank;
	int				numProcessors;
	Dictionary*			dictionary;
	XML_IO_Handler*			ioHandler;

	Stream* 			infoStream;
	
	/* context */
	AbstractContext*		context = NULL;
	//This context may be initialised iff application plugins are loaded.
	AbstractContext*		replacedContext = NULL;
	EntryPoint*			applicationsFinalise_EP;

	/* Initialise PETSc, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size( CommWorld, &numProcessors );
	MPI_Comm_rank( CommWorld, &rank );
	
	StGermain_Init( &argc, &argv );
	#ifdef HAVE_PYTHON
		Py_Initialize();
	#endif	
	MPI_Barrier( CommWorld ); /* Ensures copyright info always come first in output */
	
	/* Create the application's dictionary */
	dictionary = Dictionary_New();

	/* Read input */
	ioHandler = XML_IO_Handler_New();
	IO_Handler_ReadAllFromCommandLine( ioHandler, argc, argv, dictionary );

	Journal_ReadFromDictionary( dictionary );
	

	/* Construction phase ----------------------------------------------------------------------------------------------*/
	context = _AbstractContext_New( 
			sizeof(AbstractContext),
	       	        AbstractContext_Type,
	                _AbstractContext_Delete,
	                _AbstractContext_Print,
	                NULL,
	                NULL,
	                _AbstractContext_Construct,
	                _AbstractContext_Build,
	                _AbstractContext_Initialise,
	                _AbstractContext_Execute,
	                _AbstractContext_Destroy,
	                "context",
	                True,
	                NULL,
	                0,
	                10,
	                CommWorld,
	                dictionary );

	/* Construction phase -----------------------------------------------------------------------------------------------*/
	Stg_Component_Construct( context, 0 /* dummy */, &context, True );
	
	/* Building phase ---------------------------------------------------------------------------------------------------*/
	Stg_Component_Build( context, 0 /* dummy */, False );
	
	/* Initialisaton phase ----------------------------------------------------------------------------------------------*/
	Stg_Component_Initialise( context, 0 /* dummy */, False );
	
	/* Run (Solve) phase ------------------------------------------------------------------------------------------------*/
	AbstractContext_Dump( context );
	Stg_Component_Execute( context, 0 /* dummy */, False );

	/* Destruct phase ---------------------------------------------------------------------------------------------------*/

	//Finalise any application plugins.
	applicationsFinalise_EP = Context_GetEntryPoint( context, EP_APPLICATIONS_FINALISE );
	//If there exists an applications Finalise Entry Point, get in there and run hooks!
	if(applicationsFinalise_EP != NULL)
		((EntryPoint_VoidPtr_CallCast*) applicationsFinalise_EP->run)( applicationsFinalise_EP, NULL);

	//If the context got replaced, replacedContext != NULL & needs to be deleted.
	Stg_Component_Destroy( context, 0 /* dummy */, False );
	if(replacedContext != NULL)
		Stg_Class_Delete( replacedContext );
	Stg_Class_Delete( context );
	Stg_Class_Delete( dictionary );

	//if( rank == procToWatch ) Memory_Print();
	#ifdef HAVE_PYTHON
		Py_Finalize();
	#endif

	infoStream = Journal_Register(Info_Type, "StGermainFinalise");
	Journal_Printf( infoStream, "Finalised: StGermain Framework.\n");
	StGermain_Finalise();
		
	/* Close off MPI */
	MPI_Finalize();
	
	return 0; /* success */
}

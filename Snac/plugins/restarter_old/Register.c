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
** $Id: Register.c 2498 2005-01-07 03:57:00Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "InitialConditions.h"
#include "Register.h"
#include <stdio.h>

#ifndef PATH_MAX
	#define PATH_MAX 1024
#endif

#define DEBUG

/* Textual name of this class */
const Type SnacRestartOld_Type = "SnacRestartOld";


Index _SnacRestartOld_Register( PluginsManager* pluginsMgr ) {
	return PluginsManager_Submit( pluginsMgr, 
				      SnacRestartOld_Type, 
				      "0", 
				      _SnacRestartOld_DefaultNew );
}


void* _SnacRestartOld_DefaultNew( Name name ) {
	return _Codelet_New( sizeof(Codelet), 
			     SnacRestartOld_Type, 
			     _Codelet_Delete, 
			     _Codelet_Print, 
			     _Codelet_Copy, 
			     _SnacRestartOld_DefaultNew, 
			     _SnacRestartOld_Construct, 
			     _Codelet_Build, 
			     _Codelet_Initialise, 
			     _Codelet_Execute, 
			     _Codelet_Destroy, 
			     name );
}


void _SnacRestartOld_Construct( void* component, Stg_ComponentFactory* cf, void* data ) {
	Snac_Context*	context;
	char fname[PATH_MAX];
	FILE *fp;

	/* Retrieve context. */
	context = (Snac_Context*)Stg_ComponentFactory_ConstructByName( cf, "context", Snac_Context, True, data ); 

#ifdef DEBUG
	printf( "In: _SnacRestartOld_Register( void* )\n" );
#endif

	if( context->rank == 0 ) {
		sprintf( fname, "%s/coord.%d", context->outputPath, context->rank );
		Journal_Firewall( ( ( fp = fopen( fname, "r") ) == NULL ), 
				  context->snacError, 
				  "\n\n ###### RESTARTER ERROR ######\n Do NOT restart in %s!!\n All the existing outputs will be overwritten !!\n If absolutely sure, remove the existing outputs first.\n #############################\n\n", 
				  context->outputPath );
	}

	/*
	 *  Shift the time step range to start from the restart time step
	 *    - this doesn't seem to work since the changes don't appear to propagate into StGermain
	 *     far enough to cause the maxTimeSteps to stop the simulation when desired
	 */
/* 	context->timeStep += context->restartStep; */
/* 	context->maxTimeSteps += context->restartStep; */

	EntryPoint_InsertBefore(
		Context_GetEntryPoint( context, AbstractContext_EP_Initialise ),
		"SnacTimeStepZero",
		SnacRestartOld_Type,
		_SnacRestartOld_resetMinLengthScale,
		SnacRestartOld_Type );
	EntryPoint_InsertBefore(
		Context_GetEntryPoint( context, AbstractContext_EP_Initialise ),
		"SnacTimeStepZero",
		SnacRestartOld_Type,
		_SnacRestartOld_InitialCoords,
		SnacRestartOld_Type );
	EntryPoint_InsertBefore(
		Context_GetEntryPoint( context, AbstractContext_EP_Initialise ),
		"SnacTimeStepZero",
		SnacRestartOld_Type,
		_SnacRestartOld_InitialVelocities,
		SnacRestartOld_Type );
	EntryPoint_InsertBefore(
		Context_GetEntryPoint( context, AbstractContext_EP_Initialise ),
		"SnacTimeStepZero",
		SnacRestartOld_Type,
		_SnacRestartOld_InitialStress,
		SnacRestartOld_Type );

/* 	_SnacRestartOld_resetMinLengthScale(context,data); */
/* 	_SnacRestartOld_InitialCoords(context,data); */
/* 	_SnacRestartOld_InitialVelocities(context,data); */
/* 	_SnacRestartOld_InitialStress(context,data); */
}

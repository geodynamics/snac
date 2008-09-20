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
** $Id: Register.c 3243 2006-10-12 09:04:00Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "ConstructExtensions.h"
#include "Build.h"
#include "InitialConditions.h"
#include "Constitutive.h"
#include "DeleteExtensions.h"
#include "Register.h"
#include <stdio.h>

/* Textual name of this class */
const Type SnacElastic_Type = "SnacElastic";


Index _SnacElastic_Register( PluginsManager* pluginsMgr ) {
	return PluginsManager_Submit( pluginsMgr, 
				      SnacElastic_Type, 
				      "0", 
				      _SnacElastic_DefaultNew );
}


void* _SnacElastic_DefaultNew( Name name ) {
	return _Codelet_New( sizeof(Codelet), 
			     SnacElastic_Type, 
			     _Codelet_Delete, 
			     _Codelet_Print, 
			     _Codelet_Copy, 
			     _SnacElastic_DefaultNew, 
			     _SnacElastic_Construct, 
			     _Codelet_Build, 
			     _Codelet_Initialise, 
			     _Codelet_Execute, 
			     _Codelet_Destroy, 
			     name );
}


void _SnacElastic_Construct( void* component, Stg_ComponentFactory* cf, void* data ) {
	Snac_Context*	context;

	/* Retrieve context. */
	context = (Snac_Context*)Stg_ComponentFactory_ConstructByName( cf, "context", Snac_Context, True, data ); 
	
	Journal_Printf( context->debug, "In: %s\n", __func__ );
	
	/* Add extensions to nodes, elements and the context */
	
	/* Add extensions to the entry points */
	EntryPoint_Append(
		Context_GetEntryPoint( context, AbstractContext_EP_Build ),
		SnacElastic_Type,
		_SnacElastic_Build,
		SnacElastic_Type );
	EntryPoint_Append(
		Context_GetEntryPoint( context, AbstractContext_EP_Initialise ),
		SnacElastic_Type,
		_SnacElastic_InitialConditions,
		SnacElastic_Type );
	EntryPoint_Append(
		Context_GetEntryPoint( context, Snac_EP_Constitutive ),
		SnacElastic_Type,
		_SnacElastic_Constitutive,
		SnacElastic_Type );
	EntryPoint_Append(
		Context_GetEntryPoint( context, AbstractContext_EP_DestroyExtensions ),
		SnacElastic_Type,
		_SnacElastic_DeleteExtensions,
		SnacElastic_Type );

	/* Construct. */
	_SnacElastic_ConstructExtensions( context, data );
}

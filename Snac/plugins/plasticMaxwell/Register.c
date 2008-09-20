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
#include "RunConstitutive.h"
#include "Register.h"
#include "ConstructExtensions.h"
#include "Context.h"
#include "Output.h"
#include "types.h"
#include <stdio.h>

/* Textual name of this class */
const Type SnacPlasticMaxwell_Type = "SnacPlasticMaxwell";
ExtensionInfo_Index SnacPlasticMaxwell_ContextHandle;


Index _SnacPlasticMaxwell_Register( PluginsManager* pluginsMgr ) {
	return PluginsManager_Submit( pluginsMgr, 
				      SnacPlasticMaxwell_Type, 
				      "0", 
				      _SnacPlasticMaxwell_DefaultNew );
}


void* _SnacPlasticMaxwell_DefaultNew( Name name ) {
	return _Codelet_New( sizeof(Codelet), 
			     SnacPlasticMaxwell_Type, 
			     _Codelet_Delete, 
			     _Codelet_Print, 
			     _Codelet_Copy, 
			     _SnacPlasticMaxwell_DefaultNew, 
			     _SnacPlasticMaxwell_Construct, 
			     _Codelet_Build, 
			     _Codelet_Initialise, 
			     _Codelet_Execute, 
			     _Codelet_Destroy, 
			     name );
}


void _SnacPlasticMaxwell_Construct( void* component, Stg_ComponentFactory* cf, void* data ) {
	Snac_Context*	context;

	/* Retrieve context. */
	context = (Snac_Context*)Stg_ComponentFactory_ConstructByName( cf, "context", Snac_Context, True, data ); 

	Journal_Printf( context->debug, "In: %s\n", __func__ );

	SnacPlasticMaxwell_ContextHandle = ExtensionManager_Add( context->extensionMgr, SnacPlasticMaxwell_Type, sizeof(SnacPlasticMaxwell_Context) );

	/* This is a completely different type of plugin... all it does is change the default behaviour of the "Constitutive"
	   entry point runner, to one that will factor in the criterion in plastic-maxwell coupled case */
	Snac_EntryPoint_ChangeRunConstitutive(
		Context_GetEntryPoint( context, Snac_EP_Constitutive ),
		_SnacPlasticMaxwell_RunConstitutive );
	EntryPoint_Prepend( /* Dump the initial plastic strain */
		Context_GetEntryPoint( context, AbstractContext_EP_Execute ),
		"SnacPlasticMaxwell_Dump",
		_SnacPlasticMaxwell_DumpIRheology,
		SnacPlasticMaxwell_Type );
	EntryPoint_Append( /* and dump each loop */
		Context_GetEntryPoint( context, Snac_EP_CalcStresses ),
		"SnacPlasticMaxwell_Dump",
		_SnacPlasticMaxwell_DumpIRheology,
		SnacPlasticMaxwell_Type );

	/* Construct. */
	_SnacPlasticMaxwell_ConstructExtensions( context, data );
}

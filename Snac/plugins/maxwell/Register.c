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
#include "units.h"
#include "types.h"
#include "Context.h"
#include "Element.h"
#include "ConstructExtensions.h"
#include "Build.h"
#include "InitialConditions.h"
#include "Constitutive.h"
#include "Output.h"
#include "DeleteExtensions.h"
#include "Register.h"
#include <stdio.h>

/* Textual name of this class */
const Type SnacMaxwell_Type = "SnacMaxwell";

ExtensionInfo_Index SnacMaxwell_ElementHandle;
ExtensionInfo_Index SnacMaxwell_ContextHandle;


Index _SnacMaxwell_Register( PluginsManager* pluginsMgr ) {
	return PluginsManager_Submit( pluginsMgr, 
				      SnacMaxwell_Type, 
				      "0", 
				      _SnacMaxwell_DefaultNew );
}


void* _SnacMaxwell_DefaultNew( Name name ) {
	return _Codelet_New( sizeof(Codelet), 
			     SnacMaxwell_Type, 
			     _Codelet_Delete, 
			     _Codelet_Print, 
			     _Codelet_Copy, 
			     _SnacMaxwell_DefaultNew, 
			     _SnacMaxwell_Construct, 
			     _Codelet_Build, 
			     _Codelet_Initialise, 
			     _Codelet_Execute, 
			     _Codelet_Destroy, 
			     name );
}


void _SnacMaxwell_Construct( void* component, Stg_ComponentFactory* cf, void* data ) {
	Snac_Context*	context;

	/* Retrieve context. */
	context = (Snac_Context*)Stg_ComponentFactory_ConstructByName( cf, "context", Snac_Context, True, data ); 

	#ifdef DEBUG
		printf( "In: %s\n", __func__ );
	#endif

	/* Add extensions to nodes, elements and the context */
	SnacMaxwell_ElementHandle = ExtensionManager_Add(
		context->mesh->elementExtensionMgr,
		SnacMaxwell_Type,
		sizeof(SnacMaxwell_Element) );
	SnacMaxwell_ContextHandle = ExtensionManager_Add(
		context->extensionMgr,
		SnacMaxwell_Type,
		sizeof(SnacMaxwell_Context) );

	#ifdef DEBUG
		printf( "\telement extension handle: %u\n", SnacMaxwell_ContextHandle );
		printf( "\telement extension handle: %u\n", SnacMaxwell_ElementHandle );
	#endif

	/* Add extensions to the entry points */
	EntryPoint_Append(
		Context_GetEntryPoint( context, AbstractContext_EP_Build ),
		SnacMaxwell_Type,
		_SnacMaxwell_Build,
		SnacMaxwell_Type );
	EntryPoint_Append(
		Context_GetEntryPoint( context, AbstractContext_EP_Initialise ),
		SnacMaxwell_Type,
		_SnacMaxwell_InitialConditions,
		SnacMaxwell_Type );
	EntryPoint_Append(
		Context_GetEntryPoint( context, Snac_EP_Constitutive ),
		SnacMaxwell_Type,
		_SnacMaxwell_Constitutive,
		SnacMaxwell_Type );
	EntryPoint_Append(
		Context_GetEntryPoint( context, AbstractContext_EP_DestroyExtensions ),
		SnacMaxwell_Type,
		_SnacMaxwell_DeleteExtensions,
		SnacMaxwell_Type );
	EntryPoint_Prepend( /* Dump the initial viscosity */
		Context_GetEntryPoint( context, AbstractContext_EP_Execute ),
		"SnacMaxwell_Dump",
		_SnacMaxwell_DumpViscosity,
		SnacMaxwell_Type );
	EntryPoint_Append( /* and dump each loop */
		Context_GetEntryPoint( context, Snac_EP_CalcStresses ),
		"SnacMaxwell_Dump",
		_SnacMaxwell_DumpViscosity,
		SnacMaxwell_Type );

	/* Construct. */
	_SnacMaxwell_ConstructExtensions( context, data );
}

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
#include "Heat.h"
#include "HeatFlux.h"
#include "Node.h"
#include "ConstructExtensions.h"
#include "Build.h"
#include "Output.h"
#include "DeleteExtensions.h"
#include "VariableConditions.h"
#include "Remesh.h"
#include "Register.h"
#include <stdio.h>

/* Textual name of this class */
const Type SnacTemperature_Type = "SnacTemperature";

ExtensionInfo_Index SnacTemperature_ContextHandle;
ExtensionInfo_Index SnacTemperature_ElementHandle;
ExtensionInfo_Index SnacTemperature_NodeHandle;


Index _SnacTemperature_Register( PluginsManager* pluginsMgr ) {
	return PluginsManager_Submit( pluginsMgr, 
				      SnacTemperature_Type, 
				      "0", 
				      _SnacTemperature_DefaultNew );
}


void* _SnacTemperature_DefaultNew( Name name ) {
	return _Codelet_New( sizeof(Codelet), 
			     SnacTemperature_Type, 
			     _Codelet_Delete, 
			     _Codelet_Print, 
			     _Codelet_Copy, 
			     _SnacTemperature_DefaultNew, 
			     _SnacTemperature_Construct, 
			     _Codelet_Build, 
			     _Codelet_Initialise, 
			     _Codelet_Execute, 
			     _Codelet_Destroy, 
			     name );
}


void _SnacTemperature_Construct( void* component, Stg_ComponentFactory* cf, void* data ) {
	Snac_Context*	context;
	EntryPoint* 	interpolateNodeEP;

	/* Retrieve context. */
	context = (Snac_Context*)Stg_ComponentFactory_ConstructByName( cf, "context", Snac_Context, True, data ); 

	#ifdef DEBUG
		printf( "In: _SnacTemperature_Register( void*, void* )\n" );
	#endif

	/* Add extensions to nodes, elements and the context */
	SnacTemperature_NodeHandle = ExtensionManager_Add( context->mesh->nodeExtensionMgr, SnacTemperature_Type, sizeof(SnacTemperature_Node) );
	SnacTemperature_ElementHandle = ExtensionManager_Add( context->mesh->elementExtensionMgr, SnacTemperature_Type, sizeof(SnacTemperature_Element) );
	SnacTemperature_ContextHandle = ExtensionManager_Add( context->extensionMgr, SnacTemperature_Type, sizeof(SnacTemperature_Context) );

	#ifdef DEBUG
		printf( "\tcontext extension handle: %u\n", SnacTemperature_ContextHandle );
		printf( "\telement extension handle: %u\n", SnacTemperature_ElementHandle );
		printf( "\tnode extension handle: %u\n", SnacTemperature_NodeHandle );
	#endif


	/* Add extensions to the entry points */
	EntryPoint_Append(
		Context_GetEntryPoint( context, AbstractContext_EP_Build ),
		SnacTemperature_Type,
		_SnacTemperature_Build,
		SnacTemperature_Type );
	EntryPoint_InsertBefore(
		Context_GetEntryPoint( context, AbstractContext_EP_Initialise ),
		"SnacTimeStepZero",
		SnacTemperature_Type,
		_SnacTemperature_InitialConditions,
		SnacTemperature_Type );
	EntryPoint_Append(
		Context_GetEntryPoint( context, Snac_EP_LoopNodesEnergy ),
		SnacTemperature_Type,
		SnacTemperature_LoopNodes,
		SnacTemperature_Type );
	EntryPoint_Append(
		Context_GetEntryPoint( context, Snac_EP_LoopElementsEnergy ),
		SnacTemperature_Type,
		SnacTemperature_LoopElements,
		SnacTemperature_Type );
	EntryPoint_Prepend( /* Dump the initial temperature */
		Context_GetEntryPoint( context, AbstractContext_EP_Execute ),
		"SnacTemperature_Write",
		_SnacTemperature_WriteTemp,
		SnacTemperature_Type );
	EntryPoint_Append( /* and dump each loop */
		Context_GetEntryPoint( context, Snac_EP_LoopNodesEnergy ),
		"SnacTemperature_Write",
		_SnacTemperature_WriteTemp,
		SnacTemperature_Type );
	EntryPoint_Append(
		Context_GetEntryPoint( context, AbstractContext_EP_DestroyExtensions ),
		SnacTemperature_Type,
		_SnacTemperature_DeleteExtensions,
		SnacTemperature_Type );

	/* Add extensions to the interpolate element entry point, but it will only exist if the remesher is loaded. */
	interpolateNodeEP = Context_GetEntryPoint( context, "SnacRemesher_EP_InterpolateNode" );
	if( interpolateNodeEP ) {
		EntryPoint_Append(
			interpolateNodeEP,
			SnacTemperature_Type,
			_SnacTemperature_InterpolateNode,
			SnacTemperature_Type );
	}

	/* Construct. */
	_SnacTemperature_ConstructExtensions( context, data );
}

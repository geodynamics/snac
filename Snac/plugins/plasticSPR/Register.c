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
#include "Node.h"
#include "Element.h"
#include "Mesh.h"
#include "Context.h"
#include "Constitutive.h"
#include "ConstructExtensions.h"
#include "Output.h"
#include "InitialConditions.h"
#include "Remesh.h"
#include "Register.h"
#include <stdio.h>

/* Textual name of this class */
const Type SnacPlastic_Type = "SnacPlastic";

ExtensionInfo_Index SnacPlastic_NodeHandle;
ExtensionInfo_Index SnacPlastic_ElementHandle;
ExtensionInfo_Index SnacPlastic_MeshHandle;
ExtensionInfo_Index SnacPlastic_ContextHandle;


Index _SnacPlastic_Register( PluginsManager* pluginsMgr ) {
	return PluginsManager_Submit( pluginsMgr, 
				      SnacPlastic_Type, 
				      "0", 
				      _SnacPlastic_DefaultNew );
}


void* _SnacPlastic_DefaultNew( Name name ) {
	return _Codelet_New( sizeof(Codelet), 
			     SnacPlastic_Type, 
			     _Codelet_Delete, 
			     _Codelet_Print, 
			     _Codelet_Copy, 
			     _SnacPlastic_DefaultNew, 
			     _SnacPlastic_Construct, 
			     _Codelet_Build, 
			     _Codelet_Initialise, 
			     _Codelet_Execute, 
			     _Codelet_Destroy, 
			     name );
}


void _SnacPlastic_Construct( void* component, Stg_ComponentFactory* cf, void* data ) {
	Snac_Context*	context;
	EntryPoint* 	recoverNodeEP;
	EntryPoint* 	interpolateNodeEP;
	EntryPoint* 	interpolateElementEP;

	/* Retrieve context. */
	context = (Snac_Context*)Stg_ComponentFactory_ConstructByName( cf, "context", Snac_Context, True, data ); 

	#ifdef DEBUG
		printf( "In: _SnacPlastic_Register( void*, void* )\n" );
	#endif
	
	/* Add extensions to nodes, elements and the context */
	SnacPlastic_NodeHandle = ExtensionManager_Add( context->mesh->nodeExtensionMgr, SnacPlastic_Type, sizeof(SnacPlastic_Node) );
	SnacPlastic_ElementHandle = ExtensionManager_Add( context->mesh->elementExtensionMgr, SnacPlastic_Type, sizeof(SnacPlastic_Element) );
	SnacPlastic_MeshHandle = ExtensionManager_Add( context->meshExtensionMgr, SnacPlastic_Type, sizeof(SnacPlastic_Mesh) );
	SnacPlastic_ContextHandle = ExtensionManager_Add( context->extensionMgr, SnacPlastic_Type, sizeof(SnacPlastic_Context) );
	
	#ifdef DEBUG
		printf( "\tcontext extension handle: %u\n", SnacPlastic_ContextHandle );
		printf( "\tmesh extension handle: %u\n", SnacPlastic_MeshHandle );
		printf( "\tnode extension handle: %u\n", SnacPlastic_NodeHandle );
		printf( "\telement extension handle: %u\n", SnacPlastic_ElementHandle );
	#endif
	
	/* Add extensions to the entry points */
	EntryPoint_Append( 
		Context_GetEntryPoint( context,	Snac_EP_Constitutive ),
		SnacPlastic_Type, 
		SnacPlastic_Constitutive, 
		SnacPlastic_Type );
	EntryPoint_InsertBefore( 
		Context_GetEntryPoint( context,	AbstractContext_EP_Initialise ),
		"SnacTimeStepZero",
		SnacPlastic_Type, 
		SnacPlastic_InitialConditions,
		SnacPlastic_Type );
	EntryPoint_Prepend( /* Dump the initial plastic strain */
		Context_GetEntryPoint( context, AbstractContext_EP_Execute ),
		"SnacPlastic_Write",
		_SnacPlastic_WritePlasticStrain,
		SnacPlastic_Type );
	EntryPoint_Append( /* and dump each loop */
		Context_GetEntryPoint( context, Snac_EP_CalcStresses ),
		"SnacPlastic_Write",
		_SnacPlastic_WritePlasticStrain,
		SnacPlastic_Type );
	
	/* Add extensions to the interpolate element entry point, but it will only exist if the remesher is loaded. */
	recoverNodeEP = Context_GetEntryPoint( context,	"SnacRemesher_EP_RecoverNode" );
	if( recoverNodeEP ) {
		EntryPoint_Append(
						  recoverNodeEP,
						  SnacPlastic_Type,
						  _SnacPlastic_RecoverNode,
						  SnacPlastic_Type );
	}
	interpolateNodeEP = Context_GetEntryPoint( context,	"SnacRemesher_EP_InterpolateNode" );
	if( interpolateNodeEP ) {
		EntryPoint_Append(
						  interpolateNodeEP,
						  SnacPlastic_Type,
						  _SnacPlastic_InterpolateNode,
						  SnacPlastic_Type );
	}
	interpolateElementEP = Context_GetEntryPoint( context,	"SnacRemesher_EP_InterpolateElement" );
	if( interpolateElementEP ) {
		EntryPoint_Append( 
						  interpolateElementEP,
						  SnacPlastic_Type, 
						  _SnacPlastic_InterpolateElement, 
						  SnacPlastic_Type );
	}

	/* Construct. */
	_SnacPlastic_ConstructExtensions( context, data );
}

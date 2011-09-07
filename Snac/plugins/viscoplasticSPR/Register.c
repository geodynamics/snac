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
#include "ConstructExtensions.h"
#include "Constitutive.h"
#include "Context.h"
#include "Destroy.h"
#include "InitialConditions.h"
#include "Element.h"
#include "Mesh.h"
#include "Node.h"
#include "Output.h"
#include "Register.h"
#include "Remesh.h"
#include "types.h"
#include <stdio.h>

/* Textual name of this class */
const Type SnacViscoPlastic_Type = "SnacViscoPlastic";

ExtensionInfo_Index SnacViscoPlastic_NodeHandle;
ExtensionInfo_Index SnacViscoPlastic_ElementHandle;
ExtensionInfo_Index SnacViscoPlastic_MeshHandle;
ExtensionInfo_Index SnacViscoPlastic_ContextHandle;

Index _SnacViscoPlastic_Register( PluginsManager* pluginsMgr ) {
	return PluginsManager_Submit( pluginsMgr, 
				      SnacViscoPlastic_Type, 
				      "0", 
				      _SnacViscoPlastic_DefaultNew );
}


void* _SnacViscoPlastic_DefaultNew( Name name ) {
	return _Codelet_New( sizeof(Codelet), 
			     SnacViscoPlastic_Type, 
			     _Codelet_Delete, 
			     _Codelet_Print, 
			     _Codelet_Copy, 
			     _SnacViscoPlastic_DefaultNew, 
			     _SnacViscoPlastic_Construct, 
			     _Codelet_Build, 
			     _Codelet_Initialise, 
			     _Codelet_Execute, 
			     _Codelet_Destroy, 
			     name );
}


void _SnacViscoPlastic_Construct( void* component, Stg_ComponentFactory* cf, void* data ) {
	Snac_Context*	context;
	EntryPoint* 	recoverNodeEP;
	EntryPoint* 	interpolateNodeEP;
	EntryPoint* 	interpolateElementEP;

	/* Retrieve context. */
	context = (Snac_Context*)Stg_ComponentFactory_ConstructByName( cf, "context", Snac_Context, True, data ); 

	#ifdef DEBUG
		printf( "In: _SnacViscoPlastic_Register( void*, void* )\n" );
	#endif

	/* Add extensions to nodes, elements and the context */
	SnacViscoPlastic_NodeHandle = ExtensionManager_Add( context->mesh->nodeExtensionMgr, SnacViscoPlastic_Type, sizeof(SnacViscoPlastic_Node) );
	SnacViscoPlastic_ElementHandle = ExtensionManager_Add( context->mesh->elementExtensionMgr, SnacViscoPlastic_Type, sizeof(SnacViscoPlastic_Element) );
	SnacViscoPlastic_MeshHandle = ExtensionManager_Add( context->meshExtensionMgr, SnacViscoPlastic_Type, sizeof(SnacViscoPlastic_Mesh) );
	SnacViscoPlastic_ContextHandle = ExtensionManager_Add( context->extensionMgr, SnacViscoPlastic_Type, sizeof(SnacViscoPlastic_Context) );

	#ifdef DEBUG
		printf( "\tcontext extension handle: %u\n", SnacViscoPlastic_ContextHandle );
		printf( "\tmesh extension handle: %u\n", SnacViscoPlastic_MeshHandle );
		printf( "\tnode extension handle: %u\n", SnacViscoPlastic_ElementHandle );
		printf( "\telement extension handle: %u\n", SnacViscoPlastic_NodeHandle );
	#endif

	/* Add extensions to the entry points */
	EntryPoint_Append(
		Context_GetEntryPoint( context,	Snac_EP_Constitutive ),
		SnacViscoPlastic_Type,
		SnacViscoPlastic_Constitutive,
		SnacViscoPlastic_Type );
	EntryPoint_InsertBefore(
		Context_GetEntryPoint( context,	AbstractContext_EP_Initialise ),
		"SnacTimeStepZero",
		SnacViscoPlastic_Type,
		SnacViscoPlastic_InitialConditions,
		SnacViscoPlastic_Type );
	EntryPoint_Prepend( /* Dump the initial plastic strain */
		Context_GetEntryPoint( context, AbstractContext_EP_Execute ),
		"SnacViscoPlastic_WritePlasticStrain",
		_SnacViscoPlastic_WritePlasticStrain,
		SnacViscoPlastic_Type );
	EntryPoint_Append( /* and dump each loop */
		Context_GetEntryPoint( context, Snac_EP_CalcStresses ),
		"SnacViscoPlastic_WritePlasticStrain",
		_SnacViscoPlastic_WritePlasticStrain,
		SnacViscoPlastic_Type );
	EntryPoint_Prepend( /* Dump the initial viscosity */
		Context_GetEntryPoint( context, AbstractContext_EP_Execute ),
		"SnacViscoPlastic_WriteViscosity",
		_SnacViscoPlastic_WriteViscosity,
		SnacViscoPlastic_Type );
	EntryPoint_Append( /* and dump visocisty each loop */
		Context_GetEntryPoint( context, Snac_EP_CalcStresses ),
		"SnacViscoPlastic_WriteViscosity",
		_SnacViscoPlastic_WriteViscosity,
		SnacViscoPlastic_Type );
	EntryPoint_Append( /* perform clean-ups like closing output files. */
		Context_GetEntryPoint( context, AbstractContext_EP_DestroyExtensions ),
		"SnacViscoPlastic_Destory",
		_SnacViscoPlastic_Destroy,
		SnacViscoPlastic_Type );

	/* Add extensions to the interpolate element entry point, but it will only exist if the remesher is loaded. */
	recoverNodeEP = Context_GetEntryPoint( context,	"SnacRemesher_EP_RecoverNode" );
	if( recoverNodeEP ) {
		EntryPoint_Append(
						  recoverNodeEP,
						  SnacViscoPlastic_Type,
						  _SnacViscoPlastic_RecoverNode,
						  SnacViscoPlastic_Type );
	}
	interpolateNodeEP = Context_GetEntryPoint( context,	"SnacRemesher_EP_InterpolateNode" );
	if( interpolateNodeEP ) {
		EntryPoint_Append(
						  interpolateNodeEP,
						  SnacViscoPlastic_Type,
						  _SnacViscoPlastic_InterpolateNode,
						  SnacViscoPlastic_Type );
	}
	interpolateElementEP = Context_GetEntryPoint( context,	"SnacRemesher_EP_InterpolateElement" );
	if( interpolateElementEP ) {
		EntryPoint_Append(
						  interpolateElementEP,
						  SnacViscoPlastic_Type,
						  _SnacViscoPlastic_InterpolateElement,
						  SnacViscoPlastic_Type );
	}

	/* Construct. */
	_SnacViscoPlastic_ConstructExtensions( context, data );
}

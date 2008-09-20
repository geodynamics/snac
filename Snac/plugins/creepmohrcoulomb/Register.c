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
#include "Element.h"
#include "Context.h"
#include "Constitutive.h"
#include "ConstructExtensions.h"
#include "Output.h"
#include "InitialConditions.h"
#include "Remesh.h"
#include "Register.h"
#include <stdio.h>

/* Textual name of this class */
const Type SnacCreepMohrCoulomb_Type = "SnacCreepMohrCoulomb";

ExtensionInfo_Index SnacCreepMohrCoulomb_ElementHandle;
ExtensionInfo_Index SnacCreepMohrCoulomb_ContextHandle;


Index _SnacCreepMohrCoulomb_Register( PluginsManager* pluginsMgr ) {
	return PluginsManager_Submit( pluginsMgr, 
				      SnacCreepMohrCoulomb_Type, 
				      "0", 
				      _SnacCreepMohrCoulomb_DefaultNew );
}


void* _SnacCreepMohrCoulomb_DefaultNew( Name name ) {
	return _Codelet_New( sizeof(Codelet), 
			     SnacCreepMohrCoulomb_Type, 
			     _Codelet_Delete, 
			     _Codelet_Print, 
			     _Codelet_Copy, 
			     _SnacCreepMohrCoulomb_DefaultNew, 
			     _SnacCreepMohrCoulomb_Construct, 
			     _Codelet_Build, 
			     _Codelet_Initialise, 
			     _Codelet_Execute, 
			     _Codelet_Destroy, 
			     name );
}


void _SnacCreepMohrCoulomb_Construct( void* component, Stg_ComponentFactory* cf, void* data ) {
	Snac_Context*	context;
	EntryPoint* 	interpolateElementEP;

	/* Retrieve context. */
	context = (Snac_Context*)Stg_ComponentFactory_ConstructByName( cf, "context", Snac_Context, True, data ); 

	#ifdef DEBUG
		printf( "In: _SnacCreepMohrCoulomb_Register( void*, void* )\n" );
	#endif

	/* Add extensions to nodes, elements and the context */
	SnacCreepMohrCoulomb_ElementHandle = ExtensionManager_Add( context->mesh->elementExtensionMgr, SnacCreepMohrCoulomb_Type, sizeof(SnacCreepMohrCoulomb_Element) );
	SnacCreepMohrCoulomb_ContextHandle = ExtensionManager_Add( context->extensionMgr, SnacCreepMohrCoulomb_Type, sizeof(SnacCreepMohrCoulomb_Context) );

	#ifdef DEBUG
		printf( "\tcontext extension handle: %u\n", SnacCreepMohrCoulomb_ContextHandle );
		printf( "\telement extension handle: %u\n", SnacCreepMohrCoulomb_ElementHandle );
	#endif

	/* Add extensions to the entry points */
	EntryPoint_Append(
		Context_GetEntryPoint( context,	Snac_EP_Constitutive ),
		SnacCreepMohrCoulomb_Type,
		SnacCreepMohrCoulomb_Constitutive,
		SnacCreepMohrCoulomb_Type );
	EntryPoint_InsertBefore(
		Context_GetEntryPoint( context,	AbstractContext_EP_Initialise ),
		"SnacTimeStepZero",
		SnacCreepMohrCoulomb_Type,
		SnacCreepMohrCoulomb_InitialConditions,
		SnacCreepMohrCoulomb_Type );
	EntryPoint_Prepend( /* Dump the initial plastic strain */
		Context_GetEntryPoint( context, AbstractContext_EP_Execute ),
		"SnacCreepMohrCoulomb_Dump",
		_SnacCreepMohrCoulomb_DumpPlasticStrain,
		SnacCreepMohrCoulomb_Type );
	EntryPoint_Append( /* and dump each loop */
		Context_GetEntryPoint( context, Snac_EP_CalcStresses ),
		"SnacCreepMohrCoulomb_Dump",
		_SnacCreepMohrCoulomb_DumpPlasticStrain,
		SnacCreepMohrCoulomb_Type );
	EntryPoint_Prepend( /* Dump the initial tetra plastic strain */
		Context_GetEntryPoint( context, AbstractContext_EP_Execute ),
		"SnacCreepMohrCoulomb_Dump",
		_SnacCreepMohrCoulomb_DumpPlasticStrainTensor,
		SnacCreepMohrCoulomb_Type );
	EntryPoint_Append( /* and dump tetra plastic strain each loop */
		Context_GetEntryPoint( context, Snac_EP_CalcStresses ),
		"SnacCreepMohrCoulomb_Dump",
		_SnacCreepMohrCoulomb_DumpPlasticStrainTensor,
		SnacCreepMohrCoulomb_Type );
	EntryPoint_Prepend( /* Dump the initial viscosity*/
		Context_GetEntryPoint( context, AbstractContext_EP_Execute ),
		"SnacCreepMohrCoulomb_Dump",
		_SnacCreepMohrCoulomb_DumpViscosity,
		SnacCreepMohrCoulomb_Type );
	EntryPoint_Append( /* and dump visocisty each loop */
		Context_GetEntryPoint( context, Snac_EP_CalcStresses ),
		"SnacCreepMohrCoulomb_Dump",
		_SnacCreepMohrCoulomb_DumpViscosity,
		SnacCreepMohrCoulomb_Type );

	/* Add extensions to the interpolate element entry point, but it will only exist if the remesher is loaded. */
	interpolateElementEP = Context_GetEntryPoint( context,	"SnacRemesher_EP_InterpolateElement" );
	if( interpolateElementEP ) {
		EntryPoint_Append(
			interpolateElementEP,
			SnacCreepMohrCoulomb_Type,
			_SnacCreepMohrCoulomb_InterpolateElement,
			SnacCreepMohrCoulomb_Type );
	}

	/* Construct. */
	_SnacCreepMohrCoulomb_ConstructExtensions( context, data );
}

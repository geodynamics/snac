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
**  Mods by:
**           Colin Stark, Doherty Research Scientist, Lamont-Doherty Earth Observatory (cstark@ldeo.columbia.edu)
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
** $Id: Register.c $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "Context.h"
#include "ConstructExtensions.h"
#include "Build.h"
#include "InitialConditions.h"
#include "DeleteExtensions.h"
#include "Register.h"
#include "Snac/Plastic/Plastic.h"

#include <stdio.h>

//#define DEBUG

/* Textual name of this class */
const Type SnacHillSlope_Type = "SnacHillSlope";

ExtensionInfo_Index SnacHillSlope_ContextHandle = -1;


Index _SnacHillSlope_Register( PluginsManager* pluginsMgr ) {
	return PluginsManager_Submit( pluginsMgr, 
				      SnacHillSlope_Type, 
				      "0", 
				      _SnacHillSlope_DefaultNew );
}


void* _SnacHillSlope_DefaultNew( Name name ) {
	return _Codelet_New( sizeof(Codelet), 
			     SnacHillSlope_Type, 
			     _Codelet_Delete, 
			     _Codelet_Print, 
			     _Codelet_Copy, 
			     _SnacHillSlope_DefaultNew, 
			     _SnacHillSlope_Construct, 
			     _Codelet_Build, 
			     _Codelet_Initialise, 
			     _Codelet_Execute, 
			     _Codelet_Destroy, 
			     name );
}


void _SnacHillSlope_Construct( void* component, Stg_ComponentFactory* cf, void* data ) {
	Snac_Context*	context;

	#ifdef DEBUG
	fprintf(stderr, "Entering Register.c...\n");
	#endif

	/* Retrieve context. */
	context = (Snac_Context*)Stg_ComponentFactory_ConstructByName( cf, "context", Snac_Context, True, data ); 
	
	Journal_Printf( context->debug, "In: %s\n", __func__ );
	
	
	/* Add extensions to nodes, elements and the context */
	SnacHillSlope_ContextHandle = ExtensionManager_Add(
		context->extensionMgr,
		SnacHillSlope_Type,
		sizeof(SnacHillSlope_Context) );
	
	/* Add extensions to the entry points */
	EntryPoint_Append(
		Context_GetEntryPoint( context, AbstractContext_EP_Build ),
		SnacHillSlope_Type,
		_SnacHillSlope_Build,
		SnacHillSlope_Type );
	EntryPoint_Append(
		Context_GetEntryPoint( context, AbstractContext_EP_Initialise ),
		SnacHillSlope_Type,
		_SnacHillSlope_InitialConditions,
		SnacHillSlope_Type );

/* 	EntryPoint_Append( */
/* 		Context_GetEntryPoint( context, Snac_EP_Constitutive ), */
/* 		SnacHillSlope_Type, */
/* 		_SnacHillSlope_Constitutive, */
/* 		SnacHillSlope_Type ); */
	EntryPoint_Append(
		Context_GetEntryPoint( context, AbstractContext_EP_DestroyExtensions ),
		SnacHillSlope_Type,
		_SnacHillSlope_DeleteExtensions,
		SnacHillSlope_Type );


	/* Construct. */
	_SnacHillSlope_ConstructExtensions( context, data );

	#ifdef DEBUG
	fprintf(stderr, "In Register.c\n");
	#endif

	#ifdef DEBUG
	fprintf(stderr, "...leaving Register.c\n");
	#endif


}

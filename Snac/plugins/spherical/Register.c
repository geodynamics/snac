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
#include "Mesh.h"
#include "ConstructExtensions.h"
#include "InitialConditions.h"
#include "Register.h"
#include <stdio.h>

/* Textual name of this class */
const Type SnacSpherical_Type = "SnacSpherical";

ExtensionInfo_Index SnacSpherical_MeshHandle;


Index _SnacSpherical_Register( PluginsManager* pluginsMgr ) {
	return PluginsManager_Submit( pluginsMgr, 
				      SnacSpherical_Type, 
				      "0", 
				      _SnacSpherical_DefaultNew );
}


void* _SnacSpherical_DefaultNew( Name name ) {
	return _Codelet_New( sizeof(Codelet), 
			     SnacSpherical_Type, 
			     _Codelet_Delete, 
			     _Codelet_Print, 
			     _Codelet_Copy, 
			     _SnacSpherical_DefaultNew, 
			     _SnacSpherical_Construct, 
			     _Codelet_Build, 
			     _Codelet_Initialise, 
			     _Codelet_Execute, 
			     _Codelet_Destroy, 
			     name );
}


void _SnacSpherical_Construct( void* component, Stg_ComponentFactory* cf, void* data ) {
	Snac_Context*	context;

	/* Retrieve context. */
	context = (Snac_Context*)Stg_ComponentFactory_ConstructByName( cf, "context", Snac_Context, True, data ); 

	#ifdef DEBUG
		printf( "In: _SnacSpherical_Register( void* )\n" );
	#endif

	/* Add extensions to nodes, elements and the context */
	SnacSpherical_MeshHandle = ExtensionManager_Add( 
		context->meshExtensionMgr, 
		SnacSpherical_Type, 
		sizeof(SnacSpherical_Mesh) );

	#ifdef DEBUG
		printf( "\tMesh extension handle: %u\n", SnacSpherical_MeshHandle );
	#endif

	EntryPoint_InsertBefore( 
		Context_GetEntryPoint( context, AbstractContext_EP_Initialise ), 
		"SnacIC",
		SnacSpherical_Type, 
		_SnacSpherical_InitialConditions, 
		SnacSpherical_Type );

	/* Construct. */
	_SnacSpherical_ConstructExtensions( context, data );
}

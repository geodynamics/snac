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
** $Id: Register.c 1247 2004-04-20 00:40:15Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "Register.h"
#include "Force.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

/* Textual name of this class */
const Type SnacWinklerForce_Type = "SnacWinklerForce";
static char RMIN_STR[] = "rMin";
static char RMAX_STR[] = "rMax";
static char MESH_STR[] = "mesh";
static double MIN[] = { -45.0f, -45.0f, 0.5f };
static double MAX[] = { 45.0f, 45.0f, 1.0f };


Index _SnacWinklerForce_Register( PluginsManager* pluginsMgr ) {
	return PluginsManager_Submit( pluginsMgr, 
				      SnacWinklerForce_Type, 
				      "0", 
				      _SnacWinklerForce_DefaultNew );
}


void* _SnacWinklerForce_DefaultNew( Name name ) {
	return _Codelet_New( sizeof(Codelet), 
			     SnacWinklerForce_Type, 
			     _Codelet_Delete, 
			     _Codelet_Print, 
			     _Codelet_Copy, 
			     _SnacWinklerForce_DefaultNew, 
			     _SnacWinklerForce_Construct, 
			     _Codelet_Build, 
			     _Codelet_Initialise, 
			     _Codelet_Execute, 
			     _Codelet_Destroy, 
			     name );
}


void _SnacWinklerForce_Construct( void* component, Stg_ComponentFactory* cf, void* data ) {
	Snac_Context*	context;
	Dictionary*			meshDict;
	int Spherical = 0;
	Dictionary_Entry_Value* extensionsList;
	Dictionary_Entry_Value* extension;

	/* Retrieve context. */
	context = (Snac_Context*)Stg_ComponentFactory_ConstructByName( cf, "context", Snac_Context, True, data ); 

	#ifdef DEBUG
		printf( "In %s()\n", __func__ );
	#endif

	/* Add extensions to nodes, elements and the context */

	/* Add extensions to the entry points */
	assert( context->dictionary );
	extensionsList = Dictionary_Get( context->dictionary, "extensions" );
	if(!extensionsList)
		extensionsList = Dictionary_Get( context->dictionary, "plugins" );
	extension = Dictionary_Entry_Value_GetFirstElement(extensionsList);
	while ( extension ) {
		if ( 0 == strcmp( Dictionary_Entry_Value_AsString( extension ),
				  "SnacSpherical" ) ) {
			Spherical = 1;
			break;
		}
		extension = extension->next;
	}
	/*ccccc*/
	if(Spherical) {
		meshDict = Dictionary_Entry_Value_AsDictionary( Dictionary_Get( context->dictionary, MESH_STR ) );
		if( meshDict ) {
			if( !Dictionary_Get( meshDict, RMIN_STR ) ) {
				printf( "Warning: No \"%s\" entry in \"%s\", defaulting to %g\n", RMIN_STR, MESH_STR, MIN[2] );
			}
			Spherical_RMin = Dictionary_Entry_Value_AsDouble( Dictionary_GetDefault( meshDict, RMIN_STR, Dictionary_Entry_Value_FromDouble( MIN[2] ) ) );
			Spherical_RMax = Dictionary_Entry_Value_AsDouble( Dictionary_GetDefault( meshDict, RMAX_STR, Dictionary_Entry_Value_FromDouble( MAX[2] ) ) );
		}
		EntryPoint_Append(
				  Context_GetEntryPoint( context, Snac_EP_Force ),
				  SnacWinklerForce_Type,
				  _SnacWinklerForce_Apply_Spherical,
				  SnacWinklerForce_Type );
	}
	else {
		EntryPoint_Append(
				  Context_GetEntryPoint( context, Snac_EP_Force ),
				  SnacWinklerForce_Type,
				  _SnacWinklerForce_Apply,
				  SnacWinklerForce_Type );
	}
}

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
** $Id: Register.c 3140 2005-08-30 18:35:09Z EunseoChoi $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "Register.h"
#include "ConstructExtensions.h"
#include "VariableConditions.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

/* Textual name of this class */
const Type SnacHydroStaticIC_Type = "SnacHydroStaticIC";
static char RMIN_STR[] = "rMin";
static char RMAX_STR[] = "rMax";
static char MESH_STR[] = "mesh";
static double MIN[] = { -45.0f, -45.0f, 0.5f };
static double MAX[] = { 45.0f, 45.0f, 1.0f };
/* First-time definition of these extern variables (Force.h) */
/* The proper values are asgined in _SnacWinklerForce_Construct. */
double Spherical_RMin = 0.0;
double Spherical_RMax = 0.0;

Index _SnacHydroStaticIC_Register( PluginsManager* pluginsMgr ) {
	return PluginsManager_Submit( pluginsMgr,
				      SnacHydroStaticIC_Type,
				      "0",
				      _SnacHydroStaticIC_DefaultNew );
}

void* _SnacHydroStaticIC_DefaultNew( Name name ) {
	return _Codelet_New( sizeof(Codelet),
			     SnacHydroStaticIC_Type,
			     _Codelet_Delete,
			     _Codelet_Print,
			     _Codelet_Copy,
			     _SnacHydroStaticIC_DefaultNew,
			     _SnacHydroStaticIC_Construct,
			     _Codelet_Build,
			     _Codelet_Initialise,
			     _Codelet_Execute,
			     _Codelet_Destroy,
			     name );
}

void _SnacHydroStaticIC_Construct( void* component, Stg_ComponentFactory* cf, void* data ) {
	Snac_Context*				context;
	Dictionary*			meshDict;
	int Spherical = 0;
	Dictionary_Entry_Value* extensionsList;
	Dictionary_Entry_Value* extension;

	/* Retrieve context. */
	context = (Snac_Context*)Stg_ComponentFactory_ConstructByName( cf, "context", Snac_Context, True, data ); 

	#ifdef DEBUG
	printf( "In %s()\n", __func__ );
	#endif

	if( context->restartTimestep > 0 )
		return;

	/*ccccc*/
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
	if(Spherical) {
		meshDict = Dictionary_Entry_Value_AsDictionary( Dictionary_Get( context->dictionary, MESH_STR ) );
		if( meshDict ) {
			if( !Dictionary_Get( meshDict, RMIN_STR ) ) {
				printf( "Warning: No \"%s\" entry in \"%s\", defaulting to %g\n", RMIN_STR, MESH_STR, MIN[2] );
			}
			Spherical_RMin = Dictionary_Entry_Value_AsDouble( Dictionary_GetDefault( meshDict, RMIN_STR, Dictionary_Entry_Value_FromDouble( MIN[2] ) ) );
			Spherical_RMax = Dictionary_Entry_Value_AsDouble( Dictionary_GetDefault( meshDict, RMAX_STR, Dictionary_Entry_Value_FromDouble( MAX[2] ) ) );
		}
		EntryPoint_InsertBefore(
						  Context_GetEntryPoint( context, AbstractContext_EP_Initialise ),
						  "SnacTimeStepZero",
						  SnacHydroStaticIC_Type,
						  _SnacHydroStaticIC_IC_Spherical,
						  SnacHydroStaticIC_Type );
	}
	else {
		EntryPoint_InsertBefore(
						  Context_GetEntryPoint( context, AbstractContext_EP_Initialise ),
						  "SnacTimeStepZero",
						  SnacHydroStaticIC_Type,
						  _SnacHydroStaticIC_IC,
						  SnacHydroStaticIC_Type );
	}

	/* Construct. */
	_SnacHydroStaticIC_ConstructExtensions( context, data );
}

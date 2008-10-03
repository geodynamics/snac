/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003, Victorian Partnership for Advanced Computing (VPAC) Ltd, 110 Victoria Street, Melbourne, 3053, Australia.
**
** Authors:
**	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
**	Patrick D. Sunter, Software Engineer, VPAC. (pds@vpac.org)
**	Luke J. Hodkinson, Computational Engineer, VPAC. (lhodkins@vpac.org)
**	Siew-Ching Tan, Software Engineer, VPAC. (siew@vpac.org)
**	Alan H. Lo, Computational Engineer, VPAC. (alan@vpac.org)
**	Raquibul Hassan, Computational Engineer, VPAC. (raq@vpac.org)
**
**  This library is free software; you can redistribute it and/or
**  modify it under the terms of the GNU Lesser General Public
**  License as published by the Free Software Foundation; either
**  version 2.1 of the License, or (at your option) any later version.
**
**  This library is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
**  Lesser General Public License for more details.
**
**  You should have received a copy of the GNU Lesser General Public
**  License along with this library; if not, write to the Free Software
**  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
**
** $Id: LocalPlugin.c 3081 2005-07-08 08:24:05Z AlanLo $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"
#include "Base/Automation/Automation.h"
#include "Base/Extensibility/Extensibility.h"

#include "../../MockContext.h"

/* Plugins/Codelets need a Type string. */
const Type LocalPlugin_Type = "LocalPlugin";

/* Define the plugin's class */
typedef struct { __Codelet } LocalPlugin;


/* This is what the plugin does */
void LocalPlugin_Function( void* context ) {
	printf( "%s\n", __func__ );
}


/* Plugins are components, so they need a Construct() function. This is where you add the entry points and extensions */
void _LocalPlugin_Construct( void* component, Stg_ComponentFactory* cf, void* data ) {
	MockContext* context = (MockContext*)Stg_ComponentFactory_ConstructByName(cf, "context", Stg_Component, True, data );

	EP_Append( context->ep, LocalPlugin_Function );
}

/* Plugins are compnents, so they need a DefaultNew() function. */
void* _LocalPlugin_DefaultNew( Name name ) {
	return Codelet_New(
		LocalPlugin_Type,
		_LocalPlugin_DefaultNew,
		_LocalPlugin_Construct,
		_Codelet_Build,
		_Codelet_Initialise,
		_Codelet_Execute,
		_Codelet_Destroy,
		name );
}

/* Plugins require a _Register() function for the plugin to be loaded. */

Index LocalPlugin_Register( PluginsManager* pluginsManager ) {
	return PluginsManager_Submit( pluginsManager, LocalPlugin_Type, "0", _LocalPlugin_DefaultNew );
}


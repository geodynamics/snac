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
** $Id: Plugins.c 3081 2005-07-08 08:24:05Z AlanLo $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"
#include "Base/Automation/Automation.h"
#include "Base/Extensibility/Extensibility.h"

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#include "MockContext.h"

const Type MockContext_Type = "MockContext";

void* _MockContext_DefaultNew( Name name ) {
	return NULL;
}
void _MockContext_Construct( void* c, Stg_ComponentFactory* cf, void* data ) {

}
void _MockContext_Build( void* c, void* data ) {

}
void _MockContext_Initialise( void* c, void* data ) {

}
void _MockContext_Execute( void* c, void* data ) {

}
void _MockContext_Destroy( void* c, void* data ) {

}

MockContext* MockContext_New( Dictionary* dictionary ) {
	MockContext* self;

	self = (MockContext*) _Stg_Component_New(
		sizeof( MockContext ),
		"MockContext",
		_MockContext_Delete,
		NULL,
		NULL,
		_MockContext_DefaultNew,
		_MockContext_Construct,
		_MockContext_Build,
		_MockContext_Initialise,
		_MockContext_Execute,
		_MockContext_Destroy,
		"context",
		NON_GLOBAL );

	self->plugins = PluginsManager_New( dictionary );
	PluginsManager_AddDirectory( self->plugins, "StGermain", LIB_DIR );
	self->ep = EntryPoint_New( "TestEP", EntryPoint_VoidPtr_CastType );
	self->em = ExtensionManager_New_OfExistingObject( "Context", self );

	return self;
}

void _MockContext_Delete( void* context ) {
	MockContext* self = (MockContext*)context;

	Stg_Class_Delete( self->plugins );
	Stg_Class_Delete( self->ep );
	Stg_Class_Delete( self->em );
	
	_Stg_Class_Delete( self );
}


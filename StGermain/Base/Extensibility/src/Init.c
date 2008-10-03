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
** $Id: Init.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdarg.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"
#include "Base/Automation/Automation.h"

#include "types.h"
#include "shortcuts.h"
#include "EntryPoint.h"
#include "EntryPoint_Register.h"
#include "ExtensionInfo.h"
#include "ClassPtrExtensionInfo.h"
#include "ExtensionManager.h"
#include "ExtensionManager_Register.h"
#include "Hook.h"
#include "ClassHook.h"
#include "PluginLoader.h"
#include "PluginsManager.h"
#include "SimpleExtensionInfo.h"
#include "Init.h"
#include "Finalise.h"

#include <stdio.h>

Bool BaseExtensibility_Init( int* argc, char** argv[] ) {
	Journal_Printf( Journal_Register( DebugStream_Type, "Context" ), "In: %s\n", __func__ ); /* DO NOT CHANGE OR REMOVE */

	RegisterParent( Hook_Type,                      Stg_Object_Type );
	RegisterParent( ClassHook_Type,                 Hook_Type );

	RegisterParent( EntryPoint_Type,                Stg_Object_Type );
	RegisterParent( EntryPoint_Register_Type,       Stg_Class_Type );
	
	RegisterParent( ExtensionManager_Type,          Stg_Object_Type );
	RegisterParent( ExtensionManager_Register_Type, Stg_Class_Type );

	RegisterParent( ExtensionInfo_Type,             Stg_Object_Type );
	RegisterParent( SimpleExtensionInfo_Type,       ExtensionInfo_Type );
	RegisterParent( ClassPtrExtensionInfo_Type,     ExtensionInfo_Type );

	RegisterParent( PluginLoader_Type,                    Stg_Object_Type );
	RegisterParent( PluginsManager_Type,            Stg_Class_Type );
	
	return True;
}

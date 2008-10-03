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
*/
/** \file
**  Role:
**	Basic framework types.
**
** Assumptions:
**	None as yet.
**
** Comments:
**	None as yet.
**
** $Id: types.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Base_Extensibility_types_h__
#define __Base_Extensibility_types_h__
	
	/* Generic types */
	/* Base types/classes */
	typedef struct Hook				Hook;
	typedef struct ClassHook			ClassHook;
	typedef struct EntryPoint 			EntryPoint;
	typedef struct ExtensionInfo			ExtensionInfo;
	typedef struct SimpleExtensionInfo		SimpleExtensionInfo;
	typedef struct ClassPtrExtensionInfo		ClassPtrExtensionInfo;
	typedef struct ExtensionManager			ExtensionManager;
	typedef struct ExtensionManager_Register	ExtensionManager_Register;
	typedef struct EntryPoint_Register		EntryPoint_Register;
	typedef struct Codelet				Codelet;
	typedef struct PluginLoader			PluginLoader;
	typedef struct PluginsManager			PluginsManager;

	/* Extension types ... for readability */
	typedef Stg_ObjectList			ExtensionList;
	typedef Index				ExtensionManager_Index;
	typedef Stg_ObjectList			ExtensionInfoList;
	typedef Index				ExtensionInfo_Index;
	
	/* EntryPoint types ... for readability */
	typedef Stg_ObjectList			EntryPointList;
	typedef unsigned int			EntryPoint_Index;

	/* Hook types ... for readability */
	typedef void*				Func_Ptr;
	typedef Name				Hook_Name;
	typedef char*				Hook_AddedBy;
	typedef Hook*				Hook_Ptr;
	typedef Stg_ObjectList			HookList;
	typedef Index				Hook_Index;
	
	/* plugin types ... for readability */
	typedef void*				DLL_Handle;
	typedef DLL_Handle*			DLL_Handle_List;

#endif /* __Base_Extensibility_types_h__ */

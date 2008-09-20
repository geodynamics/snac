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
**	Shortcuts to complex types in the eyes of the Context user.
**
** Assumptions:
**	None as yet.
**
** Comments:
**	None as yet.
**
** $Id: shortcuts.h 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Base_Context_shortcuts_h__
#define __Base_Context_shortcuts_h__
	
	#define Context_PrintConcise		AbstractContext_PrintConcise
	#define Context_AddEntryPoint		AbstractContext_AddEntryPoint
	#define Context_GetEntryPoint		AbstractContext_GetEntryPoint
	#define Context_Call			AbstractContext_Call
	#define Context_KeyCall			AbstractContext_KeyCall
	#define Context_KeyHandle		AbstractContext_KeyHandle
	#define Context_Dt			AbstractContext_Dt
	#define Context_Step			AbstractContext_Step
	#define Context_WarnIfNoHooks		AbstractContext_WarnIfNoHooks
	#define Context_ErrorIfNoHooks		AbstractContext_ErrorIfNoHooks
	
	#define KeyCall				Context_KeyCall
	#define KeyHandle			Context_KeyHandle

	/* Shortcuts to other short cut Entry Point functions. */

	#define ContextEP_Prepend( context, epName, func ) \
		EP_Prepend( \
			Context_GetEntryPoint( context, epName ), \
			func )

	#define ContextEP_Prepend_AlwaysFirst( context, epName, func ) \
		EP_Prepend_AlwaysFirst( \
			Context_GetEntryPoint( context, epName ), \
			func )

	#define ContextEP_Append( context, epName, func ) \
		EP_Append( \
			Context_GetEntryPoint( context, epName ), \
			func )

	#define ContextEP_Append_AlwaysLast( context, epName, func ) \
		EP_Append_AlwaysLast( \
			Context_GetEntryPoint( context, epName ), \
			func )

	#define ContextEP_InsertBefore( context, epName, funcToInsertBefore, func ) \
		EP_InsertBefore( \
			Context_GetEntryPoint( context, epName ), \
			funcToInsertBefore, \
			func )
	
	#define ContextEP_InsertAfter( context, epName, funcToInsertAfter, func ) \
		EP_InsertAfter( \
			Context_GetEntryPoint( context, epName ), \
			funcToInsertAfter, \
			func )

	#define ContextEP_Remove( context, epName, reference )\
		EP_Remove( \
			Context_GetEntryPoint( context, epName ), \
			reference )
	
	#define ContextEP_Replace( context, epName, funcToReplace, func ) \
		EP_Replace( \
			Context_GetEntryPoint( context, epName ), \
			funcToReplace, \
			func )

	#define ContextEP_ReplaceAll( context, epName, func ) \
		EP_ReplaceAll( \
			Context_GetEntryPoint( context, epName ), \
			func )

	#define ContextEP_Purge( context, epName ) \
		EP_Purge( Context_GetEntryPoint( context, epName ) )

#endif /* __Base_Context_shortcuts_h__ */

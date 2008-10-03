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
** $Id: ContextEntryPoint.c 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdarg.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"
#include "Base/Automation/Automation.h"
#include "Base/Extensibility/Extensibility.h"

#include "units.h"
#include "types.h"
#include "shortcuts.h"
#include "ContextEntryPoint.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>

/* Textual name of this class */
const Type ContextEntryPoint_Type = "ContextEntryPoint";


ContextEntryPoint* ContextEntryPoint_New( const Name name, unsigned int castType ) {
	return _ContextEntryPoint_New( sizeof(ContextEntryPoint), ContextEntryPoint_Type, _EntryPoint_Delete, 
		_EntryPoint_Print, NULL, _ContextEntryPoint_GetRun, name, castType );
}

void ContextEntryPoint_Init( void* contextEntryPoint, Name name, unsigned int castType ) {
	ContextEntryPoint* self = (ContextEntryPoint*)contextEntryPoint;
	
	/* General info */
	self->type = ContextEntryPoint_Type;
	self->_sizeOfSelf = sizeof(ContextEntryPoint);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _EntryPoint_Delete;
	self->_print = _EntryPoint_Print;
	self->_copy = NULL;
	self->_getRun = _ContextEntryPoint_GetRun;
	_Stg_Class_Init( (Stg_Class*)self );
	_Stg_Object_Init( (Stg_Object*)self, name, GLOBAL );
	_EntryPoint_Init( (EntryPoint*)self, castType );
	
	/* ContextEntryPoint info */
	_ContextEntryPoint_Init( self );
}

ContextEntryPoint* _ContextEntryPoint_New( 
		SizeT				_sizeOfSelf,
		Type				type,
		Stg_Class_DeleteFunction*		_delete,
		Stg_Class_PrintFunction*		_print,
		Stg_Class_CopyFunction*		_copy, 
		EntryPoint_GetRunFunction*	_getRun,
		Name				name,
		unsigned int			castType )
{
	ContextEntryPoint* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(ContextEntryPoint) );
	self = (ContextEntryPoint*)_EntryPoint_New( _sizeOfSelf, type, _delete, _print, _copy, 
		_getRun, name, castType );
	
	/* General info */
	
	/* Virtual info */
	
	/* ContextEntryPoint info */
	_ContextEntryPoint_Init( self );
	
	return self;
}

void _ContextEntryPoint_Init( ContextEntryPoint* self ) {
	/* General and Virtual info should already be set */
	
	/* ContextEntryPoint info */
}


Func_Ptr _ContextEntryPoint_GetRun( void* contextEntryPoint ) {
	ContextEntryPoint* self = (ContextEntryPoint*)contextEntryPoint;
	
	switch( self->castType ) {
		case ContextEntryPoint_Dt_CastType:
			return (Func_Ptr)_ContextEntryPoint_Run_Dt;
		
		case ContextEntryPoint_Step_CastType:
			return (Func_Ptr)_ContextEntryPoint_Run_Step;
		
		default:
			return (Func_Ptr)_EntryPoint_GetRun( self );
	}
}


double _ContextEntryPoint_Run_Dt( void* contextEntryPoint, void* data0 ) {
	ContextEntryPoint* self = (ContextEntryPoint*)contextEntryPoint;
	Hook_Index hookIndex;
	double result = 0.0;
	
	#ifdef USE_PROFILE
		Stg_CallGraph_Push( stgCallGraph, _ContextEntryPoint_Run_Dt, self->name );
	#endif

	for( hookIndex = 0; hookIndex < self->hooks->count; hookIndex++ ) {
		result = ((ContextEntryPoint_Dt_Cast*)((Hook*)self->hooks->data[hookIndex])->funcPtr)( data0 );
	}
	
	#ifdef USE_PROFILE
		Stg_CallGraph_Pop( stgCallGraph );
	#endif

	return result;
}

void _ContextEntryPoint_Run_Step( void* contextEntryPoint, void* data0, double data1 ) {
	ContextEntryPoint* self = (ContextEntryPoint*)contextEntryPoint;
	Hook_Index hookIndex;
	
	#ifdef USE_PROFILE
		Stg_CallGraph_Push( stgCallGraph, _ContextEntryPoint_Run_Step, self->name );
	#endif

	for( hookIndex = 0; hookIndex < self->hooks->count; hookIndex++ ) {
		((ContextEntryPoint_Step_Cast*)((Hook*)self->hooks->data[hookIndex])->funcPtr)( data0, data1 );
	}

	#ifdef USE_PROFILE
		Stg_CallGraph_Pop( stgCallGraph );
	#endif
}


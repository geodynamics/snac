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
** $Id: ConditionFunction_Register.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdarg.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"

#include "types.h"
#include "ConditionFunction.h"
#include "ConditionFunction_Register.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>


const Type ConditionFunction_Register_Type = "ConditionFunction_Register";


/*--------------------------------------------------------------------------------------------------------------------------
** Constructor
*/

ConditionFunction_Register* ConditionFunction_Register_New(void)
{ 
	return _ConditionFunction_Register_New( sizeof(ConditionFunction_Register), ConditionFunction_Register_Type, 
		_ConditionFunction_Register_Delete, _ConditionFunction_Register_Print, NULL);
}


void ConditionFunction_Register_Init(ConditionFunction_Register* self)
{ 
	/* General info */
	self->type = ConditionFunction_Register_Type;
	self->_sizeOfSelf = sizeof(ConditionFunction_Register);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _ConditionFunction_Register_Delete;
	self->_print = _ConditionFunction_Register_Print;
	self->_copy = NULL;
	
	_Stg_Class_Init((Stg_Class*)self);
	
	/* Stg_Class info */
	_ConditionFunction_Register_Init(self);
}


ConditionFunction_Register* _ConditionFunction_Register_New( 
				SizeT			_sizeOfSelf,
				Type			type,
				Stg_Class_DeleteFunction*	_delete,
				Stg_Class_PrintFunction*	_print, 
				Stg_Class_CopyFunction*	_copy )
{
	ConditionFunction_Register*	self;
	
	/* Allocate memory */
	self = (ConditionFunction_Register*)_Stg_Class_New(_sizeOfSelf, type, _delete, _print, _copy);
	
	/* Virtual info */
	
	/* ConditionFunction_Register info */
	_ConditionFunction_Register_Init(self);
	
	return self;
}


void _ConditionFunction_Register_Init(void* conditionFunction_Register)
{ 
	ConditionFunction_Register*	self = (ConditionFunction_Register*)conditionFunction_Register;
	
	self->count = 0;
	self->_size = 8;
	self->_delta = 8;
	self->_cf = Memory_Alloc_Array( ConditionFunction*, self->_size, "ConditionFunction_Register->_cf" );
	memset(self->_cf, 0, sizeof(ConditionFunction*)*self->_size);
}


/*--------------------------------------------------------------------------------------------------------------------------
** General virtual functions
*/

void _ConditionFunction_Register_Delete(void* conditionFunction_Register)
{
	ConditionFunction_Register*	self = (ConditionFunction_Register*)conditionFunction_Register;
	
	if (self->_cf) Memory_Free(self->_cf);
		
	/* Stg_Class_Delete parent class */
	_Stg_Class_Delete(self);
}


void _ConditionFunction_Register_Print(void* conditionFunction_Register, Stream* stream)
{
	ConditionFunction_Register*	self = (ConditionFunction_Register*)conditionFunction_Register;
	
	/* Set the Journal for printing informations */
	Stream* conditionFunction_RegisterStream = stream;
	
	/* General info */
	Journal_Printf( conditionFunction_RegisterStream, "ConditionFunction_Register (ptr): %p\n", self);
	
	/* Virtual info */
	
	/* ConditionFunction_Register info */
	Journal_Printf( conditionFunction_RegisterStream, "\tcount: %u\n", self->count);
	Journal_Printf( conditionFunction_RegisterStream, "\t_size: %lu\n", self->_size);
	Journal_Printf( conditionFunction_RegisterStream, "\t_delta: %lu\n", self->_delta);
	Journal_Printf( conditionFunction_RegisterStream, "\t_cf (ptr): %p\n", self->_cf);
	if (self->_cf)
	{
		ConditionFunction_Index	cf_I;
		
		for (cf_I = 0; cf_I < self->count; cf_I++)
			Journal_Printf( conditionFunction_RegisterStream, "\t\t_cf[%u]: %p\n", cf_I, self->_cf[cf_I]);
	}
	
	/* Parent class info */
	_Stg_Class_Print( self, conditionFunction_RegisterStream );
}


/*--------------------------------------------------------------------------------------------------------------------------
** Macros
*/


/*--------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/


/*--------------------------------------------------------------------------------------------------------------------------
** Build functions
*/

ConditionFunction_Index ConditionFunction_Register_Add(void* conditionFunction_Register, ConditionFunction* cf)
{
	ConditionFunction_Register*	self = (ConditionFunction_Register*)conditionFunction_Register;
	Variable_Index			handle;
	
	if (self->count >= self->_size)
	{
		SizeT currentSize = self->_size;
		
		/* Note: why is realloc not used here? */
		self->_size += self->_delta;
		
		self->_cf = Memory_Realloc_Array( self->_cf, ConditionFunction*, self->_size );
		memset( (Pointer)((ArithPointer)self->_cf + (sizeof(ConditionFunction*) * currentSize) ),
			0, sizeof(ConditionFunction*) * (self->_size - currentSize) );
	}
	
	handle = self->count++;
	self->_cf[handle] = cf;
	
	return handle;
}


/*--------------------------------------------------------------------------------------------------------------------------
** Functions
*/

ConditionFunction_Index ConditionFunction_Register_GetIndex(void* conditionFunction_Register, Name name)
{
	ConditionFunction_Register*	self = (ConditionFunction_Register*)conditionFunction_Register;
	ConditionFunction_Index		cf_I;
	
	for (cf_I = 0; cf_I < self->count; cf_I++)
	{
		if (self->_cf[cf_I]->name && !strcmp(name, self->_cf[cf_I]->name))
			return cf_I;
	}
	
	return (ConditionFunction_Index)-1;
}


void ConditionFunction_Register_PrintNameOfEachFunc( void* conditionFunction_Register, Stream* stream ) {
	ConditionFunction_Register*	self = (ConditionFunction_Register*)conditionFunction_Register;
	ConditionFunction_Index		cf_I;
	
	for (cf_I = 0; cf_I < self->count; cf_I++)
	{
		Journal_Printf( stream, "\"%s\", ", self->_cf[cf_I]->name );
	}
}

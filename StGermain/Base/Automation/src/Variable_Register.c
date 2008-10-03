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
** $Id: Variable_Register.c 3879 2006-10-26 01:32:33Z KathleenHumble $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdarg.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"

#include "types.h"
#include "shortcuts.h"
#include "Stg_Component.h"
#include "Variable.h"
#include "Variable_Register.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>


const Type Variable_Register_Type = "Variable_Register";


/*--------------------------------------------------------------------------------------------------------------------------
** Constructor
*/

Variable_Register* Variable_Register_New(void)
{ 
	return _Variable_Register_New( sizeof(Variable_Register), Variable_Register_Type, _Variable_Register_Delete,
		_Variable_Register_Print, _Variable_Register_Copy );
}


void Variable_Register_Init(Variable_Register* self)
{ 
	/* General info */
	self->type = Variable_Register_Type;
	self->_sizeOfSelf = sizeof(Variable_Register);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _Variable_Register_Delete;
	self->_print = _Variable_Register_Print;
	self->_copy = _Variable_Register_Copy;
	
	_Stg_Class_Init((Stg_Class*)self);
	
	/* Stg_Class info */
	_Variable_Register_Init(self);
}


Variable_Register* _Variable_Register_New( 
			SizeT			_sizeOfSelf,
			Type			type,
			Stg_Class_DeleteFunction*	_delete,
			Stg_Class_PrintFunction*	_print, 
			Stg_Class_CopyFunction*	_copy )
{
	Variable_Register*	self;
	
	/* Allocate memory */
	self = (Variable_Register*)_Stg_Class_New(_sizeOfSelf, type, _delete, _print, _copy);
	
	/* Virtual info */
	
	/* Stg_Class info */
	_Variable_Register_Init(self);
	
	return self;
}


void _Variable_Register_Init(void* variable_Register)
{ 
	Variable_Register*	self = (Variable_Register*)variable_Register;
	
	self->count = 0;
	self->_size = 8;
	self->_delta = 8;
	self->_variable = Memory_Alloc_Array( Variable*, self->_size, "Variable_Register->_variable" );
	memset(self->_variable, 0, sizeof(Variable*)*self->_size);
}


/*--------------------------------------------------------------------------------------------------------------------------
** General virtual functions
*/

void _Variable_Register_Delete(void* variable_Register)
{
	Variable_Register*	self = (Variable_Register*)variable_Register;
	
	if (self->_variable) Memory_Free(self->_variable);

	/* Stg_Class_Delete parent class */
	_Stg_Class_Delete(self);
}

void _Variable_Register_Print(void* variable_Register, Stream* stream)
{
	Variable_Register*	self = (Variable_Register*)variable_Register;
	
	/* Set the Journal for printing informations */
	Stream* variable_RegisterStream = Journal_Register( InfoStream_Type,	"Variable_RegisterStream");
	
	/* General info */
	Journal_Printf( variable_RegisterStream, "Variable_Register (ptr): %p\n", self);
	
	/* Virtual info */
	
	/* Stg_Class info */
	Journal_Printf( variable_RegisterStream, "\tcount: %u\n", self->count);
	Journal_Printf( variable_RegisterStream, "\t_size: %lu\n", self->_size);
	Journal_Printf( variable_RegisterStream, "\t_delta: %lu\n", self->_delta);
	Journal_Printf( variable_RegisterStream, "\t_variable (ptr): %p\n", self->_variable);
	if (self->_variable)
	{
		Variable_Index	var_I;
		
		for (var_I = 0; var_I < self->count; var_I++)
			Print(self->_variable[var_I], stream);
	}
	
	/* Print parent */
	_Stg_Class_Print(self, variable_RegisterStream);
}

void* _Variable_Register_Copy( void* vr, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {

	Variable_Register* self = (Variable_Register*)vr;
	Variable_Register* newVariableRegister;
	int ii;

	newVariableRegister = (Variable_Register*)_Stg_Class_Copy( self, dest, deep, nameExt, ptrMap );
	PtrMap_Append( ptrMap, self, newVariableRegister );

	newVariableRegister->count = self->count;
	newVariableRegister->_size = self->_size;
	newVariableRegister->_variable = Memory_Alloc_Array( Variable*, self->_size, "Variable_Register->_variable" );
	memset(newVariableRegister->_variable, 0, sizeof(Variable*)*self->_size);

	for ( ii = 0; ii < self->count; ++ii ) {
		if ( (newVariableRegister->_variable[ii] = PtrMap_Find( ptrMap, self->_variable[ii] )) == NULL ) {
			newVariableRegister->_variable[ii] = Stg_Class_Copy( self->_variable[ii], NULL, deep, nameExt, ptrMap );
		}
	}

	return newVariableRegister;
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

Variable_Index Variable_Register_Add(void* variable_Register, Variable* variable)
{
	Variable_Register*	self = (Variable_Register*)variable_Register;
	Variable_Index		handle;
	
	if (self->count >= self->_size)
	{
		SizeT currentSize = self->_size;

		self->_size += self->_delta;
		self->_variable = Memory_Realloc_Array( self->_variable, Variable*, self->_size );
		memset( (Pointer)((ArithPointer)self->_variable + (currentSize * sizeof(Variable*)) ), 0, 
			sizeof(Variable*) * (self->_size - currentSize) );
	}
	
	handle = self->count++;
	self->_variable[handle] = variable;

	return handle;
}


void Variable_Register_BuildAll(void* variable_Register)
{
	Variable_Register*	self = (Variable_Register*)variable_Register;
	Variable_Index		var_I;
	
	for (var_I = 0; var_I < self->count; var_I++)
		Build( self->_variable[var_I], 0, False );
}


/*--------------------------------------------------------------------------------------------------------------------------
** Functions
*/

Variable_Index Variable_Register_GetIndex(void* variable_Register, Name name)
{
	Variable_Register*	self = (Variable_Register*)variable_Register;
	Variable_Index		var_I;
	
	for (var_I = 0; var_I < self->count; var_I++)
	{
		if (self->_variable[var_I]->name && !strcmp(name, self->_variable[var_I]->name))
			return var_I;
	}
	
	return (Variable_Index)-1;
}


Variable* Variable_Register_GetByName(void* variable_Register, Name name)
{
	Variable_Register*	self = (Variable_Register*)variable_Register;
	Variable_Index		varIndex;
	
	if( (varIndex = Variable_Register_GetIndex( self, name )) != (Variable_Index)-1 )
		return self->_variable[varIndex];
	
	return NULL;
}


Variable* Variable_Register_GetByIndex(void* variable_Register, Variable_Index varIndex ) {
	Variable_Register*	self = (Variable_Register*)variable_Register;
	#if DEBUG
	Stream*			error = Journal_Register( ErrorStream_Type, self->type );
	
	Journal_DFirewall(  (varIndex < self->count), error,
		"Error: Given variable index %d not between 0 and variable count %d.\n",
		varIndex, self->count );
	#endif
		
	return  self->_variable[varIndex];
}

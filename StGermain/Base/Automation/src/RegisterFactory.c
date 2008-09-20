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
** $Id: VariableDumpStream.c 789 2004-02-10 17:15:11Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"
                                                                                                                                    
#include "types.h"
#include "RegisterFactory.h"

#include <assert.h>

const Type RegisterFactory_Type = "RegisterFactory";
const Type RegisterFactory_RegisterConstructorFunction_Type = "RegisterFactory_RegisterConstructorFunction";

RegisterFactory_RegisterConstructor* RegisterFactory_RegisterConstructor_New(
	Name						name,
	RegisterFactory_RegisterConstructorFunction*	_constructor )
{
	return _RegisterFactory_RegisterConstructor_New(
		sizeof(RegisterFactory_RegisterConstructor),
		RegisterFactory_RegisterConstructorFunction_Type,
		_Stg_Object_Delete,
		_Stg_Object_Print,
		NULL, 
		name,
		NON_GLOBAL,
		_constructor );
}

RegisterFactory_RegisterConstructor* _RegisterFactory_RegisterConstructor_New(
	SizeT						_sizeOfSelf,
	Type						type,
	Stg_Class_DeleteFunction*				_delete,
	Stg_Class_PrintFunction*				_print,
	Stg_Class_CopyFunction*				_copy, 
	Name						name,
	AllocationType					nameAllocationType,
	RegisterFactory_RegisterConstructorFunction*	_constructor )
{
	RegisterFactory_RegisterConstructor* self;

	assert( _sizeOfSelf >= sizeof(RegisterFactory_RegisterConstructor) );

	self = (RegisterFactory_RegisterConstructor*)_Stg_Object_New(
		_sizeOfSelf,
		type,
		_delete,
		_print,
		_copy, 
		name,
		nameAllocationType );
	
	self->_constructor = _constructor;

	return self;
}

RegisterFactory* RegisterFactory_New()
{
	return _RegisterFactory_New(
		sizeof(RegisterFactory),
		RegisterFactory_Type,
		_RegisterFactory_Delete,
		_RegisterFactory_Print,
		NULL );
}

RegisterFactory* _RegisterFactory_New(
	SizeT						_sizeOfSelf,
	Type						type,
	Stg_Class_DeleteFunction*				_delete,
	Stg_Class_PrintFunction*				_print,
	Stg_Class_CopyFunction*				_copy )
{
	RegisterFactory* self;

	assert ( _sizeOfSelf >= sizeof(RegisterFactory) );

	self = (RegisterFactory*)_Stg_Class_New( _sizeOfSelf, type, _delete, _print, _copy );

	_RegisterFactory_Init( self );

	return self;
}

void _RegisterFactory_Init( RegisterFactory* self ) {
	self->constructors = Stg_ObjectList_New();
}

void _RegisterFactory_Delete( void* factory ) {
	RegisterFactory* self = (RegisterFactory*)factory;

	Stg_ObjectList_DeleteAllObjects( self->constructors );
	Stg_Class_Delete( self->constructors );
	_Stg_Class_Delete( self );
}

void _RegisterFactory_Print( void* factory, Stream* stream ) {
	RegisterFactory* self = (RegisterFactory*)factory;
	
	Journal_Printf( stream, "RegisterFactory\n" );
	_Stg_Class_Print( self, stream );
	Print( self->constructors, stream );
}

void RegisterFactory_Register( void* factory, Name name, void* _constructorFunction )
{
	RegisterFactory* self = (RegisterFactory*)factory;
	
	Stg_ObjectList_Append( self->constructors,
		RegisterFactory_RegisterConstructor_New(
			name,
			(RegisterFactory_RegisterConstructorFunction*)_constructorFunction ) );
}

void* RegisterFactory_Create( void* factory, Name name )
{
	RegisterFactory* self = (RegisterFactory*)factory;
	RegisterFactory_RegisterConstructor* constructor;
	RegisterFactory_RegisterConstructorFunction* constructorFunc;
	

	constructor = (RegisterFactory_RegisterConstructor*)Stg_ObjectList_Get( self->constructors, name );
	constructorFunc = (RegisterFactory_RegisterConstructorFunction*)constructor->_constructor;

	return (constructorFunc)();
}







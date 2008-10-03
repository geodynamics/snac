
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
** $Id: ObjectAdaptor.c 2136 2004-09-30 02:47:13Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdarg.h>
#include "types.h"
#include "forwardDecl.h"

#include "Memory.h"
#include "Class.h"
#include "Object.h"
#include "PrimitiveObject.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>


const Type Stg_PrimitiveObject_Type = "Stg_PrimitiveObject";

Stg_PrimitiveObject* Stg_PrimitiveObject_New_UnsignedChar( unsigned char value, Name name ) {
	Stg_C_Primitive v;
	v.asUnsignedChar = value;
	return _Stg_PrimitiveObject_New( Stg_C_Primitive_Type_UnsignedChar, v, name );
}
Stg_PrimitiveObject* Stg_PrimitiveObject_New_UnsignedShort( unsigned short value, Name name ) {
	Stg_C_Primitive v;
	v.asUnsignedShort = value;
	return _Stg_PrimitiveObject_New( Stg_C_Primitive_Type_UnsignedShort, v, name );
}
Stg_PrimitiveObject* Stg_PrimitiveObject_New_UnsignedInt( unsigned int value, Name name ) {
	Stg_C_Primitive v;
	v.asUnsignedInt = value;
	return _Stg_PrimitiveObject_New( Stg_C_Primitive_Type_UnsignedInt, v, name );
}
Stg_PrimitiveObject* Stg_PrimitiveObject_New_UnsignedLong( unsigned long value, Name name ) {
	Stg_C_Primitive v;
	v.asUnsignedLong = value;
	return _Stg_PrimitiveObject_New( Stg_C_Primitive_Type_UnsignedLong, v, name );
}
Stg_PrimitiveObject* Stg_PrimitiveObject_New_Char( char value, Name name ) {
	Stg_C_Primitive v;
	v.asChar = value;
	return _Stg_PrimitiveObject_New( Stg_C_Primitive_Type_Char, v, name );
}
Stg_PrimitiveObject* Stg_PrimitiveObject_New_Short( short value, Name name ) {
	Stg_C_Primitive v;
	v.asShort = value;
	return _Stg_PrimitiveObject_New( Stg_C_Primitive_Type_Short, v, name );
}
Stg_PrimitiveObject* Stg_PrimitiveObject_New_Int( int value, Name name ) {
	Stg_C_Primitive v;
	v.asInt = value;
	return _Stg_PrimitiveObject_New( Stg_C_Primitive_Type_Int, v, name );
}
Stg_PrimitiveObject* Stg_PrimitiveObject_New_Long( long value, Name name ) {
	Stg_C_Primitive v;
	v.asLong = value;
	return _Stg_PrimitiveObject_New( Stg_C_Primitive_Type_Long, v, name );
}
Stg_PrimitiveObject* Stg_PrimitiveObject_New_Float( float value, Name name ) {
	Stg_C_Primitive v;
	v.asFloat = value;
	return _Stg_PrimitiveObject_New( Stg_C_Primitive_Type_Float, v, name );
}
Stg_PrimitiveObject* Stg_PrimitiveObject_New_Double( double value, Name name ) {
	Stg_C_Primitive v;
	v.asDouble = value;
	return _Stg_PrimitiveObject_New( Stg_C_Primitive_Type_Double, v, name );
}

Stg_PrimitiveObject* _Stg_PrimitiveObject_New( 
	Stg_C_Primitive_Type	dataType,
	Stg_C_Primitive		value,
	Name			name )
{
	Stg_PrimitiveObject*  result;

	result = (Stg_PrimitiveObject*)_Stg_Object_New(
			sizeof(Stg_PrimitiveObject),
			Stg_PrimitiveObject_Type,
			_Stg_PrimitiveObject_Delete,
			_Stg_PrimitiveObject_Print,
			_Stg_PrimitiveObject_Copy,
			name,
			NON_GLOBAL );
	
	_Stg_PrimitiveObject_Init( result, dataType, value );

	return result;
}

void _Stg_PrimitiveObject_Init( 
	Stg_PrimitiveObject*	self, 
	Stg_C_Primitive_Type	dataType,
	Stg_C_Primitive		value )
{
	self->dataType = dataType;
	self->value = value;
}


void _Stg_PrimitiveObject_Delete( void* primitive ) {
	_Stg_Object_Delete( primitive );
}

void _Stg_PrimitiveObject_Print( void* primitive, struct Stream* stream ) {
	Stg_PrimitiveObject* self = (Stg_PrimitiveObject*)primitive;
	char* typeString;

	switch( self->dataType ) {
		case Stg_C_Primitive_Type_UnsignedChar:
			typeString = "unsigned char";
			break;
		case Stg_C_Primitive_Type_UnsignedShort:
			typeString = "unsigned short";
			break;
		case Stg_C_Primitive_Type_UnsignedInt:
			typeString = "unsigned int";
			break;
		case Stg_C_Primitive_Type_UnsignedLong:
			typeString = "unsigned long";
			break;
		case Stg_C_Primitive_Type_Char:
			typeString = "char";
			break;
		case Stg_C_Primitive_Type_Short:
			typeString = "short";
			break;
		case Stg_C_Primitive_Type_Int:
			typeString = "int";
			break;
		case Stg_C_Primitive_Type_Long:
			typeString = "long";
			break;
		case Stg_C_Primitive_Type_Float:
			typeString = "float";
			break;
		case Stg_C_Primitive_Type_Double:
			typeString = "double";
			break;
		default:
			typeString = "";
			break;
	}
	
	Journal_Printf( stream, "Primitive Object (ptr): %p\n", (void*)self );
	Stream_Indent( stream );

	Journal_Printf( stream, "%s %s = ", typeString, self->name );

	switch ( self->dataType ) {
		case Stg_C_Primitive_Type_UnsignedChar:
		case Stg_C_Primitive_Type_Char: 
			Journal_Printf( stream, "'%c'", self->value.asChar );
			break;
		case Stg_C_Primitive_Type_UnsignedShort:
			Journal_Printf( stream, "%u", self->value.asUnsignedShort );
			break;
		case Stg_C_Primitive_Type_UnsignedInt:
			Journal_Printf( stream, "%u", self->value.asUnsignedInt );
			break;
		case Stg_C_Primitive_Type_UnsignedLong:
			Journal_Printf( stream, "%u", self->value.asUnsignedLong );
			break;
		case Stg_C_Primitive_Type_Short:
			Journal_Printf( stream, "%d", self->value.asShort );
			break;
		case Stg_C_Primitive_Type_Int:
			Journal_Printf( stream, "%d", self->value.asInt );
			break;
		case Stg_C_Primitive_Type_Long:
			Journal_Printf( stream, "%d", self->value.asLong );
			break;
		case Stg_C_Primitive_Type_Float:
			Journal_Printf( stream, "%.6ff", self->value.asFloat );
			break;
		case Stg_C_Primitive_Type_Double:
			Journal_Printf( stream, "%.6f", self->value.asDouble );
			break;
		default:
			break;
	}
	Journal_Printf( stream, ";\n" );
	
	Stream_UnIndent( stream );
}

void* _Stg_PrimitiveObject_Copy( void* primitive, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap ) {
	Stg_PrimitiveObject* self = (Stg_PrimitiveObject*)primitive;
	Stg_PrimitiveObject* newCopy;

	newCopy = (Stg_PrimitiveObject*)_Stg_Object_Copy( self, dest, deep, nameExt, ptrMap );

	newCopy->dataType = self->dataType;
	newCopy->value = self->value;

	return newCopy;
}
	


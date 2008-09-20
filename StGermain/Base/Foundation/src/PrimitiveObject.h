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
**     A wrapper class for C primitives for object list
**
** Assumptions:
**
** Comments:
**     Has large memory overhead so do not use in large arrays or lists.
**
** $Id: ObjectAdaptor.h 2406 2004-12-09 01:20:49Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Base_Foundation_PrimitiveObject_h__
#define __Base_Foundation_PrimitiveObject_h__
	
	extern const Type Stg_PrimitiveObject_Type;
	
	typedef enum {
		Stg_C_Primitive_Type_UnsignedChar = 0,
		Stg_C_Primitive_Type_UnsignedShort,
		Stg_C_Primitive_Type_UnsignedInt,
		Stg_C_Primitive_Type_UnsignedLong,
		Stg_C_Primitive_Type_Char,
		Stg_C_Primitive_Type_Short,
		Stg_C_Primitive_Type_Int,
		Stg_C_Primitive_Type_Long,
		Stg_C_Primitive_Type_Float,
		Stg_C_Primitive_Type_Double,
		Stg_C_Primitive_Num_Type
	} Stg_C_Primitive_Type;

	typedef union {
		unsigned char	asUnsignedChar;
		unsigned short	asUnsignedShort;
		unsigned int	asUnsignedInt;
		unsigned long	asUnsignedLong;
		char		asChar;
		short		asShort;
		int		asInt;
		long		asLong;
		float		asFloat;
		double		asDouble;
	} Stg_C_Primitive;
	
	/** Stg_PrimitiveObject class */
	#define __Stg_PrimitiveObject \
		__Stg_Object \
		Stg_C_Primitive_Type dataType; \
		Stg_C_Primitive      value;
	
	/** Defines key information about a Stg_PrimitiveObject - see ObjectAdaptor.h. */
	struct _Stg_PrimitiveObject { __Stg_PrimitiveObject };	
 
	
	
	Stg_PrimitiveObject* Stg_PrimitiveObject_New_UnsignedChar( unsigned char value, Name name );
	Stg_PrimitiveObject* Stg_PrimitiveObject_New_UnsignedShort( unsigned short value, Name name );
	Stg_PrimitiveObject* Stg_PrimitiveObject_New_UnsignedInt( unsigned int value, Name name );
	Stg_PrimitiveObject* Stg_PrimitiveObject_New_UnsignedLong( unsigned long value, Name name );
	Stg_PrimitiveObject* Stg_PrimitiveObject_New_Char( char value, Name name );
	Stg_PrimitiveObject* Stg_PrimitiveObject_New_Short( short value, Name name );
	Stg_PrimitiveObject* Stg_PrimitiveObject_New_Int( int value, Name name );
	Stg_PrimitiveObject* Stg_PrimitiveObject_New_Long( long value, Name name );
	Stg_PrimitiveObject* Stg_PrimitiveObject_New_Float( float value, Name name );
	Stg_PrimitiveObject* Stg_PrimitiveObject_New_Double( double value, Name name );
	
	Stg_PrimitiveObject* _Stg_PrimitiveObject_New( 
		Stg_C_Primitive_Type	dataType,
		Stg_C_Primitive		value,
		Name			name );


	void _Stg_PrimitiveObject_Init( 
		Stg_PrimitiveObject*	self, 
		Stg_C_Primitive_Type	dataType,
		Stg_C_Primitive		value );
	
	
	/* Stg_Class_Delete() implementation */
	void _Stg_PrimitiveObject_Delete( void* objectAdaptor );
	
	/* Stg_Class_Print() implementation */
	void _Stg_PrimitiveObject_Print( void* objectAdaptor, struct Stream* stream );
	
	/* Stg_Class_Copy() implementation */
	void* _Stg_PrimitiveObject_Copy( void* objectAdaptor, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap );
		
	
#endif /* __Base_Foundation_PrimitiveObject_h__ */


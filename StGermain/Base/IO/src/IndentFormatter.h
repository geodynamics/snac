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
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/** \file
** <b>Role:</b>
**	Formats output such that every line is indented to an adjustable level.
**
** <b>Assumptions:</b>
**	None
**
** <b>Comments:</b>
**	None
**
** <b>Description:</b>
**
** $Id: IndentFormatter.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**/

#ifndef __Base_IO_IndentFormatter_h__
#define __Base_IO_IndentFormatter_h__


	/** Textual name for IndentFormatter class. */
	extern const Type IndentFormatter_Type;


	/** \def __IndentFormatter See IndentFormatter. */
	#define __IndentFormatter \
		__LineFormatter \
		Index	_indent; \
		char	_character;
	struct IndentFormatter { __IndentFormatter };


	/** Creates a new IndentFormatter. */
	StreamFormatter* IndentFormatter_New();

	/** Creates a new IndentFormatter which indents with the given char. */
	StreamFormatter* IndentFormatter_New2( char _character );

	/** Initialise a IndentFormatter. */
	void IndentFormatter_Init( IndentFormatter* self );


	/** Constructor interface. */
	IndentFormatter* _IndentFormatter_New(
		SizeT 				_sizeOfSelf,
		Type 				type,
		Stg_Class_DeleteFunction*		_delete,
		Stg_Class_PrintFunction*		_print,
		Stg_Class_CopyFunction*		_copy, 
		StreamFormatter_FormatFunction*	_format,
		char				_character );

	/** Init interface. */
	void _IndentFormatter_Init(
		IndentFormatter*		self,
		StreamFormatter_FormatFunction*	_format,
		char				_character );

	/** Stg_Class_Delete interface. */
	void _IndentFormatter_Delete( void* formatter );

	/** Print interface. */
	void _IndentFormatter_Print( void* formatter, Stream* stream );

	/** Copy implementation. */
	void* _IndentFormatter_Copy( void* indentFormatter, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap );


	/** Set the value of indentation. */
	void IndentFormatter_SetIndent( void* formatter, Index indent );

	/** Increase the level of indentation by 1. */
	void IndentFormatter_Increment( void* formatter );

	/** Decrease the level of indentation by 1. */
	void IndentFormatter_Decrement( void* formatter );

	/** Sets the level of indentation to zero. */
	void IndentFormatter_Zero( void* formatter);
	
	/** Sets the character to indent with */
	void IndentFormatter_SetCharacter( char _character );


#endif /* __Base_IO_IndentFormatter_h__ */

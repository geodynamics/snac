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
**	Shortcuts to functions in the IO module
**
** <b>Assumptions:</b>
**	None
**
** <b>Comments:</b>
**	None
**
** $Id: shortcuts.h 3669 2006-07-07 02:52:03Z PatrickSunter $
**
**/

#ifndef __Base_IO_shortcuts_h__
#define __Base_IO_shortcuts_h__

	#define Printf			Journal_Printf
	#define Write			Journal_Write
	#define Dump			Journal_Dump

	/* Macros to get things out of the dictionary more easily */
	#define Dictionary_GetUnsignedInt( dictionary, key ) \
		( Dictionary_Entry_Value_AsUnsignedInt( Dictionary_Get( dictionary, key ) ) )
	
	#define Dictionary_GetInt( dictionary, key ) \
		( Dictionary_Entry_Value_AsInt( Dictionary_Get( dictionary, key ) ) )
	
	#define Dictionary_GetDouble( dictionary, key ) \
		( Dictionary_Entry_Value_AsDouble( Dictionary_Get( dictionary, key ) ) )

	#define Dictionary_GetFloat( dictionary, key ) \
		( Dictionary_Entry_Value_AsFloat( Dictionary_Get( dictionary, key ) ) )

	#define Dictionary_GetUnsignedLong( dictionary, key ) \
		( Dictionary_Entry_Value_AsUnsignedLong( Dictionary_Get( dictionary, key ) ) )

	#define Dictionary_GetString( dictionary, key ) \
		( Dictionary_Entry_Value_AsString( Dictionary_Get( dictionary, key ) ) )

	#define Dictionary_GetBool( dictionary, key ) \
		( Dictionary_Entry_Value_AsBool( Dictionary_Get( dictionary, key ) ) )

	#define Dictionary_GetDictionary( dictionary, key ) \
		( Dictionary_Entry_Value_AsDictionary( Dictionary_Get( dictionary, key ) ) )

	/* shortcuts to easily get a value from the dictionary, using a default
	if not found, and converting it to the correct type */

	int	Dictionary_GetUnsignedInt_WithDefault( Dictionary* dictionary, Dictionary_Entry_Key key, const unsigned int defaultVal );
	
	int	Dictionary_GetInt_WithDefault( Dictionary* dictionary, Dictionary_Entry_Key key, const int defaultVal );

	double	Dictionary_GetDouble_WithDefault( Dictionary* dictionary, Dictionary_Entry_Key key, const double defaultVal );
	
	float	Dictionary_GetFloat_WithDefault( Dictionary* dictionary, Dictionary_Entry_Key key, const float defaultVal );
	
	Bool	Dictionary_GetBool_WithDefault( Dictionary* dictionary, Dictionary_Entry_Key key, const Bool defaultVal );

	char*	Dictionary_GetString_WithDefault( Dictionary* dictionary, Dictionary_Entry_Key key, const char* const defaultVal );

	/** Wrapper to Dictionary_GetString_WithDefault which creates a string using sprintf */
	char* Dictionary_GetString_WithPrintfDefault( Dictionary* dictionary, Dictionary_Entry_Key key, char* format, ... ) ;


	/** Redirects file and puts 'prependedPath' in front of the filename 
	 * e.g. Stream_RedirectFile_WithPrependedPath( stream, "./output", "file.dat" ) redirects to './output/file.dat' */
	Bool Stream_RedirectFile_WithPrependedPath( Stream* stream, char* prependedPath, char* filename ) ;
	Bool Stream_AppendFile_WithPrependedPath( Stream* stream, char* prependedPath, char* filename ) ;

	/** Prints a string with a certain maximum length - it truncates it in the middle with an ellipsis if it is too long */
	void Journal_PrintString_WithLength( Stream* stream, char* string, int length ) ;
		
	/* shortcuts for printing names and values */
	#define Journal_PrintValue( stream, value ) 	\
		Journal_Printf( stream, #value " = %.5g\n", (double) (value) )

	#define Journal_PrintDouble        Journal_PrintValue
	#define Journal_PrintFloat         Journal_PrintValue
	#define Journal_PrintUnsignedInt   Journal_PrintValue
	#define Journal_PrintInt           Journal_PrintValue

	#define Journal_PrintChar( stream, value ) 	\
		Journal_Printf( stream, #value " = %c\n", (char) value )
	#define Journal_PrintBool( stream, value ) 	\
		Journal_Printf( stream, #value " = %s\n", (value) ? "True" : "False" )
	#define Journal_PrintPointer( stream, value ) 	\
		Journal_Printf( stream, #value " = (ptr)%p\n", value )
	#define Journal_PrintString( stream, value ) 	\
		Journal_Printf( stream, #value " = %s\n", value )
	#define Journal_PrintArray( stream, array, count ) \
		do {	\
			Index journalPrintArray_array_I;                          \
			Journal_Printf( stream, #array " = { " ); \
			for ( journalPrintArray_array_I = 0 ; journalPrintArray_array_I < count - 1 ; journalPrintArray_array_I++ ) \
				Journal_Printf( stream, "%.5g, ", (double)array[ journalPrintArray_array_I ] ); \
			Journal_Printf( stream, "%.5g }\n", (double)array[ journalPrintArray_array_I ] ); \
		} while(0)

#endif /* __Base_IO_shortcuts_h__ */

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
** $Id: shortcuts.c 3669 2006-07-07 02:52:03Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "Base/Foundation/Foundation.h"

#include "types.h"
#include "shortcuts.h"
#include "Dictionary.h"
#include "Dictionary_Entry_Value.h"
#include "Dictionary_Entry.h"
#include "JournalFile.h"
#include "Stream.h"
#include "Journal.h"


int Dictionary_GetUnsignedInt_WithDefault( Dictionary* dictionary, Dictionary_Entry_Key key, const unsigned int defaultVal ) {
	return	Dictionary_Entry_Value_AsUnsignedInt( 
		Dictionary_GetDefault( dictionary, key, 
		Dictionary_Entry_Value_FromUnsignedInt( defaultVal ) ) );
}

int Dictionary_GetInt_WithDefault( Dictionary* dictionary, Dictionary_Entry_Key key, const int defaultVal ) {
	return	Dictionary_Entry_Value_AsInt( 
		Dictionary_GetDefault( dictionary, key, 
		Dictionary_Entry_Value_FromInt( defaultVal ) ) );
}
double Dictionary_GetDouble_WithDefault( Dictionary* dictionary, Dictionary_Entry_Key key, const double defaultVal ) {
	return	Dictionary_Entry_Value_AsDouble( 
		Dictionary_GetDefault( dictionary, key, 
		Dictionary_Entry_Value_FromDouble( defaultVal ) ) );
}

float Dictionary_GetFloat_WithDefault( Dictionary* dictionary, Dictionary_Entry_Key key, const float defaultVal ) {
	return	(float) Dictionary_Entry_Value_AsDouble( 
		Dictionary_GetDefault( dictionary, key, 
		Dictionary_Entry_Value_FromDouble( (double)defaultVal ) ) );
}


Bool Dictionary_GetBool_WithDefault( Dictionary* dictionary, Dictionary_Entry_Key key, const Bool defaultVal ) {
	return	Dictionary_Entry_Value_AsBool( 
		Dictionary_GetDefault( dictionary, key, 
		Dictionary_Entry_Value_FromBool( defaultVal ) ) );
}

char* Dictionary_GetString_WithDefault( Dictionary* dictionary, Dictionary_Entry_Key key, const char* const defaultVal ) {
	return	Dictionary_Entry_Value_AsString( 
		Dictionary_GetDefault( dictionary, key, 
		Dictionary_Entry_Value_FromString( defaultVal ) ) );
}

char* Dictionary_GetString_WithPrintfDefault( Dictionary* dictionary, Dictionary_Entry_Key key, char* format, ... ) {
	char*     returnString;
	char*     defaultVal;
	va_list   ap;
	
	va_start( ap, format );

	/* Create Default String from format arguments */
	Stg_vasprintf( &defaultVal, format, ap );

	/* Read Dictionary */
	returnString = Dictionary_GetString_WithDefault( dictionary, key, defaultVal );

	/* Clean up */
	Memory_Free( defaultVal );
	va_end( ap );

	return returnString;
}
Bool Stream_RedirectFile_WithPrependedPath( Stream* stream, char* prependedPath, char* filename ) {
	Bool result;

	/* Check to make sure output path is emtpy */ 
	if ( Stg_StringIsEmpty( prependedPath ) )
		result = Stream_RedirectFile( stream, filename );
	else {
		char* prependedFilename;

		Stg_asprintf( &prependedFilename, "%s/%s", prependedPath, filename );
		result = Stream_RedirectFile( stream, prependedFilename );
		Memory_Free( prependedFilename );
	}

	return result;
}
Bool Stream_AppendFile_WithPrependedPath( Stream* stream, char* prependedPath, char* filename ) {
	Bool result;

	/* Check to make sure output path is emtpy */ 
	if ( Stg_StringIsEmpty( prependedPath ) )
		result = Stream_AppendFile( stream, filename );
	else {
		char* prependedFilename;

		Stg_asprintf( &prependedFilename, "%s/%s", prependedPath, filename );
		result = Stream_AppendFile( stream, prependedFilename );
		Memory_Free( prependedFilename );
	}

	return result;
}
void Journal_PrintString_WithLength( Stream* stream, char* string, int length ) {
	size_t      stringLength = strlen( string ) ;
	char*       breakInMiddle = "...";
	size_t      lengthOfBreak = strlen( breakInMiddle );

	if ( length <= 0 )
		return;
	/* If length given is exeedingly short - then print initial */
	else if ( lengthOfBreak >= length ) {
		Journal_Printf( stream, "%c", string[0] );
		Journal_Write( stream, breakInMiddle, sizeof( char ), length - 1 );
	}
	/* Shorten String if nessesary */
	else if ( stringLength > length ) {
		int charsBeforeBreak = (int) ((float)length * 0.5) - 1;
		int charsAfterBreak  = length - charsBeforeBreak - lengthOfBreak;

		/* Print first half of string */
		Journal_Write( stream, string, sizeof( char ), charsBeforeBreak );

		/* Print '...' In middle */
		Journal_Printf( stream, breakInMiddle );

		/* Print end of string */
		Journal_Write( stream, &string[ stringLength - charsAfterBreak ], sizeof( char ), charsAfterBreak );
		return;
	}
	else {
		Index       char_I;

		/* If string is shorter than allowed length - then add empty space */
		for ( char_I = 0 ; char_I < length - stringLength ; char_I++ )
			Journal_Printf( stream, " " );
		
		/* Print the complete string */
		Journal_Printf( stream, string );
	}
}

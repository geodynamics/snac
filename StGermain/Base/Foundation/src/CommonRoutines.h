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
**	Collection of commonly used functions	
**
** <b>Assumptions:</b>
**	None
**
** <b>Comments:</b>
**	None
**
** $Id: CommonRoutines.h 2276 2004-11-04 02:01:18Z AlanLo $
**
**/

#ifndef __Base_Foundation_CommonRoutines_h__
#define __Base_Foundation_CommonRoutines_h__
	#include <stdarg.h>

	/** Global map from a Boolean's enum value to a string: for printing purposes */
	extern const char* StG_BoolToStringMap[2];
	
	/** Rounds a double to the specified number of significant figures */
	double StG_RoundDoubleToNSigFigs( double value, unsigned int nSigFigs );

	/** Rounds a double to a certain number of decimal places */
	double StG_RoundDoubleToNDecimalPlaces( double value, unsigned int nDecimalPlaces );
	
	/** Counts the number of characters required to display the given base 10 value. */
	unsigned int StG_IntegerLength( int number );

	/** StGermain's version of strdup() which uses Memory Module */
	char* StG_Strdup( const char* const str );

	/** StGermain's version of asprintf which allocates enough space for a string before printing to it */
	int Stg_asprintf( char** string, char* format, ... ) ;
	int Stg_vasprintf( char** string, char* format, va_list ap ) ;

	/** Check to see whether string is empty or only has white space */
	Bool Stg_StringIsEmpty( char* string ) ;

	/** Check to see whether string is can be considered to be a number - 
	 * allows digits, plus and minus signs, and scientific notation */
	Bool Stg_StringIsNumeric( char* string ) ;

	/** This function calculates the of the longest matching subsequence between string1 and string2.
	 *  A subsequence of a string is when all the characters of the subsequence appear in order within the string,
	 *  but possibly with gaps between occurrences of each character. */
	unsigned int Stg_LongestMatchingSubsequenceLength( char* string1, char* string2, Bool caseSensitive ) ;

#endif /* __Base_Foundation_CommonRoutines_h__ */

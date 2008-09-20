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
**	An abstract class representing a formatter which adds text per line.
**
** <b>Assumptions:</b>
**	None
**
** <b>Comments:</b>
**	None
**
** <b>Description:</b>
**	Encapsulates a set of code which formats printf() statements by line.
**	Sub-classes are written to describe what is prepended.
**
** $Id: LineFormatter.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**/


#ifndef __Base_IO_LineFormatter_h__
#define __Base_IO_LineFormatter_h__

	/** Textual name for LineFormatter class. */
	extern const Type LineFormatter_Type;


	/** \def __LineFormatter See LineFormatter */
	#define __LineFormatter \
		__StreamFormatter			/* Inherit StreamFormatter. */ \
							\
		Bool	_newLine;			/* Whether the current stream is at a new line. */ \
		char*	_tag;				/* The string to prepend at the start of every line. */
	struct LineFormatter { __LineFormatter };


	/** Constructor interface. */
	LineFormatter* _LineFormatter_New(
		SizeT 				_sizeOfSelf,
		Type 				type,
		Stg_Class_DeleteFunction*		_delete,
		Stg_Class_PrintFunction*		_print,
		Stg_Class_CopyFunction*		_copy, 
		StreamFormatter_FormatFunction*	_format );

	/** Init interface. */
	void _LineFormatter_Init(
		LineFormatter*			self,
		StreamFormatter_FormatFunction*	_format );
			
	/** Stg_Class_Delete interface. */
	void _LineFormatter_Delete( void* formatter );

	/** Print interface. */
	void _LineFormatter_Print( void* formatter, Stream* stream );
	
	/** Copy interface. */
	#define LineFormatter_Copy( self ) \
		(LineFormatter*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define LineFormatter_DeepCopy( self ) \
		(LineFormatter*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _LineFormatter_Copy( void* lineFormatter, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap );


	/** Formats a output string of printf by prepending a tag in front of every line detected. */
	char* _LineFormatter_Format( void* formatter, char* fmt );


#endif /* __Base_IO_LineFormatter_h__ */




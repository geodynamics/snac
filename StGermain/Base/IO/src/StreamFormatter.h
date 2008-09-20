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
**	Abstract class which formats inputs from Printf() calls.
**
** <b>Assumptions:</b>
**	None
**
** <b>Comments:</b>
**	None.
**
** <b>Description:</b>
**	A facility which allows custom outputs to be added seamlessly for Printf() calls in Journal.
**	StreamFormatters are given the Printf() format string to add/remove/modify/rearrange as desired.
**
** $Id: StreamFormatter.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**/

#ifndef __Base_IO_StreamFormatter_h__
#define __Base_IO_StreamFormatter_h__


	/** Textual name for StreamFormatter class. */
	extern const Type StreamFormatter_Type;

	typedef char* (StreamFormatter_FormatFunction) ( void* formatter, char* fmt );

	/** \def __StreamFormatter See StreamFormatter */
	#define __StreamFormatter \
		__Stg_Class						\
								\
		/* Virtual functions */				\
		StreamFormatter_FormatFunction*	_format;	\
								\
		/* Members */					\
		Stream*				_stream;
	struct StreamFormatter { __StreamFormatter };


	/** Constructor interface. */
	StreamFormatter* _StreamFormatter_New(
		SizeT 				_sizeOfSelf,
		Type 				type,
		Stg_Class_DeleteFunction*		_delete,
		Stg_Class_PrintFunction*		_print,
		Stg_Class_CopyFunction*		_copy, 
		StreamFormatter_FormatFunction*	_format );
	
	/** Init interface. */
	void _StreamFormatter_Init(
		StreamFormatter*		self,
		StreamFormatter_FormatFunction*	_format );
			
	/** Stg_Class_Delete interface. */
	void _StreamFormatter_Delete( void* formatter );

	/** Print interface. */
	void _StreamFormatter_Print( void* formatter, Stream* stream );
	
	/** Copy interface. */	
	void* _StreamFormatter_Copy( void* streamFormatter, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap );


	/** Formats a Printf() output string. */
	char* StreamFormatter_Format( void* formatter, char* fmt );


	extern StreamFormatter_Buffer* stgStreamFormatter_Buffer;

	/* A buffer swap singleton for improve formatting efficiency. */
	#define __StreamFormatter_Buffer \
		char* buffer1; \
		char* buffer2; \
		Index length1; \
		Index length2; \
		char** current; \
		Index* lengthPtr; 
	struct StreamFormatter_Buffer { __StreamFormatter_Buffer };

	StreamFormatter_Buffer* StreamFormatter_Buffer_New();
	void StreamFormatter_Buffer_Delete( StreamFormatter_Buffer* buffer );
	char* StreamFormatter_Buffer_AllocNext( Index size );

#endif /* __Base_IO_StreamFormatter_h__ */

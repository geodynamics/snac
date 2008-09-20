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
**  Role:
**	Basic framework types.
**
** Assumptions:
**	None as yet.
**
** Comments:
**	None as yet.
**
** $Id: types.h 3743 2006-08-03 03:14:38Z KentHumphries $
**
**/

#ifndef __Base_IO_types_h__
#define __Base_IO_types_h__
	
	/* Dicitonary internal types */
	typedef char*				Dictionary_Entry_Key;
	typedef char*				Dictionary_Entry_Source;
	typedef enum {
		Dictionary_Entry_Value_Type_String,
		Dictionary_Entry_Value_Type_Double,
		Dictionary_Entry_Value_Type_UnsignedInt,
		Dictionary_Entry_Value_Type_Bool,
		Dictionary_Entry_Value_Type_VoidPtr, /* proposed... not in use yet */
		Dictionary_Entry_Value_Type_Struct,
		Dictionary_Entry_Value_Type_List,
		Dictionary_Entry_Value_Type_Int,
		Dictionary_Entry_Value_Type_UnsignedLong,
		Dictionary_Entry_Value_Num_Types
	} Dictionary_Entry_Value_Type;

	/* Base types/classes */
	typedef struct _Dictionary_Entry_Value_Validator	Dictionary_Entry_Value_Validator;
	typedef struct _Dictionary_Entry_Value_List		Dictionary_Entry_Value_List;
	typedef struct _Dictionary_Entry_Value			Dictionary_Entry_Value;
	typedef struct _Dictionary_Entry			Dictionary_Entry;
	typedef struct _Dictionary				Dictionary;
	
	typedef struct _IO_Handler	IO_Handler;
	typedef struct _XML_IO_Handler	XML_IO_Handler;

	typedef unsigned int	JournalLevel;

	/* Journal class*/
	typedef struct JournalFile		JournalFile;
	typedef struct CFile			CFile;
	typedef struct MPIFile			MPIFile;
	typedef struct JournalTypedStream	JournalTypedStream;
	typedef struct Journal			Journal;	
	typedef struct Stream			Stream;
	typedef struct CStream			CStream;
	typedef struct MPIStream		MPIStream;
	
	typedef struct StreamFormatter		StreamFormatter;
	typedef struct StreamFormatter_Buffer	StreamFormatter_Buffer;
	typedef struct LineFormatter		LineFormatter;
	typedef struct RankFormatter		RankFormatter;
	typedef struct IndentFormatter		IndentFormatter;

#endif /* __Base_IO_types_h__ */

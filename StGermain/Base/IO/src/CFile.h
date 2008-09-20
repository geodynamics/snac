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
**	Encapsulation of a C file.
**
** <b>Assumptions:</b>
**	None.
**
** <b>Comments:</b>
**	Be careful in parallel situations where more than one processor may try to open the same file due
**	to executing the same piece of code. When writing to files other than stdout and stderr, only
**	allow one process to perform output.
**
** <b>Description</b>
**	CFile wraps around traditional C file operations such as fopen() and fclose() to alloc output
**	for a file.
**
** $Id: CFile.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**/

#ifndef __Base_IO_CFile_h__
#define __Base_IO_CFile_h__

	/** Textual name for CFile class. */
	extern const Type CFile_Type;
	
	/** \def __CFile See CFile. */
	#define __CFile \
		/* General info */ \
		__JournalFile
	struct CFile { __CFile };


	/** Creates a new CFile with no opened file. JournalFile_Open() must be called. */
	JournalFile* CFile_New();

	/** Creates a new CFile, opening the file given by fileName. Returns NULL if system is unable to open the given file. */
	JournalFile* CFile_New2( char* fileName );

	/** Initialises an instance of CFile. Will not open any files. */
	void CFile_Init( CFile* self );


	/** Constructor interface. */
	CFile* _CFile_New(
		SizeT _sizeOfSelf,
		Type type,
		Stg_Class_DeleteFunction* _delete,
		Stg_Class_PrintFunction* _print,
		Stg_Class_CopyFunction* _copy );
		
	/** Init interface. */
	void _CFile_Init( CFile* self );
	
	/** Stg_Class_Delete interface. */
	void _CFile_Delete( void* cfile );

	/** Print interface. */
	void _CFile_Print( void* cfile, Stream* stream );
	
	
	/** Implementation for Open function. */
	Bool _CFile_Open( void* file, char* fileName );

	/** Implementation for Append function. */
	Bool _CFile_Append( void* file, char* fileName );

	/** Implementation for Close function. */
	Bool _CFile_Close( void* file );

	/** Implementation for Flush function. */
	Bool _CFile_Flush( void* file );
	

#endif /* __Base_IO_CFile_h__ */



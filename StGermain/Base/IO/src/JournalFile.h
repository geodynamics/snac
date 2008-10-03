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
**	Abstract representation of a file.
**
** <b>Assumptions:</b>
**	None
**
** <b>Comments:</b>
**	The abstraction of this class has come from a number of reasons:
**	- Other than using traditional C files, StGermain will use HDF5 and potentially other alternatives for writing data to file.
**	  These use different functions other than fopen(), fclose()...etc. 
**
**	  Hence a level of abstract is created to allow for various implementations.
**
**	- The concept of 'File' is separated from 'Stream' to allow more flexibility. It allows users to direct output of a
**	  child stream to a different location from the parent.
**
**	  However, this will introduce a potential of mis-use when uses accidentally attempts to set a HDF5 file to a normal
**	  CStream or vice-versa. Stream_SetFile will catch this error and will print a warning message.
**
** <b>Description:</b>
**	Parent class for all files. Sub classes should implement the open, close and flush functions.
**	JournalFile is a sub-class of NamedObject, where its name is the name of the file it has opened if any.
**
** $Id: JournalFile.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**/

#ifndef __Base_IO_JournalFile_h__
#define __Base_IO_JournalFile_h__

	/** Textual name for JournalFile class. */
	extern const Type JournalFile_Type;

	typedef JournalFile* (JournalFile_Constructor) (void);
	typedef Bool (JournalFile_OpenFunction) (void* file, char* fileName );
	typedef Bool (JournalFile_AppendFunction) (void* file, char* fileName );
	typedef Bool (JournalFile_CloseFunction) (void* file );
	typedef Bool (JournalFile_FlushFunction) (void* file );
		
	/** \def __JournalFile See JournalFile */
	#define __JournalFile \
		__Stg_Object					\
								\
		/* Virtual functions */				\
		JournalFile_OpenFunction*	_open; 		\
		JournalFile_AppendFunction*	_append; 		\
		JournalFile_CloseFunction*	_close; 	\
		JournalFile_FlushFunction*	_flush; 	\
								\
		/* JournalFile members */			\
		void*				fileHandle;	/* Generic file handle for any file. */ \
		Bool				_opened;
	struct JournalFile { __JournalFile };


	/** Constructor interface. */
	JournalFile* _JournalFile_New(
		SizeT				_sizeOfSelf,
		Type				type,
		Stg_Class_DeleteFunction*		_delete,
		Stg_Class_PrintFunction*		_print,
		Stg_Class_CopyFunction*		_copy, 
		JournalFile_OpenFunction*	_open,
		JournalFile_AppendFunction*	_append,
		JournalFile_CloseFunction*	_close,
		JournalFile_FlushFunction*	_flush );
	
	/** Init interface. */
	void _JournalFile_Init(
		JournalFile*			self,
		JournalFile_OpenFunction*	_open,
		JournalFile_AppendFunction*	_append,
		JournalFile_CloseFunction*	_close,
		JournalFile_FlushFunction*	_flush );
	
	/** Stg_Class_Delete interface. */
	void _JournalFile_Delete( void* journalFile );

	/** Print interface. */
	void _JournalFile_Print( void* journalFile, Stream* stream );		
	

	/** Open a file for writing. If successful, sets the name for this instance to the file name and returns True.
	 **
	 ** @param journalFile An instance of a JournalFile sub-class.
	 **/
	Bool JournalFile_Open( void* journalFile, char* fileName );
	
	
	/** Reopen a file for writing. It will use the previous name it was opened with.
	 **
	 ** @param journalFile An instance of a JournalFile sub-class.
	 **/
	Bool JournalFile_Reopen( void* journalFile );
	
	/** Macro to improve readability */
	#define JournalFile_ReopenIfClosed JournalFile_Reopen
	
	/** Opens a file for writing, if the file exists then append future print statements to this file.
	 ** If successful, sets the name for this instance to the file name and returns True.
	 **
	 ** @param journalFile An instance of a JournalFile sub-class.
	 **/
	Bool JournalFile_Append( void* journalFile, char* fileName );

	/** Close a file. If successful, returns True.
	 **
	 ** @param journalFile An instance of a JournalFile sub-class.
	 **/
	Bool JournalFile_Close( void* journalFile );

	/** Checks whether a file is opened.
	 **
	 ** @param journalFile An instance of a JournalFile sub-class.
	 **/
	Bool JournalFile_IsOpened( void* journalFile );

	/** Flush a file, outputting all un-written data from input buffers to device/file. If successful, returns True.
	 **
	 ** @param journalFile An instance of a JournalFile sub-class.
	 **/
	Bool JournalFile_Flush( void* journalFile );


#endif /* __Base_IO_JournalFile_h__ */

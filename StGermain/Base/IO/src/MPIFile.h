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
**	Encapsulation of MPI IO routines for file writing
**
** <b>Assumptions:</b>
**	None.
**
** <b>Comments:</b>
**
** <b>Description</b>
**      MPIFile is a wrapper to the MPI IO routines for file writing.
**
** $Id: MPIFile.h 2745 2005-03-05 08:12:18Z SteveQuenette $
**
**/

#ifndef __Base_IO_MPIFile_h__
#define __Base_IO_MPIFile_h__

#include <mpi.h>

	/** Textual name for MPIFile class. */
	extern const Type MPIFile_Type;
	
	/** \def __MPIFile See MPIFile. */
	#define __MPIFile \
		/* General info */ \
		__JournalFile \
		MPI_File mpiFile;
	struct MPIFile { __MPIFile };


	/** Creates a new MPIFile with no opened file. JournalFile_Open() must be called. */
	JournalFile* MPIFile_New();

	/** Creates a new MPIFile, opening the file given by fileName. Returns NULL if system is unable to open the given file. */
	JournalFile* MPIFile_New2( char* fileName );

	/** Initialises an instance of MPIFile. Will not open any files. */
	void MPIFile_Init( MPIFile* self );


	/** Constructor interface. */
	MPIFile* _MPIFile_New(
		SizeT _sizeOfSelf,
		Type type,
		Stg_Class_DeleteFunction* _delete,
		Stg_Class_PrintFunction* _print,
		Stg_Class_CopyFunction* _copy );
		
	/** Init interface. */
	void _MPIFile_Init( MPIFile* self );
	
	/** Stg_Class_Delete interface. */
	void _MPIFile_Delete( void* cfile );

	/** Print interface. */
	void _MPIFile_Print( void* cfile, Stream* stream );
	
	
	/** Implementation for Open function. */
	Bool _MPIFile_Open( void* file, char* fileName );

	/** Implementation for Append function. */
	Bool _MPIFile_Append( void* file, char* fileName );

	/** Implementation for Close function. */
	Bool _MPIFile_Close( void* file );

	/** Implementation for Flush function. */
	Bool _MPIFile_Flush( void* file );
	

#endif /* __Base_IO_MPIFile_h__ */

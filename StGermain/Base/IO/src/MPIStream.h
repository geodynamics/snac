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
**	Encapsulation of C-style printing.
**
** <b>Assumptions</b>
**	None
**
** <b>Comments</b>
**	None
**
** <b>Description</b>
**	A Wrapper class for C output functions such as fprintf() and fwrite().
**
** Comments:
**
** $Id: MPIStream.h 2745 2005-03-05 08:12:18Z SteveQuenette $
**
**/

#ifndef __Base_IO_MPIStream_h__
#define __Base_IO_MPIStream_h__
	
	/** Textual name for MPIStream class. */
	extern const Type MPIStream_Type;
	
	
	/** \def __MPIStream See MPIStream. */
	#define __MPIStream \
		/* General info */ \
		__Stream
	struct MPIStream { __MPIStream };


	/** Create a new MPIStream */
	Stream* MPIStream_New( Name name );

	/** Inits a MPIStream. */
	void MPIStream_Init( MPIStream* self, Name name );

	/** Constructor interface. */
	MPIStream* _MPIStream_New( 
		SizeT			_sizeOfSelf, 
		Type			type, 
		Stg_Class_DeleteFunction*	_delete, 
		Stg_Class_PrintFunction* 	_print,
		Stg_Class_CopyFunction*	_copy, 
		Name			name,
		Stream_PrintfFunction*	_printf, 
		Stream_WriteFunction*	_write, 
		Stream_DumpFunction*	_dump,
		Stream_SetFileFunction*	_setFile );

	/** Init interface. */
	void _MPIStream_Init( MPIStream *self );
	
	/** Stg_Class_Delete interface. */
	void _MPIStream_Delete( void* cStream );
	
	/** Print interface. */
	void _MPIStream_Print( void* cStream, Stream* stream );


	/** Printf() implementation. */
	SizeT _MPIStream_Printf( Stream* stream, char *fmt, va_list args );
	
	/** Write() implementation. */
	SizeT _MPIStream_Write( Stream* stream, void *data, SizeT elem_size, SizeT num_elems );
	
	/** Dump() implementation. Performs no operation for MPIStreams. */
	Bool _MPIStream_Dump( Stream* stream, void *data );
	
	/** SetFile() implementation. */
	Bool _MPIStream_SetFile( Stream* stream, JournalFile* file );
	
	/** Sets the offset in bytes of this an mpi stream so that each rank may print to their own position in file */
	Bool MPIStream_SetOffset( Stream* stream, SizeT sizeToWrite, MPI_Comm communicator ) ;
	SizeT MPIStream_WriteAllProcessors( Stream* stream, void *data, SizeT elem_size, SizeT num_elems, MPI_Comm communicator ) ;
	
#endif /* __IO_MPIStreamFile_h__ */




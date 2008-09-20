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
**
**
** <b>Assumptions</b>
**	None
**
** <b>Comments</b>
**	None
**
** <b>Description</b>
**
**
** Comments:
**
** $Id: VariableDumpStream.h 789 2004-02-10 17:15:11Z SteveQuenette $
**
**/

#ifndef __Base_IO_VariableDumpStream_h__
#define __Base_IO_VariableDumpStream_h__
	
	/** Textual name for VariableDumpStream class. */
	extern const Type VariableDumpStream_Type;
	
	
	/** \def __VariableDumpStream See VariableDumpStream. */
	#define __VariableDumpStream \
		/* General info */ \
		__CStream \
		Variable*	data; \
		Index		numItems; \
		unsigned int	dumpEvery;
	struct VariableDumpStream { __VariableDumpStream };


	/** Create a new VariableDumpStream */
	Stream* VariableDumpStream_New( Name name );

	/** Inits a VariableDumpStream. */
	void VariableDumpStream_Init( VariableDumpStream* self, Name name );

	/** Constructor interface. */
	VariableDumpStream* _VariableDumpStream_New( 
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
	void _VariableDumpStream_Init( VariableDumpStream *self );
	
	/** Stg_Class_Delete interface. */
	void _VariableDumpStream_Delete( void* vStream );
	
	/** Print interface. */
	void _VariableDumpStream_Print( void* vStream, Stream* stream );

	/** Dumps the variable. */ 
	Bool _VariableDumpStream_Dump( Stream* stream, void* loop );
	
	/** Copy interface. */
	#define VariableDumpStream_Copy( self ) \
		(VariableDumpStream*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define VariableDumpStream_DeepCopy( self ) \
		(VariableDumpStream*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _VariableDumpStream_Copy( void* variableDumpStream, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap );

	/** Sets the information that this stream dumps. */
	void VariableDumpStream_SetVariable( void* stream, Variable* data, int numItems, unsigned int dumpEvery, char* fileName );
	
	
#endif /* __IO_VariableDumpStreamFile_h__ */




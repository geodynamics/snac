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
** $Id: VariableDumpStream.c 789 2004-02-10 17:15:11Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"
                                                                                                                                    
#include "types.h"
#include "Stg_Component.h"
#include "Variable.h"
#include "VariableDumpStream.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>


const Type VariableDumpStream_Type = "VariableDumpStream";


Stream* VariableDumpStream_New( Name name )
{
	return (Stream*)_VariableDumpStream_New( 
		sizeof(VariableDumpStream), 
		VariableDumpStream_Type, 
		_VariableDumpStream_Delete, 
		_VariableDumpStream_Print, 
		_VariableDumpStream_Copy, 
		name,
		_CStream_Printf,
		_CStream_Write,
		_VariableDumpStream_Dump,
		_CStream_SetFile );
}

void VariableDumpStream_Init( VariableDumpStream* self, Name name )
{
	
}


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
	Stream_SetFileFunction*	_setFile )
{
	VariableDumpStream* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(VariableDumpStream) );
	self = (VariableDumpStream*)_CStream_New( _sizeOfSelf, type, _delete, _print, _copy, name, 
		_printf, _write, _dump, _setFile );
	
	_VariableDumpStream_Init( self );
	
	return self;
}

void _VariableDumpStream_Init( VariableDumpStream* self )
{
	self->data = NULL;
	self->numItems = 0;
}
	
void _VariableDumpStream_Delete( void* cStream )
{
	VariableDumpStream* self = (VariableDumpStream*)cStream;
	
	/* Stg_Class_Delete parent */
	_Stream_Delete( self );
}

void _VariableDumpStream_Print( void* cStream, Stream* stream ) {

	VariableDumpStream* self = (VariableDumpStream*)cStream;
	
	/* Print parent */
	_Stream_Print( self, stream );
	
	/* General info */
	Journal_Printf( stream,  "VariableDumpStream (ptr): %p\n", cStream);
	
	Journal_Printf( stream, "data (ptr): %p\n", self->data );
	Journal_Printf( stream, "numItems: %d\n", self->numItems );
}

void* _VariableDumpStream_Copy( void* variableDumpStream, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap ) {
	VariableDumpStream*	self = (VariableDumpStream*)variableDumpStream;
	VariableDumpStream*	newVariableDumpStream;
	
	newVariableDumpStream = _Stream_Copy( self, dest, deep, nameExt, ptrMap );
	
	/* HACK */
	/* TODO: this should actually copy the data, i think... */
	newVariableDumpStream->data = NULL;
	newVariableDumpStream->numItems = 0;
	/* END HACK */
	newVariableDumpStream->dumpEvery = self->dumpEvery;
	
	return newVariableDumpStream;
}

Bool _VariableDumpStream_Dump( Stream* stream, void* _loop )
{
	VariableDumpStream* self = (VariableDumpStream*)stream;
	int varI, typeI;
	int* loop = (int*)_loop;
	float tmp;
	
	
	if ( self->data == NULL || self->numItems == 0 )
	{
		return True;
	}
	if ( loop != NULL )
	{
		if( !((*loop) == 0 || (((*loop) - 1) % self->dumpEvery == 0)) )
			return True;
	}

	/* testing if the others compile....
	char tmp1 = Variable_GetValueAsChar( self->data, varI );
	short tmp2 = Variable_GetValueAsShort( self->data, varI );
	int tmp3 = Variable_GetValueAsInt( self->data, varI );
	double tmp4 = Variable_GetValueAsDouble( self->data, varI );
	void* tmp5 = Variable_GetValueAsPointer( self->data, varI );*/
	for ( varI = 0; varI < self->data->arraySize; ++varI ) {
		if ( *self->data->dataTypeCounts > 1 ) {
			for ( typeI = 0; typeI < *self->data->dataTypeCounts; ++typeI ) {
				tmp = Variable_GetValueAtDoubleAsFloat( self->data, varI, typeI );
				self->_write(
					stream,
					&tmp,
					sizeof(float), 1 );
			}
		}
		else {
			tmp = Variable_GetValueDoubleAsFloat( self->data, varI );
			self->_write(
				stream,
				&tmp,
				sizeof(float), 1 );
		}
	}


	return True;
}

void VariableDumpStream_SetVariable( void* stream, Variable* data, int numItems, unsigned int dumpEvery, char* fileName )
{
	VariableDumpStream* self = (VariableDumpStream*)stream;
	JournalFile* file;
	
	self->data = data;
	self->numItems = numItems;
	self->dumpEvery = dumpEvery;
	
	file = Journal_GetFile( fileName );
	if ( file == NULL )
	{
		file = CFile_New2( fileName );
		Journal_RegisterFile( file );
	}
	Stream_SetFile( stream, file );
}

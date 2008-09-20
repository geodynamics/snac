/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003, Victorian Partnership for Advanced Computing (VPAC) Ltd, 110 Victoria Street, Melbourne, 3053, Australia.
**
** Authors:
**	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
**
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2, or (at your option) any
** later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**
** $Id: ExchangerVC.c 1166 2004-04-03 00:42:53Z SiewChingTan $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <string.h>
#include <assert.h>

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>

#include "types.h"
#include "ExchangerVC.h"


const Type ExchangerVC_Type = "ExchangerVC";


/*--------------------------------------------------------------------------------------------------------------------------
** Constructor
*/

VariableCondition* ExchangerVC_Factory(
		Variable_Register*				variable_Register, 
		ConditionFunction_Register*			condFunc_Register, 
		Dictionary*					dictionary,
		void*						data )
{
	return (VariableCondition*)ExchangerVC_New( variable_Register, condFunc_Register, dictionary, data );
}


ExchangerVC*	ExchangerVC_DefaultNew( Name name )
{
	return _ExchangerVC_New( 
		sizeof(ExchangerVC), 
		ExchangerVC_Type, 
		_ExchangerVC_Delete,
		_ExchangerVC_Print, 
		NULL, 
		(Stg_Component_DefaultConstructorFunction*)ExchangerVC_DefaultNew,
		_VariableCondition_Construct,
		_VariableCondition_Build,
		_VariableCondition_Initialise,
		_VariableCondition_Execute,
		_VariableCondition_Destroy,
		name,
		False,
		NULL, 
		_ExchangerVC_PrintConcise,
		_ExchangerVC_ReadDictionary,
		_ExchangerVC_GetSet, 
		_ExchangerVC_GetVariableCount, 
		_ExchangerVC_GetVariableIndex, 
		_ExchangerVC_GetValueIndex, 
		_ExchangerVC_GetValueCount, 
		_ExchangerVC_GetValue,
		NULL, 
		NULL, 
		NULL );
}

ExchangerVC*	ExchangerVC_New(
		Variable_Register*				variable_Register, 
		ConditionFunction_Register*			condFunc_Register,
		Dictionary*					dictionary,
		Name						varName )
{
	return _ExchangerVC_New( 
		sizeof(ExchangerVC), 
		ExchangerVC_Type, 
		_ExchangerVC_Delete,
		_ExchangerVC_Print, 
		NULL, 
		(Stg_Component_DefaultConstructorFunction*)ExchangerVC_DefaultNew,
		_VariableCondition_Construct,
		_VariableCondition_Build,
		_VariableCondition_Initialise,
		_VariableCondition_Execute,
		_VariableCondition_Destroy,
		varName,
		True,
		NULL, 
		_ExchangerVC_PrintConcise,
		_ExchangerVC_ReadDictionary,
		_ExchangerVC_GetSet, 
		_ExchangerVC_GetVariableCount, 
		_ExchangerVC_GetVariableIndex, 
		_ExchangerVC_GetValueIndex, 
		_ExchangerVC_GetValueCount, 
		_ExchangerVC_GetValue,
		variable_Register, 
		condFunc_Register, 
		dictionary );
}


void ExchangerVC_Init(
		ExchangerVC*					self,
		Variable_Register*				variable_Register, 
		ConditionFunction_Register*			condFunc_Register,
		Dictionary*					dictionary,
		Name						varName )
{
	/* General info */
	self->type = ExchangerVC_Type;
	self->_sizeOfSelf = sizeof(ExchangerVC);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _ExchangerVC_Delete;
	self->_print = _ExchangerVC_Print;
	self->_copy = NULL;
	self->_build = _VariableCondition_Build;
	self->_initialise = _VariableCondition_Initialise;
	self->_execute = _VariableCondition_Execute;
	self->_buildSelf = NULL;
	self->_printConcise = _ExchangerVC_PrintConcise;
	self->_readDictionary = _ExchangerVC_ReadDictionary;
	self->_getSet = _ExchangerVC_GetSet;
	self->_getVariableCount = _ExchangerVC_GetVariableCount;
	self->_getVariableIndex = _ExchangerVC_GetVariableIndex;
	self->_getValueIndex = _ExchangerVC_GetValueIndex;
	self->_getValueCount = _ExchangerVC_GetValueCount;
	self->_getValue = _ExchangerVC_GetValue;
	
	_Stg_Class_Init( (Stg_Class*)self );
	_VariableCondition_Init( (VariableCondition*)self, variable_Register, condFunc_Register, dictionary );
	
	/* Stg_Class info */
	_ExchangerVC_Init( self, varName );
}


ExchangerVC* _ExchangerVC_New( 
		SizeT						_sizeOfSelf, 
		Type						type,
		Stg_Class_DeleteFunction*				_delete,
		Stg_Class_PrintFunction*				_print,
		Stg_Class_CopyFunction*				_copy, 
		Stg_Component_DefaultConstructorFunction*		_defaultConstructor,
		Stg_Component_ConstructFunction*			_construct,
		Stg_Component_BuildFunction*			_build,
		Stg_Component_InitialiseFunction*			_initialise,
		Stg_Component_ExecuteFunction*			_execute,
		Stg_Component_DestroyFunction*				_destroy,
		Name						varName,
		Bool						initFlag,
		VariableCondition_BuildSelfFunc*		_buildSelf, 
		VariableCondition_PrintConciseFunc*		_printConcise,
		VariableCondition_ReadDictionaryFunc*		_readDictionary,
		VariableCondition_GetSetFunc*			_getSet,
		VariableCondition_GetVariableCountFunc*		_getVariableCount,
		VariableCondition_GetVariableIndexFunc*		_getVariableIndex,
		VariableCondition_GetValueIndexFunc*		_getValueIndex,
		VariableCondition_GetValueCountFunc*		_getValueCount,
		VariableCondition_GetValueFunc*			_getValue,
		Variable_Register*				variable_Register, 
		ConditionFunction_Register*			condFunc_Register,
		Dictionary*					dictionary )
{
	ExchangerVC*	self;
	
	/* Allocate memory/General info */
	assert(_sizeOfSelf >= sizeof(ExchangerVC));
	self = (ExchangerVC*)_VariableCondition_New( 
		_sizeOfSelf, 
		type, 
		_delete, 
		_print,
		_copy, 
		_defaultConstructor,
		_construct,
		_build,
		_initialise,
		_execute,
		_destroy,
		varName,
		initFlag,
		_buildSelf, 
		_printConcise,
		_readDictionary,
		_getSet, 
		_getVariableCount, 
		_getVariableIndex, 
		_getValueIndex, 
		_getValueCount, 
		_getValue, 
		variable_Register, 
		condFunc_Register,
		dictionary );
	
	/* Virtual info */
	
	/* Stg_Class info */
	_ExchangerVC_Init( self, varName );
	
	return self;
}


void _ExchangerVC_Init( 
		void*						exchangerVC,
		Name						varName )
{
	ExchangerVC*			self = (ExchangerVC*)exchangerVC;
	
	self->varName = varName;
	self->valuesSet = 0;
	self->valuesSize = 0;
	self->values = 0;
	self->valuesIndices = 0;
}


/*--------------------------------------------------------------------------------------------------------------------------
** General virtual functions
*/

void _ExchangerVC_ReadDictionary( void* variableCondition, void* dictionary ) {
}


void _ExchangerVC_Delete( void* exchangerVC ) {
	ExchangerVC*	self = (ExchangerVC*)exchangerVC;
	
	if( self->values ) {
		free( self->values );
	}
	
	if( self->valuesSet ) {
		Stg_Class_Delete( self->valuesSet );
	}
	
	if( self->valuesIndices ) {
		free( self->valuesIndices );
	}
	
	if( self->varName ) {
		free( self->varName );
	}
	
	/* Stg_Class_Delete parent */
	_VariableCondition_Delete( self );
}


void _ExchangerVC_Print( void* exchangerVC, Stream* stream ) {
	ExchangerVC*				self = (ExchangerVC*)exchangerVC;
	
	/* Set the Journal for printing informations */
	Stream* info = stream;
	
	/* General info */
	Journal_Printf( info, "ExchangerVC (ptr): %p\n", self);
	
	/* Print parent */
	_VariableCondition_Print( self );
	
	/* Virtual info */
	
	/* Stg_Class info */
	if( self->varName ) {
		Journal_Printf( info, "\tvalName: %s\n", self->varName );
	}
	Print( self->valuesSet, stream );
	Journal_Printf( info, "\tvaluesSize: %u\n", self->valuesSize );
	Journal_Printf( info, "\tvalues (ptr): %p\n", self->values );
	if( self->values && self->valuesIndices ) {
		Index					entry_I;
		
		Journal_Printf( info, "\tvalues[0-%u]: %u\n", self->valuesSize );
		for( entry_I = 0; entry_I < self->valuesSize; entry_I++ ) {
			Journal_Printf( info, "\t\tindex: %u\n", self->valuesIndices[entry_I] );
			switch( self->values[entry_I].type ) {
				case VC_ValueType_Double:
					Journal_Printf( info, "\t\ttype: VC_ValueType_Double\n" );
					Journal_Printf( info, "\t\tasDouble: %g\n", self->values[entry_I].as.typeDouble );
					break;
					
				case VC_ValueType_Int:
					Journal_Printf( info, "\t\ttype: VC_ValueType_Int\n" );
					Journal_Printf( info, "\t\tasInt: %i\n", self->values[entry_I].as.typeInt );
					break;
					
				case VC_ValueType_Short:
					Journal_Printf( info, "\t\ttype: VC_ValueType_Short\n" );
					Journal_Printf( info, "\t\tasShort: %i\n", self->values[entry_I].as.typeShort );
					break;
					
				case VC_ValueType_Char:
					Journal_Printf( info, "\t\ttype: VC_ValueType_Char\n");
					Journal_Printf( info, "\t\tasChar: %c\n", self->values[entry_I].as.typeChar );
					break;
					
				case VC_ValueType_Ptr:
					Journal_Printf( info, "\t\ttype: VC_ValueType_Ptr\n");
					Journal_Printf( info, "\t\tasPtr: %g\n", self->values[entry_I].as.typePtr );
					break;
					
				case VC_ValueType_DoubleArray:
					Journal_Printf( info, "\t\ttype: VC_ValueType_DoubleArray\n" );
					Journal_Printf( info, "\t\t\tarraySize: %u\n", self->values[entry_I].as.typeArray.size );
					Journal_Printf( 
						info, 
						"\t\t\tasDoubleArray (ptr): %p\n", 
						self->values[entry_I].as.typeArray.array );
					if( self->values[entry_I].as.typeArray.array ) {
						Index						i;
						
						for (i = 0; i < self->values[entry_I].as.typeArray.size; i++ ) {
							Journal_Printf( 
								info, 
								"\t\t\tasDoubleArray[%u]: %g\n", i,
								self->values[entry_I].as.typeArray.array[i] );
						}
					}
					break;
					
				case VC_ValueType_CFIndex:
					Journal_Printf( info, "\t\t\ttype: VC_ValueType_CFIndex\n" );
					Journal_Printf( 
						info, 
						"\t\t\tasCFIndex: %u\n", 
						self->values[entry_I].as.typeCFIndex );
					break;
			}
		}
		Journal_Printf( info, "\n" );
	}
}


/*--------------------------------------------------------------------------------------------------------------------------
** Macros
*/


/*--------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

IndexSet* _ExchangerVC_GetSet( void* variableCondition ) {
	ExchangerVC*		self = (ExchangerVC*)variableCondition;
	
	/* Assumes only one variable for all items in the set. The variable at each item is unique. Assume the user has initialised
	   self->valuesSize (as the count of members in:), self->valuesSet (the set) and self->valuesIndices (the array of the
	   set). */
	assert( self->valuesSet );
	
	/* Duplicate it as the caller of this function assumes ownership */
	return IndexSet_Duplicate( self->valuesSet );
}


VariableCondition_VariableIndex _ExchangerVC_GetVariableCount( void* variableCondition, Index index ) {
	ExchangerVC*	self = (ExchangerVC*)variableCondition;
	
	/* Assumes only one variable for all items in the set. The variable at each item is unique. Assume the user has initialised
	   self->valuesSize (as the count of members in:), self->valuesSet (the set) and self->valuesIndices (the array of the
	   set). */
	assert( self->valuesSet );
	
	if( IndexSet_IsMember( self->valuesSet, index ) ) {
		return 1;
	}
	else {
		return 0;
	}
}


Variable_Index _ExchangerVC_GetVariableIndex( 
		void*				variableCondition,
		Index				index,
		VariableCondition_VariableIndex	condVar_I ) 
{
	ExchangerVC*	self = (ExchangerVC*)variableCondition;
	Variable_Index	var_I = Variable_Register_GetIndex( self->variable_Register, self->varName );
	
	/* Assumes only one variable for all items in the set. The variable at each item is unique. Assume the user has initialised
	   self->valuesSize (as the count of members in:), self->valuesSet (the set) and self->valuesIndices (the array of the
	   set). */
	assert( condVar_I == 0 );
	assert( self->valuesSet );
	assert( IndexSet_IsMember( self->valuesSet, index ) );
	
	return var_I;
}


VariableCondition_ValueIndex _ExchangerVC_GetValueIndex(
		void*				variableCondition, 
		Index				index, 
		VariableCondition_VariableIndex	condVar_I )
{
	ExchangerVC*	self = (ExchangerVC*)variableCondition;
	Index		i;
	
	/* Assumes only one variable for all items in the set. The variable at each item is unique. Assume the user has initialised
	   self->valuesSize (as the count of members in:), self->valuesSet (the set) and self->valuesIndices (the array of the
	   set). */
	assert( condVar_I == 0 );
	assert( self->valuesSet );
	assert( IndexSet_IsMember( self->valuesSet, index ) );
	assert( self->valuesIndices );

	/* Given there will as many values as there are set members, we basically want the index into the indecis array for there
	   given index. */
	for( i = 0; i < self->valuesSize; i++ ) {
		if( self->valuesIndices[i] == index ) {
			return i;
		}
	}
	
	/* Shouldn't get here */
	assert( 0 );
	return 0;
}


VariableCondition_ValueIndex _ExchangerVC_GetValueCount( void* variableCondition ) {
	ExchangerVC*	self = (ExchangerVC*)variableCondition;
	
	/* Assumes only one variable for all items in the set. The variable at each item is unique. Assume the user has initialised
	   self->valuesSize (as the count of members in:), self->valuesSet (the set) and self->valuesIndices (the array of the
	   set). */
	assert( self->valuesSize );
	
	return self->valuesSize;
}


VariableCondition_Value _ExchangerVC_GetValue( void* variableCondition, VariableCondition_ValueIndex valIndex ) {
	ExchangerVC*	self = (ExchangerVC*)variableCondition;

	/* Assumes only one variable for all items in the set. The variable at each item is unique. Assume the user has initialised
	   self->valuesSize (as the count of members in:), self->valuesSet (the set) and self->valuesIndices (the array of the
	   set). */
	assert( self->values );

	return self->values[valIndex];
}


void _ExchangerVC_PrintConcise( void* variableCondition, Stream* stream ) {
	ExchangerVC*		self = (ExchangerVC*)variableCondition;
	
	/* Set the Journal for printing informations */
	Journal_Printf( stream, "\ttype: %s\n", self->type );
}

/*--------------------------------------------------------------------------------------------------------------------------
** Build functions
*/


/*--------------------------------------------------------------------------------------------------------------------------
** Functions
*/

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
** $Id: SetVC.c 3881 2006-10-26 03:14:19Z KathleenHumble $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdarg.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"

#include "types.h"
#include "shortcuts.h"
#include "Stg_Component.h"
#include "Stg_ComponentFactory.h"
#include "Variable.h"
#include "Variable_Register.h"
#include "ConditionFunction.h"
#include "ConditionFunction_Register.h"
#include "VariableCondition.h"
#include "SetVC.h"

#include <string.h>
#include <assert.h>


const Type SetVC_Type = "SetVC";
const Name defaultSetVCName = "defaultSetVCName";

/*--------------------------------------------------------------------------------------------------------------------------
** Constructor
*/

VariableCondition* SetVC_Factory(
		Variable_Register*				variable_Register, 
		ConditionFunction_Register*			conFunc_Register, 
		Dictionary*					dictionary,
		void*						data )
{
	return (VariableCondition*)SetVC_New( defaultSetVCName, NULL, variable_Register, conFunc_Register, dictionary );
}


SetVC*	SetVC_New(
		Name						name,
		Name						_dictionaryEntryName, 
		Variable_Register*				variable_Register, 
		ConditionFunction_Register*			conFunc_Register,
		Dictionary*					dictionary )
{
	return _SetVC_New( 
		sizeof(SetVC), 
		SetVC_Type, 
		_SetVC_Delete, 
		_SetVC_Print, 
		_SetVC_Copy,
		(Stg_Component_DefaultConstructorFunction*)SetVC_DefaultNew,
		_VariableCondition_Construct,
		_VariableCondition_Build,
		_VariableCondition_Initialise,
		_VariableCondition_Execute,
		_VariableCondition_Destroy,
		name,
	   True,	
		NULL,
		_SetVC_PrintConcise,
		_SetVC_ReadDictionary,
		_SetVC_GetSet, 
		_SetVC_GetVariableCount, 
		_SetVC_GetVariableIndex, 
		_SetVC_GetValueIndex, 
		_SetVC_GetValueCount, 
		_SetVC_GetValue,
		_dictionaryEntryName,
		variable_Register, 
		conFunc_Register,
		dictionary );
}

SetVC* SetVC_DefaultNew( Name name )
{
	return (SetVC*)_SetVC_New( 
		sizeof(SetVC), 
		SetVC_Type, 
		_SetVC_Delete, 
		_SetVC_Print, 
		_SetVC_Copy,
		(Stg_Component_DefaultConstructorFunction*)SetVC_DefaultNew,
		_VariableCondition_Construct,
		_VariableCondition_Build,
		_VariableCondition_Initialise,
		_VariableCondition_Execute,
		_VariableCondition_Destroy,
		name, 
		False,
		NULL,
		_SetVC_PrintConcise,
		_SetVC_ReadDictionary,
		_SetVC_GetSet, 
		_SetVC_GetVariableCount, 
		_SetVC_GetVariableIndex, 
		_SetVC_GetValueIndex, 
		_SetVC_GetValueCount, 
		_SetVC_GetValue,
		NULL,
		NULL/*variable_Register*/, 
		NULL/*conFunc_Register*/,
		NULL/*dictionary*/ );
}

void SetVC_Init(
		SetVC*						self,
		Name						name,
		Name						_dictionaryEntryName, 
		Variable_Register*				variable_Register, 
		ConditionFunction_Register*			conFunc_Register,
		Dictionary*					dictionary )
{
	/* General info */
	self->type = SetVC_Type;
	self->_sizeOfSelf = sizeof(SetVC);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _SetVC_Delete;
	self->_print = _SetVC_Print;
	self->_copy = _SetVC_Copy;
	self->_build = _VariableCondition_Build;
	self->_initialise = _VariableCondition_Initialise;
	self->_execute = _VariableCondition_Execute;
	self->_buildSelf = NULL;
	self->_printConcise = _SetVC_PrintConcise;
	self->_readDictionary = _SetVC_ReadDictionary;
	self->_getSet = _SetVC_GetSet;
	self->_getVariableCount = _SetVC_GetVariableCount;
	self->_getVariableIndex = _SetVC_GetVariableIndex;
	self->_getValueIndex = _SetVC_GetValueIndex;
	self->_getValueCount = _SetVC_GetValueCount;
	self->_getValue = _SetVC_GetValue;
	
	_Stg_Class_Init( (Stg_Class*)self );
	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );
	_Stg_Component_Init( (Stg_Component*)self );
	_VariableCondition_Init( (VariableCondition*)self, variable_Register, conFunc_Register, dictionary );
	
	
	/* Stg_Class info */
	_SetVC_Init( self,  _dictionaryEntryName );
}


SetVC* _SetVC_New( 
		SizeT						_sizeOfSelf, 
		Type						type,
		Stg_Class_DeleteFunction*				_delete,
		Stg_Class_PrintFunction*				_print,
		Stg_Class_CopyFunction*				_copy, 
		Stg_Component_DefaultConstructorFunction*	_defaultConstructor,
		Stg_Component_ConstructFunction*		_construct,
		Stg_Component_BuildFunction*			_build,
		Stg_Component_InitialiseFunction*			_initialise,
		Stg_Component_ExecuteFunction*			_execute,
		Stg_Component_DestroyFunction*		_destroy,
		Name							name,
		Bool							initFlag,
		VariableCondition_BuildSelfFunc*		_buildSelf, 
		VariableCondition_PrintConciseFunc*		_printConcise,
		VariableCondition_ReadDictionaryFunc*		_readDictionary,
		VariableCondition_GetSetFunc*			_getSet,
		VariableCondition_GetVariableCountFunc*		_getVariableCount,
		VariableCondition_GetVariableIndexFunc*		_getVariableIndex,
		VariableCondition_GetValueIndexFunc*		_getValueIndex,
		VariableCondition_GetValueCountFunc*		_getValueCount,
		VariableCondition_GetValueFunc*			_getValue,
		Name						_dictionaryEntryName, 
		Variable_Register*				variable_Register, 
		ConditionFunction_Register*			conFunc_Register,
		Dictionary*					dictionary )
{
	SetVC*	self;
	
	/* Allocate memory/General info */
	assert(_sizeOfSelf >= sizeof(SetVC));
	self = (SetVC*)_VariableCondition_New(
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
		name,
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
		conFunc_Register,
		dictionary );
	
	/* Virtual info */
	
	/* Stg_Class info */
	if( initFlag ){
		_SetVC_Init( self, _dictionaryEntryName );
	}
	
	return self;
}

void _SetVC_Init( void* setVC, Name _dictionaryEntryName ) {
	SetVC*			self = (SetVC*)setVC;
	
	self->isConstructed = True;
	self->_dictionaryEntryName = _dictionaryEntryName;
}	
	
/*--------------------------------------------------------------------------------------------------------------------------
** General virtual functions
*/

void _SetVC_ReadDictionary( void* setVC, void* dictionary ) {
	SetVC*			self = (SetVC*)setVC;
	Dictionary_Entry_Value*	vcDictVal;
	Dictionary_Entry_Value	_vcDictVal;
	Dictionary_Entry_Value*	varsVal;
	SetVC_Entry_Index	entry_I;
	
	
	/* Find dictionary entry */
	if (self->_dictionaryEntryName)
		vcDictVal = Dictionary_Get( dictionary, self->_dictionaryEntryName );
	else
	{
		vcDictVal = &_vcDictVal;
		Dictionary_Entry_Value_InitFromStruct( vcDictVal, dictionary );
	}
	
	if (vcDictVal)
	{
		Dictionary_Entry_Value*		setVal = Dictionary_Entry_Value_GetMember( vcDictVal, "indices" );
		Index				indexCnt = Dictionary_Entry_Value_AsUnsignedInt( 
							Dictionary_Entry_Value_GetMember( vcDictVal, "indexCount" ) );
		Index				i, cnt;
		
		self->_vcset = IndexSet_New( indexCnt );
		cnt = Dictionary_Entry_Value_GetCount( setVal );
		for( i = 0; i < cnt; i++ )
			IndexSet_Add( self->_vcset, Dictionary_Entry_Value_AsUnsignedInt( 
				Dictionary_Entry_Value_GetElement( setVal, i ) ) );
		
		/* Obtain the variable entries */
		varsVal = Dictionary_Entry_Value_GetMember(vcDictVal, "variables");
		self->_entryCount = Dictionary_Entry_Value_GetCount( varsVal );
		self->_entryTbl = Memory_Alloc_Array( SetVC_Entry, self->_entryCount, "SetVC->_entryTbl");
		
		for (entry_I = 0; entry_I < self->_entryCount; entry_I++)
		{
			char*			valType;
			Dictionary_Entry_Value*	valueEntry;
			Dictionary_Entry_Value*	varDictListVal;
			
			varDictListVal = Dictionary_Entry_Value_GetElement(varsVal, entry_I);
			valueEntry = Dictionary_Entry_Value_GetMember(varDictListVal, "value");
			
			self->_entryTbl[entry_I].varName = Dictionary_Entry_Value_AsString(
				Dictionary_Entry_Value_GetMember(varDictListVal, "name"));
				
			valType = Dictionary_Entry_Value_AsString(Dictionary_Entry_Value_GetMember(varDictListVal, "type"));
			if (!strcasecmp(valType, "func"))
			{
				char*	funcName = Dictionary_Entry_Value_AsString(valueEntry);
				
				self->_entryTbl[entry_I].value.type = VC_ValueType_CFIndex;
				self->_entryTbl[entry_I].value.as.typeCFIndex = ConditionFunction_Register_GetIndex(
					self->conFunc_Register, funcName);
			}
			else if (!strcasecmp(valType, "array"))
			{
				Dictionary_Entry_Value*	valueElement;
				Index			i;

				self->_entryTbl[entry_I].value.type = VC_ValueType_DoubleArray;
				self->_entryTbl[entry_I].value.as.typeArray.size = Dictionary_Entry_Value_GetCount(valueEntry);
				self->_entryTbl[entry_I].value.as.typeArray.array = Memory_Alloc_Array( double,
					self->_entryTbl[entry_I].value.as.typeArray.size, "SetVC->_entryTbl[].value.as.typeArray.array" );
					
				for (i = 0; i < self->_entryTbl[entry_I].value.as.typeArray.size; i++)
				{
					valueElement = Dictionary_Entry_Value_GetElement(valueEntry, i);
					self->_entryTbl[entry_I].value.as.typeArray.array[i] = 
						Dictionary_Entry_Value_AsDouble(valueElement);
				}
			}
			else if( !strcasecmp( valType, "double" ) || !strcasecmp( valType, "d" ) || !strcasecmp( valType, "float" ) || !strcasecmp( valType, "f" ) ) {
				self->_entryTbl[entry_I].value.type = VC_ValueType_Double;
				self->_entryTbl[entry_I].value.as.typeDouble = Dictionary_Entry_Value_AsDouble( valueEntry );
			}
			else if( !strcasecmp( valType, "integer" ) || !strcasecmp( valType, "int" ) || !strcasecmp( valType, "i" ) ) {
				self->_entryTbl[entry_I].value.type = VC_ValueType_Int;
				self->_entryTbl[entry_I].value.as.typeInt = Dictionary_Entry_Value_AsUnsignedInt( valueEntry );
			}
			else if( !strcasecmp( valType, "short" ) || !strcasecmp( valType, "s" ) ) {
				self->_entryTbl[entry_I].value.type = VC_ValueType_Short;
				self->_entryTbl[entry_I].value.as.typeShort = Dictionary_Entry_Value_AsUnsignedInt( valueEntry );
			}
			else if( !strcasecmp( valType, "char" ) || !strcasecmp( valType, "c" ) ) {
				self->_entryTbl[entry_I].value.type = VC_ValueType_Char;
				self->_entryTbl[entry_I].value.as.typeChar = Dictionary_Entry_Value_AsUnsignedInt( valueEntry );
			}
			else if( !strcasecmp( valType, "pointer" ) || !strcasecmp( valType, "ptr" ) || !strcasecmp( valType, "p" ) ) {
				self->_entryTbl[entry_I].value.type = VC_ValueType_Ptr;
				self->_entryTbl[entry_I].value.as.typePtr = (void*) ((ArithPointer) Dictionary_Entry_Value_AsUnsignedInt( valueEntry ));
			}
			else {
				/* Assume double */
				Journal_DPrintf( 
					Journal_Register( InfoStream_Type, "myStream" ), 
					"Type to variable on variable condition not given, assuming double\n" );
				self->_entryTbl[entry_I].value.type = VC_ValueType_Double;
				self->_entryTbl[entry_I].value.as.typeDouble = Dictionary_Entry_Value_AsDouble( valueEntry );
			}
		}
	}
	else
	{
		self->_entryCount = 0;
		self->_entryTbl = NULL;
	}
}

void _SetVC_Delete(void* setVC)
{
	SetVC*	self = (SetVC*)setVC;
	
	if (self->_entryTbl) Memory_Free( self->_entryTbl );
	
	/* Stg_Class_Delete parent */
	_VariableCondition_Delete(self);
}


void _SetVC_Print(void* setVC, Stream* stream)
{
	SetVC*				self = (SetVC*)setVC;
	SetVC_Entry_Index		entry_I;
	Index				i;
	
	/* Set the Journal for printing informations */
	Stream* info = stream;
	
	/* General info */
	Journal_Printf( info, "SetVC (ptr): %p\n", self);
	
	/* Virtual info */
	
	/* Stg_Class info */
	Journal_Printf( info, "\tdictionary (ptr): %p\n", self->dictionary);
	Journal_Printf( info, "\t_dictionaryEntryName (ptr): %p\n", self->_dictionaryEntryName);
	if (self->_dictionaryEntryName)
		Journal_Printf( info, "\t\t_dictionaryEntryName: %s\n", self->_dictionaryEntryName);
	Journal_Printf( info, "\t_entryCount: %u\n", self->_entryCount);
	Journal_Printf( info, "\t_entryTbl (ptr): %p\n", self->_entryTbl);
	if (self->_entryTbl)
		for (entry_I = 0; entry_I < self->_entryCount; entry_I++)
		{
			Journal_Printf( info, "\t\t_entryTbl[%u]:\n", entry_I);
			Journal_Printf( info, "\t\t\tvarName (ptr): %p\n", self->_entryTbl[entry_I].varName);
			if (self->_entryTbl[entry_I].varName)
				Journal_Printf( info, "\t\t\t\tvarName: %s\n", self->_entryTbl[entry_I].varName);
			Journal_Printf( info, "\t\t\tvalue:\n");
			switch (self->_entryTbl[entry_I].value.type)
			{
				case VC_ValueType_Double:
					Journal_Printf( info, "\t\t\t\ttype: VC_ValueType_Double\n" );
					Journal_Printf( info, "\t\t\t\tasDouble: %g\n", self->_entryTbl[entry_I].value.as.typeDouble );
					break;
					
				case VC_ValueType_Int:
					Journal_Printf( info, "\t\t\t\ttype: VC_ValueType_Int\n" );
					Journal_Printf( info, "\t\t\t\tasInt: %i\n", self->_entryTbl[entry_I].value.as.typeInt );
					break;
					
				case VC_ValueType_Short:
					Journal_Printf( info, "\t\t\t\ttype: VC_ValueType_Short\n" );
					Journal_Printf( info, "\t\t\t\tasShort: %i\n", self->_entryTbl[entry_I].value.as.typeShort );
					break;
					
				case VC_ValueType_Char:
					Journal_Printf( info, "\t\t\t\ttype: VC_ValueType_Char\n");
					Journal_Printf( info, "\t\t\t\tasChar: %c\n", self->_entryTbl[entry_I].value.as.typeChar );
					break;
					
				case VC_ValueType_Ptr:
					Journal_Printf( info, "\t\t\t\ttype: VC_ValueType_Ptr\n");
					Journal_Printf( info, "\t\t\t\tasPtr: %g\n", self->_entryTbl[entry_I].value.as.typePtr );
					break;
					
				case VC_ValueType_DoubleArray:
					Journal_Printf( info, "\t\t\t\ttype: VC_ValueType_DoubleArray\n");
					Journal_Printf( info, "\t\t\t\tarraySize: %u\n", self->_entryTbl[entry_I].value.as.typeArray.size);
					Journal_Printf( info, "\t\t\t\tasDoubleArray (ptr): %p\n", 
						self->_entryTbl[entry_I].value.as.typeArray.array);
					if (self->_entryTbl[entry_I].value.as.typeArray.array)
						for (i = 0; i < self->_entryTbl[entry_I].value.as.typeArray.size; i++)
							Journal_Printf( info, "\t\t\t\tasDoubleArray[%u]: %g\n", i,
								self->_entryTbl[entry_I].value.as.typeArray.array[i]);
					break;
					
				case VC_ValueType_CFIndex:
					Journal_Printf( info, "\t\t\t\ttype: VC_ValueType_CFIndex\n");
					Journal_Printf( info, "\t\t\t\tasCFIndex: %u\n", self->_entryTbl[entry_I].value.as.typeCFIndex);
					break;
			}
		}
	
	/* Print parent */
	_VariableCondition_Print(self);
}


void* _SetVC_Copy( void* setVC, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap ) {
	SetVC*		self = (SetVC*)setVC;
	SetVC*		newSetVC;
	PtrMap*		map = ptrMap;
	Bool		ownMap = False;
	
	if( !map ) {
		map = PtrMap_New( 10 );
		ownMap = True;
	}
	
	newSetVC = (SetVC*)_VariableCondition_Copy( self, dest, deep, nameExt, map );
	
	newSetVC->_dictionaryEntryName = self->_dictionaryEntryName;
	newSetVC->_entryCount = self->_entryCount;
	
	if( deep ) {
		newSetVC->_vcset = (IndexSet*)Stg_Class_Copy( self->_vcset, NULL, deep, nameExt, map );
		
		if( (newSetVC->_entryTbl = PtrMap_Find( map, self->_entryTbl )) == NULL && self->_entryTbl ) {
			newSetVC->_entryTbl = Memory_Alloc_Array( SetVC_Entry, newSetVC->_entryCount, "SetVC->_entryTbl");
			memcpy( newSetVC->_entryTbl, self->_entryTbl, sizeof(SetVC_Entry) * newSetVC->_entryCount );
			PtrMap_Append( map, newSetVC->_entryTbl, self->_entryTbl );
		}
	}
	else {
		newSetVC->_vcset = self->_vcset;
		newSetVC->_entryTbl = self->_entryTbl;
	}
	
	if( ownMap ) {
		Stg_Class_Delete( map );
	}
	
	return (void*)newSetVC;
}


/*--------------------------------------------------------------------------------------------------------------------------
** Macros
*/


/*--------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

IndexSet* _SetVC_GetSet( void* variableCondition ) {
	SetVC*		self = (SetVC*)variableCondition;
	
	return (IndexSet*) IndexSet_Duplicate( self->_vcset );
}


VariableCondition_VariableIndex _SetVC_GetVariableCount( void* variableCondition, Index globalIndex ) {
	SetVC*	self = (SetVC*)variableCondition;
	
	return self->_entryCount;
}


Variable_Index _SetVC_GetVariableIndex( void* variableCondition, Index globalIndex, VariableCondition_VariableIndex varIndex ) {
	SetVC*	self = (SetVC*)variableCondition;
	
	return Variable_Register_GetIndex(self->variable_Register, self->_entryTbl[varIndex].varName);
}


VariableCondition_ValueIndex _SetVC_GetValueIndex(
		void*				variableCondition, 
		Index				globalIndex, 
		VariableCondition_VariableIndex	varIndex )
{
	return varIndex;
}


VariableCondition_ValueIndex _SetVC_GetValueCount( void* variableCondition ) {
	SetVC*	self = (SetVC*)variableCondition;
	
	return self->_entryCount;
}


VariableCondition_Value _SetVC_GetValue( void* variableCondition, VariableCondition_ValueIndex valIndex ) {
	SetVC*	self = (SetVC*)variableCondition;

	return self->_entryTbl[valIndex].value;
}


void _SetVC_PrintConcise( void* variableCondition, Stream* stream ) {
	SetVC*		self = (SetVC*)variableCondition;
	IndexSet_Index	set_I;
	
	Journal_Printf( stream, "\ttype: %s, set: {", self->type );
	for( set_I = 0; set_I < self->_set->size; set_I++ ) {
		if( IndexSet_IsMember( self->_set, set_I ) ) {
			Journal_Printf( stream, "%u ", set_I );
		}
	}
	Journal_Printf( stream, "}\n" );
}

/*--------------------------------------------------------------------------------------------------------------------------
** Build functions
*/


/*--------------------------------------------------------------------------------------------------------------------------
** Functions
*/

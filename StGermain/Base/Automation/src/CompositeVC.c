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
** $Id: CompositeVC.c 3881 2006-10-26 03:14:19Z KathleenHumble $
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
#include "LiveComponentRegister.h"
#include "Variable.h"
#include "Variable_Register.h"
#include "VariableCondition.h"
#include "VariableCondition_Register.h"
#include "CompositeVC.h"

#include <string.h>
#include <assert.h>


const Type CompositeVC_Type = "CompositeVC";
const Name defaultCompositeVCName = "defaultCompositeVCName";

/*--------------------------------------------------------------------------------------------------------------------------
** Constructor
*/

VariableCondition* CompositeVC_Factory(
		Variable_Register*				variable_Register, 
		ConditionFunction_Register*			conFunc_Register,
		Dictionary*					dictionary,
		void*						data )
{
	return (VariableCondition*)CompositeVC_New( defaultCompositeVCName, variable_Register, conFunc_Register, dictionary, data );
}


CompositeVC* CompositeVC_New(
		Name						name,
		Variable_Register*				variable_Register, 
		ConditionFunction_Register*			conFunc_Register,
		Dictionary*					dictionary,
		void*						data )
{
	return _CompositeVC_New(
		sizeof(CompositeVC), 
		CompositeVC_Type, 
		_CompositeVC_Delete, 
		_CompositeVC_Print,
		_CompositeVC_Copy,
		(Stg_Component_DefaultConstructorFunction*)CompositeVC_DefaultNew,
		_CompositeVC_Construct,
		_CompositeVC_Build,
		_VariableCondition_Initialise,
		_VariableCondition_Execute,
		_VariableCondition_Destroy,
		name,
		True,
		NULL,
		_CompositeVC_PrintConcise,
		_CompositeVC_ReadDictionary,
		_CompositeVC_GetSet, 
		_CompositeVC_GetVariableCount, 
		_CompositeVC_GetVariableIndex, 
		_CompositeVC_GetValueIndex, 
		_CompositeVC_GetValueCount, 
		_CompositeVC_GetValue, 
		variable_Register, 
		conFunc_Register, 
		dictionary, 
		data );
}

CompositeVC* CompositeVC_DefaultNew( Name name )
{
	return (CompositeVC*)_CompositeVC_New(
		sizeof(CompositeVC), 
		CompositeVC_Type, 
		_CompositeVC_Delete, 
		_CompositeVC_Print,
		_CompositeVC_Copy,
		(Stg_Component_DefaultConstructorFunction*)CompositeVC_DefaultNew,
		_CompositeVC_Construct,
		_CompositeVC_Build,
		_VariableCondition_Initialise,
		_VariableCondition_Execute,
		_VariableCondition_Destroy,
		name,
		False,
		NULL,
		_CompositeVC_PrintConcise,
		_CompositeVC_ReadDictionary,
		_CompositeVC_GetSet, 
		_CompositeVC_GetVariableCount, 
		_CompositeVC_GetVariableIndex, 
		_CompositeVC_GetValueIndex, 
		_CompositeVC_GetValueCount, 
		_CompositeVC_GetValue, 
		NULL/*variable_Register*/, 
		NULL/*conFunc_Register*/,
		NULL,
		NULL );
}

void CompositeVC_Init(
		CompositeVC*					self, 
		Name						name,
		Variable_Register*				variable_Register, 
		ConditionFunction_Register*			conFunc_Register,
		Dictionary*					dictionary,
		void*						data )
{
	/* General info */
	self->type = CompositeVC_Type;
	self->_sizeOfSelf = sizeof(CompositeVC);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _CompositeVC_Delete;
	self->_print = _CompositeVC_Print;
	self->_copy = _CompositeVC_Copy;
	self->_build = _CompositeVC_Build;
	self->_initialise = _VariableCondition_Initialise;
	self->_execute = _VariableCondition_Execute;
	self->_buildSelf = NULL;
	self->_printConcise = _CompositeVC_PrintConcise;
	self->_readDictionary = _CompositeVC_ReadDictionary;
	self->_getSet = _CompositeVC_GetSet;
	self->_getVariableCount = _CompositeVC_GetVariableCount;
	self->_getVariableIndex = _CompositeVC_GetVariableIndex;
	self->_getValueIndex = _CompositeVC_GetValueIndex;
	self->_getValueCount = _CompositeVC_GetValueCount;
	self->_getValue = _CompositeVC_GetValue;
	
	_Stg_Class_Init( (Stg_Class*)self );
	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );
	_Stg_Component_Init( (Stg_Component*)self );
	_VariableCondition_Init( (VariableCondition*)self, variable_Register, conFunc_Register, dictionary );
	
	/* Stg_Class info */
	_CompositeVC_Init( self, data );
}


CompositeVC* _CompositeVC_New( 
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
		VariableCondition_GetVariableCountFunc		_getVariableCount,
		VariableCondition_GetVariableIndexFunc*		_getVariableIndex,
		VariableCondition_GetValueIndexFunc*		_getValueIndex,
		VariableCondition_GetValueCountFunc*		_getValueCount,
		VariableCondition_GetValueFunc*			_getValue,
		Variable_Register*				variable_Register,
		ConditionFunction_Register*			conFunc_Register,
		Dictionary*					dictionary,
		void*						data)
{
	CompositeVC*	self;
	
	/* Allocate memory/General info */
	assert(_sizeOfSelf >= sizeof(CompositeVC));
	self = (CompositeVC*)_VariableCondition_New(
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
		_CompositeVC_Init( self, data );
	}
	
	return self;
}


void _CompositeVC_Init(
		void*						compositeVC, 
		void*						data )
{
	CompositeVC*	self = (CompositeVC*)compositeVC;
	
	self->isConstructed = True;
	self->itemCount = 0;
	self->_size = 8;
	self->_delta = 8;
	self->itemTbl = Memory_Alloc_Array( VariableCondition*, self->_size, "CompositeCV->itemTbl" );
	memset(self->itemTbl, 0 , sizeof(VariableCondition*)*self->_size);
	self->iOwnTbl = Memory_Alloc_Array( Bool, self->_size, "CompositeCV->iOwnTbl" );
	memset(self->iOwnTbl, 0, sizeof(Bool)*self->_size);
	self->data = data;
	self->attachedSets = 0;
	self->hasReadDictionary = False;
}


/*--------------------------------------------------------------------------------------------------------------------------
** General virtual functions
*/

void _CompositeVC_ReadDictionary( void* compositeVC, void* dictionary ) {
	CompositeVC*	self = (CompositeVC*)compositeVC;

	/* View the dictionary as a list of variable conditions */
	if( dictionary && !self->hasReadDictionary ) {
		Dictionary_Entry_Value*	vcList;
		
		vcList = Dictionary_Get( dictionary, "vcList" );
		if( vcList ) {
			Index	count;
			Index	entry_I;
			
			count = Dictionary_Entry_Value_GetCount(vcList);
			for (entry_I = 0; entry_I < count; entry_I++)
			{
				Dictionary_Entry_Value*	vcEntry;
				Type			type;
				Dictionary*		dictionary;
				VariableCondition*	vc;
				
				vcEntry = Dictionary_Entry_Value_GetElement(vcList, entry_I);
				type = Dictionary_Entry_Value_AsString(Dictionary_Entry_Value_GetMember(vcEntry, "type"));
				dictionary = Dictionary_Entry_Value_AsDictionary(vcEntry);
				vc = VariableCondition_Register_CreateNew(variableCondition_Register, self->variable_Register, 
					self->conFunc_Register, type, dictionary, self->data );
				vc->_readDictionary( vc, dictionary );
				CompositeVC_Add(self, vc, True);
			}
			
		}
		self->hasReadDictionary = True;
	}
}

void _CompositeVC_Construct( void* compositeVC, Stg_ComponentFactory* cf, void* data ) {
	CompositeVC* self                      = (CompositeVC*)compositeVC;
	void*        variableRegister          = NULL;
	void*        conditionFunctionRegister = NULL;
	void*        initData                      = NULL;
	Dictionary*  vcDict                    = NULL;
	Name         vcName;
	
	self->dictionary = cf->rootDict;
	
	variableRegister = (void*)Stg_ObjectList_Get( cf->registerRegister, "Variable_Register" );
	assert( variableRegister );
	conditionFunctionRegister = (void*)Stg_ObjectList_Get( cf->registerRegister, "ConditionFunction_Register" );
	assert( conditionFunctionRegister );
	
	vcName = Stg_ComponentFactory_GetString( cf, self->name, "vcName", self->name );
	if ( cf->rootDict )
		vcDict = Dictionary_GetDictionary( cf->rootDict, vcName );
//	Journal_Firewall(
//			vcDict != NULL,
//			Journal_Register( Error_Type, self->type ),
//			"For %s '%s' - Cannont find variable condition dictionary '%s'\n", self->type, self->name, vcName );
	
	initData = Stg_ComponentFactory_ConstructByKey( cf, self->name, "Data", Stg_Component, False, data );
	
	_VariableCondition_Init( self, variableRegister, conditionFunctionRegister, vcDict );
	_CompositeVC_Init( self, initData );
}

void _CompositeVC_Delete(void* compositeVC)
{
	CompositeVC*	self = (CompositeVC*)compositeVC;
	
	if (self->itemTbl)
	{
		VariableCondition_Index	entry_I;
		
		for (entry_I = 0; entry_I < self->itemCount; entry_I++)
		{
			if (self->iOwnTbl[entry_I] && self->itemTbl[entry_I])
				Memory_Free(self->itemTbl[entry_I]);
		}
		
		Memory_Free(self->itemTbl);
	}
	
	if( self->attachedSets ) {
		Memory_Free( self->attachedSets );
	}
	
	/* Stg_Class_Delete parent */
	_VariableCondition_Delete(self);
}


void _CompositeVC_Print(void* compositeVC, Stream* stream)
{
	CompositeVC*	self = (CompositeVC*)compositeVC;
	Index		item_I;
	
	/* Set the Journal for printing informations */
	Stream* info = stream;
	
	/* General info */
	Journal_Printf( info, "CompositeVC (ptr): %p\n", self);
	
	/* Virtual info */
	
	/* Stg_Class info */
	Journal_Printf( info, "\tdictionary (ptr): %p\n", self->dictionary);
	Journal_Printf( info, "\titemCount: %u\n", self->itemCount );
	Journal_Printf( info, "\titemTbl (ptr): %p\n", self->itemTbl);
	if (self->itemTbl)
	{
		for (item_I = 0; item_I < self->itemCount; item_I++)
			Journal_Printf( info, "\t\titemTbl[%u] (ptr): %p\n", item_I, self->itemTbl[item_I]);
	}
	Journal_Printf( info, "\tiOwnTbl (ptr): %p\n", self->iOwnTbl);
	if (self->iOwnTbl)
	{
		for (item_I = 0; item_I < self->itemCount; item_I++)
			Journal_Printf( info, "\t\tiOwnTbl[%u]: %s\n", item_I, self->iOwnTbl[item_I] ? "True" : "False");
	}
	Journal_Printf( info, "\t_size: %lu\n", self->_size);
	Journal_Printf( info, "\t_delta: %lu\n", self->_delta);
	
	/* Print parent */
	_VariableCondition_Print(self);
}


void* _CompositeVC_Copy( void* compositeVC, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap ) {
	CompositeVC*	self = (CompositeVC*)compositeVC;
	CompositeVC*	newCompositeVC;
	PtrMap*		map = ptrMap;
	Bool		ownMap = False;
	
	if( !map ) {
		map = PtrMap_New( 10 );
		ownMap = True;
	}
	
	newCompositeVC = (CompositeVC*)_VariableCondition_Copy( self, dest, deep, nameExt, map );
	
	newCompositeVC->itemCount = self->itemCount;
	newCompositeVC->_size = self->_size;
	newCompositeVC->_delta = self->_delta;
	newCompositeVC->hasReadDictionary = self->hasReadDictionary;
	newCompositeVC->data = self->data;
	
	if( deep ) {
		if( (newCompositeVC->itemTbl = PtrMap_Find( map, self->itemTbl )) == NULL && self->itemTbl ) {
			Index	item_I;
			
			newCompositeVC->itemTbl = Memory_Alloc_Array( VariableCondition*, newCompositeVC->_size, "CompositeCV->itemTbl" );
			memset( newCompositeVC->itemTbl, 0, sizeof(VariableCondition*) * newCompositeVC->_size );
			for( item_I = 0; item_I < self->itemCount; item_I++ ) {
				newCompositeVC->itemTbl[item_I] = (VariableCondition*)Stg_Class_Copy( self->itemTbl[item_I], NULL, deep, nameExt, map );
			}
			PtrMap_Append( map, newCompositeVC->itemTbl, self->itemTbl );
		}
		
		if( (newCompositeVC->iOwnTbl = PtrMap_Find( map, self->iOwnTbl )) == NULL && self->iOwnTbl ) {
			newCompositeVC->iOwnTbl = Memory_Alloc_Array( Bool, newCompositeVC->_size, "CompositeCV->iOwnTbl" );
			memcpy( newCompositeVC->iOwnTbl, self->iOwnTbl, sizeof(Bool) * newCompositeVC->_size );
			PtrMap_Append( map, newCompositeVC->iOwnTbl, self->iOwnTbl );
		}
		
		if( (newCompositeVC->attachedSets = PtrMap_Find( map, self->attachedSets )) == NULL && self->attachedSets ) {
			Index	item_I;
			
			self->attachedSets = Memory_Alloc_Array( IndexSet*, newCompositeVC->itemCount, "CompositeCV->attachedSets" );
			for( item_I = 0; item_I < self->itemCount; item_I++ ) {
				newCompositeVC->attachedSets[item_I] = (IndexSet*)Stg_Class_Copy( self->attachedSets[item_I], NULL, deep, nameExt, map );
			}
			PtrMap_Append( map, newCompositeVC->attachedSets, self->attachedSets );
		}
	}
	else {
		newCompositeVC->itemTbl = self->itemTbl;
		newCompositeVC->iOwnTbl = self->iOwnTbl;
		newCompositeVC->attachedSets = self->attachedSets;
	}
	
	if( ownMap ) {
		Stg_Class_Delete( map );
	}
	
	return (void*)newCompositeVC;
}


/*--------------------------------------------------------------------------------------------------------------------------
** Macros
*/


/*--------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _CompositeVC_Build( void* compositeVC, void* data ) {
	CompositeVC*	self = (CompositeVC*)compositeVC;
	Index		index;
	
	/* Read the dictionary... we have to do this early to get self->itemCount filled in. Hence we keep a flag to ensure we
		dont read twice */
	self->_readDictionary( self, self->dictionary );
	
	/* Build a temporary array to store the sets of attached VCs for the build process and clear it afterward */
	if( self->itemCount ) {
		Index			item_I;
		
		self->attachedSets = Memory_Alloc_Array( IndexSet*, self->itemCount, "CompositeCV->attachedSets" );
		memset( self->attachedSets, 0, sizeof(IndexSet*) * self->itemCount );
		
		/* This guy is here so that a VC that was created by the composite VC can do any
		   build stuff it needs, without calling _VariableCondition_Build for itself. */
		for( item_I = 0; item_I < self->itemCount; item_I++ ) {
			VariableCondition_BuildSelf( self->itemTbl[item_I], data );
		}
		
		_VariableCondition_Build( self, data );
		for( index = 0; index < self->itemCount; index++ ) {
			Stg_Class_Delete( self->attachedSets[index] );
		}
		Memory_Free( self->attachedSets );
		self->attachedSets = 0;
	}
}


IndexSet* _CompositeVC_GetSet( void* compositeVC ) {
	CompositeVC*	self = (CompositeVC*)compositeVC;
	IndexSet*	set = NULL;
	Index		i;
	
	/* Use the first attachment to obtain the size, etc. Be effecient by duplicating if for the my own set */
	if( self->itemCount ) {
		self->attachedSets[0] = self->itemTbl[0]->_getSet( self->itemTbl[0] );
		set = IndexSet_Duplicate( self->attachedSets[0] );
	}
	
	/* For the remainder of the attachments, OR in their sets */
	for( i = 1; i < self->itemCount; i++ ) {
		self->attachedSets[i] = self->itemTbl[i]->_getSet( self->itemTbl[i] );
		IndexSet_Merge_OR( set, self->attachedSets[i] );
	}
	
	return set;
}

	
Variable_Index _CompositeVC_GetVariableCount( void* compositeVC, Index globalIndex ) {
	CompositeVC*	self = (CompositeVC*)compositeVC;
	Variable_Index*	varIndices;
	Variable_Index	varIndexCount = 0;
	Index		i;
	
	varIndices = Memory_Alloc_Array( Variable_Index, self->variable_Register->count, "varIndices" );
		
	for( i = 0; i < self->itemCount; i++ ) {
		if( IndexSet_IsMember( self->attachedSets[i], globalIndex ) ) {
			Index	varCount = self->itemTbl[i]->_getVariableCount( self->itemTbl[i], globalIndex );
			Index	j;
		
			for(j = 0; j < varCount; j++ ) {
				Index	newVarIndex = self->itemTbl[i]->_getVariableIndex(self->itemTbl[i], globalIndex, j);
				Index	k;
			
				for (k = 0; k < varIndexCount; k++)
					if (varIndices[k] == newVarIndex)
						break;
				if (k < varIndexCount)
					continue;
				
				varIndices[varIndexCount++] = newVarIndex;
			}
		}
	}
	
	if (varIndices) Memory_Free(varIndices);
	
	return varIndexCount;
}

	
Variable_Index _CompositeVC_GetVariableIndex(
		void*				compositeVC, 
		Index				globalIndex, 
		VariableCondition_VariableIndex	varIndex)
{
	CompositeVC*	self = (CompositeVC*)compositeVC;
	Variable_Index*	varIndices;
	Variable_Index	varIndexCount = 0;
	Index		i;
	
	varIndices = Memory_Alloc_Array( Variable_Index, self->variable_Register->count, "varIndices" );
		
	for( i = 0; i < self->itemCount; i++ ) {
		if( IndexSet_IsMember( self->attachedSets[i], globalIndex ) ) {
			Index	varCount = self->itemTbl[i]->_getVariableCount(self->itemTbl[i], globalIndex);
			Index	j;
		
			for (j = 0; j < varCount; j++)
			{
				Index	newVarIndex = self->itemTbl[i]->_getVariableIndex(self->itemTbl[i], globalIndex, j);
				Index	k;
			
				for (k = 0; k < varIndexCount; k++)
					if (varIndices[k] == newVarIndex)
						break;
				if (k < varIndexCount)
					continue;
			
				if (varIndexCount == varIndex)
				{
					if (varIndices) Memory_Free(varIndices);
					return newVarIndex;
				}
			
				varIndices[varIndexCount++] = newVarIndex;
			}
		}
	}
	
	if (varIndices) Memory_Free(varIndices);
	
	return (Variable_Index)-1;
}


VariableCondition_ValueIndex _CompositeVC_GetValueIndex(
				void*				compositeVC, 
				Index				globalIndex, 
				VariableCondition_VariableIndex	varIndex)
{
	CompositeVC*			self = (CompositeVC*)compositeVC;
	VariableCondition_ValueIndex	valIndex = (VariableCondition_ValueIndex)-1;
	CompositeVC_ItemIndex		valItem = 0;
	Variable_Index			varRegIndex = self->_getVariableIndex(self, globalIndex, varIndex);
	Index				i;
	
	for( i = self->itemCount; i > 0; i-- ) {
		if( IndexSet_IsMember( self->attachedSets[i-1], globalIndex ) ) {
			Index	j;
			
			for (j = 0; j < self->itemTbl[i - 1]->_getVariableCount(self->itemTbl[i - 1], globalIndex); j++)
				if (self->itemTbl[i - 1]->_getVariableIndex(self->itemTbl[i - 1], globalIndex, j) == 
					varRegIndex)
					break;
			if (j == self->itemTbl[i - 1]->_getVariableCount(self->itemTbl[i - 1], globalIndex))
				continue;
			
			valItem = i - 1;
			valIndex = self->itemTbl[valItem]->_getValueIndex(self->itemTbl[valItem], globalIndex, j);
			break;
		}
	}
	
	for (i = 0; i < valItem; i++)
		valIndex += self->itemTbl[i]->_getValueCount(self->itemTbl[i]);
	
	return valIndex;
}

	
VariableCondition_ValueIndex _CompositeVC_GetValueCount(void* compositeVC)
{
	CompositeVC*			self = (CompositeVC*)compositeVC;
	VariableCondition_ValueIndex	valCount = 0;
	Index				i;
	
	for (i = 0; i < self->itemCount; i++)
		valCount += self->itemTbl[i]->_getValueCount(self->itemTbl[i]);
		
	return valCount;
}

	
VariableCondition_Value _CompositeVC_GetValue(void* compositeVC, VariableCondition_ValueIndex valIndex)
{
	CompositeVC*			self = (CompositeVC*)compositeVC;
	VariableCondition_ValueIndex	valPos = valIndex;
	Index				i = 0;
	
	while (valPos >= self->itemTbl[i]->_getValueCount(self->itemTbl[i]))
	{
		valPos -= self->itemTbl[i]->_getValueCount(self->itemTbl[i]);
		i++;
/*		if (++i >= self->itemCount)	*/
			/* Then what? */
	}
	
	return self->itemTbl[i]->_getValue(self->itemTbl[i], valPos);
}


void _CompositeVC_PrintConcise( void* compositeVC, Stream* stream ) {
	CompositeVC*	self = (CompositeVC*)compositeVC;
	Index		item_I;
	
	Journal_Printf( stream, "\ttype: %s, {\n", self->type );
	if( self->itemTbl ) {
		for( item_I = 0; item_I < self->itemCount; item_I++ ) {
			VariableCondition_PrintConcise( self->itemTbl[item_I], stream );
		}
	}
	Journal_Printf( stream, "}\n" );
}

/*--------------------------------------------------------------------------------------------------------------------------
** Build functions
*/

CompositeVC_ItemIndex CompositeVC_Add(void* compositeVC, void* variableCondition, Bool iOwn)
{
	CompositeVC*		self = (CompositeVC*)compositeVC;
	CompositeVC_ItemIndex	handle;
	
	if (self->itemCount >= self->_size)
	{
		SizeT currentSize = self->_size;
		
		self->_size += self->_delta;

		self->itemTbl = Memory_Realloc_Array( self->itemTbl, VariableCondition*, self->_size );
		memset( (Pointer)((ArithPointer)self->itemTbl + (sizeof(VariableCondition*) * currentSize) ),
			0, sizeof(VariableCondition*) * (self->_size - currentSize) );

		self->iOwnTbl = Memory_Realloc_Array( self->iOwnTbl, Bool, self->_size );
		memset( (Pointer)((ArithPointer)self->iOwnTbl + (sizeof(Bool) * currentSize )),
			0, sizeof(Bool) * (self->_size - currentSize ) );
	}
	
	handle = self->itemCount++;
	self->itemTbl[handle] = (VariableCondition*) variableCondition;
	self->iOwnTbl[handle] = iOwn;
	
	return handle;
}


/*--------------------------------------------------------------------------------------------------------------------------
** Functions
*/

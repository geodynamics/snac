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
** $Id: WallVC.c 3884 2006-10-26 05:26:19Z KathleenHumble $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"
#include "Discretisation/Mesh/Mesh.h"

#include "types.h"
#include "WallVC.h"
#include "RegularMeshUtils.h"

#include <string.h>
#include <assert.h>


const Type WallVC_Type = "WallVC";
const Name defaultWallVCName = "defaultWallVCName";

const char* WallVC_WallEnumToStr[WallVC_Wall_Size] = {
	"back",
	"left",
	"bottom",
	"right",
	"top",
	"front" };


/*--------------------------------------------------------------------------------------------------------------------------
** Constructor
*/

VariableCondition* WallVC_Factory(
		Variable_Register*				variable_Register, 
		ConditionFunction_Register*			conFunc_Register, 
		Dictionary*					dictionary,
		void*						data )
{
	return (VariableCondition*)WallVC_New( defaultWallVCName, NULL, variable_Register, conFunc_Register, dictionary, (Mesh*)data );
}


WallVC*	WallVC_DefaultNew( Name name )
{
	return _WallVC_New(
		sizeof(WallVC), 
		WallVC_Type, 
		_WallVC_Delete, 
		_WallVC_Print, 
		_WallVC_Copy,
		(Stg_Component_DefaultConstructorFunction*)WallVC_DefaultNew,
		_WallVC_Construct,	
		_WallVC_Build,
		_VariableCondition_Initialise,
		_VariableCondition_Execute,
		_VariableCondition_Destroy,
		name,
		False,
		_WallVC_BuildSelf, 
		_WallVC_PrintConcise,
		_WallVC_ReadDictionary,
		_WallVC_GetSet, 
		_WallVC_GetVariableCount, 
		_WallVC_GetVariableIndex, 
		_WallVC_GetValueIndex, 
		_WallVC_GetValueCount, 
		_WallVC_GetValue,
		NULL,
		NULL, 
		NULL, 
		NULL, 
		NULL);
}

WallVC*	WallVC_New(
		Name						name,
		Name						_dictionaryEntryName, 
		Variable_Register*				variable_Register, 
		ConditionFunction_Register*			conFunc_Register, 
		Dictionary*					dictionary,
		void*						_mesh )
{
	return _WallVC_New(
		sizeof(WallVC), 
		WallVC_Type, 
		_WallVC_Delete, 
		_WallVC_Print, 
		_WallVC_Copy,
		(Stg_Component_DefaultConstructorFunction*)WallVC_DefaultNew,
		_WallVC_Construct,	
		_WallVC_Build,
		_VariableCondition_Initialise,
		_VariableCondition_Execute,
		_VariableCondition_Destroy,
		name,
		True,
		_WallVC_BuildSelf, 
		_WallVC_PrintConcise,
		_WallVC_ReadDictionary,
		_WallVC_GetSet, 
		_WallVC_GetVariableCount, 
		_WallVC_GetVariableIndex, 
		_WallVC_GetValueIndex, 
		_WallVC_GetValueCount, 
		_WallVC_GetValue,
		_dictionaryEntryName,
		variable_Register, 
		conFunc_Register, 
		dictionary, 
		_mesh );
}


void WallVC_Init(
		WallVC*						self,
		Name						name,
		Name						_dictionaryEntryName, 
		Variable_Register*				variable_Register, 
		ConditionFunction_Register*			conFunc_Register, 
		Dictionary*					dictionary,
		void*						_mesh )
{
	/* General info */
	self->type = WallVC_Type;
	self->_sizeOfSelf = sizeof(WallVC);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _WallVC_Delete;
	self->_print = _WallVC_Print;
	self->_copy = _WallVC_Copy;
	self->_defaultConstructor = (Stg_Component_DefaultConstructorFunction*)WallVC_DefaultNew;
	self->_construct = _WallVC_Construct;
	self->_build = _WallVC_Build;
	self->_initialise = _VariableCondition_Initialise;
	self->_execute = _VariableCondition_Execute;
	self->_destroy = _VariableCondition_Destroy;
	self->_buildSelf = _WallVC_BuildSelf;
	self->_printConcise = _WallVC_PrintConcise;
	self->_readDictionary = _WallVC_ReadDictionary;
	self->_getSet = _WallVC_GetSet;
	self->_getVariableCount = _WallVC_GetVariableCount;
	self->_getVariableIndex = _WallVC_GetVariableIndex;
	self->_getValueIndex = _WallVC_GetValueIndex;
	self->_getValueCount = _WallVC_GetValueCount;
	self->_getValue = _WallVC_GetValue;
	
	_Stg_Class_Init( (Stg_Class*)self );
	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );
	_Stg_Component_Init( (Stg_Component*)self );
	_VariableCondition_Init( (VariableCondition*)self, variable_Register, conFunc_Register, dictionary );
	
	/* Stg_Class info */
	_WallVC_Init( self, _dictionaryEntryName, _mesh );
}


WallVC* _WallVC_New( 
		SizeT						_sizeOfSelf, 
		Type						type,
		Stg_Class_DeleteFunction*				_delete,
		Stg_Class_PrintFunction*				_print,
		Stg_Class_CopyFunction*				_copy, 
		Stg_Component_DefaultConstructorFunction*	_defaultConstructor,
		Stg_Component_ConstructFunction*			_construct,
		Stg_Component_BuildFunction*			_build,
		Stg_Component_InitialiseFunction*			_initialise,
		Stg_Component_ExecuteFunction*			_execute,
		Stg_Component_DestroyFunction*			_destroy,
		Name								name, 
		Bool								initFlag,
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
		Dictionary*					dictionary,
		void*						_mesh)
{
	WallVC*	self;
	
	/* Allocate memory/General info */
	assert(_sizeOfSelf >= sizeof(WallVC));
	self = (WallVC*)_VariableCondition_New(
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
		_WallVC_Init( self, _dictionaryEntryName, _mesh );
	}
	
	return self;
}


void _WallVC_Init(
		void*						wallVC, 
		Name						_dictionaryEntryName, 
		void*						_mesh )
{
	WallVC*			self = (WallVC*)wallVC;

	self->isConstructed = True;
	self->_dictionaryEntryName = _dictionaryEntryName;
	self->_mesh = (Mesh*)_mesh;
	self->_wall = WallVC_Wall_Size;
	self->_entryTbl = 0;
	self->_entryCount = 0;
}


/*--------------------------------------------------------------------------------------------------------------------------
** General virtual functions
*/

void _WallVC_ReadDictionary( void* variableCondition, void* dictionary ) {
	WallVC*			self = (WallVC*)variableCondition;
	Dictionary_Entry_Value*	vcDictVal;
	Dictionary_Entry_Value	_vcDictVal;
	Dictionary_Entry_Value*	varsVal;
	WallVC_Entry_Index	entry_I;
	
	/* Find dictionary entry */
	if (self->_dictionaryEntryName)
		vcDictVal = Dictionary_Get(dictionary, self->_dictionaryEntryName);
	else
	{
		vcDictVal = &_vcDictVal;
		Dictionary_Entry_Value_InitFromStruct(vcDictVal, dictionary);
	}
	
	if (vcDictVal)
	{
		char*	wallStr;
		
		/* Obtain which wall */
		wallStr = Dictionary_Entry_Value_AsString(Dictionary_Entry_Value_GetMember(vcDictVal, "wall" ));
		if (!strcasecmp(wallStr, "back"))
			self->_wall = WallVC_Wall_Back;
		else if (!strcasecmp(wallStr, "left"))
			self->_wall = WallVC_Wall_Left;
		else if (!strcasecmp(wallStr, "bottom"))
			self->_wall = WallVC_Wall_Bottom;
		else if (!strcasecmp(wallStr, "right"))
			self->_wall = WallVC_Wall_Right;
		else if (!strcasecmp(wallStr, "top"))
			self->_wall = WallVC_Wall_Top;
		else if (!strcasecmp(wallStr, "front"))
			self->_wall = WallVC_Wall_Front;
		else {
			assert( 0 );
			self->_wall = WallVC_Wall_Size; /* invalid entry */
		}
		
		/* Obtain the variable entries */
		self->_entryCount = Dictionary_Entry_Value_GetCount(Dictionary_Entry_Value_GetMember(vcDictVal, "variables"));
		self->_entryTbl = Memory_Alloc_Array( WallVC_Entry, self->_entryCount, "WallVC->_entryTbl" );
		varsVal = Dictionary_Entry_Value_GetMember(vcDictVal, "variables");
		
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
			if (0 == strcasecmp(valType, "func"))
			{
				char*	funcName = Dictionary_Entry_Value_AsString(valueEntry);
				Index	cfIndex;
				
				self->_entryTbl[entry_I].value.type = VC_ValueType_CFIndex;
				cfIndex = ConditionFunction_Register_GetIndex( self->conFunc_Register, funcName);
				if ( cfIndex == (unsigned)-1 ) {	
					Stream*	errorStr = Journal_Register( Error_Type, self->type );

					Journal_Printf( errorStr, "Error- in %s: While parsing "
						"definition of wallVC \"%s\" (applies to wall \"%s\"), the cond. func. applied to "
						"variable \"%s\" - \"%s\" - wasn't found in the c.f. register.\n",
						__func__, self->_dictionaryEntryName, WallVC_WallEnumToStr[self->_wall],
						self->_entryTbl[entry_I].varName, funcName );
					Journal_Printf( errorStr, "(Available functions in the C.F. register are: ");	
					ConditionFunction_Register_PrintNameOfEachFunc( self->conFunc_Register, errorStr );
					Journal_Printf( errorStr, ")\n");	
					assert(0);
				}	
				self->_entryTbl[entry_I].value.as.typeCFIndex = cfIndex;
			}
			else if (0 == strcasecmp(valType, "array"))
			{
				Dictionary_Entry_Value*	valueElement;
				Index			i;

				self->_entryTbl[entry_I].value.type = VC_ValueType_DoubleArray;
				self->_entryTbl[entry_I].value.as.typeArray.size = Dictionary_Entry_Value_GetCount(valueEntry);
				self->_entryTbl[entry_I].value.as.typeArray.array = Memory_Alloc_Array( double,
					self->_entryTbl[entry_I].value.as.typeArray.size, "WallVC->_entryTbl[].value.as.typeArray.array" );
					
				for (i = 0; i < self->_entryTbl[entry_I].value.as.typeArray.size; i++)
				{
					valueElement = Dictionary_Entry_Value_GetElement(valueEntry, i);
					self->_entryTbl[entry_I].value.as.typeArray.array[i] = 
						Dictionary_Entry_Value_AsDouble(valueElement);
				}
			}
			else if( 0 == strcasecmp( valType, "double" ) || 0 == strcasecmp( valType, "d" ) ||
				0 == strcasecmp( valType, "float" ) || 0 == strcasecmp( valType, "f" ) )
			{
				self->_entryTbl[entry_I].value.type = VC_ValueType_Double;
				self->_entryTbl[entry_I].value.as.typeDouble = Dictionary_Entry_Value_AsDouble( valueEntry );
			}
			else if( 0 == strcasecmp( valType, "integer" ) || 0 == strcasecmp( valType, "int" ) || 0 == strcasecmp( valType, "i" ) ) {
				self->_entryTbl[entry_I].value.type = VC_ValueType_Int;
				self->_entryTbl[entry_I].value.as.typeInt = Dictionary_Entry_Value_AsUnsignedInt( valueEntry );
			}
			else if( 0 == strcasecmp( valType, "short" ) || 0 == strcasecmp( valType, "s" ) ) {
				self->_entryTbl[entry_I].value.type = VC_ValueType_Short;
				self->_entryTbl[entry_I].value.as.typeShort = Dictionary_Entry_Value_AsUnsignedInt( valueEntry );
			}
			else if( 0 == strcasecmp( valType, "char" ) || 0 == strcasecmp( valType, "c" ) ) {
				self->_entryTbl[entry_I].value.type = VC_ValueType_Char;
				self->_entryTbl[entry_I].value.as.typeChar = Dictionary_Entry_Value_AsUnsignedInt( valueEntry );
			}
			else if( 0 == strcasecmp( valType, "pointer" ) || 0 == strcasecmp( valType, "ptr" ) || 0 == strcasecmp( valType, "p" ) ) {
				self->_entryTbl[entry_I].value.type = VC_ValueType_Ptr;
				self->_entryTbl[entry_I].value.as.typePtr = (void*) ( (ArithPointer)Dictionary_Entry_Value_AsUnsignedInt( valueEntry ));
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
		self->_wall = WallVC_Wall_Size;
		self->_entryCount = 0;
		self->_entryTbl = NULL;
	}
}


void _WallVC_Delete(void* wallVC)
{
	WallVC*	self = (WallVC*)wallVC;
	
	if (self->_entryTbl) Memory_Free(self->_entryTbl);
	
	/* Stg_Class_Delete parent */
	_VariableCondition_Delete(self);
}


void _WallVC_Print(void* wallVC, Stream* stream)
{
	WallVC*				self = (WallVC*)wallVC;
	WallVC_Entry_Index		entry_I;
	Index				i;
	
	/* Set the Journal for printing informations */
	Stream* info = stream;
	
	/* General info */
	Journal_Printf( info, "WallVC (ptr): %p\n", self);
	
	/* Virtual info */
	
	/* Stg_Class info */
	Journal_Printf( info, "\tdictionary (ptr): %p\n", self->dictionary);
	Journal_Printf( info, "\t_dictionaryEntryName (ptr): %p\n", self->_dictionaryEntryName);
	if (self->_dictionaryEntryName)
		Journal_Printf( info, "\t\t_dictionaryEntryName: %s\n", self->_dictionaryEntryName);
	Journal_Printf( info, "\t_wall: %s\n", self->_wall == WallVC_Wall_Front ? "Front" :
		self->_wall == WallVC_Wall_Back ? "Back" : self->_wall == WallVC_Wall_Left ? "Left" :
		self->_wall == WallVC_Wall_Right ? "Right" : self->_wall == WallVC_Wall_Top ? "Top" :
		self->_wall == WallVC_Wall_Bottom ? "Bottom" : "None");
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
	Journal_Printf( info, "\t_mesh (ptr): %p\n", self->_mesh);
	
	/* Print parent */
	_VariableCondition_Print(self);
}


void* _WallVC_Copy( void* wallVC, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap ) {
	WallVC*		self = (WallVC*)wallVC;
	WallVC*		newWallVC;
	PtrMap*		map = ptrMap;
	Bool		ownMap = False;
	
	if( !map ) {
		map = PtrMap_New( 10 );
		ownMap = True;
	}
	
	newWallVC = (WallVC*)_VariableCondition_Copy( self, dest, deep, nameExt, map );
	
	newWallVC->_dictionaryEntryName = self->_dictionaryEntryName;
	newWallVC->_wall = self->_wall;
	newWallVC->_entryCount = self->_entryCount;
	
	if( deep ) {
		newWallVC->_mesh = (Mesh*)Stg_Class_Copy( self->_mesh, NULL, deep, nameExt, map );
		
		if( (newWallVC->_entryTbl = PtrMap_Find( map, self->_entryTbl )) == NULL && self->_entryTbl ) {
			newWallVC->_entryTbl = Memory_Alloc_Array( WallVC_Entry, newWallVC->_entryCount, "WallVC->_entryTbl");
			memcpy( newWallVC->_entryTbl, self->_entryTbl, sizeof(WallVC_Entry) * newWallVC->_entryCount );
			PtrMap_Append( map, newWallVC->_entryTbl, self->_entryTbl );
		}
	}
	else {
		newWallVC->_mesh = self->_mesh;
		newWallVC->_entryTbl = self->_entryTbl;
	}
	
	if( ownMap ) {
		Stg_Class_Delete( map );
	}
	
	return (void*)newWallVC;
}


void _WallVC_Build(  void* wallVC, void* data ) {
	WallVC*			self = (WallVC*)wallVC;
	
	_WallVC_BuildSelf( self, data );
	
	_VariableCondition_Build( self, data );
}
	

/*--------------------------------------------------------------------------------------------------------------------------
** Macros
*/


/*--------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _WallVC_Construct( void* wallVC, Stg_ComponentFactory* cf, void* data )
{
	
}

void _WallVC_BuildSelf(  void* wallVC, void* data ) {
	WallVC*			self = (WallVC*)wallVC;
	
	if( self->_mesh ) {
		Build( self->_mesh, data, False );
	}
}


IndexSet* _WallVC_GetSet(void* variableCondition)
{
	WallVC*		self = (WallVC*)variableCondition;
	IndexSet	*set = NULL;
	HexaMD*		hexaMD = (HexaMD*)self->_mesh->layout->decomp;
	Stream*		warningStr = Journal_Register( Error_Type, self->type );

	
	switch (self->_wall) {
		case WallVC_Wall_Front:
			if ( 1 == hexaMD->nodeGlobal3DCounts[K_AXIS] ) {
				Journal_Printf( warningStr, "Warning - in %s: Can't build a %s wall VC "
					"when mesh has no elements in the %s axis. Returning an empty set.\n", __func__,
					WallVC_WallEnumToStr[self->_wall], "K" );
				set = IndexSet_New(hexaMD->nodeDomainCount);	
			}
			else {
				set = RegularMeshUtils_CreateGlobalFrontSet(self->_mesh);
			}
			break;
			
		case WallVC_Wall_Back:
			if ( 1 == hexaMD->nodeGlobal3DCounts[K_AXIS] ) {
				Journal_Printf( warningStr, "Warning - in %s: Can't build a %s wall VC "
					"when mesh has no elements in the %s axis. Returning an empty set.\n", __func__,
					WallVC_WallEnumToStr[self->_wall], "K" );
				set = IndexSet_New(hexaMD->nodeDomainCount);	
			}
			else {
				set = RegularMeshUtils_CreateGlobalBackSet(self->_mesh);
			}	
			break;
			
		case WallVC_Wall_Top:
			if ( 1 == hexaMD->nodeGlobal3DCounts[J_AXIS] ) {
				Journal_Printf( warningStr, "Warning - in %s: Can't build a %s wall VC "
					"when mesh has no elements in the %s axis. Returning an empty set.\n", __func__,
					WallVC_WallEnumToStr[self->_wall], "J" );
				set = IndexSet_New(hexaMD->nodeDomainCount);	
			}
			else {
				set = RegularMeshUtils_CreateGlobalTopSet(self->_mesh);
			}	
			break;
			
		case WallVC_Wall_Bottom:
			if ( 1 == hexaMD->nodeGlobal3DCounts[J_AXIS] ) {
				Journal_Printf( warningStr, "Warning - in %s: Can't build a %s wall VC "
					"when mesh has no elements in the %s axis. Returning an empty set.\n", __func__,
					WallVC_WallEnumToStr[self->_wall], "J" );
				set = IndexSet_New(hexaMD->nodeDomainCount);	
			}
			else {
				set = RegularMeshUtils_CreateGlobalBottomSet(self->_mesh);
			}	
			break;
			
		case WallVC_Wall_Left:
			if ( 1 == hexaMD->nodeGlobal3DCounts[I_AXIS] ) {
				Journal_Printf( warningStr, "Warning - in %s: Can't build a %s wall VC "
					"when mesh has no elements in the %s axis. Returning an empty set.\n", __func__,
					WallVC_WallEnumToStr[self->_wall], "I" );
				set = IndexSet_New(hexaMD->nodeDomainCount);	
			}
			else {
				set = RegularMeshUtils_CreateGlobalLeftSet(self->_mesh);
			}	
			break;
			
		case WallVC_Wall_Right:
			if ( 1 == hexaMD->nodeGlobal3DCounts[I_AXIS] ) {
				Journal_Printf( warningStr, "Warning - in %s: Can't build a %s wall VC "
					"when mesh has no elements in the %s axis. Returning an empty set.\n", __func__,
					WallVC_WallEnumToStr[self->_wall], "I" );
				set = IndexSet_New(hexaMD->nodeDomainCount);	
			}
			else {
				set = RegularMeshUtils_CreateGlobalRightSet(self->_mesh);
			}
			break;
			
		case WallVC_Wall_Size:
		default:
			assert(0);
			break;
	}
	
	return set;
}


VariableCondition_VariableIndex _WallVC_GetVariableCount(void* variableCondition, Index globalIndex)
{
	WallVC*	self = (WallVC*)variableCondition;
	
	return self->_entryCount;
}


Variable_Index _WallVC_GetVariableIndex(void* variableCondition, Index globalIndex, VariableCondition_VariableIndex varIndex)
{
	WallVC*		self = (WallVC*)variableCondition;
	Variable_Index	searchedIndex = 0;
	Stream*		errorStr = Journal_Register( Error_Type, self->type );
	Name		varName;
	
	varName = self->_entryTbl[varIndex].varName;
	searchedIndex = Variable_Register_GetIndex(self->variable_Register, varName );
	
	Journal_Firewall( ( searchedIndex < self->variable_Register->count ), errorStr, "Error- in %s: searching for index of "
		"varIndex %u (\"%s\") at global node number %u failed - register returned index %u, greater than "
		"count %u.\n", __func__, varIndex, varName, globalIndex, searchedIndex, self->variable_Register->count );

	return searchedIndex; 
}


VariableCondition_ValueIndex _WallVC_GetValueIndex(
		void*				variableCondition, 
		Index				globalIndex, 
		VariableCondition_VariableIndex	varIndex)
{
	return varIndex;
}


VariableCondition_ValueIndex _WallVC_GetValueCount(void* variableCondition)
{
	WallVC*	self = (WallVC*)variableCondition;
	
	return self->_entryCount;
}


VariableCondition_Value _WallVC_GetValue(void* variableCondition, VariableCondition_ValueIndex valIndex)
{
	WallVC*	self = (WallVC*)variableCondition;

	return self->_entryTbl[valIndex].value;
}

void _WallVC_PrintConcise( void* variableCondition, Stream* stream ) {
	WallVC*		self = (WallVC*)variableCondition;
	
	Journal_Printf( stream, "\ttype: %s, set: ", self->type );
	Journal_Printf( stream, "%s\n", 
		self->_wall == WallVC_Wall_Front ? "Front" :
		self->_wall == WallVC_Wall_Back ? "Back" : 
		self->_wall == WallVC_Wall_Left ? "Left" :
		self->_wall == WallVC_Wall_Right ? "Right" : 
		self->_wall == WallVC_Wall_Top ? "Top" :
		self->_wall == WallVC_Wall_Bottom ? "Bottom" : "None" );
}

/*--------------------------------------------------------------------------------------------------------------------------
** Build functions
*/


/*--------------------------------------------------------------------------------------------------------------------------
** Functions
*/

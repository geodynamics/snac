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
** This file may be distributed under the terms of the VPAC Public License
** as defined by VPAC of Australia and appearing in the file
** LICENSE.VPL included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** $Id: SplitFrictionWallVC.c 3310 2005-10-26 07:10:18Z RobertTurnbull $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* Modified 2006 Walter Landry to implement Split Friction Wall VC's */

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"
#include "Discretisation/Mesh/Mesh.h"

#include "types.h"
#include "SplitFrictionWallVC.h"
#include "RegularMeshUtils.h"

#include <string.h>
#include <assert.h>


const Type SplitFrictionWallVC_Type = "SplitFrictionWallVC";
const Name defaultSplitFrictionWallVCName = "defaultSplitFrictionWallVCName";

const char* SplitFrictionWallVC_WallEnumToStr[SplitFrictionWallVC_Wall_Size] = {
	"back",
	"left",
	"bottom",
	"right",
	"top",
	"front" };


/*--------------------------------------------------------------------------------------------------------------------------
** Constructor
*/

VariableCondition* SplitFrictionWallVC_Factory(
		Variable_Register*				variable_Register, 
		ConditionFunction_Register*			conFunc_Register, 
		Dictionary*					dictionary,
		void*						data )
{
	return (VariableCondition*)SplitFrictionWallVC_New( defaultSplitFrictionWallVCName, NULL, variable_Register, conFunc_Register, dictionary, (Mesh*)data );
}


SplitFrictionWallVC*	SplitFrictionWallVC_DefaultNew( Name name )
{
	return _SplitFrictionWallVC_New(
		sizeof(SplitFrictionWallVC), 
		SplitFrictionWallVC_Type, 
		_SplitFrictionWallVC_Delete, 
		_SplitFrictionWallVC_Print, 
		_SplitFrictionWallVC_Copy,
		(Stg_Component_DefaultConstructorFunction*)SplitFrictionWallVC_DefaultNew,
		_SplitFrictionWallVC_Construct,	
		_SplitFrictionWallVC_Build,
		_VariableCondition_Initialise,
		_VariableCondition_Execute,
		_VariableCondition_Destroy,
		name,
		False,
		_SplitFrictionWallVC_BuildSelf, 
		_SplitFrictionWallVC_PrintConcise,
		_SplitFrictionWallVC_ReadDictionary,
		_SplitFrictionWallVC_GetSet, 
		_SplitFrictionWallVC_GetVariableCount, 
		_SplitFrictionWallVC_GetVariableIndex, 
		_SplitFrictionWallVC_GetValueIndex, 
		_SplitFrictionWallVC_GetValueCount, 
		_SplitFrictionWallVC_GetValue,
		NULL,
		NULL, 
		NULL, 
		NULL, 
		NULL);
}

SplitFrictionWallVC*	SplitFrictionWallVC_New(
		Name						name,
		Name						_dictionaryEntryName, 
		Variable_Register*				variable_Register, 
		ConditionFunction_Register*			conFunc_Register, 
		Dictionary*					dictionary,
		void*						_mesh )
{
	return _SplitFrictionWallVC_New(
		sizeof(SplitFrictionWallVC), 
		SplitFrictionWallVC_Type, 
		_SplitFrictionWallVC_Delete, 
		_SplitFrictionWallVC_Print, 
		_SplitFrictionWallVC_Copy,
		(Stg_Component_DefaultConstructorFunction*)SplitFrictionWallVC_DefaultNew,
		_SplitFrictionWallVC_Construct,	
		_SplitFrictionWallVC_Build,
		_VariableCondition_Initialise,
		_VariableCondition_Execute,
		_VariableCondition_Destroy,
		name,
		True,
		_SplitFrictionWallVC_BuildSelf, 
		_SplitFrictionWallVC_PrintConcise,
		_SplitFrictionWallVC_ReadDictionary,
		_SplitFrictionWallVC_GetSet, 
		_SplitFrictionWallVC_GetVariableCount, 
		_SplitFrictionWallVC_GetVariableIndex, 
		_SplitFrictionWallVC_GetValueIndex, 
		_SplitFrictionWallVC_GetValueCount, 
		_SplitFrictionWallVC_GetValue,
		_dictionaryEntryName,
		variable_Register, 
		conFunc_Register, 
		dictionary, 
		_mesh );
}


void SplitFrictionWallVC_Init(
		SplitFrictionWallVC*						self,
		Name						name,
		Name						_dictionaryEntryName, 
		Variable_Register*				variable_Register, 
		ConditionFunction_Register*			conFunc_Register, 
		Dictionary*					dictionary,
		void*						_mesh )
{
	/* General info */
	self->type = SplitFrictionWallVC_Type;
	self->_sizeOfSelf = sizeof(SplitFrictionWallVC);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _SplitFrictionWallVC_Delete;
	self->_print = _SplitFrictionWallVC_Print;
	self->_copy = _SplitFrictionWallVC_Copy;
	self->_defaultConstructor = (Stg_Component_DefaultConstructorFunction*)SplitFrictionWallVC_DefaultNew;
	self->_construct = _SplitFrictionWallVC_Construct;
	self->_build = _SplitFrictionWallVC_Build;
	self->_initialise = _VariableCondition_Initialise;
	self->_execute = _VariableCondition_Execute;
	self->_destroy = _VariableCondition_Destroy;
	self->_buildSelf = _SplitFrictionWallVC_BuildSelf;
	self->_printConcise = _SplitFrictionWallVC_PrintConcise;
	self->_readDictionary = _SplitFrictionWallVC_ReadDictionary;
	self->_getSet = _SplitFrictionWallVC_GetSet;
	self->_getVariableCount = _SplitFrictionWallVC_GetVariableCount;
	self->_getVariableIndex = _SplitFrictionWallVC_GetVariableIndex;
	self->_getValueIndex = _SplitFrictionWallVC_GetValueIndex;
	self->_getValueCount = _SplitFrictionWallVC_GetValueCount;
	self->_getValue = _SplitFrictionWallVC_GetValue;
	
	_Stg_Class_Init( (Stg_Class*)self );
	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );
	_Stg_Component_Init( (Stg_Component*)self );
	_VariableCondition_Init( (VariableCondition*)self, variable_Register, conFunc_Register, dictionary );
	
	/* Stg_Class info */
	_SplitFrictionWallVC_Init( self, _dictionaryEntryName, _mesh );
}


SplitFrictionWallVC* _SplitFrictionWallVC_New( 
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
	SplitFrictionWallVC*	self;
	
	/* Allocate memory/General info */
	assert(_sizeOfSelf >= sizeof(SplitFrictionWallVC));
	self = (SplitFrictionWallVC*)_VariableCondition_New(
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
		_SplitFrictionWallVC_Init( self, _dictionaryEntryName, _mesh );
	}
	
	return self;
}


void _SplitFrictionWallVC_Init(
		void*						wallVC, 
		Name						_dictionaryEntryName, 
		void*						_mesh )
{
	SplitFrictionWallVC*			self = (SplitFrictionWallVC*)wallVC;

	self->isConstructed = True;
	self->_dictionaryEntryName = _dictionaryEntryName;
	self->_mesh = (Mesh*)_mesh;
	self->_wall = SplitFrictionWallVC_Wall_Size;
	self->_entryTbl = 0;
	self->_entryCount = 0;
}


/*--------------------------------------------------------------------------------------------------------------------------
** General virtual functions
*/

void _SplitFrictionWallVC_ReadDictionary( void* variableCondition, void* dictionary ) {
	SplitFrictionWallVC*			self = (SplitFrictionWallVC*)variableCondition;
	Dictionary_Entry_Value*	vcDictVal;
	Dictionary_Entry_Value	_vcDictVal;
	Dictionary_Entry_Value*	varsVal;
	SplitFrictionWallVC_Entry_Index	entry_I;
	
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
			self->_wall = SplitFrictionWallVC_Wall_Back;
		else if (!strcasecmp(wallStr, "left"))
			self->_wall = SplitFrictionWallVC_Wall_Left;
		else if (!strcasecmp(wallStr, "bottom"))
			self->_wall = SplitFrictionWallVC_Wall_Bottom;
		else if (!strcasecmp(wallStr, "right"))
			self->_wall = SplitFrictionWallVC_Wall_Right;
		else if (!strcasecmp(wallStr, "top"))
			self->_wall = SplitFrictionWallVC_Wall_Top;
		else if (!strcasecmp(wallStr, "front"))
			self->_wall = SplitFrictionWallVC_Wall_Front;
		else {
			assert( 0 );
			self->_wall = SplitFrictionWallVC_Wall_Size; /* invalid entry */
		}
		
		/* Obtain the variable entries */
/* 		self->_entryCount = 0; */
		self->_entryCount = Dictionary_Entry_Value_GetCount(Dictionary_Entry_Value_GetMember(vcDictVal, "variables"));
		self->_entryTbl = Memory_Alloc_Array( SplitFrictionWallVC_Entry, self->_entryCount, "SplitFrictionWallVC->_entryTbl" );
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
						__func__, self->_dictionaryEntryName, SplitFrictionWallVC_WallEnumToStr[self->_wall],
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
					self->_entryTbl[entry_I].value.as.typeArray.size, "SplitFrictionWallVC->_entryTbl[].value.as.typeArray.array" );
					
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
		self->_wall = SplitFrictionWallVC_Wall_Size;
		self->_entryCount = 0;
		self->_entryTbl = NULL;
	}
}


void _SplitFrictionWallVC_Delete(void* wallVC)
{
	SplitFrictionWallVC*	self = (SplitFrictionWallVC*)wallVC;
	
	if (self->_entryTbl) Memory_Free(self->_entryTbl);
	
	/* Stg_Class_Delete parent */
	_VariableCondition_Delete(self);
}


void _SplitFrictionWallVC_Print(void* wallVC, Stream* stream)
{
	SplitFrictionWallVC*				self = (SplitFrictionWallVC*)wallVC;
	SplitFrictionWallVC_Entry_Index		entry_I;
	Index				i;
	
	/* Set the Journal for printing informations */
	Stream* info = stream;
	
	/* General info */
	Journal_Printf( info, "SplitFrictionWallVC (ptr): %p\n", self);
	
	/* Virtual info */
	
	/* Stg_Class info */
	Journal_Printf( info, "\tdictionary (ptr): %p\n", self->dictionary);
	Journal_Printf( info, "\t_dictionaryEntryName (ptr): %p\n", self->_dictionaryEntryName);
	if (self->_dictionaryEntryName)
		Journal_Printf( info, "\t\t_dictionaryEntryName: %s\n", self->_dictionaryEntryName);
	Journal_Printf( info, "\t_wall: %s\n", self->_wall == SplitFrictionWallVC_Wall_Front ? "Front" :
		self->_wall == SplitFrictionWallVC_Wall_Back ? "Back" : self->_wall == SplitFrictionWallVC_Wall_Left ? "Left" :
		self->_wall == SplitFrictionWallVC_Wall_Right ? "Right" : self->_wall == SplitFrictionWallVC_Wall_Top ? "Top" :
		self->_wall == SplitFrictionWallVC_Wall_Bottom ? "Bottom" : "None");
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


void* _SplitFrictionWallVC_Copy( void* wallVC, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap ) {
	SplitFrictionWallVC*		self = (SplitFrictionWallVC*)wallVC;
	SplitFrictionWallVC*		newSplitFrictionWallVC;
	PtrMap*		map = ptrMap;
	Bool		ownMap = False;
	
	if( !map ) {
		map = PtrMap_New( 10 );
		ownMap = True;
	}
	
	newSplitFrictionWallVC = (SplitFrictionWallVC*)_VariableCondition_Copy( self, dest, deep, nameExt, map );
	
	newSplitFrictionWallVC->_dictionaryEntryName = self->_dictionaryEntryName;
	newSplitFrictionWallVC->_wall = self->_wall;
	newSplitFrictionWallVC->_entryCount = self->_entryCount;
	
	if( deep ) {
		newSplitFrictionWallVC->_mesh = (Mesh*)Stg_Class_Copy( self->_mesh, NULL, deep, nameExt, map );
		
		if( (newSplitFrictionWallVC->_entryTbl = PtrMap_Find( map, self->_entryTbl )) == NULL && self->_entryTbl ) {
			newSplitFrictionWallVC->_entryTbl = Memory_Alloc_Array( SplitFrictionWallVC_Entry, newSplitFrictionWallVC->_entryCount, "SplitFrictionWallVC->_entryTbl");
			memcpy( newSplitFrictionWallVC->_entryTbl, self->_entryTbl, sizeof(SplitFrictionWallVC_Entry) * newSplitFrictionWallVC->_entryCount );
			PtrMap_Append( map, newSplitFrictionWallVC->_entryTbl, self->_entryTbl );
		}
	}
	else {
		newSplitFrictionWallVC->_mesh = self->_mesh;
		newSplitFrictionWallVC->_entryTbl = self->_entryTbl;
	}
	
	if( ownMap ) {
		Stg_Class_Delete( map );
	}
	
	return (void*)newSplitFrictionWallVC;
}


void _SplitFrictionWallVC_Build(  void* wallVC, void* data ) {
	SplitFrictionWallVC*			self = (SplitFrictionWallVC*)wallVC;
	
	_SplitFrictionWallVC_BuildSelf( self, data );
	
	_VariableCondition_Build( self, data );
}
	

/*--------------------------------------------------------------------------------------------------------------------------
** Macros
*/


/*--------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _SplitFrictionWallVC_Construct( void* wallVC, Stg_ComponentFactory* cf, void* data )
{
	
}

void _SplitFrictionWallVC_BuildSelf(  void* wallVC, void* data ) {
	SplitFrictionWallVC*			self = (SplitFrictionWallVC*)wallVC;
	
	if( self->_mesh ) {
		Build( self->_mesh, data, False );
	}
}


IndexSet* _SplitFrictionWallVC_GetSet(void* variableCondition)
{
	SplitFrictionWallVC*		self = (SplitFrictionWallVC*)variableCondition;
	IndexSet	*set = NULL;
	HexaMD*		hexaMD = (HexaMD*)self->_mesh->layout->decomp;
	Stream*		warningStr = Journal_Register( Error_Type, self->type );

	
	switch (self->_wall) {
		case SplitFrictionWallVC_Wall_Front:
			if ( 1 == hexaMD->nodeGlobal3DCounts[K_AXIS] ) {
				Journal_Printf( warningStr, "Warning - in %s: Can't build a %s wall VC "
					"when mesh has no elements in the %s axis. Returning an empty set.\n", __func__,
					SplitFrictionWallVC_WallEnumToStr[self->_wall], "K" );
				set = IndexSet_New(hexaMD->nodeDomainCount);	
			}
			else {
				set = RegularMeshUtils_CreateGlobalFrontSet(self->_mesh);
			}
			break;
			
		case SplitFrictionWallVC_Wall_Back:
			if ( 1 == hexaMD->nodeGlobal3DCounts[K_AXIS] ) {
				Journal_Printf( warningStr, "Warning - in %s: Can't build a %s wall VC "
					"when mesh has no elements in the %s axis. Returning an empty set.\n", __func__,
					SplitFrictionWallVC_WallEnumToStr[self->_wall], "K" );
				set = IndexSet_New(hexaMD->nodeDomainCount);	
			}
			else {
				set = RegularMeshUtils_CreateGlobalBackSet(self->_mesh);
			}	
			break;
			
		case SplitFrictionWallVC_Wall_Top:
			if ( 1 == hexaMD->nodeGlobal3DCounts[J_AXIS] ) {
				Journal_Printf( warningStr, "Warning - in %s: Can't build a %s wall VC "
					"when mesh has no elements in the %s axis. Returning an empty set.\n", __func__,
					SplitFrictionWallVC_WallEnumToStr[self->_wall], "J" );
				set = IndexSet_New(hexaMD->nodeDomainCount);	
			}
			else {
				set = RegularMeshUtils_CreateGlobalTopSet(self->_mesh);
			}	
			break;
			
		case SplitFrictionWallVC_Wall_Bottom:
			if ( 1 == hexaMD->nodeGlobal3DCounts[J_AXIS] ) {
				Journal_Printf( warningStr, "Warning - in %s: Can't build a %s wall VC "
					"when mesh has no elements in the %s axis. Returning an empty set.\n", __func__,
					SplitFrictionWallVC_WallEnumToStr[self->_wall], "J" );
				set = IndexSet_New(hexaMD->nodeDomainCount);	
			}
			else {
				set = RegularMeshUtils_CreateGlobalBottomSet(self->_mesh);
			}	
			break;
			
		case SplitFrictionWallVC_Wall_Left:
			if ( 1 == hexaMD->nodeGlobal3DCounts[I_AXIS] ) {
				Journal_Printf( warningStr, "Warning - in %s: Can't build a %s wall VC "
					"when mesh has no elements in the %s axis. Returning an empty set.\n", __func__,
					SplitFrictionWallVC_WallEnumToStr[self->_wall], "I" );
				set = IndexSet_New(hexaMD->nodeDomainCount);	
			}
			else {
				set = RegularMeshUtils_CreateGlobalLeftSet(self->_mesh);
			}	
			break;
			
		case SplitFrictionWallVC_Wall_Right:
			if ( 1 == hexaMD->nodeGlobal3DCounts[I_AXIS] ) {
				Journal_Printf( warningStr, "Warning - in %s: Can't build a %s wall VC "
					"when mesh has no elements in the %s axis. Returning an empty set.\n", __func__,
					SplitFrictionWallVC_WallEnumToStr[self->_wall], "I" );
				set = IndexSet_New(hexaMD->nodeDomainCount);	
			}
			else {
				set = RegularMeshUtils_CreateGlobalRightSet(self->_mesh);
			}
			break;
			
		case SplitFrictionWallVC_Wall_Size:
		default:
			assert(0);
			break;
	}
	
	return set;
}


VariableCondition_VariableIndex _SplitFrictionWallVC_GetVariableCount(void* variableCondition, Index globalIndex)
{
	SplitFrictionWallVC*	self = (SplitFrictionWallVC*)variableCondition;
	
	return self->_entryCount;
}


Variable_Index _SplitFrictionWallVC_GetVariableIndex(void* variableCondition, Index globalIndex, VariableCondition_VariableIndex varIndex)
{
	SplitFrictionWallVC*		self = (SplitFrictionWallVC*)variableCondition;
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


VariableCondition_ValueIndex _SplitFrictionWallVC_GetValueIndex(
		void*				variableCondition, 
		Index				globalIndex, 
		VariableCondition_VariableIndex	varIndex)
{
	return varIndex;
}


VariableCondition_ValueIndex _SplitFrictionWallVC_GetValueCount(void* variableCondition)
{
	SplitFrictionWallVC*	self = (SplitFrictionWallVC*)variableCondition;
	
	return self->_entryCount;
}


VariableCondition_Value _SplitFrictionWallVC_GetValue(void* variableCondition, VariableCondition_ValueIndex valIndex)
{
	SplitFrictionWallVC*	self = (SplitFrictionWallVC*)variableCondition;

	return self->_entryTbl[valIndex].value;
}

void _SplitFrictionWallVC_PrintConcise( void* variableCondition, Stream* stream ) {
	SplitFrictionWallVC*		self = (SplitFrictionWallVC*)variableCondition;
	
	Journal_Printf( stream, "\ttype: %s, set: ", self->type );
	Journal_Printf( stream, "%s\n", 
		self->_wall == SplitFrictionWallVC_Wall_Front ? "Front" :
		self->_wall == SplitFrictionWallVC_Wall_Back ? "Back" : 
		self->_wall == SplitFrictionWallVC_Wall_Left ? "Left" :
		self->_wall == SplitFrictionWallVC_Wall_Right ? "Right" : 
		self->_wall == SplitFrictionWallVC_Wall_Top ? "Top" :
		self->_wall == SplitFrictionWallVC_Wall_Bottom ? "Bottom" : "None" );
}

/*--------------------------------------------------------------------------------------------------------------------------
** Build functions
*/


/*--------------------------------------------------------------------------------------------------------------------------
** Functions
*/

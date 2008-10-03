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
** $Id: ShapeVC.c 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <Base/Base.h>
#include <Discretisation/Geometry/Geometry.h>
#include <Discretisation/Shape/Shape.h>
#include <Discretisation/Mesh/Mesh.h>

#include "types.h"
#include "ShapeVC.h"

#include <assert.h>
#include <string.h>

const Type ShapeVC_Type = "ShapeVC";
const Name defaultShapeVCName = "defaultShapeVCName";

/*-----------------------------------------------------------------------------------------------------------------
** Constructor
*/
VariableCondition* ShapeVC_Factory(
		Variable_Register*                          variable_Register, 
		ConditionFunction_Register*                 conFunc_Register, 
		Dictionary*                                 dictionary,
		void*                                       data )
{
	return (VariableCondition*) 
		ShapeVC_New( defaultShapeVCName, NULL, variable_Register, conFunc_Register, dictionary, (Mesh*)data );
}

ShapeVC* ShapeVC_New(
		Name                                        name,
		Name                                        _dictionaryEntryName, 
		Variable_Register*                          variable_Register, 
		ConditionFunction_Register*                 conFunc_Register, 
		Dictionary*	                                dictionary,
		void*                                       _mesh )
{
	ShapeVC* self = (ShapeVC*) _ShapeVC_DefaultNew( name );

	_VariableCondition_Init( self, variable_Register, conFunc_Register, dictionary );
	_ShapeVC_Init( self, _dictionaryEntryName, _mesh );

	return self;
}

ShapeVC* _ShapeVC_New( 
		SizeT                                       _sizeOfSelf, 
		Type                                        type,
		Stg_Class_DeleteFunction*                   _delete,
		Stg_Class_PrintFunction*                    _print,
		Stg_Class_CopyFunction*                     _copy,
		Stg_Component_DefaultConstructorFunction*   _defaultConstructor,
		Stg_Component_ConstructFunction*            _construct,
		Stg_Component_BuildFunction*                _build,
		Stg_Component_InitialiseFunction*           _initialise,
		Stg_Component_ExecuteFunction*              _execute,
		Stg_Component_DestroyFunction*              _destroy,
		VariableCondition_BuildSelfFunc*            _buildSelf, 
		VariableCondition_PrintConciseFunc*         _printConcise,
		VariableCondition_ReadDictionaryFunc*       _readDictionary,
		VariableCondition_GetSetFunc*               _getSet,
		VariableCondition_GetVariableCountFunc*     _getVariableCount,
		VariableCondition_GetVariableIndexFunc*     _getVariableIndex,
		VariableCondition_GetValueIndexFunc*        _getValueIndex,
		VariableCondition_GetValueCountFunc*        _getValueCount,
		VariableCondition_GetValueFunc*             _getValue,
		Name                                        name  )
{
	ShapeVC*	self;
	
	/* Allocate memory/General info */
	assert(_sizeOfSelf >= sizeof(ShapeVC));
	self = (ShapeVC*)_VariableCondition_New(
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
		False,
		_buildSelf, 
		_printConcise,	
		_readDictionary,
		_getSet, 
		_getVariableCount, 
		_getVariableIndex, 
		_getValueIndex, 
		_getValueCount, 
		_getValue, 
		NULL, 
		NULL,
		NULL );
	
	/* Virtual info */
	
	return self;
}


void _ShapeVC_Init(
		void*                                       variableCondition, 
		Name                                        _dictionaryEntryName, 
		void*                                       _mesh )
{
	ShapeVC*			self = (ShapeVC*) variableCondition;

	self->isConstructed        = True;
	self->_dictionaryEntryName = _dictionaryEntryName;
	self->_mesh                = (Mesh*)_mesh;
	self->_entryTbl            = 0;
	self->_entryCount          = 0;

	assert( _mesh && Stg_Class_IsInstance( _mesh, Mesh_Type ) );
}


/*--------------------------------------------------------------------------------------------------------------------------
** General virtual functions
*/


void _ShapeVC_Delete(void* variableCondition) {
	ShapeVC*	self = (ShapeVC*)variableCondition;
	
	if ( self->_entryTbl ) 
		Memory_Free(self->_entryTbl);

	if ( self->shapeName )
		Memory_Free( self->shapeName );
	
	/* Stg_Class_Delete parent */
	_VariableCondition_Delete(self);
}


void _ShapeVC_Print(void* variableCondition, Stream* stream) {
	ShapeVC*                self = (ShapeVC*)variableCondition;
	ShapeVC_Entry_Index     entry_I;
	Index                   array_I;
	
	/* General info */
	Journal_Printf( stream, "ShapeVC (ptr): %p\n", self);
	
	/* Virtual info */
	
	/* Stg_Class info */
	Journal_Printf( stream, "\tdictionary (ptr): %p\n", self->dictionary);
	Journal_Printf( stream, "\t_dictionaryEntryName (ptr): %p\n", self->_dictionaryEntryName);
	if (self->_dictionaryEntryName)
		Journal_Printf( stream, "\t\t_dictionaryEntryName: %s\n", self->_dictionaryEntryName);
	if ( self->_shape )
		Journal_Printf( stream, "\t_shape: %s '%s'\n", self->_shape->type, self->_shape->name );
	
	Journal_Printf( stream, "\t_entryCount: %u\n", self->_entryCount);
	Journal_Printf( stream, "\t_entryTbl (ptr): %p\n", self->_entryTbl);
	if (self->_entryTbl) {
		for (entry_I = 0; entry_I < self->_entryCount; entry_I++) {
			Journal_Printf( stream, "\t\t_entryTbl[%u]:\n", entry_I);
			Journal_Printf( stream, "\t\t\tvarName (ptr): %p\n", self->_entryTbl[entry_I].varName);
			if (self->_entryTbl[entry_I].varName)
				Journal_Printf( stream, "\t\t\t\tvarName: %s\n", self->_entryTbl[entry_I].varName);
			Journal_Printf( stream, "\t\t\tvalue:\n");
			switch (self->_entryTbl[entry_I].value.type) {
				case VC_ValueType_Double:
					Journal_Printf( stream, "\t\t\t\ttype: VC_ValueType_Double\n" );
					Journal_Printf( stream, "\t\t\t\tasDouble: %g\n", self->_entryTbl[entry_I].value.as.typeDouble );
					break;
					
				case VC_ValueType_Int:
					Journal_Printf( stream, "\t\t\t\ttype: VC_ValueType_Int\n" );
					Journal_Printf( stream, "\t\t\t\tasInt: %i\n", self->_entryTbl[entry_I].value.as.typeInt );
					break;
					
				case VC_ValueType_Short:
					Journal_Printf( stream, "\t\t\t\ttype: VC_ValueType_Short\n" );
					Journal_Printf( stream, "\t\t\t\tasShort: %i\n", self->_entryTbl[entry_I].value.as.typeShort );
					break;
					
				case VC_ValueType_Char:
					Journal_Printf( stream, "\t\t\t\ttype: VC_ValueType_Char\n");
					Journal_Printf( stream, "\t\t\t\tasChar: %c\n", self->_entryTbl[entry_I].value.as.typeChar );
					break;
					
				case VC_ValueType_Ptr:
					Journal_Printf( stream, "\t\t\t\ttype: VC_ValueType_Ptr\n");
					Journal_Printf( stream, "\t\t\t\tasPtr: %g\n", self->_entryTbl[entry_I].value.as.typePtr );
					break;
					
				case VC_ValueType_DoubleArray:
					Journal_Printf( stream, "\t\t\t\ttype: VC_ValueType_DoubleArray\n");
					Journal_Printf( stream, "\t\t\t\tarraySize: %u\n", self->_entryTbl[entry_I].value.as.typeArray.size);
					Journal_Printf( stream, "\t\t\t\tasDoubleArray (ptr): %p\n", 
						self->_entryTbl[entry_I].value.as.typeArray.array);
					if (self->_entryTbl[entry_I].value.as.typeArray.array)
						for ( array_I = 0;  array_I < self->_entryTbl[entry_I].value.as.typeArray.size;  array_I++)
							Journal_Printf( stream, "\t\t\t\tasDoubleArray[%u]: %g\n",  array_I,
								self->_entryTbl[entry_I].value.as.typeArray.array[ array_I]);
					break;
					
				case VC_ValueType_CFIndex:
					Journal_Printf( stream, "\t\t\t\ttype: VC_ValueType_CFIndex\n");
					Journal_Printf( stream, "\t\t\t\tasCFIndex: %u\n", self->_entryTbl[entry_I].value.as.typeCFIndex);
					break;
			}
		}
	}
	Journal_Printf( stream, "\t_mesh (ptr): %p\n", self->_mesh);
	
	/* Print parent */
	_VariableCondition_Print( self );
}


void* _ShapeVC_Copy( void* variableCondition, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap ) {
	ShapeVC*        self           = (ShapeVC*)variableCondition;
	ShapeVC*        newShapeVC;
	PtrMap*         map            = ptrMap;
	Bool            ownMap         = False;
	
	if( !map ) {
		map = PtrMap_New( 10 );
		ownMap = True;
	}
	
	newShapeVC = (ShapeVC*)_VariableCondition_Copy( self, dest, deep, nameExt, map );
	
	newShapeVC->_dictionaryEntryName = self->_dictionaryEntryName;
	newShapeVC->_shape = self->_shape;
	newShapeVC->_entryCount = self->_entryCount;
	
	if( deep ) {
		newShapeVC->_mesh = (Mesh*)Stg_Class_Copy( self->_mesh, NULL, deep, nameExt, map );
		
		if( (newShapeVC->_entryTbl = PtrMap_Find( map, self->_entryTbl )) == NULL && self->_entryTbl ) {
			newShapeVC->_entryTbl = Memory_Alloc_Array( ShapeVC_Entry, newShapeVC->_entryCount, "ShapeVC->_entryTbl");
			memcpy( newShapeVC->_entryTbl, self->_entryTbl, sizeof(ShapeVC_Entry) * newShapeVC->_entryCount );
			PtrMap_Append( map, newShapeVC->_entryTbl, self->_entryTbl );
		}
	}
	else {
		newShapeVC->_mesh = self->_mesh;
		newShapeVC->_entryTbl = self->_entryTbl;
	}
	
	if( ownMap ) {
		Stg_Class_Delete( map );
	}
	
	return (void*)newShapeVC;
}
	
/****************** Stg_Component Virtual Functions ******************/
void* _ShapeVC_DefaultNew( Name name ) {
	return (void*) _ShapeVC_New(
		sizeof(ShapeVC), 
		ShapeVC_Type, 
		_ShapeVC_Delete, 
		_ShapeVC_Print, 
		_ShapeVC_Copy,
		_ShapeVC_DefaultNew,
		_ShapeVC_Construct,	
		_ShapeVC_Build,
		_VariableCondition_Initialise,
		_VariableCondition_Execute,
		_VariableCondition_Destroy,
		_ShapeVC_BuildSelf, 
		_ShapeVC_PrintConcise,
		_ShapeVC_ReadDictionary,
		_ShapeVC_GetSet, 
		_ShapeVC_GetVariableCount, 
		_ShapeVC_GetVariableIndex, 
		_ShapeVC_GetValueIndex, 
		_ShapeVC_GetValueCount, 
		_ShapeVC_GetValue,
		name );
}

void _ShapeVC_Construct( void* variableCondition, Stg_ComponentFactory* cf, void* data ) {
}

void _ShapeVC_Build(  void* variableCondition, void* data ) {
	ShapeVC*			self = (ShapeVC*)variableCondition;

	_ShapeVC_BuildSelf( self, data );
	_VariableCondition_Build( self, data );
}
	
/****************** VariableCondition Virtual Functions ******************/
void _ShapeVC_BuildSelf(  void* variableCondition, void* data /* for build phase */ ) {
	ShapeVC*         self    = (ShapeVC*)variableCondition;
	AbstractContext* context = (AbstractContext*) data;

	assert( context && Stg_Class_IsInstance( context, AbstractContext_Type ) );
	assert( self->shapeName );
	Journal_Firewall( strlen( self->shapeName ) > 0, Journal_MyStream( Error_Type, self ),
			"You need to fill out the 'Shape' dictionary entry for this ShapeVC.\n" );
	assert( self->_mesh );

	self->_shape =  Stg_ComponentFactory_ConstructByName(  context->CF,  self->shapeName, Stg_Shape,  True, 0 /* dummy */  ) ;
	
	Stg_Component_Build( self->_mesh, data, False );
	Stg_Component_Build( self->_shape, data, False );
}

void _ShapeVC_PrintConcise( void* variableCondition, Stream* stream ) {
	ShapeVC* self = (ShapeVC*) variableCondition;
	
	Journal_Printf( stream, "\ttype: %s, Shape: %s '%s'", self->type, self->_shape->type, self->_shape->name );
}

void _ShapeVC_ReadDictionary( void* variableCondition, void* dictionary ) {
	ShapeVC*                  self = (ShapeVC*)variableCondition;
	Dictionary_Entry_Value*   vcDictVal;
	Dictionary_Entry_Value    _vcDictVal;
	Dictionary_Entry_Value*   varsVal;
	ShapeVC_Entry_Index	      entry_I;
	
	/* Find dictionary entry */
	if (self->_dictionaryEntryName)
		vcDictVal = Dictionary_Get(dictionary, self->_dictionaryEntryName);
	else {
		vcDictVal = &_vcDictVal;
		Dictionary_Entry_Value_InitFromStruct(vcDictVal, dictionary);
	}
	
	if (vcDictVal) {
		/* Get Name of Shape from dictionary - Grab pointer to shape later on */
		self->shapeName = StG_Strdup( 
				Dictionary_Entry_Value_AsString(Dictionary_Entry_Value_GetMember(vcDictVal, "Shape" )) );

		/* Obtain the variable entries */
		self->_entryCount = Dictionary_Entry_Value_GetCount(Dictionary_Entry_Value_GetMember(vcDictVal, "variables"));
		self->_entryTbl = Memory_Alloc_Array( ShapeVC_Entry, self->_entryCount, "ShapeVC->_entryTbl" );
		varsVal = Dictionary_Entry_Value_GetMember(vcDictVal, "variables");
		
		for (entry_I = 0; entry_I < self->_entryCount; entry_I++) {
			char*			valType;
			Dictionary_Entry_Value*	valueEntry;
			Dictionary_Entry_Value*	varDictListVal;
			
			varDictListVal = Dictionary_Entry_Value_GetElement(varsVal, entry_I);
			valueEntry = Dictionary_Entry_Value_GetMember(varDictListVal, "value");
			
			self->_entryTbl[entry_I].varName = Dictionary_Entry_Value_AsString(
				Dictionary_Entry_Value_GetMember(varDictListVal, "name"));
				
			valType = Dictionary_Entry_Value_AsString(Dictionary_Entry_Value_GetMember(varDictListVal, "type"));
			if (0 == strcasecmp(valType, "func")) {
				char*	funcName = Dictionary_Entry_Value_AsString(valueEntry);
				Index	cfIndex;
				
				self->_entryTbl[entry_I].value.type = VC_ValueType_CFIndex;
				cfIndex = ConditionFunction_Register_GetIndex( self->conFunc_Register, funcName);
				if ( cfIndex == (Index) -1 ) {	
					Stream*	errorStr = Journal_Register( Error_Type, self->type );

					Journal_Printf( errorStr, "Error- in %s: While parsing "
						"definition of shapeVC \"%s\" (applies to shape \"%s\"), the cond. func. applied to "
						"variable \"%s\" - \"%s\" - wasn't found in the c.f. register.\n",
						__func__, self->_dictionaryEntryName, self->shapeName,
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
					self->_entryTbl[entry_I].value.as.typeArray.size, "ShapeVC->_entryTbl[].value.as.typeArray.array" );
					
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
		self->_entryCount = 0;
		self->_entryTbl = NULL;
	}
}

IndexSet* _ShapeVC_GetSet(void* variableCondition) {
	ShapeVC*		self = (ShapeVC*)variableCondition;

	Stg_Component_Initialise( self->_mesh, NULL, False );

	return Mesh_CreateIndexSetFromShape( self->_mesh, self->_shape );
}

VariableCondition_VariableIndex _ShapeVC_GetVariableCount(void* variableCondition, Index globalIndex) {
	ShapeVC*	self = (ShapeVC*)variableCondition;
	
	return self->_entryCount;
}

Variable_Index _ShapeVC_GetVariableIndex(void* variableCondition, Index globalIndex, VariableCondition_VariableIndex varIndex) {
	ShapeVC*        self          = (ShapeVC*)variableCondition;
	Variable_Index  searchedIndex = 0;
	Stream*         errorStr      = Journal_Register( Error_Type, self->type );
	Name            varName;
	
	varName = self->_entryTbl[varIndex].varName;
	searchedIndex = Variable_Register_GetIndex(self->variable_Register, varName );
	
	Journal_Firewall( 
			( searchedIndex < self->variable_Register->count ),
			errorStr,
			"Error- in %s: searching for index of varIndex %u (\"%s\") at global node number %u failed"
			" - register returned index %u, greater than count %u.\n",
			__func__, varIndex, varName, globalIndex, searchedIndex, self->variable_Register->count );

	return searchedIndex; 
}


VariableCondition_ValueIndex _ShapeVC_GetValueIndex(
		void*                                       variableCondition, 
		Index                                       globalIndex, 
		VariableCondition_VariableIndex             varIndex )
{
	return varIndex;
}


VariableCondition_ValueIndex _ShapeVC_GetValueCount(void* variableCondition) {
	ShapeVC*	self = (ShapeVC*)variableCondition;
	
	return self->_entryCount;
}


VariableCondition_Value _ShapeVC_GetValue(void* variableCondition, VariableCondition_ValueIndex valIndex) {
	ShapeVC*	self = (ShapeVC*)variableCondition;

	return self->_entryTbl[valIndex].value;
}


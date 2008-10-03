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

#include "Base/Foundation/Foundation.h"

#include "types.h"
#include "shortcuts.h"
#include "Dictionary.h"
#include "Dictionary_Entry.h"
#include "Dictionary_Entry_Value.h"
#include "IO_Handler.h"
#include "Journal.h"
#include "PathUtils.h"
#include <libxml/tree.h>
#include "XML_IO_Handler.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

/* textual class name */
const Type XML_IO_Handler_Type = "XML_IO_Handler";

/* keyword strings for nodes and lists */
static const xmlChar* INCLUDE_TAG = (const xmlChar*) "include";
static const xmlChar* NAME_ATTR = (const xmlChar*) "name";
static const xmlChar* SOURCEFILE_ATTR = (const xmlChar*) "sourceFile";
static const xmlChar* MERGETYPE_ATTR = (const xmlChar*) "mergeType";
static const xmlChar* CHILDRENMERGETYPE_ATTR = (const xmlChar*) "childrenMergeType";
static const xmlChar* TYPE_ATTR = (const xmlChar*) "type";
static const xmlChar* LIST_TAG = (const xmlChar*) "list";
static const xmlChar* STRUCT_TAG = (const xmlChar*) "struct";
static const xmlChar* PARAM_TAG = (const xmlChar*) "param";
static const xmlChar* ASCII_DATA_TAG = (const xmlChar*) "asciidata";
static const xmlChar* COLUMN_DEFINITION_TAG = (const xmlChar*) "columnDefinition";
static const xmlChar* APPEND_TAG = (const xmlChar*) "append";
static const xmlChar* MERGE_TAG = (const xmlChar*) "merge";
static const xmlChar* REPLACE_TAG = (const xmlChar*) "replace";
static const xmlChar* SEARCH_PATH_TAG = (const xmlChar*) "searchPath";

/* Namespace and root node to validate against - please update on new releases */
static const xmlChar* ROOT_NODE_NAME = (xmlChar*) "StGermainData";
static const xmlChar* XML_VERSION = (xmlChar*) "1.0";

/** column handling for ascii/binary data */
#define MAX_COLUMNS 100
static const unsigned int MAX_COLUMN_NUM_LENGTH = 3;

/** struct to store column info in */
typedef struct {
	char* name;
	Dictionary_Entry_Value_Type dictValueType;
} ColumnInfo;	

static const char* ASCII_DELIMITERS = " \n\t";

static const unsigned int ASCII_LIST_STRING_BUFFER_SIZE = 1024;


/* Reading Function prototypes */
static xmlNodePtr _XML_IO_Handler_OpenCheckFile( XML_IO_Handler*, const char* );
static xmlNodePtr _XML_IO_Handler_OpenCheckBuffer( XML_IO_Handler*, const char* );
static void _XML_IO_Handler_OpenFile( XML_IO_Handler*, const char* );
static void _XML_IO_Handler_OpenBuffer( XML_IO_Handler*, const char* );
static xmlNodePtr _XML_IO_Handler_Check( XML_IO_Handler* );
static void _XML_IO_Handler_ParseNodes( XML_IO_Handler*, xmlNodePtr, Dictionary_Entry_Value*, 
					Dictionary_MergeType, Dictionary_Entry_Source source );
static void _XML_IO_Handler_ParseList( XML_IO_Handler*, xmlNodePtr, Dictionary_Entry_Value*, 
					Dictionary_MergeType, Dictionary_Entry_Source );
static void _XML_IO_Handler_ParseAsciiData( XML_IO_Handler*, xmlNodePtr, Dictionary_Entry_Value* );
static void _XML_IO_Handler_ParseColumnDefinitions( XML_IO_Handler*, xmlNodePtr, ColumnInfo [MAX_COLUMNS], int* const );
static void _XML_IO_Handler_ParseAsciiValue( char* asciiValue, ColumnInfo* columnInfo, Dictionary_Entry_Value* toDictStruct );
static char* _XML_IO_Handler_GetNextAsciiToken( XML_IO_Handler*, xmlNodePtr );
static void _XML_IO_Handler_ParseStruct( XML_IO_Handler*, xmlNodePtr, Dictionary_Entry_Value*, 
						Dictionary_MergeType, Dictionary_Entry_Source );
static void _XML_IO_Handler_ParseParameter( XML_IO_Handler*, xmlNodePtr, Dictionary_Entry_Value*, 
						Dictionary_MergeType, Dictionary_Entry_Source source );
static Dictionary_Entry_Value_Type _XML_IO_Handler_GetDictValueType( XML_IO_Handler* self, char* type );
static xmlChar* _XML_IO_Handler_StripLeadingTrailingWhiteSpace( XML_IO_Handler* self, const xmlChar* const );
static Bool _XML_IO_Handler_IsOnlyWhiteSpace( char* );
/* Writing Function prototypes */
static void _XML_IO_Handler_WriteDictionary( XML_IO_Handler*, Dictionary*, xmlNodePtr);
static void _XML_IO_Handler_WriteNode( XML_IO_Handler*, char*, Dictionary_Entry_Value*, char*, xmlNodePtr);
static void _XML_IO_Handler_WriteList( XML_IO_Handler*, char*, Dictionary_Entry_Value*, char*, xmlNodePtr);
static void _XML_IO_Handler_WriteListElementsXML( XML_IO_Handler*, Dictionary_Entry_Value*, xmlNodePtr);
static Bool _XML_IO_Handler_CheckListCanBePrintedRaw( Dictionary_Entry_Value* );
static void _XML_IO_Handler_WriteListElementsRawASCII( XML_IO_Handler*, Dictionary_Entry_Value*, xmlNodePtr);
static unsigned int _XML_IO_Handler_GetWrittenElementSize( XML_IO_Handler*, Dictionary_Entry_Value* );
static void _XML_IO_Handler_WriteMemberAscii( XML_IO_Handler*, Dictionary_Entry_Value*, char* );
static void _XML_IO_Handler_WriteListElementsRawBinary( XML_IO_Handler*, Dictionary_Entry_Value*, xmlNodePtr);
static void _XML_IO_Handler_WriteStruct( XML_IO_Handler*, char*, Dictionary_Entry_Value*, char*, xmlNodePtr);
static void _XML_IO_Handler_WriteParameter( XML_IO_Handler*, char*, Dictionary_Entry_Value*, char*, xmlNodePtr);


XML_IO_Handler* XML_IO_Handler_New( void ) {
	return _XML_IO_Handler_New( sizeof(XML_IO_Handler), XML_IO_Handler_Type, _XML_IO_Handler_Delete, _XML_IO_Handler_Print, 
		NULL, _XML_IO_Handler_ReadAllFromFile, _XML_IO_Handler_ReadAllFromFileForceSource, _XML_IO_Handler_ReadAllFromBuffer, _XML_IO_Handler_WriteAllToFile, _XML_IO_Handler_WriteEntryToFile, 
		_XML_IO_Handler_SetListEncoding, _XML_IO_Handler_SetWritingPrecision, _XML_IO_Handler_SetWriteExplicitTypes );
}


void XML_IO_Handler_Init( XML_IO_Handler* self ) {
	/* General info */
	self->type = XML_IO_Handler_Type;
	self->_sizeOfSelf = sizeof(XML_IO_Handler);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _XML_IO_Handler_Delete;
	self->_print = _XML_IO_Handler_Print;
	self->_writeEntryToFile = _XML_IO_Handler_WriteEntryToFile;
	self->_setListEncoding = _XML_IO_Handler_SetListEncoding;
	self->_setWritingPrecision = _XML_IO_Handler_SetWritingPrecision;
	self->_setWriteExplicitTypes = _XML_IO_Handler_SetWriteExplicitTypes;
	_Stg_Class_Init( (Stg_Class*)self );
	_IO_Handler_Init( (IO_Handler*)self );
	
	/* XML_IO_Handler info */
	_XML_IO_Handler_Init( self );
}

XML_IO_Handler* _XML_IO_Handler_New( 
		SizeT						_sizeOfSelf, 
		Type						type,
		Stg_Class_DeleteFunction*				_delete,
		Stg_Class_PrintFunction*				_print, 
		Stg_Class_CopyFunction*				_copy, 
		IO_Handler_ReadAllFromFileFunction*		_readAllFromFile,
		IO_Handler_ReadAllFromFileForceSourceFunction*		_readAllFromFileForceSource,
		IO_Handler_ReadAllFromBufferFunction*		_readAllFromBuffer,
		IO_Handler_WriteAllToFileFunction*		_writeAllToFile,
		XML_IO_Handler_WriteEntryToFileFunction*	_writeEntryToFile, 
		XML_IO_Handler_SetListEncodingFunction*		_setListEncoding,
		XML_IO_Handler_SetWritingPrecisionFunction*	_setWritingPrecision,
		XML_IO_Handler_SetWriteExplicitTypesFunction*	_setWriteExplicitTypes )
{
	XML_IO_Handler* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(XML_IO_Handler) );
	self = (XML_IO_Handler*)_IO_Handler_New( 
		_sizeOfSelf, 
		type,
		_delete,
		_print,
		_copy, 
		_readAllFromFile,
		_readAllFromFileForceSource,
		_readAllFromBuffer,
		_writeAllToFile );
	
	/* General info */
	
	/* Virtual info */
	self->_writeEntryToFile = _writeEntryToFile;
	self->_setListEncoding = _setListEncoding;
	self->_setWritingPrecision = _setWritingPrecision;
	self->_setWriteExplicitTypes = _setWriteExplicitTypes;
	
	/* WallBoundaryConditionLayout info */
	_XML_IO_Handler_Init( self );
	
	return self;
}

static void LookupTypePrint( void* ptr, struct Stream* stream ) {
	switch( *(Dictionary_Entry_Value_Type*)ptr ) {
		case Dictionary_Entry_Value_Type_String:
			Journal_Printf( stream, "Dictionary_Entry_Value_Type_String\n" );
			break;
		case Dictionary_Entry_Value_Type_Double:
			Journal_Printf( stream, "Dictionary_Entry_Value_Type_Double\n" );
			break;
		case Dictionary_Entry_Value_Type_UnsignedInt:
			Journal_Printf( stream, "Dictionary_Entry_Value_Type_UnsignedInt\n" );
			break;
		case Dictionary_Entry_Value_Type_Int:
			Journal_Printf( stream, "Dictionary_Entry_Value_Type_Int\n" );
			break;
		case Dictionary_Entry_Value_Type_Bool:
			Journal_Printf( stream, "Dictionary_Entry_Value_Type_Bool\n" );
			break;
		case Dictionary_Entry_Value_Type_Struct:
			Journal_Printf( stream, "Dictionary_Entry_Value_Type_Struct\n" );
			break;
		case Dictionary_Entry_Value_Type_List:
			Journal_Printf( stream, "Dictionary_Entry_Value_Type_List\n" );
			break;
		default:
			Journal_Printf( stream, "Dictionary_Entry_Value_Type_???\n" );
			break;
	}
}

static void* LookupTypeCopy( void* ptr, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap ) {
	Dictionary_Entry_Value_Type* newLookupType;
	
	Journal_Firewall( deep, Journal_Register( Error_Type, XML_IO_Handler_Type ), "Shallow copy not yet implemented\n" );
	
	newLookupType = Memory_Alloc( Dictionary_Entry_Value_Type, "Lookup-type" );
	*newLookupType = *(Dictionary_Entry_Value_Type*)ptr;
	
	return newLookupType;
}

void _XML_IO_Handler_Init( XML_IO_Handler* self ) {
	/* General and Virtual info should already be set */

	Dictionary_Entry_Value_Type* lookupType;

	/* XML_IO_Handler info */
	self->nameSpacesList = NULL;
	_XML_IO_Handler_AddNameSpace( self, "http://www.vpac.org/StGermain/XML_IO_Handler/", "Jun2003" );
	self->resource = NULL;
	self->currDoc = NULL;
	self->currNameSpace = NULL;
	self->tokeniserCalls = 0;
	self->listEncoding = PerList;
	
	self->typeKeywords = Stg_ObjectList_New();
	lookupType = Memory_Alloc( Dictionary_Entry_Value_Type, "Lookup-type" );
	*lookupType = Dictionary_Entry_Value_Type_String;
	Stg_ObjectList_PointerAppend( self->typeKeywords, lookupType, "string", 0, LookupTypePrint, LookupTypeCopy );
	lookupType = Memory_Alloc( Dictionary_Entry_Value_Type, "Lookup-type" );
	*lookupType = Dictionary_Entry_Value_Type_Double;
	Stg_ObjectList_PointerAppend( self->typeKeywords, lookupType, "double", 0, LookupTypePrint, LookupTypeCopy );
	lookupType = Memory_Alloc( Dictionary_Entry_Value_Type, "Lookup-type" );
	*lookupType = Dictionary_Entry_Value_Type_UnsignedInt;
	Stg_ObjectList_PointerAppend( self->typeKeywords, lookupType, "uint", 0, LookupTypePrint, LookupTypeCopy );
	lookupType = Memory_Alloc( Dictionary_Entry_Value_Type, "Lookup-type" );
	*lookupType = Dictionary_Entry_Value_Type_UnsignedInt;
	Stg_ObjectList_PointerAppend( self->typeKeywords, lookupType, "unsigned int", 0, LookupTypePrint, LookupTypeCopy );
	lookupType = Memory_Alloc( Dictionary_Entry_Value_Type, "Lookup-type" );
	*lookupType = Dictionary_Entry_Value_Type_Int;
	Stg_ObjectList_PointerAppend( self->typeKeywords, lookupType, "int", 0, LookupTypePrint, LookupTypeCopy );
	lookupType = Memory_Alloc( Dictionary_Entry_Value_Type, "Lookup-type" );
	*lookupType = Dictionary_Entry_Value_Type_Bool;
	Stg_ObjectList_PointerAppend( self->typeKeywords, lookupType, "bool", 0, LookupTypePrint, LookupTypeCopy );
	lookupType = Memory_Alloc( Dictionary_Entry_Value_Type, "Lookup-type" );
	*lookupType = Dictionary_Entry_Value_Type_Struct;
	Stg_ObjectList_PointerAppend( self->typeKeywords, lookupType, "struct", 0, LookupTypePrint, LookupTypeCopy );
	lookupType = Memory_Alloc( Dictionary_Entry_Value_Type, "Lookup-type" );
	*lookupType = Dictionary_Entry_Value_Type_List;
	Stg_ObjectList_PointerAppend( self->typeKeywords, lookupType, "list", 0, LookupTypePrint, LookupTypeCopy );
	
	self->TYPE_KEYWORDS[Dictionary_Entry_Value_Type_String] = "string";
	self->TYPE_KEYWORDS[Dictionary_Entry_Value_Type_Double] = "double";
	self->TYPE_KEYWORDS[Dictionary_Entry_Value_Type_UnsignedInt] = "uint";
	self->TYPE_KEYWORDS[Dictionary_Entry_Value_Type_Int] = "int";
	self->TYPE_KEYWORDS[Dictionary_Entry_Value_Type_Bool] = "bool";
	self->TYPE_KEYWORDS[Dictionary_Entry_Value_Type_Struct] = "struct";
	self->TYPE_KEYWORDS[Dictionary_Entry_Value_Type_List] = "list";
	self->WRITING_FIELD_EXTRAS[Dictionary_Entry_Value_Type_String] = 1;
	self->WRITING_FIELD_EXTRAS[Dictionary_Entry_Value_Type_Double] = 7;
	self->WRITING_FIELD_EXTRAS[Dictionary_Entry_Value_Type_UnsignedInt] = 6; 
	self->WRITING_FIELD_EXTRAS[Dictionary_Entry_Value_Type_Int] = 6; 
	self->WRITING_FIELD_EXTRAS[Dictionary_Entry_Value_Type_Bool] = 1;
	self->_setWritingPrecision( self, Dictionary_Entry_Value_Type_String, 30 );
	self->_setWritingPrecision( self, Dictionary_Entry_Value_Type_Double, 5 );
	self->_setWritingPrecision( self, Dictionary_Entry_Value_Type_UnsignedInt, 5 );
	self->_setWritingPrecision( self, Dictionary_Entry_Value_Type_Int, 5 );
	self->_setWritingPrecision( self, Dictionary_Entry_Value_Type_Bool, 5 );
	self->writeExplicitTypes = False;
	
	/* set so that whitespaces in between tags are not parsed, and indentation is generated in the output */
	xmlKeepBlanksDefault( False );
	
	/* search path info */
	self->searchPathsSize = 0;
	self->searchPaths = NULL;
}


/** delete the memory used by this level in the class hierarchy. */
void _XML_IO_Handler_Delete( void* xml_io_handler ) {
	XML_IO_Handler* self = (XML_IO_Handler*) xml_io_handler;
	NameSpaceInfo* currNameSpaceInfo;

	int ii;

	assert( self );
	
	Stg_ObjectList_DeleteAllObjects( self->typeKeywords );
	Stg_Class_Delete( self->typeKeywords );

	/* delete this level's memory */
	currNameSpaceInfo = self->nameSpacesList;
	while ( currNameSpaceInfo ) {
		NameSpaceInfo* tempNameSpaceInfo = currNameSpaceInfo;
		currNameSpaceInfo = currNameSpaceInfo->next;	
		
		Memory_Free( tempNameSpaceInfo->location );
		Memory_Free( tempNameSpaceInfo->version );
		Memory_Free( tempNameSpaceInfo );
	}
	
	for ( ii = 0; ii < self->searchPathsSize; ++ii ) {
		Memory_Free( self->searchPaths[ii] );
	}
	if ( self->searchPaths ) {
		Memory_Free( self->searchPaths );
	}

	/* Stg_Class_Delete Parent Stg_Class */
	_IO_Handler_Delete( self );
}


/** print implementation */
void _XML_IO_Handler_Print( void* xml_io_handler, Stream* stream ) {
	XML_IO_Handler* self = (XML_IO_Handler*)xml_io_handler;
	
	/* General info */
	Journal_Printf( stream, "XML_IO_Handler (ptr): %p\n", self );
	
	/* Print parent class */
	_IO_Handler_Print( self, stream );
	
	/* Virtual info */
	Journal_Printf( stream, "\t_writeEntryToFile (func ptr): %p\n", self->_writeEntryToFile );
	Journal_Printf( stream, "\t_setListEncoding (func ptr): %p\n", self-> _setListEncoding);
	Journal_Printf( stream, "\t_setWritingPrecision (func ptr): %p\n", self->_setWritingPrecision );
	Journal_Printf( stream, "\t_setWriteExplicitTypes (func ptr): %p\n", self->_setWriteExplicitTypes );
	
	/* XML_IO_Handler */
	Journal_Printf( stream, "\tcurDoc (ptr)= %p, ", self->currDoc );
	if( self->currNameSpace ) {
		Journal_Printf( stream, "currNameSpace: %s\n", self->currNameSpace->href );
	}
	else {
		Journal_Printf( stream, "currNameSpace: (null)\n" );
	}	
}


/** sets the type of list encoding you'd like output to be written in, see ::ListEncoding enum. */
void XML_IO_Handler_SetListEncoding( void* xml_io_handler, ListEncoding listEncoding ) {
	XML_IO_Handler* self = (XML_IO_Handler*) xml_io_handler;
	
	assert ( self );
	self->_setListEncoding( self, listEncoding );
}

void _XML_IO_Handler_SetListEncoding( void* xml_io_handler, ListEncoding listEncoding ) {
	XML_IO_Handler* self = (XML_IO_Handler*) xml_io_handler;
	
	self->listEncoding = listEncoding;
}


/** sets the precision to which output numbers will be printed. Field width's will be correspondingly adjusted
 * automatically. */
void XML_IO_Handler_SetWritingPrecision( void* xml_io_handler, int dictionaryEntryType, int value ) {
	XML_IO_Handler* self = (XML_IO_Handler*) xml_io_handler;
	
	assert ( self );
	self->_setWritingPrecision( xml_io_handler, dictionaryEntryType, value );
}

void _XML_IO_Handler_SetWritingPrecision( void* xml_io_handler, int dictionaryEntryType, int value ) {
	XML_IO_Handler* self = (XML_IO_Handler*) xml_io_handler;
	
	assert ( self );
	self->writingPrecision[dictionaryEntryType] = value; 
	self->writingFieldWidth[dictionaryEntryType] = value + self->WRITING_FIELD_EXTRAS[dictionaryEntryType];
}


/** Determines whether types will be written explicitly in the output (default: no). If set to yes, the type of each
 * variable will be written as an attribute. */
void XML_IO_Handler_SetWriteExplicitTypes( void* xml_io_handler, Bool writeExplicitTypes ) {
	XML_IO_Handler* self = (XML_IO_Handler*) xml_io_handler;
	
	assert ( self );
	self->_setWriteExplicitTypes( self, writeExplicitTypes );
}

void _XML_IO_Handler_SetWriteExplicitTypes( void* xml_io_handler, Bool writeExplicitTypes ) {
	XML_IO_Handler* self = (XML_IO_Handler*) xml_io_handler;
	
	self->writeExplicitTypes = writeExplicitTypes;
}


/** adds a namespace to the list of those that will be checked against when reading a file */
void _XML_IO_Handler_AddNameSpace( void* xml_io_handler, char* location, char* version )
{
	XML_IO_Handler* self = (XML_IO_Handler*) xml_io_handler;
	NameSpaceInfo* newNameSpaceInfo = Memory_Alloc( NameSpaceInfo, "XML_IO_Handler->nameSpaceList" );
	
	assert ( self );
	newNameSpaceInfo->location = StG_Strdup(location);
	newNameSpaceInfo->version = StG_Strdup(version);
	newNameSpaceInfo->next = self->nameSpacesList; 
	self->nameSpacesList = newNameSpaceInfo;
}


/** add a path to the search paths */
void _XML_IO_Handler_AddSearchPath( void* xml_io_handler, char* path ) {
	XML_IO_Handler*		self = (XML_IO_Handler*)xml_io_handler;

	if ( !path ) {
		return;
	}

	self->searchPathsSize++;
	
	if( self->searchPathsSize == 1 ) {
		self->searchPaths = Memory_Alloc_Array( char*, self->searchPathsSize, "XML_IO_Handler->searchPathsSize" );
	}
	else {
		self->searchPaths = Memory_Realloc_Array( self->searchPaths, char*, self->searchPathsSize );
	}

	self->searchPaths[ self->searchPathsSize - 1 ] = StG_Strdup( path );
}


#include <mpi.h>

/** Read all parameters from a file implementation. See IO_Handler_ReadAllFromFile(). It will first check if the file
 * exists, and contains valid XML. */
Bool _XML_IO_Handler_ReadAllFromFile( void* xml_io_handler, const char* filename, Dictionary* dictionary ) {
	XML_IO_Handler* self = (XML_IO_Handler*) xml_io_handler;
	xmlNodePtr cur = NULL;
int rank;	
	Journal_DPrintf( Journal_Register( Debug_Type, XML_IO_Handler_Type ), "XML_IO_Handler called to read file %s.\n", filename );
	
	assert( self && filename && dictionary );
MPI_Comm_rank( MPI_COMM_WORLD, &rank );	
	/* set the current dictionary to the one being read */
	self->currDictionary = dictionary;

	/* Order of search*/
	/* 1. Current directory */
	/* 2. XML, searched paths */
	/* 3. Environment Variable */
	/* 4. Default STG_MODULE_PATH */

	/* 1. Current directory */
	{
		// $PWD does not work for all ranks, in all mpi implementations
		// "./" however does so far.
		//    char* pwd = getenv( "PWD" );
		char* pwd = "./";
		if ( pwd != NULL ) {
			_XML_IO_Handler_AddSearchPath( self, pwd );
		}
	}

	/* 2. XML, searched paths */
	if( self->currPath ) /* setup initial search path (currPath) */
		_XML_IO_Handler_AddSearchPath( self, self->currPath );
	else
		_XML_IO_Handler_AddSearchPath( self, "./" );

	/* 3. Environment Variable */
	{
		char* envValue;

		envValue = getenv( "STG_MODULE_PATH" );
		if ( envValue != NULL ) {
			char* envCopy; 
			char* token;

			envCopy = StG_Strdup( envValue );
			token = strtok( envCopy, ":" );
			while ( token != NULL ) {
				_XML_IO_Handler_AddSearchPath( self, token );
				token = strtok( NULL, ":" );
			}

			Memory_Free( envCopy );
		}
	}

	/* 4. Default STG_MODULE_PATH */
	#ifdef STG_MODULE_PATH
		_XML_IO_Handler_AddSearchPath( self, STG_MODULE_PATH );
	#endif

	/* open the file and check syntax */
	if ( !(cur = _XML_IO_Handler_OpenCheckFile( self, filename )) ) {
		return False;
	}	
	
	/* call parse nodes, starting on the first child */
	_XML_IO_Handler_ParseNodes( self, cur, NULL, Dictionary_MergeType_Replace, NULL );
	
	/* free memory */
	xmlFreeDoc( self->currDoc );
	xmlCleanupParser();
	
	return True;
}

/** Read all parameters from a file implementation. See IO_Handler_ReadAllFromFile(). It will first check if the file
 * exists, and contains valid XML. 
 * FORCES the source file to be added to each Dictionary_Entry_Source (even if XML didn't have a source entry)*/
Bool _XML_IO_Handler_ReadAllFromFileForceSource( void* xml_io_handler, const char* filename, Dictionary* dictionary ) {
	XML_IO_Handler* self = (XML_IO_Handler*) xml_io_handler;
	xmlNodePtr cur = NULL;
	
	Journal_DPrintf( Journal_Register( Debug_Type, XML_IO_Handler_Type ), "XML_IO_Handler called to read file %s.\n", filename );
	
	assert( self && filename && dictionary );
	
	/* set the current dictionary to the one being read */
	self->currDictionary = dictionary;
	
	/* setup initial search path (currPath) */
	if( self->currPath )
		_XML_IO_Handler_AddSearchPath( self, self->currPath );
	else
		_XML_IO_Handler_AddSearchPath( self, "./" );
	
	/* open the file and check syntax */
	if ( !(cur = _XML_IO_Handler_OpenCheckFile( self, filename )) ) {
		xmlCleanupParser();
		return False;
	}	
	
	/* call parse nodes, starting on the first child */
	_XML_IO_Handler_ParseNodes( self, cur, NULL, Dictionary_MergeType_Replace, (char*) cur->doc->URL );
	
	/* free memory */
	xmlFreeDoc( self->currDoc );
	xmlCleanupParser();
	
	return True;
}

Bool _XML_IO_Handler_ReadAllFromBuffer( void* xml_io_handler, const char* buffer, Dictionary* dictionary ) {
	XML_IO_Handler* self = (XML_IO_Handler*) xml_io_handler;
	xmlNodePtr cur = NULL;

	assert( self && buffer && dictionary );
	
	/* set the current dictionary to the one being read */
	self->currDictionary = dictionary;
	
	/* open the buffer and check syntax */
	if ( !(cur = _XML_IO_Handler_OpenCheckBuffer( self, buffer )) ) {
		xmlCleanupParser();
		return False;
	}	
	
	/* call parse nodes, starting on the first child */
	_XML_IO_Handler_ParseNodes( self, cur, NULL, Dictionary_MergeType_Replace, NULL );
	
	/* free memory */
	xmlFreeDoc( self->currDoc );
	xmlCleanupParser();
	
	return True;
}

/** Opens a file for reading and checks it contains valid XML code.
 * \return a pointer to the root node if the file is valid, NULL otherwise. */
static xmlNodePtr _XML_IO_Handler_OpenCheckFile( XML_IO_Handler* self, const char* filename )
{
	xmlChar absolute[1024];

	if ( FindFileInPathList(
		(char*)absolute,
		(char*)filename,
		self->searchPaths,
		self->searchPathsSize ) )
	{
		_XML_IO_Handler_OpenFile( self, (char*)absolute );
	}

	Journal_Firewall( self->currDoc != NULL,
		Journal_Register( Error_Type, XML_IO_Handler_Type ),
		"Error: File %s doesn't exist, not readable, or not valid.\n",
		filename );

	return _XML_IO_Handler_Check( self );
	 
}

static xmlNodePtr _XML_IO_Handler_OpenCheckBuffer( XML_IO_Handler* self, const char* buffer ) {
	_XML_IO_Handler_OpenBuffer( self, buffer );
	if ( self->currDoc == NULL ) {
		return NULL;
	}

	return _XML_IO_Handler_Check( self );
}
static void _XML_IO_Handler_OpenFile( XML_IO_Handler* self, const char* filename ) {
	/* open an XML file and build an XML tree from it. */
	/* TODO: validate against simple dtd? */
	self->currDoc = xmlParseFile( filename );
	if ( self->currDoc == NULL ) {
		xmlCleanupParser();
	}
	if ( self->resource ) {
		Memory_Free( self->resource );
	}
	self->resource = StG_Strdup( (char*)filename );
}

static void _XML_IO_Handler_OpenBuffer( XML_IO_Handler* self, const char* buffer ) {
	self->currDoc = xmlParseDoc( BAD_CAST buffer );
	if ( self->resource ) {
		Memory_Free( self->resource );
	}
	self->resource = StG_Strdup( "buffer" );
}
static xmlNodePtr _XML_IO_Handler_Check( XML_IO_Handler* self ) {
	xmlNodePtr cur = NULL;
	
	cur = xmlDocGetRootElement( self->currDoc );
	if (cur == NULL) {
		Journal_Printf(
			Journal_Register( Info_Type, XML_IO_Handler_Type ),
			"Error: empty document. Not parsing.\n" );
		xmlFreeDoc( self->currDoc );
		return NULL;
	}
	
	/* check the namespace */
	(void) _XML_IO_Handler_CheckNameSpace( self, cur ); 
	
	/* check root element */
	if (xmlStrcmp(cur->name, (const xmlChar *) ROOT_NODE_NAME)) {
		Journal_Printf( 
			Journal_Register( Info_Type, XML_IO_Handler_Type ),
			"resource %s of wrong type, root node =<%s>, should be <%s>.\nNot parsing.\n",
			self->resource,
			(const char*) cur->name, 
			ROOT_NODE_NAME );
		xmlFreeDoc( self->currDoc );
		return NULL;
	}
	
	/* get first child */
	cur = cur->xmlChildrenNode;
	while ( cur && xmlIsBlankNode ( cur ) ) {
		cur = cur -> next;
	}
	if ( cur == NULL ) {
		Journal_Printf( 
			Journal_Register( Info_Type, XML_IO_Handler_Type ),
			"Error: no children nodes in resource (path)%s. Not parsing.\n",
			self->resource );
	}
	
	return cur;
}


/** given the root node of a document, checks the name space is among the list of allowed ones. If not, or if 
 * version is wrong, prints an appropriate warning.
 * \return True if namespace found among allowed list, false otherwise. */
Bool _XML_IO_Handler_CheckNameSpace( XML_IO_Handler* self, xmlNodePtr curNode )
{
	Bool retVal = True;
	NameSpaceInfo* currNsInfo = self->nameSpacesList;
	char* correctNameSpace;
	int correctLength = ( strlen(currNsInfo->location) + strlen(currNsInfo->version) ) + 1; 
	
	if ( !(correctNameSpace = Memory_Alloc_Array_Unnamed( char, correctLength )) ) {
		Journal_Printf( 
			Journal_Register( Info_Type, XML_IO_Handler_Type ),
			"Error: couldn't allocate memory for namespace.\n" );
	}
	strcpy( correctNameSpace, currNsInfo->location );
	strcat( correctNameSpace, currNsInfo->version );
	
	if ( (self->currNameSpace = xmlSearchNsByHref( self->currDoc , curNode, (const xmlChar *) correctNameSpace )) )
	{
		/* the correct namespace has been found. */
		retVal = True;
	}
	else {
		xmlNsPtr* nsArray = xmlGetNsList( self->currDoc, curNode );
		int i;
		
		for (i=0; nsArray[i] != NULL; i++ ) {
			char* lastSlash;
			int locationLength;
			
			self->currNameSpace = nsArray[i];
			
			if ( NULL == (lastSlash = strrchr( (const char*) nsArray[i]->href, '/' )) ) {
				Journal_Printf( 
					Journal_Register( Info_Type, XML_IO_Handler_Type ),
					"Warning: resource %s has namespace \"%s\" not of expected URI form.\n",
					self->resource, 
					nsArray[i]->href );
				Memory_Free( correctNameSpace );
				return False;
			} 
			locationLength = (char*) lastSlash - (char*) nsArray[i]->href;
			
			if ( 0 == xmlStrncmp( nsArray[i]->href, (const xmlChar*) currNsInfo->location, locationLength ) ) {
				Journal_Printf(
					Journal_Register( Info_Type, XML_IO_Handler_Type ),
					"Warning: resource %s of the correct type (\"%s\") but wrong version (\"%s\") as "
					"compared to correct one of \"%s\".\n", 
					self->resource,
					currNsInfo->location, 
					lastSlash+1,
					currNsInfo->version );			
				retVal = False;
			}
			else {
				currNsInfo = currNsInfo->next;
				
				/* search the IO_Handler's parent for matches */
				while ( currNsInfo )
				{
					self->currNameSpace = nsArray[i];
					if ( 0 == xmlStrncmp( nsArray[i]->href, (const xmlChar*) currNsInfo->location, locationLength ) ) {
						Journal_Printf(
							Journal_Register( Info_Type, XML_IO_Handler_Type ),
							"Warning: resource %s of the type of a parent namespace (\"%s\") as "
							"compared to correct one of %s. "
							"Some input features may not work.\n", 
							self->resource,
							currNsInfo->location, 
							correctNameSpace );
						retVal = False;
					}
				}	
				
				if ( True == retVal ) {
					Journal_Printf(
						Journal_Register( Info_Type, XML_IO_Handler_Type ),
						"Error: resource %s of the wrong type, unknown namespace wasn't expected "
						"value of %s.\n", 
						self->resource, 
						correctNameSpace );
					retVal = False;	
				}		
			}
		}
	}	
	
	Memory_Free( correctNameSpace );
	return retVal;
}			


/** given a document node and the parent of that node, parses all the information on that node and any of its children
 * into the Dictionary. */
static void _XML_IO_Handler_ParseNodes( XML_IO_Handler* self, xmlNodePtr cur, Dictionary_Entry_Value* parent, Dictionary_MergeType mergeType, Dictionary_Entry_Source source )
{
	/* Process each node at this depth. Allow any order, and warn on unknown nodes */
	while (cur != NULL) {
		/* if parameter */
		if ( (0 == xmlStrcmp( cur->name, (const xmlChar *) PARAM_TAG ) ) && ( cur->ns == self->currNameSpace ) ) {
			_XML_IO_Handler_ParseParameter( self, cur, parent, mergeType, source );
		}
		else if	( (0 == xmlStrcmp( cur->name, (const xmlChar *) LIST_TAG ) ) && ( cur->ns == self->currNameSpace ) ) {
			_XML_IO_Handler_ParseList( self, cur , parent, mergeType, source );
		}
		else if	( (0 == xmlStrcmp( cur->name, (const xmlChar *) STRUCT_TAG ) ) &&
			( cur->ns == self->currNameSpace ) ) {
			_XML_IO_Handler_ParseStruct( self, cur, parent, mergeType, source );
		}
		else if ( (0 == xmlStrcmp( cur->name, (const xmlChar *) SEARCH_PATH_TAG ) ) &&
			( cur->ns == self->currNameSpace ) )
		{
			xmlChar*	tmp;
			
			xmlChar* path = xmlNodeListGetString( self->currDoc, cur->xmlChildrenNode, 1 );
			
			tmp = Memory_Alloc_Array( xmlChar, 1024, "TmpPath" );
			
			/* Handle absolute paths */
			if( strlen( (char*)path ) ) {
				if( !(path[0] == '/' || path[0] == '~' || path[0] == '$' ) ) {
					PathJoin( (char*)tmp, 2, self->currPath, path );
				}
				else {
					strcpy( (char*)tmp, (char*)path );
				}
			}
			else {
				tmp[0] = 0;
			}
			
			tmp = (xmlChar*)ExpandEnvironmentVariables( (char*)tmp );
				
			_XML_IO_Handler_AddSearchPath( self, (char*)tmp );

			Memory_Free( tmp );
		}
		else if	( (0 == xmlStrcmp( cur->name, (const xmlChar *) INCLUDE_TAG ) ) &&
			( cur->ns == self->currNameSpace ) )
		{
			xmlChar* filename = xmlNodeListGetString( self->currDoc, cur->xmlChildrenNode, 1 );

			if ( filename ) {
				xmlChar*	spaceStrippedFileName = _XML_IO_Handler_StripLeadingTrailingWhiteSpace( self, filename );

				Journal_DPrintf( 
					Journal_Register( Debug_Type, XML_IO_Handler_Type ),
					"Trying to parse file %s due to include command.\n", 
					(char*)spaceStrippedFileName );
				
				if ( 0 == xmlStrcmp( spaceStrippedFileName, (const xmlChar*) self->resource ) ) { 
					Journal_Printf(
						Journal_Register( Info_Type, XML_IO_Handler_Type ),
						"Warning- while parsing file %s: Ignoring request to parse same file "
						"(to avoid infinite loop.\n", 
						self->resource );
				}
				else {
					xmlChar absolute[1024];
					Index i;
					XML_IO_Handler* newHandler = XML_IO_Handler_New();
					XML_IO_Handler_SetListEncoding( newHandler, self->listEncoding );
					XML_IO_Handler_SetWriteExplicitTypes( newHandler, self->writeExplicitTypes );
				
					for ( i = 0; i < self->searchPathsSize; ++i ) {
						_XML_IO_Handler_AddSearchPath( newHandler, self->searchPaths[i] );
					}
						
					if ( ! FindFileInPathList(
						(char*)absolute, 
						(char*)spaceStrippedFileName, 
						self->searchPaths, 
						self->searchPathsSize ) ) {
					
						/* Let IO_Handler_ReadAllFromFile() display the errors for opening the file */
						strcpy( (char*)absolute, (char *)spaceStrippedFileName );
					}
					
					/* TODO: this will need to change once we allow other file type includes */
					for (i=0; i < Dictionary_Entry_Value_Num_Types; i++ ) {
						XML_IO_Handler_SetWritingPrecision( newHandler, i, self->writingPrecision[i] );
					}
					
					if( 
						strlen( (char*)absolute ) == 0 
						|| (source != NULL && 
						 	False == IO_Handler_ReadAllFromFileForceSource( newHandler, 
												        (char*)absolute, 
													self->currDictionary ) )
						|| (source == NULL &&
						 	False == IO_Handler_ReadAllFromFile( newHandler,
											     (char*)absolute , 
											     self->currDictionary ) ) )
					{
						Journal_Printf(
							Journal_Register( Info_Type, XML_IO_Handler_Type ),
							"Warning: Failed to parse file %s from include command.\n", 
							spaceStrippedFileName );
					}
					
					Stg_Class_Delete( newHandler );
				}
				xmlFree( filename );
				Memory_Free( spaceStrippedFileName );
			}
		}
		/*TODO should we use this or validation? */
		#if 0
		else if  {
			fJournal_Printf( stream, stdout, "Warning- while parsing file %s: unknown tag \"%s\" found, ignoring.\n",
				self->resource, cur->name );
		}
		#endif
		
		cur = cur->next;
	}
}


/** parse list: given a node containing a list, parses the list into the ::Dictionary. (In future might want flags to
 * say whether each element constrained to be same type?) */
static void _XML_IO_Handler_ParseList( XML_IO_Handler* self, xmlNodePtr cur, Dictionary_Entry_Value* parent, 
					Dictionary_MergeType defaultMergeType, Dictionary_Entry_Source source )
{	
	xmlChar* name = xmlGetProp( cur, NAME_ATTR );
	xmlChar* sourceFile = xmlGetProp( cur, SOURCEFILE_ATTR );
	xmlChar* mergeTypeStr = xmlGetProp( cur, MERGETYPE_ATTR );
	xmlChar* childrenMergeTypeStr = xmlGetProp( cur, CHILDRENMERGETYPE_ATTR );
	xmlChar* spaceStrippedName = NULL;
	xmlChar* spaceStrippedSourceFile = NULL;
	xmlChar* spaceStrippedMergeType = NULL;
	xmlChar* spaceStrippedChildrenMergeType = NULL;
	Dictionary_Entry_Value* newList = NULL;
	Dictionary_MergeType mergeType = defaultMergeType;
	Dictionary_MergeType childrenMergeType = Dictionary_MergeType_Append;
	
	if ( name ) {
		spaceStrippedName = _XML_IO_Handler_StripLeadingTrailingWhiteSpace( self, name );
	}
	if ( sourceFile ) {
		spaceStrippedSourceFile = _XML_IO_Handler_StripLeadingTrailingWhiteSpace( self, sourceFile );
	}
	else if (source) {
		spaceStrippedSourceFile = _XML_IO_Handler_StripLeadingTrailingWhiteSpace( self, (xmlChar*) source );
	}
	if( mergeTypeStr ) {
		spaceStrippedMergeType = _XML_IO_Handler_StripLeadingTrailingWhiteSpace( self, mergeTypeStr );
		if( !xmlStrcmp( spaceStrippedMergeType, APPEND_TAG ) ) {
			mergeType = Dictionary_MergeType_Append;
		}
		else if( !xmlStrcmp( spaceStrippedMergeType, MERGE_TAG ) ) {
			mergeType = Dictionary_MergeType_Merge;
		}
		else if( !xmlStrcmp( spaceStrippedMergeType, REPLACE_TAG ) ) {
			mergeType = Dictionary_MergeType_Replace;
		}
		else {
			Journal_DPrintf(
				Journal_Register( Debug_Type, XML_IO_Handler_Type ),
				"_XML_IO_Handler_ParseList called on tag %s, with name=\"%s\", and "
				"mergeType \"%s\" unknown reverting to \"%s\".\n", 
				(char *)cur->name, 
				spaceStrippedName,
				spaceStrippedMergeType,
				defaultMergeType == Dictionary_MergeType_Append ? APPEND_TAG : 
					defaultMergeType == Dictionary_MergeType_Merge ? MERGE_TAG :
					defaultMergeType == Dictionary_MergeType_Replace ? APPEND_TAG :
					APPEND_TAG );
			mergeType = defaultMergeType;
		}
	}
	if( childrenMergeTypeStr ) {
		spaceStrippedChildrenMergeType = _XML_IO_Handler_StripLeadingTrailingWhiteSpace(
			self,
			childrenMergeTypeStr );
							
		if( !xmlStrcmp( spaceStrippedMergeType, APPEND_TAG ) ) {
			childrenMergeType = Dictionary_MergeType_Append;
		}
		else if( !xmlStrcmp( spaceStrippedMergeType, MERGE_TAG ) ) {
			childrenMergeType = Dictionary_MergeType_Merge;
		}
		else if( !xmlStrcmp( spaceStrippedMergeType, REPLACE_TAG ) ) {
			childrenMergeType = Dictionary_MergeType_Replace;
		}
		else {
			Journal_DPrintf(
				Journal_Register( Debug_Type, XML_IO_Handler_Type ),
				 "_XML_IO_Handler_ParseList called on tag %s, with name=\"%s\", and "
				 "mergeType \"%s\" unknown reverting to \"%s\".\n", 
				(char *)cur->name, 
				spaceStrippedName,
				spaceStrippedMergeType,
				APPEND_TAG );
			childrenMergeType = Dictionary_MergeType_Append;
		}
	}

	Journal_DPrintf(
		Journal_Register( Debug_Type, XML_IO_Handler_Type ),
		"_XML_IO_Handler_ParseList called on tag %s, with name=\"%s\", and mergeType=\"%s\"\n", 
		(char *) cur->name, 
		spaceStrippedName, 
		spaceStrippedMergeType );
	
	/* set/add the list */
	newList = IO_Handler_DictSetAddValueWithSource(
		self,
		(char*)spaceStrippedName, 
		NULL, 
		Dictionary_Entry_Value_Type_List, 
		parent, 
		mergeType,
		(char*)spaceStrippedSourceFile );

	if ( (NULL != newList) && cur->xmlChildrenNode ) {
		/* check to see if values presented as columns of binary and ascii data */
		if	( (0 == xmlStrcmp( cur->xmlChildrenNode->name, (const xmlChar *) ASCII_DATA_TAG ) ) &&
				( cur->ns == self->currNameSpace ) )
		{
			Dictionary_Entry_Value_SetEncoding( newList, RawASCII );
			_XML_IO_Handler_ParseAsciiData( self, cur->xmlChildrenNode, newList ); 
		}	
		else {
			/* TODO: working out whether to make sure all elements conform to type of first */
			_XML_IO_Handler_ParseNodes( self, cur->xmlChildrenNode, newList, childrenMergeType, source );
		}
		
		/* if global encoding flag set, change encoding */
		switch (self->listEncoding) {
			case PerList:
				break; /* will be set correctly already */
			case AllXML:
				Dictionary_Entry_Value_SetEncoding( newList, Default );
				break;
			case AllRawASCII:
				Dictionary_Entry_Value_SetEncoding( newList, RawASCII );
				break;
			case AllRawBinary:
				Dictionary_Entry_Value_SetEncoding( newList, RawBinary );
				break;
		}
	}
	
	xmlFree( name );
	if ( spaceStrippedName ) {
		Memory_Free( spaceStrippedName );
	}
	xmlFree( sourceFile );
	if ( spaceStrippedSourceFile ) {
		Memory_Free( spaceStrippedSourceFile );
	}
}			


/** parses an ascii data list. If column definitions provided, parses the data in as the defined types. Else, parses
 * in entries as strings into a struct with members called "1" to "x", where x is the number of ascii data pieces on the
 * first line.
 * TODO: generalise using a function pointer to tokeniser to handle raw binary data */
static void _XML_IO_Handler_ParseAsciiData( XML_IO_Handler* self, xmlNodePtr cur, Dictionary_Entry_Value* parentList )
{
	ColumnInfo columnInfo[MAX_COLUMNS];
	int numColumns = 0;
	xmlNodePtr currChild;
	char** asciiData = NULL;
	int asciiCount;
	int asciiSize;
	char* currAsciiToken;
	int i = 0; 
	int countI;
	
	currChild = cur->xmlChildrenNode;
	_XML_IO_Handler_ParseColumnDefinitions( self, currChild, columnInfo, &numColumns );
	
	if ( 0 == numColumns ) {
		
		/* if asciidata, decide the columns based on the first row. */
		if ( 0 == xmlStrcmp( cur->name, (const xmlChar *) ASCII_DATA_TAG ) ) { 
			xmlChar* dataString = xmlNodeListGetString( self->currDoc, currChild, 1 );
			char* firstLine = strtok( (char*) dataString, "\n" );	
			char* currValue = strtok( firstLine, ASCII_DELIMITERS );
			
			while( currValue ) {
				columnInfo[numColumns].name = Memory_Alloc_Array( char, MAX_COLUMN_NUM_LENGTH,
					"columnInfo[numColumns].name" );
				sprintf( columnInfo[numColumns].name, "%d", numColumns );
				columnInfo[numColumns].dictValueType = Dictionary_Entry_Value_Type_String;
				
				currValue = strtok( NULL, ASCII_DELIMITERS );
				numColumns++;
			}
		}
		else {
			Journal_Printf( 
				Journal_Register( Info_Type, XML_IO_Handler_Type ),
				"Error - while parsing resource %s: type of binary data must be specified "
				"through dictionary or <%s> tag. Ignoring data.\n", 
				self->resource,
				COLUMN_DEFINITION_TAG );
			return;
		}
	
	}
	
	/* reset the tokeniser if necessary */
	self->tokeniserCalls = 0;
	asciiCount = 0;
	asciiSize = 0;

	/* parse each token into asciiData */ 
	currAsciiToken = _XML_IO_Handler_GetNextAsciiToken( self, currChild );
	while ( currAsciiToken ) {
		if ( asciiSize == 0 ) {
			asciiSize += numColumns;
			asciiData = Memory_Alloc_Array( char*, asciiSize, "asciiData"  );
		}
		else if ( asciiCount == asciiSize ) {
			asciiSize += numColumns;
			asciiData = Memory_Realloc_Array( asciiData, char*, asciiSize ); 
		}
		asciiData[asciiCount] = currAsciiToken;
		asciiCount++;
		currAsciiToken = _XML_IO_Handler_GetNextAsciiToken( self, currChild );
	}
	
	countI = 0;
	
	/* parse all the ascii values into the parent list */
	while ( countI < asciiCount ) {
		Bool rowOk = True;
		Dictionary_Entry_Value* currStruct = Dictionary_Entry_Value_NewStruct(); 

		for (i=0; i < numColumns; i++) {
			currAsciiToken = asciiData[countI];
		
			if ( currAsciiToken ) {
				_XML_IO_Handler_ParseAsciiValue( currAsciiToken, &columnInfo[i], currStruct );
			}
			else {
				Journal_Printf(
					Journal_Register( Info_Type, XML_IO_Handler_Type ),
					"Warning - while parsing resource %s: last row of Ascii data partially full. "
					"Discarding row.\n", 
					self->resource );
				Memory_Free( asciiData );
				Memory_Free( currStruct );
				rowOk = False;
				break;
			}
			countI++;	
		}
		/* add element to list in dict */
		if ( True == rowOk ) {
			Dictionary_Entry_Value_AddElement( parentList, currStruct );
		}
	}
	
	Memory_Free( asciiData );
	
	/* Memory_Free the columnInfo array */
	for (i=0; i < numColumns; i++ ) {
		Memory_Free( columnInfo[i].name );
	}
}


/** recursive function to parse all the column definitions, and ignore whitespace between them */
static void _XML_IO_Handler_ParseColumnDefinitions( XML_IO_Handler* self, xmlNodePtr cur, 
	ColumnInfo columnInfo[MAX_COLUMNS], int* const numColumnsPtr )
{
	char* stringPtr;
	/* read any column definitions */
	while ( (XML_ELEMENT_NODE == cur->type) &&
		(0 == xmlStrcmp( cur->name, (const xmlChar *) COLUMN_DEFINITION_TAG ) ) &&
		( cur->ns == self->currNameSpace ) )
	{
		xmlChar* type = xmlGetProp( cur, TYPE_ATTR );
		
		stringPtr = (char*) xmlGetProp( cur, NAME_ATTR );
		columnInfo[(*numColumnsPtr)].name = StG_Strdup( stringPtr );
		xmlFree( stringPtr );
		columnInfo[(*numColumnsPtr)++].dictValueType = _XML_IO_Handler_GetDictValueType( self, (char*) type );
		cur = cur->next;
	}
	
	/* read any whitespace nodes */
	while ( _XML_IO_Handler_IsOnlyWhiteSpace( (char*) xmlNodeListGetString( self->currDoc, cur, 1 ) ) ) {
		cur = cur->next;
	}
	
	/* if more columns, make a recursive call */
	if ( (XML_ELEMENT_NODE == cur->type) &&
		(0 == xmlStrcmp( cur->name, (const xmlChar *) COLUMN_DEFINITION_TAG ) ) &&
		( cur->ns == self->currNameSpace ) )
	{
		_XML_IO_Handler_ParseColumnDefinitions( self, cur, columnInfo, numColumnsPtr );
	}
}


/** parse a single ascii value, into the given ::Dictionary struct, converting to type defined in the given ::ColumnInfo.*/
static void _XML_IO_Handler_ParseAsciiValue( char* asciiValue, ColumnInfo* columnInfo, Dictionary_Entry_Value* toDictStruct ) {
	Dictionary_Entry_Value* newValue = Dictionary_Entry_Value_FromStringTo( asciiValue, columnInfo->dictValueType );
	
	Dictionary_Entry_Value_AddMember( toDictStruct, columnInfo->name, newValue );
}


/** get the next ascii token from the list. */
static char* _XML_IO_Handler_GetNextAsciiToken( XML_IO_Handler* self, xmlNodePtr cur ) {
	char* retToken;
	
	if (0 == self->tokeniserCalls) {
		xmlChar* dataString = xmlNodeListGetString( self->currDoc, cur, 1 );
		retToken = strtok( (char*) dataString, ASCII_DELIMITERS );
	}
	else {
		retToken = strtok( NULL, ASCII_DELIMITERS );
	}
	
	self->tokeniserCalls++;
	return retToken;
}


/** parses a node containing struct info. */
static void _XML_IO_Handler_ParseStruct( 
		XML_IO_Handler*					self, 
		xmlNodePtr					cur, 
		Dictionary_Entry_Value*				parent, 
		Dictionary_MergeType				defaultMergeType,
		Dictionary_Entry_Source				source )
{
	xmlChar* name = xmlGetProp( cur, NAME_ATTR );
	xmlChar* sourceFile = xmlGetProp( cur, SOURCEFILE_ATTR );
	xmlChar* mergeTypeStr = xmlGetProp( cur, MERGETYPE_ATTR );
	xmlChar* childrenMergeTypeStr = xmlGetProp( cur, CHILDRENMERGETYPE_ATTR );
	xmlChar* spaceStrippedName = NULL;
	xmlChar* spaceStrippedSourceFile = NULL;
	xmlChar* spaceStrippedMergeType = NULL;
	xmlChar* spaceStrippedChildrenMergeType = NULL;
	Dictionary_Entry_Value* newStruct = NULL;
	Dictionary_MergeType mergeType = defaultMergeType;
	Dictionary_MergeType childrenMergeType = Dictionary_MergeType_Append;
	
	if ( name ) {
		spaceStrippedName = _XML_IO_Handler_StripLeadingTrailingWhiteSpace( self, name );
	}
	if ( sourceFile ) {
		spaceStrippedSourceFile = _XML_IO_Handler_StripLeadingTrailingWhiteSpace( self, sourceFile );
	}
	else if (source) {
		spaceStrippedSourceFile = _XML_IO_Handler_StripLeadingTrailingWhiteSpace( self, (xmlChar*) source );
	}
	if( mergeTypeStr ) {
		spaceStrippedMergeType = _XML_IO_Handler_StripLeadingTrailingWhiteSpace( self, mergeTypeStr );
		if( !xmlStrcmp( spaceStrippedMergeType, APPEND_TAG ) ) {
			mergeType = Dictionary_MergeType_Append;
		}
		else if( !xmlStrcmp( spaceStrippedMergeType, MERGE_TAG ) ) {
			mergeType = Dictionary_MergeType_Merge;
		}
		else if( !xmlStrcmp( spaceStrippedMergeType, REPLACE_TAG ) ) {
			mergeType = Dictionary_MergeType_Replace;
		}
		else {
			Journal_DPrintf( 
				Journal_Register( Debug_Type, XML_IO_Handler_Type ),
				"_XML_IO_Handler_ParseList called on tag %s, with name=\"%s\", and mergeType \"%s\" unknown "
				"reverting to \"%s\".\n", 
				(char *)cur->name, 
				spaceStrippedName,
				defaultMergeType == Dictionary_MergeType_Append ? APPEND_TAG : 
					defaultMergeType == Dictionary_MergeType_Merge ? MERGE_TAG :
					defaultMergeType == Dictionary_MergeType_Replace ? APPEND_TAG :
					APPEND_TAG );
			mergeType = defaultMergeType;
		}
	}
	if( childrenMergeTypeStr ) {
		spaceStrippedChildrenMergeType = _XML_IO_Handler_StripLeadingTrailingWhiteSpace( self, childrenMergeTypeStr );
		if( !xmlStrcmp( spaceStrippedMergeType, APPEND_TAG ) ) {
			childrenMergeType = Dictionary_MergeType_Append;
		}
		else if( !xmlStrcmp( spaceStrippedMergeType, MERGE_TAG ) ) {
			childrenMergeType = Dictionary_MergeType_Merge;
		}
		else if( !xmlStrcmp( spaceStrippedMergeType, REPLACE_TAG ) ) {
			childrenMergeType = Dictionary_MergeType_Replace;
		}
		else {
			Journal_DPrintf( 
				Journal_Register( Debug_Type, XML_IO_Handler_Type ),
				"_XML_IO_Handler_ParseList called on tag %s, with name=\"%s\", and mergeType \"%s\" unknown "
				"reverting to \"%s\".\n", 
				(char *)cur->name, 
				spaceStrippedName,
				spaceStrippedMergeType,
				APPEND_TAG );
			childrenMergeType = Dictionary_MergeType_Append;
		}
	}

	Journal_DPrintf( 
		Journal_Register( Debug_Type, XML_IO_Handler_Type ),
		"_XML_IO_Handler_ParseStruct called on tag %s, with name=\"%s\"\n",
		(char *) cur->name, 
		spaceStrippedName );
	
	/* set/add the struct */
	newStruct = IO_Handler_DictSetAddValueWithSource( 
		self, 
		(char*)spaceStrippedName, 
		NULL, 
		Dictionary_Entry_Value_Type_Struct, 
		parent, 
		mergeType,
		(char*)spaceStrippedSourceFile );

	if ( NULL != newStruct ) {
		_XML_IO_Handler_ParseNodes( self, cur->xmlChildrenNode, newStruct, childrenMergeType, source );
	}
	
	xmlFree( name );
	if ( spaceStrippedName ) {
		Memory_Free( spaceStrippedName );
	}
	xmlFree( sourceFile );
	if ( spaceStrippedSourceFile ) {
		Memory_Free( spaceStrippedSourceFile );
	}
}


/** parses a single parameter node. */
static void _XML_IO_Handler_ParseParameter( 
	XML_IO_Handler*						self,
	xmlNodePtr						cur,
	Dictionary_Entry_Value*					parent,
	Dictionary_MergeType					defaultMergeType,
	Dictionary_Entry_Source					source )
{
	xmlChar* value = xmlNodeListGetString( self->currDoc, cur->xmlChildrenNode, 1 );
	xmlChar* name = xmlGetProp( cur, NAME_ATTR );
	xmlChar* sourceFile = xmlGetProp( cur, SOURCEFILE_ATTR );
	xmlChar* type = xmlGetProp( cur, TYPE_ATTR );
	xmlChar* mergeTypeStr = xmlGetProp( cur, MERGETYPE_ATTR );
	Dictionary_Entry_Value_Type dictValueType = _XML_IO_Handler_GetDictValueType( self, (char*) type );
	xmlChar* spaceStrippedName = NULL;
	xmlChar* spaceStrippedSourceFile = NULL;
	xmlChar* spaceStrippedMergeType = NULL;
	xmlChar* spaceStrippedType = NULL;
	xmlChar* strippedVal = NULL;
	Dictionary_MergeType mergeType = defaultMergeType;
	
	if ( name ) {
		spaceStrippedName = _XML_IO_Handler_StripLeadingTrailingWhiteSpace( self, name );
	}

	if ( sourceFile ) {
		spaceStrippedSourceFile = _XML_IO_Handler_StripLeadingTrailingWhiteSpace( self, sourceFile );
	}
	else if (source) {
		spaceStrippedSourceFile = _XML_IO_Handler_StripLeadingTrailingWhiteSpace( self, (xmlChar*) source );
	}

	if( mergeTypeStr ) {
		spaceStrippedMergeType = _XML_IO_Handler_StripLeadingTrailingWhiteSpace( self, mergeTypeStr );
		if( !xmlStrcmp( spaceStrippedMergeType, APPEND_TAG ) ) {
			mergeType = Dictionary_MergeType_Append;
		}
		else if( !xmlStrcmp( spaceStrippedMergeType, MERGE_TAG ) ) {
			mergeType = Dictionary_MergeType_Merge;
		}
		else if( !xmlStrcmp( spaceStrippedMergeType, REPLACE_TAG ) ) {
			mergeType = Dictionary_MergeType_Replace;
		}
		else {
			Journal_Printf( 
				Journal_Register( Info_Type, XML_IO_Handler_Type ),
				"_XML_IO_Handler_ParseList called on tag %s, with name=\"%s\", and mergeType \"%s\" unknown "
				"reverting to \"%s\".\n", 
				(char *)cur->name, 
				spaceStrippedName,
				defaultMergeType == Dictionary_MergeType_Append ? APPEND_TAG : 
					/* WHERE THIS COME FROM?defaultMergeType == Dictionary_MergeType_Prepend ? PREPEND_TAG : */
					defaultMergeType == Dictionary_MergeType_Replace ? APPEND_TAG :
					APPEND_TAG );
			mergeType = defaultMergeType;
		}
	}

	if ( NULL == value ) {
		strippedVal = Memory_Alloc_Array( xmlChar, 1, "strippedVal" );
		sprintf( (char*)strippedVal, "%s", "" );
	}
	else {
		strippedVal = _XML_IO_Handler_StripLeadingTrailingWhiteSpace( self, value );
	}
	
	if( type ) {
		spaceStrippedType = _XML_IO_Handler_StripLeadingTrailingWhiteSpace( self, type );
	}

	Journal_DPrintf( 
		Journal_Register( Debug_Type, XML_IO_Handler_Type ),
		"_XML_IO_Handler_ParseParameter called on tag %s, with name=\"%s\", value=\"%s\"\n", 
		(char *) cur->name, 
		spaceStrippedName, 
		strippedVal );
	
	
	/*
	** Check the type and perform actions
	*/
	
	if( spaceStrippedType ) {
		/* TODO: move "path" to a constant */
		if( strcmp( (char*)spaceStrippedType, "path" ) == 0 ) {
			/* prefix the current path to 'val' */
			unsigned	size;
			xmlChar*	newVal;
			unsigned	pos = 0;
			
			size = strlen( (char*)strippedVal ) + 1;
			if( self->currPath ) {
				size += strlen( self->currPath );
				if(
					self->currPath[0] != '/' && 
					self->currPath[0] != '~' && 
					self->currPath[0] != '.' &&
					strstr( self->currPath, "file:" ) == NULL &&
					strstr( self->currPath, "http:" ) == NULL &&
					strstr( self->currPath, "ftp:" ) == NULL )
				{
					
					size += 2;
				}
			}
			
			newVal = Memory_Alloc_Array( xmlChar, size, "XML_IO_Handler_ParseParameter->newVal" );
			if( self->currPath ) {
				if(
					self->currPath[0] != '/' && 
					self->currPath[0] != '~' && 
					self->currPath[0] != '.' &&
					strstr( self->currPath, "file:" ) == NULL &&
					strstr( self->currPath, "http:" ) == NULL &&
					strstr( self->currPath, "ftp:" ) == NULL )
				{
					newVal[0] = '.';
					newVal[1] = '/';
					pos += 2;
				}
				
				strcpy( (char*)&newVal[pos], (char*)self->currPath );
				pos += strlen( self->currPath );
			}
			
			strcpy( (char*)&newVal[pos], (char*)strippedVal );
			Memory_Free( strippedVal );
			strippedVal = newVal;
		}
	}
	
	IO_Handler_DictSetAddValueWithSource( 
				self, 
				(char*)spaceStrippedName, 
				(char*)strippedVal, 
				dictValueType, 
				parent, 
				mergeType, 
				(char*)spaceStrippedSourceFile );
	
	xmlFree( name );
	if ( spaceStrippedName ) {
		Memory_Free( spaceStrippedName );
	}
	xmlFree( value );
	Memory_Free( strippedVal );
	xmlFree( sourceFile );
	if ( spaceStrippedSourceFile ) {
		Memory_Free( spaceStrippedSourceFile );
	}
	
	if( type ) {
		xmlFree( type );
	}
	if( spaceStrippedType ) {
		Memory_Free( spaceStrippedType );
	}
}


/** utility function to get the dictionary type, given a type definition attribute from the xml file.
 * TODO: use a hash table? */
Dictionary_Entry_Value_Type _XML_IO_Handler_GetDictValueType( XML_IO_Handler* self, char* type )
{
	if (NULL == type)
	{
		return Dictionary_Entry_Value_Type_String;
	}
	else {
		Dictionary_Entry_Value_Type* result;
		
		/* convert to lower case. put in string library one day if we make one or use one */
		char* lowercaseType = StG_Strdup( type );
		char* ptr = lowercaseType;
		for ( ; *ptr != '\0'; ++ptr ) {
			*ptr = tolower( *ptr );
		}
	
		result = Stg_ObjectList_Get( self->typeKeywords, lowercaseType );
		Memory_Free( lowercaseType );
		
		if ( result == NULL ) {
			/* if unknown type, return string */
			return Dictionary_Entry_Value_Type_String;
		}
		return *result;
	}
}


/** utility function to strip leading and trailing whitespace */
static xmlChar* _XML_IO_Handler_StripLeadingTrailingWhiteSpace( XML_IO_Handler* self, const xmlChar* const value ) {
	
	assert( value );
	
	if ( '\0' == (*value) ) {
		return (xmlChar*)StG_Strdup( (char*)value );
	}
	else {
		const char* startCharPtr = (const char*)value;
		const char* endCharPtr = (const char*)value + strlen( (char*)value );
		xmlChar* newString = NULL;
		size_t newLength = 0;
		
		while ( isspace( (char) *startCharPtr ) && (startCharPtr < endCharPtr) ) {
			startCharPtr++;
		}
		
		if ( startCharPtr == endCharPtr ) {
			xmlChar* ret = Memory_Alloc_Array( xmlChar, 1, "ret" );
			*ret = '\0'; /* empty string */
			return ret;
		}
		
		while ( isspace( (char) *(endCharPtr-1) ) && (endCharPtr > startCharPtr) ) {
			endCharPtr--;
		}
		
		newLength = endCharPtr - startCharPtr;
		assert( newLength > 0 );
		
		if ( !(newString = Memory_Alloc_Array_Unnamed( xmlChar, newLength + 1 ) ) ) {
			Journal_Printf( 
				Journal_Register( Info_Type, XML_IO_Handler_Type ), 
				"Error - while parsing file %s: out of memory. exiting.\n", 
				self->resource );
			exit( EXIT_FAILURE );
		}
		
		snprintf( (char*)newString, newLength+1, "%s", startCharPtr );
		
		return newString;
	}
}


/** utility function to check if a string is purely whitespace */
Bool _XML_IO_Handler_IsOnlyWhiteSpace( char* string ) {
	/* a reasonable maximum so the function doesn't run for ever */
	const int MAX_WHITESPACE = 100;
	int charIndex = 0;
	if ( !string ) return False;
	
	for (; (string[charIndex] != '\0') && charIndex < MAX_WHITESPACE; charIndex++ ) {
		if ( !isspace( string[charIndex] ) ) return False;
	}
	
	return True;
}


/** Write all the entries in a ::Dictionary to an XML file. The functions XML_IO_Handler_SetListEncoding(),
 * XML_IO_Handler_SetWriteExplicitTypes(), and XML_IO_Handler_SetWritingPrecision() can be used to customise the output.
 * see also IO_Handler_WriteAllToFile().
 * \return True on successful write, false otherwise.
 */
Bool _XML_IO_Handler_WriteAllToFile( void* xml_io_handler, const char* filename, Dictionary* dictionary ) {
	
	/* create/overwrite new document */
	XML_IO_Handler* self = (XML_IO_Handler*) xml_io_handler;
	xmlNodePtr rootNode;
	int fileSize = -1;
	char* correctNameSpace;
	NameSpaceInfo* currNsInfo = self->nameSpacesList;
	int correctLength = ( strlen(currNsInfo->location) + strlen(currNsInfo->version) ) + 1; 
	Stream* stream = Journal_Register( Info_Type, XML_IO_Handler_Type );
	
	Journal_Printf(
		Journal_Register( Info_Type, XML_IO_Handler_Type ),
		"_XML_IO_Handler_WriteAllToFile called to write to file %s.\n", 
		filename );
#if DEBUG
	assert( self );
	assert( filename );
	assert( dictionary );
#endif
	
	/* if overwrite/new */
	self->currDoc = xmlNewDoc( XML_VERSION );
	assert( self->currDoc );
	/* create root element */
	rootNode = xmlNewNode( NULL, ROOT_NODE_NAME);
	assert( rootNode );
	
	if ( !(correctNameSpace = Memory_Alloc_Array( char, correctLength, "courrectNameSpace" )) ) {
		Journal_Printf( 
			Journal_Register( Info_Type, XML_IO_Handler_Type ),
			"Error: couldn't allocate memory for namespace.\n" );
		return False;
	}
	strcpy( correctNameSpace, currNsInfo->location );
	strcat( correctNameSpace, currNsInfo->version );
	self->currNameSpace = xmlNewNs( rootNode, (xmlChar*) correctNameSpace, NULL );
	assert( self->currNameSpace );
	xmlSetNs( rootNode, self->currNameSpace );
	
	xmlDocSetRootElement( self->currDoc, rootNode );
	/* TODO else parse in filename as currDoc, if update set */
	
	_XML_IO_Handler_WriteDictionary( self, dictionary, rootNode );
	
	/* write result to file */
	if ( 0 < (fileSize = xmlSaveFormatFile( filename, self->currDoc, 1 )) ) {
		Journal_Printf( stream, "Writing dictionary contents to file %s successfully concluded.\n", filename );
	} else
	{
		Journal_Printf( 
			Journal_Register( Info_Type, XML_IO_Handler_Type ),
			 "Warning: failed to write dictionary contents to file %s.\n", 
			 filename );
	}
	
	/* Memory_Free memory */
	xmlFreeDoc( self->currDoc );
	//xmlCleanupParser();
	/* TODO if updating, xmlCleanupParser(); */
	self->currDoc = NULL;
	self->currNameSpace = NULL;
	Memory_Free( correctNameSpace );
	
	return ( fileSize > 0 ) ? True : False;
}


/** write a single dictionary entry to a file. The dictionary entry needs to be supplied as a name and value.
 * \return True on success, False otherwise. */
Bool XML_IO_Handler_WriteEntryToFile( void* xml_io_handler, const char* filename,
	Dictionary_Entry_Key name, Dictionary_Entry_Value* value, Dictionary_Entry_Source source )
{
	XML_IO_Handler* self = (XML_IO_Handler*) xml_io_handler;
	
	assert( self );
	return self->_writeEntryToFile( xml_io_handler, filename, name, value, source );
}



Bool _XML_IO_Handler_WriteEntryToFile( void* xml_io_handler, const char* filename,
	Dictionary_Entry_Key name, Dictionary_Entry_Value* value, Dictionary_Entry_Source source )
{
	Stream* stream = Journal_Register (Info_Type, "myStream");
	/* create/overwrite new document */
	XML_IO_Handler* self = (XML_IO_Handler*) xml_io_handler;
	xmlNodePtr rootNode;
	int fileSize = -1;
	char* correctNameSpace;
	NameSpaceInfo* currNsInfo = self->nameSpacesList;
	int correctLength = ( strlen(currNsInfo->location) + strlen(currNsInfo->version) ) + 1;
	
#if DEBUG
	assert( self );
	assert( filename );
	assert( name );
	assert( value );
	//Do not assert source -> it may well be NULL.
#endif
	Journal_Printf(stream, "_XML_IO_Handler_WriteEntryToFile called to write dictionary entry %s to file %s.\n", name, filename ); 
	/* if overwrite/new */
	self->currDoc = xmlNewDoc( XML_VERSION );
	assert( self->currDoc );
	/* create root element */
	rootNode = xmlNewNode( NULL, ROOT_NODE_NAME);
	assert( rootNode );
	
	if ( !(correctNameSpace = Memory_Alloc_Array( char, correctLength, "correctNameSpace" )) ) {
		Journal_Printf(
			Journal_Register( Info_Type, XML_IO_Handler_Type ),
			"Error: couldn't allocate memory for namespace.\n" );
		return False;
	}
	strcpy( correctNameSpace, currNsInfo->location );
	strcat( correctNameSpace, currNsInfo->version );
	self->currNameSpace = xmlNewNs( rootNode, (xmlChar*) correctNameSpace, NULL );
	assert( self->currNameSpace );
	xmlSetNs( rootNode, self->currNameSpace );
	
	xmlDocSetRootElement( self->currDoc, rootNode );
	/* TODO else parse in filename as currDoc, if update set */
	
	_XML_IO_Handler_WriteNode( self, name, value, source, rootNode );
	
	/* write result to file */
	if ( 0 < (fileSize = xmlSaveFormatFile( filename, self->currDoc, 1 )) ) {
		Journal_Printf( stream, "Writing dictionary entry %s to file %s successfully concluded.\n", name, filename );
	} else
	{
		Journal_Printf( 
			Journal_Register( Info_Type, XML_IO_Handler_Type ),
			"Warning: failed to write dictionary entry %s to file %s.\n", 
			name, 
			filename );
	}
	
	xmlFreeDoc( self->currDoc );
	//xmlCleanupParser();
	/* TODO if updating, xmlCleanupParser(); */
	self->currDoc = NULL;
	self->currNameSpace = NULL;
	Memory_Free( correctNameSpace );
	
	return ( fileSize > 0 ) ? True : False;
}


/** Write the contents of a dictionary to a file. Note this can be used for writing #Dictionary_Entry_Value_AsStruct
 * values. */
static void _XML_IO_Handler_WriteDictionary( XML_IO_Handler* self, Dictionary* dict, xmlNodePtr parent ) {
	Dictionary_Index index = 0;

	Journal_DPrintf( 
		Journal_Register( Debug_Type, XML_IO_Handler_Type ),
		"_XML_IO_Handler_WriteDictionary called.\n" );
		
	for (index=0; index < dict->count; index++) {
		Dictionary_Entry* currEntryPtr = dict->entryPtr[index];
	
		_XML_IO_Handler_WriteNode( self, currEntryPtr->key, currEntryPtr->value, currEntryPtr->source, parent );
	}
}


/** write a single node to file, and its children. */
static void _XML_IO_Handler_WriteNode( XML_IO_Handler* self, char* name, Dictionary_Entry_Value* value, 
					char* source, xmlNodePtr parent)

{
	switch ( value->type ) {
		case Dictionary_Entry_Value_Type_Struct:
			_XML_IO_Handler_WriteStruct( self, name, value, source, parent );
			break;
		case Dictionary_Entry_Value_Type_List:
			_XML_IO_Handler_WriteList( self, name, value, source, parent );
			break;
		default:
			_XML_IO_Handler_WriteParameter( self, name, value, source, parent );
	}
}


/** write a list and its children to a file. Depending on the encoding set by either XML_IO_Handler_SetListEncoding(),
 * or stored on the list dictionary entry value itself, will  write out as XML, ascii or binary.*/
static void _XML_IO_Handler_WriteList( XML_IO_Handler* self, char* name, Dictionary_Entry_Value* list,
					char* source, xmlNodePtr parent)
{
	xmlNodePtr newNode;

	Journal_DPrintf( Journal_Register( Debug_Type, XML_IO_Handler_Type ), "_XML_IO_Handler_WriteList called.\n" );

	/* create and add list child node */
	newNode = xmlNewTextChild( parent, self->currNameSpace, LIST_TAG, NULL );
	if ( NULL != name ) {
		xmlNewProp( newNode, (xmlChar*) NAME_ATTR, (xmlChar*) name );
	}	
	if ( NULL != source ) {
		xmlNewProp( newNode, (xmlChar*) SOURCEFILE_ATTR, (xmlChar*) source );
	}	
	
	/* write list elements*/
	switch (self->listEncoding) {
		case PerList:
			switch (Dictionary_Entry_Value_GetEncoding( list )) {
				case Default:
					_XML_IO_Handler_WriteListElementsXML( self, list, newNode );
					break;
				
				case RawASCII:
					_XML_IO_Handler_WriteListElementsRawASCII( self, list, newNode );
					break;
				
				case RawBinary:
					_XML_IO_Handler_WriteListElementsRawBinary( self, list, newNode );
					break;
				
				default:
					Journal_Printf( 
						Journal_Register( Info_Type, XML_IO_Handler_Type ),
						"Warning - while writing file %s: list %s in dictionary specifies "
						"unknown encoding format. Writing as XML.\n", 
						self->resource, 
						name );
					_XML_IO_Handler_WriteListElementsXML( self, list, newNode );
			}
			break;
		
		case AllXML:
			_XML_IO_Handler_WriteListElementsXML( self, list, newNode );
			break;
		
		case AllRawASCII:
			if ( True == _XML_IO_Handler_CheckListCanBePrintedRaw( list ) ) {
				_XML_IO_Handler_WriteListElementsRawASCII( self, list, newNode );
			}
			else {
				_XML_IO_Handler_WriteListElementsXML( self, list, newNode );
			}
			break;
		
		case AllRawBinary:
			if ( True == _XML_IO_Handler_CheckListCanBePrintedRaw( list ) ) {
				_XML_IO_Handler_WriteListElementsRawBinary( self, list, newNode );
			}	
			else {
				_XML_IO_Handler_WriteListElementsXML( self, list, newNode );
			}
			break;
		
		default:
			assert(0);
	}
}


/** write the list elements to a file as XML. */
static void _XML_IO_Handler_WriteListElementsXML( XML_IO_Handler* self, Dictionary_Entry_Value* list,
	xmlNodePtr listNode )
{
	Dictionary_Entry_Value* currChildValue = Dictionary_Entry_Value_GetFirstElement( list );
	
	while ( NULL != currChildValue ) {
		_XML_IO_Handler_WriteNode( self, NULL, currChildValue, NULL, listNode );
		currChildValue = currChildValue->next; 
	}
}


/** Checks if a lsit can be printed in raw (ascii or binary) format. Raw data lists should only contain structs,
 * where each member is a 'simple' type (i.e. not a struct or list) */
static Bool _XML_IO_Handler_CheckListCanBePrintedRaw( Dictionary_Entry_Value* list ) {
	Dictionary_Entry_Value* firstElement = Dictionary_Entry_Value_GetFirstElement( list );
	
	if ( Dictionary_Entry_Value_Type_Struct != firstElement->type ) {
		return False;
	}
	else {
		Dictionary_Index i;
		for (i=0; i < firstElement->as.typeStruct->count; i++)
		{
			int type = firstElement->as.typeStruct->entryPtr[i]->value->type;
			if ( (Dictionary_Entry_Value_Type_Struct == type) || (Dictionary_Entry_Value_Type_List == type) )
			{
				return False;
			}
		}
		
		return True;
	}
}


/** write the elements of a list from a dictionary value to raw ASCII. */
static void _XML_IO_Handler_WriteListElementsRawASCII( XML_IO_Handler* self, Dictionary_Entry_Value* list, xmlNodePtr listNode ) {
	Dictionary_Entry_Value* currMemberValue;
	Dictionary_Entry_Value* currChildStruct;
	xmlBufferPtr bufferPtr;
	char* charBuffer = Memory_Alloc_Array_Unnamed( char, ASCII_LIST_STRING_BUFFER_SIZE );
	unsigned int writtenElementSize;
	xmlNodePtr rawDataNode;
	Index i;
	
	/* check contents suitable for raw printing, error if not. */
	if ( False == _XML_IO_Handler_CheckListCanBePrintedRaw( list ) )
	{
		Journal_Printf(
			Journal_Register( Info_Type, XML_IO_Handler_Type ),
			"Warning- while writing file %s: _XML_IO_Handler_WriteListElementsRaw called on "
			"list %s not suited for Raw writing. Printing as plain XML instead.\n", 
			self->resource,
			xmlGetProp( listNode, NAME_ATTR ) );
		_XML_IO_Handler_WriteListElementsXML( self, list, listNode );
		return;
	}
	
	/* create and add raw data child node from buffer */
	rawDataNode = xmlNewTextChild( listNode, self->currNameSpace, ASCII_DATA_TAG, NULL );
	
	currChildStruct = Dictionary_Entry_Value_GetFirstElement( list );	
	
	/* allocate the memory buffer */
	writtenElementSize = _XML_IO_Handler_GetWrittenElementSize( self, currChildStruct );
	if ( NULL == (bufferPtr = xmlBufferCreateSize( writtenElementSize ) ) )
	{
		Journal_Printf( 
			Journal_Register( Info_Type, XML_IO_Handler_Type ),
			"Error- while writing file %s: out of memory allocating raw data buffer. "
			"Returning.\n", 
			self->resource );
		return;
	}
	
	xmlNodeAddContentLen( rawDataNode, (xmlChar*) "\n", 1 ); 
	
	/* write the column definition tags */
	for (i=0; i < Dictionary_Entry_Value_GetCount( currChildStruct ); i++) {
		xmlNodePtr currColumnDefNode;
		char* typeString = self->TYPE_KEYWORDS[ (int) currChildStruct->as.typeStruct->entryPtr[i]->value->type ];
		
		xmlNodeAddContentLen( rawDataNode, (xmlChar*) "\n", 1 ); 
		currColumnDefNode = xmlNewTextChild( rawDataNode, self->currNameSpace, COLUMN_DEFINITION_TAG, NULL );
		xmlNewProp( currColumnDefNode, NAME_ATTR, (xmlChar*) currChildStruct->as.typeStruct->entryPtr[i]->key );
		xmlNewProp( currColumnDefNode, TYPE_ATTR, (xmlChar*) typeString );
	}
	
	/* will cause trouble if func generalised for binary */
	xmlBufferWriteChar( bufferPtr, "\n\n" );
	
	/* write element contents to buffer, then node */
	while ( NULL != currChildStruct ) {
		/* print the elements into the buffer */
		for (i=0; i < currChildStruct->as.typeStruct->count; i++) {
			if ( (currMemberValue = currChildStruct->as.typeStruct->entryPtr[i]->value) )
			{
				_XML_IO_Handler_WriteMemberAscii( self, currMemberValue, charBuffer );
				xmlBufferWriteChar( bufferPtr, charBuffer );
			}
		}
		
		/* will cause trouble if func generalised for binary */
		xmlBufferWriteChar( bufferPtr, "\n" );
		
		xmlNodeAddContentLen( rawDataNode, xmlBufferContent( bufferPtr ), xmlBufferLength( bufferPtr ) ); 
		xmlBufferEmpty( bufferPtr );
		currChildStruct = currChildStruct->next; 
	}
	
	/* will cause trouble if func generalised for binary */
	xmlNodeAddContentLen( rawDataNode, (xmlChar*) "\n", 1 ); 
	xmlBufferFree( bufferPtr );
	Memory_Free( charBuffer );
}


/** calculates how big a text buffer needs to be created to hold the data from a dictionary list entry as raw ascii. */
static unsigned int _XML_IO_Handler_GetWrittenElementSize( XML_IO_Handler* self, Dictionary_Entry_Value* listElement ) {
	unsigned int i;	
	int totalChars = 0; 
	
	for ( i=0; i < Dictionary_Entry_Value_GetCount( listElement ); i++ )
	{
		totalChars += self->writingFieldWidth[ (int) listElement->as.typeStruct->entryPtr[i]->value->type ];
	}
	
	totalChars++;	/* for the newline character */	
	
	return totalChars;
}


/** writes a signle member data element. NOTE: would have been nice just to use Dictionary_Entry_Value_Print(), but
 * then wouldn't have been able to use custom formatting. */
static void _XML_IO_Handler_WriteMemberAscii( XML_IO_Handler* self, Dictionary_Entry_Value* member, char* buffer ) {
	unsigned int fw = self->writingFieldWidth[ (int) member->type ];
	unsigned int precision = self->writingPrecision[ (int) member->type ];
	char*        string = NULL;
	Stream*      errorStr = NULL;
	
	switch (member->type) {
		case Dictionary_Entry_Value_Type_String:
			string = Dictionary_Entry_Value_AsString( member );
			errorStr = Journal_Register( Error_Type, self->type );
			Journal_Firewall( ( strlen(string) + 1 < ASCII_LIST_STRING_BUFFER_SIZE ), errorStr,
				"Error- in %s: asked to write out a string of length %d, but this "
				"is greater than the max string buffer length of %d. Exiting.\n",
				__func__, strlen(string), ASCII_LIST_STRING_BUFFER_SIZE );
			sprintf(buffer, "%*s ", fw, Dictionary_Entry_Value_AsString( member ) );
			break;
		
		case Dictionary_Entry_Value_Type_Double:
			sprintf(buffer, "%*.*g", fw, precision, Dictionary_Entry_Value_AsDouble( member ) );	
			break;
		
		case Dictionary_Entry_Value_Type_UnsignedInt:
			sprintf(buffer, "%*.*g", fw, precision, Dictionary_Entry_Value_AsDouble( member ) );	
			break;
		
		case Dictionary_Entry_Value_Type_Int:
			sprintf(buffer, "%*.*g", fw, precision, Dictionary_Entry_Value_AsDouble( member ) );	
			break;
		
		case Dictionary_Entry_Value_Type_Bool:
			sprintf(buffer, "%*.*s", fw, precision, Dictionary_Entry_Value_AsString( member ) );	
			break;
		
		default:
			Journal_Printf( 
				Journal_Register( Info_Type, XML_IO_Handler_Type ),
				"Warning- while writing to file %s: unknown type of Dictionary_Entry_Value. "
				"Outputting as a string.\n", 
				self->resource );
			errorStr = Journal_Register( Error_Type, self->type );
			Journal_Firewall( ( strlen(string) + 1 < ASCII_LIST_STRING_BUFFER_SIZE ), errorStr,
				"Error- in %s: asked to write out a string of length %d, but this "
				"is greater than the max string buffer length of %d. Exiting.\n",
				__func__, strlen(string), ASCII_LIST_STRING_BUFFER_SIZE );
			sprintf(buffer, "%*.*s ", self->writingFieldWidth[Dictionary_Entry_Value_Type_String],
				self->writingPrecision[Dictionary_Entry_Value_Type_String],
				Dictionary_Entry_Value_AsString( member ) );
			break;
	}
}


/** TODO: write the lsit elements as raw binary. */
static void _XML_IO_Handler_WriteListElementsRawBinary( XML_IO_Handler* self, Dictionary_Entry_Value* list, xmlNodePtr listNode ) {
	Journal_Printf( 
		Journal_Register( Info_Type, XML_IO_Handler_Type ),
		"_XML_IO_Handler_WriteListElementsRawBinary stub called." );
}


/** writes a struct dictionary entry to the file */
static void _XML_IO_Handler_WriteStruct( XML_IO_Handler* self, char* name, Dictionary_Entry_Value* value, 
						char* source, xmlNodePtr parent)
{	
	xmlNodePtr newNode;

	Journal_DPrintf( 
		Journal_Register( Debug_Type, XML_IO_Handler_Type ),
		"_XML_IO_Handler_WriteStruct called.\n" );

	/* create and add struct child node*/
	newNode = xmlNewTextChild( parent, self->currNameSpace, STRUCT_TAG, NULL );
	if ( NULL != name ) {
		xmlNewProp( newNode, (xmlChar*) NAME_ATTR, (xmlChar*) name );
	}
	if ( NULL != source ) {
		xmlNewProp( newNode, (xmlChar*) SOURCEFILE_ATTR, (xmlChar*) source );
	}
	
	_XML_IO_Handler_WriteDictionary( self, value->as.typeStruct, newNode ); 
}


/** writes a single dictionary parameter to file. */
static void _XML_IO_Handler_WriteParameter( XML_IO_Handler* self, char* name, Dictionary_Entry_Value* value,
						char* source, xmlNodePtr parent)
{
	xmlNodePtr newNode;

	Journal_DPrintfL( 
		Journal_Register( Info_Type, XML_IO_Handler_Type ), 2,
		"_XML_IO_Handler_WriteParameter called.\n");

	/* add new child to parent, with correct value*/
	newNode = xmlNewTextChild( parent, self->currNameSpace, PARAM_TAG, (xmlChar*) Dictionary_Entry_Value_AsString( value ) );
	if ( NULL != name ) {
		xmlNewProp( newNode, (xmlChar*) NAME_ATTR, (xmlChar*) name );
	}
	if ( NULL != source ) {
		xmlNewProp( newNode, (xmlChar*) SOURCEFILE_ATTR, (xmlChar*) source );
	}
	
	if ( True == self->writeExplicitTypes )
	{
		xmlNewProp( newNode, TYPE_ATTR, (xmlChar*) self->TYPE_KEYWORDS[(int) value->type] );
	}
}

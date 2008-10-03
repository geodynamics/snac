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
*/
/** \file
**  Role:
**
** Assumptions:
**
** Invariants:
**
** Comments:
**
** $Id: Map.h 2225 1970-01-02 13:48:23Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Base_Container_Map_h__
#define __Base_Container_Map_h__
	

	/* Textual name of this class */
	extern const Type Map_Type;

	/* Virtual function types */
	typedef void* (Map_UnionFunc)( void* map, void* operand );
	typedef void* (Map_IntersectionFunc)( void* map, void* operand );
	typedef void* (Map_SubtractionFunc)( void* map, void* operand );
	
	/* Support structures */
	struct MapTuple {
		void*	keyData;
		void*	valueData;
	};

	
	/** Map class contents */
	#define __Map \
		/* General info */ \
		__Stg_Class \
		Dictionary*		dictionary; \
		\
		/* Virtual info */ \
		Map_UnionFunc*		_unionFunc; \
		Map_IntersectionFunc*	_intersectionFunc; \
		Map_SubtractionFunc*	_subtractionFunc; \
		\
		/* Map info ... */ \
		SizeT		    	_keySize; \
		SizeT			_valSize; \
		BTree*			_btree;

	struct Map { __Map };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/

	/* Create an instance */
	#define Map_New( dictionary, keyType, valueType, compareFunc, dataCopyFunc, dataDeleteFunc ) \
		Map_New_all( dictionary, sizeof(keyType), sizeof(valueType), compareFunc, dataCopyFunc, dataDeleteFunc )
	
	/* Create an instance with all parameters directly specified */
	Map* Map_New_all( 
		Dictionary*					dictionary, 
		SizeT						keySize, 
		SizeT						valueSize, 
		BTree_compareFunction*				compareFunc, 
		BTree_dataCopyFunction*				dataCopyFunc, 
		BTree_dataDeleteFunction*			dataDeleteFunc );
	
	/* Creation implementation */
	Map* _Map_New(
		SizeT						_sizeOfSelf, 
		Type						type,
		Stg_Class_DeleteFunction*				_delete,
		Stg_Class_PrintFunction*				_print, 
		Stg_Class_CopyFunction*				_copy, 
		Map_UnionFunc*					_unionFunc, 
		Map_IntersectionFunc*				_intersectionFunc, 
		Map_SubtractionFunc*				_subtractionFunc, 
		Dictionary*					dictionary, 
		SizeT						keySize, 
		SizeT						valueSize, 
		BTree_compareFunction*				compareFunc, 
		BTree_dataCopyFunction*				dataCopyFunc, 
		BTree_dataDeleteFunction*			dataDeleteFunc );
	
	
	/* Initialise an instance */
	void Map_Init(
		Map*						self,
		Dictionary*					dictionary, 
		SizeT						keySize,
		SizeT						valueSize, 
		BTree_compareFunction*				compareFunc, 
		BTree_dataCopyFunction*				dataCopyFunc, 
		BTree_dataDeleteFunction*			dataDeleteFunc );
	
	/* Initialisation implementation functions */
	void _Map_Init(
		Map*						self, 
		SizeT						keySize,
		SizeT						valueSize, 
		BTree_compareFunction*				compareFunc, 
		BTree_dataCopyFunction*				dataCopyFunc, 
		BTree_dataDeleteFunction*			dataDeleteFunc );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/* Stg_Class_Delete implementation */
	void _Map_Delete(
		void*						map );
	
	/* Print implementation */
	void _Map_Print(
		void*						map, 
		Stream*						stream );

	void* _Map_Union( void* map, void* operand );

	void* _Map_Intersection( void* map, void* operand );

	void* _Map_Subtraction( void* map, void* operand );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	Bool Map_Insert( Map* self, void* keyData, void* valueData );

	#define Map_Union( self, operand ) \
		(self)->_unionFunc( self, operand )

	#define Map_Intersection( self, operand ) \
		(self)->_intersectionFunc( self, operand )

	#define Map_Subtraction( self, operand ) \
		(self)->_subtractionFunc( self, operand )

	void Map_Traverse( Map* self, BTree_parseFunction* func, void* args );

	void* Map_Find( Map* self, void* keyData );

	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

	void _Map_BTreeUnion( void* tuple, void* map );

	void _Map_BTreeIntersection( void* tuple, void* pack );

	void _Map_BTreeSubtraction( void* tuple, void* pack );

	void _Map_BTreeEval( void* keyData, void* pack );
	

#endif /* __Base_Container_Map_h__ */

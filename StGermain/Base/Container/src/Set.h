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
** $Id: Set.h 2225 1970-01-02 13:48:23Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Base_Container_Set_h__
#define __Base_Container_Set_h__
	

	/* Textual name of this class */
	extern const Type Set_Type;

	/* Virtual function types */
	typedef void* (Set_UnionFunc)( void* set, void* operand );
	typedef void* (Set_IntersectionFunc)( void* set, void* operand );
	typedef void* (Set_SubtractionFunc)( void* set, void* operand );
	
	/* Support structures */
	
	/** Set class contents */
	#define __Set \
		/* General info */ \
		__Stg_Class \
		Dictionary*		dictionary; \
		\
		/* Virtual info */ \
		Set_UnionFunc*		_unionFunc; \
		Set_IntersectionFunc*	_intersectionFunc; \
		Set_SubtractionFunc*	_subtractionFunc; \
		\
		/* Set info ... */ \
		SizeT			_elSize; \
		BTree*			_btree;

	struct Set { __Set };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/

	/* Create an instance */
	#define Set_New( dictionary, elementType, compareFunc, dataCopyFunc, dataDeleteFunc ) \
		Set_New_all( dictionary, sizeof(elementType), compareFunc, dataCopyFunc, dataDeleteFunc )

	/* Create an instance with all parameters */
	Set* Set_New_all( 
		Dictionary*					dictionary, 
		SizeT						elementSize, 
		BTree_compareFunction*				compareFunc, 
		BTree_dataCopyFunction*				dataCopyFunc, 
		BTree_dataDeleteFunction*			dataDeleteFunc );
	
	/* Creation implementation */
	Set* _Set_New(
		SizeT						_sizeOfSelf, 
		Type						type,
		Stg_Class_DeleteFunction*				_delete,
		Stg_Class_PrintFunction*				_print, 
		Stg_Class_CopyFunction*				_copy, 
		Set_UnionFunc*					_unionFunc, 
		Set_IntersectionFunc*				_intersectionFunc, 
		Set_SubtractionFunc*				_subtractionFunc, 
		Dictionary*					dictionary, 
		SizeT						elementSize, 
		BTree_compareFunction*				compareFunc, 
		BTree_dataCopyFunction*				dataCopyFunc, 
		BTree_dataDeleteFunction*			dataDeleteFunc );
	
	
	/* Initialise an instance */
	void Set_Init(
		Set*						self,
		Dictionary*					dictionary, 
		SizeT						elementSize, 
		BTree_compareFunction*				compareFunc, 
		BTree_dataCopyFunction*				dataCopyFunc, 
		BTree_dataDeleteFunction*			dataDeleteFunc );
	
	/* Initialisation implementation functions */
	void _Set_Init(
		Set*						self, 
		SizeT						elementSize, 
		BTree_compareFunction*				compareFunc, 
		BTree_dataCopyFunction*				dataCopyFunc, 
		BTree_dataDeleteFunction*			dataDeleteFunc );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/* Stg_Class_Delete implementation */
	void _Set_Delete(
		void*						set );
	
	/* Print implementation */
	void _Set_Print(
		void*						set, 
		Stream*						stream );

	void* _Set_Union( void* set, void* operand );

	void* _Set_Intersection( void* set, void* operand );

	void* _Set_Subtraction( void* set, void* operand );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	Bool Set_Insert( Set* self, void* data );

	#define Set_Union( self, operand ) \
		(self)->_unionFunc( self, operand )

	#define Set_Intersection( self, operand ) \
		(self)->_intersectionFunc( self, operand )

	#define Set_Subtraction( self, operand ) \
		(self)->_subtractionFunc( self, operand )

	void Set_Traverse( Set* self, BTree_parseFunction* func, void* args );

	Bool Set_Exists( Set* self, void* data );

	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

	void _Set_BTreeUnion( void* data, void* set );

	void _Set_BTreeIntersection( void* data, void* pack );

	void _Set_BTreeSubtraction( void* data, void* pack );


#endif /* __Base_Container_Set_h__ */

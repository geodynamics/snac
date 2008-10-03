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
** $Id: PtrSet.h 2225 1970-01-02 13:48:23Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Base_Container_PtrSet_h__
#define __Base_Container_PtrSet_h__
	

	/* Textual name of this class */
	extern const Type PtrSet_Type;

	/* Virtual function types */
	
	/* Support structures */
	
	/** PtrSet class contents */
	#define __PtrSet \
		/* General info */ \
		__Set \
		\
		/* Virtual info */ \
		\
		/* PtrSet info ... */ \

	struct PtrSet { __PtrSet };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/

	/* Create an instance with all parameters */
	PtrSet* PtrSet_New( 
		Dictionary*					dictionary );
	
	/* Creation implementation */
	PtrSet* _PtrSet_New(
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
	void PtrSet_Init(
		PtrSet*						self,
		Dictionary*					dictionary );
	
	/* Initialisation implementation functions */
	void _PtrSet_Init(
		PtrSet*						self );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/* Stg_Class_Delete implementation */
	void _PtrSet_Delete(
		void*						ptrSet );
	
	/* Print implementation */
	void _PtrSet_Print(
		void*						ptrSet, 
		Stream*						stream );

	void* _PtrSet_Union( void* ptrSet, void* operand );

	void* _PtrSet_Intersection( void* ptrSet, void* operand );

	void* _PtrSet_Subtraction( void* ptrSet, void* operand );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	#define PtrSet_Insert( self, data ) \
		Set_Insert( (Set*)self, data )

	#define PtrSet_Union \
		Set_Union

	#define PtrSet_Intersection \
		Set_Intersection

	#define PtrSet_Subtraction \
		Set_Subtraction

	#define PtrSet_Traverse( self, func, args ) \
		Set_Traverse( (Set*)self, func, args )

	#define PtrSet_Exists( self, data ) \
		Set_Exists( (Set*)self, data )

	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

	int _PtrSet_CompareData( void* left, void* right );

	void _PtrSet_DeleteData( void* data );


#endif /* __Base_Container_PtrSet_h__ */

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
** $Id: IndexMap.h 2225 1970-01-02 13:48:23Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Base_Container_IndexMap_h__
#define __Base_Container_IndexMap_h__
	
	
	/** Textual name of this class */
	extern const Type IndexMap_Type;

	/* Virtual function types */
	
	/** IndexMap class contents */
	typedef struct IndexMapTuple {
		Index			key;
		Index			idx;
	} IndexMapTuple;
	
	#define __IndexMap \
		/* General info */ \
		__Stg_Class \
		\
		Dictionary*				dictionary; \
		\
		/* Virtual info */ \
		\
		/* IndexMap info ... */ \
		unsigned				delta; \
		unsigned				maxTuples; \
		unsigned				tupleCnt; \
		IndexMapTuple*				tupleTbl;

	struct IndexMap { __IndexMap };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	
	/* Create a default IndexMap */
	#define INDEXMAP_DEFAULT_DELTA		10
	
	#define IndexMap_New() \
		IndexMap_New_Param( INDEXMAP_DEFAULT_DELTA )
	
	/* Create a IndexMap */
	IndexMap* IndexMap_New_Param( 
		unsigned					delta );
	
	/* Creation implementation */
	IndexMap* _IndexMap_New(
		SizeT						_sizeOfSelf, 
		Type						type,
		Stg_Class_DeleteFunction*				_delete,
		Stg_Class_PrintFunction*				_print, 
		Stg_Class_CopyFunction*				_copy, 
		unsigned					delta );
	
	
	/* Initialise a IndexMap */
	void IndexMap_Init(
		IndexMap*					self,
		unsigned					delta );
	
	/* Initialisation implementation functions */
	void _IndexMap_Init(
		IndexMap*					self,
		unsigned					delta );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/* Stg_Class_Delete mesh implementation */
	void _IndexMap_Delete( void* indexMap );
	
	/* Print mesh implementation */
	void _IndexMap_Print( void* indexMap, Stream* stream );
	
	/* Copy implementation */
	#define IndexMap_Copy( self ) \
		(IndexMap*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define IndexMap_DeepCopy( self ) \
		(IndexMap*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _IndexMap_Copy( void* indexMap, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/
	
	#define IndexMap_Size( self ) \
		(self)->tupleCnt
	
	void IndexMap_Append( void* indexMap, Index key, Index idx );
	
	Index IndexMap_Find( void* indexMap, Index key );
	
	void IndexMap_Remap( void* indexMap, void* mapThrough );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/
	

#endif /* __Base_Container_IndexMap_h__ */

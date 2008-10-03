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
** $Id: PtrMap.h 2225 1970-01-02 13:48:23Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Base_Container_PtrMap_h__
#define __Base_Container_PtrMap_h__
	

	/** Textual name of this class */
	extern const Type PtrMap_Type;

	/* Virtual function types */
	
	/** PtrMap class contents */
	typedef struct PtrMapTuple {
		void*			key;
		void*			ptr;
	} PtrMapTuple;
	
	#define __PtrMap \
		/* General info */ \
		__Stg_Class \
		\
		Dictionary*				dictionary; \
		\
		/* Virtual info */ \
		\
		/* PtrMap info ... */ \
		unsigned				delta; \
		unsigned				maxTuples; \
		unsigned				tupleCnt; \
		PtrMapTuple*				tupleTbl;

	struct PtrMap { __PtrMap };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	
	/* Create a PtrMap */
	PtrMap* PtrMap_New( 
		unsigned					delta );
	
	/* Creation implementation */
	PtrMap* _PtrMap_New(
		SizeT						_sizeOfSelf, 
		Type						type,
		Stg_Class_DeleteFunction*				_delete,
		Stg_Class_PrintFunction*				_print, 
		Stg_Class_CopyFunction*				_copy, 
		unsigned					delta );
	
	
	/* Initialise a PtrMap */
	void PtrMap_Init(
		PtrMap*						self,
		unsigned					delta );
	
	/* Initialisation implementation functions */
	void _PtrMap_Init(
		PtrMap*						self,
		unsigned					delta );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/* Stg_Class_Delete mesh implementation */
	void _PtrMap_Delete( void* ptrMap );
	
	/* Print mesh implementation */
	void _PtrMap_Print( void* ptrMap, Stream* stream );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/
	
	void PtrMap_Append( void* ptrMap, void* key, void* ptr );
	
	void* PtrMap_Find( void* ptrMap, void* key );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/
	

#endif /* __Base_Container_PtrMap_h__ */

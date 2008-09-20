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
** $Id: UniqueList.h 2225 1970-01-02 13:48:23Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Base_Container_UniqueList_h__
#define __Base_Container_UniqueList_h__
	

	/* Textual name of this class */
	extern const Type UniqueList_Type;

	/* Virtual function types */
	
	/* Support structures */
	
	/** UniqueList class contents */
	#define __UniqueList \
		/* General info */ \
		__List \
		\
		/* Virtual info */ \
		\
		/* UniqueList info ... */

	struct UniqueList { __UniqueList };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	
	/* Create an instance */
	#define UniqueList_New( elementSize ) \
		UniqueList_New_Param( elementSize, 10 )
	
	UniqueList* UniqueList_New_Param( 
		SizeT						elementSize,
		unsigned					delta );
	
	/* Creation implementation */
	UniqueList* _UniqueList_New(
		SizeT						_sizeOfSelf, 
		Type						type,
		Stg_Class_DeleteFunction*				_delete,
		Stg_Class_PrintFunction*				_print, 
		Stg_Class_CopyFunction*				_copy, 
		List_AppendFunc*				_append,
		List_MutateFunc*				_mutate, 
		SizeT						elementSize,
		unsigned					delta );
	
	
	/* Initialise an instance */
	void UniqueList_Init(
		UniqueList*					self,
		SizeT						elementSize,
		unsigned					delta );
	
	/* Initialisation implementation functions */
	void _UniqueList_Init(
		UniqueList*					self );
	
	/* Copy an instance */
	#define UniqueList_Copy( self ) \
		(UniqueList*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	
	#define UniqueList_DeepCopy( self ) \
		(UniqueList*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/* Stg_Class_Delete implementation */
	void _UniqueList_Delete(
		void*						uniqueList );
	
	/* Print implementation */
	void _UniqueList_Print(
		void*						uniqueList, 
		Stream*						stream );
	
	/* Copy implementation */
	void* _UniqueList_Copy(
		void*						uniqueList, 
		void*						dest, 
		Bool						deep, 
		Name						nameExt,
		PtrMap*						ptrMap );
	
	/* Append implementation */
	unsigned _UniqueList_Append( void* list, void* data );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/
	
	#define UniqueList_Size \
		List_Size
	
	#define UniqueList_IsValidIndex \
		List_IsValidIndex
	
	#define UniqueList_ElementAt \
		List_ElementAt
	
	#define UniqueList_Resize \
		List_Resize
	
	#define UniqueList_Append \
		List_Append
	
	#define UniqueList_Mutate \
		List_Mutate
	
	#define UniqueList_Array \
		List_Array
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/
	

#endif /* __Base_Container_UniqueList_h__ */

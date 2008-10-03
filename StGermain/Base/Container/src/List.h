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
** $Id: List.h 2225 1970-01-02 13:48:23Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Base_Container_List_h__
#define __Base_Container_List_h__
	

	/** Textual name of this class */
	extern const Type List_Type;

	/* Virtual function types */
	typedef	unsigned		(List_AppendFunc)	( void* list, void* data );
	typedef void			(List_MutateFunc)	( void* list, unsigned index, void* data );
	
	/** List class contents */
	#define __List \
		/* General info */ \
		__Stg_Class \
		\
		/* Virtual info */ \
		List_AppendFunc*			_append; \
		List_MutateFunc*			_mutate; \
		\
		/* List info ... */ \
		SizeT					elementSize; \
		unsigned				delta; \
		unsigned				maxElements; \
		unsigned				elementCnt; \
		void*					elements;

	struct List { __List };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	
	/* Create a List */
	#define List_New( elementSize ) \
		List_New_Param( elementSize, 10 )
	
	List* List_New_Param( 
		SizeT						elementSize, 
		unsigned					delta );
	
	/* Creation implementation */
	List* _List_New(
		SizeT						_sizeOfSelf, 
		Type						type,
		Stg_Class_DeleteFunction*				_delete,
		Stg_Class_PrintFunction*				_print, 
		Stg_Class_CopyFunction*				_copy, 
		List_AppendFunc*				_append,
		List_MutateFunc*				_mutate, 
		SizeT						elementSize, 
		unsigned					delta );
	
	
	/* Initialise a List */
	void List_Init(
		List*						self,
		SizeT						elementSize, 
		unsigned					delta );
	
	/* Initialisation implementation functions */
	void _List_Init(
		List*						self,
		SizeT						elementSize, 
		unsigned					delta );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/* Stg_Class_Delete mesh implementation */
	void _List_Delete( void* list );
	
	/* Print mesh implementation */
	void _List_Print( void* list, Stream* stream );
	
	/* Append implementation */
	unsigned _List_Append( void* list, void* data );
	
	/* Mutate implementation */
	void _List_Mutate( void* list, unsigned index, void* data );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/
	
	/* Use this to get the current size of the list */
	#define List_Size( self ) ((self)->elementCnt)
	
	/* Use this to check if a given index is within the list's bounds */
	#define List_IsValidIndex( self, index ) (index < (self)->elementCnt)
	
	/* This guy gives access to the list's elements directly.  This is kind of dangerous
	   as inherited lists, such as UniqueList, rely on elements only being added with one
	   of 'Append', 'Prepend' or 'Insert*'. */
	#define List_ElementAt( self, type, index ) (((type*)((List*)self)->elements)[index])
	
	/* Used to resize the list's maximum size */
	void List_Resize( void* list, unsigned size );
	
	/* The standard means of adding an element to the list. */
	#define List_Append( self, data ) (self)->_append( self, data )
	
	/* Modify the content of list element */
	#define List_Mutate( self, index, data ) \
		(self)->_mutate( self, index, data )
	
	/* Get the list's array */
	#define List_Array( self, type ) \
		(type*)(self)->elements
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/
	

#endif /* __Base_Container_List_h__ */

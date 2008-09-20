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
**	An Array implements a typical static/dynamic c-styled array whilst also maintaining information about the number of elements
**	in the array and the size of each element. It creates the actual array on construction and has copying implemented.
**
** Assumptions:
**	The array content is of non-pointer form... i.e. if the array content also needs a custom copying behaviour then one needs
**	to replace the Copy implementation with the smarts.
**
** Invariants:
**
** Comments:
**	Copy in the deep and shallow forms work. In the shallow case, the data points to the source's data. It is safe to delete,
**	as a check is done to see if it owns the data first. The deep case will copy the source's data.
**	
**	TODO: If MACRO_AS_FUNC and DEBUG is enabled, then the functions also check that the if we don't own the data we'll check to
**	ensure it hasn't been de-allocated.
**
** $Id: Array.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Base_Container_Array_h__
#define __Base_Container_Array_h__
	

	/** Textual name of this class */
	extern const Type Array_Type;

	/** Array class contents */
	#define __Array \
		/* General info */ \
		__Stg_Class \
		\
		/* Array info ... */ \
		SizeT					elementSize; \
		Index					count; \
		Array*					owner; \
		void*					ptr;

	/** Defines key information about an object/struct extension - see Array.h. */
	struct Array { __Array };
	
	
	/* Class Administration members ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	
	/* Create a new Array */
	Array* Array_New( SizeT elementSize, Index count );
	
	/* Initialise a Array */
	void Array_Init( Array* self, SizeT elementSize, Index count );
	
	/* Creation implementation */
	Array* _Array_New(
		SizeT						_sizeOfSelf, 
		Type						type,
		Stg_Class_DeleteFunction*				_delete,
		Stg_Class_PrintFunction*				_print, 
		Stg_Class_CopyFunction*				_copy, 
		SizeT						elementSize, 
		Index						count );
	
	/* Initialisation implementation functions */
	void _Array_Init( Array* self, SizeT elementSize, Index count );
	
	/* Class_Delete() implementation */
	void _Array_Delete( void* array );
	
	/* Stg_Class_Print() implementation */
	void _Array_Print( void* array, Stream* stream );
	
	/** Stg_Class_Copy() implementation: derivatives should call this in their implementation */
	void* _Array_Copy( void* array, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	
	/* Public member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	
	/* Obtain the element size of the array */
	#define Array_ElementSizeMacro( self ) \
		( (self)->elementSize )
	SizeT Array_ElementSizeFunc( void* array );
	#ifdef MACRO_AS_FUNC
		/** Obtain the element size of the array */
		#define Array_ElementSize Array_ElementSizeFunc
	#else
		/** Obtain the element size of the array */
		#define Array_ElementSize Array_ElementSizeMacro
	#endif
	
	/* Obtain the number of elements in the array */
	#define Array_CountMacro( self ) \
		( (self)->count )
	Index Array_CountFunc( void* array );
	#ifdef MACRO_AS_FUNC
		/** Obtain the number of elements in the array */
		#define Array_Count Array_CountFunc
	#else
		/** Obtain the number of elements in the array */
		#define Array_Count Array_CountMacro
	#endif
	
	/* Do I own the array (memory allocation)? */
	#define Array_IOwnMacro( self ) \
		( (self)->owner == (self) )
	Bool Array_IOwnFunc( void* array );
	#ifdef MACRO_AS_FUNC
		/** Do I own the array (memory allocation)? */
		#define Array_IOwn Array_IOwnFunc
	#else
		/** Do I own the array (memory allocation)? */
		#define Array_IOwn Array_IOwnMacro
	#endif
	
	/* Who owns the array (memory allocation)? */
	#define Array_OwnerMacro( self ) \
		( (self)->owner )
	void* Array_OwnerFunc( void* array );
	#ifdef MACRO_AS_FUNC
		/** Who owns the array (memory allocation)? */
		#define Array_Owner Array_OwnerFunc
	#else
		/** Who owns the array (memory allocation)? */
		#define Array_Owner Array_OwnerMacro
	#endif
	
	/* Obtain the array pointer */
	#define Array_PtrMacro( self ) \
		( (self)->ptr )
	void* Array_PtrFunc( void* array );
	#ifdef MACRO_AS_FUNC
		/** Obtain the array pointer */
		#define Array_Ptr Array_PtrFunc
	#else
		/** Obtain the array pointer */
		#define Array_Ptr Array_PtrMacro
	#endif
	
	/* Obtain an element of the array */
	#define Array_AtMacro( self, index ) \
		( index < Array_Count( (self) ) ? \
			(void*)((ArithPointer)Array_Ptr( (self) ) +  (self)->elementSize * (index) ) : \
			Array_AtFunc( (self), (index) ) )
	void* Array_AtFunc( void* array, Index index );
	#ifdef MACRO_AS_FUNC
		/** Obtain an element of the array */
		#define Array_At Array_AtFunc
	#else
		/** Obtain an element of the array */
		#define Array_At Array_AtMacro
	#endif
	
	/* Private member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#endif /* __Base_Container_Array_h__ */

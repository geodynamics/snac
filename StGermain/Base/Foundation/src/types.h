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
/** \file
** <b>Role:</b>
**	Basic framework types.
**
** <b>Assumptions:</b>
**	None as yet.
**
** <b>Comments:</b>
**	None as yet.
**
** $Id: types.h 3758 2006-08-08 09:17:16Z LukeHodkinson $
**
**/

#ifndef __Base_Foundation_types_h__
#define __Base_Foundation_types_h__
	
	typedef void*				Pointer;		/**< General Pointer. */
	typedef unsigned long			Stg_Word;		/**< A word on the cpu architecture */
	typedef unsigned long			ArithPointer;		/**< Used to do arithmetic on a pointer. Usually a long. */
	typedef unsigned long			SizeT;			/**< Size of memory allocations. */
	typedef unsigned int			Index;			/**< Generic array index. Positive, not constrained. */

	/* Generic types */
	/* Base types/classes */	
	typedef struct _Stg_Class		Stg_Class;
		
	typedef struct _Stg_Object		Stg_Object;
	typedef Stg_Object*			Stg_ObjectPtr;
	typedef Index				Stg_Object_Index;
	
	typedef struct _Stg_ObjectAdaptor	Stg_ObjectAdaptor;
	typedef struct _Stg_PrimitiveObject	Stg_PrimitiveObject;
		
	typedef struct NamedObject_Register	NamedObject_Register;
	typedef struct _Stg_ObjectList		Stg_ObjectList;

	typedef struct TestSuite		TestSuite;
	
	/* Memory module classes */
	typedef struct MemoryTag		MemoryTag;
	typedef struct MemoryField		MemoryField;
	typedef struct MemoryPointer		MemoryPointer;
	typedef struct MemoryReport		MemoryReport;
	typedef struct Memory			Memory;
	typedef unsigned long			MemoryOpStamp;
	

	typedef char*				Name;			/**< A string used for names. */
	#define					Name_Invalid	0	/**< An invalid or empty name. */
	
	/** Unique ID of a type/class. (runtime-proc persistance). Pointer used as ID. Must point to a valid string. */
	typedef char*				Type;
	#define					Type_Invalid	0	/**< An invalid or empty type. */
	
	
	/*
	 * #ifdef barrier for Bool type. Added 07-Sep-2004 -- Alan
	 *
	 * Change for Cecile so that she may procede with gLucifier work.
	 * This was causing problems when compiling StGermain with gLucifier/vtk using g++.
	 * It seems to suggest that this line was trying to redefine the int primitive.
	 *
	 * Grep searches reveal that neither gLucifier or vtk use the "Bool" word.
	 *
	 * TODO: Find a more elegant way to fix this issue.
	 */
	#ifndef Bool	
		/** Boolean type in StGermain. */
		typedef enum			Bool { False=0, True=1 } Bool;
	#endif
	

	typedef enum MemoryPointerStatus_t{
		MEMORY_POINTER_RELEASED,				/**< If memory held by pointer is released. */
		MEMORY_POINTER_PERSISTENT				/**< If memory held by pointer still exists. */
	}
	MemoryPointerStatus;

	/** Used to indicate whether an allocation is for a global variable or deleted locally.
	 ** Currently used in NamedObject.
	 **/
	typedef enum AllocationType {
		GLOBAL,
		NON_GLOBAL
	} AllocationType;


	/** Enum to tell the replace function whether to delete the replaced object or not. */
	typedef enum ReplacementOption {
		KEEP,
		DELETE
	} ReplacementOption;
	
#endif /* __Base_Foundation_types_h__ */

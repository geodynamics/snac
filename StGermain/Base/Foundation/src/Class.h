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
**	Abstract class faciliting how class inheritance is done.
**
** <b>Assumptions:</b>
**	None
**
** <b>Comments:</b>
**	None
**
** $Id: Class.h 3654 2006-06-27 04:34:03Z LukeHodkinson $
**
**/

#ifndef __Base_Foundation_Class_h__
#define __Base_Foundation_Class_h__

	/* Textual name of this class */
	extern const Type Stg_Class_Type;

	/* Forward declaring Stream */
	struct Stream;
	struct PtrMap;
	
	/* Child classes must define these abstract functions */
	typedef void (Stg_Class_DeleteFunction)	( void* _class );
	typedef void (Stg_Class_PrintFunction)	( void* _class, struct Stream* stream );
	typedef void* (Stg_Class_CopyFunction)	( void* _class, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap );

	/** \def __Stg_Class information */
	#define __Stg_Class \
		/* General info */ 					\
		SizeT				_sizeOfSelf;		/**< Size of the final class type */ \
		Bool				_deleteSelf;		/**< True it is to be deallocated in Delete */ \
		Type				type;			/**< Global const char* to string of class' name */ \
									\
		/* Virtual info */ 					\
		Stg_Class_DeleteFunction*	_delete;		/**< Virtual function for deleting a class. */ \
		Stg_Class_PrintFunction*	_print;			/**< Virtual function for printing a class. */ \
		Stg_Class_CopyFunction*		_copy;			/**< Virtual funciton for copying a class. */
	
	struct _Stg_Class { __Stg_Class };
	
	
	/* Class Administration members ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	#define STG_CLASS_DEFARGS					\
		SizeT				sizeOfSelf,		\
		Type				type,			\
		Stg_Class_DeleteFunction*	deleteFunc,		\
		Stg_Class_PrintFunction*	printFunc,		\
		Stg_Class_CopyFunction*		copyFunc

	#define STG_CLASS_PASSARGS					\
		sizeOfSelf, type, deleteFunc, printFunc, copyFunc
    
	
	/** Constructor interface. */
	Stg_Class* _Stg_Class_New( SizeT _sizeOfSelf, 
			Type type, 
			Stg_Class_DeleteFunction*	_delete, 
			Stg_Class_PrintFunction*	_print,
			Stg_Class_CopyFunction*		_copy );
	
	/** Init interface. */
	void _Stg_Class_Init( Stg_Class* self );
	
	
	/** Deletes any instance of Stg_Class or a sub-class. */
	void Stg_Class_Delete( void* _class );
	
	/** Prints any instance of Stg_Class or a sub-class. */
	void Stg_Class_Print( void* _class, struct Stream* stream );

	/** Prints any instance of Stg_Class or a sub-class. */
	#define Print Stg_Class_Print
	
	/** Generic copy function caller */
	void* Stg_Generic_Copy( 
		Stg_Class_CopyFunction* copyFunc,
		void* _class, 
		void* dest, 
		Bool deep, 
		Name nameExt, 
		struct PtrMap* ptrMap );
	
	/** Copy a class. */
	void* Stg_Class_Copy( void* _class, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap );

	/** Delete interface. */
	void _Stg_Class_Delete( void* _class );
	
	/** Print interface. */
	void _Stg_Class_Print( void* _class, struct Stream* stream );
	
	/** Coy interface. */
	void* _Stg_Class_Copy( void* _class, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap );
	
	
	/* Public member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	
	/* Get the class type */
	#define Stg_Class_GetTypeMacro( _class ) \
		((_class)->type)
	Type Stg_Class_GetTypeFunc( void* _class ); 
	#ifdef MACRO_AS_FUNC
		/** Get the class type */
		#define Stg_Class_GetType Stg_Class_GetTypeFunc
	#else
		/** Get the class type */
		#define Stg_Class_GetType Stg_Class_GetTypeMacro
	#endif
	
	
	/* Private member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#endif /* __Base_Foundation_Class_h__ */

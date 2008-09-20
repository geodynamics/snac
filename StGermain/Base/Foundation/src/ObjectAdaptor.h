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
**	Wrapper class allowing us to associate a Name with any StGermain object or non-StGermain object. If ownership is set to
**	true, then it will attempt to delete the object on deletion.
**
** Assumptions:
**	The attached ptr is to a StGermain class if the "OfClass" constructor(s) are used... there is no safe way to check.
**
** Comments:
**	Deep copying DOES NOT check if the object has already been copied (i.e. use ptrMap). Beacuse if we assume ownership, we
**		could not do so without an instance count machanism.
**	Deep copying when we are not the owner and it is not a StGermain class is not yet implemented.
**	When "OfPointer", 
**		*) The delete function should delete the content within the pointer (but NOT the pointer itself). It is useful
**			when the pointer is to something that contains a pointer itself. Pass NULL to do nothing.
**		*) The print function should print the contents of within the pointer. Pass NULL to print a default message.
**		*) The copy function should copy the content within the pointer. If NULL is given, the Stg_ObjectAdaptor has NO choice
**			but to firewall on being copied because it does not know how to copy the pointer (and a pointer copy
**			will be problematic at Delete time).
**
** $Id: ObjectAdaptor.h 2406 2004-12-09 01:20:49Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Base_Automation_ObjectAdaptor_h__
#define __Base_Automation_ObjectAdaptor_h__
	
	/** Delete/Print/Copy function prototypes for attached pointers (not StGermain classes) */
	/** *Note*: this function should only delete extra memory allocated by the object, _not
	the object itself_ - the ObjectAdaptor wrapper takes responsibility for this */
	typedef void  (Stg_ObjectAdaptor_DeletePointerFunction)	( void* ptr );
	typedef void  (Stg_ObjectAdaptor_PrintPointerFunction)	( void* ptr, struct Stream* stream );
	typedef void* (Stg_ObjectAdaptor_CopyPointerFunction)	( 
									void*					ptr, 
									void*					dest,
									Bool					deep,
									Name					nameExt, 
									struct PtrMap*				ptrMap );
	
	extern const Type Stg_ObjectAdaptor_Type;
	
	/** Stg_ObjectAdaptor class */
	#define __Stg_ObjectAdaptor \
		__Stg_Object \
		\
		void*						dataPtr; \
		Bool						iOwn; \
		Bool						isGlobal; \
		Bool						isStgClass; \
		Stg_ObjectAdaptor_DeletePointerFunction*	ptrDelete; \
		Stg_ObjectAdaptor_PrintPointerFunction*		ptrPrint; \
		Stg_ObjectAdaptor_CopyPointerFunction*		ptrCopy;
	
	/** Defines key information about a Stg_ObjectAdaptor - see ObjectAdaptor.h. */
	struct _Stg_ObjectAdaptor{ __Stg_ObjectAdaptor };	
	
	
	/* Class Administration members ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	
	/** Create a new Stg_ObjectAdaptor, when the the object is a StGermain object. */ 
	Stg_ObjectAdaptor* Stg_ObjectAdaptor_NewOfClass( void* dataPtr, Name name, Bool iOwn, Bool isGlobal );
	
	/** Initialise a Stg_ObjectAdaptor, when the the object is a StGermain object. */
	void Stg_ObjectAdaptor_InitOfClass( Stg_ObjectAdaptor* self, void* objPtr, Name name, Bool iOwn, Bool isGlobal );
	
	
	/** Create a new Stg_ObjectAdaptor, when the the object is not a StGermain object.
		See note before Stg_ObjectAdaptor_DeletePointerFunction declaration 
		about what this function has to be responsible for */ 
	Stg_ObjectAdaptor* Stg_ObjectAdaptor_NewOfPointer( 
		void*						dataPtr, 
		Name						name, 
		Bool						iOwn,
		Bool						isGlobal,
		Stg_ObjectAdaptor_DeletePointerFunction*	ptrDelete,
		Stg_ObjectAdaptor_PrintPointerFunction*		ptrPrint,
		Stg_ObjectAdaptor_CopyPointerFunction*		ptrCopy );
	
	/** Initialise a Stg_ObjectAdaptor, when the the object is not a StGermain object. 
		See note before Stg_ObjectAdaptor_DeletePointerFunction declaration 
		about what this function has to be responsible for */ 
	void Stg_ObjectAdaptor_InitOfPointer( 
		Stg_ObjectAdaptor* 				self, 
		void*						dataPtr, 
		Name						name, 
		Bool						iOwn,
		Bool						isGlobal,
		Stg_ObjectAdaptor_DeletePointerFunction*	ptrDelete,
		Stg_ObjectAdaptor_PrintPointerFunction*		ptrPrint,
		Stg_ObjectAdaptor_CopyPointerFunction*		ptrCopy );
	
	
	/* Creation implementation */
	Stg_ObjectAdaptor* _Stg_ObjectAdaptor_New( 
		SizeT						_sizeOfSelf, 
		Type						type,
		Stg_Class_DeleteFunction*			_delete,
		Stg_Class_PrintFunction*			_print, 
		Stg_Class_CopyFunction*				_copy,
		Name						name,
		void*						dataPtr,
		Bool						iOwn,
		Bool						isGlobal,
		Bool						isStgClass,
		Stg_ObjectAdaptor_DeletePointerFunction*	ptrDelete,
		Stg_ObjectAdaptor_PrintPointerFunction*		ptrPrint,
		Stg_ObjectAdaptor_CopyPointerFunction*		ptrCopy );
	
	/* Initialisation implementation */
	void _Stg_ObjectAdaptor_Init( 
		Stg_ObjectAdaptor*				self, 
		void*						dataPtr, 
		Bool						iOwn,
		Bool						isGlobal,
		Bool						isStgClass,
		Stg_ObjectAdaptor_DeletePointerFunction*	ptrDelete,
		Stg_ObjectAdaptor_PrintPointerFunction*		ptrPrint,
		Stg_ObjectAdaptor_CopyPointerFunction*		ptrCopy );
	
	
	/* Stg_Class_Delete() implementation */
	void _Stg_ObjectAdaptor_Delete( void* objectAdaptor );
	
	/* Stg_Class_Print() implementation */
	void _Stg_ObjectAdaptor_Print( void* objectAdaptor, struct Stream* stream );
	
	/* Stg_Class_Copy() implementation */
	void* _Stg_ObjectAdaptor_Copy( void* objectAdaptor, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap );
		

	/* Public member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	
	/* Obtain the object of the tuple. */
	#define Stg_ObjectAdaptor_ObjectMacro( self ) \
		( (self)->dataPtr )
	void* Stg_ObjectAdaptor_ObjectFunc( void* objectAdaptor );
	#ifdef MACRO_AS_FUNC
		/** Obtain the object of the tuple. */
		#define Stg_ObjectAdaptor_Object Stg_ObjectAdaptor_ObjectFunc
	#else
		/** Obtain the object of the tuple. */
		#define Stg_ObjectAdaptor_Object Stg_ObjectAdaptor_ObjectMacro
	#endif
	
	/* Do I own the object (i.e. will I attempt to delete it on my deletion)? */
	#define Stg_ObjectAdaptor_IOwnMacro( self ) \
		( (self)->iOwn )
	Bool Stg_ObjectAdaptor_IOwnFunc( void* objectAdaptor );
	#ifdef MACRO_AS_FUNC
		/** Do I own the object (i.e. will I attempt to delete it on my deletion)? */
		#define Stg_ObjectAdaptor_IOwn Stg_ObjectAdaptor_IOwnFunc
	#else
		/** Do I own the object (i.e. will I attempt to delete it on my deletion)? */
		#define Stg_ObjectAdaptor_IOwn Stg_ObjectAdaptor_IOwnMacro
	#endif
	
	/* Is the data a global object (i.e. there is and should be only one instance... don't deep copy or delete)? */
	#define Stg_ObjectAdaptor_IsGlobalMacro( self ) \
		( (self)->iOwn )
	Bool Stg_ObjectAdaptor_IsGlobalFunc( void* objectAdaptor );
	#ifdef MACRO_AS_FUNC
		/** Is the data a global object (i.e. there is and should be only one instance... don't deep copy or delete)? */
		#define Stg_ObjectAdaptor_IsGlobal Stg_ObjectAdaptor_IsGlobalFunc
	#else
		/** Is the data a global object (i.e. there is and should be only one instance... don't deep copy or delete)? */
		#define Stg_ObjectAdaptor_IsGlobal Stg_ObjectAdaptor_IsGlobalMacro
	#endif
	
	/* Has the object been marked as a StGermain class (i.e. will I attempt to call Stg_Class_Print, Stg_Class_Copy & 
	   Stg_Class_Delete on it where appropriate)? */
	#define Stg_ObjectAdaptor_IsStgClassMacro( self ) \
		( (self)->isStgClass )
	Bool Stg_ObjectAdaptor_IsStgClassFunc( void* objectAdaptor );
	#ifdef MACRO_AS_FUNC
		/** Has the object been marked as a StGermain class (i.e. will I attempt to call Stg_Class_Print, Stg_Class_Copy & 
		   Stg_Class_Delete on it where appropriate)? */
		#define Stg_ObjectAdaptor_IsStgClass Stg_ObjectAdaptor_IsStgClassFunc
	#else
		/** Has the object been marked as a StGermain class (i.e. will I attempt to call Stg_Class_Print, Stg_Class_Copy & 
		   Stg_Class_Delete on it where appropriate)? */
		#define Stg_ObjectAdaptor_IsStgClass Stg_ObjectAdaptor_IsStgClassMacro
	#endif
	
	/* Private member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#endif /* __Base_Automation_ObjectAdaptor_h__ */

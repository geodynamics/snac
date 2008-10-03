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
*/
/** \file
**  Role:
**	Defines the information that goes with a function pointer.
**
** Assumptions:
**
** Comments:
**
** $Id: ClassHook.h 2933 2005-05-12 06:13:10Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Base_Extensibility_ClassHook_h__
#define __Base_Extensibility_ClassHook_h__
	

	/** Textual name of this class */
	extern const Type ClassHook_Type;

	/** ClassHook struct- stores a func pointer plus info */
	#define __ClassHook \
		__Hook \
		\
		void*		reference;		/**< Reference to the instance this member function belongs to */
	
	struct ClassHook { __ClassHook };
	
	/* Create a new ClassHook */
	ClassHook* ClassHook_New( Name name, Func_Ptr funcPtr, char* addedBy, void* reference );
	
	/* Initialise an ClassHook */
	void ClassHook_Init( void* hook, Name name, Func_Ptr funcPtr, char* addedBy, void* reference );
	
	/* Creation implementation */
	ClassHook* _ClassHook_New( 
		SizeT 				_sizeOfSelf, 
		Type 				type, 
		Stg_Class_DeleteFunction* 	_delete,
		Stg_Class_PrintFunction*	_print,
		Stg_Class_CopyFunction*		_copy, 
		Name 				name, 
		Func_Ptr 			funcPtr,
		char*				addedBy,
		void*			reference );
	
	/* Initialisation implementation */
	void _ClassHook_Init( ClassHook* self, Func_Ptr funcPtr, char* addedBy, void* reference );
	
	
	/* Stg_Class_Delete implementation */
	void _ClassHook_Delete( void* hook );
	
	/* Print implementation */
	void _ClassHook_Print( void* hook, Stream* stream );
	
	/* Copy */
	#define ClassHook_Copy( self ) \
		(ClassHook*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define ClassHook_DeepCopy( self ) \
		(ClassHook*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _ClassHook_Copy( void* hook, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
#endif /* __Base_Extensibility_ClassHook_h__ */

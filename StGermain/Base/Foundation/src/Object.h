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
**	Abstract class for objects. Objects are named.
**
** <b>Assumptions:</b>
**	None
**
** <b>Comments:</b>
**	None
**
** $Id: Object.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**/

#ifndef __Base_Foundation_Object_h__
#define __Base_Foundation_Object_h__

	/** Textual name for Stg_Object class. */
	extern const Type Stg_Object_Type;

	/** \def __Stg_Object See Object */
	#define __Stg_Object \
		/* General info */ \
		__Stg_Class \
		\
		/* Virtual info */ \
		\
		/* Stg_Object info */ \
		Name				name; \
		AllocationType			nameAllocationType;
	
	struct _Stg_Object { __Stg_Object };
	
	
	/* Class Administration members ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	
	/** Constructor interface. */
	Stg_Object* _Stg_Object_New( 
		SizeT				_sizeOfSelf, 
		Type				type,
		Stg_Class_DeleteFunction*	_delete,
		Stg_Class_PrintFunction*	_print, 
		Stg_Class_CopyFunction*		_copy, 
		Name				name,
		AllocationType			nameAllocationType );
	
	/** Init interface. */
	void _Stg_Object_Init( Stg_Object* self, Name name, AllocationType nameAllocationType );
	
	/** Delete interface. */
	void _Stg_Object_Delete( void* object );
	
	/** Print interface. */
	void _Stg_Object_Print( void* object, struct Stream* stream );
	
	/** Copy interface. */
	void* _Stg_Object_Copy( void* object, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap );
	
	
	/* Public member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	
	/** Sets the name of the object. */
	void Stg_Object_SetName( void* object, Name name );
	
	
	/** \internal Get the object name. */
	#define _Stg_Object_GetNameMacro( object ) ( (object)->name )
	Name _Stg_Object_GetNameFunc( void* object );
	#ifdef MACRO_AS_FUNC
		/** Get the object name. */
		#define Stg_Object_GetName _Stg_Object_GetNameFunc
	#else
		#define Stg_Object_GetName _Stg_Object_GetNameMacro
	#endif
	
	/** Function which simply tacks on a suffix to an object's name of form "ObjectName-Suffix" 
	 *  Pointer returned must be free'd */
	Name Stg_Object_AppendSuffix( void* object, Name suffix ) ;
	
	/* Private member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#endif /* __Base_Foundation_Object_h__ */

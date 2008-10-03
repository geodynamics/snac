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
** Role:
**	Extensions are the effective equivalent of no-typing (i.e. the attributes/methods of am object are changable at runtime). 
**	By definition Object Oriented programming assumes "no-typing" ability, but popular Object Oriented languages such as C++
**	and Java (which are weakly-typed) have impeded its uptake. StGermain's ExtensionManager/ExtensionInfo combination mimics
**	"no-typing" in C by allowing users to add attributes to an object at run-time. The StGermain inheritance scheme already 
**	provides runtime changing of method implementations on an object. However note, given that StGermain was originally built
**	for computational purposes, speed requirements dictate that at some point the ability to add extensions is stopped, and the
**	object effectively becomes weakly-typed.
**
**	ClassPtrExtensionInfo concretes ExtensionInfo for extensions that are arrays. The Array class is used to manage the array.
**
** Assumptions:
**
** Comments:
**	Read ExtensionInfo and Array's Role, Assumptions and Comments.
**	The size of the extension is the size of the Array class. Initialisation of the Array is up to the user. It is assumed to be
**		done before this ClassPtrExtensionInfo is deleted (as it will attempt to delete it too).
**
** $Id: ClassPtrExtensionInfo.h 2915 2005-05-08 12:51:46Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Base_Extensibility_ClassPtrExtensionInfo_h__
#define __Base_Extensibility_ClassPtrExtensionInfo_h__
	
	/** Textual name of this class */
	extern const Type ClassPtrExtensionInfo_Type;

	typedef struct {
		void* ptr;
	} ClassPtrExtensionInfo_PtrClass;

	/** ClassPtrExtensionInfo struct */
	#define __ClassPtrExtensionInfo \
		__ExtensionInfo \
		Stg_Class_CopyFunction* copyFunc;
		
	/** Defines key information about an object/struct extension - see ClassPtrExtensionInfo.h. */
	struct ClassPtrExtensionInfo { __ClassPtrExtensionInfo };
	
	
	/* Class Administration members ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	
	/** Create a new ClassPtrExtensionInfo */
	ClassPtrExtensionInfo* ClassPtrExtensionInfo_New( const Name name, Stg_Class_CopyFunction* copyFunc, Index count );
	
	/** Initialise an ClassPtrExtensionInfo */
	void ClassPtrExtensionInfo_Init( void* arrayExtensionInfo, const Name name, Stg_Class_CopyFunction* copyFunc, Index count );
	
	/* Creation implementation */
	ClassPtrExtensionInfo* _ClassPtrExtensionInfo_New( 
		SizeT 				_sizeOfSelf, 
		Type 				type, 
		Stg_Class_DeleteFunction* 	_delete,
		Stg_Class_PrintFunction*	_print,
		Stg_Class_CopyFunction*		_copy, 
		ExtensionInfo_DataCopyFunction*	_dataCopy,
		Name 				name,
		Stg_Class_CopyFunction*		copyFunc,
		Index				count );
	
	/* Initialisation implementation */
	void _ClassPtrExtensionInfo_Init( ClassPtrExtensionInfo* self, Name name, Stg_Class_CopyFunction* copyFunc, Index count );
	
	/* Class_Delete() implementation */
	void _ClassPtrExtensionInfo_Delete( void* arrayExtensionInfo );
	
	/* Class_Print() implementation */
	void _ClassPtrExtensionInfo_Print( void* arrayExtensionInfo, Stream* stream );
	
	/** Class_Copy() implementation: derivatives should call this in their implementation */
	void* _ClassPtrExtensionInfo_Copy( void* extensionInfo, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	

	void* _ClassPtrExtensionInfo_DataCopy(
		void* extensionInfo,
		void* source,
		void* dest,
		Bool deep,
		Name nameExt,
		PtrMap* ptrMap );

	/* Public member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	/* Private member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#endif /* __Base_Extensibility_ClassPtrExtensionInfo_h__ */

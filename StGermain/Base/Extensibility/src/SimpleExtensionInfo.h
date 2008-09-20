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
**	SimpleExtensionInfo concretes ExtensionInfo for extensions that are merely a block of memory (e.g. a fundamental type or
**	struct) where none of its components reqiure extra behaviour to copy or delete (e.g. any pointers to malloced memory). 
**
** Assumptions:
**	Usage: A "memcpy" of the extension is sufficient an implementation of "copy"
**	Usage: It is ok to do nothing (i.e. not release any memory from pointers within the extension) on "delete"
**
** Comments:
**	Read ExtensionInfo's Role, Assumptions and Comments.
**
** $Id: SimpleExtensionInfo.h 2915 2005-05-08 12:51:46Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Base_Extensibility_SimpleExtensionInfo_h__
#define __Base_Extensibility_SimpleExtensionInfo_h__
	
	/** Textual name of this class */
	extern const Type SimpleExtensionInfo_Type;
	
	/** SimpleExtensionInfo struct */
	#define __SimpleExtensionInfo \
		__ExtensionInfo
		
	/** Defines key information about an object/struct extension - see SimpleExtensionInfo.h. */
	struct SimpleExtensionInfo { __SimpleExtensionInfo };
	
	
	/* Class Administration members ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	
	/** Create a new SimpleExtensionInfo */
	SimpleExtensionInfo* SimpleExtensionInfo_New( const Name name, SizeT size, Index count );
	
	/** Initialise an SimpleExtensionInfo */
	void SimpleExtensionInfo_Init( void* simpleExtensionInfo, const Name name, SizeT size, Index count );
	
	/* Creation implementation */
	SimpleExtensionInfo* _SimpleExtensionInfo_New( 
		SizeT 				_sizeOfSelf, 
		Type 				type, 
		Stg_Class_DeleteFunction* 	_delete,
		Stg_Class_PrintFunction*	_print,
		Stg_Class_CopyFunction*		_copy, 
		ExtensionInfo_DataCopyFunction*	_dataCopy,
		Name 				name, 
		SizeT 				size,
		Index				count );
	
	/* Initialisation implementation */
	void _SimpleExtensionInfo_Init( SimpleExtensionInfo* self, Name name, SizeT size, Index count );
	
	/* Class_Delete() implementation */
	void _SimpleExtensionInfo_Delete( void* simpleExtensionInfo );
	
	/* Class_Print() implementation */
	void _SimpleExtensionInfo_Print( void* simpleExtensionInfo, Stream* stream );
	
	/** Class_Copy() implementation: derivatives should call this in their implementation */
	void* _SimpleExtensionInfo_Copy( void* simpleExtensionInfo, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	void* _SimpleExtensionInfo_DataCopy( 
		void* extensionInfo, 
		void* source, 
		void* dest, 
		Bool deep, 
		Name nameExt, 
		PtrMap* ptrMap );

	/* Public member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	/* Private member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#endif /* __Base_Extensibility_SimpleExtensionInfo_h__ */

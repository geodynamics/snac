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
**	ExtensionInfo is an abstract class that defines the key information about an object/struct extension, but does not define
**	how an extension is to behave when it is being copied, printed or deleted. Specifically this class refers to ONE extension
**	to a class/object. The ExtensionManager manages the collection of extensions (ExtensionInfo, this class) to a class/object.
**
** Assumptions:
**	Conceptual: Architecturally assumes that extensions themselves are static in size (i.e. typed).
**	Conceptual: The user of the extension knows how to cast into a useful form the block of memory this class manages.
**	Usage: The physical offset (which we store here) of this extension in the collection of extensions MUST be provided by
**	ExtensionManager.
**
** Comments:
**	An ExtensionInfo (derivative) when constructed is in an invalid state. It must be registered with a manager (to obtain the
**	offset, etc) before this object is legally usable.
**	An ExtensionManager is required as it is the only way to obtain the actual extension (i.e. the block of memory), which is
**		likely not be be allocated at the time of this class' contruction.
**
** $Id: ExtensionInfo.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Base_Extensibility_ExtensionInfo_h__
#define __Base_Extensibility_ExtensionInfo_h__
	
	/** Textual name of this class */
	extern const Type ExtensionInfo_Type;

	typedef void* (ExtensionInfo_DataCopyFunction)( 
		void* extensionInfo, 
		void* source, 
		void* dest, 
		Bool deep, 
		Name nameExt,
		PtrMap* ptrMap );
	
	/** ExtensionInfo struct- stores a func pointer plus info */
	#define __ExtensionInfo \
		__Stg_Object \
		ExtensionInfo_DataCopyFunction*	_dataCopy;		/**< Virtual func which copies the data of extension. */ \
		Name				key;			/**< Pointer to const key */ \
		SizeT				originalSize;		/**< How big the extension is (as provided). */\
		SizeT				size;			/**< How big the extension is (word aligned). */\
		Bool				isRegistered;		/**< Is this ExtensionInfo registered with a manager. */\
		SizeT				itemSize;		/**< If array, the size of each item. */\
		Index				count;			/**< If array, the number of items. */\
		SizeT				offset;			/**< The extension's offset in memory. */\
		ExtensionManager*		extensionManager;	/**< The extension manager that this is part of. */\
		ExtensionInfo_Index		handle;			/**< The extension handle in the manager. */ \
		void*				data;			/**< If extending extended array, the data of extension.*/
		
	/** Defines key information about an object/struct extension - see ExtensionInfo.h. */
	struct ExtensionInfo { __ExtensionInfo };
	
	
	/* Class Administration members ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	
	/* Creation implementation */
	ExtensionInfo* _ExtensionInfo_New( 
		SizeT 				_sizeOfSelf, 
		Type 				type, 
		Stg_Class_DeleteFunction* 	_delete,	/**< Derivatives: implement how to delete the extension */
		Stg_Class_PrintFunction*	_print,		/**< Derivatives: implement how to print the extension */
		Stg_Class_CopyFunction*		_copy, 		/**< Derivatives: implement how to copy the extension */
		ExtensionInfo_DataCopyFunction*	_dataCopy,	/**< Derivatives: implement how to copy the extension data */
		Name 				name,
		SizeT 				size,
		Index				count );
	
	/* Initialisation implementation */
	void _ExtensionInfo_Init( ExtensionInfo* self, Name name, SizeT size, Index count );
	
	/** Class_Delete() implementation: derivatives should call this in their implementation */
	void _ExtensionInfo_Delete( void* extensionInfo );
	
	/** Class_Print() implementation: derivatives should call this in their implementation */
	void _ExtensionInfo_Print( void* extensionInfo, Stream* stream );
	
	/** Class_Copy() implementation: derivatives should call this in their implementation */
	void* _ExtensionInfo_Copy( void* extensionInfo, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	
	/* Public member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	/** Copyies the extended dats from source to dest */
	void* ExtensionInfo_DataCopy( void* extensionInfo, void* source, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/* Obtain the original (provided) size of the extension */
	#define ExtensionInfo_OriginalSizeMacro( self ) \
		( (self)->originalSize )
	SizeT ExtensionInfo_OriginalSizeFunc( void* extensionInfo );
	#ifdef MACRO_AS_FUNC
		/** Obtain the original (provided) size of the extension */
		#define ExtensionInfo_OriginalSize ExtensionInfo_OriginalSizeFunc
	#else
		/** Obtain the original (provided) size of the extension */
		#define ExtensionInfo_OriginalSize ExtensionInfo_OriginalSizeMacro
	#endif
	
	/* Obtain the size (word alignment compliant) of the extension */
	#define ExtensionInfo_SizeMacro( self ) \
		( (self)->size )
	SizeT ExtensionInfo_SizeFunc( void* extensionInfo );
	#ifdef MACRO_AS_FUNC
		/** Obtain the size (word alignment compliant) of the extension */
		#define ExtensionInfo_Size ExtensionInfo_SizeFunc
	#else
		/** Obtain the size (word alignment compliant) of the extension */
		#define ExtensionInfo_Size ExtensionInfo_SizeMacro
	#endif
	
	/* Obtain the size per item for extension which are arrays */
	#define ExtensionInfo_ItemSizeMacro( self ) \
		( (self)->itemSize )
	SizeT ExtensionInfo_ItemSizeFunc( void* extensionInfo );
	#ifdef MACRO_AS_FUNC
		/** Obtain the size per item for extension which are arrays */
		#define ExtensionInfo_ItemSize ExtensionInfo_ItemSizeFunc
	#else
		/** Obtain the size per item for extension which are arrays */
		#define ExtensionInfo_ItemSize ExtensionInfo_ItemSizeMacro
	#endif
	
	/* Obtain the count of items for extension which are arrays */
	#define ExtensionInfo_CountMacro( self ) \
		( (self)->count )
	SizeT ExtensionInfo_ItemSizeFunc( void* extensionInfo );
	#ifdef MACRO_AS_FUNC
		/** Obtain the count of items for extension which are arrays */
		#define ExtensionInfo_Count ExtensionInfo_CountFunc
	#else
		/** Obtain the count of items for extension which are arrays */
		#define ExtensionInfo_Count ExtensionInfo_CountMacro
	#endif
	
	/* Obtain the offset of the extension */
	#define ExtensionInfo_OffsetMacro( self ) \
		( (self)->offset )
	SizeT ExtensionInfo_OffsetFunc( void* extensionInfo );
	#ifdef MACRO_AS_FUNC
		/** Obtain the offset of the extension */
		#define ExtensionInfo_Offset ExtensionInfo_OffsetFunc
	#else
		/** Obtain the offset of the extension */
		#define ExtensionInfo_Offset ExtensionInfo_OffsetMacro
	#endif
	
	/* Has this extension info been registered with a manager? This object is useless until this occours. */
	#define ExtensionInfo_IsRegisteredMacro( self ) \
		( (self)->isRegistered )
	Bool ExtensionInfo_IsRegisteredFunc( void* extensionInfo );
	#ifdef MACRO_AS_FUNC
		/** Has this extension info been registered with a manager? This object is useless until this occours. */
		#define ExtensionInfo_IsRegistered ExtensionInfo_IsRegisteredFunc
	#else
		/** Has this extension info been registered with a manager? This object is useless until this occours. */
		#define ExtensionInfo_IsRegistered ExtensionInfo_IsRegisteredMacro
	#endif
	
	
	/* This function is a friend of ExtensionManager. It is called by ithe manager to register its info with this extension.
	   Registration is done by the manager on an Add. An ExtensionInfo is useless without being registered. */
	void ExtensionInfo_Register( 
		void*				extensionInfo,
		SizeT				offset,
		ExtensionManager*		extensionManager,
		ExtensionInfo_Index		handle,
		void*				data );
	
#endif /* __Base_Extensibility_ExtensionInfo_h__ */

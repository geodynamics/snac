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
** Role:
**	Keeps track of all the Extended data managers in use by a program, so they can be easily printed / deleted.
**
** Assumptions:
**	ExtensionRegister_Delete() deletes all the individial Extension managers it has pointers to ... so they
**	mustn't also be deleted in the user's code.
**
** Comments:
**	The original sizes need to be manually set by the user.... this whole system needs rethinking... it can be done better
**
** $Id: ExtensionManager_Register.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Base_Extensibility_ExtensionManager_Register_h__
#define __Base_Extensibility_ExtensionManager_Register_h__
	

	/** Textual name of this class */
	extern const Type ExtensionManager_Register_Type;
	
	/** ExtensionManager_Register class contents */
	#define __ExtensionManager_Register \
		/* General info */ \
		__Stg_Class \
		\
		/* Virtual info */ \
		\
		/* ExtensionManager_Register info */ \
		ExtensionList*				extensions;

	/** Keeps track of all the Extended data managers in use by a program - see ExtensionRegister.h */
	struct ExtensionManager_Register { __ExtensionManager_Register };
	
	/** Create a new ExtensionManager_Register */
	ExtensionManager_Register* ExtensionManager_Register_New( void );
	
	/** Initialise an ExtensionManager_Register */
	void ExtensionManager_Register_Init( void* extension_Register );
	
	/** Creation implementation / Virtual constructor */
	ExtensionManager_Register* _ExtensionManager_Register_New( 
		SizeT				_sizeOfSelf,
		Type				type,
		Stg_Class_DeleteFunction*		_delete,
		Stg_Class_PrintFunction*		_print,
		Stg_Class_CopyFunction*		_copy );
	
	/** Initialisation implementation */
	void _ExtensionManager_Register_Init( void* extension_Register );
	
	/** Stg_Class_Delete() implementation. Note that this also deletes the extension managers themselves ... so the user
	shouldn't manually delete them. */
	void _ExtensionManager_Register_Delete( void* extension_Register );
	
	/** Stg_Class_Print() implementation. */
	void _ExtensionManager_Register_Print( void* extension_Register, Stream* stream );
	
	/** Add a new Extension to the register. */
	ExtensionManager_Index ExtensionManager_Register_Add( void* extension_Register, void* extension );
	
	/** Get the handle of an extension in the register. */
	ExtensionManager_Index ExtensionManager_Register_GetExtensionHandle( void* extension_Register, Name toGet );
	
	/* Get an extension manager from the register. */
	#define ExtensionManager_Register_GetExtension( extension_Register, index ) \
		( ExtensionManager_FromList( (extension_Register)->extensions, index ) )

	ExtensionManager* _ExtensionManager_Register_At( void* extension_Register, ExtensionManager_Index handle );
	
#endif /* __Base_Extensibility_ExtensionManager_Register_h__ */

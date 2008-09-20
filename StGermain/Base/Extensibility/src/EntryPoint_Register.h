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
**
** Assumptions:
**
** Comments:
** TODO; convert this to use Stg_ObjectList to manage memory.
**
** $Id: EntryPoint_Register.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Base_Extensibility_EntryPoint_Register_h__
#define __Base_Extensibility_EntryPoint_Register_h__
	

	/* Textual name of this class */
	extern const Type EntryPoint_Register_Type;
	
	/* EntryPoint_Register info */
	#define __EntryPoint_Register \
		/* General info */ \
		__Stg_Class \
		\
		/* Virtual info */ \
		\
		/* EntryPoint_Register info */ \
		EntryPoint_Index			count; \
		SizeT					_size; \
		SizeT					_delta; \
		EntryPoint**				entryPoint;
	struct EntryPoint_Register { __EntryPoint_Register };
	
	
	/* Create a new EntryPoint_Register */
	EntryPoint_Register* EntryPoint_Register_New( void );
	
	/* Initialise a EntryPoint_Register */
	void EntryPoint_Register_Init( void* entryPoint_Register );
	
	/* Creation implementation / Virtual constructor */
	EntryPoint_Register* _EntryPoint_Register_New( 
		SizeT				_sizeOfSelf,
		Type				type,
		Stg_Class_DeleteFunction*		_delete,
		Stg_Class_PrintFunction*		_print,
		Stg_Class_CopyFunction*		_copy );
	
	/* Initialisation implementation */
	void _EntryPoint_Register_Init( void* entryPoint_Register );
	
	
	/* Stg_Class_Delete implementation */
	void _EntryPoint_Register_Delete( void* entryPoint_Register );
	
	/* Print implementation */
	void _EntryPoint_Register_Print( void* entryPoint_Register, Stream* stream );
	
	
	/* Add a new elementType */
	EntryPoint_Index EntryPoint_Register_Add( void* entryPoint_Register, void* entryPoint );
	
	/* Get the handle to an elementType */
	EntryPoint_Index EntryPoint_Register_GetHandle( void* entryPoint_Register, Type type );
	
	/* Get an element type from the register... SLOW */
	#define EntryPoint_Register_At( entryPoint_Register, handle )		((entryPoint_Register)->entryPoint[(handle)] )
	EntryPoint* _EntryPoint_Register_At( void* entryPoint_Register, EntryPoint_Index handle );
	
	/* If 'entryPoint' is in the register, return its index */
	EntryPoint_Index EntryPoint_Register_Find( void* entryPoint_Register, void* entryPoint );
	
#endif /* __Base_Extensibility_EntryPoint_Register_h__ */

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
**
**
** <b>Assumptions</b>
**	None
**
** <b>Comments</b>
**	None
**
** <b>Description</b>
**
**
** Comments:
**
** $Id: RegisterFactory.h 789 2004-02-10 17:15:11Z SteveQuenette $
**
**/

#ifndef __Base_IO_RegisterFactory_h__
#define __Base_IO_RegisterFactory_h__
	
	/** Textual name for RegisterFactory class. */
	extern const Type RegisterFactory_Type;
	
	/** Textual name for RegisterFactory_RegisterConstructorFunction class. */
	extern const Type RegisterFactory_RegisterConstructorFunction_Type;
	
	typedef void* (RegisterFactory_RegisterConstructorFunction) ( );

	/** \def __RegisterFactory_RegisterConstructor See RegisterFactory_RegisterConstructor */
	#define __RegisterFactory_RegisterConstructor \
		__Stg_Object \
		RegisterFactory_RegisterConstructorFunction*	_constructor;
		
	struct RegisterFactory_RegisterConstructor { __RegisterFactory_RegisterConstructor };

	RegisterFactory_RegisterConstructor* RegisterFactory_RegisterConstructor_New(
		Name						name,
		RegisterFactory_RegisterConstructorFunction*	_constructor );

	RegisterFactory_RegisterConstructor* _RegisterFactory_RegisterConstructor_New(
		SizeT						_sizeOfSelf,
		Type						type,
		Stg_Class_DeleteFunction*				_delete,
		Stg_Class_PrintFunction*				_print,
		Stg_Class_CopyFunction*				_copy, 
		Name						name,
		AllocationType					nameAllocationType,
		RegisterFactory_RegisterConstructorFunction*	_constructor );

	/** \def __RegisterFactory See RegisterFactory. */
	#define __RegisterFactory \
		/* General info */ \
		__Stg_Class \
		\
		Stg_ObjectList*	constructors;
		
	struct RegisterFactory { __RegisterFactory };

	RegisterFactory* RegisterFactory_New();

	RegisterFactory* _RegisterFactory_New(
		SizeT						_sizeOfSelf,
		Type						type,
		Stg_Class_DeleteFunction*				_delete,
		Stg_Class_PrintFunction*				_print,
		Stg_Class_CopyFunction*				_copy );
	
	void _RegisterFactory_Init( RegisterFactory* self );

	void _RegisterFactory_Delete( void* factory );

	void _RegisterFactory_Print( void* factory, Stream* stream );

	void RegisterFactory_Register( void* factory, Name name, void* _constructorFunction );

	void* RegisterFactory_Create( void* factory, Name name );

	
#endif /* __IO_RegisterFactoryFile_h__ */




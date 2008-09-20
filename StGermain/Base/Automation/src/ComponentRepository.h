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
**	Creator of arbitary components.
**
** Assumptions:
**
** Comments:
**	Does not assumes ownership of the factories registered (i.e. it will not free the memory associated to them ).
**	Do we eventually want to keep a registry of instantiated components?
**
** $Id: ComponentRepository.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Stg_Component_Stg_ComponentRepository_h__
#define __Stg_Component_Stg_ComponentRepository_h__
	
	/* Textual name of this class */
	extern const Type Stg_ComponentRepository_Type;
	
	/* Stg_ComponentRepository information */
	#define __Stg_ComponentRepository \
		/* General info */ \
		__Stg_Class \
		Dictionary*		dictionary; \
		\
		/* Virtual info */ \
		\
		/* Stg_ComponentRepository info */ \
		Stg_ComponentFactoryPtr*	factories; \
		Factory_Index		factoriesSize; \
		Factory_Index		factoriesDelta; \
		Factory_Index		factoryCount; 
	struct _Stg_ComponentRepository { __Stg_ComponentRepository };
	
	
	/* Create a new Stg_ComponentRepository and initialise */
	Stg_ComponentRepository* Stg_ComponentRepository_New( Dictionary* dictionary );
	
	/* Initialise a Stg_ComponentRepository construct */
	void Stg_ComponentRepository_Init( Stg_ComponentRepository* self, Dictionary* dictionary );
	
	/* Creation implementation / Virtual constructor */
	Stg_ComponentRepository* _Stg_ComponentRepository_New( 
		SizeT				_sizeOfSelf,
		Type				type,
		Stg_Class_DeleteFunction*		_delete,
		Stg_Class_PrintFunction*		_print,
		Dictionary*			dictionary );
	
	/* Initialise implementation */
	void _Stg_ComponentRepository_Init( Stg_ComponentRepository* self );
	
	/* Stg_Class_Delete a Stg_ComponentRepository implementation */
	void _Stg_ComponentRepository_Delete( void* componentRepository );
	
	/* Print a Stg_ComponentRepository implementation */
	void _Stg_ComponentRepository_Print( void* componentRepository );
	
	/* Register a new type of component. Stg_ComponentRepository does not assume ownership of the factory. */
	void Stg_ComponentRepository_Register( void* componentRepository, Stg_ComponentFactory* factory );
	
	/* Create a new component */
	void* Stg_ComponentRepository_Build( void* componentRepository, Category category, ClassType class );
	
#endif /* __Stg_Component_Stg_ComponentRepository_h__ */

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
** $Id: ComponentRepository.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "Base/Base.h"
#include "Context/Context.h"
#include "types.h"
#include "units.h"
#include "AbstractFactory.h"
#include "Stg_ComponentFactory.h"
#include "Stg_ComponentRepository.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

const Type Stg_ComponentRepository_Type = "Stg_ComponentRepository";


Stg_ComponentRepository* Stg_ComponentRepository_New( Dictionary* dictionary ) {
	return _Stg_ComponentRepository_New( sizeof(Stg_ComponentRepository), Stg_ComponentRepository_Type, _Stg_ComponentRepository_Delete,
		_Stg_ComponentRepository_Print, dictionary );
}

void Stg_ComponentRepository_Init( Stg_ComponentRepository* self, Dictionary* dictionary )
{
	/* General info */
	self->type = Stg_ComponentRepository_Type;
	self->_sizeOfSelf = sizeof(Stg_ComponentRepository);
	self->_deleteSelf = False;
	self->dictionary = dictionary;
	
	/* Virtual info */
	self->_delete = _Stg_ComponentRepository_Delete;
	self->_print = _Stg_ComponentRepository_Print;
	_Stg_Class_Init( (Stg_Class*)self );
	
	/* Stg_ComponentRepository */
	_Stg_ComponentRepository_Init( self );
}

Stg_ComponentRepository* _Stg_ComponentRepository_New( 
		SizeT				_sizeOfSelf,
		Type				type,
		Stg_Class_DeleteFunction*		_delete,
		Stg_Class_PrintFunction*		_print,
		Dictionary*			dictionary )
{
	Stg_ComponentRepository* self;
	
	/* Allocate memory */
	self = (Stg_ComponentRepository*)_Stg_Class_New( _sizeOfSelf, type, _delete, _print );
	
	/* General info */
	self->dictionary = dictionary;
	
	/* Virtual info */
	
	/* Stg_ComponentRepository info */
	_Stg_ComponentRepository_Init( self );
	
	return self;
}

void _Stg_ComponentRepository_Init( Stg_ComponentRepository* self ) {
	/* General and Virtual info should already be set */
	
	/* Stg_ComponentRepository info */
	self->factoriesSize = Dictionary_Entry_Value_AsUnsignedInt( Dictionary_GetDefault( self->dictionary, 
		"componentFactoriesSize", Dictionary_Entry_Value_FromUnsignedInt( 256 ) ) );
	self->factoriesDelta = Dictionary_Entry_Value_AsUnsignedInt( Dictionary_GetDefault( self->dictionary, 
		"componentFactoriesDelta", Dictionary_Entry_Value_FromUnsignedInt( 256 ) ) );
	self->factories = Memory_Alloc_Array( Stg_ComponentFactoryPtr, self->factoriesSize, "Stg_ComponentRepository->factories" );
	self->factories = 0;
}

void _Stg_ComponentRepository_Delete( void* componentRepository ) {
	Stg_ComponentRepository* self = (Stg_ComponentRepository*)componentRepository;
	
	if( self->factories ) {
		Memory_Free( self->factories );
	}
	
	/* Stg_Class_Delete parent */
	_Stg_Class_Delete( self );
}

void _Stg_ComponentRepository_Print( void* componentRepository ) {
	Stg_ComponentRepository* self = (Stg_ComponentRepository*)componentRepository;
	Factory_Index factoryIndex;
	
	/* Set the Journal for printing informations */
	Stream* componentRepositoryStream = Journal_Register( InfoStream_Type,	"Stg_ComponentRepositoryStream");
	
	/* General info */
	Journal_Printf( componentRepositoryStream, "Stg_ComponentRepository (%p):\n", self );
	
	/* Print parent */
	_Stg_Class_Print( self );
	
	/* Virtual info */
	
	/* Stg_ComponentRepository info */
	Journal_Printf( componentRepositoryStream, "\tfactoriesSize: %lu\n", self->factoriesSize );
	Journal_Printf( componentRepositoryStream, "\tfactoriesDelta: %lu\n", self->factoriesDelta );
	Journal_Printf( componentRepositoryStream, "\tfactoryCount: %lu\n", self->factoryCount );
	
	Journal_Printf( componentRepositoryStream, "\tfactories: %p\n", self->factories );
	Journal_Printf( componentRepositoryStream, "\tfactories[0-%lu]: ", self->factoryCount );
	for( factoryIndex = 0; factoryIndex < self->factoryCount; factoryIndex++ ) {
		Journal_Printf( componentRepositoryStream, "%p ", self->factories[factoryIndex] );
	}
	Journal_Printf( componentRepositoryStream, "\n" );
}

void Stg_ComponentRepository_Register( void* componentRepository, Stg_ComponentFactory* factory ) {
	Stg_ComponentRepository* self = (Stg_ComponentRepository*)componentRepository;
	
	if( self->factoryCount == self->factoriesSize ) {
		self->factoriesSize += self->factoriesDelta;
		self->factories = Memory_Realloc_Array( self->factories, Stg_ComponentFactoryPtr, self->factoriesSize );
		assert( self->factories );
	}
	self->factories[self->factoryCount] = factory;
}
	
void* Stg_ComponentRepository_Build( void* componentRepository, Category category, ClassType class ) {
	Stg_ComponentRepository* self = (Stg_ComponentRepository*)componentRepository;
	
	Factory_Index index;
	Stg_ComponentFactory* factory;
	
	factory = Stg_ComponentFactory_New( category, class, 0 );
	
	for( index = 0; index < self->factoryCount; index++ ) {
		if( Stg_ComponentFactory_IsEqual( self->factories[index], factory ) ) {
			Memory_Free( factory );
			return self->factories[index];
		}
	}
	return 0;
}

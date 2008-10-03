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
** $Id: MeshLayout.c 3883 2006-10-26 05:00:23Z KathleenHumble $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"

#include "types.h"
#include "shortcuts.h"
#include "ElementLayout.h"
#include "NodeLayout.h"
#include "MeshDecomp.h"
#include "MeshLayout.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


/* Textual name of this class */
const Type MeshLayout_Type = "MeshLayout";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

MeshLayout* MeshLayout_DefaultNew( Name name )
{
	return _MeshLayout_New( 
		sizeof(MeshLayout), 
		MeshLayout_Type, 
		_MeshLayout_Delete, 
		_MeshLayout_Print,
		_MeshLayout_Copy,
		(Stg_Component_DefaultConstructorFunction*)MeshLayout_DefaultNew,
		_MeshLayout_Construct,
		_MeshLayout_Build,
		_MeshLayout_Initialise,
		_MeshLayout_Execute,
		_MeshLayout_Destroy,
		name,
		False,
		NULL,
		NULL,
		NULL );
}

MeshLayout* MeshLayout_New(
		Name						name,
		ElementLayout*					elementLayout,
		NodeLayout*					nodeLayout,
		MeshDecomp*					decomp )
{
	return _MeshLayout_New( 
		sizeof(MeshLayout), 
		MeshLayout_Type, 
		_MeshLayout_Delete, 
		_MeshLayout_Print,
		_MeshLayout_Copy,
		(Stg_Component_DefaultConstructorFunction*)MeshLayout_DefaultNew,
		_MeshLayout_Construct,
		_MeshLayout_Build,
		_MeshLayout_Initialise,
		_MeshLayout_Execute,
		_MeshLayout_Destroy,
		name,
		True,
		elementLayout,
		nodeLayout,
		decomp );
}

void MeshLayout_Init(
		MeshLayout*					self,
		Name						name,
		ElementLayout*				elementLayout,
		NodeLayout*					nodeLayout,
		MeshDecomp*					decomp )
{
	/* General info */
	self->type = MeshLayout_Type;
	self->_sizeOfSelf = sizeof(MeshLayout);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _MeshLayout_Delete;
	self->_print = _MeshLayout_Print;
	self->_copy = _MeshLayout_Copy;
	self->_defaultConstructor = (Stg_Component_DefaultConstructorFunction*)MeshLayout_DefaultNew;
	self->_construct = _MeshLayout_Construct;
	self->_build = _MeshLayout_Build;
	self->_initialise = _MeshLayout_Initialise;
	self->_execute = _MeshLayout_Execute;
	self->_destroy = _MeshLayout_Destroy;
	
	/* MeshLayout info */

	_Stg_Class_Init( (Stg_Class*)self );
	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );
	_Stg_Component_Init( (Stg_Component*)self );
	_MeshLayout_Init( self, elementLayout, nodeLayout, decomp );
}

MeshLayout* _MeshLayout_New(
		SizeT						_sizeOfSelf, 
		Type						type,
		Stg_Class_DeleteFunction*				_delete,
		Stg_Class_PrintFunction*				_print,
		Stg_Class_CopyFunction*				_copy, 
		Stg_Component_DefaultConstructorFunction*	_defaultConstructor,
		Stg_Component_ConstructFunction*			_construct,
		Stg_Component_BuildFunction*				_build,
		Stg_Component_InitialiseFunction*			_initialise,
		Stg_Component_ExecuteFunction*				_execute,
		Stg_Component_DestroyFunction*				_destroy,
		Name									name,
		Bool							initFlag,
		ElementLayout*					elementLayout,
		NodeLayout*					nodeLayout,
		MeshDecomp*					decomp )
{
	MeshLayout* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(MeshLayout) );
	self = (MeshLayout*)_Stg_Component_New( _sizeOfSelf, type, _delete, _print, _copy, _defaultConstructor, _construct, _build, 
			_initialise, _execute, _destroy, name, NON_GLOBAL );
	
	/* General info */
	
	/* Virtual info */
	
	/* MeshLayout info */
	if( initFlag ){
		_MeshLayout_Init( self, elementLayout, nodeLayout, decomp );
	}
	
	return self;
}

void _MeshLayout_Init(
		MeshLayout*					self,
		ElementLayout*					elementLayout,
		NodeLayout*					nodeLayout,
		MeshDecomp*					decomp )
{
	/* General and Virtual info should already be set */
	
	/* MeshLayout info */
	self->isConstructed = True;
	self->elementLayout = elementLayout;
	self->nodeLayout = nodeLayout;
	self->decomp = decomp;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _MeshLayout_Delete( void* meshLayout ) {
	MeshLayout* self = (MeshLayout*)meshLayout;
	
	/* Stg_Class_Delete parent */
	_Stg_Class_Delete( self );
}


void _MeshLayout_Print( void* meshLayout, Stream* stream ) {
	MeshLayout* self = (MeshLayout*)meshLayout;
	
	/* Set the Journal for printing informations */
	Stream* meshLayoutStream;
	meshLayoutStream = Journal_Register( InfoStream_Type, "MeshLayoutStream" );

	/* Print parent */
	_Stg_Component_Print( self, stream );
	
	/* General info */
	Journal_Printf( stream, "MeshLayout (ptr): (%p)\n", self );
	
	/* Virtual info */
	
	/* MeshLayout info */
}

void* _MeshLayout_Copy( void* meshLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	MeshLayout*	self = (MeshLayout*)meshLayout;
	MeshLayout*	newMeshLayout;
	PtrMap*		map = ptrMap;
	Bool		ownMap = False;
	
	if( !map ) {
		map = PtrMap_New( 10 );
		ownMap = True;
	}
	
	newMeshLayout = (MeshLayout*)_Stg_Component_Copy( self, dest, deep, nameExt, map );
	
	if( deep ) {
		newMeshLayout->nodeLayout = (NodeLayout*)Stg_Class_Copy( self->nodeLayout, NULL, deep, nameExt, map );
		newMeshLayout->elementLayout = (ElementLayout*)Stg_Class_Copy( self->elementLayout, NULL, deep, nameExt, map );
		newMeshLayout->decomp = (MeshDecomp*)Stg_Class_Copy( self->decomp, NULL, deep, nameExt, map );
	}
	else {
		newMeshLayout->nodeLayout = self->nodeLayout;
		newMeshLayout->elementLayout = self->elementLayout;
		newMeshLayout->decomp = self->decomp;
	}
	
	if( ownMap ) {
		Stg_Class_Delete( map );
	}
	
	return (void*)newMeshLayout;
}

void _MeshLayout_Construct( void* meshLayout, Stg_ComponentFactory *cf, void* data ){
	MeshLayout *self = (MeshLayout*)meshLayout;
	ElementLayout *elementLayout = NULL;
	NodeLayout *nodeLayout = NULL;
	MeshDecomp *meshDecomp = NULL;

	elementLayout =  Stg_ComponentFactory_ConstructByKey( cf, self->name, ElementLayout_Type, ElementLayout, True, data );
	nodeLayout =  Stg_ComponentFactory_ConstructByKey(  cf,  self->name,  NodeLayout_Type, NodeLayout,  True, data ) ;
	meshDecomp =  Stg_ComponentFactory_ConstructByKey(  cf,  self->name,  MeshDecomp_Type, MeshDecomp,  True, data ) ;
	
	_MeshLayout_Init( self, elementLayout, nodeLayout, meshDecomp );
}
	
void _MeshLayout_Build( void* meshLayout, void *data ){
	MeshLayout *self = (MeshLayout*)meshLayout;

	/* Decomp needs to be built first */
	Build( self->decomp, data, False );
	Build( self->nodeLayout, data, False );
	Build( self->elementLayout, data, False );
	
}
	
void _MeshLayout_Initialise( void* meshLayout, void *data ){
	
}
	
void _MeshLayout_Execute( void* meshLayout, void *data ){
	
}
	
void _MeshLayout_Destroy( void* meshLayout, void *data ){
	
}

/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

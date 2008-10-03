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
** $Id: DummyMD.c 3883 2006-10-26 05:00:23Z KathleenHumble $
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
#include "DummyMD.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


/* Textual name of this class */
const Type DummyMD_Type = "DummyMD";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

DummyMD* DummyMD_DefaultNew( Name name ){
	
	return (DummyMD*)_DummyMD_New( 
		sizeof(DummyMD), 
		DummyMD_Type, 
		_DummyMD_Delete, 
		_DummyMD_Print,
		NULL,
		(Stg_Component_DefaultConstructorFunction*)DummyMD_DefaultNew,
		_DummyMD_Construct,
		_DummyMD_Build,
		_DummyMD_Initialise,
		_DummyMD_Execute,
		_DummyMD_Destroy,
		name,
		False,
		_MeshDecomp_Node_LocalToGlobal1D,
		_MeshDecomp_Node_DomainToGlobal1D,
		_MeshDecomp_Node_ShadowToGlobal1D, 
		_MeshDecomp_Node_GlobalToLocal1D, 
		_MeshDecomp_Node_GlobalToDomain1D, 
		_MeshDecomp_Node_GlobalToShadow1D,
		_MeshDecomp_Element_LocalToGlobal1D,
		_MeshDecomp_Element_DomainToGlobal1D,
		_MeshDecomp_Element_ShadowToGlobal1D, 
		_MeshDecomp_Element_GlobalToLocal1D, 
		_MeshDecomp_Element_GlobalToDomain1D, 
		_MeshDecomp_Element_GlobalToShadow1D,
		_DummyMD_Shadow_ProcCount,
		_DummyMD_Shadow_BuildProcs,
		_MeshDecomp_Shadow_ProcElementCount,
		_MeshDecomp_Proc_WithElement,
		NULL,
		MPI_COMM_WORLD,
		NULL,
		NULL );
}

DummyMD* DummyMD_New(
		Name						name,
		Dictionary*					dictionary,
		MPI_Comm					communicator,
		ElementLayout*					eLayout,
		NodeLayout*					nLayout )
{
	return _DummyMD_New( 
		sizeof(DummyMD), 
		DummyMD_Type, 
		_DummyMD_Delete, 
		_DummyMD_Print,
		NULL,
		(Stg_Component_DefaultConstructorFunction*)DummyMD_DefaultNew,
		_DummyMD_Construct,
		_DummyMD_Build,
		_DummyMD_Initialise,
		_DummyMD_Execute,
		_DummyMD_Destroy,
		name,
		True,
		_MeshDecomp_Node_LocalToGlobal1D,
		_MeshDecomp_Node_DomainToGlobal1D,
		_MeshDecomp_Node_ShadowToGlobal1D, 
		_MeshDecomp_Node_GlobalToLocal1D, 
		_MeshDecomp_Node_GlobalToDomain1D, 
		_MeshDecomp_Node_GlobalToShadow1D,
		_MeshDecomp_Element_LocalToGlobal1D,
		_MeshDecomp_Element_DomainToGlobal1D,
		_MeshDecomp_Element_ShadowToGlobal1D, 
		_MeshDecomp_Element_GlobalToLocal1D, 
		_MeshDecomp_Element_GlobalToDomain1D, 
		_MeshDecomp_Element_GlobalToShadow1D,
		_DummyMD_Shadow_ProcCount,
		_DummyMD_Shadow_BuildProcs,
		_MeshDecomp_Shadow_ProcElementCount,
		_MeshDecomp_Proc_WithElement,
		dictionary,
		communicator,
		eLayout,
		nLayout );
}


void DummyMD_Init(
		DummyMD*					self,
		Name						name,
		Dictionary*					dictionary,
		MPI_Comm					communicator,
		ElementLayout*					eLayout,
		NodeLayout*					nLayout )
{
	/* General info */
	self->type = DummyMD_Type;
	self->_sizeOfSelf = sizeof(DummyMD);
	self->_deleteSelf = False;
	self->dictionary = dictionary;
	
	/* Virtual info */
	self->_delete = _DummyMD_Delete;
	self->_print = _DummyMD_Print;
	self->_copy = NULL;
	self->_defaultConstructor = (Stg_Component_DefaultConstructorFunction*)DummyMD_DefaultNew;
	self->_construct = _DummyMD_Construct;
	self->_build = _DummyMD_Build;
	self->_initialise = _DummyMD_Initialise;
	self->_execute = _DummyMD_Execute;
	self->_destroy = _DummyMD_Destroy;
	self->nodeMapLocalToGlobal = _MeshDecomp_Node_LocalToGlobal1D;
	self->nodeMapDomainToGlobal = _MeshDecomp_Node_DomainToGlobal1D;
	self->nodeMapShadowToGlobal = _MeshDecomp_Node_ShadowToGlobal1D;
	self->nodeMapGlobalToLocal = _MeshDecomp_Node_GlobalToLocal1D;
	self->nodeMapGlobalToDomain = _MeshDecomp_Node_GlobalToDomain1D;
	self->nodeMapGlobalToShadow = _MeshDecomp_Node_GlobalToShadow1D;
	self->elementMapLocalToGlobal = _MeshDecomp_Element_LocalToGlobal1D;
	self->elementMapDomainToGlobal = _MeshDecomp_Element_DomainToGlobal1D;
	self->elementMapShadowToGlobal = _MeshDecomp_Element_ShadowToGlobal1D;
	self->elementMapGlobalToLocal = _MeshDecomp_Element_GlobalToLocal1D;
	self->elementMapGlobalToDomain = _MeshDecomp_Element_GlobalToDomain1D;
	self->elementMapGlobalToShadow = _MeshDecomp_Element_GlobalToShadow1D;
	self->shadowProcCount = _DummyMD_Shadow_ProcCount;
	self->shadowBuildProcs = _DummyMD_Shadow_BuildProcs;
	self->shadowProcElementCount = _MeshDecomp_Shadow_ProcElementCount;
	self->procWithElement = _MeshDecomp_Proc_WithElement;

	_Stg_Class_Init( (Stg_Class*)self );
	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );
	_Stg_Component_Init( (Stg_Component*)self );
	_MeshDecomp_Init( (MeshDecomp*)self, communicator, eLayout, nLayout, NULL );
	
	/* DummyMD info */
	_DummyMD_Init( self );
}


DummyMD* _DummyMD_New(
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
		Bool									initFlag,
		MeshDecomp_Node_MapLocalToGlobalFunction*       nodeMapLocalToGlobal,
		MeshDecomp_Node_MapDomainToGlobalFunction*      nodeMapDomainToGlobal,
		MeshDecomp_Node_MapShadowToGlobalFunction*      nodeMapShadowToGlobal,
		MeshDecomp_Node_MapGlobalToLocalFunction*       nodeMapGlobalToLocal,
		MeshDecomp_Node_MapGlobalToDomainFunction*      nodeMapGlobalToDomain,
		MeshDecomp_Node_MapGlobalToShadowFunction*      nodeMapGlobalToShadow,
		MeshDecomp_Element_MapLocalToGlobalFunction*	elementMapLocalToGlobal,
		MeshDecomp_Element_MapDomainToGlobalFunction*	elementMapDomainToGlobal,
		MeshDecomp_Element_MapShadowToGlobalFunction*	elementMapShadowToGlobal,
		MeshDecomp_Element_MapGlobalToLocalFunction*	elementMapGlobalToLocal,
		MeshDecomp_Element_MapGlobalToDomainFunction*	elementMapGlobalToDomain,
		MeshDecomp_Element_MapGlobalToShadowFunction*	elementMapGlobalToShadow,
		MeshDecomp_Shadow_ProcCountFunction*		shadowProcCount,
		MeshDecomp_Shadow_BuildProcsFunction*		shadowBuildProcs,
		MeshDecomp_Shadow_ProcElementCountFunction*     shadowProcElementCount,
		MeshDecomp_Proc_WithElementFunction*		procWithElement,
		Dictionary*					dictionary,
		MPI_Comm					communicator,
		ElementLayout*					eLayout,
		NodeLayout*					nLayout )
{
	DummyMD* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(DummyMD) );
	self = (DummyMD*)_MeshDecomp_New(
		_sizeOfSelf,
		type,
		_delete,
		_print,
		_copy,
		_defaultConstructor,
		_construct,
		_build,
		_initialise,
		_execute,
		_destroy,
		name,
		initFlag,
		nodeMapLocalToGlobal, 
		nodeMapDomainToGlobal,
		nodeMapShadowToGlobal,
		nodeMapGlobalToLocal,
		nodeMapGlobalToDomain,
		nodeMapGlobalToShadow, 
		elementMapLocalToGlobal,
		elementMapDomainToGlobal,
		elementMapShadowToGlobal, 
		elementMapGlobalToLocal,
		elementMapGlobalToDomain, 
		elementMapGlobalToShadow,
		shadowProcCount,
		shadowBuildProcs,
		shadowProcElementCount,
		procWithElement,
		dictionary, 
		communicator,
		eLayout,
		nLayout );
	
	/* General info */
	
	/* Virtual info */
	
	/* DummyMD info */
	if( initFlag ){
		_DummyMD_Init( self );
	}
	
	return self;
}


void _DummyMD_Init(
		DummyMD*					self )
{
	/* General and Virtual info should already be set */
	Element_GlobalIndex e_I;
	
	assert( self->rank == 0 );
	/* DummyMD info */
	
	self->isConstructed = True;
	self->procsInUse = 1;
	
	self->elementGlobalCount = self->elementLayout->elementCount;
	self->nodeGlobalCount = self->nodeLayout->nodeCount;
	
	self->localElementSets = Memory_Alloc( IndexSet*, "DummyMD" );
	self->localElementSets[0] = IndexSet_New( self->elementGlobalCount );
	for( e_I = 0; e_I < self->elementGlobalCount; e_I++ )
		IndexSet_Add( self->localElementSets[0], e_I );
	
	_MeshDecomp_DecomposeNodes( self );
	
	self->elementLocalCount = self->elementGlobalCount;
	self->elementShadowCount = 0;
	self->elementDomainCount = self->elementLocalCount;
	
	self->nodeLocalCount = self->nodeGlobalCount;
	self->nodeShadowCount = 0;
	self->nodeDomainCount = self->nodeLocalCount;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _DummyMD_Delete( void* dummyMD ) {
	DummyMD* self = (DummyMD*)dummyMD;
	
	/* Stg_Class_Delete parent */
	_MeshDecomp_Delete( self );
}

void _DummyMD_Print( void* dummyMD, Stream* stream ) {
	DummyMD* self = (DummyMD*)dummyMD;
	
	/* Set the Journal for printing informations */
	Stream* dummyMDStream;
	dummyMDStream = Journal_Register( InfoStream_Type, "DummyMDStream" );

	/* Print parent */
	_MeshDecomp_Print( self, stream );
	
	/* General info */
	Journal_Printf( stream, "DummyMD (ptr): (%p)\n", self );
	
	/* Virtual info */
	
	/* DummyMD info */
}

void _DummyMD_Construct( void* dummyMD, Stg_ComponentFactory *cf, void* data ){
	DummyMD*        self          = (DummyMD*)dummyMD;
	ElementLayout*  elementLayout = NULL;
	NodeLayout*     nodeLayout    = NULL;
	Stg_ObjectList* pointerRegister;

	self->dictionary = cf->rootDict;
	
	elementLayout =  Stg_ComponentFactory_ConstructByKey(  cf,  self->name,  ElementLayout_Type, ElementLayout,  True, data ) ;
	nodeLayout =  Stg_ComponentFactory_ConstructByKey(  cf,  self->name,  NodeLayout_Type, NodeLayout,  True, data ) ;
	pointerRegister = Stg_ObjectList_Get( cf->registerRegister, "Pointer_Register" );

	_MeshDecomp_Init( (MeshDecomp*)self, MPI_COMM_WORLD, elementLayout, nodeLayout, pointerRegister );
	_DummyMD_Init( self );
}
	
void _DummyMD_Build( void* dummyMD, void* data ){
	
}
	
void _DummyMD_Initialise( void* dummyMD, void* data ){
	
}
	
void _DummyMD_Execute( void* dummyMD, void* data ){
	
}
	
void _DummyMD_Destroy( void* dummyMD, void* data ){
	
}

Partition_Index _DummyMD_Shadow_ProcCount( void* dummyMD ) {
	return 0;
}


void _DummyMD_Shadow_BuildProcs( void* dummyMD, Partition_Index* procs ) {
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/


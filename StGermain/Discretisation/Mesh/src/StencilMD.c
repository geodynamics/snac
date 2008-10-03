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
** $Id: StencilMD.c 3462 2006-02-19 06:53:24Z WalterLandry $
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
#include "StencilMD.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


/* Textual name of this class */
const Type StencilMD_Type = "StencilMD";


/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

StencilMD* StencilMD_DefaultNew( )
{
	
}

StencilMD* StencilMD_New(
		Dictionary*					dictionary,
		MPI_Comm					communicator,
		ElementLayout*			 		elementLayout,
		NodeLayout*					nodeLayout,
		MeshLayout*					stencilLayout )
{
	return _StencilMD_New( 
		sizeof(StencilMD), 
		StencilMD_Type, 
		_StencilMD_Delete, 
		_StencilMD_Print,
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
		stencilLayout->decomp->shadowProcCount,
		stencilLayout->decomp->shadowBuildProcs, 
		_MeshDecomp_Shadow_ProcElementCount,
		_MeshDecomp_Proc_WithElement,
		dictionary,
		communicator,
		elementLayout,
		nodeLayout,
		stencilLayout );
}


void StencilMD_Init(
		StencilMD*					self,
		Dictionary*					dictionary,
		MPI_Comm					communicator,
		ElementLayout*					elementLayout,
		NodeLayout*					nodeLayout,
		MeshLayout*				 	stencilLayout )
{
	/* General info */
	self->type = StencilMD_Type;
	self->_sizeOfSelf = sizeof(StencilMD);
	self->_deleteSelf = False;
	self->dictionary = dictionary;
	
	/* Virtual info */
	self->_delete = _StencilMD_Delete;
	self->_print = _StencilMD_Print;
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
	self->shadowProcCount = stencilLayout->decomp->shadowProcCount;
	self->shadowBuildProcs = stencilLayout->decomp->shadowBuildProcs;
	self->shadowProcElementCount = _MeshDecomp_Shadow_ProcElementCount;
	self->procWithElement = _MeshDecomp_Proc_WithElement;
	_MeshDecomp_Init( (MeshDecomp*)self, communicator, elementLayout, nodeLayout );
	
	/* StencilMD info */
	_StencilMD_Init( self, stencilLayout );
}


StencilMD* _StencilMD_New(
		SizeT						_sizeOfSelf, 
		Type						type,
		Stg_Class_DeleteFunction*				_delete,
		Stg_Class_PrintFunction*				_print,
		Stg_Class_CopyFunction*					_copy,
		Stg_Component_DefaultConstructorFunction*	_defaultConstructor,
		Stg_Component_ConstructFunction*			_construct,
		Stg_Component_BuildFunction*				_build,
		Stg_Component_InitialiseFunction*			_initialise,
		Stg_Component_ExecuteFunction*				_execute,
		Stg_Component_DestroyFunction*				_destroy,
		Name									name,
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
		ElementLayout*					elementLayout,
		NodeLayout*					nodeLayout,
		MeshLayout*					stencilLayout )
{
	StencilMD* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(StencilMD) );
	self = (StencilMD*)_MeshDecomp_New(
		_sizeOfSelf,
		type,
		_delete,
		_print,
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
		elementLayout,
		nodeLayout );
	
	/* General info */
	
	/* Virtual info */
	
	/* StencilMD info */
	_StencilMD_Init( self, stencilLayout );
	
	return self;
}

void _StencilMD_Init(
		StencilMD*					self,
		MeshLayout*					stencilLayout )
{
	/* General and Virtual info should already be set */
	
	/* StencilMD info */
	self->stencilLayout = stencilLayout;
	
	if( self->rank < self->stencilLayout->decomp->procsInUse ) {
		ElementLayout*		eLayout = self->elementLayout;
		NodeLayout*		nLayout = self->nodeLayout;
		Index*			corners;
		Partition_Index		proc_I;
		Partition_Index*	procs;
		Index*			indices;
		Element_GlobalIndex     e_I;
		
		self->procsInUse = stencilLayout->decomp->procsInUse;
		self->storage = stencilLayout->decomp->storage;
		self->elementGlobalCount = eLayout->elementCount;
		self->nodeGlobalCount = nLayout->nodeCount;
		
		self->localElementSets = Memory_Alloc_Array( IndexSet*, self->procsInUse, "StencilMD->localElementSets" );
		for( proc_I = 0; proc_I < self->procsInUse; proc_I++ ) {
			if( stencilLayout->decomp->localElementSets[proc_I] )
				self->localElementSets[proc_I] = IndexSet_New( eLayout->elementCount );
			else
				self->localElementSets[proc_I] = NULL;
		}
		
		corners = Memory_Alloc_Array( Index, eLayout->elementCornerCount, "corners" );
		procs = Memory_Alloc_Array( Partition_Index, eLayout->elementCornerCount, "procs" );
		
		for( e_I = 0; e_I < eLayout->elementCount; e_I++ ) {
			Index p_I;
			
			eLayout->buildCornerIndices( eLayout, e_I, corners );
			
			for( p_I = 0; p_I < eLayout->elementCornerCount; p_I++ ) {
				Coord			point;
				Element_GlobalIndex     element;
				
				eLayout->geometry->pointAt( eLayout->geometry, corners[p_I], point );
				element = stencilLayout->elementLayout->elementWithPoint( stencilLayout->elementLayout, point );
				if( element >= stencilLayout->elementLayout->elementCount )
					break;
				
				procs[p_I] = stencilLayout->decomp->procWithElement( stencilLayout->decomp, element );
				if( procs[p_I] >= stencilLayout->decomp->procsInUse )
					break;
			}
			
			if( p_I < eLayout->elementCornerCount )
				continue;
			
			for( p_I = 1; p_I < eLayout->elementCornerCount; p_I++ )
				if( procs[p_I] != procs[p_I - 1] )
					break;
			
			if( p_I < eLayout->elementCornerCount )
				continue;
			
			IndexSet_Add( self->localElementSets[procs[p_I]], e_I );
		}
		
		_MeshDecomp_DecomposeNodes( self );
		
		IndexSet_GetMembers( self->localElementSets[self->rank], &self->elementLocalCount, &indices );
		Memory_Free( indices );
		self->elementShadowCount = 0;
		self->elementDomainCount = self->elementLocalCount + self->elementShadowCount;
		
		IndexSet_GetMembers( self->localNodeSets[self->rank], &self->nodeLocalCount, &indices );
		Memory_Free( indices );
		self->nodeShadowCount = 0;
		self->nodeDomainCount = self->nodeLocalCount + self->nodeShadowCount;
	}
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _StencilMD_Delete( void* stencilMD ) {
	StencilMD* self = (StencilMD*)stencilMD;
	
	/* Stg_Class_Delete parent */
	_MeshDecomp_Delete( self );
}


void _StencilMD_Print( void* stencilMD, Stream* stream ) {
	StencilMD* self = (StencilMD*)stencilMD;
	
	/* Set the Journal for printing informations */
	Stream* stencilMDStream;
	stencilMDStream = Journal_Register( InfoStream_Type, "StencilMDStream" );

	/* Print parent */
	_MeshDecomp_Print( self, stream );
	
	/* General info */
	Journal_Printf( stream, "StencilMD (ptr): (%p)\n", self );
	
	/* Virtual info */
	
	/* StencilMD info */
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/


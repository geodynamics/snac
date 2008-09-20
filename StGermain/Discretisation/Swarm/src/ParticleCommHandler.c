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
** $Id: ParticleCommHandler.c 3885 2006-10-26 05:52:26Z KathleenHumble $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"
#include "Discretisation/Mesh/Mesh.h"
#include "Discretisation/Utils/Utils.h"

#include "types.h"
#include "shortcuts.h"
#include "ParticleCommHandler.h"

#include "SwarmClass.h"
#include "CellLayout.h"
#include "ElementCellLayout.h"
#include "StandardParticle.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

const Type ParticleCommHandler_Type = "ParticleCommHandler";

/* MPI tags */
static const int SHADOW_PARTICLE_COUNTS_PER_CELL = 10;
static const int SHADOW_PARTICLES = 20;

ParticleCommHandler* ParticleCommHandler_DefaultNew( Name name )
{
	return _ParticleCommHandler_New( sizeof(ParticleCommHandler), ParticleCommHandler_Type,
		_ParticleCommHandler_Delete, _ParticleCommHandler_Print, _ParticleCommHandler_Copy,
		(Stg_Component_DefaultConstructorFunction*)ParticleCommHandler_DefaultNew,
		_ParticleCommHandler_Construct, _ParticleCommHandler_Build, _ParticleCommHandler_Initialise,
		_ParticleCommHandler_Execute, _ParticleCommHandler_Destroy, name, False,
		_ParticleCommHandler_HandleParticleMovementBetweenProcs, NULL );
}

ParticleCommHandler* ParticleCommHandler_New( 
		Name name,
		void* swarm )
{
	return _ParticleCommHandler_New( sizeof(ParticleCommHandler), ParticleCommHandler_Type,
		_ParticleCommHandler_Delete, _ParticleCommHandler_Print, _ParticleCommHandler_Copy,
		(Stg_Component_DefaultConstructorFunction*)ParticleCommHandler_DefaultNew,
		_ParticleCommHandler_Construct, _ParticleCommHandler_Build, _ParticleCommHandler_Initialise,
		_ParticleCommHandler_Execute, _ParticleCommHandler_Destroy, name, True,
		_ParticleCommHandler_HandleParticleMovementBetweenProcs, swarm );
}

ParticleCommHandler* _ParticleCommHandler_New( 
		SizeT								_sizeOfSelf,
		Type								type,
		Stg_Class_DeleteFunction*						_delete,
		Stg_Class_PrintFunction*						_print,
		Stg_Class_CopyFunction*						_copy, 
		Stg_Component_DefaultConstructorFunction*	_defaultConstructor,
		Stg_Component_ConstructFunction*			_construct,
		Stg_Component_BuildFunction*		_build,
		Stg_Component_InitialiseFunction*		_initialise,
		Stg_Component_ExecuteFunction*		_execute,
		Stg_Component_DestroyFunction*		_destroy,
		Name							name,
		Bool							initFlag,
		ParticleCommHandler_HandleParticleMovementBetweenProcsFunction	handleParticleMovementBetweenProcs,
		void*								swarm )
{
	ParticleCommHandler* self;
	
	/* Allocate memory */
	self = (ParticleCommHandler*)_Stg_Component_New( _sizeOfSelf, type, _delete, _print, _copy, _defaultConstructor,
		   _construct, _build, _initialise, _execute, _destroy, name, NON_GLOBAL );
	
	/* General info */
	/* Virtual info */
	self->_handleParticleMovementBetweenProcs = handleParticleMovementBetweenProcs;
	
	/* ParticleCommHandler info */
	if( initFlag ){
		_ParticleCommHandler_Init( self, swarm );
	}
	
	return self;
}


void _ParticleCommHandler_Init( ParticleCommHandler* self, void* swarm ) {
	self->isConstructed = True;
	self->swarm = (Swarm*) swarm;
	self->debug = Stream_RegisterChild( Swarm_Debug, self->type );
	self->shadowParticlesLeavingMeIndices = NULL;
	self->shadowParticlesLeavingMeTotalCount = 0;
	self->shadowParticlesLeavingMeUnfilledCount = 0;
	self->currShadowParticleLeavingMeIndex = 0;
	self->particlesOutsideDomainIndices = NULL;
	self->particlesOutsideDomainTotalCount = 0;
	self->particlesOutsideDomainUnfilledCount = 0;
	self->currParticleLeavingMyDomainIndex = 0;
	self->defensive = True;
}


void _ParticleCommHandler_Delete(void* pCommsHandler ) {
	/* Nothing to do, as the temporary arrays are deleted each step as soon as they are finished with. */
	_Stg_Class_Delete( pCommsHandler );
}


void _ParticleCommHandler_Print( void* pCommsHandler, Stream* stream ) {
	ParticleCommHandler*	self = (ParticleCommHandler*)pCommsHandler;
	
	/* General info */
	Journal_Printf( stream, "ParticleCommHandler (ptr): %p\n", self );
	
	/* Parent class info */
	_Stg_Class_Print( self, stream );

	/* Virtual info */
	Journal_Printf( stream, "self->_handleParticleMovementBetweenProcs (ptr): %p\n", 
		self->_handleParticleMovementBetweenProcs );

	/* class info */	
	Journal_Printf( stream, "self->swarm (ptr): %p\n", self->swarm ); 
	Journal_Printf( stream, "self->shadowParticlesLeavingMeTotalCount: %d\n", self->shadowParticlesLeavingMeTotalCount ); 
	Journal_Printf( stream, "self->shadowParticlesLeavingMeUnfilledCount: %d\n", self->shadowParticlesLeavingMeUnfilledCount ); 
	Journal_Printf( stream, "self->currShadowParticleLeavingMeIndex: %d\n", self->currShadowParticleLeavingMeIndex ); 
	Journal_Printf( stream, "self->shadowParticlesLeavingMeIndices (ptr): %p\n", self->shadowParticlesLeavingMeIndices ); 
	Journal_Printf( stream, "self->particlesOutsideDomainTotalCount: %d\n", self->particlesOutsideDomainTotalCount ); 
	Journal_Printf( stream, "self->particlesOutsideDomainUnfiledCount: %d\n", self->particlesOutsideDomainUnfilledCount ); 
	Journal_Printf( stream, "self->currParticleLeavingMyDomainIndex: %d\n", self->currParticleLeavingMyDomainIndex ); 
	Journal_Printf( stream, "self->particlesOutsideDomainIndices(ptr): %p\n", self->particlesOutsideDomainIndices ); 
}


void* _ParticleCommHandler_Copy( void* particleCommHandler, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	ParticleCommHandler*	self = (ParticleCommHandler*)particleCommHandler;
	ParticleCommHandler*	newParticleCommHandler;
	PtrMap*			map = ptrMap;
	Bool			ownMap = False;
	
	if( !map ) {
		map = PtrMap_New( 10 );
		ownMap = True;
	}
	
	newParticleCommHandler = _Stg_Class_Copy( self, dest, deep, nameExt, map );
	
	/* Virtual methods */
	newParticleCommHandler->_handleParticleMovementBetweenProcs = self->_handleParticleMovementBetweenProcs;
	
	newParticleCommHandler->shadowParticlesLeavingMeTotalCount = self->shadowParticlesLeavingMeTotalCount;
	newParticleCommHandler->shadowParticlesLeavingMeUnfilledCount = self->shadowParticlesLeavingMeUnfilledCount;
	newParticleCommHandler->currShadowParticleLeavingMeIndex = self->currShadowParticleLeavingMeIndex;
	newParticleCommHandler->particlesOutsideDomainTotalCount = self->particlesOutsideDomainTotalCount;
	newParticleCommHandler->particlesOutsideDomainUnfilledCount = self->particlesOutsideDomainUnfilledCount;
	newParticleCommHandler->currParticleLeavingMyDomainIndex = self->currParticleLeavingMyDomainIndex;
	
	if( deep ) {
		newParticleCommHandler->debug = (Stream*)Stg_Class_Copy( self->debug, NULL, deep, nameExt, map );
		newParticleCommHandler->swarm = (Swarm*)Stg_Class_Copy( self->swarm, NULL, deep, nameExt, map );
		
		if( (newParticleCommHandler->shadowParticlesLeavingMeIndices = PtrMap_Find( map, self->shadowParticlesLeavingMeIndices )) == NULL && self->shadowParticlesLeavingMeIndices ) {
			newParticleCommHandler->shadowParticlesLeavingMeIndices = Memory_Alloc_Array( Index, newParticleCommHandler->shadowParticlesLeavingMeTotalCount, "ParticleCommHandler->shadowParticlesLeavingMeIndices" );
			memcpy( newParticleCommHandler->shadowParticlesLeavingMeIndices, self->shadowParticlesLeavingMeIndices, sizeof(Index) * newParticleCommHandler->shadowParticlesLeavingMeTotalCount );
			PtrMap_Append( map, self->shadowParticlesLeavingMeIndices, newParticleCommHandler->shadowParticlesLeavingMeIndices );
		}
		
		if( (newParticleCommHandler->particlesOutsideDomainIndices = PtrMap_Find( map, self->particlesOutsideDomainIndices )) == NULL && self->particlesOutsideDomainIndices ) {
			newParticleCommHandler->particlesOutsideDomainIndices = Memory_Alloc_Array( Index, newParticleCommHandler->particlesOutsideDomainTotalCount, "ParticleCommHandler->particlesOutsideDomainIndices" );
			memcpy( newParticleCommHandler->particlesOutsideDomainIndices, self->particlesOutsideDomainIndices, sizeof(Index) * newParticleCommHandler->particlesOutsideDomainTotalCount );
			PtrMap_Append( map, self->particlesOutsideDomainIndices, newParticleCommHandler->particlesOutsideDomainIndices );
		}
		
		if( (newParticleCommHandler->shadowParticlesLeavingMeTotalCounts = PtrMap_Find( map, self->shadowParticlesLeavingMeTotalCounts )) == NULL && self->shadowParticlesLeavingMeTotalCounts ) {
			ShadowInfo*		cellShadowInfo = CellLayout_GetShadowInfo( self->swarm->cellLayout );
			ProcNbrInfo*		procNbrInfo = cellShadowInfo->procNbrInfo;
			Neighbour_Index		nbrCount = procNbrInfo->procNbrCnt;
			
			newParticleCommHandler->shadowParticlesLeavingMeTotalCounts = Memory_Alloc_Array( Particle_Index, nbrCount, "ParticleCommHandler->shadowParticlesLeavingMeTotalCounts" );
			memcpy( newParticleCommHandler->shadowParticlesLeavingMeTotalCounts, self->shadowParticlesLeavingMeTotalCounts, sizeof(Particle_Index) * nbrCount );
			PtrMap_Append( map, self->shadowParticlesLeavingMeTotalCounts, newParticleCommHandler->shadowParticlesLeavingMeTotalCounts );
		}
		
		if( (newParticleCommHandler->shadowParticlesLeavingMeCountsPerCell = PtrMap_Find( map, self->shadowParticlesLeavingMeCountsPerCell )) == NULL && self->shadowParticlesLeavingMeCountsPerCell ) {
			ShadowInfo*		cellShadowInfo = CellLayout_GetShadowInfo( self->swarm->cellLayout );
			ProcNbrInfo*		procNbrInfo = cellShadowInfo->procNbrInfo;
			Neighbour_Index		nbrCount = procNbrInfo->procNbrCnt;
			Neighbour_Index		nbr_I;
			
			newParticleCommHandler->shadowParticlesLeavingMeCountsPerCell = Memory_Alloc_2DComplex( Particle_Index, nbrCount, newParticleCommHandler->shadowParticlesLeavingMeTotalCounts, "ParticleCommHandler->shadowParticlesLeavingMeCountsPerCell" );
			for( nbr_I = 0; nbr_I < nbrCount; nbr_I++ ) {
				memcpy( newParticleCommHandler->shadowParticlesLeavingMeCountsPerCell[nbr_I], self->shadowParticlesLeavingMeCountsPerCell[nbr_I], sizeof(Particle_Index) * newParticleCommHandler->shadowParticlesLeavingMeTotalCounts[nbr_I] );
			}
			PtrMap_Append( map, self->shadowParticlesLeavingMeCountsPerCell, newParticleCommHandler->shadowParticlesLeavingMeCountsPerCell );
		}
		
		if( (newParticleCommHandler->shadowParticlesLeavingMe = PtrMap_Find( map, self->shadowParticlesLeavingMe )) == NULL && self->shadowParticlesLeavingMe ) {
			ShadowInfo*		cellShadowInfo = CellLayout_GetShadowInfo( self->swarm->cellLayout );
			ProcNbrInfo*		procNbrInfo = cellShadowInfo->procNbrInfo;
			Neighbour_Index		nbrCount = procNbrInfo->procNbrCnt;
			Neighbour_Index		nbr_I;
			
			newParticleCommHandler->shadowParticlesLeavingMe = Memory_Alloc_Array( Particle*, nbrCount, "ParticleCommHandler->shadowParticlesLeavingMe" );
			for( nbr_I = 0; nbr_I < nbrCount; nbr_I++ ) {
				SizeT	particlesArraySize = self->swarm->particleExtensionMgr->finalSize * newParticleCommHandler->shadowParticlesLeavingMeTotalCounts[nbr_I];
				
				newParticleCommHandler->shadowParticlesLeavingMe[nbr_I] = Memory_Alloc_Bytes( particlesArraySize, "Particle", "particleCommHandler->shadowParticlesLeavingMe[]" );
				memcpy( newParticleCommHandler->shadowParticlesLeavingMe[nbr_I], self->shadowParticlesLeavingMe[nbr_I], particlesArraySize );
			}
			PtrMap_Append( map, self->shadowParticlesLeavingMe, newParticleCommHandler->shadowParticlesLeavingMe );
		}
		
		if( (newParticleCommHandler->shadowParticlesLeavingMeHandles = PtrMap_Find( map, self->shadowParticlesLeavingMeHandles )) == NULL && self->shadowParticlesLeavingMeHandles ) {
			ShadowInfo*		cellShadowInfo = CellLayout_GetShadowInfo( self->swarm->cellLayout );
			ProcNbrInfo*		procNbrInfo = cellShadowInfo->procNbrInfo;
			Neighbour_Index		nbrCount = procNbrInfo->procNbrCnt;
			Neighbour_Index		nbr_I;
			
			newParticleCommHandler->shadowParticlesLeavingMeHandles = Memory_Alloc_Array( MPI_Request*, nbrCount, "ParticleCommHandler->shadowParticlesLeavingMeHandles" );
			for( nbr_I = 0; nbr_I < nbrCount; nbr_I++ ) {
				if( self->shadowParticlesLeavingMeHandles[nbr_I] ) {
					newParticleCommHandler->shadowParticlesLeavingMeHandles[nbr_I] = Memory_Alloc( MPI_Request, "particleCommHandler->shadowParticlesLeavingMeHandles[]" );
					memcpy( newParticleCommHandler->shadowParticlesLeavingMeHandles[nbr_I], self->shadowParticlesLeavingMeHandles[nbr_I], sizeof(MPI_Request) );
				}
				else {
					newParticleCommHandler->shadowParticlesLeavingMeHandles[nbr_I] = NULL;
				}
			}
			PtrMap_Append( map, self->shadowParticlesLeavingMeHandles, newParticleCommHandler->shadowParticlesLeavingMeHandles );
		}
		
		/*
		** Temporary
		*/
		
		if( (newParticleCommHandler->particlesArrivingFromNbrShadowCellsTotalCounts = PtrMap_Find( map, self->particlesArrivingFromNbrShadowCellsTotalCounts )) == NULL && self->particlesArrivingFromNbrShadowCellsTotalCounts ) {
			ShadowInfo*		cellShadowInfo = CellLayout_GetShadowInfo( self->swarm->cellLayout );
			ProcNbrInfo*		procNbrInfo = cellShadowInfo->procNbrInfo;
			Neighbour_Index		nbrCount = procNbrInfo->procNbrCnt;
			
			newParticleCommHandler->particlesArrivingFromNbrShadowCellsTotalCounts = Memory_Alloc_Array( Particle_Index, nbrCount, "ParticleCommHandler->particlesArrivingFromNbrShadowCellsTotalCounts" );
			memcpy( newParticleCommHandler->particlesArrivingFromNbrShadowCellsTotalCounts, self->particlesArrivingFromNbrShadowCellsTotalCounts, sizeof(Particle_Index) * nbrCount );
			PtrMap_Append( map, self->particlesArrivingFromNbrShadowCellsTotalCounts, newParticleCommHandler->particlesArrivingFromNbrShadowCellsTotalCounts );
		}
		
		if( (newParticleCommHandler->particlesArrivingFromNbrShadowCellCounts = PtrMap_Find( map, self->particlesArrivingFromNbrShadowCellCounts )) == NULL && self->particlesArrivingFromNbrShadowCellCounts ) {
			ShadowInfo*		cellShadowInfo = CellLayout_GetShadowInfo( self->swarm->cellLayout );
			ProcNbrInfo*		procNbrInfo = cellShadowInfo->procNbrInfo;
			Neighbour_Index		nbrCount = procNbrInfo->procNbrCnt;
			Neighbour_Index		nbr_I;
			
			newParticleCommHandler->particlesArrivingFromNbrShadowCellCounts = Memory_Alloc_2DComplex( Particle_Index, nbrCount, newParticleCommHandler->particlesArrivingFromNbrShadowCellsTotalCounts, "ParticleCommHandler->particlesArrivingFromNbrShadowCellCounts" );
			for( nbr_I = 0; nbr_I < nbrCount; nbr_I++ ) {
				memcpy( newParticleCommHandler->particlesArrivingFromNbrShadowCellCounts[nbr_I], self->particlesArrivingFromNbrShadowCellCounts[nbr_I], sizeof(Particle_Index) * newParticleCommHandler->particlesArrivingFromNbrShadowCellsTotalCounts[nbr_I] );
			}
			PtrMap_Append( map, self->particlesArrivingFromNbrShadowCellCounts, newParticleCommHandler->particlesArrivingFromNbrShadowCellCounts );
		}
		
		if( (newParticleCommHandler->particlesArrivingFromNbrShadowCells = PtrMap_Find( map, self->particlesArrivingFromNbrShadowCells )) == NULL && self->particlesArrivingFromNbrShadowCells ) {
			ShadowInfo*		cellShadowInfo = CellLayout_GetShadowInfo( self->swarm->cellLayout );
			ProcNbrInfo*		procNbrInfo = cellShadowInfo->procNbrInfo;
			Neighbour_Index		nbrCount = procNbrInfo->procNbrCnt;
			Neighbour_Index		nbr_I;
			
			newParticleCommHandler->particlesArrivingFromNbrShadowCells = Memory_Alloc_Array( Particle*, nbrCount, "ParticleCommHandler->particlesArrivingFromNbrShadowCells" );
			for( nbr_I = 0; nbr_I < nbrCount; nbr_I++ ) {
				SizeT	particlesArraySize = self->swarm->particleExtensionMgr->finalSize * newParticleCommHandler->particlesArrivingFromNbrShadowCellsTotalCounts[nbr_I];
				
				newParticleCommHandler->particlesArrivingFromNbrShadowCells[nbr_I] = Memory_Alloc_Bytes( particlesArraySize, "Particle", "particleCommHandler->particlesArrivingFromNbrShadowCells[]" );
				memcpy( newParticleCommHandler->particlesArrivingFromNbrShadowCells[nbr_I], self->particlesArrivingFromNbrShadowCells[nbr_I], particlesArraySize );
			}
			PtrMap_Append( map, self->particlesArrivingFromNbrShadowCells, newParticleCommHandler->particlesArrivingFromNbrShadowCells );
		}
		
		if( (newParticleCommHandler->particlesArrivingFromNbrShadowCellsHandles = PtrMap_Find( map, self->particlesArrivingFromNbrShadowCellsHandles )) == NULL && self->particlesArrivingFromNbrShadowCellsHandles ) {
			ShadowInfo*		cellShadowInfo = CellLayout_GetShadowInfo( self->swarm->cellLayout );
			ProcNbrInfo*		procNbrInfo = cellShadowInfo->procNbrInfo;
			Neighbour_Index		nbrCount = procNbrInfo->procNbrCnt;
			Neighbour_Index		nbr_I;
			
			newParticleCommHandler->particlesArrivingFromNbrShadowCellsHandles = Memory_Alloc_Array( MPI_Request*, nbrCount, "ParticleCommHandler->particlesArrivingFromNbrShadowCellsHandles" );
			for( nbr_I = 0; nbr_I < nbrCount; nbr_I++ ) {
				if( self->particlesArrivingFromNbrShadowCellsHandles[nbr_I] ) {
					newParticleCommHandler->particlesArrivingFromNbrShadowCellsHandles[nbr_I] = Memory_Alloc( MPI_Request, "particleCommHandler->particlesArrivingFromNbrShadowCellsHandles[]" );
					memcpy( newParticleCommHandler->particlesArrivingFromNbrShadowCellsHandles[nbr_I], self->particlesArrivingFromNbrShadowCellsHandles[nbr_I], sizeof(MPI_Request) );
				}
				else {
					newParticleCommHandler->particlesArrivingFromNbrShadowCellsHandles[nbr_I] = NULL;
				}
			}
			PtrMap_Append( map, self->particlesArrivingFromNbrShadowCellsHandles, newParticleCommHandler->particlesArrivingFromNbrShadowCellsHandles );
		}
		
		if( (newParticleCommHandler->particlesArrivingFromNbrShadowCellCountsHandles = PtrMap_Find( map, self->particlesArrivingFromNbrShadowCellCountsHandles )) == NULL && self->particlesArrivingFromNbrShadowCellCountsHandles ) {
			ShadowInfo*		cellShadowInfo = CellLayout_GetShadowInfo( self->swarm->cellLayout );
			ProcNbrInfo*		procNbrInfo = cellShadowInfo->procNbrInfo;
			Neighbour_Index		nbrCount = procNbrInfo->procNbrCnt;
			Neighbour_Index		nbr_I;
			
			newParticleCommHandler->particlesArrivingFromNbrShadowCellCountsHandles = Memory_Alloc_Array( MPI_Request*, nbrCount, "ParticleCommHandler->particlesArrivingFromNbrShadowCellCountsHandles" );
			for( nbr_I = 0; nbr_I < nbrCount; nbr_I++ ) {
				if( self->particlesArrivingFromNbrShadowCellCountsHandles[nbr_I] ) {
					newParticleCommHandler->particlesArrivingFromNbrShadowCellCountsHandles[nbr_I] = Memory_Alloc( MPI_Request, "particleCommHandler->particlesArrivingFromNbrShadowCellCountsHandles[]" );
					memcpy( newParticleCommHandler->particlesArrivingFromNbrShadowCellCountsHandles[nbr_I], self->particlesArrivingFromNbrShadowCellCountsHandles[nbr_I], sizeof(MPI_Request) );
				}
				else {
					newParticleCommHandler->particlesArrivingFromNbrShadowCellCountsHandles[nbr_I] = NULL;
				}
			}
			PtrMap_Append( map, self->particlesArrivingFromNbrShadowCellCountsHandles, newParticleCommHandler->particlesArrivingFromNbrShadowCellCountsHandles );
		}
	}
	else {
		newParticleCommHandler->debug = self->debug;
		newParticleCommHandler->swarm = self->swarm;
		newParticleCommHandler->shadowParticlesLeavingMeIndices = self->shadowParticlesLeavingMeIndices;
		newParticleCommHandler->particlesOutsideDomainIndices = self->particlesOutsideDomainIndices;
		newParticleCommHandler->shadowParticlesLeavingMeTotalCounts = self->shadowParticlesLeavingMeTotalCounts;
		newParticleCommHandler->shadowParticlesLeavingMeCountsPerCell = self->shadowParticlesLeavingMeCountsPerCell;
		newParticleCommHandler->shadowParticlesLeavingMe = self->shadowParticlesLeavingMe;
		newParticleCommHandler->shadowParticlesLeavingMeHandles = self->shadowParticlesLeavingMeHandles;
		newParticleCommHandler->particlesArrivingFromNbrShadowCellsTotalCounts = self->particlesArrivingFromNbrShadowCellsTotalCounts;
		newParticleCommHandler->particlesArrivingFromNbrShadowCellCounts = self->particlesArrivingFromNbrShadowCellCounts;
		newParticleCommHandler->particlesArrivingFromNbrShadowCells = self->particlesArrivingFromNbrShadowCells;
		newParticleCommHandler->particlesArrivingFromNbrShadowCellsHandles = self->particlesArrivingFromNbrShadowCellsHandles;
		newParticleCommHandler->particlesArrivingFromNbrShadowCellCountsHandles = self->particlesArrivingFromNbrShadowCellCountsHandles;
	}
	
	if( ownMap ) {
		Stg_Class_Delete( map );
	}
	
	return (void*)newParticleCommHandler;
}

void _ParticleCommHandler_Construct( void* pCommsHandler, Stg_ComponentFactory* cf, void* data ){
	
}
	
void _ParticleCommHandler_Build( void* pCommsHandler, void *data ){
	
}
	
void _ParticleCommHandler_Initialise( void* pCommsHandler, void *data ){
	
}
	
void _ParticleCommHandler_Execute( void* pCommsHandler, void *data ){
	
}

void _ParticleCommHandler_Destroy( void* pCommsHandler, void *data ){
	
}

void ParticleCommHandler_HandleParticleMovementBetweenProcs( void* pCommsHandler ) {
	ParticleCommHandler*	self = (ParticleCommHandler*)pCommsHandler;

	self->_handleParticleMovementBetweenProcs( self );
}

void _ParticleCommHandler_HandleParticleMovementBetweenProcs( void* pCommsHandler ) {
	ParticleCommHandler*	self = (ParticleCommHandler*)pCommsHandler;
	double                  startTime = 0;
	double                  myProcTime = 0;
	double*                 procTimes = NULL;
	double                  maxProcTime = 0;
	Processor_Index         proc_I = 0;
	Neighbour_Index         nbrCount = 0;
	Particle_Index          totalParticlesRecvdViaShadowFromNbrs = 0;
	Stream*                 info = Journal_Register( Info_Type, self->type );
	Particle_Index          globalParticlesArrivingMyDomainCount = 0;
	Particle_Index          globalParticlesOutsideDomainTotal = 0;
	
	Journal_DPrintfL( self->debug, 1, "In %s(), for swarm \"%s\":\n", __func__, self->swarm->name );
	if ( 1 == self->swarm->nProc ) {
		Journal_DPrintfL( self->debug, 1, "Serial run -> nothing to communicate, returning.\n" );
		Stream_UnIndentBranch( Swarm_Debug );
		return;
	}

	Journal_Printf( info, "Proc %d in %s() for swarm \"%s\": beginning comms...\n",
		self->swarm->myRank, __func__, self->swarm->name );
	startTime = MPI_Wtime();
	Stream_IndentBranch( Swarm_Debug );

	if ( self->swarm->cellShadowCount > 0 ) {
		ShadowInfo*		cellShadowInfo = CellLayout_GetShadowInfo( self->swarm->cellLayout );
		ProcNbrInfo*		procNbrInfo = cellShadowInfo->procNbrInfo;
		Neighbour_Index		nbr_I;
		Processor_Index		proc_I;
		Neighbour_Index		nbrCount = procNbrInfo->procNbrCnt;

		/* Allocate the recv count arrays and handles */
		self->particlesArrivingFromNbrShadowCellCounts = Memory_Alloc_2DComplex(
			Particle_Index, nbrCount, cellShadowInfo->procShadowedCnt,
			"ParticleCommHandler->particlesArrivingFromNbrShadowCellCounts" );
		self->particlesArrivingFromNbrShadowCellCountsHandles = Memory_Alloc_Array( MPI_Request*, nbrCount,
			"ParticleCommHandler->particlesArrivingFromNbrShadowCellCountsHandles" );
		for ( nbr_I=0; nbr_I < procNbrInfo->procNbrCnt; nbr_I++ ) {
			self->particlesArrivingFromNbrShadowCellCountsHandles[nbr_I] = Memory_Alloc( MPI_Request,
				"ParticleCommHandler->particlesArrivingFromNbrShadowCellCountsHandles[]" );
		}
		self->particlesArrivingFromNbrShadowCellsTotalCounts = Memory_Alloc_Array( Particle_Index, nbrCount,
			"ParticleCommHandler->particlesArrivingFromNbrShadowCellsTotalCounts" );

		for ( nbr_I=0; nbr_I < procNbrInfo->procNbrCnt; nbr_I++ ) {
			proc_I = procNbrInfo->procNbrTbl[nbr_I];
			MPI_Irecv( self->particlesArrivingFromNbrShadowCellCounts[nbr_I], cellShadowInfo->procShadowedCnt[nbr_I],
				MPI_UNSIGNED, proc_I, SHADOW_PARTICLE_COUNTS_PER_CELL, self->swarm->comm,
				self->particlesArrivingFromNbrShadowCellCountsHandles[nbr_I] );
		}

		self->shadowParticlesLeavingMeCountsPerCell = Memory_Alloc_2DComplex(
			Particle_Index, nbrCount, cellShadowInfo->procShadowCnt,
			"ParticleCommHandler->shadowParticlesLeavingMeCountsPerCell" );
		self->shadowParticlesLeavingMeTotalCounts = Memory_Alloc_Array( Particle_Index, nbrCount,
			"ParticleCommHandler->shadowParticlesLeavingMeTotalCounts" );

		_ParticleCommHandler_SendParticleTotalsInShadowCellsToNbrs( self );

		/* wait until recv of totals from nbrs complete, then start non-blocking recv of particles */
		{	
			Particle_Index			incomingCellParticleCount = 0;
			SizeT				particlesArraySize;
			MPI_Status			status;
			Cell_ShadowTransferIndex	stCell_I;
			Processor_Index			proc_I;

			self->particlesArrivingFromNbrShadowCellsHandles = Memory_Alloc_Array(
				MPI_Request*, procNbrInfo->procNbrCnt,
				"ParticleCommHandler->particlesArrivingFromNbrShadowCellsHandles");
			self->particlesArrivingFromNbrShadowCells = Memory_Alloc_Array(
				Particle*, procNbrInfo->procNbrCnt,
				"ParticleCommHandler->particlesArrivingFromNbrShadowCells" );
			
			/* TODO: may be worth converting the below into an MPI_Test loop */
			for ( nbr_I=0; nbr_I < procNbrInfo->procNbrCnt; nbr_I++ ) {
				
				proc_I = procNbrInfo->procNbrTbl[nbr_I];
				MPI_Wait( self->particlesArrivingFromNbrShadowCellCountsHandles[nbr_I], &status );
				Memory_Free( self->particlesArrivingFromNbrShadowCellCountsHandles[nbr_I] );
				
				Journal_DPrintfL( self->debug, 1, "proc %d: recv shadow counts from nbr %d (rank %d):\n",
					self->swarm->myRank, nbr_I, proc_I );
				Journal_DPrintfL( self->debug, 2, "\tare [" );
				self->particlesArrivingFromNbrShadowCellsTotalCounts[nbr_I] = 0;
				for ( stCell_I=0; stCell_I < cellShadowInfo->procShadowedCnt[nbr_I]; stCell_I++ ) {
					incomingCellParticleCount = self->particlesArrivingFromNbrShadowCellCounts[nbr_I][stCell_I];
					Journal_DPrintfL( self->debug, 2, "%d, ", incomingCellParticleCount );
					self->particlesArrivingFromNbrShadowCellsTotalCounts[nbr_I] += incomingCellParticleCount;
				}
				Journal_DPrintfL( self->debug, 2, "]\n" );
				Journal_DPrintfL( self->debug, 1, "(Proc %d):....totalled to %d\n", self->swarm->myRank,
					self->particlesArrivingFromNbrShadowCellsTotalCounts[nbr_I] );

				if ( self->particlesArrivingFromNbrShadowCellsTotalCounts[nbr_I] == 0 ) {
					/* No particles to receive from this proc -> just clear recv ptr */
					self->particlesArrivingFromNbrShadowCellsHandles[nbr_I] = NULL;
					self->particlesArrivingFromNbrShadowCells[nbr_I] = NULL;
				}	
				else { 
					self->particlesArrivingFromNbrShadowCellsHandles[nbr_I] = Memory_Alloc(
						MPI_Request,
						"ParticleCommHandler->particlesArrivingFromNbrShadowCellsHandles[]" );

					/* allocate particles recv array to right size */
					particlesArraySize = self->swarm->particleExtensionMgr->finalSize * 
						self->particlesArrivingFromNbrShadowCellsTotalCounts[nbr_I];
					self->particlesArrivingFromNbrShadowCells[nbr_I] = Memory_Alloc_Bytes( particlesArraySize,
						"Particle", "particleCommHandler->particlesArrivingFromNbrShadowCells[]" );
					/* start non-blocking recv of particles */
					MPI_Irecv( self->particlesArrivingFromNbrShadowCells[nbr_I], particlesArraySize, MPI_BYTE,
						proc_I, SHADOW_PARTICLES, self->swarm->comm,
						self->particlesArrivingFromNbrShadowCellsHandles[nbr_I] );
				}		
			}

			Memory_Free( self->particlesArrivingFromNbrShadowCellCountsHandles );
		}
			
		_ParticleCommHandler_NonBlockingSendParticlesInShadowCellsToNbrs( self );

		Memory_Free( self->shadowParticlesLeavingMeCountsPerCell );
	}
	
	/* Now check for particles that have moved further than the shadow cells */ 
	_ParticleCommHandler_FindParticlesThatHaveMovedOutsideMyDomain( self );

	_ParticleCommHandler_ShareAndUpdateParticlesThatHaveMovedOutsideDomains( self,
		&globalParticlesArrivingMyDomainCount,
		&globalParticlesOutsideDomainTotal );

	if ( self->swarm->cellShadowCount > 0 ) {
		_ParticleCommHandler_ReceiveAndUpdateShadowParticlesEnteringMyDomain( self );
	}

	/* final update of the 'holes' in my particles list, if any left, of particles that left */
	_ParticleCommHandler_FillRemainingHolesInLocalParticlesArray( self );

	if ( self->swarm->cellShadowCount > 0 ) {
		ShadowInfo*		cellShadowInfo = CellLayout_GetShadowInfo( self->swarm->cellLayout );
		ProcNbrInfo*		procNbrInfo = cellShadowInfo->procNbrInfo;
		Neighbour_Index		nbr_I;
		MPI_Status		status;
	
		/* MPI_Wait for all shadow sends to complete */
		for ( nbr_I=0; nbr_I < procNbrInfo->procNbrCnt; nbr_I++ ) {
			if ( self->shadowParticlesLeavingMeTotalCounts[nbr_I] > 0 ) {
				MPI_Wait( self->shadowParticlesLeavingMeHandles[nbr_I], &status );
				Memory_Free( self->shadowParticlesLeavingMe[nbr_I] );
				Memory_Free( self->shadowParticlesLeavingMeHandles[nbr_I] );
			}	
		}
		Memory_Free( self->shadowParticlesLeavingMe );
		Memory_Free( self->shadowParticlesLeavingMeHandles );
		Memory_Free( self->shadowParticlesLeavingMeTotalCounts );
		Memory_Free( self->shadowParticlesLeavingMeIndices );
	}


	/* Useful info for profiling... */
	myProcTime = MPI_Wtime() - startTime;

	totalParticlesRecvdViaShadowFromNbrs = 0;
	nbrCount = 0;
	if ( self->swarm->cellShadowCount > 0 ) {
		ShadowInfo*	        cellShadowInfo = CellLayout_GetShadowInfo( self->swarm->cellLayout );
		ProcNbrInfo*		procNbrInfo = cellShadowInfo->procNbrInfo;
		Neighbour_Index		nbr_I;

		nbrCount = procNbrInfo->procNbrCnt;
		for ( nbr_I=0; nbr_I < procNbrInfo->procNbrCnt; nbr_I++ ) {
			totalParticlesRecvdViaShadowFromNbrs += 
				self->particlesArrivingFromNbrShadowCellsTotalCounts[nbr_I];
		}	
	}


	for ( proc_I = 0; proc_I < self->swarm->nProc; proc_I++ ) {
		MPI_Barrier( self->swarm->comm );
		if ( self->swarm->myRank == proc_I ) {
			Journal_Printf( info, "...proc %d finished particle communication:\n", self->swarm->myRank );
			Stream_Indent( info );
			Journal_Printf( info, "- Particle comm totals via shadow cells (%d nbr procs): sent %d, recvd %d\n",
				nbrCount, self->shadowParticlesLeavingMeTotalCount,
				totalParticlesRecvdViaShadowFromNbrs );
			Journal_Printf( info, "- Particle comm totals via global synch (%d total procs): sent %d, recvd %d (of %d synched)\n", 
				self->swarm->nProc,
				self->particlesOutsideDomainTotalCount,
				globalParticlesArrivingMyDomainCount,
				globalParticlesOutsideDomainTotal );
			Journal_Printf( info, "- time taken = %.2f (secs)\n", myProcTime );
			Stream_UnIndent( info );
		}
	}

	procTimes = Memory_Alloc_Array( double, self->swarm->nProc, "procTimes" );
	MPI_Gather( &myProcTime, 1, MPI_DOUBLE, procTimes, 1, MPI_DOUBLE, 0, self->swarm->comm );
	if (self->swarm->myRank == 0 ) {
		for ( proc_I = 0; proc_I < self->swarm->nProc; proc_I++ ) {
			if ( procTimes[proc_I] > maxProcTime ) {
				maxProcTime = procTimes[proc_I];
			}
		}
		// TODO: print some stats on max particles sent/recvd and total sent/recvd
		Journal_Printf( info, "...Max Communication time by any proc was %.2f (secs)\n", maxProcTime );
	}
	Memory_Free( procTimes );
	MPI_Barrier( self->swarm->comm );

	/* clean up allocated memory, and zero counters, ready for next timestep */
	Memory_Free( self->particlesArrivingFromNbrShadowCellsTotalCounts );
	self->shadowParticlesLeavingMeIndices = NULL;
	self->currShadowParticleLeavingMeIndex = 0;
	self->shadowParticlesLeavingMeTotalCount = 0;
	self->shadowParticlesLeavingMeUnfilledCount = 0;
	Memory_Free( self->particlesOutsideDomainIndices );
	self->particlesOutsideDomainIndices = NULL;
	self->particlesOutsideDomainTotalCount = 0;
	self->particlesOutsideDomainUnfilledCount = 0;
	self->currParticleLeavingMyDomainIndex = 0;
	
	Stream_UnIndentBranch( Swarm_Debug );
}


void _ParticleCommHandler_SendParticleTotalsInShadowCellsToNbrs( ParticleCommHandler* self )
{	
	Cell_ShadowTransferIndex	stCell_I;
	Cell_DomainIndex		dCell_I;
	Index				nbr_I;
	Processor_Index			proc_I;
	Cell_ShadowTransferIndex	shadowCellsToProcCount;
	ShadowInfo*			cellShadowInfo = CellLayout_GetShadowInfo( self->swarm->cellLayout );
	ProcNbrInfo*			procNbrInfo = cellShadowInfo->procNbrInfo;
	Cell_PointIndex			currCellParticleCount;

	Journal_DPrintfL( self->debug, 1, "In %s():\n", __func__ );
	Stream_IndentBranch( Swarm_Debug );

	self->shadowParticlesLeavingMeTotalCount = 0;

	for ( nbr_I = 0; nbr_I < procNbrInfo->procNbrCnt; nbr_I++ ) {
		proc_I = procNbrInfo->procNbrTbl[nbr_I];
	
		shadowCellsToProcCount = cellShadowInfo->procShadowCnt[nbr_I];	
		Journal_DPrintfL( self->debug, 3, "Saving particle count in %d shadow cells going to nbr %d (proc %d):\n\t",
			shadowCellsToProcCount, nbr_I, proc_I );

		self->shadowParticlesLeavingMeTotalCounts[nbr_I] = 0;

		for ( stCell_I=0; stCell_I < shadowCellsToProcCount; stCell_I++ ) {
			dCell_I = cellShadowInfo->procShadowTbl[nbr_I][stCell_I];
			
			currCellParticleCount =  self->swarm->cellParticleCountTbl[dCell_I];
			Journal_DPrintfL( self->debug, 3, "(stCell_I=%d, dCell_I=%d, cnt=%d), ",
				stCell_I, dCell_I, currCellParticleCount );
			self->shadowParticlesLeavingMeCountsPerCell[nbr_I][stCell_I] = currCellParticleCount;
			self->shadowParticlesLeavingMeTotalCounts[nbr_I] += currCellParticleCount;
			self->shadowParticlesLeavingMeTotalCount += currCellParticleCount;
		}	
		Journal_DPrintfL( self->debug, 3, "\n" );

		MPI_Ssend( self->shadowParticlesLeavingMeCountsPerCell[nbr_I], shadowCellsToProcCount, MPI_UNSIGNED,
			proc_I, SHADOW_PARTICLE_COUNTS_PER_CELL, self->swarm->comm );
	}	

	self->shadowParticlesLeavingMeUnfilledCount = self->shadowParticlesLeavingMeTotalCount;
	Stream_UnIndentBranch( Swarm_Debug );
}


void _ParticleCommHandler_NonBlockingSendParticlesInShadowCellsToNbrs( ParticleCommHandler* self ) {	
	Cell_ShadowTransferIndex	stCell_I;
	Cell_DomainIndex		dCell_I;
	Neighbour_Index			nbr_I;
	Processor_Index			proc_I;
	Cell_ShadowTransferIndex	shadowCellsToProcCount;
	ShadowInfo*			cellShadowInfo = CellLayout_GetShadowInfo( self->swarm->cellLayout );
	ProcNbrInfo*			procNbrInfo = cellShadowInfo->procNbrInfo;
	Neighbour_Index			nbrCount = procNbrInfo->procNbrCnt;
	Particle_InCellIndex		currCellParticleCount;
	Particle_InCellIndex		cParticle_I;
	Particle_Index			lParticle_I;
	Particle_Index			tParticle_I=0; /*Index into the particle transfer array */
	Index				leavingParticle_I=0; /*Index into the array of all leaving particle indices */
	SizeT				particlesArrayBytes;
	#if DEBUG
	GlobalParticle*                 currParticle;
	#endif
	#if CAUTIOUS
	Bool*                           cellsClearedForTransfer = NULL;
	Neighbour_Index*                cellsClearedForTransferDests = NULL;
	Stream*                         errorStream = Journal_Register( Error_Type, self->type );
	#endif

	Journal_DPrintfL( self->debug, 1, "In %s():\n", __func__ );
	Stream_Indent( self->debug );

	#if CAUTIOUS
	/* These guys are used to check that we never try and send the same cell of particles twice */
	cellsClearedForTransfer = Memory_Alloc_Array( Bool, self->swarm->cellDomainCount,
		"cellsClearedForTransfer" );
	cellsClearedForTransferDests = Memory_Alloc_Array( Neighbour_Index, self->swarm->cellDomainCount,
		"cellsClearedForTransferDests" );
	for ( dCell_I = 0; dCell_I < self->swarm->cellDomainCount; dCell_I++ ) {
		cellsClearedForTransfer[dCell_I] = False;
		cellsClearedForTransferDests[dCell_I] = 0;
	}
	#endif

	/* Allocate the list of shadow particle indices to all nbrs */
	self->shadowParticlesLeavingMeIndices = Memory_Alloc_Array( Particle_Index, self->shadowParticlesLeavingMeTotalCount,
		"self->shadowParticlesLeavingMeIndices" );
	self->shadowParticlesLeavingMeHandles = Memory_Alloc_Array(
		MPI_Request*, procNbrInfo->procNbrCnt,
		"ParticleCommHandler->shadowParticlesLeavingMeHandles");
	self->shadowParticlesLeavingMe = Memory_Alloc_Array(
		Particle*, procNbrInfo->procNbrCnt,
		"ParticleCommHandler->shadowParticlesLeavingMe" );

	Journal_DPrintfL( self->debug, 1, "Sending the shadow particles going to my %d neighbours:\n", nbrCount );

	for ( nbr_I=0; nbr_I < nbrCount; nbr_I++ ) {
		tParticle_I=0; /* Reset index for new neighbour processor */

		if ( self->shadowParticlesLeavingMeTotalCounts[nbr_I] == 0 ) {
			/* If we're not sending any particles to this proc, skip to next */
			self->shadowParticlesLeavingMeHandles[nbr_I] = NULL;
			self->shadowParticlesLeavingMe[nbr_I] = NULL;
		}
		else {
			#ifdef CAUTIOUS
			Neighbour_Index      prevNbr_I;
			#endif

			shadowCellsToProcCount = cellShadowInfo->procShadowCnt[nbr_I];	
			proc_I = procNbrInfo->procNbrTbl[nbr_I];
			Journal_DPrintfL( self->debug, 3, "nbr %d (proc %d) - %d shadow cells going to it:\n",
				nbr_I, proc_I, shadowCellsToProcCount ); 
			#ifdef CAUTIOUS
			for ( prevNbr_I=0; prevNbr_I < nbr_I; prevNbr_I++ ) {
				Journal_Firewall( proc_I != procNbrInfo->procNbrTbl[prevNbr_I], errorStream,
					"Error - in %s(), on proc %u: found in our Swarm's ProcNbrInfo "
					"that our nbr %u is proc %u, but we already sent particles to "
					"that processor as nbr %u! Can't send same particles to same "
					"proc twice.\n", __func__, self->swarm->myRank, nbr_I, proc_I,
					prevNbr_I );
			}
			#endif

			Stream_Indent( self->debug );

			self->shadowParticlesLeavingMeHandles[nbr_I] = Memory_Alloc( MPI_Request,
				"ParticleCommHandler->shadowParticlesLeavingMeHandles[]" );
			particlesArrayBytes = self->swarm->particleExtensionMgr->finalSize * 
				self->shadowParticlesLeavingMeTotalCounts[nbr_I];
			self->shadowParticlesLeavingMe[nbr_I] = Memory_Alloc_Bytes( particlesArrayBytes,
				"Particle", "ParticleCommHandler->shadowParticlesLeavingMe[]" );

			for ( stCell_I=0; stCell_I < shadowCellsToProcCount; stCell_I++ ) {
				currCellParticleCount = self->shadowParticlesLeavingMeCountsPerCell[nbr_I][stCell_I];
				dCell_I = cellShadowInfo->procShadowTbl[nbr_I][stCell_I];
				#ifdef CAUTIOUS
				Journal_Firewall( cellsClearedForTransfer[dCell_I] == False, errorStream,
					"Error - in %s(), on proc %u: while trying to send shadow particles to "
					"nbr %u (proc %u), tried to copy particles from domain cell %u, but "
					"this cell has already had all its particles cleared for send to "
					"nbr %u (proc %u).\n", __func__, self->swarm->myRank, nbr_I, proc_I,
					dCell_I, cellsClearedForTransferDests[dCell_I],
					procNbrInfo->procNbrTbl[cellsClearedForTransferDests[dCell_I]] );
				#endif

				Journal_DPrintfL( self->debug, 3, "Processing Cell %d (%d particles):\n", dCell_I,
					currCellParticleCount );
			
				Stream_Indent( self->debug );

				for ( cParticle_I=0; cParticle_I < currCellParticleCount; cParticle_I++ ) {

					lParticle_I = self->swarm->cellParticleTbl[dCell_I][cParticle_I];
					#if DEBUG
					currParticle = (GlobalParticle*)Swarm_ParticleAt( self->swarm, lParticle_I );
					Journal_DPrintfL( self->debug, 3, "Copying PIC %d, particle %d at "
						"(%.2f,%.2f,%.2g) to shadowParticlesLeavingMe[%d][%d]\n",
						cParticle_I, lParticle_I,
						currParticle->coord[0], currParticle->coord[1], currParticle->coord[2],
						nbr_I, tParticle_I );
					#endif	
					Swarm_CopyParticleOffSwarm( self->swarm,
						self->shadowParticlesLeavingMe[nbr_I], tParticle_I++,
						lParticle_I );
					/* Note: we have no guarantee that the local particle index of where these
					shadow cells are leaving from is monotonically increasing: thus do an insertion
					at the right place. */
					{
						Index		insertionIndex = 0;
						Particle_Index*	currInsertionPtr;
						for ( ;insertionIndex < leavingParticle_I; insertionIndex++ ) {
							currInsertionPtr = &self->shadowParticlesLeavingMeIndices[insertionIndex];
							if (lParticle_I < (*currInsertionPtr) )
							{
								memmove(
									(Pointer)(((ArithPointer)currInsertionPtr) + sizeof(Particle_Index)), 
									currInsertionPtr,
									(leavingParticle_I - insertionIndex) * sizeof(Particle_Index) );

								(*currInsertionPtr) = lParticle_I;
								break;
							}
						}
						if ( insertionIndex == leavingParticle_I) {
							self->shadowParticlesLeavingMeIndices[leavingParticle_I] = lParticle_I;
						}
						leavingParticle_I++;
					}

				}
				Stream_UnIndent( self->debug );

				#ifdef CAUTIOUS
				cellsClearedForTransfer[dCell_I] = True;
				cellsClearedForTransferDests[dCell_I] = nbr_I;
				#endif
				/* Remember to clear the entries for that cell now. */
				self->swarm->cellParticleCountTbl[dCell_I] = 0;
				self->swarm->cellParticleSizeTbl[dCell_I] = 0;
				if ( self->swarm->cellParticleTbl[dCell_I] ) {
					Memory_Free( self->swarm->cellParticleTbl[dCell_I] );
				}
				self->swarm->cellParticleTbl[dCell_I] = NULL;

			}
			Stream_UnIndent( self->debug );

			/* non blocking send out particles */
			MPI_Issend( self->shadowParticlesLeavingMe[nbr_I],
				self->shadowParticlesLeavingMeTotalCounts[nbr_I] * self->swarm->particleExtensionMgr->finalSize,
				MPI_BYTE, proc_I, SHADOW_PARTICLES, self->swarm->comm,
				self->shadowParticlesLeavingMeHandles[nbr_I] );
		}
	}
	#if CAUTIOUS
	Memory_Free( cellsClearedForTransfer );
	Memory_Free( cellsClearedForTransferDests );
	#endif
	Stream_UnIndent( self->debug );
}


/* TODO: look at using MPI_Indexed instead */
void _ParticleCommHandler_ReceiveAndUpdateShadowParticlesEnteringMyDomain( ParticleCommHandler* self ) {
	MPI_Status	status;
	Cell_ShadowTransferIndex	stCell_I;
	Cell_LocalIndex			lCell_I;
	Neighbour_Index			nbr_I;
	Cell_ShadowTransferIndex	shadowCellsFromProcCount;
	ShadowInfo*			cellShadowInfo = CellLayout_GetShadowInfo( self->swarm->cellLayout );
	ProcNbrInfo*			procNbrInfo = cellShadowInfo->procNbrInfo;
	Neighbour_Index			nbrCount = procNbrInfo->procNbrCnt;
	Particle_InCellIndex		incomingCellParticleCount;
	Particle_InCellIndex		cParticle_I;
	Particle_Index			lParticle_I;
	Index				incomingParticle_I=0; /*Index into the array of all leaving particle indices */
	Index				incomingParticleSetsNotYetReceivedCount;
	Bool*				incomingParticlesReceived;
	#if DEBUG
	GlobalParticle*                 currParticle;
	#endif

	Journal_DPrintf( self->debug, "In %s():\n", __func__ );
	Stream_IndentBranch( Swarm_Debug );
	
	incomingParticlesReceived = Memory_Alloc_Array_Unnamed( Bool, nbrCount );

	/* Calculate how many particle sets we have to receive */
	incomingParticleSetsNotYetReceivedCount = 0;
	for ( nbr_I=0; nbr_I < nbrCount; nbr_I++ ) {
		incomingParticlesReceived[nbr_I] = False;
		if (self->particlesArrivingFromNbrShadowCellsTotalCounts[nbr_I] > 0) {
			incomingParticleSetsNotYetReceivedCount++;
		}
	}

	while ( incomingParticleSetsNotYetReceivedCount > 0 ) {
		int flag = 0;
		Journal_DPrintfL( self->debug, 3, "%d particle sets still to go...\n", incomingParticleSetsNotYetReceivedCount );
		for ( nbr_I=0; nbr_I < nbrCount; nbr_I++ ) {
			if ( (self->particlesArrivingFromNbrShadowCellsTotalCounts[nbr_I] > 0) &&
				(False == incomingParticlesReceived[nbr_I]) )
			{
				MPI_Test( self->particlesArrivingFromNbrShadowCellsHandles[nbr_I], &flag, &status );
				if ( False == flag ) {
					/* No results yet from this proc -> continue to next. */
					continue;
				}
				else {
					Journal_DPrintfL( self->debug, 3, "Received particles from nbr %d (proc %d):\n",
						nbr_I, procNbrInfo->procNbrTbl[nbr_I] );
					Stream_Indent( self->debug );

					incomingParticle_I = 0;
					shadowCellsFromProcCount = cellShadowInfo->procShadowedCnt[nbr_I];

				
					for ( stCell_I=0; stCell_I < shadowCellsFromProcCount; stCell_I++ ) {

						lCell_I = cellShadowInfo->procShadowedTbl[nbr_I][stCell_I];
						Journal_DPrintfL( self->debug, 3, "Incoming cell %d (local index %d):\n",
							stCell_I, lCell_I );
						Stream_Indent( self->debug );

						incomingCellParticleCount =
							self->particlesArrivingFromNbrShadowCellCounts[nbr_I][stCell_I];

						for ( cParticle_I=0; cParticle_I < incomingCellParticleCount; cParticle_I++ ) {	

							#if DEBUG
							currParticle = (GlobalParticle*)ParticleAt(
								self->particlesArrivingFromNbrShadowCells[nbr_I],
								incomingParticle_I,
								self->swarm->particleExtensionMgr->finalSize );
							Journal_DPrintfL( self->debug, 3, "Handling its PIC %d: - at "
								"(%.2f,%.2f,%.2f)\n", cParticle_I,
								currParticle->coord[0], currParticle->coord[1],
								currParticle->coord[2] );
							#endif

							Stream_Indent( self->debug );

							lParticle_I = _ParticleCommHandler_FindFreeSlotAndPrepareForInsertion( self );

							Swarm_CopyParticleOntoSwarm(
								self->swarm,
								lParticle_I,
								self->particlesArrivingFromNbrShadowCells[nbr_I], incomingParticle_I++ ); 

							Swarm_AddParticleToCell( self->swarm, lCell_I, lParticle_I );
							
							Stream_UnIndent( self->debug );
						}	
						Stream_UnIndent( self->debug );
					}
					incomingParticlesReceived[nbr_I] = True;
					incomingParticleSetsNotYetReceivedCount--;
					Memory_Free( self->particlesArrivingFromNbrShadowCells[nbr_I] );
					Memory_Free( self->particlesArrivingFromNbrShadowCellsHandles[nbr_I] );
					Stream_UnIndent( self->debug );
				}
			}
		}
	}	
	Memory_Free( incomingParticlesReceived );
	Memory_Free( self->particlesArrivingFromNbrShadowCells );
	Memory_Free( self->particlesArrivingFromNbrShadowCellsHandles );
	Memory_Free( self->particlesArrivingFromNbrShadowCellCounts );
	Stream_UnIndentBranch( Swarm_Debug );
}	


void _ParticleCommHandler_FindParticlesThatHaveMovedOutsideMyDomain( ParticleCommHandler* self )
{
	Particle_Index		particlesOutsideDomainSize = 0;
	GlobalParticle*         currParticle = NULL;
	Particle_Index		lParticle_I = 0;

	Journal_DPrintfL( self->debug, 1, "In %s():\n", __func__ );
	Stream_IndentBranch( Swarm_Debug );

	self->particlesOutsideDomainTotalCount = 0;
	particlesOutsideDomainSize = self->swarm->particlesArrayDelta;
	self->particlesOutsideDomainIndices = Memory_Alloc_Array( Particle_Index, particlesOutsideDomainSize,
		"self->particlesOutsideDomainIndices" );


	Journal_DPrintfL( self->debug, 1, "Checking the owning cell of each of my swarm's %d particles:\n",
		self->swarm->particleLocalCount );
	Stream_IndentBranch( Swarm_Debug );

	for ( lParticle_I=0; lParticle_I < self->swarm->particleLocalCount; lParticle_I++ ) {

		currParticle = (GlobalParticle*)Swarm_ParticleAt( self->swarm, lParticle_I );
		if ( currParticle->owningCell == self->swarm->cellDomainCount ) {
			Journal_DPrintfL( self->debug, 3, "particle %d has moved outside domain to (%.2f,%.2f,%.2f): "
				"saving index\n", lParticle_I, currParticle->coord[0], currParticle->coord[1],
								currParticle->coord[2] );
			if ( self->particlesOutsideDomainTotalCount == particlesOutsideDomainSize ) { 
				particlesOutsideDomainSize += self->swarm->particlesArrayDelta;
				Journal_DPrintfL( self->debug, 3, "(Need more memory to save indexes: increasing from %d to %d.)\n",
					self->particlesOutsideDomainTotalCount, particlesOutsideDomainSize );
				self->particlesOutsideDomainIndices = Memory_Realloc_Array( self->particlesOutsideDomainIndices,
					Particle_Index, particlesOutsideDomainSize );
			}
			self->particlesOutsideDomainIndices[self->particlesOutsideDomainTotalCount++] = lParticle_I;
		}	

	}	
	Stream_UnIndentBranch( Swarm_Debug );

	self->particlesOutsideDomainUnfilledCount = self->particlesOutsideDomainTotalCount;

	#if DEBUG
	{
		Particle_Index		particle_I = 0;
		if ( Stream_IsPrintableLevel( self->debug, 2 ) ) {
			Journal_DPrintf( self->debug, "%d Particles have moved outside my domain:\n\t[",
				self->particlesOutsideDomainTotalCount );
			for ( ; particle_I < self->particlesOutsideDomainTotalCount; particle_I++ ) {
				Journal_DPrintf( self->debug, "%d, ", self->particlesOutsideDomainIndices[particle_I] );
			}
			Journal_DPrintf( self->debug, "]\n" );
		}
	}
	#endif
	Stream_UnIndentBranch( Swarm_Debug );
}


void _ParticleCommHandler_ShareAndUpdateParticlesThatHaveMovedOutsideDomains(
		ParticleCommHandler* self,
		Particle_Index*      globalParticlesArrivingMyDomainCountPtr,
		Particle_Index*      globalParticlesOutsideDomainTotalPtr )
{
	Particle_Index*		globalParticlesOutsideDomainCounts = NULL;		
	Particle_Index		maxGlobalParticlesOutsideDomainCount = 0;		
	Processor_Index		proc_I = 0;
	Particle_Index		lParticle_I = 0;
	Particle_Index		particle_I = 0;

	Journal_DPrintfL( self->debug, 2, "In %s():\n", __func__ );
	Stream_IndentBranch( Swarm_Debug );

	(*globalParticlesArrivingMyDomainCountPtr) = 0;
	(*globalParticlesOutsideDomainTotalPtr) = 0;		

	/* Find the counts of particles	outside domain... */
	_ParticleCommHandler_GetCountOfParticlesOutsideDomainPerProcessor(
		self,
		&globalParticlesOutsideDomainCounts,
		&maxGlobalParticlesOutsideDomainCount,
		globalParticlesOutsideDomainTotalPtr );

	if ( (*globalParticlesOutsideDomainTotalPtr) > 0 ) {
		Particle*		particlesLeavingMyDomain = NULL;
		Particle*		globalParticlesLeavingDomains = NULL;
		SizeT			particlesLeavingDomainSizeBytes = 0;
		Cell_DomainIndex	lCell_I = 0;
		GlobalParticle*	        currParticle = NULL;
		Particle_Index		currProcParticlesOutsideDomainCount = 0;
		Particle_Index		currProcOffset = 0;
		Particle_Index		totalParticlesFoundEnteringDomains = 0;

		particlesLeavingDomainSizeBytes = self->swarm->particleExtensionMgr->finalSize
			* maxGlobalParticlesOutsideDomainCount;
		particlesLeavingMyDomain = Memory_Alloc_Bytes( particlesLeavingDomainSizeBytes, "Particle",
			"particlesLeavingMyDomain" );

		// TODO: investigate doing this with an MPI_Indexed datatype instead...
		Journal_DPrintfL( self->debug, 2, "Copying particles leaving my domain to temp. transfer array\n" );
		Stream_IndentBranch( Swarm_Debug );

		#if 0
		MPI_Type_indexed( 
			self->particlesOutsideDomainTotalCount,
			blocklens,
			self->particlesOutsideDomainIndices,//change to contiguous indices?
			MPI_BYTE,
			ParticlesLeavingDomainTransferIndexed
			);
		#endif	

		for ( particle_I=0; particle_I < self->particlesOutsideDomainTotalCount; particle_I++ ) {
			Journal_DPrintfL( self->debug, 3, "Copying particle %d to particlesLeavingMyDomain[%d]\n",
				self->particlesOutsideDomainIndices[particle_I], particle_I );
			Swarm_CopyParticleOffSwarm( self->swarm,
				particlesLeavingMyDomain, particle_I,
				self->particlesOutsideDomainIndices[particle_I] );
		}	
		Stream_UnIndentBranch( Swarm_Debug );

		/* allocate the big global receive buffer */
		globalParticlesLeavingDomains = Memory_Alloc_Bytes( particlesLeavingDomainSizeBytes * self->swarm->nProc,
			"Particle", "globalParticlesLeavingDomains" );

		Journal_DPrintfL( self->debug, 2, "Getting the global array of particles leaving domains\n" );
		MPI_Allgather( particlesLeavingMyDomain, particlesLeavingDomainSizeBytes, MPI_BYTE,
			globalParticlesLeavingDomains, particlesLeavingDomainSizeBytes, MPI_BYTE,
			self->swarm->comm );

		Journal_DPrintfL( self->debug, 2, "Checking through the global array of particles leaving domains, "
			"and snaffling those moving into my domain:\n" );
		Stream_IndentBranch( Swarm_Debug );
		for ( proc_I=0; proc_I < self->swarm->nProc; proc_I++ ) {

			if ( proc_I == self->swarm->myRank ) continue;

			currProcOffset = proc_I * maxGlobalParticlesOutsideDomainCount;
			currProcParticlesOutsideDomainCount = globalParticlesOutsideDomainCounts[proc_I];
			
			Journal_DPrintfL( self->debug, 3, "Checking particles that left proc. %d:\n", proc_I );
			for ( particle_I=0; particle_I < currProcParticlesOutsideDomainCount; particle_I++ ) {
				currParticle = (GlobalParticle*)ParticleAt( globalParticlesLeavingDomains,
					(currProcOffset + particle_I),
					self->swarm->particleExtensionMgr->finalSize );
				lCell_I = CellLayout_CellOf( self->swarm->cellLayout, currParticle );
				if ( lCell_I < self->swarm->cellLocalCount ) { 
					#if DEBUG
					Journal_DPrintfL( self->debug, 3, "Found particle at (%.2f,%.2f,%.2f) that's moved "
						"into my local cell %d...\n", currParticle->coord[0],
						currParticle->coord[1], currParticle->coord[2], lCell_I );
					#endif	
					
					/* copy particle to the lowest available slot in my particles array */
					lParticle_I = _ParticleCommHandler_FindFreeSlotAndPrepareForInsertion( self );

					Swarm_CopyParticleOntoSwarm( self->swarm, lParticle_I,
						globalParticlesLeavingDomains, (currProcOffset + particle_I) );
					Swarm_AddParticleToCell( self->swarm, lCell_I, lParticle_I );
					(*globalParticlesArrivingMyDomainCountPtr)++;
				}
				#if DEBUG
				else {
					currParticle = (GlobalParticle*)ParticleAt( globalParticlesLeavingDomains, 
						(currProcOffset + particle_I),
						self->swarm->particleExtensionMgr->finalSize );
					Journal_DPrintfL( self->debug, 3, "Ignoring particle at (%.2f,%.2f,%.2f) since "
						"not in my local cells...\n", currParticle->coord[0],
						currParticle->coord[1], currParticle->coord[2] );
				}
				#endif
			}		
		}	
		Stream_UnIndentBranch( Swarm_Debug );

		Memory_Free( particlesLeavingMyDomain );
		Memory_Free( globalParticlesLeavingDomains );

		/* Defensive check to make sure particles not lost/created accidentally somehow */
		if( self->defensive == True ) {
			MPI_Reduce( globalParticlesArrivingMyDomainCountPtr, &totalParticlesFoundEnteringDomains,
				    1, MPI_UNSIGNED, MPI_SUM, 0, self->swarm->comm );
			if ( 0 == self->swarm->myRank ) {
				Stream*   errorStream = Journal_Register( Error_Type, self->type );

				Journal_Firewall( totalParticlesFoundEnteringDomains == (*globalParticlesOutsideDomainTotalPtr),
						  errorStream, "Error - in %s(): %d particles were found across all processors to be "
						  "leaving the individual domains directly, but after sharing and searching %d were "
						  "found entering them directly! These must match as no particles should be "
						  "lost/created through advection.\n",
						  __func__, (*globalParticlesOutsideDomainTotalPtr),
						  totalParticlesFoundEnteringDomains );
			}
		}
	}	
	Memory_Free( globalParticlesOutsideDomainCounts );
	Stream_UnIndentBranch( Swarm_Debug );
}


void _ParticleCommHandler_GetCountOfParticlesOutsideDomainPerProcessor(
	ParticleCommHandler*	self,
	Particle_Index**	globalParticlesOutsideDomainCountsPtr,
	Particle_Index*		maxGlobalParticlesOutsideDomainCountPtr,
	Particle_Index*		globalParticlesOutsideDomainTotalPtr )
{
	Processor_Index		proc_I;

	(*globalParticlesOutsideDomainCountsPtr) = Memory_Alloc_Array( Particle_Index, self->swarm->nProc,
		"(*globalParticlesOutsideDomainCountsPtr)" );
		
	MPI_Allgather( (&self->particlesOutsideDomainTotalCount), 1, MPI_UNSIGNED, 
		(*globalParticlesOutsideDomainCountsPtr), 1, MPI_UNSIGNED, self->swarm->comm );
	
	(*globalParticlesOutsideDomainTotalPtr) = 0;
	for ( proc_I=0; proc_I < self->swarm->nProc; proc_I++ ) {
		(*globalParticlesOutsideDomainTotalPtr) += (*globalParticlesOutsideDomainCountsPtr)[proc_I];
		if ( (*globalParticlesOutsideDomainCountsPtr)[proc_I] > (*maxGlobalParticlesOutsideDomainCountPtr) ) {
			(*maxGlobalParticlesOutsideDomainCountPtr) = (*globalParticlesOutsideDomainCountsPtr)[proc_I];
		}
	}	

	#if DEBUG
	if ( Stream_IsPrintableLevel( self->debug, 2 ) ) {
		Journal_DPrintf( self->debug, "Global counts of particles moving outside domains:\n" );
		Journal_DPrintf( self->debug, "\tTotal: %d, Counts: [", (*globalParticlesOutsideDomainTotalPtr) );
		for ( proc_I=0; proc_I < self->swarm->nProc; proc_I++ ) {
			Journal_DPrintf( self->debug, "%d, ", (*globalParticlesOutsideDomainCountsPtr)[proc_I] );
		}	
		Journal_DPrintf( self->debug, "]\n" );
	}	
	#endif
}


Particle_Index _ParticleCommHandler_FindFreeSlotAndPrepareForInsertion( ParticleCommHandler* self )
{
	Particle_Index	lParticle_I = 0;

	if ( self->shadowParticlesLeavingMeUnfilledCount > 0 ) {
		Journal_DPrintfL( self->debug, 3, "Still %d holes available from "
			"particles leaving via shadow cells\n-> free slot to add into is %d\n",
			self->shadowParticlesLeavingMeUnfilledCount,
			self->shadowParticlesLeavingMeIndices[self->currShadowParticleLeavingMeIndex]);
			
		lParticle_I = self->shadowParticlesLeavingMeIndices[self->currShadowParticleLeavingMeIndex];

		self->currShadowParticleLeavingMeIndex++;
		self->shadowParticlesLeavingMeUnfilledCount--;
	}
	else if ( self->particlesOutsideDomainUnfilledCount ) {
		Journal_DPrintfL( self->debug, 3, "Still %d holes available from "
			"particles leaving domain direct\n-> free slot to add into is %d\n", 
			self->particlesOutsideDomainUnfilledCount,
			self->particlesOutsideDomainIndices[self->currParticleLeavingMyDomainIndex]);

		lParticle_I = self->particlesOutsideDomainIndices[self->currParticleLeavingMyDomainIndex];

		self->currParticleLeavingMyDomainIndex++;
		self->particlesOutsideDomainUnfilledCount--;
	}
	else {
		Journal_DPrintfL( self->debug, 3, "No holes left from leaving "
			"particles\n-> slot to insert into is end of array %d\n", 
			self->swarm->particleLocalCount );
		lParticle_I = self->swarm->particleLocalCount;
		if ( self->swarm->particleLocalCount == self->swarm->particlesArraySize ) {
			Journal_DPrintfL( self->debug, 3, "Particles array memory used up "
				"-> increasing from %d entries by %d\n",
				self->swarm->particlesArraySize, self->swarm->particlesArrayDelta );
			self->swarm->particlesArraySize += self->swarm->particlesArrayDelta;
			self->swarm->particles = Memory_Realloc_Array_Bytes(
				self->swarm->particles,
				self->swarm->particleExtensionMgr->finalSize,
				self->swarm->particlesArraySize );
		}
		self->swarm->particleLocalCount++;
	}

	return lParticle_I;
}


void _ParticleCommHandler_FillRemainingHolesInLocalParticlesArray( ParticleCommHandler* self )
{
	Particle_Index		prevParticlesArraySize = self->swarm->particlesArraySize;
	Particle_Index		numHolesToFill;
	Particle_InCellIndex	cParticle_I;
	StandardParticle*	oldPtrToMovedParticle;
	Cell_LocalIndex		owningCell;
	Particle_Index		indexToInsertAt;
	Particle_Index*		leavingParticlesArray = NULL;
	Index			currLeavingParticleArrayEntry = 0;
	Index			highestLeavingParticleArrayEntry;
	Index			leavingParticlesUnfilledCount = 0;
	Particle_Index		highestLeavingParticleIndex;
	Particle_Index		candidateParticleToMove;
	Bool			finishedFlag = False;
	Bool			mergedArrayCreated = False;
	Particle_Index		prevParticleCount = self->swarm->particleLocalCount;
	
	Journal_DPrintf( self->debug, "In %s():\n", __func__ );
	Stream_IndentBranch( Swarm_Debug );

	numHolesToFill = self->particlesOutsideDomainUnfilledCount + self->shadowParticlesLeavingMeUnfilledCount;
	if ( numHolesToFill == 0 ) {
		Journal_DPrintfL( self->debug, 2, "No holes to fill -> nothing to do, returning.\n" );
		Stream_UnIndentBranch( Swarm_Debug );
		return;
	}
	#if DEBUG
	if ( Stream_IsPrintableLevel( self->debug, 2 ) ) {
		_ParticleCommHandler_PrintParticleSlotsYetToFill( self );
	}	
	#endif

	/* work out the list we have to iterate over: */
	if ( self->shadowParticlesLeavingMeUnfilledCount && !self->particlesOutsideDomainUnfilledCount ) {
		Journal_DPrintfL( self->debug, 2, "Particles have only left via shadow cells -> no need to merge lists\n" );
		leavingParticlesArray = &self->shadowParticlesLeavingMeIndices[self->currShadowParticleLeavingMeIndex];
	}
	else if ( self->particlesOutsideDomainUnfilledCount && !self->shadowParticlesLeavingMeUnfilledCount ) {
		Journal_DPrintfL( self->debug, 2, "Particles have only left domain directly -> no need to merge lists\n" );
		leavingParticlesArray = &self->particlesOutsideDomainIndices[self->currParticleLeavingMyDomainIndex];
	} 
	else {
		Journal_DPrintfL( self->debug, 2, "Particles have left both via shadow cells and domain directly -> merge lists\n" );
		leavingParticlesArray = _ParticleCommHandler_MergeListsOfUnfilledParticleSlots( self );
		mergedArrayCreated = True;
	}

	/* Ok: while there are holes left to fill, find the highest candidate, move it, and reduce the count. */
	Journal_DPrintfL( self->debug, 2, "Starting run through the %d particles to fill:\n", numHolesToFill );
	Stream_IndentBranch( Swarm_Debug );

	currLeavingParticleArrayEntry = 0;
	highestLeavingParticleArrayEntry = numHolesToFill-1;
	leavingParticlesUnfilledCount = numHolesToFill;

	while ( leavingParticlesUnfilledCount > 0 ) {

		indexToInsertAt = leavingParticlesArray[currLeavingParticleArrayEntry];
		Journal_DPrintfL( self->debug, 3, "Attempting to fill leaving slot %d (at particle index %d):\n",
			currLeavingParticleArrayEntry, indexToInsertAt );

		Stream_Indent( self->debug );

		/* This is where we work out the index of which particle to move into the free slot.
		 * We Start from the end of the particles array, then decrement by 1 until we find a candidate that
		 * hasn't itself already left. 
		 * We also need to consider the possibility that every candidate higher than the current index
		 * has also left, in which case we are done, and finish the while process. 
		 * See the ParticleCommHandler Twiki page for diagrams illustrating this algorithm.
		 */

		candidateParticleToMove = self->swarm->particleLocalCount-1;
		highestLeavingParticleIndex = leavingParticlesArray[highestLeavingParticleArrayEntry];
		Journal_DPrintfL( self->debug, 3, "Searching for highest particle that hasn't also moved:\n" );
		Stream_Indent( self->debug );	

		while ( candidateParticleToMove == leavingParticlesArray[highestLeavingParticleArrayEntry] ) {
			/* Check if that was the last candidate particle above the current one: */
			/* This test needs to be at the top of this loop to handle the case where we have one
			particle that's leaving */

			if ( candidateParticleToMove <= indexToInsertAt ) {
				Journal_DPrintfL( self->debug, 3, "** No more particles above current "
					"hole %d to fill: we're done. **\n", indexToInsertAt );
				/* Need the line below to mark the fact we failed to fill the current indexToInsertAt hole */
				self->swarm->particleLocalCount--;
				finishedFlag = True;
				break;
			}

			Journal_DPrintfL( self->debug, 3, "Candidate particle %d has also left...\n",
				candidateParticleToMove );

			highestLeavingParticleArrayEntry--;
			highestLeavingParticleIndex = leavingParticlesArray[highestLeavingParticleArrayEntry];
			leavingParticlesUnfilledCount--;
			self->swarm->particleLocalCount--;
			candidateParticleToMove--;
		}	
		Stream_UnIndent( self->debug );	

		if ( True == finishedFlag ) {
			/* We must have hit the "no more candidate particles" criterion in the search loop, so
			 * quit trying to fill empty holes entirely. */
			Stream_UnIndent( self->debug );
			break;
		}

		Journal_DPrintfL( self->debug, 3, "Highest valid particle found at index %d:\n",
			candidateParticleToMove );
		Journal_DFirewall( (candidateParticleToMove > indexToInsertAt), Swarm_Error,
			"Error in %s: Empty hole filling\nalgorithm has stuffed up somehow,"
			" since particle to be moved %d is <= slot to insert into %d.\n",
			__func__, candidateParticleToMove, indexToInsertAt );
		Stream_Indent( self->debug );	

		Journal_DPrintfL( self->debug, 3, "Copying particle data from %d to %d\n",
			candidateParticleToMove, indexToInsertAt );
		Swarm_CopyParticleWithinSwarm( self->swarm, indexToInsertAt, candidateParticleToMove );

		/* update the cell that the moved particle lives in to have the correct index into the
		 * particle array for it. */
		oldPtrToMovedParticle = Swarm_ParticleAt( self->swarm, candidateParticleToMove );
		owningCell = oldPtrToMovedParticle->owningCell;
		cParticle_I = Swarm_GetParticleIndexWithinCell( self->swarm, owningCell, candidateParticleToMove );
		Journal_DPrintfL( self->debug, 3, "Updating owning cell: (Cell %d, PIC index %d) now -> p.i. %d\n",
			owningCell, cParticle_I, indexToInsertAt );
		self->swarm->cellParticleTbl[owningCell][cParticle_I] = indexToInsertAt;


		Stream_UnIndent( self->debug );	

		/* update the counters/indices */
		currLeavingParticleArrayEntry++;
		leavingParticlesUnfilledCount--;
		self->swarm->particleLocalCount--;

		Stream_UnIndent( self->debug );
	}
	Stream_UnIndentBranch( Swarm_Debug );

	/* we only need to free the array of leaving particle slots if its a new merged list */
	if ( mergedArrayCreated == True ) {
		Memory_Free( leavingParticlesArray );
	}

	/* ------------------------- */
	Journal_DPrintfL( self->debug, 2, "Local particle count reduced from %d to %d\n", prevParticleCount,
		self->swarm->particleLocalCount );

	/* Update the memory allocated to the particles array if particle count has reduced significantly */
	while ( self->swarm->particlesArraySize > self->swarm->particleLocalCount + self->swarm->particlesArrayDelta ) {
		self->swarm->particlesArraySize -= self->swarm->particlesArrayDelta;
	}
	if ( self->swarm->particlesArraySize < prevParticlesArraySize ) {
		Journal_DPrintfL( self->debug, 2, "Reducing particles array entries from %d to %d\n",
			prevParticlesArraySize, self->swarm->particlesArraySize );
		self->swarm->particles = Memory_Realloc_Array_Bytes(
			self->swarm->particles,
			self->swarm->particleExtensionMgr->finalSize,
			self->swarm->particlesArraySize );
	}	

	Stream_UnIndentBranch( Swarm_Debug );
}


Particle_Index* _ParticleCommHandler_MergeListsOfUnfilledParticleSlots( ParticleCommHandler* self )
{
	Particle_Index*		mergedLeavingParticleArray = NULL;
	Particle_Index		slotsToFillTotalCount = 0;
	Index			currMergedLeavingParticleEntry = 0;
	Index			lowestUnmergedLeavingViaShadow = self->currShadowParticleLeavingMeIndex;
	Index			lowestUnmergedLeavingDomain = self->currParticleLeavingMyDomainIndex;
	Particle_Index		indexOfLowestUnmergedLeavingDomain = 0;
	Index*			lowestUnmergedLeavingEntryToUpdatePtr = NULL;
	Particle_Index		candidateMergeParticle = 0;

	Journal_DPrintfL( self->debug, 1, "In %s():\n", __func__ );
	Stream_Indent( self->debug );

	slotsToFillTotalCount = self->particlesOutsideDomainUnfilledCount + self->shadowParticlesLeavingMeUnfilledCount;
	mergedLeavingParticleArray = Memory_Alloc_Array( Particle_Index, slotsToFillTotalCount, "mergedLeavingParticlesArray" );

	while ( currMergedLeavingParticleEntry < slotsToFillTotalCount ) {
		/* Need to initialise this to the max particle count every loop, in case the first condition is false,
		so the 2nd will always hit it. */
		candidateMergeParticle = self->swarm->particleLocalCount;
		 
		if ( lowestUnmergedLeavingViaShadow < self->shadowParticlesLeavingMeTotalCount ) {
			candidateMergeParticle = self->shadowParticlesLeavingMeIndices[lowestUnmergedLeavingViaShadow];
			lowestUnmergedLeavingEntryToUpdatePtr = &lowestUnmergedLeavingViaShadow;
		}
		if ( lowestUnmergedLeavingDomain < self->particlesOutsideDomainTotalCount ) {
			indexOfLowestUnmergedLeavingDomain = self->particlesOutsideDomainIndices[lowestUnmergedLeavingDomain];

			if ( indexOfLowestUnmergedLeavingDomain < candidateMergeParticle ) { 
				candidateMergeParticle = indexOfLowestUnmergedLeavingDomain;
				lowestUnmergedLeavingEntryToUpdatePtr = &lowestUnmergedLeavingDomain;
			}	
		}
		
		mergedLeavingParticleArray[currMergedLeavingParticleEntry++] = candidateMergeParticle;
		(*lowestUnmergedLeavingEntryToUpdatePtr)++;

		#if DEBUG
		Journal_Firewall( lowestUnmergedLeavingViaShadow <= self->shadowParticlesLeavingMeTotalCount,
			Swarm_Error, "Error: merging of unfilled particle lists stuffed up.\n" );
		Journal_Firewall( lowestUnmergedLeavingDomain <= self->particlesOutsideDomainTotalCount,
			Swarm_Error, "Error: merging of unfilled particle lists stuffed up.\n" );
		#endif
	}

	#if DEBUG
	if ( Stream_IsPrintableLevel( self->debug, 2 ) ) {
		Journal_DPrintf( self->debug, "Merged list of particles leaving proc:\n\t{" );
		for ( currMergedLeavingParticleEntry=0; currMergedLeavingParticleEntry < slotsToFillTotalCount;
			currMergedLeavingParticleEntry++ ) 
		{
			Journal_DPrintf( self->debug, "%d, ",
				mergedLeavingParticleArray[currMergedLeavingParticleEntry] );
		}
		Journal_DPrintf( self->debug, "}\n" );
	}
	#endif
	
	Stream_UnIndent( self->debug );

	return mergedLeavingParticleArray;
}


void _ParticleCommHandler_PrintParticleSlotsYetToFill( ParticleCommHandler* self ) {	
	Index leavingParticleEntry;

	Journal_DPrintf( self->debug, "%d slots yet to fill from particles leaving via shadow cells:\n",
		self->shadowParticlesLeavingMeUnfilledCount );
	leavingParticleEntry = self->currShadowParticleLeavingMeIndex;
	Journal_DPrintf( self->debug, "\t{ " );
	for ( ; leavingParticleEntry < self->shadowParticlesLeavingMeTotalCount; leavingParticleEntry++ ) {
		Journal_DPrintf( self->debug, "%d, ", 
			self->shadowParticlesLeavingMeIndices[leavingParticleEntry] );
	}
	Journal_DPrintf( self->debug, "}\n" );
		
	Journal_DPrintf( self->debug, "%d slots yet to fill from particles leaving domain directly:\n",
		self->particlesOutsideDomainUnfilledCount );
	leavingParticleEntry = self->currParticleLeavingMyDomainIndex;
	Journal_DPrintf( self->debug, "\t{ " );
	for ( ; leavingParticleEntry < self->particlesOutsideDomainTotalCount; leavingParticleEntry++ ) {
		Journal_DPrintf( self->debug, "%d, ", 
			self->particlesOutsideDomainIndices[leavingParticleEntry] );
	}
	Journal_DPrintf( self->debug, "}\n" );
}



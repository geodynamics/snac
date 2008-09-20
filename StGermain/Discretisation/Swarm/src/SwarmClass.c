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
** $Id: SwarmClass.c 4044 2007-03-21 23:35:21Z PatrickSunter $
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
#include "SwarmClass.h"

#include "StandardParticle.h"
#include "CellLayout.h"
#include "ElementCellLayout.h"
#include "ParticleLayout.h"
#include "GlobalParticleLayout.h"
#include "SingleCellLayout.h"
#include "FileParticleLayout.h"
#include "ParticleCommHandler.h"
#include "IntegrationPoint.h"
#include "SwarmVariable_Register.h"
#include "SwarmVariable.h"
#include "Swarm_Register.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


const Type Swarm_Type = "Swarm";
const Name defaultSwarmParticleCommHandlerName = "defaultSwarmPHandlerName";

/** Default extra factor to malloc for particles array. This is because the number of particles on each process
may increase due to advection, splitting/merging etc, and we want to avoid reallocing if possible. */
static const double       DEFAULT_EXTRA_PARTICLES_FACTOR = 0.05;
static const unsigned int MINIMUM_PARTICLES_ARRAY_DELTA = 100;
static const unsigned int DEFAULT_CELL_PARTICLE_TBL_DELTA = 4;

/* --- Function Definitions --- */

Swarm* Swarm_DefaultNew( Name name ) {
	return (Swarm*)_Swarm_DefaultNew( name );
}

Swarm* Swarm_New( 
		Name                                  name,
		void*                                 cellLayout,
		void*                                 particleLayout,
		Dimension_Index                       dim,
		SizeT                                 particleSize,
		ExtensionManager_Register*            extensionMgr_Register,
		Variable_Register*                    variable_Register,
		MPI_Comm                              comm ) 
{
	return _Swarm_New( 
			sizeof(Swarm),
			Swarm_Type,
			_Swarm_Delete,
			_Swarm_Print,
			_Swarm_Copy,
			_Swarm_DefaultNew,
			_Swarm_Construct,
			_Swarm_Build,
			_Swarm_Initialise,
			_Swarm_Execute,
			_Swarm_Destroy,
			name, 
			True, 
			(CellLayout*)cellLayout,
			(ParticleLayout*)particleLayout,
			dim,
			particleSize,
			DEFAULT_CELL_PARTICLE_TBL_DELTA,
			DEFAULT_EXTRA_PARTICLES_FACTOR,
			extensionMgr_Register, 
			variable_Register,
			comm );
}

Swarm* _Swarm_New(
		SizeT                                 _sizeOfSelf,
		Type                                  type,
		Stg_Class_DeleteFunction*             _delete,
		Stg_Class_PrintFunction*              _print,
		Stg_Class_CopyFunction*               _copy, 
		Stg_Component_DefaultConstructorFunction* _defaultConstructor,
		Stg_Component_ConstructFunction*      _construct,
		Stg_Component_BuildFunction*          _build,
		Stg_Component_InitialiseFunction*     _initialise,
		Stg_Component_ExecuteFunction*        _execute,
		Stg_Component_DestroyFunction*        _destroy,
		Name                                  name,
		Bool                                  initFlag,
		CellLayout*                           cellLayout,
		ParticleLayout*                       particleLayout,
		Dimension_Index                       dim,
		SizeT                                 particleSize,
		Particle_InCellIndex                  cellParticleTblDelta, 
		double                                extraParticlesFactor,
		ExtensionManager_Register*            extensionMgr_Register,
		Variable_Register*                    variable_Register,
		MPI_Comm                              comm )
{
	Swarm* self;
	
	/* Allocate memory */
	self = (Swarm*)_Stg_Component_New( 
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
			NON_GLOBAL );

	self->particleSize = particleSize;
	
	/* Swarm info */
	if( initFlag )
		_Swarm_Init( 
				self,
				cellLayout,
				particleLayout,
				dim,
				cellParticleTblDelta,
				extraParticlesFactor,
				extensionMgr_Register,
				variable_Register,
				comm );
	
	return self;
}

void _Swarm_Init( 
		Swarm*                                self, 
		void*                                 cellLayout,
		void*                                 particleLayout,
		Dimension_Index                       dim,
		Particle_InCellIndex                  cellParticleTblDelta, 
		double                                extraParticlesFactor,
		ExtensionManager_Register*            extensionMgr_Register,
		Variable_Register*                    variable_Register,
		MPI_Comm                              comm )
{
	StandardParticle   particle;
	Stream*            errorStream = Journal_Register( Error_Type, self->type );
	
	self->isConstructed = True;
	self->debug = Stream_RegisterChild( Swarm_Debug, self->type );
	self->dim   = dim;
	self->comm = comm;
	MPI_Comm_rank( comm, (int *)&self->myRank );
	MPI_Comm_size( comm, (int *)&self->nProc );
	self->stillDoingInitialisation = True;
	/* Check point and reload by default - only things like integration swarms will turn this off */
	self->isSwarmTypeToCheckPointAndReload = True;

	self->cellLayout = (CellLayout*)cellLayout;
	self->particleLayout = (ParticleLayout*)particleLayout;
	
	/* Check that if either the CellLayout or ParticleLayout is inappropriate to be checkpointed,
		we set tthe appropriate flags -- PatrickSunter, 22 August 2006 */
	if ( Stg_Class_IsInstance( self->cellLayout, SingleCellLayout_Type ) ) {
		self->isSwarmTypeToCheckPointAndReload = False;
	}
	
	self->cellLocalCount = 0;
	self->cellDomainCount = 0;
	self->cellShadowCount = 0;
	self->cellPointTbl = NULL;
	self->cellPointCountTbl = NULL;
	
	self->cellParticleTbl = NULL;
	self->cellParticleCountTbl = NULL;
	self->cellParticleSizeTbl = NULL;
	self->cellParticleTblDelta = cellParticleTblDelta;
	
	self->particles = NULL;
	self->particleLocalCount = 0;
	self->particlesArraySize = 0;
	self->particlesArrayDelta = 0;
	self->extraParticlesFactor = extraParticlesFactor;
	Journal_Firewall( extraParticlesFactor > 0.0, errorStream, "Error - in %s: extraParticlesFactor "
		"given as %.3f, but this must be greater than zero to allow swarm to be realloc'ed larger "
		"if necessary\n", extraParticlesFactor );

	if ( variable_Register ) {
		self->swarmVariable_Register = SwarmVariable_Register_New( variable_Register );
	}
	else {
		self->swarmVariable_Register = NULL;
	}

	self->particleExtensionMgr = ExtensionManager_New_OfStruct( "particle", self->particleSize );
	ExtensionManager_Register_Add( extensionMgr_Register, self->particleExtensionMgr );

	self->particleCommunicationHandler = ParticleCommHandler_New( defaultSwarmParticleCommHandlerName, self );

	self->owningCellVariable = Swarm_NewScalarVariable(
			self,
			"OwningCell",
			GetOffsetOfMember( particle , owningCell ),
			Variable_DataType_Int ); /* Should be unsigned int */

	self->swarmReg_I = Swarm_Register_Add( Swarm_Register_GetSwarm_Register(), self );

}


void* _Swarm_ParticleInCellAt( void* swarm, Cell_Index cell_I, Particle_InCellIndex cParticle_I ) {
	Swarm* self =	(Swarm*)swarm;
	
	return (void*)Swarm_ParticleInCellAt( self, cell_I, cParticle_I );
}


void* _Swarm_ParticleAt( void* swarm, Particle_Index dParticle_I ) {
	Swarm* self = (Swarm*)swarm;
	
	return (void*)Swarm_ParticleAt( self, dParticle_I );
}


void _Swarm_Delete( void* swarm ) {
	Swarm*			self = (Swarm*)swarm;
	Cell_LocalIndex		cell_I;
	
	Stg_Class_Delete( self->particleCommunicationHandler );
	
	Memory_Free( self->cellPointTbl );
	Memory_Free( self->cellPointCountTbl );
	
	for( cell_I = 0; cell_I < self->cellLocalCount; cell_I++ ) {
		Memory_Free( self->cellParticleTbl[cell_I] );
	}
	Memory_Free( self->cellParticleTbl );
	Memory_Free( self->cellParticleCountTbl );
	Memory_Free( self->cellParticleSizeTbl );
	if ( self->particles ) {
		ExtensionManager_Free( self->particleExtensionMgr, self->particles );
	}

	/* Delete SwarmVariable_Register if it has been created */
	if ( self->swarmVariable_Register ) {
		Stg_Class_Delete( self->swarmVariable_Register );
	}
	
	/* Stg_Class_Delete parent class */
	_Stg_Component_Delete( self );
}


void _Swarm_Print( void* swarm, Stream* stream ) {
	Swarm* self = (Swarm*)swarm;
	
	Cell_Index cell_I;
	
	/* Set the Journal for printing informations */
	Stream* swarmStream = stream;
	
	/* General info */
	Journal_Printf( swarmStream, "Swarm (ptr): %p\n", self );
	
	/* Parent class info */
	_Stg_Component_Print( self, stream );
	
	/* Virtual info */

	/* Swarm info */
	Print( self->cellLayout, stream );
	Print( self->particleLayout, stream );
	Journal_Printf( swarmStream, "\tcellLocalCount: %u\n", self->cellLocalCount );
	Journal_Printf( swarmStream, "\tcellDomainCount: %u\n", self->cellDomainCount );
	Journal_Printf( swarmStream, "\tcellShadowCount: %u\n", self->cellShadowCount );
	Journal_Printf( swarmStream, "\tcellPointCountTbl (ptr): %p\n", self->cellPointCountTbl );
	Journal_Printf( swarmStream, "\tcellPointCountTbl[0-%u]: ", self->cellDomainCount );
	for( cell_I = 0; cell_I < self->cellDomainCount; cell_I++ ) {
		Journal_Printf( swarmStream, "%.3u ", self->cellPointCountTbl[cell_I] );
	}
	Journal_Printf( swarmStream, "\n" );
	
	Journal_Printf( swarmStream, "\tcellPointTbl (ptr): %p\n", self->cellPointTbl );
	Journal_Printf( swarmStream, "\tcellPointTbl[0-%u]: {\n", self->cellDomainCount );
	for( cell_I = 0; cell_I < self->cellDomainCount; cell_I++ ) {
		Cell_PointIndex point_I;

		Journal_Printf( swarmStream, "\tcellPointTbl[%u][0-%u]: ", cell_I, self->cellPointCountTbl[cell_I] );
		for( point_I = 0; point_I < self->cellPointCountTbl[cell_I]; point_I++ ) {
			Journal_Printf( swarmStream, "{%.3g %.3g %.3g} ",
				(*self->cellPointTbl[cell_I][point_I])[0], 
				(*self->cellPointTbl[cell_I][point_I])[1], 
				(*self->cellPointTbl[cell_I][point_I])[2] );
		}
		Journal_Printf( swarmStream, "\n" );
	}
	Journal_Printf( swarmStream, "}\n" );
	Journal_Printf( swarmStream, "\n" );
	
	Journal_Printf( swarmStream, "\tcellParticleCountTbl (ptr): %p\n", self->cellParticleCountTbl );
	Journal_Printf( swarmStream, "\tcellParticleCountTbl[0-%u]: ", self->cellDomainCount );
	for( cell_I = 0; cell_I < self->cellDomainCount; cell_I++ ) {
		Journal_Printf( swarmStream, "%.3u ", self->cellParticleCountTbl[cell_I] );
	}
	Journal_Printf( swarmStream, "\n" );
	Journal_Printf( swarmStream, "\tcellParticleSizeTbl (ptr): %p\n", self->cellParticleSizeTbl );
	Journal_Printf( swarmStream, "\tcellParticleSizeTbl[0-%u]: ", self->cellDomainCount );
	for( cell_I = 0; cell_I < self->cellDomainCount; cell_I++ ) {
		Journal_Printf( swarmStream, "%.3u ", self->cellParticleSizeTbl[cell_I] );
	}
	Journal_Printf( swarmStream, "\n" );

	/* Print( self->particleExtensionMgr, stream ); */
	Journal_Printf( swarmStream, "\tcellParticleTbl (ptr): %p\n", self->cellParticleTbl );
	Journal_Printf( swarmStream, "\tcellParticleTbl [0-%u]: {\n", self->cellDomainCount );
	for( cell_I = 0; cell_I < self->cellDomainCount; cell_I++ ) {
		Particle_InCellIndex cParticle_I;
		Particle_Index dParticle_I;

		Journal_Printf( swarmStream, "\t\tcellParticleTbl[%u][0-%u]:\n", cell_I, self->cellParticleCountTbl[cell_I] );
		for( cParticle_I = 0; cParticle_I < self->cellParticleCountTbl[cell_I]; cParticle_I++ ) {
			dParticle_I = self->cellParticleTbl[cell_I][cParticle_I];
			Journal_Printf( swarmStream, "\t\t\t(part. index) %d\n", dParticle_I );
			// TODO: should probably handle this by having a particle print E.P.
			// which the swarm holds, which can be extended to print the 
			// particles correctly.
			// Check how the mesh handles this for elements and nodes...
			#if 0
			Journal_Printf( swarmStream, "\t\t\t(ptr) %p\n", Swarm_ParticleAt( self, dParticle_I ) );
			Journal_Printf( swarmStream, "\t\t\t%g %g %g\n", 
				(*(Coord*)Swarm_ParticleAt( self, cell_I, particle_I ))[0], 
				(*(Coord*)Swarm_ParticleAt( self, cell_I, particle_I ))[1], 
				(*(Coord*)Swarm_ParticleAt( self, cell_I, particle_I ))[2] );
			#endif	
		}
		Journal_Printf( swarmStream, "\n" );
	}
	Journal_Printf( swarmStream, "\t}\n" );
	Journal_Printf( swarmStream, "\tparticlesArraySize: %d\n", self->particlesArraySize );
	Journal_Printf( swarmStream, "\tparticlesArrayDelta: %d\n", self->particlesArrayDelta );
	Journal_Printf( swarmStream, "\textraParticlesFactor: %.3g\n", self->extraParticlesFactor );
}


void* _Swarm_Copy( void* swarm, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	Swarm*		self = (Swarm*)swarm;
	Swarm*		newSwarm;
	PtrMap*		map = ptrMap;
	Bool		ownMap = False;
	
	if( !map ) {
		map = PtrMap_New( 10 );
		ownMap = True;
	}
	
	newSwarm = _Stg_Component_Copy( self, dest, deep, nameExt, map );
	PtrMap_Append( map, self, newSwarm );
	
	newSwarm->myRank = self->myRank;
	newSwarm->nProc = self->nProc;
	newSwarm->comm = self->comm;
	newSwarm->cellLocalCount = self->cellLocalCount;
	newSwarm->cellDomainCount = self->cellDomainCount;
	newSwarm->cellShadowCount = self->cellShadowCount;
	newSwarm->cellParticleTblDelta = self->cellParticleTblDelta;
	newSwarm->particleLocalCount = self->particleLocalCount;
	newSwarm->particlesArraySize = self->particlesArraySize;
	newSwarm->particlesArrayDelta = self->particlesArrayDelta;
	newSwarm->extraParticlesFactor = self->extraParticlesFactor;
	
	if( deep ) {
		/* Classes */
		newSwarm->cellLayout = (CellLayout*)Stg_Class_Copy( self->cellLayout, NULL, deep, nameExt, map );
		newSwarm->particleLayout = (ParticleLayout*)Stg_Class_Copy( self->particleLayout, NULL, deep, nameExt, map );
		newSwarm->debug = (Stream*)Stg_Class_Copy( self->debug, NULL, deep, nameExt, map );
		newSwarm->particleExtensionMgr = (ExtensionManager*)Stg_Class_Copy( self->particleExtensionMgr, NULL, deep, nameExt, map 
);
		newSwarm->particleCommunicationHandler = (ParticleCommHandler*)Stg_Class_Copy( self->particleCommunicationHandler, NULL, deep, nameExt, map );
		
		/* Arrays */
		if( (newSwarm->cellPointCountTbl = PtrMap_Find( map, self->cellPointCountTbl )) == NULL ) {
			if( self->cellPointCountTbl ) {
				newSwarm->cellPointCountTbl = Memory_Alloc_Array( Cell_PointIndex, newSwarm->cellDomainCount, "Swarm->cellPointCountTbl" );
				memcpy( newSwarm->cellPointCountTbl, self->cellPointCountTbl, newSwarm->cellDomainCount * sizeof( Cell_PointIndex ) );
				PtrMap_Append( map, self->cellPointCountTbl, newSwarm->cellPointCountTbl );
			}
			else {
				newSwarm->cellPointCountTbl = NULL;
			}
		}
		
		if( (newSwarm->cellPointTbl = PtrMap_Find( map, self->cellPointTbl )) == NULL ) {
			if( newSwarm->cellPointCountTbl && self->cellPointTbl ) {
				Index	cell_I;
				
				newSwarm->cellPointTbl = Memory_Alloc_2DComplex( Cell_Point, newSwarm->cellDomainCount, newSwarm->cellPointCountTbl, "Swarm->cellPointTbl" );
				for( cell_I = 0; cell_I < newSwarm->cellDomainCount; cell_I++ ) {
					memcpy( newSwarm->cellPointTbl[cell_I], self->cellPointTbl[cell_I], newSwarm->cellPointCountTbl[cell_I] * sizeof(Cell_Point) );
				}
				PtrMap_Append( map, self->cellPointTbl, newSwarm->cellPointTbl );
			}
			else {
				newSwarm->cellPointTbl = NULL;
			}
		}
		
		if( (newSwarm->cellParticleCountTbl = PtrMap_Find( map, self->cellParticleCountTbl )) == NULL ) {
			if( self->cellParticleCountTbl ) {
				newSwarm->cellParticleCountTbl = Memory_Alloc_Array( Particle_InCellIndex, newSwarm->cellDomainCount, "Swarm->cellParticleCountTbl" );
				memcpy( newSwarm->cellParticleCountTbl, self->cellParticleCountTbl, newSwarm->cellDomainCount * sizeof( Particle_InCellIndex ) );
				PtrMap_Append( map, self->cellParticleCountTbl, newSwarm->cellParticleCountTbl );
			}
			else {
				newSwarm->cellParticleCountTbl = NULL;
			}
		}
		
		if( (newSwarm->cellParticleSizeTbl = PtrMap_Find( map, self->cellParticleSizeTbl )) == NULL ) {
			if( self->cellParticleSizeTbl ) {
				newSwarm->cellParticleSizeTbl = Memory_Alloc_Array( Particle_InCellIndex, newSwarm->cellDomainCount, "Swarm->cellParticleSizeTbl" );
				memcpy( newSwarm->cellParticleSizeTbl, self->cellParticleSizeTbl, newSwarm->cellDomainCount * sizeof( Particle_InCellIndex ) );
				PtrMap_Append( map, self->cellParticleSizeTbl, newSwarm->cellParticleSizeTbl );
			}
			else {
				newSwarm->cellParticleSizeTbl = NULL;
			}
		}
		
		if( (newSwarm->cellParticleTbl = PtrMap_Find( map, self->cellParticleTbl )) == NULL ) {
			if( newSwarm->cellParticleCountTbl && self->cellParticleTbl ) {
				Index	cell_I;
				
				newSwarm->cellParticleTbl = Memory_Alloc_Array( Cell_Particles, newSwarm->cellDomainCount, "Swarm->cellParticleTbl" );
				for( cell_I = 0; cell_I < newSwarm->cellDomainCount; cell_I++ ) {
					if( newSwarm->cellParticleCountTbl[cell_I] ) {
						newSwarm->cellParticleTbl[cell_I] = Memory_Alloc_Array( Particle_Index, newSwarm->cellParticleCountTbl[cell_I], "Swarm->cellParticleTbl[]" );
						memcpy( newSwarm->cellParticleTbl[cell_I], self->cellParticleTbl[cell_I], newSwarm->cellParticleCountTbl[cell_I] * sizeof(Particle_Index) );
					}
					else {
						newSwarm->cellParticleTbl[cell_I] = NULL;
					}
				}
				PtrMap_Append( map, self->cellParticleTbl, newSwarm->cellParticleTbl );
			}
			else {
				newSwarm->cellParticleTbl = NULL;
			}
		}
		
		if( (newSwarm->particles = PtrMap_Find( map, self->particles )) == NULL ) {
			if( self->particles ) {
				newSwarm->particles = (Particle_List)ExtensionManager_Malloc( newSwarm->particleExtensionMgr, newSwarm->particlesArraySize );
				memcpy( newSwarm->particles, self->particles, newSwarm->particlesArraySize * ExtensionManager_GetFinalSize( newSwarm->particleExtensionMgr ) );
				PtrMap_Append( map, self->particles, newSwarm->particles );
			}
			else {
				newSwarm->particles = NULL;
			}
		}
	}
	else {
		newSwarm->cellLayout = self->cellLayout;
		newSwarm->particleLayout = self->particleLayout;
		newSwarm->debug = self->debug;
		newSwarm->cellPointTbl = self->cellPointTbl;
		newSwarm->cellPointCountTbl = self->cellPointCountTbl;
		newSwarm->cellParticleTbl = self->cellParticleTbl;
		newSwarm->cellParticleCountTbl = self->cellParticleCountTbl;
		newSwarm->cellParticleSizeTbl = self->cellParticleSizeTbl;
		newSwarm->particles = self->particles;
		newSwarm->particleExtensionMgr = self->particleExtensionMgr;
		newSwarm->particleCommunicationHandler = self->particleCommunicationHandler;
	}
	
	if( ownMap ) {
		Stg_Class_Delete( map );
	}
	
	return (void*)newSwarm;
}

void* _Swarm_DefaultNew( Name name ) {
	return _Swarm_New( 
			sizeof(Swarm),
			Swarm_Type,
			_Swarm_Delete,
			_Swarm_Print,
			_Swarm_Copy,
			_Swarm_DefaultNew,
			_Swarm_Construct,
			_Swarm_Build, 
			_Swarm_Initialise,
			_Swarm_Execute,
			_Swarm_Destroy,
			name, 
			False, 
			NULL,                       /* cellLayout */
			NULL,                       /* particleLayout */
			0,                          /* dim */
			sizeof(IntegrationPoint),   /* particleSize */
			0,	                    /* cellParticleTblDelta */
			0,                          /* extraParticlesFactor */
			NULL,                       /* extensionMgr_Register */
			NULL,                       /* variable_Register */
			MPI_COMM_WORLD );
}

void _Swarm_Construct( void* swarm, Stg_ComponentFactory* cf, void* data ) {
	Swarm*                  self                     = (Swarm*)swarm;
	CellLayout*             cellLayout               = NULL;
	ParticleLayout*         particleLayout           = NULL;
	void*                   extensionManagerRegister = NULL;
	double                  extraParticlesFactor     = 0.0;
	Particle_InCellIndex    cellParticleTblDelta     = 0;
	Dimension_Index         dim;
	Type                    particleType;
	Variable_Register*      variable_Register        = NULL;
	AbstractContext*        context = NULL;

	context = Stg_ComponentFactory_ConstructByName( cf, "context", AbstractContext, True, data );

	dim = Stg_ComponentFactory_GetRootDictUnsignedInt( cf, "dim", 0 );
	
	particleType = Stg_ComponentFactory_GetString( cf, self->name, "ParticleType", IntegrationPoint_Type );
	
	cellLayout =  Stg_ComponentFactory_ConstructByKey(  cf,  self->name,  CellLayout_Type, CellLayout,  True, data ) ;
	particleLayout =  Stg_ComponentFactory_ConstructByKey(  cf,  self->name,  ParticleLayout_Type, ParticleLayout, True, data );
	
	extensionManagerRegister = Stg_ObjectList_Get( cf->registerRegister, "ExtensionManager_Register" );
	assert( extensionManagerRegister );
	variable_Register = Stg_ObjectList_Get( cf->registerRegister, "Variable_Register" );
	assert( variable_Register );
	
	cellParticleTblDelta = 
		Stg_ComponentFactory_GetUnsignedInt( cf, self->name, "cellParticleTblDelta", DEFAULT_CELL_PARTICLE_TBL_DELTA);
	extraParticlesFactor = 
		Stg_ComponentFactory_GetDouble( cf, self->name, "extraParticlesFactor", DEFAULT_EXTRA_PARTICLES_FACTOR );
	
	_Swarm_Init( 
			self,
			cellLayout,
			particleLayout, 
			dim,
			cellParticleTblDelta,
			extraParticlesFactor,
			extensionManagerRegister,
			variable_Register,
			context->communicator );
}


void _Swarm_Build( void* swarm, void* data ) {
	Swarm* self = (Swarm*)swarm;
	AbstractContext* context = (AbstractContext*)data;
	
	Journal_DPrintf( self->debug, "In %s(): for swarm \"%s\" (of type %s)\n", __func__, self->name, self->type ); 
	Stream_IndentBranch( Swarm_Debug );
	
	Journal_DPrintf( self->debug, "allocating memory for cell->particle mappings:\n" );
	_Swarm_BuildCells( self, data );
	Journal_DPrintf( self->debug, "...done.\n" );

	/* if loading from checkpoint, then delete the particle layout
	 * created due to the user's specification in the input file,
	 * and replace it with a FileParticleLayout, so that the particles
	 * can be re-loaded with their exact state at the checkpointed time
	 */
	if ( context && context->loadFromCheckPoint ) {
		Journal_DPrintf( self->debug, "detected loadFromCheckPoint mode enabled:\n" );
		Stream_Indent( self->debug );

		if ( False == self->isSwarmTypeToCheckPointAndReload ) {
			Journal_DPrintf( self->debug, "...but this swarm type is set to not be checkpointed/reloaded, "
				"so continuing.\n" );
		}
		else {
			Swarm_ReplaceCurrentParticleLayoutWithFileParticleLayout( self, context );
		}	
		Stream_UnIndent( self->debug );
	}
	
	Journal_DPrintf( self->debug, "allocating memory for particles:\n" );
	_Swarm_BuildParticles( self, data );
	Journal_DPrintf( self->debug, "...done.\n" );

	Journal_DPrintf( self->debug, "setting up the particle owningCell SwarmVariable:\n" );
	Stg_Component_Build( self->owningCellVariable, data, False );
	Journal_DPrintf( self->debug, "...done.\n" );
	Stream_UnIndentBranch( Swarm_Debug );
	Journal_DPrintf( self->debug, "...done in %s().\n", __func__ );
}

			
void _Swarm_Initialise( void* swarm, void* data ) {
	Swarm* self = (Swarm*)swarm;
	
	Journal_DPrintf( self->debug, "In %s(): for swarm \"%s\" (of type %s)\n", __func__, self->name, self->type ); 
	Stream_IndentBranch( Swarm_Debug );
	
	_Swarm_InitialiseCells( self, data );
	_Swarm_InitialiseParticles( self, data );

	Stg_Component_Initialise( self->owningCellVariable, data, False );
	self->stillDoingInitialisation = False;

	Stream_UnIndentBranch( Swarm_Debug );
	Journal_DPrintf( self->debug, "...done in %s().\n", __func__ );
}


void _Swarm_Execute( void* swarm, void* data ) {
}

void _Swarm_Destroy( void* swarm, void* data ) {
}

void _Swarm_BuildCells( void* swarm, void* data ) {
	Swarm*			self = (Swarm*)swarm;
	Cell_Index		cell_I;
	Cell_PointIndex		pointCount;
	
	/* Need to do this first - as the cellLayout may be dependent on a mesh etc */
	Stg_Component_Build( self->cellLayout, data, False );

	Journal_DPrintf( self->debug, "In %s():\n", __func__ ); 
	Stream_IndentBranch( Swarm_Debug );

	self->cellLocalCount = CellLayout_CellLocalCount( self->cellLayout );
	self->cellShadowCount = CellLayout_CellShadowCount( self->cellLayout );
	self->cellDomainCount = self->cellLocalCount + self->cellShadowCount;

	Journal_DPrintf( self->debug, "CellLayout \"%s\" (of type %s) returned cell counts of:\n"
		"cellLocalCount: %u, cellShadowCount: %u, cellDomainCount: %u",
		self->cellLayout->name, self->cellLayout->type,
		self->cellLocalCount, self->cellShadowCount, self->cellDomainCount );

	self->cellPointCountTbl = Memory_Alloc_Array( Cell_PointIndex, self->cellDomainCount, "Swarm->cellPointCountTbl" );
	pointCount = 0;

	for( cell_I = 0; cell_I < self->cellDomainCount; cell_I++ ) {
		self->cellPointCountTbl[cell_I] = CellLayout_PointCount( self->cellLayout, cell_I );
		pointCount += self->cellPointCountTbl[cell_I];
	}

	self->cellPointTbl = Memory_Alloc_2DComplex( Cell_Point, self->cellDomainCount, self->cellPointCountTbl, "Swarm->cellPointTbl" );
	Stream_UnIndentBranch( Swarm_Debug );
}


void _Swarm_BuildParticles( void* swarm, void* data ) {
	Swarm*			self = (Swarm*)swarm;
	
	/* Need to do this first - as the particleLayout may be dependent on a mesh etc */
	Stg_Component_Build( self->particleLayout, data, False );

	self->cellParticleCountTbl = Memory_Alloc_Array( Particle_InCellIndex, self->cellDomainCount,
		"Swarm->cellParticleCountTbl" );
	self->cellParticleSizeTbl = Memory_Alloc_Array( Particle_InCellIndex, self->cellDomainCount,
		"Swarm->cellParticleSizeTbl" );
	self->cellParticleTbl = Memory_Alloc_Array( Cell_Particles, self->cellDomainCount, "Swarm->cellParticleTbl" ); 
	
	ParticleLayout_SetInitialCounts( self->particleLayout, self );
	
	/* Now allocate the particles array */
	/* We allocate extra space to try & avoid avoid reallocing later as particles are advected etc. */
	self->particlesArrayDelta = (Particle_Index)( (double) self->particleLocalCount * self->extraParticlesFactor);
	if ( 0 == self->particlesArrayDelta ) {
		self->particlesArrayDelta = MINIMUM_PARTICLES_ARRAY_DELTA;		
	}

	self->particlesArraySize = self->particleLocalCount + self->particlesArrayDelta;

	self->particles = (Particle_List)ExtensionManager_Malloc( self->particleExtensionMgr, self->particlesArraySize );
}


void _Swarm_InitialiseCells( void* swarm, void* data ) {
	Swarm*			self = (Swarm*)swarm;
	Cell_Index		cell_I;
	
	/* Need to do this first - as the cellLayout may be dependent on a mesh etc */
	Stg_Component_Initialise( self->cellLayout, data, False );

	for( cell_I = 0; cell_I < self->cellDomainCount; cell_I++ ) {
		CellLayout_InitialiseCellPoints( self->cellLayout, cell_I, self->cellPointCountTbl[cell_I], 
			self->cellPointTbl[cell_I] );
	}
}


void _Swarm_InitialiseParticles( void* swarm, void* data ) {
	Swarm*			self = (Swarm*)swarm;
	
	/* Need to do this first - as the particleLayout may be dependent on a mesh etc */
	Stg_Component_Initialise( self->particleLayout, data, False );

	ParticleLayout_InitialiseParticles( self->particleLayout, self );

	/* Need to re-do the delta calculation here, since if using a global particle layouts, we didn't know the 
	 * localParticleCount until the above call, and had just used a default delta. We can now calculate the
	 * correct one. */
	self->particlesArrayDelta = (Particle_Index)( (double) self->particleLocalCount * self->extraParticlesFactor);
	if ( 0 == self->particlesArrayDelta ) {
		self->particlesArrayDelta = MINIMUM_PARTICLES_ARRAY_DELTA;		
	}

	Swarm_UpdateAllParticleOwners( self );
}


/** This function assmes particle advection and hence global coords are used */
void Swarm_UpdateAllParticleOwners( void* swarm ) {
	Swarm*			self = (Swarm*)swarm;
	Particle_Index		lParticle_I;

	/* TODO: need to reconsideer - gauss particle layout should be allowed, but not swarms that have no local
	 * co-ordinates */
	if ( self->particleLayout->coordSystem == LocalCoordSystem ) {
		/* Assumption: Local coord layouts don't go through advection so no need to update */
		return;
	}

	Journal_DPrintfL( self->debug, 1, "In %s() for Swarm \"%s\"\n", __func__, self->name );
	Stream_IndentBranch( Swarm_Debug );	
	for ( lParticle_I=0; lParticle_I < self->particleLocalCount; lParticle_I++ ) {
		Swarm_UpdateParticleOwner( self, lParticle_I );
	}	

	/* UpdateAllParticleOwners is called during initialisation,to set up initial
	 * ownership relationships, and if that's the case we don't want to invoke
	 * the ParticleCommHandler since we know there's been no movement between
	 * processors yet. */
	if ( False == self->stillDoingInitialisation ) { 
		ParticleCommHandler_HandleParticleMovementBetweenProcs( self->particleCommunicationHandler );
	}

	Stream_UnIndentBranch( Swarm_Debug );	
}


void Swarm_UpdateParticleOwner( void* swarm, Particle_Index particle_I ) {
	Swarm* 			self           = (Swarm*)swarm;
	GlobalParticle*	        particle       = (GlobalParticle*) Swarm_ParticleAt( self, particle_I );
	Cell_DomainIndex	newOwningCell;
	Particle_InCellIndex	cParticle_I;
	Coord*			coordPtr       = &particle->coord;

	Journal_DPrintfL( self->debug, 3, "In %s: for particle %d, old cell %d\n", __func__,
		particle_I, particle->owningCell );
	Stream_Indent( self->debug );	
	
	Journal_DPrintfL( self->debug, 3, "updated coord (%f,%f,%f) is: ", (*coordPtr)[0], (*coordPtr)[1], (*coordPtr)[2] );

	newOwningCell = CellLayout_CellOf( self->cellLayout, particle );

	if ( newOwningCell == particle->owningCell ) {
		Journal_DPrintfL( self->debug, 3, "still in same cell.\n" );
	}
	else {
		Cell_LocalIndex		oldOwningCell = particle->owningCell;

		Journal_DPrintfL( self->debug, 3, "in new cell %d.\n", newOwningCell );
		cParticle_I = Swarm_GetParticleIndexWithinCell( self, particle->owningCell, particle_I );
		Swarm_RemoveParticleFromCell( self, oldOwningCell, cParticle_I );
	
		/* if new cell is in my domain, add entry to new cell's table */
		if ( newOwningCell == self->cellDomainCount ) {
			Journal_DPrintfL( self->debug, 3, "New cell == domain count -> Particle has moved outside domain.\n" );
			particle->owningCell = self->cellDomainCount;
		}	
		#ifdef CAUTIOUS
		else if ( newOwningCell >= self->cellDomainCount ) {	
			Stream*    errorStream = Journal_Register( Error_Type, self->type );

			Journal_Firewall( 0, errorStream,
				"Error - in %s(): particle %u's new domain cell calculated as "
				"%u, but this is greater than the count of domain cells %u. "
				"Something has gone wrong.\n",
				__func__, particle_I, newOwningCell, self->cellDomainCount );
		}		
		#endif
		else {
			Swarm_AddParticleToCell( self, newOwningCell, particle_I );
		}
	}
	Stream_UnIndent( self->debug );	
}


void Swarm_RemoveParticleFromCell( void* swarm, Cell_DomainIndex dCell_I, Particle_InCellIndex cParticle_I ) {
	Swarm* 			self = (Swarm*)swarm;
	Particle_InCellIndex*	sizePtr = &self->cellParticleSizeTbl[dCell_I];
	Particle_InCellIndex*	countPtr = &self->cellParticleCountTbl[dCell_I];

	Journal_DPrintfL( self->debug, 3, "Removing PIC %d from cell %d: particle count now %d",
		cParticle_I, dCell_I, (*countPtr)-1 );
	self->cellParticleTbl[dCell_I][cParticle_I] = self->cellParticleTbl[dCell_I][*countPtr-1];
	(*countPtr)--;
	if ( *countPtr == (*sizePtr - self->cellParticleTblDelta) ) {
		Journal_DPrintfL( self->debug, 3, " - (reducing entries alloced to %d)", (*countPtr) );
		(*sizePtr) = *countPtr;
		self->cellParticleTbl[dCell_I] = Memory_Realloc_Array( self->cellParticleTbl[dCell_I],
			Particle_Index, *sizePtr );
	}
	Journal_DPrintfL( self->debug, 3, "\n" );
}


void Swarm_DeleteParticle( void* swarm, Particle_Index particleToDelete_lI ) {
	Swarm* 			self = (Swarm*)swarm;
	Particle_InCellIndex    cParticle_I = 0;
	Particle_Index          lastParticle_I = 0;
	GlobalParticle*         lastParticle = NULL;	
	GlobalParticle*         particleToDelete = NULL;	
	SizeT                   particleSize = self->particleExtensionMgr->finalSize;
	Stream*                 errorStr = Journal_Register( Error_Type, self->type );

	Journal_Firewall( particleToDelete_lI < self->particleLocalCount, errorStr,
		"Error- in %s(): particleToDelete_lI passed in (%u) is >= swarm's local particle count %u.\n",
		__func__, particleToDelete_lI, self->particleLocalCount );

	particleToDelete = (GlobalParticle*)Swarm_ParticleAt( self, particleToDelete_lI );
	cParticle_I = Swarm_GetParticleIndexWithinCell( self, particleToDelete->owningCell, particleToDelete_lI );

	Swarm_RemoveParticleFromCell( self, particleToDelete->owningCell, cParticle_I );

	lastParticle_I = self->particleLocalCount - 1;
	lastParticle   = (GlobalParticle*)Swarm_ParticleAt( self, lastParticle_I );

	/* In the current data structure for particles (a regular array), if we delete a particle we need to "swap"
	 * the 'last' particle into the hole we just created, and update it's cell's reference to it.
	 * The only special case is if the particle we are deleting happens to be the last particle, in which case
	 * no swap is necessary. */ 
	if ( particleToDelete_lI != lastParticle_I ) {
		/* Get last Particle information */
		Cell_Index            lastParticle_CellIndex       = lastParticle->owningCell;
		Particle_InCellIndex  lastParticle_IndexWithinCell = 0;
		
		lastParticle_IndexWithinCell = Swarm_GetParticleIndexWithinCell( self, lastParticle_CellIndex, lastParticle_I);

		Journal_DPrintfL( self->debug, 2, 
				"Copying over particle %u using last particle %u from cell %u (cell particle index - %u)\n", 
				particleToDelete_lI, lastParticle_I, lastParticle_CellIndex, lastParticle_IndexWithinCell );

		/* Copy over particle */
		memcpy( particleToDelete, lastParticle, particleSize );
			
		/* Change value in cell particle table to point to new index in array */
		self->cellParticleTbl[lastParticle_CellIndex][ lastParticle_IndexWithinCell ] = particleToDelete_lI;
	}

	/* Re-set memory at location of last particle to zero so it is clear that it's been deleted */
	memset( lastParticle, 0, particleSize );

	self->particleLocalCount--;
	/* Call the memory management function in case we need to re-allocate the swarm size smaller now */
	Swarm_Realloc( swarm );
}


void Swarm_DeleteParticleAndReplaceWithNew( void* swarm, Particle_Index particleToDelete_lI,
		void* replacementParticle, Cell_Index replacementParticle_cellIndex )
{
	Swarm* 			self = (Swarm*)swarm;
	Particle_InCellIndex    cParticle_I = 0;
	GlobalParticle*         particleToDelete = NULL;	
	Stream*                 errorStr = Journal_Register( Error_Type, self->type );
	
	Journal_Firewall( particleToDelete_lI < self->particleLocalCount, errorStr,
		"Error- in %s(): particleToDelete_lI passed in (%u) is >= swarm's local particle count %u.\n",
		__func__, particleToDelete_lI, self->particleLocalCount );

	particleToDelete = (GlobalParticle*)Swarm_ParticleAt( self, particleToDelete_lI );
	cParticle_I = Swarm_GetParticleIndexWithinCell( self, particleToDelete->owningCell, particleToDelete_lI );

	Swarm_RemoveParticleFromCell( self, particleToDelete->owningCell, cParticle_I );

	Journal_DPrintfL( self->debug, 2, 
		"Copying over particle %u using replacement particle, and adding it to cell %u\n", 
		particleToDelete_lI, replacementParticle_cellIndex );

	/* Copy over particle to delete with it's replacement */
	memcpy( particleToDelete, replacementParticle, self->particleExtensionMgr->finalSize );
			
	/* Add a reference to replacement particle in appropriate cell entry */
	Swarm_AddParticleToCell( self, replacementParticle_cellIndex, particleToDelete_lI );
}


void Swarm_AddParticleToCell( void* swarm, Cell_DomainIndex dCell_I, Particle_Index particle_I ) {
	Swarm* 			self = (Swarm*)swarm;
	Particle_InCellIndex*	newCountPtr = &self->cellParticleCountTbl[dCell_I];
	Particle_InCellIndex*	newSizePtr = &self->cellParticleSizeTbl[dCell_I];
	#ifdef CAUTIOUS
	Stream*                 errorStream = Journal_Register( Error_Type, self->type );
	#endif

	Journal_DPrintfL( self->debug, 3, "Adding particle %d to cell %d: cell's particle count now %d",
		particle_I, dCell_I, (*newCountPtr)+1 );

	#ifdef CAUTIOUS
	Journal_Firewall( dCell_I < self->cellDomainCount, errorStream,
		"Error - in %s(): cannot add particle %u to req. domain cell, since "
		"dCell_I passed in of %u is greater than the count of domain cells %u.\n",
		__func__, particle_I, dCell_I, self->cellDomainCount );
	#endif

	Swarm_ParticleAt( self, particle_I)->owningCell = dCell_I;

	if ( *newCountPtr == *newSizePtr ) {
		(*newSizePtr) += self->cellParticleTblDelta;
		Journal_DPrintfL( self->debug, 3, " - (increasing entries alloced to %d)", (*newSizePtr) );
		self->cellParticleTbl[dCell_I] = Memory_Realloc_Array( self->cellParticleTbl[dCell_I],
			Particle_Index, (*newSizePtr) );
	}	
	self->cellParticleTbl[dCell_I][*newCountPtr] = particle_I;
	(*newCountPtr)++;
	Journal_DPrintfL( self->debug, 3, "\n" );
}


Particle_InCellIndex Swarm_GetParticleIndexWithinCell( void* swarm, Cell_DomainIndex owningCell, Particle_Index particle_I) {
	Swarm* 			self = (Swarm*)swarm;
	Particle_InCellIndex	cParticle_I;
	Particle_InCellIndex	particleCount = self->cellParticleCountTbl[owningCell];

	/* find the PIC index in the cell */
	for ( cParticle_I=0; cParticle_I < particleCount; cParticle_I++ ) {
		if ( particle_I == self->cellParticleTbl[owningCell][cParticle_I] ) break;
	}	
	#if DEBUG
		Journal_Firewall( cParticle_I < self->cellParticleCountTbl[owningCell], Swarm_Error,
			"Error- In func %s: Particle %u not found among cell %u's %u particles.\n",
			__func__, particle_I, owningCell, particleCount );
	#endif

	return cParticle_I;
}

Particle_Index Swarm_FindClosestParticle( void* _swarm, Dimension_Index dim, double* coord, double *distance) {
	Swarm*               swarm       = (Swarm*) _swarm;
	Particle_InCellIndex cParticle_I;
	Cell_LocalIndex      lCell_I;
	GlobalParticle       testParticle;
	double               minDistance;
	double               distanceToParticle;
	Topology*            topology;
	NeighbourIndex       neighbourCount;
	NeighbourIndex       neighbour_I;
	NeighbourIndex*      neighbourList;
	Particle_Index       closestParticle_I;

	/* Find cell this coordinate is in */
	memcpy( testParticle.coord, coord, sizeof(Coord) );
	/* First specify the particle doesn't have an owning cell yet, so as
	not to confuse the search algorithm */
	testParticle.owningCell = swarm->cellDomainCount;
	lCell_I = CellLayout_CellOf( swarm->cellLayout, &testParticle );

	/* Test if this cell is on this processor - if not then bail */
	if (lCell_I >= swarm->cellLocalCount)
		return (Particle_Index) -1;

	/* Find Closest Particle in this Cell */
	cParticle_I = Swarm_FindClosestParticleInCell( swarm, lCell_I, dim, coord, &minDistance );

	/* Convert to Local Particle Index */
	closestParticle_I = swarm->cellParticleTbl[ lCell_I ][ cParticle_I ];

	/* Find neighbours to this cell - TODO This Assumes ElementCellLayout */
	topology = ((ElementCellLayout*)swarm->cellLayout)->mesh->layout->elementLayout->topology;
	neighbourCount = Topology_NeighbourCount( topology, lCell_I );
	neighbourList  = Memory_Alloc_Array( NeighbourIndex, neighbourCount, "ElementNeighbours" );
	Topology_BuildNeighbours( topology, lCell_I, neighbourList );

	/* Loop over neighbours */
	for ( neighbour_I = 0 ; neighbour_I < neighbourCount ; neighbour_I++ ) {
		lCell_I = neighbourList[ neighbour_I ];
		if ( lCell_I == Topology_Invalid( topology ) )
			continue;

		/* TODO - Be more clever than checking every particle in this cell */
		cParticle_I = Swarm_FindClosestParticleInCell( swarm, lCell_I, dim, coord, &distanceToParticle );

		/* Check to see if closest particle in this cell is closest to this coord */
		if (minDistance > distanceToParticle) {
			minDistance = distanceToParticle;
			closestParticle_I = swarm->cellParticleTbl[ lCell_I ][ cParticle_I ];
		}
	}
	Memory_Free( neighbourList );

	/* Return Distance to this particle */
	if (distance != NULL)
		*distance = minDistance;
	return closestParticle_I;
}


Particle_InCellIndex Swarm_FindClosestParticleInCell( void* swarm, Cell_DomainIndex dCell_I, Dimension_Index dim, double* coord, double* distance ) {
	Swarm*               self                = (Swarm*) swarm;
	Particle_InCellIndex cParticle_I         = 0;
	Particle_InCellIndex particle_I          = 0;
	GlobalParticle*      particle            = NULL;
	double               minDistance         = HUGE_VAL;
	double               distanceToParticle;

	/* TODO: need to reconsideer - gauss particle layout should be allowed, but not swarms that have no local
	 * co-ordinates */
	/*
	Journal_Firewall(
		self->particleLayout->coordSystem == GlobalCoordSystem,
		Journal_MyStream( Error_Type, self ),
		"Error in %s(), swarm %s:%s is not using a global coord system\n",
		__func__,
		self->type,
		self->name );
	*/	
	
	Journal_Firewall( dCell_I < self->cellDomainCount, Swarm_Error, "Bad Cell_DomainIndex %u.\n", dCell_I );

	/* Loop over particles find closest to vertex */
	for( cParticle_I = 0 ; cParticle_I < self->cellParticleCountTbl[dCell_I] ; cParticle_I++ ) {
		particle = (GlobalParticle*)Swarm_ParticleInCellAt( self, dCell_I, cParticle_I );

		/* Calculate distance to particle */
		distanceToParticle = 
			(particle->coord[ I_AXIS ] - coord[ I_AXIS ]) * 
			(particle->coord[ I_AXIS ] - coord[ I_AXIS ]) +
			(particle->coord[ J_AXIS ] - coord[ J_AXIS ]) * 
			(particle->coord[ J_AXIS ] - coord[ J_AXIS ]) ;

		if (dim == 3) {
			distanceToParticle += 
				(particle->coord[ K_AXIS ] - coord[ K_AXIS ]) * 
				(particle->coord[ K_AXIS ] - coord[ K_AXIS ]) ;
		}
		/* Don't do square root here because it is unnessesary: i.e. a < b <=> sqrt(a) < sqrt(b) */
			
		/* Check if this is the closest particle */
		if (minDistance > distanceToParticle) {
			particle_I = cParticle_I;
			minDistance = distanceToParticle;
		}
	}

	/* Return Distance to this particle */
	if (distance != NULL)
		/* Do square root here in case someone wants to actually use this distance */
		*distance = sqrt(minDistance);

	return particle_I;
}

void Swarm_PrintParticleCoords( void* swarm, Stream* stream ) {
	Swarm* 			self = (Swarm*)swarm;
	Particle_Index		lParticle_I=0;
	GlobalParticle*   	currParticle = NULL;
	double*			coord = NULL;
	
	Journal_Printf( stream, "Printing coords of all local processor particles:\n" );
	
	Stream_Indent( stream );
	for ( lParticle_I = 0; lParticle_I < self->particleLocalCount; lParticle_I++ ) {
		currParticle = (GlobalParticle*)Swarm_ParticleAt( self, lParticle_I );
		coord = currParticle->coord;

		Journal_Printf( stream, "local particle %6d: (%7.5g, %7.5g, %7.5g)\n",
			lParticle_I, coord[0], coord[1], coord[2] );
	}
	Stream_UnIndent( stream );
}


void Swarm_PrintParticleCoords_ByCell( void* swarm, Stream* stream ) {
	Swarm* 			self = (Swarm*)swarm;
	Cell_Index		lCell_I=0;
	Particle_InCellIndex	cParticle_I=0;
	Particle_Index		lParticle_I=0;
	GlobalParticle*         currParticle = NULL;
	double*			coord = NULL;
	
	Journal_Printf( stream, "Printing coords of all local particles, cell-by-cell:\n" );
	
	Stream_Indent( stream );
	for ( lCell_I = 0; lCell_I < self->cellLocalCount; lCell_I++ ) {
		Journal_Printf( stream, "Local Cell %3d:\n", lCell_I );
		
		for ( cParticle_I = 0; cParticle_I < self->cellParticleCountTbl[lCell_I]; cParticle_I++ ) {
			lParticle_I = self->cellParticleTbl[lCell_I][cParticle_I];
			currParticle = (GlobalParticle*)Swarm_ParticleAt( self, lParticle_I );
			coord = currParticle->coord;

			Journal_Printf( stream, "\tpart. InCell %3d (local index %6d): (%7.5g, %7.5g, %7.5g)\n",
				cParticle_I, lParticle_I, coord[0], coord[1], coord[2] );
		}		
	}
	Stream_UnIndent( stream );
}


void Swarm_GetCellMinMaxCoords( void* swarm, Cell_DomainIndex cell_I, Coord min, Coord max ) {
	Swarm*              self     =  (Swarm*) swarm;
	Dimension_Index		dim_I;
	Cell_PointIndex		cPoint_I;
	double*             currCoord;

	for ( dim_I = 0; dim_I < 3; dim_I++ ) {
		min[dim_I] = (*self->cellPointTbl[cell_I][0])[dim_I];
		max[dim_I] = (*self->cellPointTbl[cell_I][0])[dim_I];
	}

	for ( cPoint_I = 1; cPoint_I < self->cellPointCountTbl[cell_I]; cPoint_I++ ) {
		for ( dim_I = 0; dim_I < 3; dim_I++ ) {
			currCoord = (*self->cellPointTbl[cell_I][cPoint_I]);
			if ( currCoord[dim_I] < min[dim_I] ) {
				min[dim_I] = currCoord[dim_I];
			}
			else if ( currCoord[dim_I] > max[dim_I] ) {
				max[dim_I] = currCoord[dim_I];
			}
		}
	}
}



SwarmVariable* Swarm_NewScalarVariable( 
		void*                           swarm,
		Name                            nameExt,
		SizeT                           dataOffset,
		Variable_DataType               dataType )
{
	Swarm*                   self              = (Swarm*) swarm;
	Name                     name;
	Variable*                variable;
	SizeT                    dataOffsets[]     = { 0 };		/* Init value later */
	Variable_DataType        dataTypes[]       = { 0 };		/* Init value later */
	Index                    dataTypeCounts[]  = { 1 };
	SwarmVariable*           swarmVariable;
	Variable_Register*       variable_Register      = NULL;
	SwarmVariable_Register*  swarmVariable_Register = NULL;

	Journal_Firewall(
		dataOffset < ExtensionManager_GetFinalSize( self->particleExtensionMgr ),
		Journal_MyStream( Error_Type, self ),
		"Error in func %s - Attempting to create a SwarmVariable on an extension that does not exists.\n"
		"\t%s %s\n"
		"\tdataOffset = %d, particle size = %d\n",
		__func__,
		self->type,
		self->name,
		dataOffset,
		ExtensionManager_GetFinalSize( self->particleExtensionMgr ) );
		
	dataOffsets[0] = dataOffset;
	dataTypes[0]   = dataType;
	
	/* Get Pointers To Registers */
	swarmVariable_Register = self->swarmVariable_Register;
	if ( swarmVariable_Register ) 
		variable_Register = swarmVariable_Register->variable_Register;
	
	name = Stg_Object_AppendSuffix( self, nameExt );
	variable = Variable_New( 
		name,
		1, 
		dataOffsets, 
		dataTypes, 
		dataTypeCounts, 
		0, /* no component names */
		&self->particleExtensionMgr->finalSize,
		&self->particleLocalCount,
		(void**)&self->particles,
		variable_Register );

	swarmVariable = SwarmVariable_New( name, self, variable, 1 );
	
	Memory_Free( name );

	return swarmVariable;
}

SwarmVariable* Swarm_NewVectorVariable( 
		void*                           _swarm,
		Name                            nameExt,
		SizeT                           dataOffset,
		Variable_DataType               dataType,
		Index                           dataTypeCount,
		...                         /* vector component names */ )
{
	Swarm*                   self             = (Swarm*) _swarm;
	Variable*                variable;
	SizeT                    dataOffsets[]    = { 0 };	/* Init later... */
	Variable_DataType	     dataTypes[]      = { 0 };	/* Init later... */	
	Index                    dataTypeCounts[] = { 0 };	/* Init later... */	
	Name*                    dataNames;
	Index                    vector_I;
	Name                     name;
	SwarmVariable*           swarmVariable;
	Variable_Register*       variable_Register      = NULL;
	SwarmVariable_Register*  swarmVariable_Register = NULL;
	va_list                  ap;

	/* Initialise arrays */
	dataOffsets[0]    = dataOffset;
	dataTypes[0]      = dataType;
	dataTypeCounts[0] = dataTypeCount;
	
	/* Create name for normal variable */
	Stg_asprintf( &name, "%s-%s", self->name, nameExt );

	/* Get names of extra variables */
	dataNames = Memory_Alloc_Array( Name, dataTypeCount, "dataNames" );
	va_start( ap, dataTypeCount );
	for( vector_I = 0; vector_I < dataTypeCount; vector_I++ ) {
		dataNames[vector_I] = Stg_Object_AppendSuffix( self, (Name) va_arg( ap, Name ) );
	}
	va_end( ap );
	
	/* Get Pointers To Registers */
	swarmVariable_Register = self->swarmVariable_Register;
	if ( swarmVariable_Register ) 
		variable_Register = swarmVariable_Register->variable_Register;
	
	/* Construct */
	variable = Variable_New( 
		name,
		1, 
		dataOffsets, 
		dataTypes, 
		dataTypeCounts, 
		dataNames,
		&self->particleExtensionMgr->finalSize,
		&self->particleLocalCount,
		(void**)&self->particles,
		variable_Register );

	/* Need to free these guys individually */
	for( vector_I = 0; vector_I < dataTypeCount; vector_I++ ) {
		if ( swarmVariable_Register && variable_Register ) {
			SwarmVariable_New( 
					dataNames[ vector_I ],
					self, 
					Variable_Register_GetByName( variable_Register, dataNames[ vector_I ] ),
					1 );
		}
		Memory_Free( dataNames[ vector_I ] );
	}
	swarmVariable = SwarmVariable_New( name, self, variable, dataTypeCount );

	Memory_Free( dataNames );
	Memory_Free( name );
	
	return swarmVariable;
}


void Swarm_Realloc( void* swarm ) {
	Swarm*         self               = (Swarm*) swarm;
	Particle_Index particleLocalCount = self->particleLocalCount;
	Particle_Index delta              = self->particlesArrayDelta;

	if ( particleLocalCount <= self->particlesArraySize - delta ) {
		/* Decrease size of array if necessary */
		self->particlesArraySize = particleLocalCount;
	}
	else if ( particleLocalCount >= self->particlesArraySize ) {
		/* Increase size of array if necessary */
		self->particlesArraySize = particleLocalCount + delta;
	}
	else {
		/* If no change in the size of the array happened then get out of this function */
		return;
	}

	/* Do realloc */
	self->particles = Memory_Realloc_Array_Bytes(
			self->particles,
			self->particleExtensionMgr->finalSize,
			self->particlesArraySize );


#ifdef DEBUG
	/* Set extra memory at end of array to zero */
	if ( self->particlesArraySize > particleLocalCount ) {
		Particle_Index newParticleCount = self->particlesArraySize - particleLocalCount;
		void*          startNewParticlePtr = Swarm_ParticleAt( self, particleLocalCount );
		memset( startNewParticlePtr, 0, self->particleExtensionMgr->finalSize * newParticleCount );
	}
#endif	
}


void Swarm_CheckCoordsAreFinite( void* swarm ) {
	Swarm*              self               = (Swarm*) swarm;
	GlobalParticle*     particle;
	double*             coord;
	Stream*             errorStream        = Journal_Register( Error_Type, self->type );
	Dimension_Index     dim                = self->dim;
	Particle_Index      particleLocalCount = self->particleLocalCount;
	Particle_Index      lParticle_I;

	for ( lParticle_I = 0 ; lParticle_I < particleLocalCount ; lParticle_I++ ) {
		particle = (GlobalParticle*)Swarm_ParticleAt( self, lParticle_I );
		coord    = particle->coord;
		
		Journal_Firewall( 
				! isinf( coord[0] ) && ! isinf( coord[1] ) && ( dim == 2 || ! isinf(coord[2]) ), 
				errorStream,
				"Error in func %s - Coord for particle with %u is not finite (%g, %g, %g).\n", 
				__func__, lParticle_I, coord[0], coord[1], (dim == 3 ? coord[2] : 0.0) );
	}
}


void Swarm_AssignIndexWithinShape( void* swarm, void* _shape, Variable* variableToAssign, Index indexToAssign ) {
	Swarm*            self              = Stg_CheckType( swarm, Swarm );
	Stg_Shape*        shape             = Stg_CheckType( _shape, Stg_Shape );
	GlobalParticle* particle;
	Particle_Index    lParticle_I;

	Journal_Firewall( 
		self->particleLocalCount == variableToAssign->arraySize,
		Journal_Register( Error_Type, self->type ),
		"In func %s: Trying to assign to variable '%s' with a different number of values \
		than the number of particles in swarm '%s'.\n",
		__func__, variableToAssign->name, self->name );
		


	for ( lParticle_I = 0 ; lParticle_I < self->particleLocalCount ; lParticle_I++ ) {
		particle = (GlobalParticle*)Swarm_ParticleAt( self, lParticle_I );

		if ( Stg_Shape_IsCoordInside( shape, particle->coord ) ) 
			Variable_SetValueInt( variableToAssign, lParticle_I, indexToAssign );
	}
}

StandardParticle* Swarm_CreateNewParticle( void* swarm, Particle_Index* newParticle_I ) {
	Swarm*              self               = (Swarm*) swarm;

	*newParticle_I = self->particleLocalCount;
	self->particleLocalCount++;
	Swarm_Realloc( self );
	
	return Swarm_ParticleAt( self, *newParticle_I );
}


void Swarm_ReplaceCurrentParticleLayoutWithFileParticleLayout( void* swarm, void* _context ) {
	Swarm*               self = (Swarm*)swarm;
	AbstractContext*     context = (AbstractContext*)_context;
	char                 name[1024];
	char                 swarmSaveFileName[4096];

	sprintf( name, "%s-fileParticleLayout", self->name );
	Swarm_GetCheckpointFilenameForGivenTimestep( self, context, swarmSaveFileName );
	
	Journal_DPrintf( self->debug, "overriding the particleLayout specified via XML/constructor\n"
		"of \"%s\" (of type %s) with a FileParticleLayout to load\n"
		"this swarm's checkpoint file from outputPath \"%s\",\n"
		"with prefix \"%s\" from timestep %u with total name:\n\"%s\"\n",
		self->particleLayout->name, self->particleLayout->type,
		context->outputPath,
		context->checkPointPrefixString, context->restartTimestep,
		swarmSaveFileName );

	/* TODO: deleting this makes sense if this swarm "owns" the particle layout. Is it
	* possible for 2 swarms to have the same particle layout? I guess so - may need
	* to rethink later. Deleting is ok though for now since the "reference counter"
	* was incremented when we got it out of the LC register.
	* PatrickSunter - 13 June 2006
	*/
	Stg_Class_Delete( self->particleLayout );

	self->particleLayout = (ParticleLayout*)FileParticleLayout_New( name, swarmSaveFileName ); 
}


void Swarm_GetCheckpointFilenameForGivenTimestep( Swarm* self, AbstractContext* context, char* swarmSaveFileName ) {
	if ( strlen(context->checkPointPrefixString) > 0 ) {
		sprintf( swarmSaveFileName, "%s/%s.%s.%05d.dat", context->outputPath,
			context->checkPointPrefixString, self->name, context->restartTimestep );
	}
	else {
		sprintf( swarmSaveFileName, "%s/%s.%05d.dat", context->outputPath,
			self->name, context->restartTimestep );
	}
}


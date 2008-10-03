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
** $Id: PerCellParticleLayout.c 3526 2006-04-13 03:59:17Z AlanLo $
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
#include "ParticleLayout.h"
#include "PerCellParticleLayout.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "SwarmClass.h"
#include "StandardParticle.h"

const Type PerCellParticleLayout_Type = "PerCellParticleLayout";

PerCellParticleLayout* _PerCellParticleLayout_New( 
                SizeT                                                       _sizeOfSelf,
                Type                                                        type,
                Stg_Class_DeleteFunction*                                   _delete,
                Stg_Class_PrintFunction*                                    _print,
                Stg_Class_CopyFunction*                                     _copy,
                Stg_Component_DefaultConstructorFunction*                   _defaultConstructor,
                Stg_Component_ConstructFunction*                            _construct,
                Stg_Component_BuildFunction*                                _build,
                Stg_Component_InitialiseFunction*                           _initialise,
                Stg_Component_ExecuteFunction*                              _execute,
                Stg_Component_DestroyFunction*                              _destroy,
                ParticleLayout_SetInitialCountsFunction*                    _setInitialCounts,
                ParticleLayout_InitialiseParticlesFunction*                 _initialiseParticles,
                PerCellParticleLayout_InitialCountFunction*                 _initialCount,
                PerCellParticleLayout_InitialiseParticlesOfCellFunction*    _initialiseParticlesOfCell,
                Name                                                        name,
                Bool                                                        initFlag,
		CoordSystem                                                 coordSystem,
                Bool                                                        weightsInitialisedAtStartup )
{
	PerCellParticleLayout*		self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(PerCellParticleLayout) );
	self = (PerCellParticleLayout*)_ParticleLayout_New( 
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
			_setInitialCounts, 
			_initialiseParticles, 
			name, 
			initFlag,
			coordSystem,
			weightsInitialisedAtStartup );
	
	/* General info */
	
	/* Virtual functions */
	self->_initialCount = _initialCount;
	self->_initialiseParticlesOfCell = _initialiseParticlesOfCell;
	
	/* PerCellParticleLayout info */
	if( initFlag ){
		_PerCellParticleLayout_Init( self, coordSystem, weightsInitialisedAtStartup );
	}
	
	return self;
}


void _PerCellParticleLayout_Init(
		void*                  particleLayout,
		CoordSystem            coordSystem,
		Bool                   weightsInitialisedAtStartup )
{
	PerCellParticleLayout* self = (PerCellParticleLayout*)particleLayout;
	
	self->isConstructed = True;

	_ParticleLayout_Init( particleLayout, coordSystem, weightsInitialisedAtStartup );
}

void _PerCellParticleLayout_Delete( void* particleLayout ) {
	PerCellParticleLayout* self = (PerCellParticleLayout*)particleLayout;
	
	_ParticleLayout_Delete( self );
}

void _PerCellParticleLayout_Print( void* particleLayout, Stream* stream ) {
	PerCellParticleLayout* self = (PerCellParticleLayout*)particleLayout;
	
	Journal_Printf( stream, "PerCellParticleLayout (ptr): %p\n", self );
	
	/* Parent class info */
	_ParticleLayout_Print( self, stream );
	
	/* Virtual info */
	Journal_Printf( stream, "\t_initialCount (func ptr): %p\n", self->_initialCount );
	Journal_Printf( stream, "\t_initialiseParticlesOfCell (func ptr): %p\n", self->_initialiseParticlesOfCell );
	
}


void* _PerCellParticleLayout_Copy( void* particleLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	PerCellParticleLayout*		self = (PerCellParticleLayout*)particleLayout;
	PerCellParticleLayout*		newPerCellParticleLayout;
	
	newPerCellParticleLayout = (PerCellParticleLayout*)_ParticleLayout_Copy( self, dest, deep, nameExt, ptrMap );

	/* Virtual methods */
	newPerCellParticleLayout->_initialCount = self->_initialCount;
	newPerCellParticleLayout->_initialiseParticlesOfCell = self->_initialiseParticlesOfCell;
	
	return (void*)newPerCellParticleLayout;
}


/* Note: this function is required to also set the total particle count */
void _PerCellParticleLayout_SetInitialCounts( void* particleLayout, void* _swarm )
{
	PerCellParticleLayout*	self = (PerCellParticleLayout*)particleLayout;
	Swarm*			swarm = (Swarm*)_swarm;
	Cell_DomainIndex	cell_I = 0;
	char			tempStr[100];

	for( cell_I = 0; cell_I < swarm->cellLocalCount; cell_I++ ) {
		swarm->cellParticleCountTbl[cell_I] = PerCellParticleLayout_InitialCount( self, swarm->cellLayout, cell_I );
		
		/* Initially, just set to the same sizes as initial particle counts */
		swarm->cellParticleSizeTbl[cell_I] = swarm->cellParticleCountTbl[cell_I];
		sprintf( tempStr, "Swarm->cellParticleTbl[%d]", cell_I );
		swarm->cellParticleTbl[cell_I] = Memory_Alloc_Array( Particle_Index, swarm->cellParticleCountTbl[cell_I], tempStr );
		swarm->particleLocalCount += swarm->cellParticleCountTbl[cell_I];
	}

	/* Now initialise the shadow cell particle counts */
	for (; cell_I < swarm->cellDomainCount; cell_I++ ) {
		swarm->cellParticleCountTbl[cell_I] = 0;
		swarm->cellParticleSizeTbl[cell_I] = 0;
		swarm->cellParticleTbl[cell_I] = NULL;
	}
}

void _PerCellParticleLayout_InitialiseParticles( void* particleLayout, void* _swarm )
{
	PerCellParticleLayout*	self = (PerCellParticleLayout*)particleLayout;
	Swarm*			swarm = (Swarm*)_swarm;
	Cell_Index		cell_I;
	Particle_InCellIndex	cellParticle_I;
	Particle_Index		currParticleIndex = 0;
	StandardParticle*	particle = NULL;
	
	/* Go through and init particles, cell-by-cell */
	/* Note: don't put particles in the shadow cells... */
	for( cell_I = 0; cell_I < swarm->cellLocalCount; cell_I++ ) {
		Particle_InCellIndex		count = swarm->cellParticleCountTbl[cell_I];
		
		for( cellParticle_I = 0; cellParticle_I < count; cellParticle_I++ ) {
			swarm->cellParticleTbl[cell_I][cellParticle_I] = currParticleIndex++;
			particle = Swarm_ParticleInCellAt( swarm, cell_I, cellParticle_I );
			particle->owningCell = cell_I;
		}	

		PerCellParticleLayout_InitialiseParticlesOfCell( self, swarm, cell_I );
	}
}

void PerCellParticleLayout_InitialiseParticlesOfCell( void* particleLayout, void* _swarm, Cell_Index cell_I )
{
	PerCellParticleLayout* self = (PerCellParticleLayout*)particleLayout;
	
	self->_initialiseParticlesOfCell( self, _swarm, cell_I );
}

Particle_InCellIndex PerCellParticleLayout_InitialCount( void* particleLayout, void* _swarm, Cell_Index cell_I )
{
	PerCellParticleLayout* self = (PerCellParticleLayout*)particleLayout;
	
	return self->_initialCount( self, _swarm, cell_I );
}





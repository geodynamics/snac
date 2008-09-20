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
** $Id: GlobalParticleLayout.c 3768 2006-08-22 00:33:26Z PatrickSunter $
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
#include "GlobalParticleLayout.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "SwarmClass.h"
#include "StandardParticle.h"
#include "CellLayout.h"

const Type GlobalParticleLayout_Type = "GlobalParticleLayout";

GlobalParticleLayout* _GlobalParticleLayout_New( 
		SizeT                                               _sizeOfSelf,
		Type                                                type,
		Stg_Class_DeleteFunction*                           _delete,
		Stg_Class_PrintFunction*                            _print,
		Stg_Class_CopyFunction*                             _copy,
		Stg_Component_DefaultConstructorFunction*           _defaultConstructor,
		Stg_Component_ConstructFunction*                    _construct,
		Stg_Component_BuildFunction*                        _build,
		Stg_Component_InitialiseFunction*                   _initialise,
		Stg_Component_ExecuteFunction*                      _execute,
		Stg_Component_DestroyFunction*                      _destroy,
		ParticleLayout_SetInitialCountsFunction*            _setInitialCounts,
		ParticleLayout_InitialiseParticlesFunction*         _initialiseParticles,
		GlobalParticleLayout_InitialiseParticleFunction*    _initialiseParticle,
		Name                                                name,
		Bool                                                initFlag,
		CoordSystem                                         coordSystem,
		Bool                                                weightsInitialisedAtStartup,
		Particle_Index                                      totalInitialParticles,
		double                                              averageInitialParticlesPerCell )
{
	GlobalParticleLayout*		self;
	
	assert( _sizeOfSelf >= sizeof(GlobalParticleLayout) );
	self = (GlobalParticleLayout*)_ParticleLayout_New( 
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
	
	self->_initialiseParticle = _initialiseParticle;
	
	if( initFlag ){
		_GlobalParticleLayout_Init( 
				self, 
				coordSystem, 
				weightsInitialisedAtStartup, 
				totalInitialParticles, 
				averageInitialParticlesPerCell );
	}
	
	return self;
}


void _GlobalParticleLayout_Init(
		void*                                               particleLayout,
		CoordSystem                                         coordSystem,
		Bool                                                weightsInitialisedAtStartup,
		Particle_Index                                      totalInitialParticles,
		double                                              averageInitialParticlesPerCell )
{
	GlobalParticleLayout* self = (GlobalParticleLayout*)particleLayout;

	self->isConstructed = True;

	/* Note the total and average particles per cell need to be set in child
	classes, as they may be worked out differently (eg the ManualParticleLayout
	specifies the particles directly, so the total is implicit) */

	self->totalInitialParticles = totalInitialParticles;
	self->averageInitialParticlesPerCell = averageInitialParticlesPerCell;

	_ParticleLayout_Init( self, coordSystem, weightsInitialisedAtStartup );
}

void _GlobalParticleLayout_Delete( void* particleLayout ) {
	GlobalParticleLayout* self = (GlobalParticleLayout*)particleLayout;
	
	_ParticleLayout_Delete( self );
}

void _GlobalParticleLayout_Print( void* particleLayout, Stream* stream ) {
	GlobalParticleLayout* self = (GlobalParticleLayout*)particleLayout;
	
	Journal_Printf( stream, "GlobalParticleLayout (ptr): %p\n", self );
	
	/* Parent class info */
	_ParticleLayout_Print( self, stream );
	
	/* Virtual info */
	Journal_Printf( stream, "\t_initialiseParticle (func ptr): %p\n", self->_initialiseParticle );
	
	/* GlobalParticleLayout info */
	Journal_Printf( stream, "\ttotalInitialParticles: %u\n", self->totalInitialParticles );
	Journal_Printf( stream, "\taverageInitialParticlesPerCell: %g\n", self->averageInitialParticlesPerCell );
}


void* _GlobalParticleLayout_Copy( void* particleLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	GlobalParticleLayout*		self = (GlobalParticleLayout*)particleLayout;
	GlobalParticleLayout*		newGlobalParticleLayout;
	
	newGlobalParticleLayout = (GlobalParticleLayout*)_ParticleLayout_Copy( self, dest, deep, nameExt, ptrMap );

	/* Virtual methods */
	newGlobalParticleLayout->_initialiseParticle= self->_initialiseParticle;

	/* data */
	newGlobalParticleLayout->averageInitialParticlesPerCell = self->averageInitialParticlesPerCell;
	newGlobalParticleLayout->totalInitialParticles = self->totalInitialParticles;
	
	return (void*)newGlobalParticleLayout;
}


/* Note: this function is required to also set the total particle count */
void _GlobalParticleLayout_SetInitialCounts( void* particleLayout, void* _swarm )
{
	GlobalParticleLayout*	self = (GlobalParticleLayout*)particleLayout;
	Swarm*			swarm = (Swarm*)_swarm;
	Cell_LocalIndex		cell_I;
	char			tempStr[100];
	Cell_Index		globalCellCount;

	Journal_DPrintf( self->debug, "In %s(): for ParticleLayout \"%s\" (of type %s):\n",
		__func__, self->name, self->type );
	Stream_IndentBranch( Swarm_Debug );

	Journal_DPrintf( self->debug, "Calculating total number of cells in system across all processors "
		"using MPI_Allreduce:\n" );
	MPI_Allreduce( &swarm->cellLocalCount, &globalCellCount, 1, MPI_UNSIGNED, MPI_SUM, swarm->comm );
	Journal_DPrintf( self->debug, "...calculated total global cell count is %u\n", globalCellCount );
	
	Journal_DPrintf( self->debug, "Calculating totalInitialParticles and averageInitialParticlesPerCell:\n" );
	Stream_IndentBranch( Swarm_Debug );
	if ( self->totalInitialParticles ) {
		self->averageInitialParticlesPerCell = self->totalInitialParticles / (double)globalCellCount;
		Journal_DPrintf( self->debug, "totalInitialParticles set as %u, thus calculated "
			"averageInitialParticlesPerCell as %f.\n", self->totalInitialParticles,
			self->averageInitialParticlesPerCell );
	}
	else if ( self->averageInitialParticlesPerCell ) {
		self->totalInitialParticles = globalCellCount * self->averageInitialParticlesPerCell;
		Journal_DPrintf( self->debug, "averageInitialParticlesPerCell set as %f, thus calculated "
			"totalInitialParticles as %u.\n", self->averageInitialParticlesPerCell,
			self->totalInitialParticles );
	}
	else {
		Stream*     errorStream = Journal_Register( Error_Type, self->type );
		
		Journal_Firewall( self->totalInitialParticles || self->averageInitialParticlesPerCell,
			errorStream, "Error - in %s(), ParticleLayout \"%s\" (of type %s): neither "
			"totalInitialParticles or averageInitialParticlesPerCell was specified by "
			"child class - one of these needs to be calculated so the "
			"swarm cell memory storage can be set.\n",
			__func__, self->name, self->type );
	}
	Stream_UnIndentBranch( Swarm_Debug );

	/* We don't set the local particle count equal to totalInitialParticles,
	since we're not sure how many of those are local yet */
	swarm->particleLocalCount = 0;
		
	for( cell_I = 0; cell_I < swarm->cellLocalCount; cell_I++ ) {
		/* Set initial counts to empty, till we add the particles */
		swarm->cellParticleCountTbl[cell_I] = 0;
		/* Set initial sizes to averageInitialParticlesPerCell, since its our best guess at how many
		 * particles there will be after setup in this cell, and hence how much memory needed */
		swarm->cellParticleSizeTbl[cell_I] = (int)( self->averageInitialParticlesPerCell + 0.5 );
		
		sprintf( tempStr, "Swarm->cellParticleTbl[%d]", cell_I );
		swarm->cellParticleTbl[cell_I] = Memory_Alloc_Array( Particle_Index, swarm->cellParticleSizeTbl[cell_I], tempStr );
	}

	/* Now initialise the shadow cell particle counts */
	for (; cell_I < swarm->cellDomainCount; cell_I++ ) {
		swarm->cellParticleCountTbl[cell_I] = 0;
		swarm->cellParticleSizeTbl[cell_I] = 0;
		swarm->cellParticleTbl[cell_I] = NULL;
	}
	Stream_UnIndentBranch( Swarm_Debug );
}


void _GlobalParticleLayout_InitialiseParticles( void* particleLayout, void* _swarm )
{
	GlobalParticleLayout*	self = (GlobalParticleLayout*)particleLayout;
	Swarm*			swarm = (Swarm*)_swarm;
	GlobalParticle*         particle = NULL;
	Particle_Index		lParticle_I=0;
	Particle_Index		newParticle_I=0;
	Cell_Index		cell_I;
	Particle_Index          globalParticlesInitialisedCount=0;
	Stream*                 errorStream = Journal_Register( Error_Type, self->type );

	Journal_DPrintf( self->debug, "In %s(): for ParticleLayout \"%s\" (of type %s):\n",
		__func__, self->name, self->type );
	Stream_IndentBranch( Swarm_Debug );

	Journal_DPrintf( self->debug, "For each of the %u total global requested particles, "
		"generating a particle, and checking if it's in this processor's domain. If so, "
		"adding it to the appropriate local cell.\n", self->totalInitialParticles );
	Stream_IndentBranch( Swarm_Debug );

	while( newParticle_I < self->totalInitialParticles ) {
		
		particle = (GlobalParticle*)Swarm_ParticleAt( swarm, lParticle_I );
		GlobalParticleLayout_InitialiseParticle( self, swarm, newParticle_I, particle );
		/* Work out which cell the new particle is in */
		/* First specify the particle doesn't have an owning cell yet, so as
		not to confuse the search algorithm if its an irregular cell/mesh layout */
		particle->owningCell = swarm->cellDomainCount;

		cell_I = CellLayout_CellOf( swarm->cellLayout, particle );

		/* If we found a further particle inside our domain, add it to a cell */
		if ( cell_I < swarm->cellLocalCount ) {
			Journal_DPrintfL( self->debug, 3, "global particle %u at (%.2f,%.2f,%.2f) inside local cell %u\n"
				"adding it to cell and saving it as local particle %u.\n",
				newParticle_I, particle->coord[0], particle->coord[1], particle->coord[2],
				cell_I, lParticle_I );
				
			Stream_IndentBranch( Swarm_Debug );
			/* Add it to that cell */
			Swarm_AddParticleToCell( swarm, cell_I, lParticle_I );
			lParticle_I++;
			swarm->particleLocalCount++;
			Swarm_Realloc( swarm );
			Stream_UnIndentBranch( Swarm_Debug );
		}
		else {
			Journal_DPrintfL( self->debug, 4, "global particle %u at (%.2f,%.2f,%.2f) outside this proc's domain:\n"
				"ignoring.\n", newParticle_I, particle->coord[0], particle->coord[1], particle->coord[2] );
		}		
				
		newParticle_I++;
	}

	Stream_UnIndentBranch( Swarm_Debug );

	/* Do a test to make sure that the total particles assigned across all processors ==
		totalInitialParticles count */
	MPI_Allreduce( &swarm->particleLocalCount, &globalParticlesInitialisedCount, 1, MPI_UNSIGNED, MPI_SUM, swarm->comm );
	Journal_Firewall( globalParticlesInitialisedCount == self->totalInitialParticles, errorStream,
		"Error - in %s() - for GlobalParticleLayout \"%s\", of type %s: after initialising particles, "
		"actual global count of particles initialised was %u, whereas requested global total "
		"totalInitialParticles was %u. If actual is < requested, it means some particles were not "
		"identified by any processor as inside their domain. If actual > requested, it means that "
		"some particles were identified by _multiple_ processors as belonging to their domain. Both "
		"these states are erroneous.\n",
		__func__, self->name, self->type, globalParticlesInitialisedCount, self->totalInitialParticles );

	Stream_UnIndentBranch( Swarm_Debug );
}


void GlobalParticleLayout_InitialiseParticle( void* particleLayout, void* _swarm, Particle_Index newParticle_I, void* particle )
{
	GlobalParticleLayout* self = (GlobalParticleLayout*)particleLayout;
	
	self->_initialiseParticle( self, _swarm, newParticle_I, particle );
}



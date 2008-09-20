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
** $Id: RandomParticleLayout.c 3851 2006-10-12 08:57:22Z SteveQuenette $
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
#include "RandomParticleLayout.h"

#include "SwarmClass.h"
#include "Random.h"
#include "StandardParticle.h"

#include <stdio.h>
#include <stdlib.h>

const Type RandomParticleLayout_Type = "RandomParticleLayout";


RandomParticleLayout* RandomParticleLayout_New( 
		Name                 name, 
		Particle_InCellIndex cellParticleCount, 
		unsigned int         seed ) 
{
	RandomParticleLayout* self = (RandomParticleLayout*) _RandomParticleLayout_DefaultNew( name );
	_RandomParticleLayout_Init( self, cellParticleCount, seed );

	return self;
}

RandomParticleLayout* _RandomParticleLayout_New( 
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
		Bool                                                         initFlag,
		Particle_InCellIndex                                        cellParticleCount,
		unsigned int                                                seed )
{
	RandomParticleLayout* self;
	
	/* Allocate memory */
	self = (RandomParticleLayout*)_PerCellParticleLayout_New( 
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
		_initialCount,
		_initialiseParticlesOfCell,
		name,
		initFlag,
		GlobalCoordSystem,
		False );

	if ( initFlag ) {
		_RandomParticleLayout_Init( self, cellParticleCount, seed );
	}
	
	return self;
}

void _RandomParticleLayout_Init( void* randomParticleLayout, Particle_InCellIndex cellParticleCount, unsigned int seed ) {
	RandomParticleLayout* self = (RandomParticleLayout*)randomParticleLayout;

	self->isConstructed     = True;
	self->cellParticleCount = cellParticleCount;
	self->seed              = seed;
	
	Swarm_Random_Seed( self->seed );

	_PerCellParticleLayout_Init( randomParticleLayout, GlobalCoordSystem, False );
}


void _RandomParticleLayout_Delete( void* randomParticleLayout ) {
	RandomParticleLayout* self = (RandomParticleLayout*)randomParticleLayout;
	
	/* Stg_Class_Delete parent class */
	_PerCellParticleLayout_Delete( self );
}

void _RandomParticleLayout_Print( void* randomParticleLayout, Stream* stream ) {
	RandomParticleLayout* self = (RandomParticleLayout*)randomParticleLayout;
	
	/* Set the Journal for printing informations */
	Stream* randomParticleLayoutStream = stream;
	
	/* General info */
	Journal_Printf( randomParticleLayoutStream, "RandomParticleLayout (ptr): %p:\n", self );
	
	/* Parent class info */
	_PerCellParticleLayout_Print( self, stream );
	
	/* RandomParticleLayout */
	Journal_Printf( randomParticleLayoutStream, "\tcellParticleCount: %u\n", self->cellParticleCount );
	Journal_Printf( randomParticleLayoutStream, "\tseed: %u\n", self->seed );
}


void* _RandomParticleLayout_Copy( void* randomParticleLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	RandomParticleLayout*		self = (RandomParticleLayout*)randomParticleLayout;
	RandomParticleLayout*		newRandomParticleLayout;
	
	newRandomParticleLayout = (RandomParticleLayout*)_PerCellParticleLayout_Copy( self, dest, deep, nameExt, ptrMap );
	
	newRandomParticleLayout->cellParticleCount = self->cellParticleCount;
	newRandomParticleLayout->seed = self->seed;
	
	return (void*)newRandomParticleLayout;
}


void* _RandomParticleLayout_DefaultNew( Name name ) {
	return (void*)_RandomParticleLayout_New( 
			sizeof(RandomParticleLayout),
			RandomParticleLayout_Type,
			_RandomParticleLayout_Delete,
			_RandomParticleLayout_Print, 
			_RandomParticleLayout_Copy,
			_RandomParticleLayout_DefaultNew,
			_RandomParticleLayout_Construct,
			_RandomParticleLayout_Build,
			_RandomParticleLayout_Initialise,
			_RandomParticleLayout_Execute,
			_RandomParticleLayout_Destroy,
			_PerCellParticleLayout_SetInitialCounts,
			_PerCellParticleLayout_InitialiseParticles,
			_RandomParticleLayout_InitialCount,
			_RandomParticleLayout_InitialiseParticlesOfCell, 
			name,
			False,
			0, /* cellParticleCount */
			0  /* seed */ );
}

void _RandomParticleLayout_Construct( void* randomParticleLayout, Stg_ComponentFactory* cf, void* data ) {
	RandomParticleLayout*       self = (RandomParticleLayout*)randomParticleLayout;
	Particle_InCellIndex        cellParticleCount;
	unsigned int                seed;

	cellParticleCount = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, "cellParticleCount", 0 );
	seed = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, "seed", 13 );
	
	_RandomParticleLayout_Init( self, cellParticleCount, seed );
}
	
void _RandomParticleLayout_Build( void* randomParticleLayout, void* data ) {
	
}
	
void _RandomParticleLayout_Initialise( void* randomParticleLayout, void* data ) {
	
}
	
void _RandomParticleLayout_Execute( void* randomParticleLayout, void* data ) {
	
}
	
void _RandomParticleLayout_Destroy( void* randomParticleLayout, void* data ) {
	
}


Particle_InCellIndex _RandomParticleLayout_InitialCount( void* randomParticleLayout, void* celllayout, Cell_Index cell_I ) {
	RandomParticleLayout* self = (RandomParticleLayout*)randomParticleLayout;
	return self->cellParticleCount;
}

void _RandomParticleLayout_InitialiseParticlesOfCell( void* randomParticleLayout, void* _swarm, Cell_Index cell_I ) {
	Swarm*              	swarm = (Swarm*)_swarm;
	Coord               	min;
	Coord               	max;
	Dimension_Index     	dim_I;
	Particle_InCellIndex	particlesThisCell = swarm->cellParticleCountTbl[cell_I];
	Particle_InCellIndex	cParticle_I = 0;
	GlobalParticle*	        particle = NULL;
	
	Swarm_GetCellMinMaxCoords( swarm, cell_I, min, max );

	for ( cParticle_I = 0; cParticle_I < particlesThisCell; cParticle_I++ ) {	
		
		particle = (GlobalParticle*)Swarm_ParticleInCellAt( swarm, cell_I, cParticle_I );
		particle->owningCell = cell_I;
		
		for ( dim_I = 0 ; dim_I < 3 ; dim_I++ ) {
			particle->coord[ dim_I ] = Swarm_Random_Random_WithMinMax( min[ dim_I ], max[ dim_I ] );
		}
	}
}


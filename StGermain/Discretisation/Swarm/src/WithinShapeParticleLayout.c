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
** $Id: WithinShapeParticleLayout.c 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <Base/Base.h>
#include <Discretisation/Geometry/Geometry.h>
#include <Discretisation/Shape/Shape.h>
#include <Discretisation/Mesh/Mesh.h>
#include <Discretisation/Utils/Utils.h>

#include "types.h"
#include "shortcuts.h"
#include "ParticleLayout.h"
#include "GlobalParticleLayout.h"
#include "SpaceFillerParticleLayout.h"
#include "WithinShapeParticleLayout.h"
#include "CellLayout.h"
#include "SwarmClass.h"
#include "StandardParticle.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const Type WithinShapeParticleLayout_Type = "WithinShapeParticleLayout";

const Index WithinShapeParticleLayout_Invalid = (Index) 0;

WithinShapeParticleLayout* WithinShapeParticleLayout_New(
		Name                    name,
		Dimension_Index         dim,
		Particle_Index          totalInitialParticles,
		Stg_Shape*              shape )
{
	WithinShapeParticleLayout* self = (WithinShapeParticleLayout*) _WithinShapeParticleLayout_DefaultNew( name );
	_WithinShapeParticleLayout_Init( self, dim, totalInitialParticles, 0.0, shape );

	return self;
}

WithinShapeParticleLayout* _WithinShapeParticleLayout_New( 
		SizeT                                            _sizeOfSelf,
		Type                                             type,
		Stg_Class_DeleteFunction*                        _delete,
		Stg_Class_PrintFunction*                         _print,
		Stg_Class_CopyFunction*                          _copy,
		Stg_Component_DefaultConstructorFunction*        _defaultConstructor,
		Stg_Component_ConstructFunction*                 _construct,
		Stg_Component_BuildFunction*                     _build,
		Stg_Component_InitialiseFunction*                _initialise,
		Stg_Component_ExecuteFunction*                   _execute,
		Stg_Component_DestroyFunction*                   _destroy,
		ParticleLayout_SetInitialCountsFunction*         _setInitialCounts,
		ParticleLayout_InitialiseParticlesFunction*      _initialiseParticles,
		GlobalParticleLayout_InitialiseParticleFunction* _initialiseParticle,
		Name                                             name,
		Bool                                             initFlag,
		Dimension_Index                                  dim,
		Particle_Index                                   totalInitialParticles,
		double                                           averageInitialParticlesPerCell,
		Stg_Shape*                                       shape )
{
	WithinShapeParticleLayout* self;
	
	/* Allocate memory */
	self = (WithinShapeParticleLayout*)_SpaceFillerParticleLayout_New( 
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
		_initialiseParticle,
		name,
		initFlag,
		dim,
		totalInitialParticles,
		averageInitialParticlesPerCell );

	if ( initFlag ) {
		_WithinShapeParticleLayout_Init( self, dim, totalInitialParticles, averageInitialParticlesPerCell, shape );
	}

	return self;
}

void _WithinShapeParticleLayout_Init(
		void*                   withinShapeParticleLayout,
		Dimension_Index         dim,
		Particle_Index          totalInitialParticles,
		double                  averageInitialParticlesPerCell,
		Stg_Shape*              shape )
{
	WithinShapeParticleLayout*	self = (WithinShapeParticleLayout*) withinShapeParticleLayout;

	self->isConstructed = True;
	self->shape         = shape;

	_SpaceFillerParticleLayout_Init( self, dim, totalInitialParticles, averageInitialParticlesPerCell );
}



	
void _WithinShapeParticleLayout_Delete( void* withinShapeParticleLayout ) {
	WithinShapeParticleLayout* self = (WithinShapeParticleLayout*)withinShapeParticleLayout;

	/* Stg_Class_Delete parent class */
	_SpaceFillerParticleLayout_Delete( self );

}

void _WithinShapeParticleLayout_Print( void* withinShapeParticleLayout, Stream* stream ) {
	WithinShapeParticleLayout* self  = (WithinShapeParticleLayout*)withinShapeParticleLayout;
	
	/* General info */
	Journal_Printf( stream, "WithinShapeParticleLayout (ptr): %p:\n", self );
	Stream_Indent( stream );
	
	/* Parent class info */
	_SpaceFillerParticleLayout_Print( self, stream );
	
	/* WithinShapeParticleLayout */
	Print( self->shape, stream );
	
	Stream_UnIndent( stream );
}


void* _WithinShapeParticleLayout_Copy( void* withinShapeParticleLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	WithinShapeParticleLayout*		self                    = (WithinShapeParticleLayout*)withinShapeParticleLayout;
	WithinShapeParticleLayout*		newWithinShapeParticleLayout;
	
	newWithinShapeParticleLayout = _SpaceFillerParticleLayout_Copy( self, dest, deep, nameExt, ptrMap );
	
	if ( deep ) {
		newWithinShapeParticleLayout->shape = (Stg_Shape*)Stg_Class_Copy( self->shape, NULL, deep, nameExt, ptrMap );
	}
	else {
		newWithinShapeParticleLayout->shape = self->shape;
	}

	return (void*)newWithinShapeParticleLayout;
}

void* _WithinShapeParticleLayout_DefaultNew( Name name ) {
	return (void*)_WithinShapeParticleLayout_New( 
			sizeof(WithinShapeParticleLayout),
			WithinShapeParticleLayout_Type,
			_WithinShapeParticleLayout_Delete,
			_WithinShapeParticleLayout_Print,
			_WithinShapeParticleLayout_Copy,
			_WithinShapeParticleLayout_DefaultNew,
			_WithinShapeParticleLayout_Construct,
			_WithinShapeParticleLayout_Build,
			_WithinShapeParticleLayout_Initialise,
			_WithinShapeParticleLayout_Execute,
			_WithinShapeParticleLayout_Destroy,
			_GlobalParticleLayout_SetInitialCounts,
			_WithinShapeParticleLayout_InitialiseParticles,
			_SpaceFillerParticleLayout_InitialiseParticle,
			name,
			False,
			0,   /* dim */
			0,   /* totalInitialParticles */
			0.0, /* averageInitialParticlesPerCell */
			NULL /* shape */ );
}

void _WithinShapeParticleLayout_Construct( void* withinShapeParticleLayout, Stg_ComponentFactory *cf, void* data ) {
	WithinShapeParticleLayout* self = (WithinShapeParticleLayout*) withinShapeParticleLayout;
	Stg_Shape*      shape;
	
	_SpaceFillerParticleLayout_Construct( self, cf, data );

	shape = Stg_ComponentFactory_ConstructByKey(  cf,  self->name,  "shape", Stg_Shape,  True, data ) ;

	_WithinShapeParticleLayout_Init( 
			self, 
			self->dim, 
			self->totalInitialParticles, 
			self->averageInitialParticlesPerCell, 
			shape );
}
	
void _WithinShapeParticleLayout_Build( void* withinShapeParticleLayout, void* data ) {
}
void _WithinShapeParticleLayout_Initialise( void* withinShapeParticleLayout, void* data ) {
}
void _WithinShapeParticleLayout_Execute( void* withinShapeParticleLayout, void* data ) {
}
void _WithinShapeParticleLayout_Destroy( void* withinShapeParticleLayout, void* data ) {
}

void _WithinShapeParticleLayout_InitialiseParticles( void* particleLayout, void* _swarm )
{
	WithinShapeParticleLayout*	self          = (WithinShapeParticleLayout*)particleLayout;
	Swarm*			        swarm         = (Swarm*)_swarm;
	GlobalParticle*                 particle      = NULL;
	Particle_Index		        lParticle_I   = 0;
	Particle_Index		        newParticle_I = 0;
	Cell_Index		        cell_I;
	
	/* Go through and init particles */
	while( newParticle_I < self->totalInitialParticles ) {
		
		particle = (GlobalParticle*)Swarm_ParticleAt( swarm, lParticle_I );

		GlobalParticleLayout_InitialiseParticle( self, swarm, newParticle_I, particle );

		/* Test the particle is inside our desired shape */
		if ( Stg_Shape_IsCoordInside( self->shape, particle->coord ) ) {
			
			newParticle_I++;
			/* Work out which cell the new particle is in */
			/* First specify the particle doesn't have an owning cell yet, so as
			not to confuse the search algorithm */
			particle->owningCell = swarm->cellDomainCount;
			cell_I = CellLayout_CellOf( swarm->cellLayout, particle );

			/* If we found a further particle inside our domain add it to a cell */
			if ( cell_I < swarm->cellLocalCount ) {
				/* Add it to that cell */
				Swarm_AddParticleToCell( swarm, cell_I, lParticle_I );
				lParticle_I++;
				swarm->particleLocalCount++;
				Swarm_Realloc( swarm );
			}
		}
	}
}

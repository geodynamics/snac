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
** $Id: UnionParticleLayout.c 3851 2006-10-12 08:57:22Z SteveQuenette $
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
#include "UnionParticleLayout.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const Type UnionParticleLayout_Type = "UnionParticleLayout";

UnionParticleLayout* UnionParticleLayout_New(
		Name                                             name, 
		GlobalParticleLayout**                           particleLayoutList,
		Index                                            particleLayoutCount )
{
	UnionParticleLayout* self = (UnionParticleLayout*) _UnionParticleLayout_DefaultNew( name );
	_UnionParticleLayout_Init( self, particleLayoutList, particleLayoutCount );
	return self;
}

UnionParticleLayout* _UnionParticleLayout_New( 
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
		GlobalParticleLayout**                           particleLayoutList,
		Index                                            particleLayoutCount )
{
	UnionParticleLayout* self;
	
	/* Allocate memory */
	self = (UnionParticleLayout*)_GlobalParticleLayout_New( 
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
		GlobalCoordSystem,
		False,
		0,
		0 );

	if ( initFlag ) {
		_UnionParticleLayout_Init( self, particleLayoutList, particleLayoutCount );
	}

	return self;
}

void _UnionParticleLayout_Init( 
		void*                                            unionParticleLayout, 
		GlobalParticleLayout**                           particleLayoutList,
		Index                                            particleLayoutCount )
{
	UnionParticleLayout*	self = (UnionParticleLayout*) unionParticleLayout;
	self->isConstructed = True;
	
	self->particleLayoutList = particleLayoutList;
	self->particleLayoutCount = particleLayoutCount;

	_GlobalParticleLayout_Init( self, GlobalCoordSystem, False, 0, 0 );
}


	
void _UnionParticleLayout_Delete( void* unionParticleLayout ) {
	UnionParticleLayout* self = (UnionParticleLayout*)unionParticleLayout;

	_GlobalParticleLayout_Delete( self );
}

void _UnionParticleLayout_Print( void* unionParticleLayout, Stream* stream ) {
	UnionParticleLayout* self  = (UnionParticleLayout*)unionParticleLayout;
	
	/* General info */
	Journal_Printf( stream, "UnionParticleLayout (ptr): %p:\n", self );
	Stream_Indent( stream );
	
	/* Parent class info */
	_GlobalParticleLayout_Print( self, stream );
	
	/* UnionParticleLayout */
	
	Stream_UnIndent( stream );
}

void* _UnionParticleLayout_Copy( void* unionParticleLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	UnionParticleLayout*		self                    = (UnionParticleLayout*)unionParticleLayout;
	UnionParticleLayout*		newUnionParticleLayout;
	
	newUnionParticleLayout = (UnionParticleLayout*)_GlobalParticleLayout_Copy( self, dest, deep, nameExt, ptrMap );
	
	return (void*)newUnionParticleLayout;
}

void* _UnionParticleLayout_DefaultNew( Name name ) {
	return (void*)_UnionParticleLayout_New( 
			sizeof(UnionParticleLayout),
			UnionParticleLayout_Type,
			_UnionParticleLayout_Delete,
			_UnionParticleLayout_Print,
			_UnionParticleLayout_Copy,
			_UnionParticleLayout_DefaultNew,
			_UnionParticleLayout_Construct,
			_UnionParticleLayout_Build,
			_UnionParticleLayout_Initialise,
			_UnionParticleLayout_Execute,
			_UnionParticleLayout_Destroy,
			_UnionParticleLayout_SetInitialCounts,
			_UnionParticleLayout_InitialiseParticles,
			_UnionParticleLayout_InitialiseParticle,
			name,
			False,
			NULL,
			0 );
}


void _UnionParticleLayout_Construct( void* unionParticleLayout, Stg_ComponentFactory *cf, void* data ) {
	UnionParticleLayout*   self = (UnionParticleLayout*) unionParticleLayout;
	GlobalParticleLayout** particleLayoutList;
	Index                  particleLayoutCount;

	particleLayoutList = Stg_ComponentFactory_ConstructByList( 
		cf, 
		self->name, 
		"ParticleLayoutList", 
		Stg_ComponentFactory_Unlimited, 
		GlobalParticleLayout, 
		True, 
		&particleLayoutCount,
		data );

	_UnionParticleLayout_Init( self, particleLayoutList, particleLayoutCount );
}
	
void _UnionParticleLayout_Build( void* unionParticleLayout, void* data ) {
}
void _UnionParticleLayout_Initialise( void* unionParticleLayout, void* data ) {
}	
void _UnionParticleLayout_Execute( void* unionParticleLayout, void* data ) {	
}
void _UnionParticleLayout_Destroy( void* unionParticleLayout, void* data ) {	
}

void _UnionParticleLayout_SetInitialCounts( void* unionParticleLayout, void* _swarm ) {
	UnionParticleLayout*        self     = (UnionParticleLayout*)unionParticleLayout;
	Swarm*                      swarm    = (Swarm*)_swarm;
	Index                       layout_I;

	/* Get total number of particles from total number of layouts in the list */
	self->totalInitialParticles = 0 ;
	for ( layout_I = 0 ; layout_I < self->particleLayoutCount ; layout_I++ ) {
		ParticleLayout_SetInitialCounts( self->particleLayoutList[ layout_I ], swarm );
		self->totalInitialParticles += self->particleLayoutList[ layout_I ]->totalInitialParticles;
	}

	/* Call parent function */
	_GlobalParticleLayout_SetInitialCounts( self, swarm );
}
void _UnionParticleLayout_InitialiseParticles( void* unionParticleLayout, void* swarm ) {
	UnionParticleLayout*        self     = (UnionParticleLayout*)unionParticleLayout;

	/* Call parent function */
	_GlobalParticleLayout_InitialiseParticles( self, swarm );
}

void _UnionParticleLayout_InitialiseParticle( 
		void*             unionParticleLayout, 
		void*             swarm, 
		Particle_Index    newParticle_I,
		void*             particle )
{
	UnionParticleLayout*        self     = (UnionParticleLayout*)unionParticleLayout;
	Index                       layout_I;

	/* Loop over particle layouts until you find which layout should initialise this particle 
	 * When one particle layout finishes initialises all its particles, then the next one begins. */
	for ( layout_I = 0 ; layout_I < self->particleLayoutCount ; layout_I++ ) {
		/* See if this particle belongs to this particle layout */
		if ( newParticle_I < self->particleLayoutList[ layout_I ]->totalInitialParticles ) {
			GlobalParticleLayout_InitialiseParticle( self->particleLayoutList[ layout_I ], swarm, newParticle_I, particle );
			return;
		}

		/* Decrement particle number */
		newParticle_I -= self->particleLayoutList[ layout_I ]->totalInitialParticles;
	}
}


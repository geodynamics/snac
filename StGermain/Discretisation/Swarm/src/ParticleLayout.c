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
** $Id: ParticleLayout.c 3629 2006-06-13 08:36:51Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"

#include "types.h"
#include "shortcuts.h"
#include "ParticleLayout.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

const Type ParticleLayout_Type = "ParticleLayout";

ParticleLayout* _ParticleLayout_New( 
                SizeT                                       _sizeOfSelf,
                Type                                        type,
                Stg_Class_DeleteFunction*                   _delete,
                Stg_Class_PrintFunction*                    _print,
                Stg_Class_CopyFunction*                     _copy,
                Stg_Component_DefaultConstructorFunction*   _defaultConstructor,
                Stg_Component_ConstructFunction*            _construct,
                Stg_Component_BuildFunction*                _build,
                Stg_Component_InitialiseFunction*           _initialise,
                Stg_Component_ExecuteFunction*              _execute,
                Stg_Component_DestroyFunction*              _destroy,
                ParticleLayout_SetInitialCountsFunction*    _setInitialCounts,
                ParticleLayout_InitialiseParticlesFunction* _initialiseParticles,
                Name                                        name,
                Bool                                        initFlag,
		CoordSystem                                 coordSystem,
                Bool                                        weightsInitialisedAtStartup )
{
	ParticleLayout*		self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(ParticleLayout) );
	self = (ParticleLayout*)_Stg_Component_New( 
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
	
	/* General info */
	
	/* Virtual functions */
	self->_setInitialCounts = _setInitialCounts;
	self->_initialiseParticles = _initialiseParticles;
	
	if( initFlag ){
		_ParticleLayout_Init( self, coordSystem, weightsInitialisedAtStartup );
	}
	
	return self;
}


void _ParticleLayout_Init( 
		void*           particleLayout, 
		CoordSystem     coordSystem,
		Bool            weightsInitialisedAtStartup ) 
{ 
	ParticleLayout* self = (ParticleLayout*)particleLayout;
	self->isConstructed  = True;

	self->debug = Stream_RegisterChild( Swarm_Debug, self->type );

	self->coordSystem = coordSystem;
	self->weightsInitialisedAtStartup = weightsInitialisedAtStartup;
}

void _ParticleLayout_Delete( void* particleLayout ) {
	ParticleLayout* self = (ParticleLayout*)particleLayout;
	
	/* Stg_Class_Delete parent class */
	_Stg_Component_Delete( self );
}

void _ParticleLayout_Print( void* particleLayout, Stream* stream ) {
	ParticleLayout* self = (ParticleLayout*)particleLayout;
	
	Journal_Printf( stream, "ParticleLayout (ptr): %p\n", self );
	
	/* Parent class info */
	_Stg_Class_Print( self, stream );
	
	/* Virtual info */
	Journal_Printf( stream, "\t_setInitialCounts (func ptr): %p\n", self->_setInitialCounts );
	Journal_Printf( stream, "\t_initialiseParticles (func ptr): %p\n", self->_initialiseParticles );
	
	/* ParticleLayout info */
	Journal_Printf( 
		stream, 
		"\tcoordSystem: %s\n", 
		( self->coordSystem == LocalCoordSystem ) ? "LocalCoordSystem" : "GlobalCoordSystem" );
	Journal_Printf( 
		stream, 
		"\tweightsInitialisedAtStartup : %s\n",
		self->weightsInitialisedAtStartup ? "True" : "False" );
}


void* _ParticleLayout_Copy( void* particleLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	ParticleLayout*		self = (ParticleLayout*)particleLayout;
	ParticleLayout*		newParticleLayout;
	
	newParticleLayout = _Stg_Class_Copy( self, dest, deep, nameExt, ptrMap );
	
	/* Virtual methods */
	newParticleLayout->_setInitialCounts = self->_setInitialCounts;
	newParticleLayout->_initialiseParticles = self->_initialiseParticles;
	newParticleLayout->coordSystem = self->coordSystem;
	newParticleLayout->weightsInitialisedAtStartup = self->weightsInitialisedAtStartup;

	return (void*)newParticleLayout;
}


void ParticleLayout_InitialiseParticles( void* particleLayout, void* swarm ) {
	ParticleLayout* self = (ParticleLayout*)particleLayout;
	
	self->_initialiseParticles( self, swarm );
}

void ParticleLayout_SetInitialCounts( void* particleLayout, void* swarm ) {
	ParticleLayout* self = (ParticleLayout*)particleLayout;
	
	self->_setInitialCounts( particleLayout, swarm );
}

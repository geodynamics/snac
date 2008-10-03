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
** $Id: PlaneParticleLayout.c 3851 2006-10-12 08:57:22Z SteveQuenette $
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
#include "PlaneParticleLayout.h"
#include "CellLayout.h"
#include "SwarmClass.h"
#include "StandardParticle.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

const Type PlaneParticleLayout_Type = "PlaneParticleLayout";

const Index PlaneParticleLayout_Invalid = (Index) 0;

PlaneParticleLayout* PlaneParticleLayout_New( 
		Name name,
		Dimension_Index dim,
		Index totalInitialParticles, 
		Axis planeAxis, 
		double planeCoord )
{
	PlaneParticleLayout* self = (PlaneParticleLayout*) _PlaneParticleLayout_DefaultNew( name );

	_PlaneParticleLayout_Init( self, dim, totalInitialParticles, planeAxis, planeCoord );
	
	return self;
}

PlaneParticleLayout* _PlaneParticleLayout_New( 
		SizeT                                                   _sizeOfSelf,
		Type                                                    type,
		Stg_Class_DeleteFunction*                               _delete,
		Stg_Class_PrintFunction*                                _print,
		Stg_Class_CopyFunction*                                 _copy, 
		Stg_Component_DefaultConstructorFunction*               _defaultConstructor,
		Stg_Component_ConstructFunction*                        _construct,
		Stg_Component_BuildFunction*                            _build,
		Stg_Component_InitialiseFunction*                       _initialise,
		Stg_Component_ExecuteFunction*                          _execute,
		Stg_Component_DestroyFunction*                          _destroy,
		ParticleLayout_SetInitialCountsFunction*                _setInitialCounts,
		ParticleLayout_InitialiseParticlesFunction*             _initialiseParticles,
		GlobalParticleLayout_InitialiseParticleFunction*        _initialiseParticle,
		Name                                                    name,
		Bool                                                    initFlag,
		Dimension_Index                                         dim,
		Particle_Index                                          totalInitialParticles,
		double                                                  averageInitialParticlesPerCell,
		Axis                                                    planeAxis, 
		double                                                  planeCoord )
{
	PlaneParticleLayout* self;
	
	/* Allocate memory */
	self = (PlaneParticleLayout*)_SpaceFillerParticleLayout_New( 
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
		_PlaneParticleLayout_Init( self, dim, totalInitialParticles, planeAxis, planeCoord );
	}
	
	return self;
}

void _PlaneParticleLayout_Init( 
		void*           particleLayout, 
		Dimension_Index dim, 
		Particle_Index  totalInitialParticles, 
		Axis            planeAxis, 
		double          planeCoord )
{
	PlaneParticleLayout* self = (PlaneParticleLayout*) particleLayout;

	_SpaceFillerParticleLayout_Init( self, dim, totalInitialParticles, SpaceFillerParticleLayout_Invalid );

	self->planeAxis  = planeAxis;
	self->planeCoord = planeCoord;
}


void* _PlaneParticleLayout_DefaultNew( Name name ) {
	return (void*)_PlaneParticleLayout_New( 
			sizeof(PlaneParticleLayout),
			PlaneParticleLayout_Type,
			_PlaneParticleLayout_Delete,
			_PlaneParticleLayout_Print,
			_PlaneParticleLayout_Copy,
			_PlaneParticleLayout_DefaultNew,
			_PlaneParticleLayout_Construct,
			_PlaneParticleLayout_Build,
			_PlaneParticleLayout_Initialise,
			_PlaneParticleLayout_Execute,
			_PlaneParticleLayout_Destroy,
			_GlobalParticleLayout_SetInitialCounts,
			_SpaceFillerParticleLayout_InitialiseParticles,
			_PlaneParticleLayout_InitialiseParticle,
			name,
			False,
			0, /* dim */
			0, /* totalInitialParticles */
			0.0, /* averageInitialParticlesPerCell */
			0, /* planeAxis */
			0.0 /* planeCoord */ );
}

	
void _PlaneParticleLayout_Destroy( void* particleLayout, void* data ){
	
}

void _PlaneParticleLayout_Delete( void* particleLayout ) {
	PlaneParticleLayout* self = (PlaneParticleLayout*)particleLayout;

	/* Stg_Class_Delete parent class */
	_SpaceFillerParticleLayout_Delete( self );

}

void _PlaneParticleLayout_Print( void* particleLayout, Stream* stream ) {
	PlaneParticleLayout* self  = (PlaneParticleLayout*)particleLayout;
	
	/* General info */
	Journal_Printf( stream, "PlaneParticleLayout (ptr): %p:\n", self );
	Stream_Indent( stream );
	
	/* Parent class info */
	_SpaceFillerParticleLayout_Print( self, stream );
	
	/* PlaneParticleLayout */
	Journal_PrintValue( stream, self->planeAxis );
	Journal_PrintValue( stream, self->planeCoord );
	
	Stream_UnIndent( stream );
}


void* _PlaneParticleLayout_Copy( void* particleLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	PlaneParticleLayout*		self                    = (PlaneParticleLayout*)particleLayout;
	PlaneParticleLayout*		newPlaneParticleLayout;
	
	newPlaneParticleLayout = _SpaceFillerParticleLayout_Copy( self, dest, deep, nameExt, ptrMap );
	
	newPlaneParticleLayout->planeAxis  = self->planeAxis;
	newPlaneParticleLayout->planeCoord = self->planeCoord;

	return (void*)newPlaneParticleLayout;
}


void _PlaneParticleLayout_Construct( void* particleLayout, Stg_ComponentFactory *cf, void* data ){
	PlaneParticleLayout* self = (PlaneParticleLayout*) particleLayout;
	Axis   planeAxis;
	double planeCoord;
	char*  planeAxisString;
	
	_SpaceFillerParticleLayout_Construct( self, cf, data );

	planeAxisString = Stg_ComponentFactory_GetString( cf, self->name, "planeAxis", "" );
	planeCoord = Stg_ComponentFactory_GetDouble( cf, self->name, "planeCoord", 0.0 );

	/* Check to make sure that some value is given for plane axis */
	Journal_Firewall( strlen( planeAxisString ) > 0, Journal_MyStream( Error_Type, self ),
		"Error for %s '%s': No axis given in param 'planeAxis'.\n", self->type, self->name );

	/* Make axis case insensitive */
	planeAxisString[0] = toupper(planeAxisString[0]);
	Journal_Firewall( planeAxisString[0] >= 'X' && planeAxisString[0] <= 'Z', Journal_MyStream( Error_Type, self ),
		"Error for %s '%s': Incorrect axis '%c' given for param 'planeAxis'.\n", self->type, self->name,planeAxisString[0]);

	planeAxis = planeAxisString[0] - 'X';

	_PlaneParticleLayout_Init( self, self->dim, self->totalInitialParticles, planeAxis, planeCoord );
	
}
	
void _PlaneParticleLayout_Build( void* particleLayout, void* data ){
	
}
	
void _PlaneParticleLayout_Initialise( void* particleLayout, void* data ){
}
	
void _PlaneParticleLayout_Execute( void* particleLayout, void* data ){
	
}

void _PlaneParticleLayout_InitialiseParticle( 
		void* spaceFillerParticleLayout, 
		void* _swarm, 
		Particle_Index newParticle_I,
		void* _particle )
{
	PlaneParticleLayout*        self     = (PlaneParticleLayout*)spaceFillerParticleLayout;
	Swarm*                      swarm    = (Swarm*)_swarm;
	GlobalParticle*             particle = (GlobalParticle*)_particle;

	_SpaceFillerParticleLayout_InitialiseParticle( self, swarm, newParticle_I, particle );
	particle->coord[ self->planeAxis ] = self->planeCoord;
}


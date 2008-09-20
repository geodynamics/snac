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
** $Id: TriGaussParticleLayout.c 3851 2006-10-12 08:57:22Z SteveQuenette $
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
#include "TriGaussParticleLayout.h"

#include "SwarmClass.h"
#include "StandardParticle.h"
#include "IntegrationPoint.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>


const Type TriGaussParticleLayout_Type = "TriGaussParticleLayout";

TriGaussParticleLayout* TriGaussParticleLayout_New( Name name, unsigned int dim, unsigned int particlesPerCell ) {
	TriGaussParticleLayout* self = (TriGaussParticleLayout*)_TriGaussParticleLayout_DefaultNew( name );

	_TriGaussParticleLayout_Init( self, dim, particlesPerCell );

	return self;
}

TriGaussParticleLayout* _TriGaussParticleLayout_New( 
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
                unsigned int                                                dim,
                unsigned int                                                particlesPerCell )
{
	TriGaussParticleLayout* self;
	
	/* Allocate memory */
	self = (TriGaussParticleLayout*)_PerCellParticleLayout_New( 
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
		LocalCoordSystem,
		True );
	
	if( initFlag ){
		_TriGaussParticleLayout_Init( self, dim, particlesPerCell );
	}
	
	return self;
}


void _TriGaussParticleLayout_Init(
		TriGaussParticleLayout* self,
		unsigned int            dim,
		unsigned int            particlesPerCell )
{
	self->isConstructed    = True;
	self->dim              = dim;
	self->particlesPerCell = particlesPerCell;
	
	_PerCellParticleLayout_Init( self, LocalCoordSystem, True );
}

void _TriGaussParticleLayout_Delete( void* triGaussParticleLayout ) {
	TriGaussParticleLayout* self = (TriGaussParticleLayout*)triGaussParticleLayout;
	
	_PerCellParticleLayout_Delete( self );
}

void _TriGaussParticleLayout_Print( void* triGaussParticleLayout, Stream* stream ) {
	TriGaussParticleLayout* self = (TriGaussParticleLayout*)triGaussParticleLayout;
	
	/* Set the Journal for printing informations */
	Stream* triGaussParticleLayoutStream = stream;
	
	/* General info */
	Journal_Printf( triGaussParticleLayoutStream, "TriGaussParticleLayout (ptr): %p:\n", self );
	
	/* Parent class info */
	_PerCellParticleLayout_Print( self, stream );
	
	/* Virtual info */
	
	/* TriGaussParticleLayout */
	Journal_Printf( triGaussParticleLayoutStream, "\tdim: %u\n", self->dim );
	Journal_Printf( triGaussParticleLayoutStream, "\tparticlesPerCell: %u\n", self->particlesPerCell  );
}


void* _TriGaussParticleLayout_Copy( void* triGaussParticleLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	TriGaussParticleLayout*		self = (TriGaussParticleLayout*)triGaussParticleLayout;
	TriGaussParticleLayout*		newTriGaussParticleLayout;
	
	newTriGaussParticleLayout = (TriGaussParticleLayout*)_PerCellParticleLayout_Copy( self, dest, deep, nameExt, ptrMap );
	
	return (void*)newTriGaussParticleLayout;
}

void* _TriGaussParticleLayout_DefaultNew( Name name ) {
	return _TriGaussParticleLayout_New(
			sizeof(TriGaussParticleLayout),
			TriGaussParticleLayout_Type,
			_TriGaussParticleLayout_Delete,
			_TriGaussParticleLayout_Print,
			_TriGaussParticleLayout_Copy,
			_TriGaussParticleLayout_DefaultNew,
			_TriGaussParticleLayout_Construct,
			_TriGaussParticleLayout_Build,
			_TriGaussParticleLayout_Initialise,
			_TriGaussParticleLayout_Execute,
			_TriGaussParticleLayout_Destroy,
			_PerCellParticleLayout_SetInitialCounts,
			_PerCellParticleLayout_InitialiseParticles,
			_TriGaussParticleLayout_InitialCount,
			_TriGaussParticleLayout_InitialiseParticlesOfCell,
			name,
			False,
			0,
			0 );
}

void _TriGaussParticleLayout_Construct( void* triGaussParticleLayout, Stg_ComponentFactory* cf, void* data ){
	TriGaussParticleLayout *self = (TriGaussParticleLayout*)triGaussParticleLayout;
	unsigned int dim;
	unsigned int particlesPerCell;

	dim = Dictionary_Entry_Value_AsUnsignedInt(
		Dictionary_GetDefault( cf->rootDict, "dim", Dictionary_Entry_Value_FromUnsignedInt( 3 ) ) );

	particlesPerCell = Dictionary_Entry_Value_AsUnsignedInt(
		Dictionary_GetDefault( cf->rootDict, "particlesPerCell", Dictionary_Entry_Value_FromUnsignedInt( 1 ) ) );

	_TriGaussParticleLayout_Init( self, dim, particlesPerCell );
}
	
void _TriGaussParticleLayout_Build( void* triGaussParticleLayout, void* data ) {
}
	
void _TriGaussParticleLayout_Initialise( void* triGaussParticleLayout, void* data ) {
}
	
void _TriGaussParticleLayout_Execute( void* triGaussParticleLayout, void* data ) {
}
	
void _TriGaussParticleLayout_Destroy( void* triGaussParticleLayout, void* data ) {
}

Particle_InCellIndex _TriGaussParticleLayout_InitialCount( void* triGaussParticleLayout, void* celllayout, Cell_Index cell_I )
{
	TriGaussParticleLayout* self = (TriGaussParticleLayout*)triGaussParticleLayout;
	Particle_InCellIndex count;
	int dim;	
	
	dim = self->dim;
	count = (Particle_InCellIndex)( self->particlesPerCell );
	
	return count;
}


/* remember this only has to initialise one cell of particles at a time */
void _TriGaussParticleLayout_InitialiseParticlesOfCell( void* triGaussParticleLayout, void* _swarm, Cell_Index cell_I )
{
	#define MAX_DIM 3
	#define MAX_GAUSS_POINTS_2D 1
	#define MAX_GAUSS_POINTS_3D 1
	TriGaussParticleLayout*   self = (TriGaussParticleLayout*)triGaussParticleLayout;
	Swarm*                    swarm = (Swarm*)_swarm;
	IntegrationPoint*         integrationPoint = NULL;

	Particle_InCellIndex      cParticle_I = 0;

	Particle_InCellIndex ppc;
	int dim;
	static double weight[20];
	static double xi[20][3];
	static int beenHere = 0;
	int d;
	
	dim = self->dim;
	ppc = self->particlesPerCell;
	
	if( dim == 2 ) {
		if( ppc == 1 ) {
			weight[0] = 0.5;

			xi[0][0] = 0.333333333333;
			xi[0][1] = 0.333333333333;				
		}
		if( ppc > MAX_GAUSS_POINTS_2D ) {
			Journal_Firewall(
				ppc > MAX_GAUSS_POINTS_2D,
				Journal_MyStream( Error_Type, self ),
				"In %s(), error: particlesPerCell greater than implementated tabulated gauss values of %d\n",
				__func__,
				MAX_GAUSS_POINTS_2D );
		}
	}
	else if ( dim == 3 ) {
		if( ppc == 1 ) {
			weight[0] = 0.5;			

			xi[0][0] = 0.333333333333;
			xi[0][1] = 0.333333333333;				
			xi[0][2] = 0.333333333333;				
		}
		if( ppc > MAX_GAUSS_POINTS_3D ) {
			Journal_Firewall(
				ppc > MAX_GAUSS_POINTS_3D,
				Journal_MyStream( Error_Type, self ),
				"In %s(), error: particlesPerCell greater than implementated tabulated gauss values of %d\n",
				__func__,
				MAX_GAUSS_POINTS_3D );
		}
	}

	assert( ppc <= swarm->cellParticleCountTbl[cell_I] );
	
	for ( cParticle_I = 0; cParticle_I < ppc; cParticle_I++ ) {
		integrationPoint = (IntegrationPoint*)Swarm_ParticleInCellAt( swarm, cell_I, cParticle_I );
		integrationPoint->owningCell = cell_I;
			
		for( d = 0; d < dim; d++ ) {
			/*integrationPoint->coord[d] = xi[cParticle_I][d];*/
			integrationPoint->xi[d] = xi[cParticle_I][d];
		}
		
		integrationPoint->weight = weight[cParticle_I];
		
		beenHere = 1;
	}	
	
	
}

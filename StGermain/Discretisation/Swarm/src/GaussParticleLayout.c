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
** $Id: GaussParticleLayout.c 3851 2006-10-12 08:57:22Z SteveQuenette $
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
#include "GaussParticleLayout.h"

#include "SwarmClass.h"
#include "StandardParticle.h"
#include "IntegrationPoint.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

const Type GaussParticleLayout_Type = "GaussParticleLayout";

GaussParticleLayout* GaussParticleLayout_New( Name name, Dimension_Index dim, Particle_InCellIndex* particlesPerDim ) {
	GaussParticleLayout* self = _GaussParticleLayout_DefaultNew( name );

	_GaussParticleLayout_Init( self, dim, particlesPerDim );

	return self;
}

GaussParticleLayout* _GaussParticleLayout_New( 
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
		Dimension_Index                                             dim,
		Particle_InCellIndex*                                       particlesPerDim )
{
	GaussParticleLayout* self;
	
	/* Allocate memory */
	self = (GaussParticleLayout*)_PerCellParticleLayout_New( 
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
	
	if( initFlag ) {
		_GaussParticleLayout_Init( self, dim, particlesPerDim );
	}
	
	return self;
}

void _GaussParticleLayout_Init( void* gaussParticleLayout, Dimension_Index dim, Particle_InCellIndex* particlesPerDim ) {
	GaussParticleLayout* self = (GaussParticleLayout*)gaussParticleLayout;

	self->isConstructed       = True;
	self->dim                 = dim;
	memcpy( self->particlesPerDim, particlesPerDim, 3 * sizeof(Particle_InCellIndex) );

	_PerCellParticleLayout_Init( self, LocalCoordSystem, True );
}

void _GaussParticleLayout_Delete( void* gaussParticleLayout ) {
	GaussParticleLayout* self = (GaussParticleLayout*)gaussParticleLayout;
	
	_PerCellParticleLayout_Delete( self );
}

void _GaussParticleLayout_Print( void* gaussParticleLayout, Stream* stream ) {
	GaussParticleLayout* self = (GaussParticleLayout*)gaussParticleLayout;
	
	/* General info */
	Journal_Printf( stream, "GaussParticleLayout (ptr): %p:\n", self );
	
	/* Parent class info */
	_PerCellParticleLayout_Print( self, stream );
	
	/* Virtual info */
	
	/* GaussParticleLayout */
	Stream_Indent( stream );
	Journal_PrintValue( stream, self->dim );
	Journal_PrintArray( stream, self->particlesPerDim, self->dim );
	Stream_UnIndent( stream );
}


void* _GaussParticleLayout_Copy( void* gaussParticleLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	GaussParticleLayout*	self = (GaussParticleLayout*)gaussParticleLayout;
	GaussParticleLayout*	newGaussParticleLayout;
	
	newGaussParticleLayout = (GaussParticleLayout*)_PerCellParticleLayout_Copy( self, dest, deep, nameExt, ptrMap );
	
	newGaussParticleLayout->dim = self->dim;
	memcpy( newGaussParticleLayout->particlesPerDim, self->particlesPerDim, 3 * sizeof(unsigned int) );

	return (void*)newGaussParticleLayout;
}

void* _GaussParticleLayout_DefaultNew( Name name ) {
	return (GaussParticleLayout*)_GaussParticleLayout_New( 
			sizeof(GaussParticleLayout), 
			GaussParticleLayout_Type,
			_GaussParticleLayout_Delete,
			_GaussParticleLayout_Print, 
			_GaussParticleLayout_Copy, 
			_GaussParticleLayout_DefaultNew,
			_GaussParticleLayout_Construct,
			_GaussParticleLayout_Build, 
			_GaussParticleLayout_Initialise,
			_GaussParticleLayout_Execute,
			_GaussParticleLayout_Destroy, 
			_PerCellParticleLayout_SetInitialCounts,
			_PerCellParticleLayout_InitialiseParticles,
			_GaussParticleLayout_InitialCount,
			_GaussParticleLayout_InitialiseParticlesOfCell,
			name, 
			False,    
			0       /* dim */,
			NULL    /* particlesPerDim */ );
}

void _GaussParticleLayout_Construct( void* gaussParticleLayout, Stg_ComponentFactory* cf, void* data ) {
	GaussParticleLayout*   self = (GaussParticleLayout*)gaussParticleLayout;
	Particle_InCellIndex   particlesPerDim[3];
	Particle_InCellIndex   defaultVal;
	Dimension_Index        dim;

	dim = Stg_ComponentFactory_GetRootDictUnsignedInt( cf, "dim", 0 );

	defaultVal = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, "gaussParticles", 2 );

	particlesPerDim[ I_AXIS ] = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, "gaussParticlesX", defaultVal );
	particlesPerDim[ J_AXIS ] = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, "gaussParticlesY", defaultVal );
	if ( dim == 3 )
		particlesPerDim[ K_AXIS ] = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, "gaussParticlesZ", defaultVal );
	else
		particlesPerDim[ K_AXIS ] = 1;	

	_GaussParticleLayout_Init( self, dim, particlesPerDim );
}
	
void _GaussParticleLayout_Build( void* gaussParticleLayout, void* data ) {
}
	
void _GaussParticleLayout_Initialise( void* gaussParticleLayout, void* data ) {
}
	
void _GaussParticleLayout_Execute( void* gaussParticleLayout, void* data ) {
}

void _GaussParticleLayout_Destroy( void* gaussParticleLayout, void* data ) {
}

Particle_InCellIndex _GaussParticleLayout_InitialCount( void* gaussParticleLayout, void* celllayout, Cell_Index cell_I )
{
	GaussParticleLayout* self   = (GaussParticleLayout*)gaussParticleLayout;
	Particle_InCellIndex count;
	Dimension_Index      dim;	
	Dimension_Index      dim_I;
	
	dim = self->dim;
	count = 1;
	for( dim_I = 0; dim_I < dim; dim_I++ ) {
		count = count * (Particle_InCellIndex)( self->particlesPerDim[ dim_I ] );
	}
	
	return count;
	
}

#define TRIPLE_MAX( A, B, C )  MAX( MAX( (A), (B) ), (C) )

/* remember this only has to initialise one particle at a time */
void _GaussParticleLayout_InitialiseParticlesOfCell( void* gaussParticleLayout, void* _swarm, Cell_Index cell_I )
{
	GaussParticleLayout*      self                = (GaussParticleLayout*)gaussParticleLayout;
	Swarm*                    swarm               = (Swarm*)_swarm;
	IntegrationPoint*         particle            = NULL;
	Index                     index2D;
	Particle_InCellIndex      maxParticlesPerDim;
	IJK                       ijkIndex;
	Index                     index;
	Dimension_Index           dim_I;
	div_t                     divide;
	double*                   weights;
	double*                   abscissa;
	Coord                     min;
	Coord                     max;
	Particle_InCellIndex      particlesThisCell = swarm->cellParticleCountTbl[cell_I];
	Particle_InCellIndex      cParticle_I = 0;
	

	if ( 0 == strcmp( swarm->type, "MaterialPointsSwarm" ) ) {
		/* TODO: This is a special rule allowing a Gauss particle layout to be used to initialise
		global co-ords if you want to use it in a material swarm */
		self->coordSystem = GlobalCoordSystem;
	}

	Swarm_GetCellMinMaxCoords( _swarm, cell_I, min, max );

	/* Allocate Memory */
	maxParticlesPerDim = TRIPLE_MAX( self->particlesPerDim[ I_AXIS ],
		self->particlesPerDim[ J_AXIS ], self->particlesPerDim[ K_AXIS ] );

	weights   = Memory_Alloc_Array( double, maxParticlesPerDim, "gauss weights" );
	abscissa  = Memory_Alloc_Array( double, maxParticlesPerDim, "gauss abscissa" );

	for ( cParticle_I = 0; cParticle_I < particlesThisCell; cParticle_I++ ) {
		particle = (IntegrationPoint*)Swarm_ParticleInCellAt( swarm, cell_I, cParticle_I );
		particle->owningCell = cell_I;
		
		/* Find the i, j, k index of this particular particle */
		divide = div( cParticle_I, self->particlesPerDim[ I_AXIS ] * self->particlesPerDim[ J_AXIS ] );
		ijkIndex[ K_AXIS ] = divide.quot;
		index2D = divide.rem;

		divide = div( index2D, self->particlesPerDim[ I_AXIS ] );
		ijkIndex[ J_AXIS ] = divide.quot;
		ijkIndex[ I_AXIS ] = divide.rem;

		particle->weight = 1.0;
		for( dim_I = 0 ; dim_I < self->dim ; dim_I++ ) {
			index = ijkIndex[ dim_I ];
			GaussParticleLayout_GetAbscissaAndWeights1D( weights, abscissa, self->particlesPerDim[ dim_I ] );

			/* Assign particle stuff */
			/* TODO: this is a hack, since this class doesn't officially know that the MaterialPointsSwarm
			 * exists yet. However, we need some way for material swarms to use this layout, for testing
			 * purposes. In the simple system of only 1 swarm type, this component always initialised
			 * both local and global co-ords.
			 * -- PatrickSunter - 1 May 2006
			 */
			if ( 0 == strcmp( swarm->type, "MaterialPointsSwarm" ) ) {
				((GlobalParticle*)particle)->coord[ dim_I ] = 
					min[ dim_I ] +
						0.5 * ( max[ dim_I ] - min[ dim_I ] ) 
						* ( abscissa[ index ] + 1.0 );
			}
			else {
				particle->xi[ dim_I ] = abscissa[ index ];
				particle->weight *= weights[ index ];
			}	
		}
		
	}	

	Memory_Free( weights );
	Memory_Free( abscissa );
}


/* Values taken from table from:
 * Eric W. Weisstein. "Legendre-Gauss Quadrature." From MathWorld--A Wolfram Web Resource. http://mathworld.wolfram.com/Legendre-GaussQuadrature.html */
void GaussParticleLayout_GetAbscissaAndWeights1D( double* weight, double* abscissa, Index pointCount ) {
	switch ( pointCount ) {
		case 1:
			abscissa[0]  = 0.0;
			weight[0] = 2.0;
			break;
		case 2:
			abscissa[0]  = - 1.0/sqrt(3.0);
			abscissa[1]  = - abscissa[0];
		
			weight[0] = 1.0;
			weight[1] = weight[0];
			break;
		case 3:
			abscissa[0]  = - sqrt(15.0)/5.0;
			abscissa[1]  = 0.0;
			abscissa[2]  = - abscissa[0];
		
			weight[0] = 5.0/9.0;
			weight[1] = 8.0/9.0;
			weight[2] = weight[0];
			break;
		case 4:
			abscissa[0]  = - sqrt( 525.0 + 70.0 * sqrt(30.0) )/35.0;
			abscissa[1]  = - sqrt( 525.0 - 70.0 * sqrt(30.0) )/35.0;
			abscissa[2]  = - abscissa[1];
			abscissa[3]  = - abscissa[0];
			
			weight[0] = (18.0 - sqrt(30.0))/36.0;
			weight[1] = (18.0 + sqrt(30.0))/36.0;
			weight[2] = weight[1];
			weight[3] = weight[0];
			break;
		case 5:
			abscissa[0]  = - sqrt( 245.0 + 14.0 * sqrt( 70.0 ) )/21.0;
			abscissa[1]  = - sqrt( 245.0 - 14.0 * sqrt( 70.0 ) )/21.0;
			abscissa[2]  = 0.0;
			abscissa[3]  = - abscissa[1];
			abscissa[4]  = - abscissa[2];

			weight[0] = ( 322.0 - 13.0 * sqrt( 70.0 ) )/900.0;
			weight[1] = ( 322.0 + 13.0 * sqrt( 70.0 ) )/900.0;
			weight[2] = 128.0/225.0;
			weight[3] = weight[1];
			weight[4] = weight[0];
			break;
		default:
			Journal_Firewall( 
				pointCount <= 5, 
				Journal_Register( Error_Type, GaussParticleLayout_Type ),
				"In func %s: Cannot give values for '%u' gauss points\n.", 
				__func__, 
				pointCount );
	}
}

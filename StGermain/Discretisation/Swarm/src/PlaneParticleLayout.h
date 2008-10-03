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
*/
/** \file
**  Role:
**	Instantiates the ParticleLayout abstract class to a space fill of the given shape.
**
** Assumptions:
**	Cell is a right-angled cuboid.
**
** Comments:
**
** $Id: PlaneParticleLayout.h 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Swarm_PlaneParticleLayout_h__
#define __Discretisation_Swarm_PlaneParticleLayout_h__
	

	/* Textual name of this class */
	extern const Type PlaneParticleLayout_Type;
	
	extern const Index PlaneParticleLayout_Invalid;

	/* PlaneParticleLayout information */
	#define __PlaneParticleLayout \
		/* General info */ \
		__SpaceFillerParticleLayout \
		/* Virtual info */ \
		/* Other info */ \
		Axis              planeAxis;        \
		double            planeCoord;

	struct PlaneParticleLayout { __PlaneParticleLayout };
	
	/* Create a new PlaneParticleLayout and initialise */
	
	PlaneParticleLayout* PlaneParticleLayout_New( 
		Name name, 
		Dimension_Index dim, 
		Particle_Index totalInitialParticles, 
		Axis planeAxis, 
		double planeCoord );
	
	/* Creation implementation / Virtual constructor */
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
		double                                                  planeCoord );

	void _PlaneParticleLayout_Init( 
			void* particleLayout, 
			Dimension_Index dim, 
			Particle_Index totalInitialParticles, 
			Axis planeAxis, 
			double planeCoord );
	
	/* 'Stg_Class' Stuff */
	void _PlaneParticleLayout_Delete( void* particleLayout );
	void _PlaneParticleLayout_Print( void* particleLayout, Stream* stream );
	#define PlaneParticleLayout_Copy( self ) \
		(PlaneParticleLayout*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define PlaneParticleLayout_DeepCopy( self ) \
		(PlaneParticleLayout*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _PlaneParticleLayout_Copy( void* particleLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/* 'Stg_Component' Stuff */
	void* _PlaneParticleLayout_DefaultNew( Name name ) ;
	void _PlaneParticleLayout_Construct( void* particleLayout, Stg_ComponentFactory *cf, void* data );
	void _PlaneParticleLayout_Build( void* particleLayout, void* data );
	void _PlaneParticleLayout_Initialise( void* particleLayout, void* data );
	void _PlaneParticleLayout_Execute( void* particleLayout, void* data );
	void _PlaneParticleLayout_Destroy( void* particleLayout, void* data );
	
	/* Initialises the coordinates of a cell's particle */
	void _PlaneParticleLayout_InitialiseParticles( void* particleLayout, void* swarm ) ;
	void _PlaneParticleLayout_InitialiseParticle( 
			void* particleLayout, 
			void* swarm, 
			Particle_Index newParticle_I,
			void* particle );

#endif /* __StGermain_Discretisation_PlaneParticleLayout_h__ */

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
** $Id: WithinShapeParticleLayout.h 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Swarm_WithinShapeParticleLayout_h__
#define __Discretisation_Swarm_WithinShapeParticleLayout_h__
	

	/* Textual name of this class */
	extern const Type WithinShapeParticleLayout_Type;
	
	extern const Index WithinShapeParticleLayout_Invalid;

	/* WithinShapeParticleLayout information */
	#define __WithinShapeParticleLayout \
		/* General info */ \
		__SpaceFillerParticleLayout \
		/* Virtual info */ \
		/* Other info */ \
		Stg_Shape*                    shape; 

	struct WithinShapeParticleLayout { __WithinShapeParticleLayout };
	
	/* Create a new WithinShapeParticleLayout and initialise */
	
	WithinShapeParticleLayout* WithinShapeParticleLayout_New( 
			Name                    name, 
			Dimension_Index         dim, 
			Particle_Index          totalInitialParticles, 
			Stg_Shape*              shape );
	
	/* Creation implementation / Virtual constructor */
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
		Stg_Shape*                                       shape );
	
	void _WithinShapeParticleLayout_Init( 
			void*                   withinShapeParticleLayout, 
			Dimension_Index         dim, 
			Particle_Index          totalInitialParticles, 
			double                  averageInitialParticlesPerCell,
			Stg_Shape*              shape );

	/* 'Stg_Class' Stuff */
	void _WithinShapeParticleLayout_Delete( void* withinShapeParticleLayout );
	void _WithinShapeParticleLayout_Print( void* withinShapeParticleLayout, Stream* stream );
	#define WithinShapeParticleLayout_Copy( self ) \
		(WithinShapeParticleLayout*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define WithinShapeParticleLayout_DeepCopy( self ) \
		(WithinShapeParticleLayout*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _WithinShapeParticleLayout_Copy( void* withinShapeParticleLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/* 'Stg_Component' Stuff */
	void* _WithinShapeParticleLayout_DefaultNew( Name name ) ;
	void _WithinShapeParticleLayout_Construct( void* withinShapeParticleLayout, Stg_ComponentFactory *cf, void* data );
	void _WithinShapeParticleLayout_Build( void* withinShapeParticleLayout, void* data );
	void _WithinShapeParticleLayout_Initialise( void* withinShapeParticleLayout, void* data );
	void _WithinShapeParticleLayout_Execute( void* withinShapeParticleLayout, void* data );
	void _WithinShapeParticleLayout_Destroy( void* withinShapeParticleLayout, void* data );
	
	void _WithinShapeParticleLayout_InitialiseParticles( void* withinShapeParticleLayout, void* swarm ) ;
	void _WithinShapeParticleLayout_InitialiseParticle( 
			void*          withinShapeParticleLayout, 
			void*          swarm, 
			Particle_Index newParticle_I,
			void*          particle);

#endif /* __StGermain_Discretisation_Shape_WithinShapeParticleLayout_h__ */

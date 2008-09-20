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
**	Abstract class faciliting how particles are arranged. 
**	This class is for layouts that define the initial particle positions
**	globally, in relation to the entire cellSpace rather than a
**	particular cell.
**
** Assumptions:
**	Subclasses must set either totalInitialParticles, or averageInitialParticlesPerCell
**
** Comments:
**
** $Id: GlobalParticleLayout.h 3526 2006-04-13 03:59:17Z AlanLo $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Swarm_GlobalParticleLayout_h__
#define __Discretisation_Swarm_GlobalParticleLayout_h__
	
	/** @see GlobalParticleLayout_InitialiseParticle */
	typedef void (GlobalParticleLayout_InitialiseParticleFunction) ( 
			void*              particleLayout,
			void*              swarm, 
			Particle_Index     newParticle_I, 
			void*              particle );
	
	/* Textual name of this class */
	extern const Type GlobalParticleLayout_Type;
	
	/* ParticleLayout information */
	#define __GlobalParticleLayout \
		__ParticleLayout \
		/* virtual information */ \
		GlobalParticleLayout_InitialiseParticleFunction*	_initialiseParticle; \
		/* member data */ \
		Particle_Index		totalInitialParticles;          /**< Subclasses must set during init */\
		double			averageInitialParticlesPerCell; /**< Subclasses must set during init */

	struct GlobalParticleLayout { __GlobalParticleLayout };
	
	/* No "_New" and "_Init" as this is an abstract class */
	
	/* Creation implementation / Virtual constructor */
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
		double                                              averageInitialParticlesPerCell );

	void _GlobalParticleLayout_Init( 
		void *                                              particleLayout,
		CoordSystem                                         coordSystem,
		Bool                                                weightsInitialisedAtStartup,
		Particle_Index                                      totalInitialParticles,
		double                                              averageInitialParticlesPerCell );
	
	void _GlobalParticleLayout_Delete( void* particleLayout );
	
	void _GlobalParticleLayout_Print( void* particleLayout, Stream* stream );
	
	/* Copy */
	#define GlobalParticleLayout_Copy( self ) \
		(GlobalParticleLayout*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define GlobalParticleLayout_DeepCopy( self ) \
		(GlobalParticleLayout*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _GlobalParticleLayout_Copy( void* particleLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	void _GlobalParticleLayout_SetInitialCounts( void* particleLayout, void* _swarm );

	/** Initialise all particles in system, adding it to the swarm if its within the local processor's cell. */
	void _GlobalParticleLayout_InitialiseParticles( void* particleLayout, void* _swarm );
	
	/** Init given particle's position.
	 *     @param newParticle_I The global particle index (of all particles, not just local processor) */
	void GlobalParticleLayout_InitialiseParticle( 
			void*             particleLayout, 
			void*             swarm, 
			Particle_Index    newParticle_I,
			void*             particle );
	
#endif /* __Discretisation_Swarm_GlobalParticleLayout_h__ */

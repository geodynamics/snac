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
**	Instantiates the ParticleLayout abstract class to a randomly distributed particle layout within each cell.
**
** Assumptions:
**	Cell is a right-angled cuboid.
**
** Comments:
**
** $Id: RandomParticleLayout.h 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Swarm_RandomParticleLayout_h__
#define __Discretisation_Swarm_RandomParticleLayout_h__
	

	/* Textual name of this class */
	extern const Type RandomParticleLayout_Type;
	
	/* RandomParticleLayout information */
	#define __RandomParticleLayout \
		__PerCellParticleLayout \
		\
		Particle_InCellIndex         cellParticleCount; /**< Number of particles in each cell to create initially */ \
		unsigned int                 seed;              /**< The random seed used to generate positions. */

	struct RandomParticleLayout { __RandomParticleLayout };
	
	/* Create a new RandomParticleLayout and initialise */
	RandomParticleLayout* RandomParticleLayout_New( Name name, Particle_InCellIndex cellParticleCount, unsigned int seed ) ;
	
	/* Creation implementation / Virtual constructor */
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
		Bool                                                        initFlag,
		Particle_InCellIndex                                        cellParticleCount,
		unsigned int                                                seed );

	void _RandomParticleLayout_Init( 
			void*                randomParticleLayout, 
			Particle_InCellIndex cellParticleCount, 
			unsigned int         seed);

	/* 'Class' Stuff */
	void _RandomParticleLayout_Delete( void* randomParticleLayout );
	void _RandomParticleLayout_Print( void* randomParticleLayout, Stream* stream );
	
	#define RandomParticleLayout_Copy( self ) \
		(RandomParticleLayout*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define RandomParticleLayout_DeepCopy( self ) \
		(RandomParticleLayout*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _RandomParticleLayout_Copy( void* randomParticleLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/* 'Stg_Component' Stuff */
	void* _RandomParticleLayout_DefaultNew( Name name ) ;
	void _RandomParticleLayout_Construct( void* randomParticleLayout, Stg_ComponentFactory* cf, void* data );
	void _RandomParticleLayout_Build( void* randomParticleLayout, void* data );
	void _RandomParticleLayout_Initialise( void* randomParticleLayout, void* data );
	void _RandomParticleLayout_Execute( void* randomParticleLayout, void* data );
	void _RandomParticleLayout_Destroy( void* randomParticleLayout, void* data );
	
	Particle_InCellIndex _RandomParticleLayout_InitialCount( void* randomParticleLayout, void* celllayout,  Cell_Index cell_I );
	
	void _RandomParticleLayout_InitialiseParticlesOfCell( void* randomParticleLayout, void* _swarm, Cell_Index cell_I );


	
#endif /* __Discretisation_Swarm_RandomParticleLayout_h__ */

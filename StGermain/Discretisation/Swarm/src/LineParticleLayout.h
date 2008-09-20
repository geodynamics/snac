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
**	Instantiates the ParticleLayout abstract class to a manually distributed particle layout.
**
** Assumptions:
**	Cell is a right-angled cuboid.
**
** Comments:
**
** $Id: LineParticleLayout.h 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Swarm_LineParticleLayout_h__
#define __Discretisation_Swarm_LineParticleLayout_h__
	

	/* Textual name of this class */
	extern const Type LineParticleLayout_Type;
	
	/* LineParticleLayout information */
	#define __LineParticleLayout \
		__GlobalParticleLayout \
		\
		Dimension_Index                                  dim;                      \
		Index                                            vertexCount;              \
		Coord*                                           vertexList;               \
		double*                                          segmentLengthList;        \
		double                                           dx;

	struct LineParticleLayout { __LineParticleLayout };
	
	/* Create a new LineParticleLayout and initialise */
	LineParticleLayout* LineParticleLayout_New(
		Name                                             name,
		Dimension_Index                                  dim,
		Particle_Index                                   totalInitialParticles,
		Index                                            vertexCount,
		Coord*                                           vertexList );

	/* Creation implementation / Virtual constructor */
	LineParticleLayout* _LineParticleLayout_New( 
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
		Bool                                             initFlag );
	
	void _LineParticleLayout_Init( 
		void*                                            particleLayout,
		Dimension_Index                                  dim,
		Particle_Index                                   totalInitialParticles,
		Index                                            vertexCount,
		Coord*                                           vertexList );
	
	/* 'Stg_Class' Stuff */
	void _LineParticleLayout_Delete( void* particleLayout );
	void _LineParticleLayout_Print( void* particleLayout, Stream* stream );
	#define LineParticleLayout_Copy( self ) \
		(LineParticleLayout*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define LineParticleLayout_DeepCopy( self ) \
		(LineParticleLayout*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _LineParticleLayout_Copy( void* particleLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/* 'Stg_Component' Stuff */
	void* _LineParticleLayout_DefaultNew( Name name ) ;
	void _LineParticleLayout_Construct( void* particleLayout, Stg_ComponentFactory *cf, void* data );
	void _LineParticleLayout_Build( void* particleLayout, void* data );
	void _LineParticleLayout_Initialise( void* particleLayout, void* data );
	void _LineParticleLayout_Execute( void* particleLayout, void* data );
	void _LineParticleLayout_Destroy( void* particleLayout, void* data );
	
	/* Initialises the coordinates of a cell's particle */
	void _LineParticleLayout_InitialiseParticle( void* particleLayout, void* swarm, Particle_Index newParticle_I, void* particle);

#endif /* __Discretisation_Swarm_LineParticleLayout_h__ */

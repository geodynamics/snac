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
**	Instantiates the ParticleLayout abstract class to a particle layout at points designed for optimal gaussian integration.
**
** Assumptions:
**	Cell is a right-angled cuboid.
**
** Comments:
**
** $Id: GaussParticleLayout.h 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Swarm_GaussParticleLayout_h__
#define __Discretisation_Swarm_GaussParticleLayout_h__
	

	/* Textual name of this class */
	extern const Type GaussParticleLayout_Type;
	
	/* GaussParticleLayout information */
	#define __GaussParticleLayout \
		__PerCellParticleLayout \
		\
		Dimension_Index      dim;                /**< Number of dimensions to use */ \
		Particle_InCellIndex particlesPerDim[3]; /**< Number of points per dimension */

	struct GaussParticleLayout { __GaussParticleLayout };
	
	/* Create a new GaussParticleLayout and initialise */
	GaussParticleLayout* GaussParticleLayout_New( Name name, Dimension_Index dim, Particle_InCellIndex* particlesPerDim ) ;
	
	/* Creation implementation / Virtual constructor */
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
		Particle_InCellIndex*                                       particlesPerDim );
		
	/* Initialise implementation */
	void _GaussParticleLayout_Init( void* gaussParticleLayout, Dimension_Index dim, Particle_InCellIndex* particlesPerDim );
	
	/* Stg_Class_Delete implementation */
	void _GaussParticleLayout_Delete( void* gaussParticleLayout );
	
	/* Print implementation */
	void _GaussParticleLayout_Print( void* gaussParticleLayout, Stream* stream );
	
	/* Copy */
	#define GaussParticleLayout_Copy( self ) \
		(GaussParticleLayout*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define GaussParticleLayout_DeepCopy( self ) \
		(GaussParticleLayout*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _GaussParticleLayout_Copy( void* gaussParticleLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	void* _GaussParticleLayout_DefaultNew( Name name );
	void  _GaussParticleLayout_Construct( void* gaussParticleLayout, Stg_ComponentFactory* cf, void* data );
	void  _GaussParticleLayout_Build( void* gaussParticleLayout, void* data );
	void  _GaussParticleLayout_Initialise( void* gaussParticleLayout, void* data );
	void  _GaussParticleLayout_Execute( void* gaussParticleLayout, void* data );
	void  _GaussParticleLayout_Destroy( void* gaussParticleLayout, void* data );
	
	Particle_InCellIndex _GaussParticleLayout_InitialCount( void* gaussParticleLayout, void* celllayout, Cell_Index cell_I );
	void _GaussParticleLayout_InitialiseParticlesOfCell( void* gaussParticleLayout, void* swarm, Cell_Index cell_I );

	/** Calculates the weight and abscissa for the given number of points. Upto 5 points can be used.
	 *     @param weight Array of weights, length equal to pointCount
	 *     @param abscissa Array of abscissa, length equal to pointCount
	 *     @param pointCount Number of points to create in a dimension */
	void GaussParticleLayout_GetAbscissaAndWeights1D( double* weight, double* abscissa, Index pointCount ) ;
	
#endif /* __Discretisation_Swarm_GaussParticleLayout_h__ */

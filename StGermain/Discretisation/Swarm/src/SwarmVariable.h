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
**
** Assumptions:
**
** Comments:
**
**	$Id: SwarmVariable.h 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Swarm_SwarmVariable_h__
#define __Discretisation_Swarm_SwarmVariable_h__

	/** Textual name of this class */
	extern const Type SwarmVariable_Type;
	
	typedef void    (SwarmVariable_ValueAtFunction) ( void* swarmVariable, Particle_Index lParticle_I, double* value );
	typedef double  (SwarmVariable_GetGlobalValueFunction) ( void* swarmVariable );
	
	/** SwarmVariable contents */
	#define __SwarmVariable \
		/* General info */ \
		__Stg_Component \
		\
		/* Virtual info */ \
		SwarmVariable_ValueAtFunction*                     _valueAt; \
		SwarmVariable_GetGlobalValueFunction*              _getMinGlobalMagnitude; \
		SwarmVariable_GetGlobalValueFunction*              _getMaxGlobalMagnitude; \
		\
		/* Member info */ \
		Swarm*                                             swarm;                       \
		Variable*                                          variable;                    \
		Index                                              dofCount;                    \
		Dimension_Index                                    dim;                         \
		SwarmVariable_Register*                            swarmVariable_Register;      \

	struct SwarmVariable { __SwarmVariable };	

	/** General Virtual Functions */
	#define SwarmVariable_Copy( self ) \
		(SwarmVariable*)Stg_Class_Copy( self, NULL, False, NULL, NULL )

	/** Creation implementation */
	void* _SwarmVariable_DefaultNew( Name name );

	SwarmVariable* SwarmVariable_New(		
		Name                                               name,
		Swarm*                                             swarm,
		Variable*                                          variable,
		Index                                              dofCount );
	
	SwarmVariable* _SwarmVariable_New(
		SizeT                                              _sizeOfSelf, 
		Type                                               type,
		Stg_Class_DeleteFunction*	                   _delete,
		Stg_Class_PrintFunction*	                   _print, 
		Stg_Class_CopyFunction*	                           _copy, 
		Stg_Component_DefaultConstructorFunction*          _defaultConstructor,
		Stg_Component_ConstructFunction                    _construct,
		Stg_Component_BuildFunction*                       _build,
		Stg_Component_InitialiseFunction*                  _initialise,
		Stg_Component_ExecuteFunction*                     _execute,
		Stg_Component_DestroyFunction*                     _destroy,
		SwarmVariable_ValueAtFunction*                     _valueAt,
		SwarmVariable_GetGlobalValueFunction*              _getMinGlobalMagnitude,
		SwarmVariable_GetGlobalValueFunction*              _getMaxGlobalMagnitude,		
		Name                                               name );


	/** Member initialisation implementation */
	void SwarmVariable_InitAll( void* swarmVariable, Swarm* swarm, Variable* variable, Index dofCount ) ;
	
	void _SwarmVariable_Delete( void* swarmVariable ) ;
	void _SwarmVariable_Print( void* _swarmVariable, Stream* stream ) ;
	void _SwarmVariable_Construct( void* swarmVariable, Stg_ComponentFactory* cf, void* data ) ;
	void _SwarmVariable_Build( void* swarmVariable, void* data ) ;
	void _SwarmVariable_Execute( void* swarmVariable, void* data ) ;
	void _SwarmVariable_Destroy( void* swarmVariable, void* data ) ;
	void _SwarmVariable_Initialise( void* swarmVariable, void* data ) ;
	
	/* Copy */
	void* _SwarmVariable_Copy( void* swarmVariable, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/** Interface to determine the value of the swarm at a specific particle. */
	#define SwarmVariable_ValueAt( swarmVariable, lParticle_I, value ) \
		((SwarmVariable*)swarmVariable)->_valueAt( swarmVariable, lParticle_I, value )

	/** Interface for finding the minimum value for all particles globally */
	double SwarmVariable_GetMinGlobalMagnitude( void* swarmVariable );

	/** Interface for finding the maximum value for all particles globally */
	double SwarmVariable_GetMaxGlobalMagnitude( void* swarmVariable );

	/*** Default Implementations ***/
	void _SwarmVariable_ValueAt( void* swarmVariable, Particle_Index lParticle_I, double* value ) ;
	void _SwarmVariable_ValueAtDouble( void* swarmVariable, Particle_Index lParticle_I, double* value ) ;
	void _SwarmVariable_ValueAtInt( void* swarmVariable, Particle_Index lParticle_I, double* value ) ;
	void _SwarmVariable_ValueAtFloat( void* swarmVariable, Particle_Index lParticle_I, double* value ) ;
	void _SwarmVariable_ValueAtChar( void* swarmVariable, Particle_Index lParticle_I, double* value ) ;
	void _SwarmVariable_ValueAtShort( void* swarmVariable, Particle_Index lParticle_I, double* value ) ;

	double _SwarmVariable_GetMaxGlobalMagnitude( void* swarmVariable ) ;
	double _SwarmVariable_GetMinGlobalMagnitude( void* swarmVariable ) ;

#endif /* __Discretisation_Swarm_SwarmVariable_h__ */

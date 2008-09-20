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
**	This module represents the state of a particle swarm on a serial or parallel implementation.
**	Its behaviour is dedicated to ensuring the integrity of the swarm.. it does not effect the state of what the swarm contains.
**	An swarm is an organised collection of particles... equivalent to a mesh being a collection of organised nodes.
**
** Assumptions:	stream = Journal_Register (Info_Type, "myStream");	
**	The cell layout does not change... i.e. number of cells does not change.
**	All cells are considered local.
**
** Comments:
**	All particles are considered local (those that move into shadow cells will be moved
**	to appropriate processor immediately.)
**	Communication is more efficient when shadow depth is equal to at least 1
**
** $Id: SwarmClass.h 4044 2007-03-21 23:35:21Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Swarm_SwarmClass_h__
#define __Discretisation_Swarm_SwarmClass_h__
	

	/* Textual name of this class */
	extern const Type Swarm_Type;

	/* Swarm information */
	#define __Swarm \
		/* General info */ \
		__Stg_Component \
		\
		/* Virtual info */ \
		\
		/* General info */ \
		CellLayout*                     cellLayout; \
		ParticleLayout*                 particleLayout; \
		Dimension_Index                 dim;             \
		ExtensionManager_Register*      extension_Register; \
		Stream*                         debug; \
		Partition_Index                 myRank; \
		Partition_Index                 nProc; \
		MPI_Comm                        comm; \
		/** Used to tell the Swarm_UpdateParticleOwners function whether or not
		 * parallel comms is necessary. */ \
		Bool                            stillDoingInitialisation; \
		/** Used to tell the checkpointing and reloading functions whether this
		 * swarm type should actually be checkpointed. */ \
		Bool                            isSwarmTypeToCheckPointAndReload; \
		\
		/* Cell related */ \
		/** The number of local 'cells': equiv to elements for the swarm */ \
		Cell_Index                      cellLocalCount; \
		/** The number of domain cells */ \
		Cell_Index                      cellDomainCount; \
		/** The number of shadow cells */ \
		Cell_Index                      cellShadowCount; \
		/** Cell points are the geometry of each cell: indexed by cell */ \
		Cell_PointsList                 cellPointTbl; \
		/** Count of how many cell points per cell */ \
		Cell_PointIndexList             cellPointCountTbl; \
		\
		/* Particle related */ \
		/** The indices of each particle... */ \
		Cell_ParticlesList              cellParticleTbl; \
		/** The count of how many particles are in each cell */ \
		Cell_ParticlesIndexList         cellParticleCountTbl; \
		/** The size of how many particle index entries are alloced in each cell */ \
		Cell_ParticlesIndexList         cellParticleSizeTbl; \
		/** Number of particle index entries to change by when memory added/removed */ \
		Particle_InCellIndex            cellParticleTblDelta; \
		/** The actual array of particles... */ \
		Particle_List                   particles; \
		SizeT                           particleSize;          /**< Size of each particle in bytes */ \
		Particle_Index                  particleLocalCount; \
		Particle_Index                  particlesArraySize; \
		Particle_Index                  particlesArrayDelta; \
		double                          extraParticlesFactor; \
		/** Variable Stuff */ \
		SwarmVariable_Register*         swarmVariable_Register; \
		SwarmVariable*                  owningCellVariable; \
		/** Extension manager for particles */ \
		ExtensionManager*               particleExtensionMgr; \
		/** Particle Communication handler */ \
		ParticleCommHandler*            particleCommunicationHandler; \
		\
		Index                           swarmReg_I; /**< Own index inside the Swarm_Register */

	struct Swarm { __Swarm };
	
	
	/* Create a new Swarm and initialise */
	Swarm* Swarm_DefaultNew( Name name );

	Swarm* Swarm_New( 
		Name                                  name,
		void*                                 cellLayout,
		void*                                 particleLayout,
		Dimension_Index                       dim,
		SizeT                                 particleSize,
		ExtensionManager_Register*            extensionMgr_Register,
		Variable_Register*                    variable_Register,
		MPI_Comm                              comm ) ;
	
	/* Creation implementation / Virtual constructor */
	Swarm* _Swarm_New(
		SizeT                                 _sizeOfSelf,
		Type                                  type,
		Stg_Class_DeleteFunction*             _delete,
		Stg_Class_PrintFunction*              _print,
		Stg_Class_CopyFunction*               _copy, 
		Stg_Component_DefaultConstructorFunction* _defaultConstructor,
		Stg_Component_ConstructFunction*      _construct,
		Stg_Component_BuildFunction*          _build,
		Stg_Component_InitialiseFunction*     _initialise,
		Stg_Component_ExecuteFunction*        _execute,
		Stg_Component_DestroyFunction*        _destroy,
		Name                                  name,
		Bool                                  initFlag,
		CellLayout*                           cellLayout,
		ParticleLayout*                       particleLayout,
		Dimension_Index                       dim,
		SizeT                                 particleSize,
		Particle_InCellIndex                  cellParticleTblDelta, 
		double                                extraParticlesFactor,
		ExtensionManager_Register*            extensionMgr_Register,
		Variable_Register*                    variable_Register,
		MPI_Comm                              comm );

	void _Swarm_Init( 
		Swarm*                                self, 
		void*                                 cellLayout,
		void*                                 particleLayout,
		Dimension_Index                       dim,
		Particle_InCellIndex                  cellParticleTblDelta, 
		double                                extraParticlesFactor,
		ExtensionManager_Register*            extensionMgr_Register,
		Variable_Register*                    variable_Register,
		MPI_Comm                              comm );
	
	/* For use when Particle is not yet a complete type */
	#define Swarm_ParticleInElementAt( self, dElement_I, cParticle_I ) \
		Swarm_ParticleInCellAt( self, CellLayout_MapElementIdToCellId( ((Swarm*)(swarm))->cellLayout, (dElement_I) ) , (cParticle_I) )
	#define Swarm_ParticleInCellAt( self, cell_I, cParticle_I ) \
		Swarm_ParticleAt( self, (self)->cellParticleTbl[(cell_I)][(cParticle_I)] )
		
	void* _Swarm_ParticleInCellAt( void* swarm, Cell_Index cell_I, Particle_InCellIndex cParticle_I );
	
	#define Swarm_ParticleAt( self, dParticle_I ) \
		( ParticleAt( (self)->particles, (dParticle_I), (self)->particleExtensionMgr->finalSize ) )

	#define ParticleAt( array, particle_I, particleSize ) \
		((StandardParticle*)((ArithPointer)(array) + (particle_I) * (particleSize)))

	#define CopyParticle( destArray, destIndex, srcArray, srcIndex, pSize ) \
		(memcpy( ParticleAt( (destArray), (destIndex), (pSize) ), ParticleAt( (srcArray), (srcIndex), (pSize) ), (pSize) ))

	#define Swarm_CopyParticleWithinSwarm( self, destIndex, srcIndex ) \
		( CopyParticle( (self)->particles, (destIndex), (self)->particles, (srcIndex), (self)->particleExtensionMgr->finalSize ) )

	#define Swarm_CopyParticleOntoSwarm( self, destIndex, srcArray, srcIndex ) \
		( CopyParticle( (self)->particles, destIndex, srcArray, srcIndex, (self)->particleExtensionMgr->finalSize ) )

	#define Swarm_CopyParticleOffSwarm( self, destArray, destIndex, srcIndex ) \
		( CopyParticle( (destArray), (destIndex), (self)->particles, (srcIndex), (self)->particleExtensionMgr->finalSize ) )

	void* _Swarm_ParticleAt( void* swarm, Particle_Index dParticle_I );
	
	/* --- Virtual Functions --- */
	
	/* Deletion implementation */
	void _Swarm_Delete( void* swarm );
	
	/* Print implementation */
	void _Swarm_Print( void* swarm, Stream* stream );
	
	/* Copy */
	#define Swarm_Copy( self ) \
		(Swarm*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define Swarm_DeepCopy( self ) \
		(Swarm*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _Swarm_Copy( void* swarm, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	void* _Swarm_DefaultNew( Name name );
	void _Swarm_Construct( void* swarm, Stg_ComponentFactory* cf, void* data );
	void _Swarm_Build( void* swarm, void* data );
	#define _Swarm_Initialize _Swarm_Initialise
	void _Swarm_Initialise( void* swarm, void* data );
	void _Swarm_Execute( void* swarm, void* data );
	void _Swarm_Destroy( void* swarm, void* data );
	
	/* --- Public functions --- */
	
	/** Update the owning cell of all particles: should be called just after the user sets the new position of each
	cell. As part of this function, the ParticleCommHandler is used to move the particles between 
	processors where necessary. */
	void Swarm_UpdateAllParticleOwners( void* swarm );
	
	/** Update the owning cell of a particle: usually called as part of the Swarm_UpdateAllParticleOwners() function.  */
	void Swarm_UpdateParticleOwner( void* swarm, Particle_Index particle_I );

	/** Removes a particle from a cell: the cell will have no record of owning that particle. */
	void Swarm_RemoveParticleFromCell( void* swarm, Cell_DomainIndex dCell_I, Particle_InCellIndex cParticle_I );

	/** Removes a particle from both its cell, and the particles array. Current implementation will switch the last
 	 *  particle to the empty hole created by the one being removed, but a linked list algorithm would
 	 *  be different. */
	void Swarm_DeleteParticle( void* swarm, Particle_Index lParticle_I );

	/** Removes a particle from both its cell, and the particles array - replacing it with a new particle.
	 *  This function should be used, as compared to Swarm_DeleteParticle(), when you have a new/incoming
	 *  particle needing to be inserted at the same time. Using this function will save overhead by
	 *  combining the two operations. */
	void Swarm_DeleteParticleAndReplaceWithNew( void* swarm, Particle_Index particleToDelete_lI,
		void* replacementParticle, Cell_Index replacementParticle_cellIndex );

	/** Adds a particle to a cell: the cell will now have that particle's index stored in its table of owned cells. */
	void Swarm_AddParticleToCell( void* swarm, Cell_DomainIndex dCell_I, Particle_Index particle_I );

	/** Utility function to get a particle's PIC index within a cell from its index in the array of all local particles */
	Particle_InCellIndex Swarm_GetParticleIndexWithinCell( void* swarm, Cell_DomainIndex owningCell, Particle_Index particle_I);

	/** Utility functions to find closest particles to a particular coordinate 'coord' */
	Particle_Index       Swarm_FindClosestParticle( void* _swarm, Dimension_Index dim, double* coord, double* distance) ;
	Particle_InCellIndex Swarm_FindClosestParticleInCell( void* _swarm, Cell_DomainIndex dCell_I, Dimension_Index dim, double* coord, double* distance ) ;

	/** Prints coordinates of all local particles */
	void Swarm_PrintParticleCoords( void* swarm, Stream* stream );

	/** Prints coordinates of all local particles - indexed by cell */
	void Swarm_PrintParticleCoords_ByCell( void* swarm, Stream* stream );

	/** Gets the min and max coordinates of the cell */
	void Swarm_GetCellMinMaxCoords( void* swarm, Cell_DomainIndex cell_I, Coord min, Coord max ) ;

	SwarmVariable* Swarm_NewScalarVariable( 
		void*                           swarm,
		Name                            nameExt,
		SizeT                           dataOffset,
		Variable_DataType               dataType );
	
	SwarmVariable* Swarm_NewVectorVariable( 
		void*                           _swarm,
		Name                            nameExt,
		SizeT                           dataOffset,
		Variable_DataType               dataType,
		Index                           dataTypeCount,
		...                         /* vector component names */ );

	void Swarm_Realloc( void* swarm ) ;
	void Swarm_CheckCoordsAreFinite( void* swarm ) ;
	void Swarm_AssignIndexWithinShape( void* swarm, void* _shape, Variable* variableToAssign, Index indexToAssign ) ;

	/* --- Private Functions --- */
	
	/* Build cells */
	void _Swarm_BuildCells( void* swarm, void* data );
	
	/* Build particles */
	void _Swarm_BuildParticles( void* swarm, void* data );
	
	/* Initialise cells */
	void _Swarm_InitialiseCells( void* swarm, void* data );
	
	/* Initialise particles */
	void _Swarm_InitialiseParticles( void* swarm, void* data );

	/** This function creates a new particle at the end of the swarm */
	StandardParticle* Swarm_CreateNewParticle( void* swarm, Particle_Index* newParticle_I ) ;

	/** For loading from checkpoints : replaces current particle layout with a file particle layout */
	void Swarm_ReplaceCurrentParticleLayoutWithFileParticleLayout( void* swarm, void* _context );

	/** simple function that determines logic for creating a swarm checkpoint filename */
	void Swarm_GetCheckpointFilenameForGivenTimestep( Swarm* self, AbstractContext* context, char* swarmSaveFileName );

#endif /* __Discretisation_Swarm_SwarmClass_h__ */

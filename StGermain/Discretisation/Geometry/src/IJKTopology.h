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
**	Abstract class inheriting from Topology, which handles the 3D topology for
**	an element or node, in an IJK co-ordinate system.
**
** Assumptions:
**
** Comments:
** This class was abstracted from the common code in IJK6Topology.h and
**	IJK26Topology.h on 8 July 2004.
**
** $Id: IJKTopology.h 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "Dimension.h"

#ifndef __Discretisation_Geometry_IJKTopology_h__
#define __Discretisation_Geometry_IJKTopology_h__

	/* Virtual function types */
	
	/** Textual name of this class */
	extern const Type IJKTopology_Type;

	/** Mapping from integer dimension to letter */
	extern const char IJKTopology_DimNumToDimLetter[3];

	/** IJKTopology class contents (see IJKTopology) */
	#define __IJKTopology \
		/* General info */ \
		/** Parent class - Topology */ \
		__Topology \
		Dictionary*			dictionary; \
		\
		/* Virtual info */ \
		\
		/* IJKTopology info ... */ \
		/** The total number of elements/nodes in the I,J, and K directions for the entire mesh. */ \
		IJK				size; \
		/** Determines, for each dimension, whether the topology is periodic */ \
		Bool                            isPeriodic[3];	\
		/** Determines whether objects outside the mesh should be counted - see Topology_NeighbourCount() and Topology_BuildNeighbours(). */ \
		Bool				dynamicSizes;

	/**	Abstract class inheriting from Topology, which handles the topology for
	**	an element/node, in an IJK co-ordinate system - see IJKTopology.h for functions etc.
	*/
	struct IJKTopology { __IJKTopology };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	
	/** Creation implementation */
	IJKTopology* _IJKTopology_New(
		SizeT						_sizeOfSelf, 
		Type						type,
		Stg_Class_DeleteFunction*				_delete,
		Stg_Class_PrintFunction*				_print,
		Stg_Class_CopyFunction*				_copy, 
		Stg_Component_DefaultConstructorFunction*	_defaultConstructor,
		Stg_Component_ConstructFunction*			_construct,
		Stg_Component_BuildFunction*		_build,
		Stg_Component_InitialiseFunction*		_initialise,
		Stg_Component_ExecuteFunction*		_execute,
		Stg_Component_DestroyFunction*		_destroy,
		Name							name,
		Bool							initFlag,
		Topology_NeighbourCountFunction*		neighbourCount,
		Topology_BuildNeighboursFunction*		buildNeighbours,
		Dictionary*					dictionary,
		IJK						size,
		Bool                                            isPeriodic[3] );
	
	/** Initialises members. If a valid dictionary has been passed in, the mesh sizes
	in the dictionary are used to determine the number of elements. Otherwise, the
	values in the "size" input parameter are used. */
	void _IJKTopology_Init(
		IJKTopology*		self,
		IJK			size,
		Bool			isPeriodic[3], 
		Bool			dynamic );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/** Stg_Class_Delete() IJKTopology implementation */
	void _IJKTopology_Delete( void* ijk26Topology );
	
	/** Stg_Class_Print() IJKTopology implementation */
	void _IJKTopology_Print( void* ijk26Topology, Stream* stream );
	
	/** Prints core info without all the Class overhead */
	void IJKTopology_PrintConcise( IJKTopology* self, Stream* stream );

	/* Copy */
	#define IJKTopology_Copy( self ) \
		(IJKTopology*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define IJKTopology_DeepCopy( self ) \
		(IJKTopology*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _IJKTopology_Copy( void* ijkTopology, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	
	void _IJKTopology_Construct( void* ijkTopology, Stg_ComponentFactory* cf, void* data );
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions/macros
	*/
	
	/** Converts i,j,k coordinates into the exact element number for this topology type. */ 
	#define IJK_3DTo1D( self, ijk, indexPtr ) \
		Dimension_3DTo1D( (ijk), (self)->size, indexPtr )

	/** Converts i,j,k coordinates into the exact element number for this topology type. */ 
	#define IJK_3DTo1D_3( self, i, j, k, indexPtr ) \
		Dimension_3DTo1D_3( (i), (j), (k), (self)->size[I_AXIS], (self)->size[J_AXIS], (self)->size[K_AXIS], indexPtr )

	/** Converts an element number in the mesh into i,j,k co-ordinates, returned separately */
	#define IJK_1DTo3D_3( self, index, iPtr, jPtr, kPtr ) \
		Dimension_1DTo3D_3( (index), (self)->size[I_AXIS], (self)->size[J_AXIS], (self)->size[K_AXIS], (iPtr), (jPtr), (kPtr) )
		
	/** Converts an element number in to mesh into i,j,k co-ordinates, returned in an ijk array */
	#define IJK_1DTo3D(self, index, ijk) \
		IJK_1DTo3D_3(self, index, &(ijk)[I_AXIS], &(ijk)[J_AXIS], &(ijk)[K_AXIS])
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/
	
	
#endif /* __Discretisation_Geometry_IJKTopology_h__ */

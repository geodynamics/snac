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
** Role:
**	Abstract class which handles the 3D topology for an element or node. 
**	This includes responsibility for finding neighbour information
**	for a given element or node.
**
** Assumptions:
**
** Comments:
**
** $Id: Topology.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Geometry_Topology_h__
#define __Discretisation_Geometry_Topology_h__
	
	/** Textual name of this class */
	extern const Type Topology_Type;
	
	/* Virtual function types */
	typedef NeighbourIndex	(Topology_NeighbourCountFunction)   ( void* topology, Index index );
	typedef void		(Topology_BuildNeighboursFunction)  ( void* topology, Index index, NeighbourIndex* neighbours );
	
	/** Topology class contents (see Topology) */
	#define __Topology \
		/* General info */ \
		/** Parent class - Stg_Component */ \
		__Stg_Component \
		\
		/* Virtual info */ \
		/**	Virtual function ptr for Topology_NeighbourCount() */ \
		Topology_NeighbourCountFunction*		neighbourCount; \
		/**	Virtual function ptr for Topology_BuildNeighbours() */ \
		Topology_BuildNeighboursFunction*		buildNeighbours; \
		\
		/* Stg_Class info */ \
		/** The maximum number of nodes/elements in the mesh */ \
		Index						indexCount;

	/**	Abstract class which handles the 3D topology for an element or node
	- see Topology.h for member functions. */
	struct Topology { __Topology };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	
	/** Creation implementation */

	Topology* _Topology_New( 
		SizeT							_sizeOfSelf, 
		Type							type,
		Stg_Class_DeleteFunction*					_delete,
		Stg_Class_PrintFunction*					_print, 
		Stg_Class_CopyFunction*					_copy, 
		Stg_Component_DefaultConstructorFunction*	_defaultConstructor,
		Stg_Component_ConstructFunction*			_construct,
		Stg_Component_BuildFunction*		_build,
		Stg_Component_InitialiseFunction*		_initialise,
		Stg_Component_ExecuteFunction*		_execute,
		Stg_Component_DestroyFunction*		_destroy,
		Name							name,
		Bool							initFlag,
		Topology_NeighbourCountFunction*			neighbourCount,
		Topology_BuildNeighboursFunction*			buildNeighbours );
	
	/** Initialisation implementation functions */
	void _Topology_Init(
		Topology*						self );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/** Stg_Class_Delete() implementation */
	void _Topology_Delete( void* topology );
	
	/** Stg_Class_Print() implementation */
	void _Topology_Print( void* topology, Stream* stream);
	
	/* Copy */
	#define Topology_Copy( self ) \
		(Topology*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define Topology_DeepCopy( self ) \
		(Topology*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _Topology_Copy( void* topology, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	/** Returns the number of neigbours for the given element. If the mesh
	system is 2D or 1D, the count will be decreased appropriately. For regular
	systems, the IJKTopology::dynamicSizes parameter effects the count. If
	False, the count will ignore the possibility of neighbours outside the
	mesh not existing. If True, the count will return the number of 
	neighbours that actually exist. */
	NeighbourIndex Topology_NeighbourCount( void* topology, Index index );
	
	/** Returns a dynamically allocated array containing the Index
	number of the neighbours of a given node or element.
	
	Assumes that the neighbours input array has been pre-allocated to the 
	correct size, as returned by Topology_NeighbourCount().

	For regular topologies, the returned contents of the neighbours array
	depends on the IJKTopology::dynamicSizes parameter. If False, the
	positions in the array indexing Neigbours that don't exist will be 
	filled with Topology_Invalid(). If true, neigbours that don't exist
	will be skipped and only neighbours that exist will be returned.
	*/
	void Topology_BuildNeighbours( void* topology, Index index, NeighbourIndex* neighbours );
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Functions/Macros
	*/

	/** This macro defines what should be used to record an invalid value, eg an
	element in the neighbour table that doesn't exist (if you're on the outside
	boundary on the mesh. */
	#define Topology_Invalid( self ) \
		(self)->indexCount
	
#endif /* __Discretisation_Geometry_Topology_h__ */

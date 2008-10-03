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
**	Concrete IJKTopology class for elements or nodes with 26 neighbours
**	(all directions, including diagonals).
**
** Assumptions:
**
** Comments:
** Refactored by PatrickSunter on 7th July 2004 for easier readability.
**
** $Id: IJK26Topology.h 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Geometry_IJK26Topology_h__
#define __Discretisation_Geometry_IJK26Topology_h__

	/* Virtual function types */
	
	/** Textual name of this class */
	extern const Type IJK26Topology_Type;

	/** IJK26Topology class contents (see IJK26Topology) */
	#define __IJK26Topology \
		/* General info */ \
		__IJKTopology \
		\
	
	/**	Handles the 3D topology for an element/node, with 26 neighbours - see IJK26Topology.h.
	That file has full information on the class and its member functions. */
	struct IJK26Topology { __IJK26Topology };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	
	/** Create a IJK26Topology, using the sizes in the dictionary */
	#define IJK26Topology_New( name, dictionary ) \
		IJK26Topology_New_All( name, dictionary, NULL, NULL )
	
	/** Create a IJK26Topology */
	IJK26Topology* IJK26Topology_DefaultNew( Name name );
	
	IJK26Topology* IJK26Topology_New_All(
		Name						name,
		Dictionary*					dictionary,
		IJK						size,
		Bool						isPeriodic[3] );
	
	/** Initialise a IJK26Topology */
	void IJK26Topology_Init(
		IJK26Topology*					self,
		Name						name,
		Dictionary*					dictionary,
		IJK						size,
		Bool						isPeriodic[3] );
	
	/** Creation implementation */
	IJK26Topology* _IJK26Topology_New(
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
		Bool						isPeriodic[3] );
	
	/** Initialises members. Uses IJKTopology_Init(), but behaviour is different
	if the mesh sizes are read from the dictionary: for the IJK26 system we assume
	the topology relates to elements, and thus 1 is subtracted from each of the
	total node numbers per dimension read from the dictionary. */
	void _IJK26Topology_Init( IJK26Topology* self, Bool shiftNegOne );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/** Stg_Class_Delete() implementation */
	void _IJK26Topology_Delete( void* ijk26Topology );
	
	/** Stg_Class_Print() implementation */
	void _IJK26Topology_Print( void* ijk26Topology, Stream* stream );
	
	void _IJK26Topology_Construct( void* ijk26Topology, Stg_ComponentFactory* cf, void* data );
	
	void _IJK26Topology_Build( void* ijk26Topology, void* data );
	
	void _IJK26Topology_Initialise( void* ijk26Topology, void* data );
	
	void _IJK26Topology_Execute( void* ijk26Topology, void* data );
	
	void _IJK26Topology_Destroy( void* ijk26Topology, void* data );
	
	/** Topology_NeighbourCount() implementation. */
	NeighbourIndex _IJK26Topology_NeighbourCount( void* ijk26Topology, Index index );
	
	/** Topology_BuildNeighbours() implementation. */
	void _IJK26Topology_BuildNeighbours( void* ijk26Topology, Index index, NeighbourIndex* neighbours );
	
	/* ******************** Public Functions ********************* */

	/** Handy little function to print the neighbours of an item in an easy to read manner using
		high-tech ASCII art techniques ;) */
	void IJK26Topology_PrintNeighboursOfIndex( IJK26Topology* topology, Index index, Stream* stream );

	/** Version of IJKTopology_PrintNeighboursOfIndex() for when you already have generated the nbr array */
	void IJK26Topology_PrintNeighbourOfIndexFromArray( IJK26Topology* topology, Index index,
		NeighbourIndex* neighbours, NeighbourIndex neighbourCount, Stream* stream );
	
	/* TODO: it would be cool to have a little ASCII art function to print the whole topology layout too
		for 2D topologies, including some special symbols for periodicity too. */
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/
	
	/* Some little utility functions for printing the nbr info */
	
	void IJK26Topology_PrintRowSeparator( IJK26Topology* self, Stream* stream );
	void IJK26Topology_PrintNeighbour( IJK26Topology* topology, Index itemIndex, Stream* stream );
	void IJK26Topology_PrintNormalRow( IJK26Topology* topology, NeighbourIndex* neighbours,
		Index nbr1, Index nbr2, Index nbr3, Stream* stream );
	void IJK26Topology_PrintMiddleRow( IJK26Topology* topology, NeighbourIndex* neighbours,
		Index leftNbr, Index refPos, Index rightNbr, Stream* stream );	
	
#endif /* __Discretisation_Geometry_IJK26Topology_h__ */

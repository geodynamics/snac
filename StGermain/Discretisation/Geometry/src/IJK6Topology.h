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
**	Concrete IJKTopology class for element or node with 6 neighbours
**	(only fully adjoining directions, no diagonals).
**
** Assumptions:
**
** Comments:
**	In this topology, the ordering goes:
**	---In 3D---
**	0:+ve I, 1:+ve J, 2:+ve K, 3:-ve I, 4:-ve J, 5:-ve K
**
**  ---In 2D---
**  In 2D, the same convention applies, except the missing dimension is
**	skipped. Eg if there was no K dimension:
**	0:+ve I, 1:+ve J, 2: -ve I, 3: -ve J
**
**	TODO: Add a link to a TWiki page with a diagram.
**
** $Id: IJK6Topology.h 3851 2006-10-12 08:57:22Z SteveQuenette $
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Geometry_IJK6Topology_h__
#define __Discretisation_Geometry_IJK6Topology_h__

	/* Virtual function types */
	
	/* Textual name of this class */
	extern const Type IJK6Topology_Type;

	/** IJK6Topology class contents */
	#define __IJK6Topology \
		/* General info */ \
		__IJKTopology \

	/**	Concrete IJKTopology class for element or node with 6 neighbours
	- see IJK6Topology.h for member functions. */
	struct IJK6Topology { __IJK6Topology };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	
	/** Create an IJK6Topology, using the dictonary */
	#define IJK6Topology_New( name, dictionary ) \
		IJK6Topology_New_All( name, dictionary, NULL, NULL )
	
	/** Create an IJK6Topology */
	
	IJK6Topology* IJK6Topology_DefaultNew( Name name );
	
	IJK6Topology* IJK6Topology_New_All(
		Name						name,
		Dictionary*					dictionary,
		IJK						size,
		Bool						isPeriodic[3] );

	/** Initialise an IJK6Topology */
	void IJK6Topology_Init(
		IJK6Topology*					self,
		Name						name,
		Dictionary*					dictionary,
		IJK						size,
		Bool						isPeriodic[3] );

	
	/** Creation implementation */
	IJK6Topology* _IJK6Topology_New(
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
		IJK					size,
		Bool						isPeriodic[3] );

	/** Initialisation implementation */
	void _IJK6Topology_Init(
		IJK6Topology*					self );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/** Stg_Class_Delete() implementation */
	void _IJK6Topology_Delete( void* ijk6Topology );
	
	/** Stg_Class_Print() implementation */
	void _IJK6Topology_Print( void* ijk6Topology, Stream* stream );
	
	void _IJK6Topology_Construct( void* ijk6Topology, Stg_ComponentFactory* cf, void* data );
	
	void _IJK6Topology_Build( void* ijk6Topology, void* data );
	
	void _IJK6Topology_Initialise( void* ijk6Topology, void* data );
	
	void _IJK6Topology_Execute( void* ijk6Topology, void* data );
	
	void _IJK6Topology_Destroy( void* ijk6Topology, void* data );
	
	/** Topology_NeighbourCount() implementation. */
	NeighbourIndex _IJK6Topology_NeighbourCount( void* ijk6Topology, Index index );
	
	/** Topology_BuildNeighbours() implementation. */
	void _IJK6Topology_BuildNeighbours( void* ijk6Topology, Index index, NeighbourIndex* neighbours );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public member functions
	*/
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/
	
	
#endif /* __Discretisation_Geometry_IJK6Topology_h__ */

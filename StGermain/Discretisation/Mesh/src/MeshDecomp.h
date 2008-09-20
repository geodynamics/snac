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
**	Interface class for dividing a mesh into partitions between processors, and
**	mapping from local to global numbers.
**
** Assumptions:
**
** Comments:
**	TODO: partition on element is currently known not to work. Have turned it off by default.
**
**	Note: all of the options controlling mesh building, such as MeshDecomp::allowUnbalancing, can be set through
**	the dictionary by adding a Bool entry with the same name as the option you wish to set.
**
**
** $Id: MeshDecomp.h 3574 2006-05-15 11:30:33Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisaton_Mesh_MeshDecomp_h__
#define __Discretisaton_Mesh_MeshDecomp_h__


	/** determines which sets of global->local node information are stored. */
	typedef enum {
		/** Only store 1 set, for the current processor */
		StoreSelf,
		/** (DEFAULT) Store the set for the current processor, plus all processors which neighbour this one.
		The current processor's set goes first. Makes shadow calculations faster. */
		StoreNeighbours,		/* default */
		/** Store the set for every processor. Potentially memory-hungry, even for IndexSets. */
		StoreAll
	} MDStorage;

	/* Textual name of this class */
	extern const Type MeshDecomp_Type;

	/* Virtual function types */
	typedef Node_GlobalIndex	(MeshDecomp_Node_MapLocalToGlobalFunction)      ( void* meshDecomp, Node_LocalIndex localIndex );
	typedef Node_GlobalIndex	(MeshDecomp_Node_MapDomainToGlobalFunction)     ( void* meshDecomp, Node_DomainIndex domainIndex );
	typedef Node_GlobalIndex	(MeshDecomp_Node_MapShadowToGlobalFunction)     ( void* meshDecomp, Node_ShadowIndex shadowIndex );
	typedef Node_LocalIndex		(MeshDecomp_Node_MapGlobalToLocalFunction)      ( void* meshDecomp, Node_GlobalIndex globalIndex );
	typedef Node_DomainIndex	(MeshDecomp_Node_MapGlobalToDomainFunction)     ( void* meshDecomp, Node_GlobalIndex globalIndex );
	typedef Node_ShadowIndex	(MeshDecomp_Node_MapGlobalToShadowFunction)     ( void* meshDecomp, Node_GlobalIndex globalIndex );
	typedef Element_GlobalIndex	(MeshDecomp_Element_MapLocalToGlobalFunction)	( void* meshDecomp, Element_LocalIndex localIndex );
	typedef Element_GlobalIndex	(MeshDecomp_Element_MapDomainToGlobalFunction)	( void* meshDecomp, Element_DomainIndex domainIndex );
	typedef Element_GlobalIndex	(MeshDecomp_Element_MapShadowToGlobalFunction)	( void* meshDecomp, Element_ShadowIndex shadowIndex );
	typedef Element_LocalIndex	(MeshDecomp_Element_MapGlobalToLocalFunction)	( void* meshDecomp, Element_GlobalIndex globalIndex );
	typedef Element_DomainIndex	(MeshDecomp_Element_MapGlobalToDomainFunction)	( void* meshDecomp, Element_GlobalIndex globalIndex );
	typedef Element_ShadowIndex	(MeshDecomp_Element_MapGlobalToShadowFunction)	( void* meshDecomp, Element_GlobalIndex globalIndex );
	typedef Partition_Index         (MeshDecomp_Shadow_ProcCountFunction)           ( void* meshDecomp );
	typedef void			(MeshDecomp_Shadow_BuildProcsFunction)		( void* meshDecomp, Partition_Index* procs );
	typedef Element_ShadowIndex	(MeshDecomp_Shadow_ProcElementCountFunction)	( void* meshDecomp, Partition_Index rank );
	typedef Partition_Index		(MeshDecomp_Proc_WithElementFunction)		( void* meshDecomp, Element_GlobalIndex globalIndex );

	/** MeshDecomp class contents. */
	#define __MeshDecomp \
		/* General info */ \
		__Stg_Component \
		Stream*				debug; \
		Dictionary*					dictionary; \
		\
		/* Virtual info */ \
		MeshDecomp_Node_MapLocalToGlobalFunction*       nodeMapLocalToGlobal; \
		MeshDecomp_Node_MapDomainToGlobalFunction*      nodeMapDomainToGlobal; \
		MeshDecomp_Node_MapShadowToGlobalFunction*      nodeMapShadowToGlobal; \
		MeshDecomp_Node_MapGlobalToLocalFunction*       nodeMapGlobalToLocal; \
		MeshDecomp_Node_MapGlobalToDomainFunction*      nodeMapGlobalToDomain; \
		MeshDecomp_Node_MapGlobalToShadowFunction*      nodeMapGlobalToShadow; \
		MeshDecomp_Element_MapLocalToGlobalFunction*	elementMapLocalToGlobal; \
		MeshDecomp_Element_MapDomainToGlobalFunction*	elementMapDomainToGlobal; \
		MeshDecomp_Element_MapShadowToGlobalFunction*	elementMapShadowToGlobal; \
		MeshDecomp_Element_MapGlobalToLocalFunction*	elementMapGlobalToLocal; \
		MeshDecomp_Element_MapGlobalToDomainFunction*	elementMapGlobalToDomain; \
		MeshDecomp_Element_MapGlobalToShadowFunction*	elementMapGlobalToShadow; \
		MeshDecomp_Shadow_ProcCountFunction*            shadowProcCount; \
		MeshDecomp_Shadow_BuildProcsFunction*           shadowBuildProcs; \
		MeshDecomp_Shadow_ProcElementCountFunction*     shadowProcElementCount; \
		MeshDecomp_Proc_WithElementFunction*            procWithElement; \
		\
		/* MeshDecomp info ... */ \
		/** communicator for this mesh. */ \
		MPI_Comm                                        communicator; \
		/** rank of current processor. */ \
		Partition_Index                                 rank; \
		/** total processors available - other classes should use MeshDecomp::procsInUse instead in most cases. */ \
		Partition_Index                                 nproc; \
		/** Processors actually in use (may be lower than nproc if MeshDecomp::allowUnusedCPUs enabled. */ \
		Partition_Index                                 procsInUse; \
		/** Determines whether some CPUs will sit idle if MeshDecomp::allowUnbalancing is disabled and nodes don't
		divide evenly between processors */ \
		Bool                                            allowUnusedCPUs; \
		/** determines if the mesh should be decomposed so that nodes form the boundaries between partitions,
		and thus the boundary nodes are shared between neighbour partitions. Used in preference to
		partition by element if activated. */ \
		Bool                                            allowPartitionOnNode; \
		/** determines if the mesh should be decomposed so that elements form the boundaries between partitions,
		and thus the boundary elements are shared between neighbour partitions. */ \
		Bool						allowPartitionOnElement; \
		/** */ \
		Bool						allowUnbalancing; \
		/** determines processor arrangement - and thus which processors are neighbours for shadow purposes. */ \
		Topology*					procTopology; \
		\
		/** controls element layout. */ \
		ElementLayout*					elementLayout; \
		/** controls node layout. */ \
		NodeLayout*					nodeLayout; \
		/** determines which sets of global->local node information are stored - see MDStorage doc for what
		the values mean. */ \
		MDStorage					storage; \
		/** Records which of the global nodes are also local nodes. Which processors these 
		are stored for is determined by MeshDecomp::storage */ \
		IndexSet**					localNodeSets; \
		/** Records which of the global nodes are also local nodes. Which processors these 
		are stored for is determined by MeshDecomp::storage */ \
		IndexSet**					shadowNodeSets; \
		/** Records which of the global elements are also local elements. Which processors these 
		are stored for is determined by MeshDecomp::storage */ \
		IndexSet**					localElementSets; \
		/** Records which of the global elements are also shadow elements. Which processors these 
		are stored for is determined by MeshDecomp::storage */ \
		IndexSet**					shadowElementSets; \
		\
		/** determines how many nodes/elements deep shadowing should be done. */ \
		Node_ShadowIndex				shadowDepth; \
		/* counts */ \
		Node_GlobalIndex				nodeGlobalCount; \
		Node_LocalIndex					nodeLocalCount; \
		Node_ShadowIndex				nodeShadowCount; \
		Node_DomainIndex				nodeDomainCount; \
		Element_GlobalIndex				elementGlobalCount; \
		Element_LocalIndex				elementLocalCount; \
		Element_ShadowIndex				elementShadowCount; \
		Element_DomainIndex				elementDomainCount; 

	/** Controls mesh decomposition - see MeshDecomp.h for further documentation. */
	struct MeshDecomp { __MeshDecomp };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/

	/** Class_New() Creation implementation */

	MeshDecomp* _MeshDecomp_New(
		SizeT                                           _sizeOfSelf, 
		Type                                            type,
		Stg_Class_DeleteFunction*                       _delete,
		Stg_Class_PrintFunction*                        _print,
		Stg_Class_CopyFunction*                         _copy, 
		Stg_Component_DefaultConstructorFunction*       _defaultConstructor,
		Stg_Component_ConstructFunction*                _construct,
		Stg_Component_BuildFunction*                    _build,
		Stg_Component_InitialiseFunction*               _initialise,
		Stg_Component_ExecuteFunction*                  _execute,
		Stg_Component_DestroyFunction*                  _destroy,
		Name                                            name,
		Bool                                            initFlag,
		MeshDecomp_Node_MapLocalToGlobalFunction*       nodeMapLocalToGlobal,
		MeshDecomp_Node_MapDomainToGlobalFunction*      nodeMapDomainToGlobal,
		MeshDecomp_Node_MapShadowToGlobalFunction*      nodeMapShadowToGlobal,
		MeshDecomp_Node_MapGlobalToLocalFunction*       nodeMapGlobalToLocal,
		MeshDecomp_Node_MapGlobalToDomainFunction*      nodeMapGlobalToDomain,
		MeshDecomp_Node_MapGlobalToShadowFunction*      nodeMapGlobalToShadow,
		MeshDecomp_Element_MapLocalToGlobalFunction*	elementMapLocalToGlobal,
		MeshDecomp_Element_MapDomainToGlobalFunction*	elementMapDomainToGlobal,
		MeshDecomp_Element_MapShadowToGlobalFunction*	elementMapShadowToGlobal,
		MeshDecomp_Element_MapGlobalToLocalFunction*	elementMapGlobalToLocal,
		MeshDecomp_Element_MapGlobalToDomainFunction*	elementMapGlobalToDomain,
		MeshDecomp_Element_MapGlobalToShadowFunction*	elementMapGlobalToShadow,
		MeshDecomp_Shadow_ProcCountFunction*            shadowProcCount,
		MeshDecomp_Shadow_BuildProcsFunction*           shadowBuildProcs,
		MeshDecomp_Shadow_ProcElementCountFunction*     shadowProcElementCount,
		MeshDecomp_Proc_WithElementFunction*            procWithElement,
		Dictionary*                                     dictionary,
		MPI_Comm                                        communicator,
		ElementLayout*                                  elementLayout,
		NodeLayout*                                     nodeLayout );

	/** Class_Init() implementation functions. */
	void _MeshDecomp_Init(
		MeshDecomp*					self,
		MPI_Comm					communicator,
		ElementLayout*					elementLayout,
		NodeLayout*					nodeLayout,
		Stg_ObjectList*             pointer_Register );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/** Stg_Class_Delete() implementation */
	void _MeshDecomp_Delete( void* meshDecomp );
	
	/** Stg_Class_Print() MeshDecomp implementation */
	void _MeshDecomp_Print( void* meshDecomp, Stream* stream );
	
	/* Copy */
	#define MeshDecomp_Copy( self ) \
		(MeshDecomp*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define MeshDecomp_DeepCopy( self ) \
		(MeshDecomp*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _MeshDecomp_Copy( void* meshDecomp, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	Node_GlobalIndex _MeshDecomp_Node_LocalToGlobal1D( void* meshDecomp, Node_LocalIndex localIndex );
	
	Node_GlobalIndex _MeshDecomp_Node_ShadowToGlobal1D( void* meshDecomp, Node_ShadowIndex shadowIndex );
	
	Node_GlobalIndex _MeshDecomp_Node_DomainToGlobal1D( void* meshDecomp, Node_DomainIndex domainIndex );
	
	Node_LocalIndex _MeshDecomp_Node_GlobalToLocal1D( void* meshDecomp, Node_GlobalIndex globalIndex );
	
	Node_ShadowIndex _MeshDecomp_Node_GlobalToShadow1D( void* meshDecomp, Node_GlobalIndex globalIndex );
	
	Node_DomainIndex _MeshDecomp_Node_GlobalToDomain1D( void* meshDecomp, Node_GlobalIndex globalIndex );

	Element_GlobalIndex _MeshDecomp_Element_LocalToGlobal1D( void* meshDecomp, Element_LocalIndex localIndex );
	
	Element_GlobalIndex _MeshDecomp_Element_ShadowToGlobal1D( void* meshDecomp, Element_ShadowIndex shadowIndex );
	
	Element_GlobalIndex _MeshDecomp_Element_DomainToGlobal1D( void* meshDecomp, Element_DomainIndex domainIndex );

	Element_LocalIndex _MeshDecomp_Element_GlobalToLocal1D( void* meshDecomp, Element_GlobalIndex globalIndex );
	
	Element_ShadowIndex _MeshDecomp_Element_GlobalToShadow1D( void* meshDecomp, Element_GlobalIndex globalIndex );

	Element_DomainIndex _MeshDecomp_Element_GlobalToDomain1D( void* meshDecomp, Element_GlobalIndex globalIndex );
	
	Element_ShadowIndex _MeshDecomp_Shadow_ProcElementCount( void* meshDecomp, Partition_Index rank );
	
	Partition_Index _MeshDecomp_Proc_WithElement( void* meshDecomp, Element_GlobalIndex globalIndex );
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	#define MD_N_Invalid( self ) \
		(self)->nodeGlobalCount

	#define MD_E_Invalid( self ) \
		(self)->elementGlobalCount


	Node_GlobalIndex* MeshDecomp_BuildNodeLocalToGlobalMap( void* meshDecomp );
	Node_GlobalIndex* MeshDecomp_BuildNodeDomainToGlobalMap( void* meshDecomp );
	Node_LocalIndex* MeshDecomp_BuildNodeGlobalToLocalMap( void* meshDecomp );
	Node_DomainIndex* MeshDecomp_BuildNodeGlobalToDomainMap( void* meshDecomp );

	Element_GlobalIndex* MeshDecomp_BuildElementLocalToGlobalMap( void* meshDecomp );
	Element_GlobalIndex* MeshDecomp_BuildElementDomainToGlobalMap( void* meshDecomp );
	Element_LocalIndex* MeshDecomp_BuildElementGlobalToLocalMap( void* meshDecomp );
	Element_DomainIndex* MeshDecomp_BuildElementGlobalToDomainMap( void* meshDecomp );
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/
	
	void _MeshDecomp_DecomposeNodes( void* meshDecomp );
	
	/** builds shadow element and node information for a given processor relative to its neighbours.
		NOTE: that it only works for a shadowDepth of 1, since it uses the Topology_BuildNeighbours
		function to generate shadow nodes from those on the boundary.

		\param meshDecomp self
		\param proc the neighbour processor to build shadow information for.
		\param edgeElementCount the number of elements on the edge of home proc's mesh section
		\param edgeElements array of edge elements to test neighbours of.
	*/
	void _MeshDecomp_BuildShadowSets(
		void*					meshDecomp,
		Partition_Index				proc,
		Element_GlobalIndex			edgeElementCount,
		Element_GlobalIndex*			edgeElements );

	/* Serial Functions - Reassign function pointers for superfast serial implementations */
	void MeshDecomp_OverrideMethodsIfSerial( void* meshDecomp ) ;
	Node_GlobalIndex _MeshDecomp_Node_MapLocalToGlobal_Serial( void* meshDecomp, Node_LocalIndex localIndex ) ;
	Node_GlobalIndex _MeshDecomp_Node_MapDomainToGlobal_Serial( void* meshDecomp, Node_DomainIndex domainIndex ) ;
	Node_GlobalIndex _MeshDecomp_Node_MapShadowToGlobal_Serial( void* meshDecomp, Node_ShadowIndex shadowIndex ) ;

	Node_LocalIndex _MeshDecomp_Node_MapGlobalToLocal_Serial( void* meshDecomp, Node_GlobalIndex globalIndex ) ;
	Node_DomainIndex _MeshDecomp_Node_MapGlobalToDomain_Serial( void* meshDecomp, Node_GlobalIndex globalIndex ) ;
	Node_ShadowIndex _MeshDecomp_Node_MapGlobalToShadow_Serial( void* meshDecomp, Node_GlobalIndex globalIndex ) ;

	Element_GlobalIndex _MeshDecomp_Element_MapLocalToGlobal_Serial( void* meshDecomp, Element_LocalIndex localIndex ) ;
	Element_GlobalIndex _MeshDecomp_Element_MapDomainToGlobal_Serial( void* meshDecomp, Element_DomainIndex domainIndex ) ;
	Element_GlobalIndex _MeshDecomp_Element_MapShadowToGlobal_Serial( void* meshDecomp, Element_DomainIndex domainIndex ) ;

	Element_LocalIndex _MeshDecomp_Element_MapGlobalToLocal_Serial( void* meshDecomp, Element_GlobalIndex globalIndex ) ;
	Element_DomainIndex _MeshDecomp_Element_MapGlobalToDomain_Serial( void* meshDecomp, Element_GlobalIndex globalIndex ) ;
	Element_ShadowIndex _MeshDecomp_Element_MapGlobalToShadow_Serial( void* meshDecomp, Element_GlobalIndex globalIndex ) ;

	Partition_Index _MeshDecomp_Shadow_ProcCount_Serial( void* meshDecomp ) ;
	void _MeshDecomp_Shadow_BuildProcs_Serial( void* meshDecomp, Partition_Index* procs ) ;
	Element_ShadowIndex _MeshDecomp_Shadow_ProcElementCount_Serial( void* meshDecomp, Partition_Index rank ) ;
	Partition_Index _MeshDecomp_Proc_WithElement_Serial( void* meshDecomp, Element_GlobalIndex globalIndex ) ;

#endif /* __Discretisaton_Mesh_MeshDecomp_h__ */

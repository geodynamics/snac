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
**	This module represents the state of a mesh on a serial or parallel implementation.
**	It may take out any form supported by the attached MeshLayout.
**	Its behaviour is dedicated to ensuring the integrity of the mesh... it does not effect the state of what the mesh contains.
**
** Assumptions:
**	The Node and Element state storage types are defined elsewhere.
**	The Node and Element (if shadowed) state storage types can be mem-copied and MPI-sent/recvd (i.e they contain no pointers).
**
** Invariants:
**	If shadowing is enabled, then shadow nodes are always placed after any local nodes in the domain tables. This
**	means that a local node index is always the same as its domain equivalent. It also implies a shadow node index
**	can be converted to a domain index by subtracting the appropriate local count.
**
** Comments:
**	Originally made for Snark and Snac in mind. The Mesh class should be used as the interface to
**	the Mesh's contents for other parts of the code (including e.g. mapping node numbers between
**	global to local indices.)
**
**	Note for all the functions to map global indices to local ones such as Mesh::nodeMapGlobalToLocal, if
**	the given global index corresponds to a node not held in the requested set (Local|Domain|Shadow), then the
**	value looked up will be Mesh_Node_Invalid() or Mesh_Element_Invalid(), as appropriate.
**	
**	Client classes can perform local to global and vice versa transformations by either
**	a) using the Mesh_NodeMapLocalToGlobal() and other public functions,
**	since these will use the tables if they have been build, but otherwise calculate
**	them on the fly.
**	b) Asserting that the appropriate table has been built, then using it directly 
**	(more efficient, so use if the mapping will be made frequently.)
**	
**	Not building the tables saves memory, but can cause a _very significant_ 
**	performance hit for large meshes where the mappings are frequently required.
**	Consequently, the local to global mapping tables are built by default, but
**	the global to local ones aren't since they could be memory-hungry for large
**	parallel runs.
**	
**	Mesh tables will be re-used where appropriate, e.g. the Mesh::nodeL2G and the Mesh::nodeD2G
**	tables, if both requested, will point to the same mappings. (This can be done because of
**	the invariant mentioned above.)
**
** $Id: MeshClass.h 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisaton_Mesh_MeshClass_h__
#define __Discretisaton_Mesh_MeshClass_h__
	

	/** Textual name of this class */
	extern const Type Mesh_Type;

	/* Virtual function types */
	typedef Bool			(Mesh_Node_IsLocalFunction)		( void* mesh, Node_DomainIndex domainIndex );
	typedef Bool			(Mesh_Node_IsShadowFunction)		( void* mesh, Node_DomainIndex domainIndex );
	typedef Bool			(Mesh_Element_IsLocalFunction)		( void* mesh, Element_DomainIndex domainIndex );
	typedef Bool			(Mesh_Element_IsShadowFunction)		( void* mesh, Element_DomainIndex domainIndex );
	
	/** Mesh class contents */
	#define __Mesh								\
		/* General info */						\
		__Stg_Component							\
		Stream*					debug;			\
		Dictionary*				dictionary;		\
										\
		/* Virtual info */						\
										\
		MeshTopology*				topo;			\
		Bool					useTopo;		\
		MeshGenerator*				generator;		\
		ExtensionManager*			info;			\
										\
		/* Node mapping functions */					\
		Mesh_Node_IsLocalFunction*		nodeIsLocal;		\
		Mesh_Node_IsShadowFunction*		nodeIsShadow;		\
		/* Element mapping functions */					\
		Mesh_Element_IsLocalFunction*		elementIsLocal;		\
		Mesh_Element_IsShadowFunction*		elementIsShadow;	\
										\
		/* Mesh info ... */ \
		MeshLayout*				layout; \
		/** Extensions to individual nodes */ \
		ExtensionManager*			nodeExtensionMgr; \
		/** Extensions to individual elements */ \
		ExtensionManager*			elementExtensionMgr; \
		ExtensionManager_Register*		extensionMgr_Register; \
		\
		/* Node */ \
		Node_List				node; \
		Coord*					nodeCoord; \
		unsigned				nSpaceDims;\
		/* counts */ \
		Node_LocalIndex				nodeLocalCount; \
		Node_DomainIndex			nodeDomainCount; \
		Node_ShadowIndex			nodeShadowCount; \
		Node_GlobalIndex			nodeGlobalCount; \
		/* permanent mapping table options */ \
		Bool					buildNodeLocalToGlobalMap; \
		Bool					buildNodeDomainToGlobalMap; \
		Bool					buildNodeGlobalToLocalMap; \
		Bool					buildNodeGlobalToDomainMap; \
		/* permanent mapping tables */ \
		Node_GlobalIndex*			nodeL2G; \
		Node_GlobalIndex*			nodeD2G; \
		Node_DomainIndex*			nodeG2L; \
		Node_DomainIndex*			nodeG2D; \
		/* ownership status of mapping tables */ \
		Bool					ownsNodeLocalToGlobalMap; \
		Bool					ownsNodeDomainToGlobalMap; \
		Bool					ownsNodeGlobalToLocalMap; \
		Bool					ownsNodeGlobalToDomainMap; \
		/* node neighbour and node->elements table options */ \
		Bool					buildNodeNeighbourTbl; \
		Bool					buildNodeElementTbl; \
		/* node neighbour and node->elements tables */ \
		Node_NeighboursList			nodeNeighbourTbl; \
		Node_NeighbourIndexList			nodeNeighbourCountTbl; \
		Node_ElementsList			nodeElementTbl; \
		Node_ElementIndexList			nodeElementCountTbl; \
		/* ownership of node neighbour and node->element tables */ \
		Bool					ownsNodeNeighbourTbl; \
		Bool					ownsNodeNeighbourCountTbl; \
		Bool					ownsNodeElementTbl;\
		Bool					ownsNodeElementCountTbl; \
		\
		/* Element */ \
		Element_List				element; \
		/* counts */ \
		Element_LocalIndex			elementLocalCount; \
		Element_DomainIndex			elementDomainCount; \
		Element_ShadowIndex			elementShadowCount; \
		Element_GlobalIndex			elementGlobalCount; \
		/* permanent mapping table options */ \
		Bool					buildElementLocalToGlobalMap; \
		Bool					buildElementDomainToGlobalMap; \
		Bool					buildElementGlobalToLocalMap; \
		Bool					buildElementGlobalToDomainMap; \
		/* permanent mapping tables */ \
		Element_DomainIndex*			elementL2G; \
		Element_GlobalIndex*			elementD2G; \
		Element_DomainIndex*			elementG2L; \
		Element_DomainIndex*			elementG2D; \
		/* ownership status of mapping tables */ \
		Bool					ownsElementLocalToGlobalMap; \
		Bool					ownsElementDomainToGlobalMap; \
		Bool					ownsElementGlobalToLocalMap; \
		Bool					ownsElementGlobalToDomainMap; \
		/* element neighbour and element->node sets table options */ \
		Bool					buildElementNodeTbl; \
		Bool					buildElementNeighbourTbl; \
		/* element neighbour and element->node sets tables */ \
		Element_NodesList			elementNodeTbl; \
		Element_NodeIndexList			elementNodeCountTbl; \
		Element_NeighboursList			elementNeighbourTbl; \
		Element_NeighbourIndexList		elementNeighbourCountTbl; \
		/* element neighbour and element->node sets table ownership */ \
		Bool					ownsElementNodeTbl; \
		Bool					ownsElementNodeCountTbl; \
		Bool					ownsElementNeighbourTbl; \
		Bool					ownsElementNeighbourCountTbl; \
		\
		/** Determines if G2D tables will be built temporarily to speed up building of \
		neighbour tables or nodeElement/elementNode tables */ \
		Bool					buildTemporaryGlobalTables; \
		\
		/* sync info */ \
		\
		/** Info on nbr procs for shadowing purposes */ \
		ProcNbrInfo*				procNbrInfo; \
		/** Shadow information for all the elements */ \
		ShadowInfo*				elementShadowInfo; \
		/** Shadow information for all the nodes */ \
		ShadowInfo*				nodeShadowInfo;	\
		\
		MPI_Request**				syncRequestTbl; \
		Element**				syncElementTbl; \
		Coord**					syncNodeCoordTbl; \
		Node**					syncNodeTbl; \
		\
		Element*				syncElementSendTbl; \
		Coord*					syncNodeCoordSendTbl; \
		Node*					syncNodeSendTbl;
		

	struct _Mesh { __Mesh };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	
	/* Create a Mesh */
	Mesh* Mesh_DefaultNew( Name name );
	
	Mesh* Mesh_New( 
		Name						name,
		void*						layout,
		SizeT						_nodeSize,
		SizeT						_elementSize,
		void*						extension_Register,
		Dictionary*					dictionary );

	/* Initialise a Mesh */
	void Mesh_Init(
		Mesh*						self,
		Name						name,
		void*						layout,
		SizeT						_nodeSize,
		SizeT						_elementSize,
		void*						extension_Register,
		Dictionary*					dictionary );
	
	/* Creation implementation */
	Mesh* _Mesh_New(
		SizeT						_sizeOfSelf, 
		Type						type,
		Stg_Class_DeleteFunction*				_delete,
		Stg_Class_PrintFunction*				_print,
		Stg_Class_CopyFunction*				_copy, 
		Stg_Component_DefaultConstructorFunction*	_defaultConstructor,
		Stg_Component_ConstructFunction*			_construct,
		Stg_Component_BuildFunction*			_build,
		Stg_Component_InitialiseFunction*			_initialise,
		Stg_Component_ExecuteFunction*			_execute,
		Stg_Component_DestroyFunction*			_destroy,
		Name								name,
		Bool								initFlag,
		Mesh_Node_IsLocalFunction*			nodeIsLocal,
		Mesh_Node_IsShadowFunction*			nodeIsShadow,
		Mesh_Element_IsLocalFunction*			elementIsLocal,
		Mesh_Element_IsShadowFunction*			elementIsShadow,
		void*						layout,
		SizeT						_nodeSize,
		SizeT						_elementSize, 
		void*						extension_Register,
		Dictionary*					dictionary );

	void _Mesh_LoadFromDict( void* mesh, Dictionary* subDict, Dictionary* dictionary, Stg_ObjectList* objList);

	/* Initialisation implementation functions */
	void _Mesh_Init(
		Mesh*						self,
		void*						layout,
		SizeT						_nodeSize,
		SizeT						_elementSize,
		void*						extension_Register );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/* Stg_Class_Delete mesh implementation */
	void _Mesh_Delete( void* mesh );
	
	void Mesh_DestroyTables( void* mesh );
	
	/* Print mesh implementation */
	void _Mesh_Print( void* mesh, Stream* stream );
	
	void _Mesh_PrintCoords( Mesh* self, Stream* meshStream );

	void _Mesh_PrintTable( Stream* meshStream, Index* table, const Name tableName, Index itemCount );
	
	void _Mesh_Print2DTable( Stream* meshStream, Index** table, Index* tableEntryCounts, const Name tableName, Index entryCount );
	
	/* Copy */
	#define Mesh_Copy( self ) \
		(Mesh*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define Mesh_DeepCopy( self ) \
		(Mesh*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _Mesh_Copy( void* mesh, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/* Build implementation */
	void _Mesh_Build( void* mesh, void* data );
	
	/* Construct implementation */
	void _Mesh_Construct( void* mesh, Stg_ComponentFactory* cf, void* data );
	
	/* Initialisation implementation */
	void _Mesh_Initialise( void* mesh, void* data );
	
	/* Execution implementation */
	void _Mesh_Execute( void* mesh, void* data );
	
	/* Destruct implementation */
	void _Mesh_Destroy( void* mesh, void* data );
	
	Bool _Mesh_Node_IsLocal1D( void* mesh, Node_LocalIndex localIndex );
	
	Bool _Mesh_Node_IsShadow1D( void* mesh, Node_ShadowIndex shadowIndex );
	
	Bool _Mesh_Element_IsLocal1D( void* mesh, Element_LocalIndex localIndex );
	
	Bool _Mesh_Element_IsShadow1D( void* mesh, Element_ShadowIndex shadowIndex );
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/
	
	Node_GlobalIndex Mesh_NodeMapLocalToGlobal( void* mesh, Node_LocalIndex localIndex );
	
	Node_GlobalIndex Mesh_NodeMapShadowToGlobal( void* mesh, Node_ShadowIndex shadowIndex );

	Node_GlobalIndex Mesh_NodeMapDomainToGlobal( void* mesh, Node_DomainIndex domainIndex );

	Node_LocalIndex Mesh_NodeMapGlobalToLocal( void* mesh, Node_GlobalIndex globalIndex );

	Node_DomainIndex Mesh_NodeMapGlobalToDomain( void* mesh, Node_GlobalIndex globalIndex );

	Node_ShadowIndex Mesh_NodeMapGlobalToShadow( void* mesh, Node_GlobalIndex globalIndex );

	Element_GlobalIndex Mesh_ElementMapLocalToGlobal( void* mesh, Element_LocalIndex localIndex );
	
	Element_GlobalIndex Mesh_ElementMapShadowToGlobal( void* mesh, Element_ShadowIndex shadowIndex );

	Element_GlobalIndex Mesh_ElementMapDomainToGlobal( void* mesh, Element_DomainIndex domainIndex );

	Element_LocalIndex Mesh_ElementMapGlobalToLocal( void* mesh, Element_GlobalIndex globalIndex );

	Element_DomainIndex Mesh_ElementMapGlobalToDomain( void* mesh, Element_GlobalIndex globalIndex );

	Element_ShadowIndex Mesh_ElementMapGlobalToShadow( void* mesh, Element_GlobalIndex globalIndex );

	#define Mesh_Node_Invalid( self ) \
		(self)->nodeGlobalCount

	#define Mesh_Element_Invalid( self ) \
		(self)->elementGlobalCount
	
	#define Mesh_CoordAtMacro( self, node_I ) \
		((self)->nodeCoord[(node_I)] )
	double* Mesh_CoordAtFunc( Mesh* self, Node_DomainIndex node_dI );
	#ifdef MACRO_AS_FUNC
		#define Mesh_CoordAt Mesh_CoordAtFunc
	#else
		#define Mesh_CoordAt Mesh_CoordAtMacro
	#endif
	
	#define Mesh_NodeAtMacro( self, node_I ) \
		((Node*)((ArithPointer)(self)->node + ((node_I) * (self)->nodeExtensionMgr->finalSize)))
	Node* Mesh_NodeAtFunc( Mesh* self, Node_DomainIndex node_dI );
	#ifdef MACRO_AS_FUNC
		#define Mesh_NodeAt Mesh_NodeAtFunc
	#else
		#define Mesh_NodeAt Mesh_NodeAtMacro
	#endif
	
	#define Mesh_ElementAtMacro( self, element_I ) \
		((Element*)((ArithPointer)(self)->element + ((element_I) * (self)->elementExtensionMgr->finalSize)))
	Element* Mesh_ElementAtFunc( Mesh* self, Element_DomainIndex element_dI );
	#ifdef MACRO_AS_FUNC
		#define Mesh_ElementAt Mesh_ElementAtFunc
	#else
		#define Mesh_ElementAt Mesh_ElementAtMacro
	#endif
	
	/** Places into the input argument nodeCoordPtrs the pointers to the Coord value of each node in the
	given element (often needed in F.E. stuff) */
	void Mesh_GetNodeCoordPtrsOfElement( void* mesh, Element_DomainIndex element_dI, Coord** nodeCoordPtrs );

	/** Prints the coordinates of every node in the current element */
	void Mesh_PrintNodeCoordsOfElement( void* mesh, Element_DomainIndex element_dI, Stream* stream );

	unsigned Mesh_ClosestNode( void* _self, Coord crd );

	double Mesh_NodeSeparation( Mesh* self );

	unsigned Mesh_ElementWithPoint( void* mesh, Coord pnt, PartitionBoundaryStatus bndStat );

	void Mesh_Sync( void* mesh );
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/
		
	/* Build nodes list */
	void _Mesh_BuildNodes( void* mesh );
	
	/* Build node mappings */
	void _Mesh_BuildNodeMaps( void* mesh );
	
	/* Build element list */
	void _Mesh_BuildElements( void* mesh );
	
	/* Build node mappings */
	void _Mesh_BuildElementMaps( void* mesh );
	
	#define Mesh_ActivateNodeLocalToGlobalMap( mesh ) \
		_Mesh_ActivateMap( mesh, buildNodeLocalToGlobalMap, nodeL2G, MeshDecomp_BuildNodeLocalToGlobalMap ) 
	#define Mesh_ActivateNodeDomainToGlobalMap( mesh ) \
		_Mesh_ActivateMap( mesh, buildNodeDomainToGlobalMap, nodeD2G, MeshDecomp_BuildNodeDomainToGlobalMap ) 
	#define Mesh_ActivateNodeGlobalToLocalMap( mesh ) \
		_Mesh_ActivateMap( mesh, NodeGlobalToLocalMap, nodeG2L, MeshDecomp_BuildNodeGlobalToLocalMap ) 
	#define Mesh_ActivateNodeGlobalToDomainMap( mesh ) \
		_Mesh_ActivateMap( mesh, NodeGlobalToDomainMap, nodeG2D, MeshDecomp_BuildNodeGlobalToDomainMap ) 

	#define Mesh_ActivateNodeNeighbourTbl( mesh ) \
		_Mesh_ActivateTable( mesh, buildNodeNeighbourTbl, _Mesh_BuildNodeNeighbours ) 
	#define Mesh_ActivateNodeElementTbl( mesh ) \
		_Mesh_ActivateTable( mesh, buildNodeElementTbl, _Mesh_BuildNodeElements ) 

	#define Mesh_ActivateElementLocalToGlobalMap( mesh ) \
		_Mesh_ActivateMap( mesh, buildElementLocalToGlobalMap, nodeL2G, MeshDecomp_BuildElementLocalToGlobalMap ) 
	#define Mesh_ActivateElementDomainToGlobalMap( mesh ) \
		_Mesh_ActivateMap( mesh, buildElementDomainToGlobalMap, nodeD2G, MeshDecomp_BuildElementDomainToGlobalMap ) 
	#define Mesh_ActivateElementGlobalToLocalMap( mesh ) \
		_Mesh_ActivateMap( mesh, buildElementGlobalToLocalMap, nodeG2L, MeshDecomp_BuildElementGlobalToLocalMap ) 
	#define Mesh_ActivateElementGlobalToDomainMap( mesh ) \
		_Mesh_ActivateMap( mesh, buildElementGlobalToDomainMap, nodeG2D, MeshDecomp_BuildElementGlobalToDomainMap ) 
	
	#define Mesh_ActivateElementNeighbourTbl( mesh ) \
		_Mesh_ActivateTable( mesh, buildElementNeighbourTbl, _Mesh_BuildElementNeighbours ) 
	#define Mesh_ActivateElementNodeTbl( mesh ) \
		_Mesh_ActivateTable( mesh, buildElementNodeTbl, _Mesh_BuildElementNodes ) 

	#define _Mesh_ActivateMap( mesh, mapBuildOption, mapName, mapBuildFunction ) \
		do { \
			(mesh)->mapBuildOption = True; \
			Dictionary_Set( (mesh)->dictionary, #mapBuildOption, Dictionary_Entry_Value_FromBool( True ) ); \
			if ( True == (mesh)->isBuilt ) { \
				(mesh)->mapName = mapBuildFunction( (mesh)->layout->decomp ); \
			} \
		} while (0)

	
	#define _Mesh_ActivateTable( mesh, tableBuildOption, tableBuildFunction ) \
		do { \
			(mesh)->tableBuildOption = True; \
			Dictionary_Set( (mesh)->dictionary, #tableBuildOption, Dictionary_Entry_Value_FromBool( True ) ); \
			if ( True == (mesh)->isBuilt ) { \
				tableBuildFunction( mesh ); \
			} \
		} while (0)

	/* Build node neighbours list */
	void _Mesh_BuildNodeNeighbours( void* mesh );
	
	/* Build node elements list */
	void _Mesh_BuildNodeElements( void* mesh );
	
	/* Build element neighbours list */
	void _Mesh_BuildElementNeighbours( void* mesh );
	
	/* Build element nodes list */
	void _Mesh_BuildElementNodes( void* mesh );
	
	/* Set the initial Node Geometries */
	void _Mesh_InitialNodeGeometries( void* mesh );
	
	void _Mesh_BuildSyncTables( void* mesh );

	IndexSet* Mesh_CreateIndexSetFromShape( void* mesh, void* _shape );
	
#endif /* __Discretisaton_Mesh_MeshClass_h__ */

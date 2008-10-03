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
** $Id: HexaEL.h 3858 2006-10-14 18:08:55Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisaton_Mesh_HexaEL_h__
#define __Discretisaton_Mesh_HexaEL_h__

	/* Virtual function types */
	
	/* Textual name of this class */
	extern const Type HexaEL_Type;

	/* HexaEL information */
	#define __HexaEL \
		/* General info */ \
		__ElementLayout \
		\
		/* Virtual info */ \
		\
		/* HexaEL info ... */ \
		Dimension_Index dim;       \
		IJK				pointSize; \
		IJK				elementSize; \
		Bool                            topologyWasCreatedInternally;
		
	struct _HexaEL { __HexaEL };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	
	/* Create a HexaEL */
	HexaEL* HexaEL_DefaultNew( Name name );
	
	HexaEL* HexaEL_New(
		Name                        name,
		Dimension_Index             dim,
		Dictionary*                 dictionary,
		Geometry*                   geometry );

	/* Initialise a HexaEL */
	void HexaEL_Init(
		HexaEL*                     self,
		Name                        name,
		Dimension_Index             dim,
		Dictionary*                 dictionary,
		Geometry*                   geometry );
	
	/* Creation implementation */
	HexaEL* _HexaEL_New(
		SizeT                                                   _sizeOfSelf, 
		Type                                                    type,
		Stg_Class_DeleteFunction*                               _delete,
		Stg_Class_PrintFunction*                                _print,
		Stg_Class_CopyFunction*                                 _copy, 
		Stg_Component_DefaultConstructorFunction*               _defaultConstructor,
		Stg_Component_ConstructFunction*                        _construct,
		Stg_Component_BuildFunction*                            _build,
		Stg_Component_InitialiseFunction*                       _initialise,
		Stg_Component_ExecuteFunction*                          _execute,
		Stg_Component_DestroyFunction*                          _destroy,
		Name                                                    name,
		Bool                                                    initFlag,
		ElementLayout_BuildFunction*                            build,		
		ElementLayout_BuildCornerIndicesFunction*               buildCornerIndices,
		ElementLayout_CornerElementCountFunction*               cornerElementCount,
		ElementLayout_BuildCornerElementsFunction*              buildCornerElements,
		ElementLayout_BuildEdgeIndicesFunction*                 buildEdgeIndices,
		ElementLayout_EdgeElementCountFunction*                 edgeElementCount,
		ElementLayout_BuildEdgeElementsFunction*                buildEdgeElements,
		ElementLayout_EdgeAtFunction*                           edgeAt,
		ElementLayout_GetStaticMinAndMaxLocalCoordsFunction*    getStaticMinAndMaxLocalCoords,
		ElementLayout_GetStaticMinAndMaxGlobalCoordsFunction*   getStaticMinAndMaxGlobalCoords,				
		ElementLayout_ElementWithPointFunction*                 elementWithPoint,
		Dimension_Index                                         dim,
		Dictionary*                                             dictionary,
		Geometry*                                               geometry );

	/* Initialisation implementation functions */
	void _HexaEL_Init( HexaEL* self, Dimension_Index dim, IJK pointSize, IJK elementSize ) ;
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/* Stg_Class_Delete HexaEL implementation */
	void _HexaEL_Delete( void* hexaEL );
	
	/* Print HexaEL implementation */
	void _HexaEL_Print( void* hexaEL, Stream* stream );
	
	void _HexaEL_Construct( void* hexaEL, Stg_ComponentFactory* cf, void* data );
	
	void _HexaEL_Build( void* hexaEL, void* data );
	
	void _HexaEL_Initialise( void* hexaEL, void* data );
	
	void _HexaEL_Execute( void* hexaEL, void* data );
	
	void _HexaEL_Destroy( void* hexaEL, void* data );
	
	/* Copy */
	#define HexaEL_Copy( self ) \
		(HexaEL*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define HexaEL_DeepCopy( self ) \
		(HexaEL*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _HexaEL_Copy( void* hexaEL, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	void _HexaEL_BuildCornerIndices( void* hexaEL, Element_GlobalIndex globalIndex, Index* points );
	
	Element_GlobalIndex _HexaEL_CornerElementCount( void* hexaEL, Index corner );
	
	void _HexaEL_BuildCornerElements( void* hexaEL, Index corner, Element_GlobalIndex* elements );
	void _HexaEL_BuildCornerElements1D( void* hexaEL, Index corner, Element_GlobalIndex* elements ) ;
	void _HexaEL_BuildCornerElements2D( void* hexaEL, Index corner, Element_GlobalIndex* elements ) ;
	void _HexaEL_BuildCornerElements3D( void* hexaEL, Index corner, Element_GlobalIndex* elements ) ;
	
	void _HexaEL_BuildEdgeIndices( void* hexaEL, Element_GlobalIndex globalIndex, Index* edges );
	void _HexaEL_BuildEdgeIndices1D( void* hexaEL, Element_GlobalIndex globalIndex, Index* edges );
	void _HexaEL_BuildEdgeIndices2D( void* hexaEL, Element_GlobalIndex globalIndex, Index* edges ) ;
	void _HexaEL_BuildEdgeIndices3D( void* hexaEL, Element_GlobalIndex globalIndex, Index* edges ) ;
	
	Element_GlobalIndex _HexaEL_EdgeElementCount( void* hexaEL, Index edge );
	
	void _HexaEL_BuildEdgeElements( void* hexaEL, Index edge, Element_GlobalIndex* elements );
	
	void _HexaEL_EdgeAt( void* hexaEL, Index index, Edge edge );
	void _HexaEL_EdgeAt1D( void* hexaEL, Index index, Edge edge ) ;
	void _HexaEL_EdgeAt2D( void* hexaEL, Index index, Edge edge ) ;
	void _HexaEL_EdgeAt3D( void* hexaEL, Index index, Edge edge ) ;

	void _HexaEL_TriBarycenter( Coord tri[3], const Coord pnt, Coord dst );
	void _HexaEL_TetBarycenter( Coord tet[4], const Coord pnt, double* dst );
	Bool _HexaEL_FindTriBarycenter( const Coord crds[4], const Coord pnt, double* bcs, unsigned* dstInds, 
					PartitionBoundaryStatus bndStatus, MeshTopology* topo, unsigned gElInd );
	Bool _HexaEL_FindTetBarycenter( const Coord crds[8], const Coord pnt, double* bcs, unsigned* dstInds, 
					PartitionBoundaryStatus bndStatus, MeshTopology* topo, unsigned gElInd );
	
	/** TODO: not sure how these functions handle points on the upper border. Have to test.
		PatrickSunter, 7 Mar 2006 */
	Element_GlobalIndex _HexaEL_ElementWithPoint( void* hexaEL, void* decomp, Coord point, void* mesh, 
						      PartitionBoundaryStatus boundaryStatus, unsigned nHints, unsigned* hints );
	Element_DomainIndex _HexaEL_ElementWithPoint1D( void* hexaEL, void* _decomp, Coord point, void* _mesh, 
							PartitionBoundaryStatus boundaryStatus, unsigned nHints, unsigned* hints );
	Element_DomainIndex _HexaEL_ElementWithPoint2D( void* hexaEL, void* decomp, Coord point, void* mesh, 
							PartitionBoundaryStatus boundaryStatus, unsigned nHints, unsigned* hints );
	Element_DomainIndex _HexaEL_ElementWithPoint3D( void* hexaEL, void* decomp, Coord point, void* mesh, 
							PartitionBoundaryStatus boundaryStatus, unsigned nHints, unsigned* hints );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public member functions
	*/
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/
	
	void _HexaEL_BuildElementPlanes( void* hexaEL, Element_GlobalIndex globalIndex, Plane_List planes );
	void _HexaEL_BuildElementLines( void* hexaEL, Element_GlobalIndex globalIndex, Stg_Line_List lines ) ;
	
	
#endif /* __Discretisaton_Mesh_HexaEL_h__ */

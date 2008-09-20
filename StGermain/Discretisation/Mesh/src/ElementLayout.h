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
** $Id: ElementLayout.h 3817 2006-09-27 06:55:42Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisaton_Mesh_ElementLayout_h__
#define __Discretisaton_Mesh_ElementLayout_h__


	/* Virtual function types */
	typedef void			(ElementLayout_BuildFunction)				( void* elementLayout, void* decomp );
	typedef void			(ElementLayout_BuildCornerIndicesFunction)		( void* elementLayout, Element_GlobalIndex globalIndex, Index* points );
	typedef Element_GlobalIndex     (ElementLayout_CornerElementCountFunction)		( void* elementLayout, Index corner );
	typedef void			(ElementLayout_BuildCornerElementsFunction)		( void* elementLayout, Index corner, Element_GlobalIndex* elements );
	typedef void			(ElementLayout_BuildEdgeIndicesFunction)		( void* elementLayout, Element_GlobalIndex globalIndex, Index* edges );
	typedef Element_GlobalIndex     (ElementLayout_EdgeElementCountFunction)		( void* elementLayout, Index edge );
	typedef void			(ElementLayout_BuildEdgeElementsFunction)		( void* elementLayout, Index edge, Element_GlobalIndex* elements );
	
	typedef void			(ElementLayout_EdgeAtFunction)				( void* elementLayout, Index index, Edge edge );
	typedef Element_GlobalIndex     (ElementLayout_ElementWithPointFunction)		( void* elementLayout,
												  void* meshDecomp, Coord point, void* mesh, PartitionBoundaryStatus boundaryStatus, unsigned nHints, unsigned* hints );
	typedef Bool			(ElementLayout_GetStaticMinAndMaxLocalCoordsFunction)	( void* elementLayout, Coord min, Coord max );
	typedef Bool			(ElementLayout_GetStaticMinAndMaxGlobalCoordsFunction)	( void* elementLayout, Coord min, Coord max );
		
	
	/* Textual name of this class */
	extern const Type ElementLayout_Type;

	/* ElementLayout information */
	#define __ElementLayout \
		/* General info */ \
		__Stg_Component \
		Dictionary*					dictionary; \
		\
		/* Virtual info */ \
		ElementLayout_BuildFunction*				build; \
		ElementLayout_BuildCornerIndicesFunction*       	buildCornerIndices; \
		ElementLayout_CornerElementCountFunction*       	cornerElementCount; \
		ElementLayout_BuildCornerElementsFunction*      	buildCornerElements; \
		ElementLayout_BuildEdgeIndicesFunction*			buildEdgeIndices; \
		ElementLayout_EdgeElementCountFunction*			edgeElementCount; \
		ElementLayout_BuildEdgeElementsFunction*		buildEdgeElements; \
		ElementLayout_EdgeAtFunction*				edgeAt; \
		ElementLayout_ElementWithPointFunction*			elementWithPoint; \
		ElementLayout_GetStaticMinAndMaxLocalCoordsFunction*	getStaticMinAndMaxLocalCoords; \
		ElementLayout_GetStaticMinAndMaxGlobalCoordsFunction*	getStaticMinAndMaxGlobalCoords; \
		\
		/* ElementLayout info ... */ \
		Geometry*						geometry; \
		Topology*						topology; \
		Element_Index						elementCount; \
		\
		Index							elementCornerCount; \
		Index							cornerCount; \
		Index							elementEdgeCount; \
		Index							edgeCount; \
		\
		MeshTopology*						topo;

	struct _ElementLayout { __ElementLayout };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	
	/* Creation implementation */

	ElementLayout* _ElementLayout_New(
		SizeT							_sizeOfSelf, 
		Type							type,
		Stg_Class_DeleteFunction*					_delete,
		Stg_Class_PrintFunction*					_print,
		Stg_Class_CopyFunction*					_copy,
		Stg_Component_DefaultConstructorFunction*	_defaultConstructor,
		Stg_Component_ConstructFunction*			_construct,
		Stg_Component_BuildFunction*				_build,
		Stg_Component_InitialiseFunction*			_initialise,
		Stg_Component_ExecuteFunction*				_execute,
		Stg_Component_DestroyFunction*				_destroy,
		Name									name,
		Bool									initFlag,
		ElementLayout_BuildFunction*  						build,
		ElementLayout_BuildCornerIndicesFunction*       	buildCornerIndices,
		ElementLayout_CornerElementCountFunction*       	cornerElementCount,
		ElementLayout_BuildCornerElementsFunction*      	buildCornerElements,
		ElementLayout_BuildEdgeIndicesFunction*			buildEdgeIndices,
		ElementLayout_EdgeElementCountFunction*			edgeElementCount,
		ElementLayout_BuildEdgeElementsFunction*		buildEdgeElements,
		ElementLayout_EdgeAtFunction*				edgeAt,
		ElementLayout_GetStaticMinAndMaxLocalCoordsFunction*	getStaticMinAndMaxLocalCoords,
		ElementLayout_GetStaticMinAndMaxGlobalCoordsFunction*	getStaticMinAndMaxGlobalCoords,
		ElementLayout_ElementWithPointFunction*			elementWithPoint,
		Dictionary*						dictionary,
		Geometry*						geometry,
		Topology*						topology );

	/* Initialisation implementation functions */
	void _ElementLayout_Init(
		ElementLayout*					self,
		Geometry*					geometry,
		Topology*					topology );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/* Stg_Class_Delete ElementLayout implementation */
	void _ElementLayout_Delete( void* elementLayout );
	
	/* Print ElementLayout implementation */
	void _ElementLayout_Print( void* elementLayout, Stream* stream );
	
	/* Copy */
	#define ElementLayout_Copy( self ) \
		(ElementLayout*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define ElementLayout_DeepCopy( self ) \
		(ElementLayout*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _ElementLayout_Copy( void* elementLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/** Build: For preparing point searching stuff etc. */
	void ElementLayout_Build( void* elementLayout, void* decomp );
	/** Default implementation, does nothing. */
	void _ElementLayout_Build( void* elementLayout, void* decomp );
	
	void ElementLayout_BuildCornerElements( void* elementLayout, Index corner, Element_GlobalIndex* elements ) ;
	void ElementLayout_BuildEdgeIndices( void* elementLayout, Element_GlobalIndex globalIndex, Index* edges ) ;
	void ElementLayout_EdgeAt( void* elementLayout, Index index, Edge edge ) ;
	Element_GlobalIndex ElementLayout_ElementWithPoint( void* elementLayout, void* decomp, Coord point, void* mesh, 
							    PartitionBoundaryStatus boundaryStatus, unsigned nHints, unsigned* hints ) ;
	Bool ElementLayout_GetStaticMinAndMaxLocalCoords( void* elementLayout, Coord min, Coord max );
	Bool _ElementLayout_GetStaticMinAndMaxLocalCoords( void* elementLayout, Coord min, Coord max );
	
	Bool ElementLayout_GetStaticMinAndMaxGlobalCoords( void* elementLayout, Coord min, Coord max );
	Bool _ElementLayout_GetStaticMinAndMaxGlobalCoords( void* elementLayout, Coord min, Coord max );
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public member functions
	*/
	
	Index ElementLayout_BuildEdgeSubset( void* elementLayout, Index elementCount, Index* elements, Index** edges );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/
	
	
#endif /* __Discretisaton_Mesh_ElementLayout_h__ */

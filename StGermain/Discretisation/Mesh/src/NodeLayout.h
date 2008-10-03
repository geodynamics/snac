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
**	Lays out the nodes relative to a given ElementLayout (eg 1 node in centre of each element, 1 node in corner of
**	each.
**
** Assumptions:
**
** Comments:
**
** $Id: NodeLayout.h 3574 2006-05-15 11:30:33Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisaton_Mesh_NodeLayout_h__
#define __Discretisaton_Mesh_NodeLayout_h__


	/* Virtual function types */
	typedef void			(NodeLayout_NodeCoordAtFunction)	( void* nodeLayout, Node_Index index, Coord coord );
	typedef void			(NodeLayout_BuildNodeCoordsFunction)    ( void* nodeLayout, Coord_List coords );
	typedef Node_GlobalIndex	(NodeLayout_ElementNodeCountFunction)   ( void* nodeLayout, Element_GlobalIndex globalIndex );
	typedef void			(NodeLayout_BuildElementNodesFunction)  ( void* nodeLayout, Element_GlobalIndex globalIndex, Element_Nodes nodes );
	typedef Element_GlobalIndex     (NodeLayout_NodeElementCountFunction)   ( void* nodeLayout, Node_GlobalIndex globalIndex );
	typedef void			(NodeLayout_BuildNodeElementsFunction)  ( void* nodeLayout, Node_GlobalIndex globalIndex, Node_Elements elements );
	typedef Node_GlobalIndex	(NodeLayout_CalcNodeCountInDimFromElementCountFunction) ( void* nodeLayout, Element_GlobalIndex globalIndex );
	
	/* Textual name of this class */
	extern const Type NodeLayout_Type;

	/* NodeLayout information */
	#define __NodeLayout \
		/* General info */ \
		__Stg_Component \
		Stream*                                                debug; \
		Dictionary*			                       dictionary; \
		\
		/* Virtual info */ \
		NodeLayout_NodeCoordAtFunction*		               nodeCoordAt; \
		NodeLayout_BuildNodeCoordsFunction*                    buildNodeCoords; \
		NodeLayout_ElementNodeCountFunction*                   elementNodeCount; \
		NodeLayout_BuildElementNodesFunction*                  buildElementNodes; \
		NodeLayout_NodeElementCountFunction*                   nodeElementCount; \
		NodeLayout_BuildNodeElementsFunction*                  buildNodeElements; \
		NodeLayout_CalcNodeCountInDimFromElementCountFunction* calcNodeCountInDimFromElementCount; \
		\
		/* NodeLayout info ... */ \
		ElementLayout*                                 elementLayout; \
		Topology*                                      topology; \
		Node_Index                                     nodeCount;
	struct _NodeLayout { __NodeLayout };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	
	/* Creation implementation */
	NodeLayout* _NodeLayout_New(
		SizeT						_sizeOfSelf, 
		Type						type,
		Stg_Class_DeleteFunction*				_delete,
		Stg_Class_PrintFunction*				_print,
		Stg_Class_CopyFunction*				_copy, 
		Stg_Component_DefaultConstructorFunction*	_defaultConstructor,
		Stg_Component_ConstructFunction*			_construct,
		Stg_Component_BuildFunction*				_build,
		Stg_Component_InitialiseFunction*			_initialise,
		Stg_Component_ExecuteFunction*				_execute,
		Stg_Component_DestroyFunction*				_destroy,
		Name							name,
		Bool							initFlag,
		NodeLayout_NodeCoordAtFunction*		        	nodeCoordAt,
		NodeLayout_BuildNodeCoordsFunction*	        	buildNodeCoords,
		NodeLayout_ElementNodeCountFunction*		        elementNodeCount,
		NodeLayout_BuildElementNodesFunction*		        buildElementNodes,
		NodeLayout_NodeElementCountFunction*		        nodeElementCount,
		NodeLayout_BuildNodeElementsFunction*		        buildNodeElements,
		NodeLayout_CalcNodeCountInDimFromElementCountFunction*  calcNodeCountInDimFromElementCount,
		Dictionary*					dictionary,
		ElementLayout*					elementLayout,
		Topology*					topology );

	void _NodeLayout_LoadFromDict( void* nodeLayout, Dictionary* subDict, Dictionary* dictionary, Stg_ObjectList* objList);

	/* Initialisation implementation functions */
	void _NodeLayout_Init(
		NodeLayout*					self,
		ElementLayout*					elementLayout,
		Topology*					topology );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/* Stg_Class_Delete nodeLayout implementation */
	void _NodeLayout_Delete( void* nodeLayout );
	
	/* Print nodeLayout implementation */
	void _NodeLayout_Print( void* nodeLayout, Stream* stream );
	
	/* Copy */
	#define NodeLayout_Copy( self ) \
		(NodeLayout*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define NodeLayout_DeepCopy( self ) \
		(NodeLayout*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _NodeLayout_Copy( void* nodeLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/** Calculates how many nodes are present in a given dimension if a given number of elements are present.
		Note this onyl worth with the regular element layouts. */
	Node_GlobalIndex NodeLayout_CalcNodeCountInDimFromElementCount( void* nodeLayout, Element_GlobalIndex elementCountInDim );
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public member functions
	*/
	
	Node_GlobalIndex NodeLayout_BuildSubset( void* nodeLayout, Index elementCount, Index* elements, Node_GlobalIndex** nodes );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/
	
	
#endif /* __Discretisaton_Mesh_NodeLayout_h__ */

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
** $Id: BodyNL.h 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisaton_Mesh_BodyNL_h__
#define __Discretisaton_Mesh_BodyNL_h__

	/* Virtual function types */
	
	/* Textual name of this class */
	extern const Type BodyNL_Type;

	/* BodyNL information */
	#define __BodyNL \
		/* General info */ \
		__NodeLayout \
		\
		/* Virtual info */ \
		\
		/* BodyNL info ... */
	struct _BodyNL { __BodyNL };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	
	/* Create a BodyNL with all additional parameters */
	BodyNL* BodyNL_DefaultNew( Name name );
	
	BodyNL* BodyNL_New(
		Name						name,
		Dictionary*					dictionary,
		ElementLayout*					elementLayout,
		Topology*					topology );
	
	/* Initialise a BodyNL */
	void BodyNL_Init(
		BodyNL*						self,
		Name						name,
		Dictionary*					dictionary,
		ElementLayout*					elementLayout,
		Topology*					topology );
	
	/* Creation implementation */
	BodyNL* _BodyNL_New(
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
		Name									name,
		Bool									initFlag,
		NodeLayout_NodeCoordAtFunction*			nodeCoordAt,
		NodeLayout_BuildNodeCoordsFunction*		buildNodeCoords,
		NodeLayout_ElementNodeCountFunction*		elementNodeCount,
		NodeLayout_BuildElementNodesFunction*		buildElementNodes,
		NodeLayout_NodeElementCountFunction*		nodeElementCount,
		NodeLayout_BuildNodeElementsFunction*		buildNodeElements,
		NodeLayout_CalcNodeCountInDimFromElementCountFunction*  calcNodeCountInDimFromElementCount,
		Dictionary*					dictionary,
		ElementLayout*					elementLayout,
		Topology*					topology );
	
	/* Initialisation implementation functions */
	void _BodyNL_Init(
		BodyNL*						self );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/* Stg_Class_Delete bodyNL implementation */
	void _BodyNL_Delete( void* bodyNL );
	
	/* Print bodyNL implementation */
	void _BodyNL_Print( void* bodyNL, Stream* stream );
	
	void _BodyNL_Construct( void* bodyNL, Stg_ComponentFactory *cf, void* data );
	
	void _BodyNL_Build( void* bodyNL, void* data );
	
	void _BodyNL_Initialise( void* bodyNL, void* data );
	
	void _BodyNL_Execute( void* bodyNL, void* data );

	void _BodyNL_Destroy( void* bodyNL, void* data );
	
	void _BodyNL_NodeCoordAt( void* bodyNL, Node_Index index, Coord coord );
	
	void _BodyNL_BuildNodeCoords( void* bodyNL, Coord_List coords );
	
	Node_GlobalIndex _BodyNL_ElementNodeCount( void* bodyNL, Element_GlobalIndex globalIndex );
	
	void _BodyNL_BuildElementNodes( void* bodyNL, Element_GlobalIndex globalIndex, Element_Nodes nodes );
	
	Element_GlobalIndex _BodyNL_NodeElementCount( void* bodyNL, Node_GlobalIndex globalIndex );
	
	void _BodyNL_BuildNodeElements( void* bodyNL, Node_GlobalIndex globalIndex, Node_Elements elements );
	
	Node_GlobalIndex _BodyNL_CalcNodeCountInDimFromElementCount( void* nodeLayout, Element_GlobalIndex elementCountInDim );
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public member functions
	*/
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/
	
	
#endif /* __Discretisaton_Mesh_BodyNL_h__ */

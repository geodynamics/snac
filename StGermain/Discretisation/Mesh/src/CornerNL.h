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
** $Id: CornerNL.h 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisaton_Mesh_CornerNL_h__
#define __Discretisaton_Mesh_CornerNL_h__

	/* Virtual function types */
	
	/* Textual name of this class */
	extern const Type CornerNL_Type;

	/* CornerNL information */
	#define __CornerNL \
		/* General info */ \
		__NodeLayout \
		\
		/* Virtual info */ \
		\
		/* CornerNL info ... */
	struct _CornerNL { __CornerNL };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	
	/* Create a CornerNL with all additional parameters */
	CornerNL* CornerNL_DefaultNew( Name name );
	
	CornerNL* CornerNL_New(
		Name						name,
		Dictionary*					dictionary,
		ElementLayout*					elementLayout,
		Topology*					topology );

	void CornerNL_LoadFromDict( void* nodeLayout, Dictionary* subDict, Dictionary* dictionary, Stg_ObjectList* objList);
	
	/* Initialise a CornerNL */
	void CornerNL_Init(
		CornerNL*					self,
		Name						name,
		Dictionary*					dictionary,
		ElementLayout*					elementLayout,
		Topology*					topology );
	
	/* Creation implementation */
	CornerNL* _CornerNL_New(
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
	void _CornerNL_Init(
		CornerNL*					self );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/* Stg_Class_Delete cornerNL implementation */
	void _CornerNL_Delete( void* cornerNL );
	
	/* Print cornerNL implementation */
	void _CornerNL_Print( void* cornerNL, Stream* stream );
	
	void _CornerNL_Construct( void* cornerNL, Stg_ComponentFactory* cf, void* data );
	
	void _CornerNL_Build( void* cornerNL, void* data );
	
	void _CornerNL_Initialise( void* cornerNL, void* data );
	
	void _CornerNL_Execute( void* cornerNL, void* data );
	
	void _CornerNL_Destroy( void* cornerNL, void* data );
	
	/* Copy */
	#define CornerNL_Copy( self ) \
		(CornerNL*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define CornerNL_DeepCopy( self ) \
		(CornerNL*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _CornerNL_Copy( void* cornerNL, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	void _CornerNL_NodeCoordAt( void* cornerNL, Node_Index index, Coord coord );
	
	void _CornerNL_BuildNodeCoords( void* cornerNL, Coord_List coords );
	
	Node_GlobalIndex _CornerNL_ElementNodeCount( void* cornerNL, Element_GlobalIndex globalIndex );
	
	void _CornerNL_BuildElementNodes( void* cornerNL, Element_GlobalIndex globalIndex, Element_Nodes nodes );
	
	Element_GlobalIndex _CornerNL_NodeElementCount( void* cornerNL, Node_GlobalIndex globalIndex );
	
	void _CornerNL_BuildNodeElements( void* cornerNL, Node_GlobalIndex globalIndex, Node_Elements elements );
	
	Node_GlobalIndex _CornerNL_CalcNodeCountInDimFromElementCount( void* nodeLayout, Element_GlobalIndex elementCountInDim );
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public member functions
	*/
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/
	
	
#endif /* __Discretisaton_Mesh_CornerNL_h__ */

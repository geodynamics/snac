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
** $Id: IrregEL.h 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisaton_Mesh_IrregEL_h__
#define __Discretisaton_Mesh_IrregEL_h__

	/* Virtual function types */
	
	/* Textual name of this class */
	extern const Type IrregEL_Type;

	/* IrregEL information */
	#define __IrregEL \
		/* General info */ \
		__ElementLayout \
		\
		/* Virtual info */ \
		\
		/* IrregEL info ... */ \
		Index**				elementTbl;
	struct _IrregEL { __IrregEL };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	
	/* Create a IrregEL */
	IrregEL* IrregEL_DefaultNew( Name name );

	IrregEL* IrregEL_New(
		Name						name,
		Dictionary*					dictionary,
		Geometry*					geometry,
		Topology*					topology,
		Name						listKey );
	
	/* Initialise a IrregEL */
	void IrregEL_Init(
		IrregEL*					self,
		Name						name,
		Dictionary*					dictionary,
		Geometry*					geometry,
		Topology*					topology,
		Name						listKey );
	
	/* Creation implementation */
	IrregEL* _IrregEL_New(
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
		ElementLayout_BuildFunction*  				build,		
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
		Topology*						topology,
		Name							listKey );
	
	/* Initialisation implementation functions */
	void _IrregEL_Init(
		IrregEL*					self,
		Name						listKey );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/* Stg_Class_Delete IrregEL implementation */
	void _IrregEL_Delete( void* irregEL );
	
	/* Print IrregEL implementation */
	void _IrregEL_Print( void* irregEL, Stream* stream );
	
	void _IrregEL_Construct( void* irregEL, Stg_ComponentFactory* cf, void* data );
	
	void _IrregEL_ComponentBuild( void* irregEL, void* data );
	
	void _IrregEL_Initialise( void* irregEL, void* data );
	
	void _IrregEL_Execute( void* irregEL, void* data );
	
	void _IrregEL_Destroy( void* irregEL, void* data );
	
	void _IrregEL_BuildCornerIndices( void* irregEL, Element_GlobalIndex globalIndex, Index* points );
	
	Element_GlobalIndex _IrregEL_CornerElementCount( void* irregEL, Index corner );
	
	void _IrregEL_BuildCornerElements( void* irregEL, Index corner, Element_GlobalIndex* elements );
	
	void _IrregEL_BuildEdgeIndices( void* irregEL, Element_GlobalIndex globalIndex, Index* edges );
	
	Element_GlobalIndex _IrregEL_EdgeElementCount( void* irregEL, Index edge );
	
	void _IrregEL_BuildEdgeElements( void* irregEL, Index edge, Element_GlobalIndex* elements );
	
	void _IrregEL_EdgeAt( void* irregEL, Index index, Edge edge );
	
	Element_GlobalIndex _IrregEL_ElementWithPoint( void* irregEL, void* decomp, Coord point, void* mesh, 
						       PartitionBoundaryStatus boundaryStatus, unsigned nHints, unsigned* hints );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public member functions
	*/
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/
	
	
#endif /* __Discretisaton_Mesh_IrregEL_h__ */

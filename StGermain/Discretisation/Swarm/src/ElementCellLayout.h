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
**	Instantiates the CellLayout abstract class to a cell layout that has the same / is geometry as a mesh's elements.
**
** Assumptions:
**	Element layout (node interconnects) does not change. Element node coordinates do not change (for IsInCell and CellOf).
**	Element nodes are a subclass of Advectable (coord is the first variable in the structure).
**
** Comments:
**	Created with Snac in mind.
**	"Mesh*"  in the constuctor can be set to 0, but must then be set manually before the ensemble is created.
**
** $Id: ElementCellLayout.h 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Swarm_ElementCellLayout_h__
#define __Discretisation_Swarm_ElementCellLayout_h__
	

	/* Textual name of this class */
	extern const Type ElementCellLayout_Type;
	
	/* ElementCellLayout information */
	#define __ElementCellLayout \
		/* General info */ \
		__CellLayout \
		\
		/* Virtual info */ \
		\
		/* ElementCellLayout info */ \
		Mesh*					mesh;

	struct ElementCellLayout { __ElementCellLayout };
	
	
	/* Create a new ElementCellLayout and initialise */
	void* _ElementCellLayout_DefaultNew( Name name );
	
	ElementCellLayout* ElementCellLayout_New( 
			Name name,
			void* mesh );
	
	/* Initialise a ElementCellLayout construct */
	void ElementCellLayout_Init( 
			ElementCellLayout* self, 
			Name name,
			void* mesh );
	
	/* Creation implementation / Virtual constructor */
	ElementCellLayout* _ElementCellLayout_New( 
		SizeT					_sizeOfSelf,
		Type					type,
		Stg_Class_DeleteFunction*			_delete,
		Stg_Class_PrintFunction*			_print,
		Stg_Class_CopyFunction*			_copy, 
		Stg_Component_DefaultConstructorFunction*	_defaultConstructor,
		Stg_Component_ConstructFunction*			_construct,
		Stg_Component_BuildFunction*		_build,
		Stg_Component_InitialiseFunction*		_initialise,
		Stg_Component_ExecuteFunction*		_execute,
		Stg_Component_DestroyFunction*		_destroy,
		Name							name,
		Bool							initFlag,
		CellLayout_CellCountFunction*		_cellLocalCount,
		CellLayout_CellCountFunction*		_cellShadowCount,
		CellLayout_PointCountFunction*		_pointCount,
		CellLayout_InitialisePointsFunction*	_initialisePoints,
		CellLayout_MapElementIdToCellIdFunction*	_mapElementIdToCellId,		
		CellLayout_IsInCellFunction*		_isInCell, 
		CellLayout_CellOfFunction*		_cellOf,
		CellLayout_GetShadowInfoFunction*	_getShadowInfo,		
		void*					mesh );
	
	/* Initialise implementation */
	void _ElementCellLayout_Init( ElementCellLayout* self, void* mesh );
	
	/* Stg_Class_Delete implementation */
	void _ElementCellLayout_Delete( void* elementCellLayout );
	
	/* Print implementation */
	void _ElementCellLayout_Print( void* elementCellLayout, Stream* stream );
	
	/* Copy */
	#define ElementCellLayout_Copy( self ) \
		(ElementCellLayout*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define ElementCellLayout_DeepCopy( self ) \
		(ElementCellLayout*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _ElementCellLayout_Copy( void* elementCellLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );

	void _ElementCellLayout_Construct( void *elementCellLayout, Stg_ComponentFactory *cf, void* data );
	
	void _ElementCellLayout_Build( void *elementCellLayout, void *data );
	
	void _ElementCellLayout_Initialise( void *elementCellLayout, void *data );
	
	void _ElementCellLayout_Execute( void *elementCellLayout, void *data );

	void _ElementCellLayout_Destroy( void *elementCellLayout, void *data );
	
	/* Returns the number of cells in this layout*/
	Cell_Index _ElementCellLayout_CellLocalCount( void* elementCellLayout );
	
	/* Returns the number of cells in this layout*/
	Cell_Index _ElementCellLayout_CellShadowCount( void* elementCellLayout );
	
	/* Returns the cell point count... this is the mesh's element node count */
	Cell_PointIndex _ElementCellLayout_PointCount( void* elementCellLayout, Cell_Index cellIndex );
	
	/* Returns the cell point array... this is the mesh's element node array */
	void _ElementCellLayout_InitialisePoints( void* elementCellLayout, Cell_Index cellIndex, Cell_PointIndex pointCount, 
		Cell_Points points );
	
	/** Implements CellLayout_MapElementIdToCellId(): just returns a cell number, after checking size */
	Cell_Index _ElementCellLayout_MapElementIdToCellId( void* cellLayout, Element_DomainIndex element_dI ); 
	
	/* Specify whether a particle is in a given cell enough */
	Bool _ElementCellLayout_IsInCell( void* elementCellLayout, Cell_Index cellIndex, void* particle );
	
	/* Obtain which cell a given coord lives in */
	Cell_Index _ElementCellLayout_CellOf( void* elementCellLayout, void* particle );

	/* Get the shadow info: uses the mesh's element one */
	ShadowInfo* _ElementCellLayout_GetShadowInfo( void* elementCellLayout );
	
#endif /* __Discretisation_Swarm_ElementCellLayout_h__ */

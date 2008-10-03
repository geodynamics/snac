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
**	Instantiates the CellLayout abstract class to a cell layout that has one single cell, of same dimensionality as a
given mesh layout.
**
** Assumptions:
**
** Comments:
**	"RegularMeshLayout*"  in the constuctor can be set to 0, but must then be set manually before the swarm is created.
**	At the moment, only does a rectangular or hexahedral cell - need to generalise for triangular mesh shapes
**		(more info. needs to be provided on the meshLayout).
**
** $Id: TriSingleCellLayout.h 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Swarm_TriSingleCellLayout_h__
#define __Discretisation_Swarm_TriSingleCellLayout_h__

	
	/* Textual name of this class */
	extern const Type TriSingleCellLayout_Type;
	
	/* TriSingleCellLayout information */
	#define __TriSingleCellLayout \
		/* General info */ \
		__CellLayout \
		\
		/* Virtual info */ \
		\
		/* TriSingleCellLayout info */ \
		Dictionary*			dictionary; \
		int				dim;

	struct TriSingleCellLayout { __TriSingleCellLayout };
	
	
	/* Create a new TriSingleCellLayout and initialise */
	TriSingleCellLayout* TriSingleCellLayout_DefaultNew( Name name );
	
	TriSingleCellLayout* TriSingleCellLayout_New( Name name, int dim, Dictionary* dictionary );
	
	/* Initialise a TriSingleCellLayout construct */
	void TriSingleCellLayout_Init( TriSingleCellLayout* self, Name name, int dim, Dictionary* dictionary );
	
	/* Creation implementation / Virtual constructor */
	TriSingleCellLayout* _TriSingleCellLayout_New( 
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
		int					dim,
		Dictionary* dictionary );
	
	/* Initialise implementation */
	void _TriSingleCellLayout_Init( TriSingleCellLayout* self, int dim );
	
	/* Stg_Class_Delete implementation */
	void _TriSingleCellLayout_Delete( void* triSingleCellLayout );
	
	/* Print implementation */
	void _TriSingleCellLayout_Print( void* triSingleCellLayout, Stream* stream );
	
	/* Copy */
	#define TriSingleCellLayout_Copy( self ) \
		(TriSingleCellLayout*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define TriSingleCellLayout_DeepCopy( self ) \
		(TriSingleCellLayout*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _TriSingleCellLayout_Copy( void* triSingleCellLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );

	void _TriSingleCellLayout_Construct( void *triSingleCellLayout, Stg_ComponentFactory *cf, void* data );
	
	void _TriSingleCellLayout_Build( void* triSingleCellLayout, void* data );
	
	void _TriSingleCellLayout_Initialise( void* triSingleCellLayout, void* data );
	
	void _TriSingleCellLayout_Execute( void* triSingleCellLayout, void* data );
	
	void _TriSingleCellLayout_Destroy( void* triSingleCellLayout, void* data );
	
	/* Returns the number of cells in this layout*/
	Cell_Index _TriSingleCellLayout_CellLocalCount( void* triSingleCellLayout );
	
	/* Returns the number of cells in this layout*/
	Cell_Index _TriSingleCellLayout_CellShadowCount( void* triSingleCellLayout );
	
	/* Returns the cell point count... this is the layout's single node count */
	Cell_PointIndex _TriSingleCellLayout_PointCount( void* triSingleCellLayout, Cell_Index cellIndex );
	
	/* Returns the cell point array... this is the layout's single node array */
	void _TriSingleCellLayout_InitialisePoints( 
		void*			triSingleCellLayout, 
		Cell_Index		cellIndex, 
		Cell_PointIndex		pointCount, 
		Cell_Points		points );
	
	/** Implements CellLayout_MapElementIdToCellId(): always return 0, since all elements correspond to the same cell */
	Cell_Index _TriSingleCellLayout_MapElementIdToCellId( void* cellLayout, Element_DomainIndex element_dI );		

	/* Specify whether a particle is in a given cell */
	Bool _TriSingleCellLayout_IsInCell( void* triSingleCellLayout, Cell_Index cellIndex, void* particle );
	
	/* Obtain which cell a given coord lives in */
	Cell_Index _TriSingleCellLayout_CellOf( void* triSingleCellLayout, void* particle );
	
	/* Get the shadow info */
	ShadowInfo* _TriSingleCellLayout_GetShadowInfo( void* triSingleCellLayout );
	
#endif /* __Discretisation_Swarm_TriSingleCellLayout_h__ */

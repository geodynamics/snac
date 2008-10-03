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
**	Instantiates the CellLayout abstract class to a cell dimExists that has one single cell (when used with a
**	FiniteElementMesh, all elements in the mesh will use this same cell).
**
** Assumptions:
**
** Comments:
**	User needs to pass in an array, saying which dimensions exist ( can be generated from the MeshDecomp)
**
** $Id: SingleCellLayout.h 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Swarm_SingleCellLayout_h__
#define __Discretisation_Swarm_SingleCellLayout_h__
	

	/* Textual name of this class */
	extern const Type SingleCellLayout_Type;
	
	/* SingleCellLayout information */
	#define __SingleCellLayout \
		/* General info */ \
		__CellLayout \
		\
		/* Virtual info */ \
		\
		/* SingleCellLayout info */ \
		Bool            dimExists[3];    /** Defines whether each dimension exists in current sim*/\
		XYZ             min;             /** Max top-right-back corner of cell */\
		XYZ             max;             /** Min bottom-left-front corner of cell */ \
		Coord*          cellPointCoords; /** Generated cell points in single cell */\
		Cell_PointIndex pointCount;	     /** Count of points in single cell */\

	struct SingleCellLayout { __SingleCellLayout };
	
	/* Create a new SingleCellLayout and initialise */
	
	SingleCellLayout* SingleCellLayout_New( Name name, const Bool dimExists[3], const XYZ min, const XYZ max ) ;

	/* Creation implementation / Virtual constructor */
	SingleCellLayout* _SingleCellLayout_New( 
		SizeT                                     _sizeOfSelf,
		Type                                      type,
		Stg_Class_DeleteFunction*                 _delete,
		Stg_Class_PrintFunction*                  _print,
		Stg_Class_CopyFunction*                   _copy, 
		Stg_Component_DefaultConstructorFunction* _defaultConstructor,
		Stg_Component_ConstructFunction*          _construct,
		Stg_Component_BuildFunction*              _build,
		Stg_Component_InitialiseFunction*         _initialise,
		Stg_Component_ExecuteFunction*            _execute,
		Stg_Component_DestroyFunction*            _destroy,
		CellLayout_CellCountFunction*             _cellLocalCount,
		CellLayout_CellCountFunction*             _cellShadowCount,
		CellLayout_PointCountFunction*            _pointCount,
		CellLayout_InitialisePointsFunction*      _initialisePoints,
		CellLayout_MapElementIdToCellIdFunction*  _mapElementIdToCellId,
		CellLayout_IsInCellFunction*              _isInCell, 
		CellLayout_CellOfFunction*                _cellOf,
		CellLayout_GetShadowInfoFunction*         _getShadowInfo,		
		Name                                      name,
		Bool                                      initFlag,
		const Bool                                dimExists[3],
		const XYZ                                 min,
		const XYZ                                 max );

	/* Initialise implementation */
	void _SingleCellLayout_Init( void* cellLayout, const Bool dimExists[3], const XYZ min, const XYZ max );
	
	/* Stg_Class_Delete implementation */
	void _SingleCellLayout_Delete( void* singleCellLayout );
	void _SingleCellLayout_Print( void* singleCellLayout, Stream* stream );
	#define SingleCellLayout_Copy( self ) \
		(SingleCellLayout*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define SingleCellLayout_DeepCopy( self ) \
		(SingleCellLayout*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _SingleCellLayout_Copy( void* singleCellLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/* 'Stg_Component' Class Info */
	void* _SingleCellLayout_DefaultNew( Name name );
	void _SingleCellLayout_Construct( void* singleCellLayout, Stg_ComponentFactory* cf, void* data );
	void _SingleCellLayout_Build( void* singleCellLayout, void* data );
	void _SingleCellLayout_Initialise( void* singleCellLayout, void* data );
	void _SingleCellLayout_Execute( void* singleCellLayout, void* data );
	void _SingleCellLayout_Destroy( void* singleCellLayout, void* data );
	
	/** Returns the number of local cells in this cellLayout */
	Cell_Index _SingleCellLayout_CellLocalCount( void* singleCellLayout );
	
	/** Returns the number of shadow cells in this cellLayout */
	Cell_Index _SingleCellLayout_CellShadowCount( void* singleCellLayout );
	
	/** Returns the cell point count... this is the cellLayout's single node count */
	Cell_PointIndex _SingleCellLayout_PointCount( void* singleCellLayout, Cell_Index cellIndex );
	
	/** Returns the cell point array... this is the cellLayout's single node array */
	void _SingleCellLayout_InitialisePoints( void* singleCellLayout, Cell_Index cellIndex, Cell_PointIndex pointCount, 
		Cell_Points points );
	
	/** Implements CellLayout_MapElementIdToCellId(): always return 0, since all elements correspond to the same cell */
	Cell_Index _SingleCellLayout_MapElementIdToCellId( void* cellLayout, Element_DomainIndex element_dI );
	
	/* Specify whether a particle is in a given cell */
	Bool _SingleCellLayout_IsInCell( void* singleCellLayout, Cell_Index cellIndex, void* particle );
	
	/* Obtain which cell a given coord lives in */
	Cell_Index _SingleCellLayout_CellOf( void* singleCellLayout, void* particle );
	
	/* Get the shadow info */
	ShadowInfo* _SingleCellLayout_GetShadowInfo( void* singleCellLayout );
	
	/* --- Private Functions --- */

	void _SingleCellLayout_CalculateGlobalPointCount( SingleCellLayout* self );

	void _SingleCellLayout_InitialiseGlobalCellPointPositions( SingleCellLayout* self );
	
#endif /* __Discretisation_Swarm_SingleCellLayout_h__ */

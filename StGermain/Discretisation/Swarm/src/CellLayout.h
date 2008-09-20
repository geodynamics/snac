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
**	Abstract class faciliting how cells of a particle swarms are arranged. 
**	Swarms should only reference this... the base class, not the implementations.
**
** Assumptions:
**
** Comments:
**	Currently built with only ElementCellLayout in mind (for Snac).
**
** $Id: CellLayout.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Swarm_CellLayout_h__
#define __Discretisation_Swarm_CellLayout_h__
	

	/* Child classes must define these abstract functions */
	typedef Cell_Index	(CellLayout_CellCountFunction)		( void* cellLayout );
	typedef Cell_PointIndex	(CellLayout_PointCountFunction)		( void* cellLayout, Cell_Index cell_I );
	typedef void		(CellLayout_InitialisePointsFunction)	( void* cellLayout, 
										Cell_Index cell_I, 
										Cell_PointIndex pointCount, 
										Cell_Points points );
	typedef Cell_Index	(CellLayout_MapElementIdToCellIdFunction)	( void* cellLayout, Element_DomainIndex element_dI ); 
	typedef Bool		(CellLayout_IsInCellFunction)		( void* cellLayout, Cell_Index cell_I, void* particle );
	typedef Cell_Index	(CellLayout_CellOfFunction)		( void* cellLayout, void* particle );
	typedef ShadowInfo*	(CellLayout_GetShadowInfoFunction)	( void* cellLayout );
	
	/* Textual name of this class */
	extern const Type CellLayout_Type;
	
	/* CellLayout information */
	#define __CellLayout \
		/* General info */ \
		__Stg_Component\
		\
		/* Virtual info */ \
		CellLayout_CellCountFunction*		_cellLocalCount; \
		CellLayout_CellCountFunction*		_cellShadowCount; \
		CellLayout_PointCountFunction*		_pointCount; \
		CellLayout_InitialisePointsFunction*	_initialisePoints; \
		CellLayout_MapElementIdToCellIdFunction*	_mapElementIdToCellId; \
		CellLayout_IsInCellFunction*		_isInCell; \
		CellLayout_CellOfFunction*		_cellOf; \
		CellLayout_GetShadowInfoFunction*	_getShadowInfo; \
		\
		/* CellLayout info */ \

	struct CellLayout { __CellLayout };
	
	
	/* No "CellLayout_New" and "CellLayout_Init" as this is an abstract class */
	
	/* Creation implementation / Virtual constructor */
	CellLayout* _CellLayout_New( 
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
		CellLayout_GetShadowInfoFunction*	_getShadowInfo );
	
	/* Initialise implementation */
	void _CellLayout_Init( CellLayout* self );
	
	/* Stg_Class_Delete a CellLayout construct */
	void _CellLayout_Delete( void* cellLayout );
	
	/* Print a CellLayout construct */
	void _CellLayout_Print( void* cellLayout, Stream* stream );
	
	/* Copy */
	#define CellLayout_Copy( self ) \
		(CellLayout*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define CellLayout_DeepCopy( self ) \
		(CellLayout*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _CellLayout_Copy( void* cellLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/* Get the cell counts */
	Cell_Index CellLayout_CellDomainCount( void* cellLayout );
	
	Cell_Index CellLayout_CellLocalCount( void* cellLayout );
	
	Cell_Index CellLayout_CellShadowCount( void* cellLayout );
	
	/** Get the point count for a given cell */
	Cell_PointIndex CellLayout_PointCount( void* cellLayout, Cell_Index cell_I );
	
	/** Initialise cell points (geometry) */
	void CellLayout_InitialiseCellPoints( void* cellLayout, Cell_Index cell_I, Cell_PointIndex pointCount, Cell_Points points );

	/** Get the Cell ID corresponding to a mesh element (only makes sense when the cell layout is used with a mesh
	 * in the same app */
	Cell_Index CellLayout_MapElementIdToCellId( void* cellLayout, Element_DomainIndex element_dI ); 

	/* Specify whether a particle is in a given cell */
	Bool CellLayout_IsInCell( void* cellLayout, Cell_Index cellIndex, void* particle );
	
	/* Obtain which cell a given coord lives in */
	Cell_Index CellLayout_CellOf( void* cellLayout, void* particle );

	ShadowInfo* CellLayout_GetShadowInfo( void* cellLayout );
	
#endif /* __Discretisation_Swarm_CellLayout_h__ */

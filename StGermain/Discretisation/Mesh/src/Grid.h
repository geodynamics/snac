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
** Invariants:
**
** Comments:
**
** $Id: Grid.h 3584 2006-05-16 11:11:07Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisaton_Mesh_Grid_h__
#define __Discretisaton_Mesh_Grid_h__

	/** Textual name of this class */
	extern const Type Grid_Type;

	/** Virtual function types */

	/** Grid class contents */
	#define __Grid			\
		/* General info */	\
		__Stg_Class		\
					\
		/* Virtual info */	\
					\
		/* Grid info */		\
		unsigned	nDims;	\
		unsigned*	sizes;	\
		unsigned*	basis;	\
		unsigned	nPoints;

	struct Grid { __Grid };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/

	#define GRID_DEFARGS		\
		STG_CLASS_DEFARGS

	#define GRID_PASSARGS		\
		STG_CLASS_PASSARGS

	Grid* Grid_New();
	Grid* _Grid_New( GRID_DEFARGS );
	void _Grid_Init( Grid* self );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _Grid_Delete( void* grid );
	void _Grid_Print( void* grid, Stream* stream );

	#define Grid_Copy( self ) \
		(Mesh*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define Grid_DeepCopy( self ) \
		(Mesh*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _Grid_Copy( void* grid, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	void Grid_SetNDims( void* grid, unsigned nDims );
	void Grid_SetSizes( void* grid, unsigned* sizes );

	void Grid_Lift( void* grid, unsigned ind, unsigned* params );
	unsigned Grid_Project( void* grid, unsigned* params );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

#endif /* __Discretisaton_Mesh_Grid_h__ */

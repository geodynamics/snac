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
** $Id: GeometryClass.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Geometry_GeometryClass_h__
#define __Discretisation_Geometry_GeometryClass_h__

	/* Virtual function types */
	typedef void	(Geometry_BuildPointsFunction)	( void* geometry, Coord_List points );
	typedef void	(Geometry_PointAtFunction)	( void* geometry, Index index, Coord point );
	
	/* Textual name of this class */
	extern const Type Geometry_Type;

	/* Geometry information */
	#define __Geometry \
		/* General info */ \
		__Stg_Component \
		Dictionary*				dictionary; \
		\
		/* Virtual info */ \
		Geometry_BuildPointsFunction*		buildPoints; \
		Geometry_PointAtFunction*		pointAt; \
		\
		/* Geometry info ... */ \
		Index					pointCount;
	struct Geometry { __Geometry };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	
	/* Creation implementation */
	Geometry* _Geometry_New(
		SizeT						_sizeOfSelf, 
		Type						type,
		Stg_Class_DeleteFunction*				_delete,
		Stg_Class_PrintFunction*				_print,
		Stg_Class_CopyFunction*				_copy, 
		Stg_Component_DefaultConstructorFunction*	_defaultConstructor,
		Stg_Component_ConstructFunction*			_construct,
		Stg_Component_BuildFunction*		_build,
		Stg_Component_InitialiseFunction*		_initialise,
		Stg_Component_ExecuteFunction*		_execute,
		Stg_Component_DestroyFunction*		_destroy,
		Name							name,
		Bool							initFlag,
		Geometry_BuildPointsFunction*			buildPoints,
		Geometry_PointAtFunction*			pointAt,
		Dictionary*					dictionary );
	
	/* Initialisation implementation functions */
	void _Geometry_Init(
		Geometry*					self );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions ) {
	IrregTopology* self = (IrregTopology*)ir
	*/
	
	/* Stg_Class_Delete Geometry implementation */
	void _Geometry_Delete( void* geometry );
	
	/* Print Geometry implementation */
	void _Geometry_Print( void* geometry, Stream* stream );
	
	/* Copy */
	#define Geometry_Copy( self ) \
		(Geometry*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define Geometry_DeepCopy( self ) \
		(Geometry*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _Geometry_Copy( void* geometry, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public member functions
	*/
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/
	
	
#endif /* __Discretisation_Geometry_GeometryClass_h__ */

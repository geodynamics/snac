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
** $Id: BlockGeometry.h 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Geometry_BlockGeometry_h__
#define __Discretisation_Geometry_BlockGeometry_h__

	/* Virtual function types */
	
	/* Textual name of this class */
	extern const Type BlockGeometry_Type;

	/* BlockGeometry information */
	#define __BlockGeometry \
		/* General info */ \
		__Geometry \
		\
		/* Virtual info */ \
		\
		/* BlockGeometry info ... */ \
		IJK				size; \
		XYZ				min; \
		XYZ				max;
	struct BlockGeometry { __BlockGeometry };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	
	/* Create a BlockGeometry */
	BlockGeometry* BlockGeometry_DefaultNew( Name name );
	
	BlockGeometry* BlockGeometry_New(
		Name						name,
		Dictionary*					dictionary );
	
	/* Initialise a BlockGeometry */
	void BlockGeometry_Init(
		BlockGeometry*					self,
		Name						name,
		Dictionary*					dictionary );
	
	/* Creation implementation */
	BlockGeometry* _BlockGeometry_New(
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
	void _BlockGeometry_Init( BlockGeometry* self, IJK size );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/* Stg_Class_Delete BlockGeometry implementation */
	void _BlockGeometry_Delete( void* blockGeometry );
	
	/* Print BlockGeometry implementation */
	void _BlockGeometry_Print( void* blockGeometry, Stream* stream );
	
	void _BlockGeometry_Construct( void* blockGeometry, Stg_ComponentFactory *cf, void* data );
	
	void _BlockGeometry_Build( void* blockGeometry, void *data );
	
	void _BlockGeometry_Initialise( void* blockGeometry, void *data );
	
	void _BlockGeometry_Execute( void* blockGeometry, void *data );
	
	void _BlockGeometry_Destroy( void* blockGeometry, void *data );
	
	/* Copy */
	#define BlockGeometry_Copy( self ) \
		(BlockGeometry*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define BlockGeometry_DeepCopy( self ) \
		(BlockGeometry*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _BlockGeometry_Copy( void* geometry, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	void _BlockGeometry_BuildPoints( void* blockGeometry, Coord_List points );
	
	void _BlockGeometry_PointAt( void* blockGeometry, Index index, Coord point );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public member functions
	*/
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/
	
	
#endif /* __Discretisation_Geometry_BlockGeometry_h__ */

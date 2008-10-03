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
** $Id: IrregGeometry.h 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Geometry_IrregGeometry_h__
#define __Discretisation_Geometry_IrregGeometry_h__

	/* Virtual function types */
	
	/* Textual name of this class */
	extern const Type IrregGeometry_Type;

	/* IrregGeometry information */
	#define __IrregGeometry \
		/* General info */ \
		__Geometry \
		\
		/* Virtual info */ \
		\
		/* IrregGeometry info ... */ \
		Coord_List			pointTbl;
	struct IrregGeometry { __IrregGeometry };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	
	/* Create a IrregGeometry */
	IrregGeometry* IrregGeometry_DefaultNew( );

	IrregGeometry* IrregGeometry_New(
		Name						name,
		Dictionary*					dictionary,
		Name						listKey );
	
	/* Initialise a IrregGeometry */
	void IrregGeometry_Init(
		IrregGeometry*					self,
		Name						name,
		Dictionary*					dictionary,
		Name						listKey );
	
	/* Creation implementation */
	IrregGeometry* _IrregGeometry_New(
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
		Dictionary*					dictionary,
		Name						listKey );
	
	/* Initialisation implementation functions */
	void _IrregGeometry_Init(
		IrregGeometry*					self,
		Name						listKey );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/* Stg_Class_Delete IrregGeometry implementation */
	void _IrregGeometry_Delete( void* irregGeometry );
	
	/* Print IrregGeometry implementation */
	void _IrregGeometry_Print( void* irregGeometry, Stream* stream );
	
	void _IrregGeometry_Construct( void* irregGeometry, Stg_ComponentFactory *cf, void* data );
	
	void _IrregGeometry_Build( void* irregGeometry, void *data );
	
	void _IrregGeometry_Initialise( void* irregGeometry, void *data );
	
	void _IrregGeometry_Execute( void* irregGeometry, void *data );

	void _IrregGeometry_Destroy( void* irregGeometry, void *data );
	
	void _IrregGeometry_BuildPoints( void* irregGeometry, Coord_List points );
	
	void _IrregGeometry_PointAt( void* irregGeometry, Index index, Coord point );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public member functions
	*/
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/
	
	
#endif /* __Discretisation_Geometry_IrregGeometry_h__ */

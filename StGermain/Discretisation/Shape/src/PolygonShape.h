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
** $Id: PolygonShape.h 4054 2007-03-28 06:46:32Z JulianGiordani $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __StGermain_Discretisation_Shape_PolygonShapeClass_h__
#define __StGermain_Discretisation_Shape_PolygonShapeClass_h__

	/* Textual name of this class */
	extern const Type PolygonShape_Type;

	/* PolygonShape information */
	#define __PolygonShape \
		/* General info */ \
		__Stg_Shape \
		/* Virtual Info */\
		\
		Coord_List              vertexList;    \
		Index                   vertexCount;   \
		XYZ                     start;        \
		XYZ                     end;          \
		Axis                    perpendicularAxis; \

	struct PolygonShape { __PolygonShape };
	
	
	/*---------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	PolygonShape* PolygonShape_New(
		Name                                  name,
		Dimension_Index                       dim,
		XYZ                                   centre, 
		double                                alpha,
		double                                beta,
		double                                gamma,
		Coord_List                            vertexList,
		Index                                 vertexCount,
		XYZ                                   start,
		XYZ                                   end,
		Axis                                  perpendicularAxis);
		
	PolygonShape* _PolygonShape_New(
		SizeT                                 _sizeOfSelf, 
		Type                                  type,
		Stg_Class_DeleteFunction*             _delete,
		Stg_Class_PrintFunction*              _print,
		Stg_Class_CopyFunction*               _copy, 
		Stg_Component_DefaultConstructorFunction* _defaultConstructor,
		Stg_Component_ConstructFunction*      _construct,
		Stg_Component_BuildFunction*          _build,
		Stg_Component_InitialiseFunction*     _initialise,
		Stg_Component_ExecuteFunction*        _execute,
		Stg_Component_DestroyFunction*        _destroy,		
		Stg_Shape_IsCoordInsideFunction*      _isCoordInside,
		Stg_Shape_CalculateVolumeFunction*    _calculateVolume,
		Name                                  name );
	
	void _PolygonShape_Init( void* polygon, Coord_List vertexList, Index vertexCount, XYZ start, XYZ end, Axis perpendicular ) ;
	void PolygonShape_InitAll( 
		void*                                 polygon, 
		Dimension_Index                       dim, 
		Coord                                 centre,
		double                                alpha,
		double                                beta,
		double                                gamma,
		Coord_List                            vertexList,
		Index                                 vertexCount,
		XYZ                                   start,
		XYZ                                   end,
		Axis                                  perpendicularAxis
		) ;
		
	/* Stg_Class_Delete PolygonShape implementation */
	void _PolygonShape_Delete( void* polygon );
	void _PolygonShape_Print( void* polygon, Stream* stream );
	#define PolygonShape_Copy( self ) \
		(PolygonShape*) Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define PolygonShape_DeepCopy( self ) \
		(PolygonShape*) Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _PolygonShape_Copy( void* polygon, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	void* _PolygonShape_DefaultNew( Name name ) ;
	void _PolygonShape_Construct( void* shape, Stg_ComponentFactory* cf, void* data ) ;
	void _PolygonShape_Build( void* polygon, void* data ) ;
	void _PolygonShape_Initialise( void* polygon, void* data ) ;
	void _PolygonShape_Execute( void* polygon, void* data );
	void _PolygonShape_Destroy( void* polygon, void* data ) ;
	
	Bool _PolygonShape_IsCoordInside( void* polygon, Coord coord ) ;
	double _PolygonShape_CalculateVolume( void* polygon );

	/*---------------------------------------------------------------------------------------------------------------------
	** Public member functions
	*/
	
	/*---------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/
	
	
#endif 

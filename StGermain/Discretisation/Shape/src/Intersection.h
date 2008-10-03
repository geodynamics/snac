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
** $Id: Intersection.h 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __StGermain_Discretisation_Shape_IntersectionClass_h__
#define __StGermain_Discretisation_Shape_IntersectionClass_h__

	/* Textual name of this class */
	extern const Type Intersection_Type;

	/* Intersection information */
	#define __Intersection \
		/* General info */ \
		__Stg_Shape \
		/* Virtual Info */\
		\
		Stg_Shape**                   shapeList;     \
		Index                         shapeCount;    \
		Bool*                         isComplement; \

	struct Intersection { __Intersection };
	
	
	/*---------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	Intersection* Intersection_New(
		Name                                  name,
		Dimension_Index                       dim,
		XYZ                                   centre, 
		double                                alpha,
		double                                beta,
		double                                gamma,
		Stg_Shape**                           shapeList,
		Index                                 shapeCount,
		Bool*                                 isComplement);
		
	Intersection* _Intersection_New(
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
	
	void _Intersection_Init( void* intersection, Stg_Shape** shapeList, Index shapeCount, Bool* isComplement ) ;
	void Intersection_InitAll( 
		void*                                 intersection, 
		Dimension_Index                       dim, 
		Coord                                 centre,
		double                                alpha,
		double                                beta,
		double                                gamma,
		Stg_Shape**                           shapeList,
		Index                                 shapeCount,
		Bool*                                 isComplement
		) ;
		
	/* Stg_Class_Delete Intersection implementation */
	void _Intersection_Delete( void* intersection );
	void _Intersection_Print( void* intersection, Stream* stream );
	#define Intersection_Copy( self ) \
		(Intersection*) Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define Intersection_DeepCopy( self ) \
		(Intersection*) Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _Intersection_Copy( void* intersection, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	void* _Intersection_DefaultNew( Name name ) ;
	void _Intersection_Construct( void* shape, Stg_ComponentFactory* cf, void* data ) ;
	void _Intersection_Build( void* intersection, void* data ) ;
	void _Intersection_Initialise( void* intersection, void* data ) ;
	void _Intersection_Execute( void* intersection, void* data );
	void _Intersection_Destroy( void* intersection, void* data ) ;
	
	Bool _Intersection_IsCoordInside( void* intersection, Coord coord ) ;
	double _Intersection_CalculateVolume( void* intersection );

	/*---------------------------------------------------------------------------------------------------------------------
	** Public member functions
	*/
	
	/*---------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/
	
	
#endif 

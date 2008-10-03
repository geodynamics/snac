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
** $Id: ShapeClass.h 3869 2006-10-16 13:42:59Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __StGermain_Discretisation_Shape_Shape_h__
#define __StGermain_Discretisation_Shape_Shape_h__

	typedef Bool (Stg_Shape_IsCoordInsideFunction) ( void* shape, Coord coord );

	typedef double (Stg_Shape_CalculateVolumeFunction) ( void* shape );


	/* Textual name of this class */
	extern const Type Stg_Shape_Type;

	/* Stg_Shape information */
	#define __Stg_Shape \
		/* General info */ \
		__Stg_Component \
		/* Virtual Info */                                         \
		Stg_Shape_IsCoordInsideFunction*      _isCoordInside;      \
		Stg_Shape_CalculateVolumeFunction*    _calculateVolume;    \
		/* Other info */                                           \
		Dimension_Index                       dim;                 \
		XYZ                                   centre;              \
		Bool                                  invert;              \
		double                                rotationMatrix[3][3];

	struct Stg_Shape { __Stg_Shape };
	
	
	/*---------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	Stg_Shape* _Stg_Shape_New(
		SizeT                                       _sizeOfSelf, 
		Type                                        type,
		Stg_Class_DeleteFunction*                   _delete,
		Stg_Class_PrintFunction*                    _print,
		Stg_Class_CopyFunction*                     _copy, 
		Stg_Component_DefaultConstructorFunction*   _defaultConstructor,
		Stg_Component_ConstructFunction*            _construct,
		Stg_Component_BuildFunction*                _build,
		Stg_Component_InitialiseFunction*           _initialise,
		Stg_Component_ExecuteFunction*              _execute,
		Stg_Component_DestroyFunction*              _destroy,
		Stg_Shape_IsCoordInsideFunction*            _isCoordInside,
		Stg_Shape_CalculateVolumeFunction*          _calculateVolume,
		Name                                        name );
	
	void _Stg_Shape_Init( void* shape, Dimension_Index dim, Coord centre, Bool invert, double alpha, double beta, double gamma ) ;
	void Stg_Shape_InitAll( void* shape, Dimension_Index dim, Coord centre, double alpha, double beta, double gamma );

	void _Stg_Shape_Delete( void* shape );
	void _Stg_Shape_Print( void* shape, Stream* stream );
	#define Stg_Shape_Copy( self ) \
		(Stg_Shape*) Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define Stg_Shape_DeepCopy( self ) \
		(Stg_Shape*) Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _Stg_Shape_Copy( void* shape, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );

	/* 'Stg_Component' Virtual Function Implementations */
	void _Stg_Shape_Construct( void* shape, Stg_ComponentFactory* cf, void* data ) ;
	void _Stg_Shape_Build( void* shape, void* data ) ;
	void _Stg_Shape_Initialise( void* shape, void* data ) ;
	void _Stg_Shape_Execute( void* shape, void* data ) ;
	void _Stg_Shape_Destroy( void* shape, void* data ) ;

	
	/*---------------------------------------------------------------------------------------------------------------------
	** Virtual Function Implementation 
	*/
	Bool Stg_Shape_IsCoordInside( void* shape, Coord coord ) ;

	double Stg_Shape_CalculateVolume( void* shape );
	
	/*---------------------------------------------------------------------------------------------------------------------
	** Public member functions
	*/
	void Stg_Shape_TransformCoord( void* shape, Coord coord, Coord transformedCoord ) ;
	void Stg_Shape_TranslateCoord( void* shape, Coord coord, Coord translatedCoord ) ;
	
#endif 

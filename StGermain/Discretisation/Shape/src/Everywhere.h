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
** $Id: Everywhere.h 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __StGermain_Discretisation_Shape_EverywhereClass_h__
#define __StGermain_Discretisation_Shape_EverywhereClass_h__

	/* Textual name of this class */
	extern const Type Everywhere_Type;

	/* Everywhere information */
	#define __Everywhere \
		/* General info */ \
		__Stg_Shape \
		/* Virtual Info */\
		\

	struct Everywhere { __Everywhere };
	
	
	/*---------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	Everywhere* Everywhere_New(
		Name                                  name,
		Dimension_Index                       dim );

	Everywhere* _Everywhere_New(
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
	
	void _Everywhere_Init( void* everywhere ) ;
	void Everywhere_InitAll( 
		void*                                 everywhere, 
		Dimension_Index                       dim, 
		Coord                                 centre,
		double                                alpha,
		double                                beta,
		double                                gamma );

	/* Stg_Class_Delete Everywhere implementation */
	void _Everywhere_Delete( void* everywhere );
	void _Everywhere_Print( void* everywhere, Stream* stream );
	#define Everywhere_Copy( self ) \
		(Everywhere*) Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define Everywhere_DeepCopy( self ) \
		(Everywhere*) Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _Everywhere_Copy( void* everywhere, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	void* _Everywhere_DefaultNew( Name name ) ;
	void _Everywhere_Construct( void* shape, Stg_ComponentFactory* cf, void* data ) ;
	void _Everywhere_Build( void* everywhere, void* data ) ;
	void _Everywhere_Initialise( void* everywhere, void* data ) ;
	void _Everywhere_Execute( void* everywhere, void* data );
	void _Everywhere_Destroy( void* everywhere, void* data ) ;
	
	Bool _Everywhere_IsCoordInside( void* everywhere, Coord coord ) ;
	double _Everywhere_CalculateVolume( void* everywhere );

	/*---------------------------------------------------------------------------------------------------------------------
	** Public member functions
	*/
	
	/*---------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/
	
#endif 

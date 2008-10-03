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
** $Id: PythonShape.h 3869 2006-10-16 13:42:59Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifdef HAVE_PYTHON
#ifndef __StGermain_Discretisation_Shape_PythonShapeClass_h__
#define __StGermain_Discretisation_Shape_PythonShapeClass_h__

	/* Textual name of this class */
	extern const Type PythonShape_Type;

	/* PythonShape information */
	#define __PythonShape \
		/* General info */ \
		__Stg_Shape \
		/* Virtual Info */\
		\
		char*                                 testCondition;           \
		void*                                 pythonDictionary;        \

	struct PythonShape { __PythonShape };
	
	
	/*---------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	PythonShape* PythonShape_New(
		Name                                  name,
		Dimension_Index                       dim,
		XYZ                                   centre, 
		double                                alpha,
		double                                beta,
		double                                gamma,
		char*                                 conditionFunction );

	PythonShape* _PythonShape_New(
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
	
	void _PythonShape_Init( void* pythonShape, char* conditionFunction ) ;
	void PythonShape_InitAll( 
		void*                                 pythonShape, 
		Dimension_Index                       dim, 
		Coord                                 centre,
		double                                alpha,
		double                                beta,
		double                                gamma,
		char*                                 conditionFunction );

	/* Stg_Class_Delete PythonShape implementation */
	void _PythonShape_Delete( void* pythonShape );
	void _PythonShape_Print( void* pythonShape, Stream* stream );
	#define PythonShape_Copy( self ) \
		(PythonShape*) Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define PythonShape_DeepCopy( self ) \
		(PythonShape*) Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _PythonShape_Copy( void* pythonShape, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	void* _PythonShape_DefaultNew( Name name ) ;
	void _PythonShape_Construct( void* shape, Stg_ComponentFactory* cf, void* data ) ;
	void _PythonShape_Build( void* pythonShape, void* data ) ;
	void _PythonShape_Initialise( void* pythonShape, void* data ) ;
	void _PythonShape_Execute( void* pythonShape, void* data );
	void _PythonShape_Destroy( void* pythonShape, void* data ) ;
	
	Bool _PythonShape_IsCoordInside( void* pythonShape, Coord coord ) ;
	double _PythonShape_CalculateVolume( void* pythonShape );

	/*---------------------------------------------------------------------------------------------------------------------
	** Public member functions
	*/
	
	/*---------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/
	
	
#endif 
#endif /* HAVE_PYTHON */

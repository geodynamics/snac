/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003, Victorian Partnership for Advanced Computing (VPAC) Ltd, 110 Victoria Street, Melbourne, 3053, Australia.
**
** Authors:
**	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
**	Patrick D. Sunter, Software Engineer, VPAC. (pds@vpac.org)
**	Luke J. Hodkinson, Computational Engineer, VPAC. (lhodkins@vpac.org)
**	Siew-Ching Tan, Software Engineer, VPAC. (siew@vpac.org) ) 
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
** $Id: Intersection.c 3869 2006-10-16 13:42:59Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <Base/Base.h>
#include <Discretisation/Geometry/Geometry.h>

#include "types.h"
#include "ShapeClass.h"
#include "Intersection.h"

#include <assert.h>
#include <string.h>
#include <math.h>

/* Textual name of this class */
const Type Intersection_Type = "Intersection";

/*----------------------------------------------------------------------------------------------------------------------------------
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
		Bool*                                 isComplement
		)
{
	Intersection* self = (Intersection*)_Intersection_DefaultNew( name );

	Intersection_InitAll( 
		self, 
		dim,
		centre,
		alpha,
		beta,
		gamma,
		shapeList,
		shapeCount,
		isComplement);
	return self;
}

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
		Name                                  name )
{
	Intersection* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(Intersection) );
	self = (Intersection*)_Stg_Shape_New( 
			_sizeOfSelf,
			type,
			_delete,
			_print,
			_copy,
			_defaultConstructor,
			_construct,
			_build,
			_initialise,
			_execute,
			_destroy,		
			_isCoordInside ,
			_calculateVolume,
			name );
	
	/* General info */

	/* Virtual Info */
	self->_isCoordInside = _isCoordInside;
	
	return self;
}

void _Intersection_Init( void* intersection,  Stg_Shape** shapeList, Index shapeCount, Bool* isComplement ) {
	Intersection* self = (Intersection*)intersection;
	
	self->shapeList    = Memory_Alloc_Array( Stg_Shape* , shapeCount , "shapeList" );
	self->isComplement = Memory_Alloc_Array( Bool,        shapeCount , "isComplement" );

	memcpy( self->shapeList , shapeList, sizeof(Stg_Shape*) * shapeCount );
	memcpy( self->isComplement , isComplement, sizeof(Bool) * shapeCount );
	self->shapeCount = shapeCount;
}


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
		)
{
	Intersection* self = (Intersection*)intersection;

	Stg_Shape_InitAll( self, dim, centre, alpha, beta, gamma);
	_Intersection_Init( self, shapeList, shapeCount, isComplement );
}
	

/*------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _Intersection_Delete( void* intersection ) {
	Intersection*       self = (Intersection*)intersection;

	Memory_Free( self->shapeList );
	Memory_Free( self->isComplement );

	/* Delete parent */
	_Stg_Shape_Delete( self );
}


void _Intersection_Print( void* intersection, Stream* stream ) {
	Intersection* self = (Intersection*)intersection;
	
	/* Print parent */
	_Stg_Shape_Print( self, stream );
}



void* _Intersection_Copy( void* intersection, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	Intersection*	self = (Intersection*)intersection;
	Intersection*	newIntersection;
	
	newIntersection = (Intersection*)_Stg_Shape_Copy( self, dest, deep, nameExt, ptrMap );

	newIntersection->shapeList = Memory_Alloc_Array( Stg_Shape*, self->shapeCount, "shapeList" );
	memcpy( newIntersection->shapeList , self->shapeList, sizeof(Stg_Shape*) * self->shapeCount );
	
	newIntersection->isComplement = Memory_Alloc_Array( Bool , self->shapeCount, "isComplement" );
	memcpy( newIntersection->isComplement , self->isComplement, sizeof(Bool) * self->shapeCount );

	newIntersection->isComplement = self->isComplement;
	newIntersection->shapeList = self->shapeList;
	newIntersection->shapeCount = self->shapeCount;
	
	return (void*)newIntersection;
}

void* _Intersection_DefaultNew( Name name ) {
	return (void*) _Intersection_New(
			sizeof(Intersection),
			Intersection_Type,
			_Intersection_Delete,
			_Intersection_Print,
			_Intersection_Copy,
			_Intersection_DefaultNew,
			_Intersection_Construct,
			_Intersection_Build,
			_Intersection_Initialise,
			_Intersection_Execute,
			_Intersection_Destroy,
			_Intersection_IsCoordInside,
			_Intersection_CalculateVolume,
			name );
}


void _Intersection_Construct( void* intersection, Stg_ComponentFactory* cf, void* data ) {
	Intersection*	        self       = (Intersection*)intersection;
	Index                   shapeCount;
	Stg_Shape**             shapeList;
	Bool*                   isComplement;
	Index                   shape_I;
	Dictionary_Entry_Value* optionsList;
	Dictionary*             dictionary = Dictionary_GetDictionary( cf->componentDict, self->name );
	char*                   nameShape;
	Stream*                 stream     = Journal_Register( Info_Type, CURR_MODULE_NAME );
	
	_Stg_Shape_Construct( self, cf, data );

	optionsList = Dictionary_Get( dictionary, "shapes" );
/*	Journal_Firewall( vertexList != NULL, 
		Journal_Register( Error_Type, self->type ),
		"In func %s: vertexList specifying the intersection is NULL.\n",
		__func__);

*/	shapeCount = Dictionary_Entry_Value_GetCount(optionsList);

	/* Allocate space */
	shapeList     = Memory_Alloc_Array( Stg_Shape* , shapeCount, "Shape Array" );
	isComplement  = Memory_Alloc_Array( Bool, shapeCount, "Complement Array" );
	memset( shapeList,     0, shapeCount * sizeof(Stg_Shape*) );
	memset( isComplement,  0, shapeCount * sizeof(Bool) );
	
	// TALK ABOUT WITH ROB, will there be some Flag for Union or Intersection type in the XML file.
	// If so do we need another variable on the intersection class which is this flag, and then use that flag 
	// to determine the method of test if a particle is inside or outside the shape


	
	Stream_Indent( stream );
	for ( shape_I = 0 ; shape_I < shapeCount ; shape_I++) { 
		// gets the textual name corresponding to the shape elements //	
		nameShape = Dictionary_Entry_Value_AsString( Dictionary_Entry_Value_GetElement( optionsList, shape_I));

		if ( nameShape[0] == '!' ) {
			shapeList[ shape_I ] =  Stg_ComponentFactory_ConstructByName( cf, &nameShape[1], Stg_Shape, True, data ) ;
			isComplement[ shape_I ] = True;
		}
		else {
			shapeList[ shape_I ] =  Stg_ComponentFactory_ConstructByName( cf, nameShape, Stg_Shape, True, data ) ;
			isComplement[ shape_I ] = False;
		}
		
	}
	Stream_UnIndent( stream );

	_Intersection_Init( self, shapeList, shapeCount, isComplement );

	Memory_Free( shapeList );
	Memory_Free( isComplement );
}

void _Intersection_Build( void* intersection, void* data ) {
	Intersection*	self = (Intersection*)intersection;

	_Stg_Shape_Build( self, data );
}
void _Intersection_Initialise( void* intersection, void* data ) {
	Intersection*	self = (Intersection*)intersection;
	
	_Stg_Shape_Initialise( self, data );
}
void _Intersection_Execute( void* intersection, void* data ) {
	Intersection*	self = (Intersection*)intersection;
	
	_Stg_Shape_Execute( self, data );
}
void _Intersection_Destroy( void* intersection, void* data ) {
	Intersection*	self = (Intersection*)intersection;
	
	_Stg_Shape_Destroy( self, data );
}

/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/
	
/*---------------------------------------------------------------------------------------------------------------------
** Private Member functions
*/
Bool _Intersection_IsCoordInside( void* intersection, Coord coord ) {
	Intersection*   self           = (Intersection*)intersection;
	Index           shapeCount     = self->shapeCount;
	Index           shape_I;
	Bool            value;

	for( shape_I = 0 ; shape_I < shapeCount ; shape_I++ ) {
		value = Stg_Shape_IsCoordInside( self->shapeList[ shape_I ], coord );

		
		if ( self->isComplement[ shape_I ] )
			value = !value;
			
		if ( ! value )
			return False;
	}
	return True;
}	

double _Intersection_CalculateVolume( void* intersection ) {
	assert( 0 /* not impossible as long as you have other shapes with volum */ );
	return 0.0;
}
	

/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/


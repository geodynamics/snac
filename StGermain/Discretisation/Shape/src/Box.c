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
** $Id: Box.c 3869 2006-10-16 13:42:59Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <Base/Base.h>
#include <Discretisation/Geometry/Geometry.h>

#include "types.h"
#include "ShapeClass.h"
#include "Box.h"

#include <assert.h>
#include <string.h>
#include <math.h>


/* Textual name of this class */
const Type Box_Type = "Box";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/
Box* Box_New(
		Name                                  name,
		Dimension_Index                       dim,
		XYZ                                   centre, 
		double                                alpha,
		double                                beta,
		double                                gamma,
		XYZ                                   width )
{
	Box* self = (Box*) _Box_DefaultNew( name );

	Box_InitAll( 
		self, 
		dim,
		centre,
		alpha,
		beta,
		gamma,
		width) ;
	return self;
}

Box* _Box_New(
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
	Box* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(Box) );
	self = (Box*)_Stg_Shape_New( 
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
			_isCoordInside,
			_calculateVolume,
			name );
	
	/* General info */

	/* Virtual Info */
	self->_isCoordInside = _isCoordInside;
	
	return self;
}

void _Box_Init( void* box, XYZ width ) {
	Box* self = (Box*)box;
	
	memcpy( self->width, width, sizeof(XYZ));
}


void Box_InitAll( 
		void*                                 box, 
		Dimension_Index                       dim, 
		Coord                                 centre,
		double                                alpha,
		double                                beta,
		double                                gamma,
		XYZ                                   width) 
{
	Box* self = (Box*)box;

	Stg_Shape_InitAll( self, dim, centre, alpha, beta, gamma );
	_Box_Init( self, width );
}
	

/*------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _Box_Delete( void* box ) {
	Box* self = (Box*)box;
	
	/* Delete parent */
	_Stg_Shape_Delete( self );
}


void _Box_Print( void* box, Stream* stream ) {
	Box* self = (Box*)box;
	
	/* Print parent */
	_Stg_Shape_Print( self, stream );
}



void* _Box_Copy( void* box, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	Box*	self = (Box*)box;
	Box*	newBox;
	
	newBox = (Box*)_Stg_Shape_Copy( self, dest, deep, nameExt, ptrMap );
	
	memcpy( newBox->width, self->width, sizeof(XYZ));
	
	return (void*)newBox;
}

void* _Box_DefaultNew( Name name ) {
	return (void*) _Box_New(
			sizeof(Box),
			Box_Type,
			_Box_Delete,
			_Box_Print,
			_Box_Copy,
			_Box_DefaultNew,
			_Box_Construct,
			_Box_Build,
			_Box_Initialise,
			_Box_Execute,
			_Box_Destroy,
			_Box_IsCoordInside,
			_Box_CalculateVolume,
			name );
}


void _Box_Construct( void* box, Stg_ComponentFactory* cf, void* data ) {
	Box*	             self          = (Box*) box;
	Dictionary*          dictionary    = Dictionary_GetDictionary( cf->componentDict, self->name );
	XYZ                  width;
	double               start, end;
	Dictionary_Entry_Key startKey      = StG_Strdup("startX");
	Dictionary_Entry_Key endKey        = StG_Strdup("endX");
	Dictionary_Entry_Key widthKey      = StG_Strdup("widthX");
	char*                startCharPtr  = strchr( startKey, 'X' );
	char*                endCharPtr    = strchr( endKey, 'X' );
	char*                widthCharPtr  = strchr( widthKey, 'X' );
	char                 axisLetters[] = {'X','Y','Z'};
	Dimension_Index      dim_I;

	_Stg_Shape_Construct( self, cf, data );

	for ( dim_I = 0 ; dim_I < 3 ; dim_I++ ) {
		*startCharPtr = axisLetters[ dim_I ];
		*endCharPtr   = axisLetters[ dim_I ];
		*widthCharPtr = axisLetters[ dim_I ];

		/* Check to see whether the user wants to specify the start and end explicitly */
		if ( Dictionary_Get( dictionary, startKey ) && Dictionary_Get( dictionary, endKey ) ) {
			start = Stg_ComponentFactory_GetDouble( cf, self->name, startKey, 0.0 );
			end   = Stg_ComponentFactory_GetDouble( cf, self->name, endKey,   0.0 );

			width[ dim_I ] = end - start;
			self->centre[ dim_I ] = start + 0.5 * width[dim_I];
		}
		else 
			width[ dim_I ] = Stg_ComponentFactory_GetDouble( cf, self->name, widthKey, 0.0 );
	}

	Memory_Free( startKey );
	Memory_Free( endKey );
	Memory_Free( widthKey );

	_Box_Init( self, width );
}

void _Box_Build( void* box, void* data ) {
	Box*	self = (Box*)box;

	_Stg_Shape_Build( self, data );
}
void _Box_Initialise( void* box, void* data ) {
	Box*	self = (Box*)box;
	
	_Stg_Shape_Initialise( self, data );
}
void _Box_Execute( void* box, void* data ) {
	Box*	self = (Box*)box;
	
	_Stg_Shape_Execute( self, data );
}
void _Box_Destroy( void* box, void* data ) {
	Box*	self = (Box*)box;
	
	_Stg_Shape_Destroy( self, data );
}

/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/
	
/*---------------------------------------------------------------------------------------------------------------------
** Private Member functions
*/

Bool _Box_IsCoordInside( void* box, Coord coord ) {
	Box*            self       = (Box*)box;
	Coord           newCoord;
	Dimension_Index dim_I;

	/* Transform coordinate into canonical reference frame */
	Stg_Shape_TransformCoord( self, coord, newCoord );
	
	for ( dim_I = 0 ; dim_I < self->dim ; dim_I++ ) {
		if ( fabs( newCoord[ dim_I ] ) > 0.5 * self->width[ dim_I ] ) 
			return False;
	}
	return True;
}

double _Box_CalculateVolume( void* box ) {
	Box* self = (Box*)box;
	Dimension_Index dim_I;
	double result;
	result = 1.0;
	for ( dim_I = 0; dim_I < self->dim; dim_I++ ) {
		result *= self->width[dim_I];
	}
	return result;
}


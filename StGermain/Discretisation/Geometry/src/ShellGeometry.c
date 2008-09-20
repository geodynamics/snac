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
** $Id: ShellGeometry.c 3882 2006-10-26 04:41:18Z KathleenHumble $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include <mpi.h>
#include "Base/Base.h"


#include "units.h"
#include "types.h"
#include "GeometryClass.h"
#include "ShellGeometry.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>


/* Textual name of this class */
const Type ShellGeometry_Type = "ShellGeometry";


/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

ShellGeometry* ShellGeometry_DefaultNew( Name name )
{
	return (ShellGeometry*)_ShellGeometry_New(
		sizeof(ShellGeometry), 
		ShellGeometry_Type, 
		_ShellGeometry_Delete, 
		_ShellGeometry_Print,
		NULL,
		(Stg_Component_DefaultConstructorFunction*)ShellGeometry_DefaultNew,
		_ShellGeometry_Construct,
		_ShellGeometry_Build,
		_ShellGeometry_Initialise,
		_ShellGeometry_Execute,
		_ShellGeometry_Destroy,
		name,
		False,
		_ShellGeometry_BuildPoints,
		_ShellGeometry_PointAt,
		NULL );
}

ShellGeometry* ShellGeometry_New(
		Name name,
		Dictionary*					dictionary )
{
	return _ShellGeometry_New( 
		sizeof(ShellGeometry), 
		ShellGeometry_Type, 
		_ShellGeometry_Delete, 
		_ShellGeometry_Print,
		NULL,
		(Stg_Component_DefaultConstructorFunction*)ShellGeometry_DefaultNew,
		_ShellGeometry_Construct,
		_ShellGeometry_Build,
		_ShellGeometry_Initialise,
		_ShellGeometry_Execute,
		_ShellGeometry_Destroy,
		name,
		True,
		_ShellGeometry_BuildPoints,
		_ShellGeometry_PointAt,
		dictionary );
}


void ShellGeometry_Init(
		ShellGeometry*					self,
		Name						name,
		Dictionary*					dictionary )
{
	/* General info */
	self->type = ShellGeometry_Type;
	self->_sizeOfSelf = sizeof(ShellGeometry);
	self->_deleteSelf = False;
	self->dictionary = dictionary;
	
	/* Virtual info */
	self->_delete = _ShellGeometry_Delete;
	self->_print = _ShellGeometry_Print;
	self->_copy = NULL;
	self->_defaultConstructor = (Stg_Component_DefaultConstructorFunction*)ShellGeometry_DefaultNew;
	self->_construct = _ShellGeometry_Construct;
	self->_build = _ShellGeometry_Build;
	self->_initialise = _ShellGeometry_Initialise;
	self->_execute = _ShellGeometry_Execute;
	self->_destroy = _ShellGeometry_Destroy;
	self->buildPoints = _ShellGeometry_BuildPoints;
	self->pointAt = _ShellGeometry_PointAt;

	_Stg_Class_Init( (Stg_Class*)self );
	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );
	_Stg_Component_Init( (Stg_Component*)self );

	_Geometry_Init( (Geometry*)self );
	
	/* ShellGeometry info */
	_ShellGeometry_Init( self );
}


ShellGeometry* _ShellGeometry_New(
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
		Dictionary*					dictionary )
{
	ShellGeometry* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(ShellGeometry) );
	self = (ShellGeometry*)_Geometry_New(
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
		name, 
		initFlag,
		buildPoints,
		pointAt,
		dictionary );
	
	/* General info */
	
	/* Virtual info */
	
	/* ShellGeometry info */
	if( initFlag ){
		_ShellGeometry_Init( self );
	}
	
	return self;
}

void _ShellGeometry_Init(
		ShellGeometry*					self )
{
	/* General and Virtual info should already be set */
	
	/* ShellGeometry info */
	self->isConstructed = True;
	self->size[0] = Dictionary_Entry_Value_AsUnsignedInt( Dictionary_GetDefault( self->dictionary, "meshSizeI",
		Dictionary_Entry_Value_FromUnsignedInt( 2 ) ) );
	self->size[1] = Dictionary_Entry_Value_AsUnsignedInt( Dictionary_GetDefault( self->dictionary, "meshSizeJ",
		Dictionary_Entry_Value_FromUnsignedInt( 2 ) ) );
	self->size[2] = Dictionary_Entry_Value_AsUnsignedInt( Dictionary_GetDefault( self->dictionary, "meshSizeK",
		Dictionary_Entry_Value_FromUnsignedInt( 2 ) ) );
	self->pointCount = self->size[0] * self->size[1] * self->size[2];
	assert( self->pointCount );
	
	self->min[0] = Dictionary_Entry_Value_AsDouble( Dictionary_GetDefault( self->dictionary, "minTheta",
		Dictionary_Entry_Value_FromDouble( 2.0 * M_PI / 3.0 ) ) );
	self->min[1] = Dictionary_Entry_Value_AsDouble( Dictionary_GetDefault( self->dictionary, "minPhi",
		Dictionary_Entry_Value_FromDouble( 2.0 * M_PI / 3.0 ) ) );
	self->min[2] = Dictionary_Entry_Value_AsDouble( Dictionary_GetDefault( self->dictionary, "minR",
		Dictionary_Entry_Value_FromDouble( 0.5f ) ) );
	self->max[0] = Dictionary_Entry_Value_AsDouble( Dictionary_GetDefault( self->dictionary, "maxTheta",
		Dictionary_Entry_Value_FromDouble( M_PI / 3.0 ) ) );
	self->max[1] = Dictionary_Entry_Value_AsDouble( Dictionary_GetDefault( self->dictionary, "maxPhi",
		Dictionary_Entry_Value_FromDouble( M_PI / 3.0 ) ) );
	self->max[2] = Dictionary_Entry_Value_AsDouble( Dictionary_GetDefault( self->dictionary, "maxR",
		Dictionary_Entry_Value_FromDouble( 1.0f ) ) );
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _ShellGeometry_Delete( void* shellGeometry ) {
	ShellGeometry* self = (ShellGeometry*)shellGeometry;
	
	/* Stg_Class_Delete parent */
	_Geometry_Delete( self );
}


void _ShellGeometry_Print( void* shellGeometry, Stream* stream ) {
	ShellGeometry* self = (ShellGeometry*)shellGeometry;
	
	/* Set the Journal for printing informations */
	Stream* shellGeometryStream;
	shellGeometryStream = Journal_Register( InfoStream_Type, "ShellGeometryStream" );

	/* Print parent */
	_Geometry_Print( self, stream );
	
	/* General info */
	Journal_Printf( stream, "ShellGeometry (ptr): (%p)\n", self );
	
	/* Virtual info */
	
	/* ShellGeometry info */
}

void _ShellGeometry_Construct( void* shellGeometry, Stg_ComponentFactory *cf, void* data ){
	
}
	
void _ShellGeometry_Build( void* shellGeometry, void *data ){
	
}
	
void _ShellGeometry_Initialise( void* shellGeometry, void *data ){
	
}
	
void _ShellGeometry_Execute( void* shellGeometry, void *data ){
	
}

void _ShellGeometry_Destroy( void* shellGeometry, void *data ){
	
}

void _ShellGeometry_BuildPoints( void* shellGeometry, Coord_List points )
{
	ShellGeometry*  self = (ShellGeometry*)shellGeometry;
	Index		i;
	
	for( i = 0; i < self->pointCount; i++ ) {
		IJK	ijk;
		XYZ     coords;
	
		ijk[0] = i % self->size[0];
		ijk[1] = (i / self->size[0]) % self->size[1];
		ijk[2] = i / (self->size[0] * self->size[1]);
	
		coords[0] = self->size[0] > 1 ? self->min[0] + (self->max[0] - self->min[0]) * ((double)ijk[0] / 
			(double)(self->size[0] - 1)) : self->min[0];
		coords[1] = self->size[1] > 1 ? self->min[1] + (self->max[1] - self->min[1]) * ((double)ijk[1] / 
			(double)(self->size[1] - 1)) : self->min[1];
		coords[2] = self->size[2] > 1 ? self->min[2] + (self->max[2] - self->min[2]) * ((double)ijk[2] / 
			(double)(self->size[2] - 1)) : self->min[2];
		
		points[i][0] = coords[2] * cos( coords[0] ) * sin( coords[1] );
		points[i][1] = coords[2] * sin( coords[0] ) * sin( coords[1] );
		points[i][2] = coords[2] * cos( coords[1] );
	}
}


void _ShellGeometry_PointAt( void* shellGeometry, Index index, Coord point )
{
	ShellGeometry* self = (ShellGeometry*)shellGeometry;
	
	if( index < self->pointCount ) {
		IJK	ijk;
		XYZ     coords;
	
		ijk[0] = index % self->size[0];
		ijk[1] = (index / self->size[0]) % self->size[1];
		ijk[2] = index / (self->size[0] * self->size[1]);
	
		coords[0] = self->size[0] > 1 ? self->min[0] + (self->max[0] - self->min[0]) * ((double)ijk[0] / 
			(double)(self->size[0] - 1)) : self->min[0];
		coords[1] = self->size[1] > 1 ? self->min[1] + (self->max[1] - self->min[1]) * ((double)ijk[1] / 
			(double)(self->size[1] - 1)) : self->min[1];
		coords[2] = self->size[2] > 1 ? self->min[2] + (self->max[2] - self->min[2]) * ((double)ijk[2] / 
			(double)(self->size[2] - 1)) : self->min[2];
		
		point[0] = coords[2] * cos( coords[0] ) * sin( coords[1] );
		point[1] = coords[2] * sin( coords[0] ) * sin( coords[1] );
		point[2] = coords[2] * cos( coords[1] );
	}
	else {
		point[0] = HUGE_VAL;
		point[1] = HUGE_VAL;
		point[2] = HUGE_VAL;
	}
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/


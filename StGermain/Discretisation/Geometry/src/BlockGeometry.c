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
** $Id: BlockGeometry.c 3882 2006-10-26 04:41:18Z KathleenHumble $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"


#include "units.h"
#include "types.h"
#include "GeometryClass.h"
#include "BlockGeometry.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>


/* Textual name of this class */
const Type BlockGeometry_Type = "BlockGeometry";


/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

BlockGeometry* BlockGeometry_DefaultNew( Name name )
{
	return (BlockGeometry*)_BlockGeometry_New(
		sizeof(BlockGeometry), 
		BlockGeometry_Type, 
		_BlockGeometry_Delete, 
		_BlockGeometry_Print,
		_BlockGeometry_Copy,
		(Stg_Component_DefaultConstructorFunction*)BlockGeometry_DefaultNew,
		_BlockGeometry_Construct,
		_BlockGeometry_Build,
		_BlockGeometry_Initialise,
		_BlockGeometry_Execute,
		_BlockGeometry_Destroy,
		name,
		False,
		_BlockGeometry_BuildPoints,
		_BlockGeometry_PointAt,
		NULL );
}

BlockGeometry* BlockGeometry_New(
		Name name,
		Dictionary*					dictionary )
{
	return _BlockGeometry_New( 
		sizeof(BlockGeometry), 
		BlockGeometry_Type, 
		_BlockGeometry_Delete, 
		_BlockGeometry_Print,
		_BlockGeometry_Copy,
		(Stg_Component_DefaultConstructorFunction*)BlockGeometry_DefaultNew,
		_BlockGeometry_Construct,
		_BlockGeometry_Build,
		_BlockGeometry_Initialise,
		_BlockGeometry_Execute,
		_BlockGeometry_Destroy,
		name,
		True,
		_BlockGeometry_BuildPoints,
		_BlockGeometry_PointAt,
		dictionary );
}


void BlockGeometry_Init(
		BlockGeometry*					self,
		Name						name,
		Dictionary*					dictionary )
{
	/* General info */
	self->type = BlockGeometry_Type;
	self->_sizeOfSelf = sizeof(BlockGeometry);
	self->_deleteSelf = False;
	self->dictionary = dictionary;
	
	/* Virtual info */
	self->_delete = _BlockGeometry_Delete;
	self->_print = _BlockGeometry_Print;
	self->_copy = _BlockGeometry_Copy;
	self->_defaultConstructor = (Stg_Component_DefaultConstructorFunction*)BlockGeometry_DefaultNew;
	self->_construct = _BlockGeometry_Construct;
	self->_build = _BlockGeometry_Build;
	self->_initialise = _BlockGeometry_Initialise;
	self->_execute = _BlockGeometry_Execute;
	self->_destroy = _BlockGeometry_Destroy;
	self->buildPoints = _BlockGeometry_BuildPoints;
	self->pointAt = _BlockGeometry_PointAt;
	
	_Stg_Class_Init( (Stg_Class*)self );
	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );
	_Stg_Component_Init( (Stg_Component*)self );
	
	_Geometry_Init( (Geometry*)self );
	
	/* BlockGeometry info */
	_BlockGeometry_Init( self, NULL );
}


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
		Dictionary*					dictionary )
{
	BlockGeometry* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(BlockGeometry) );
	self = (BlockGeometry*)_Geometry_New(
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
	
	/* BlockGeometry info */
	if( initFlag ){
		_BlockGeometry_Init( self, NULL );
	}
	
	return self;
}

void _BlockGeometry_Init( BlockGeometry* self, IJK size ) {
	/* General and Virtual info should already be set */
	
	/* BlockGeometry info */
	self->isConstructed = False;

	if ( size ) {
		memcpy( self->size, size, sizeof(IJK) );
	}
	else {
		self->size[ I_AXIS ] = Dictionary_GetUnsignedInt_WithDefault( self->dictionary, "meshSizeI", 2 );
		self->size[ J_AXIS ] = Dictionary_GetUnsignedInt_WithDefault( self->dictionary, "meshSizeJ", 2 );
		self->size[ K_AXIS ] = Dictionary_GetUnsignedInt_WithDefault( self->dictionary, "meshSizeK", 2 );
	}
	self->pointCount = self->size[I_AXIS] * self->size[J_AXIS] * self->size[K_AXIS];
	assert( self->pointCount );
	
	self->min[ I_AXIS ] = Dictionary_GetDouble_WithDefault( self->dictionary, "minX", 0.0f );
	self->min[ J_AXIS ] = Dictionary_GetDouble_WithDefault( self->dictionary, "minY", 0.0f );
	self->min[ K_AXIS ] = Dictionary_GetDouble_WithDefault( self->dictionary, "minZ", 0.0f );

	self->max[ I_AXIS ] = Dictionary_GetDouble_WithDefault( self->dictionary, "maxX", 1.0f );
	self->max[ J_AXIS ] = Dictionary_GetDouble_WithDefault( self->dictionary, "maxY", 1.0f );
	self->max[ K_AXIS ] = Dictionary_GetDouble_WithDefault( self->dictionary, "maxZ", 1.0f );
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _BlockGeometry_Delete( void* blockGeometry ) {
	BlockGeometry* self = (BlockGeometry*)blockGeometry;
	
	/* Stg_Class_Delete parent */
	_Geometry_Delete( self );
}


void _BlockGeometry_Print( void* blockGeometry, Stream* stream ) {
	BlockGeometry* self = (BlockGeometry*)blockGeometry;
	
	/* Set the Journal for printing informations */
	Stream* blockGeometryStream;
	blockGeometryStream = Journal_Register( InfoStream_Type, "BlockGeometryStream" );

	/* Print parent */
	_Geometry_Print( self, stream );
	
	/* General info */
	Journal_Printf( stream, "BlockGeometry (ptr): (%p)\n", self );
	
	/* Virtual info */
	
	/* BlockGeometry info */
}

void _BlockGeometry_Construct( void* blockGeometry, Stg_ComponentFactory *cf, void* data ){
	BlockGeometry*  self   = (BlockGeometry*)blockGeometry;
	IJK             size;
	int             shift;
	Dimension_Index dim;
	Dimension_Index dim_I;

	self->dictionary = cf->rootDict;

	/* Get Size from Dictionary */
	dim = Stg_ComponentFactory_GetRootDictUnsignedInt( cf, "dim", 0 );
		
	size[ I_AXIS ] = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, "sizeI", 1 );  
	size[ J_AXIS ] = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, "sizeJ", 1 );
	size[ K_AXIS ] = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, "sizeK", 1 );
	if ( dim == 2 )
		size[ K_AXIS ] = 1;

	/* Shift the size if nessesary */
	shift = Stg_ComponentFactory_GetInt( cf, self->name, "sizeShift", 0 );
	for ( dim_I = I_AXIS ; dim_I < dim ; dim_I++ )
		size[ dim_I ] += shift;
	
	_Geometry_Init( (Geometry*)self );
	_BlockGeometry_Init( self, size );
}
	
void _BlockGeometry_Build( void* blockGeometry, void *data ){
	
}
	
void _BlockGeometry_Initialise( void* blockGeometry, void *data ){
	
}
	
void _BlockGeometry_Execute( void* blockGeometry, void *data ){
	
}
	
void _BlockGeometry_Destroy( void* blockGeometry, void *data ){
	
}

void* _BlockGeometry_Copy( void* blockGeometry, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	BlockGeometry*	self = (BlockGeometry*)blockGeometry;
	BlockGeometry*	newBlockGeometry;
	Index		idx_I;
	
	newBlockGeometry = (BlockGeometry*)_Geometry_Copy( self, dest, deep, nameExt, ptrMap );
	
	for( idx_I = 0; idx_I < 3; idx_I++ ) {
		newBlockGeometry->size[idx_I] = self->size[idx_I];
		newBlockGeometry->min[idx_I] = self->min[idx_I];
		newBlockGeometry->max[idx_I] = self->max[idx_I];
	}
	
	return (void*)newBlockGeometry;
}


void _BlockGeometry_BuildPoints( void* blockGeometry, Coord_List points )
{
	BlockGeometry*  self = (BlockGeometry*)blockGeometry;
	Index		i;
	
	for( i = 0; i < self->pointCount; i++ ) {
		IJK	ijk;
	
		ijk[0] = i % self->size[0];
		ijk[1] = (i / self->size[0]) % self->size[1];
		ijk[2] = i / (self->size[0] * self->size[1]);
	
		points[i][0] = self->size[0] > 1 ? self->min[0] + (self->max[0] - self->min[0]) * ((double)ijk[0] / 
			(double)(self->size[0] - 1)) : self->min[0];
		points[i][1] = self->size[1] > 1 ? self->min[1] + (self->max[1] - self->min[1]) * ((double)ijk[1] / 
			(double)(self->size[1] - 1)) : self->min[1];
		points[i][2] = self->size[2] > 1 ? self->min[2] + (self->max[2] - self->min[2]) * ((double)ijk[2] / 
			(double)(self->size[2] - 1)) : self->min[2];
	}
}


void _BlockGeometry_PointAt( void* blockGeometry, Index index, Coord point )
{
	BlockGeometry* self = (BlockGeometry*)blockGeometry;
	
	if( index < self->pointCount ) {
		IJK	ijk;
	
		ijk[0] = index % self->size[0];
		ijk[1] = (index / self->size[0]) % self->size[1];
		ijk[2] = index / (self->size[0] * self->size[1]);
	
		point[0] = self->size[0] > 1 ? self->min[0] + (self->max[0] - self->min[0]) * ((double)ijk[0] / 
			(double)(self->size[0] - 1)) : self->min[0];
		point[1] = self->size[1] > 1 ? self->min[1] + (self->max[1] - self->min[1]) * ((double)ijk[1] / 
			(double)(self->size[1] - 1)) : self->min[1];
		point[2] = self->size[2] > 1 ? self->min[2] + (self->max[2] - self->min[2]) * ((double)ijk[2] / 
			(double)(self->size[2] - 1)) : self->min[2];
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


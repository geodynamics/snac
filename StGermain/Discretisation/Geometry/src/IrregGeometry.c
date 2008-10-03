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
** $Id: IrregGeometry.c 3882 2006-10-26 04:41:18Z KathleenHumble $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"


#include "units.h"
#include "types.h"
#include "GeometryClass.h"
#include "IrregGeometry.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>


/* Textual name of this class */
const Type IrregGeometry_Type = "IrregGeometry";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

IrregGeometry* IrregGeometry_DefaultNew( Name name )
{
	return (IrregGeometry*)_IrregGeometry_New(
		sizeof(IrregGeometry), 
		IrregGeometry_Type, 
		_IrregGeometry_Delete, 
		_IrregGeometry_Print,
		NULL,
		(Stg_Component_DefaultConstructorFunction*)IrregGeometry_DefaultNew,
		_IrregGeometry_Construct,
		_IrregGeometry_Build,
		_IrregGeometry_Initialise,
		_IrregGeometry_Execute,
		_IrregGeometry_Destroy,
		name,
		False,
		_IrregGeometry_BuildPoints,
		_IrregGeometry_PointAt,
		NULL, 
		NULL );
}

IrregGeometry* IrregGeometry_New(
		Name						name,
		Dictionary*					dictionary,
		Name						listKey )
{
	return _IrregGeometry_New( 
		sizeof(IrregGeometry), 
		IrregGeometry_Type, 
		_IrregGeometry_Delete, 
		_IrregGeometry_Print,
		NULL,
		(Stg_Component_DefaultConstructorFunction*)IrregGeometry_DefaultNew,
		_IrregGeometry_Construct,
		_IrregGeometry_Build,
		_IrregGeometry_Initialise,
		_IrregGeometry_Execute,
		_IrregGeometry_Destroy,
		name,
		True,
		_IrregGeometry_BuildPoints,
		_IrregGeometry_PointAt,
		dictionary,
		listKey );
}


void IrregGeometry_Init(
		IrregGeometry*					self,
		Name						name,
		Dictionary*					dictionary,
		Name						listKey )
{
	/* General info */
	self->type = IrregGeometry_Type;
	self->_sizeOfSelf = sizeof(IrregGeometry);
	self->_deleteSelf = False;
	self->dictionary = dictionary;
	
	/* Virtual info */
	self->_delete = _IrregGeometry_Delete;
	self->_print = _IrregGeometry_Print;
	self->_copy = NULL;
	self->_defaultConstructor = (Stg_Component_DefaultConstructorFunction*)IrregGeometry_DefaultNew;
	self->_construct = _IrregGeometry_Construct;
	self->_build = _IrregGeometry_Build;
	self->_initialise = _IrregGeometry_Initialise;
	self->_execute = _IrregGeometry_Execute;
	self->_destroy = _IrregGeometry_Destroy;
	self->buildPoints = _IrregGeometry_BuildPoints;
	self->pointAt = _IrregGeometry_PointAt;
	
	_Stg_Class_Init( (Stg_Class*)self );
	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );
	_Stg_Component_Init( (Stg_Component*)self );
	
	_Geometry_Init( (Geometry*)self );
	
	/* IrregGeometry info */
	_IrregGeometry_Init( self, listKey );
}


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
		Name						listKey )
{
	IrregGeometry* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(IrregGeometry) );
	self = (IrregGeometry*)_Geometry_New(
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
	
	/* IrregGeometry info */
	if( initFlag ){
		_IrregGeometry_Init( self, listKey );
	}
	
	return self;
}

void _IrregGeometry_Init(
		IrregGeometry*					self,
		Name						listKey )
{
	/* General and Virtual info should already be set */
	
	/* IrregGeometry info */
	Dictionary_Entry_Value*		list;
	Dictionary_Entry_Value*		point;
	Index				i;
	
	self->isConstructed = True;
	list = Dictionary_Get( self->dictionary, listKey );
	self->pointCount = Dictionary_Entry_Value_GetCount( list );
	self->pointTbl = Memory_Alloc_Array( Coord, self->pointCount, "IrregGeometry->pointTbl" );
	point = Dictionary_Entry_Value_GetFirstElement( list );
	
	for( i = 0; i < self->pointCount; i++ ) {
		self->pointTbl[i][0] = Dictionary_Entry_Value_AsDouble( Dictionary_Entry_Value_GetMember( point, "x") );
		self->pointTbl[i][1] = Dictionary_Entry_Value_AsDouble( Dictionary_Entry_Value_GetMember( point, "y") );
		self->pointTbl[i][2] = Dictionary_Entry_Value_AsDouble( Dictionary_Entry_Value_GetMember( point, "z") );
		point = point->next;
	}
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _IrregGeometry_Delete( void* irregGeometry ) {
	IrregGeometry* self = (IrregGeometry*)irregGeometry;
	
	/* Stg_Class_Delete parent */
	_Geometry_Delete( self );
}


void _IrregGeometry_Print( void* irregGeometry, Stream* stream ) {
	IrregGeometry* self = (IrregGeometry*)irregGeometry;
	
	/* Set the Journal for printing informations */
	Stream* irregGeometryStream;
	irregGeometryStream = Journal_Register( InfoStream_Type, "IrregGeometryStream" );

	/* Print parent */
	_Geometry_Print( self, stream );
	
	/* General info */
	Journal_Printf( stream, "IrregGeometry (ptr): (%p)\n", self );
	
	/* Virtual info */
	
	/* IrregGeometry info */
}

void _IrregGeometry_Construct( void* irregGeometry, Stg_ComponentFactory *cf, void* data ){
	IrregGeometry *self = (IrregGeometry*)irregGeometry;
	Dictionary *componentDict = NULL;
	Dictionary *thisComponentDict = NULL;
	Name listName = NULL;

	self->dictionary = cf->rootDict;
	componentDict = cf->componentDict;
	assert( componentDict );
	thisComponentDict = Dictionary_GetDictionary( componentDict, self->name );
	assert( thisComponentDict );
	
	listName = Dictionary_GetString( thisComponentDict, "ListName" );

	_Geometry_Init( (Geometry*)self );
	_IrregGeometry_Init( self, listName );
}
	
void _IrregGeometry_Build( void* irregGeometry, void *data ){
	
}
	
void _IrregGeometry_Initialise( void* irregGeometry, void *data ){
	
}
	
void _IrregGeometry_Execute( void* irregGeometry, void *data ){
	
}

void _IrregGeometry_Destroy( void* irregGeometry, void *data ){
	
}

void _IrregGeometry_BuildPoints( void* irregGeometry, Coord_List points )
{
	IrregGeometry*  self = (IrregGeometry*)irregGeometry;
	Index		i;
	
	for( i = 0; i < self->pointCount; i++ ) {
		points[i][0] = self->pointTbl[i][0];
		points[i][1] = self->pointTbl[i][1];
		points[i][2] = self->pointTbl[i][2];
	}
}


void _IrregGeometry_PointAt( void* irregGeometry, Index index, Coord point )
{
	IrregGeometry* self = (IrregGeometry*)irregGeometry;
	
	if( index < self->pointCount ) {
		point[0] = self->pointTbl[index][0];
		point[1] = self->pointTbl[index][1];
		point[2] = self->pointTbl[index][2];
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


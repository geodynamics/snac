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
** $Id: /local/local/Discretisation/Geometry/src/MeshGeometry.c 4177 2006-02-23T02:16:21.399539Z boo  $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"
#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"

#include "types.h"
#include "shortcuts.h"
#include "MeshDecomp.h"
#include "MeshClass.h"
#include "MeshGeometry.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>


/* Textual name of this class */
const Type MeshGeometry_Type = "MeshGeometry";


/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

MeshGeometry* MeshGeometry_DefaultNew( Name name )
{
	return (MeshGeometry*)_MeshGeometry_New(
		sizeof(MeshGeometry), 
		MeshGeometry_Type, 
		_MeshGeometry_Delete, 
		_MeshGeometry_Print,
		_MeshGeometry_Copy,
		(Stg_Component_DefaultConstructorFunction*)MeshGeometry_DefaultNew,
		_MeshGeometry_Construct,
		_MeshGeometry_Build,
		_MeshGeometry_Initialise,
		_MeshGeometry_Execute,
		_MeshGeometry_Destroy,
		name,
		False,
		_BlockGeometry_BuildPoints,
		_MeshGeometry_PointAt,
		NULL );
}

MeshGeometry* MeshGeometry_New(
		Name name,
		Dictionary*					dictionary )
{
	return _MeshGeometry_New( 
		sizeof(MeshGeometry), 
		MeshGeometry_Type, 
		_MeshGeometry_Delete, 
		_MeshGeometry_Print,
		_MeshGeometry_Copy,
		(Stg_Component_DefaultConstructorFunction*)MeshGeometry_DefaultNew,
		_MeshGeometry_Construct,
		_MeshGeometry_Build,
		_MeshGeometry_Initialise,
		_MeshGeometry_Execute,
		_MeshGeometry_Destroy,
		name,
		True,
		_BlockGeometry_BuildPoints,
		_MeshGeometry_PointAt,
		dictionary );
}


void MeshGeometry_Init(
		MeshGeometry*					self,
		Name						name,
		Dictionary*					dictionary )
{
	/* General info */
	self->type = MeshGeometry_Type;
	self->_sizeOfSelf = sizeof(MeshGeometry);
	self->_deleteSelf = False;
	self->dictionary = dictionary;
	
	/* Virtual info */
	self->_delete = _MeshGeometry_Delete;
	self->_print = _MeshGeometry_Print;
	self->_copy = _MeshGeometry_Copy;
	self->_defaultConstructor = (Stg_Component_DefaultConstructorFunction*)MeshGeometry_DefaultNew;
	self->_construct = _MeshGeometry_Construct;
	self->_build = _MeshGeometry_Build;
	self->_initialise = _MeshGeometry_Initialise;
	self->_execute = _MeshGeometry_Execute;
	self->_destroy = _MeshGeometry_Destroy;
	self->buildPoints = _BlockGeometry_BuildPoints;
	self->pointAt = _MeshGeometry_PointAt;
	
	_Stg_Class_Init( (Stg_Class*)self );
	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );
	_Stg_Component_Init( (Stg_Component*)self );

	_Geometry_Init( (Geometry*)self );
        _BlockGeometry_Init( (BlockGeometry*)self, NULL );
	
	/* MeshGeometry info */
	_MeshGeometry_Init( self );
}


MeshGeometry* _MeshGeometry_New(
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
	MeshGeometry* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(MeshGeometry) );
	self = (MeshGeometry*)_BlockGeometry_New(
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
	
	/* MeshGeometry info */
	if( initFlag ){
		_MeshGeometry_Init( self );
	}
	
	return self;
}

void _MeshGeometry_Init( MeshGeometry* self ) {
	/* General and Virtual info should already be set */
	
	/* MeshGeometry info */
        self->useMesh = False;
        self->mesh = NULL;
	self->decomp = NULL;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _MeshGeometry_Delete( void* meshGeometry ) {
	MeshGeometry* self = (MeshGeometry*)meshGeometry;
	
	/* Stg_Class_Delete parent */
	_BlockGeometry_Delete( self );
}


void _MeshGeometry_Print( void* meshGeometry, Stream* stream ) {
	MeshGeometry* self = (MeshGeometry*)meshGeometry;
	
	/* Set the Journal for printing informations */
	Stream* meshGeometryStream;
	meshGeometryStream = Journal_Register( InfoStream_Type, "MeshGeometryStream" );

	/* Print parent */
	_BlockGeometry_Print( self, stream );
	
	/* General info */
	Journal_Printf( stream, "MeshGeometry (ptr): (%p)\n", self );
	
	/* Virtual info */
	
	/* MeshGeometry info */
}

void _MeshGeometry_Construct( void* meshGeometry, Stg_ComponentFactory *cf, void* data ){
	MeshGeometry*	self = (MeshGeometry*)meshGeometry;
        Dictionary*	dict;

        _BlockGeometry_Construct( self, cf, data );

	/* Get Size from Dictionary */
	self->cf = cf;
        dict = Dictionary_Entry_Value_AsDictionary( Dictionary_Get( cf->componentDict, self->name ) );
        self->meshName = StG_Strdup( Dictionary_GetString( dict, "mesh" ) );
	self->decompName = StG_Strdup( Dictionary_GetString( dict, "decomp" ) );
}

void _MeshGeometry_Build( void* meshGeometry, void *data ){
	MeshGeometry*	self = (MeshGeometry*)meshGeometry;

	self->mesh = Stg_ComponentFactory_ConstructByName( self->cf, self->meshName, Mesh, True, data );
	self->decomp = Stg_ComponentFactory_ConstructByName( self->cf, self->decompName, MeshDecomp, True, data );
}
	
void _MeshGeometry_Initialise( void* meshGeometry, void *data ){
}
	
void _MeshGeometry_Execute( void* meshGeometry, void *data ){
	
}
	
void _MeshGeometry_Destroy( void* meshGeometry, void *data ){
	
}

void* _MeshGeometry_Copy( void* meshGeometry, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
        assert( 0 );

	return NULL;
}


void _MeshGeometry_PointAt( void* meshGeometry, Index index, Coord point ) {
	MeshGeometry*	self = (MeshGeometry*)meshGeometry;
	unsigned	mapped;

        if( !self->useMesh ) {
                _BlockGeometry_PointAt( self, index, point );
                return;
        }

	assert( self->mesh );

	if( self->mesh->nodeG2D )
		mapped = Mesh_NodeMapGlobalToDomain( self->mesh, index );
	else
		mapped = self->decomp->nodeMapGlobalToDomain( self->decomp, index );
	assert( mapped < self->decomp->nodeDomainCount );
	point[0] = self->mesh->nodeCoord[mapped][0];
	point[1] = self->mesh->nodeCoord[mapped][1];
	point[2] = self->mesh->nodeCoord[mapped][2];
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/


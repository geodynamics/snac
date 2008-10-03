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
** $Id: CellRemesher.c 2192 2004-10-15 02:45:38Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <mpi.h>

#include "Base/Base.h"
#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"
#include "Discretisation/Mesh/Mesh.h"

#include "types.h"
#include "Remesher.h"
#include "CellRemesher.h"


/* Textual name of this class */
const Type CellRemesher_Type = "CellRemesher";


/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

#define REMESHER_DEFARGS				\
	sizeof(CellRemesher),				\
	CellRemesher_Type,				\
	_CellRemesher_Delete,				\
	_CellRemesher_Print,				\
	NULL,						\
	(void*(*)(Name))_CellRemesher_DefaultNew,	\
	_CellRemesher_Construct,			\
	_CellRemesher_Build,				\
	_CellRemesher_Initialise,			\
	_CellRemesher_Execute,				\
	_CellRemesher_Destroy,				\
	name,						\
	False,						\
	_CellRemesher_SetMesh


CellRemesher* CellRemesher_New( Name name ) {
	return _CellRemesher_New( REMESHER_DEFARGS );
}


CellRemesher* _CellRemesher_New( CLASS_ARGS, 
				 COMPONENT_ARGS, 
				 REMESHER_ARGS )
{
	CellRemesher*	self;

	/* Allocate memory. */
	self = (CellRemesher*)_Remesher_New( _sizeOfSelf,
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
					      setMeshFunc );

	/* CellRemesher info */
	if( initFlag ) {
		_CellRemesher_Init( self );
	}

	return self;
}


void CellRemesher_Init( CellRemesher* self ) {
	assert( 0 ); /* TODO */
#if 0
	/* General info */
	self->type = CellRemesher_Type;
	self->_sizeOfSelf = sizeof(CellRemesher);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _CellRemesher_Delete;
	self->_print = _CellRemesher_Print;
	self->_copy = NULL;
	_Stg_Class_Init( (Stg_Class*)self );
	
	/* CellRemesher info */
	_CellRemesher_Init( self );
#endif
}


void _CellRemesher_Init( CellRemesher* self ) {
	/* CellRemesher info */
	memset( &self->nDims, 
		0, 
		(size_t)&self->cellMesh - (size_t)&self->nDims + sizeof(Mesh*) );
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _CellRemesher_Delete( void* cellRemesher ) {
	CellRemesher*	self = (CellRemesher*)cellRemesher;

	/* Delete the class itself */
	_CellRemesher_Free( self );

	/* Delete parent */
	_Remesher_Delete( cellRemesher );
}


void _CellRemesher_Print( void* cellRemesher, Stream* stream ) {
	CellRemesher*	self = (CellRemesher*)cellRemesher;
	Stream*		myStream;
	
	/* Set the Journal for printing informations */
	myStream = Journal_Register( InfoStream_Type, "CellRemesherStream" );

	/* Print parent */
	_Stg_Component_Print( self, stream );

	/* General info */
	Journal_Printf( myStream, "CellRemesher (ptr): (%p)\n", self );

	/* Virtual info */

	/* CellRemesher info */
}


CellRemesher* _CellRemesher_DefaultNew( Name name ) {
	return _CellRemesher_New( REMESHER_DEFARGS );
}


void _CellRemesher_Construct( void* cellRemesher, Stg_ComponentFactory* cf, void* data ) {
	CellRemesher*	self = (CellRemesher*)cellRemesher;
	Dictionary*	dict;
	char*		cmName;

	assert( self );
	assert( cf );
	assert( cf->componentDict );

	_Remesher_Construct( self, cf, data );

	/* Get the number of dimensions. */
	dict = Dictionary_Entry_Value_AsDictionary( Dictionary_Get( cf->componentDict, self->name ) );
	self->nDims = Dictionary_GetUnsignedInt( dict, "dims" );
	assert( self->nDims > 0 );

	/* Extract cell mesh. */
	cmName = Dictionary_GetString( dict, "cellMesh" );
	assert( cmName );
	self->cellMesh = Stg_ComponentFactory_ConstructByName( cf, cmName, Mesh, True, data );
}


void _CellRemesher_Build( void* cellRemesher, void* data ) {
	CellRemesher*	self = (CellRemesher*)cellRemesher;

	assert( self );

	if( !self->meshType ) {
		return;
	}

	assert( self->mesh );

	/* Build parent. */
	_Remesher_Build( self, data );

	/* Build the cell mesh. */
	Build( self->cellMesh, data, False );
}


void _CellRemesher_Initialise( void* cellRemesher, void* data ) {
	CellRemesher*	self = (CellRemesher*)cellRemesher;

	assert( self );

	/* Initialise parent. */
	_Remesher_Initialise( self, data );

	/* Initialise the cell mesh. */
	Initialise( self->cellMesh, data, False );
}


void _CellRemesher_Execute( void* cellRemesher, void* data ) {
	CellRemesher*	self = (CellRemesher*)cellRemesher;
	Coord*		meshCrds;
	Coord*		cellCrds;
	unsigned	e_i;

	assert( self );
	assert( self->mesh );
	/* TODO: remaining asserts */

	meshCrds = self->mesh->nodeCoord;
	cellCrds = self->cellMesh->nodeCoord;

	/* Center the cell mesh's node coordinates in the cell mesh's elements. We assume identical
	   ordering. */
	for( e_i = 0; e_i < self->cellMesh->elementLocalCount; e_i++ ) {
		unsigned	nElNodes = self->cellMesh->elementNodeCountTbl[e_i];
		unsigned*	elNodes = self->cellMesh->elementNodeTbl[e_i];
		double		fac = 1.0 / (double)nElNodes;
		unsigned	d_i;

		memset( meshCrds[e_i], 0, sizeof(Coord) );
		for( d_i = 0; d_i < self->nDims; d_i++ ) {
			unsigned	en_i;

			for( en_i = 0; en_i < nElNodes; en_i++ ) {
				meshCrds[e_i][d_i] += cellCrds[elNodes[en_i]][d_i];
			}
			meshCrds[e_i][d_i] *= fac;
		}
	}
}


void _CellRemesher_Destroy( void* cellRemesher, void* data ) {
	CellRemesher*	self = (CellRemesher*)cellRemesher;

	assert( self );

	/* TODO: If delete deletes, what does destroy do? */
}


void _CellRemesher_SetMesh( void* cellRemesher, Mesh* mesh ) {
	CellRemesher*	self = (CellRemesher*)cellRemesher;

	assert( self );

	/* Kill all internals. */
	_CellRemesher_Free( self );

	/* Store the mesh. */
	self->mesh = mesh;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

void _CellRemesher_Free( CellRemesher* self ) {
	assert( self );
}

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
** $Id: MeshGenerator.c 3584 2006-05-16 11:11:07Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"

#include "types.h"
#include "shortcuts.h"
#include "MeshClass.h"
#include "MeshGenerator.h"


/* Textual name of this class */
const Type MeshGenerator_Type = "MeshGenerator";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

MeshGenerator* _MeshGenerator_New( MESHGENERATOR_DEFARGS ) {
	MeshGenerator* self;
	
	/* Allocate memory */
	assert( sizeOfSelf >= sizeof(MeshGenerator) );
	self = (MeshGenerator*)_Stg_Component_New( STG_COMPONENT_PASSARGS );

	/* Virtual info */
	self->generateFunc = generateFunc;

	/* MeshGenerator info */
	_MeshGenerator_Init( self );

	return self;
}

void _MeshGenerator_Init( MeshGenerator* self ) {
	self->comm = MPI_COMM_WORLD;
	self->nMeshes = 0;
	self->meshes = NULL;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _MeshGenerator_Delete( void* meshGenerator ) {
	MeshGenerator*	self = (MeshGenerator*)meshGenerator;

	FreeArray( self->meshes );

	/* Delete the parent. */
	_Stg_Component_Delete( self );
}

void _MeshGenerator_Print( void* meshGenerator, Stream* stream ) {
	MeshGenerator*	self = (MeshGenerator*)meshGenerator;
	
	/* Set the Journal for printing informations */
	Stream* meshGeneratorStream;
	meshGeneratorStream = Journal_Register( InfoStream_Type, "MeshGeneratorStream" );

	/* Print parent */
	Journal_Printf( stream, "MeshGenerator (ptr): (%p)\n", self );
	_Stg_Component_Print( self, stream );
}

void* _MeshGenerator_Copy( void* meshGenerator, void* destProc_I, Bool deep, Name nameExt, PtrMap* ptrMap ) {
#if 0
	MeshGenerator*	self = (MeshGenerator*)meshGenerator;
	MeshGenerator*	newMeshGenerator;
	PtrMap*	map = ptrMap;
	Bool	ownMap = False;

	/* Damn me for making copying so difficult... what was I thinking? */
	
	/* We need to create a map if it doesn't already exist. */
	if( !map ) {
		map = PtrMap_New( 10 );
		ownMap = True;
	}
	
	newMeshGenerator = (MeshGenerator*)_Mesh_Copy( self, destProc_I, deep, nameExt, map );
	
	/* Copy the virtual methods here. */

	/* Deep or shallow? */
	if( deep ) {
	}
	else {
	}
	
	/* If we own the map, get rid of it here. */
	if( ownMap ) Stg_Class_Delete( map );
	
	return (void*)newMeshGenerator;
#endif

	return NULL;
}

void _MeshGenerator_Construct( void* meshGenerator, Stg_ComponentFactory* cf, void* data ) {
	MeshGenerator*		self = (MeshGenerator*)meshGenerator;
	Dictionary*		dict;
	Dictionary_Entry_Value*	meshList;
	Mesh*			mesh;

	assert( self );
	assert( cf );

	/* Rip out the components structure as a dictionary. */
	dict = Dictionary_Entry_Value_AsDictionary( Dictionary_Get( cf->componentDict, self->name ) );

	/* Set the communicator to a default. */
	MeshGenerator_SetComm( self, MPI_COMM_WORLD );

	/* Read the individual mesh if specified. */
	mesh = Stg_ComponentFactory_ConstructByKey( cf, self->name, "mesh", Mesh, False, data );
	if( mesh )
		MeshGenerator_AddMesh( self, mesh );

	/* Read the mesh list, if it's there. */
	meshList = Dictionary_Get( dict, "meshes" );
	if( meshList ) {
		unsigned	nMeshes;
		char*		name;
		unsigned	m_i;

		nMeshes = Dictionary_Entry_Value_GetCount( meshList );
		for( m_i = 0; m_i < nMeshes; m_i++ ) {
			Mesh*	mesh;

			name = Dictionary_Entry_Value_AsString( Dictionary_Entry_Value_GetElement( meshList, m_i ) );
			mesh = Stg_ComponentFactory_ConstructByName( cf, name, Mesh, True, data );
			MeshGenerator_AddMesh( self, mesh );
		}
	}

	/* Add to live component register. */
	LiveComponentRegister_Add( cf->LCRegister, (Stg_Component*)self );
}

void _MeshGenerator_Build( void* meshGenerator, void* data ) {
	MeshGenerator*	self = (MeshGenerator*)meshGenerator;
	unsigned	m_i;

	/* Sanity check. */
	assert( self );
	assert( !self->nMeshes || self->meshes );

	/* Generate each mesh in our list. */
	for( m_i = 0; m_i < self->nMeshes; m_i++ )
		MeshGenerator_Generate( self, self->meshes[m_i] );
}

void _MeshGenerator_Initialise( void* meshGenerator, void* data ) {
}

void _MeshGenerator_Execute( void* meshGenerator, void* data ) {
}

void _MeshGenerator_Destroy( void* meshGenerator, void* data ) {
}

/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void MeshGenerator_SetComm( void* meshGenerator, MPI_Comm comm ) {
	MeshGenerator*	self = (MeshGenerator*)meshGenerator;

	/* Sanity check. */
	assert( self );

	/* Should probably kill some stuff when I do this. Oh well. */
	self->comm = comm;
}

void MeshGenerator_AddMesh( void* meshGenerator, void* mesh ) {
	MeshGenerator*	self = (MeshGenerator*)meshGenerator;

	/* Sanity check. */
	assert( self );

	/* If not already allocated, allocate now. */
	if( !self->meshes ) {
		self->nMeshes = 1;
		self->meshes = Memory_Alloc_Array( Mesh*, 1, "MeshGenerator::meshes" );
	}
	else {
		/* Otherwise reallocate. */
		self->nMeshes++;
		self->meshes = Memory_Realloc_Array( self->meshes, Mesh*, self->nMeshes );
	}

	/* Add the new mesh. */
	self->meshes[self->nMeshes - 1] = mesh;
	((Mesh*)mesh)->generator = self;
}

/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

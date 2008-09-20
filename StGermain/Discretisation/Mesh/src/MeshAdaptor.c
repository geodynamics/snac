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
** $Id: MeshAdaptor.c 3584 2006-05-16 11:11:07Z PatrickSunter $
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
#include "MeshAdaptor.h"


/* Textual name of this class */
const Type MeshAdaptor_Type = "MeshAdaptor";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

MeshAdaptor* _MeshAdaptor_New( MESHADAPTOR_DEFARGS ) {
	MeshAdaptor* self;
	
	/* Allocate memory */
	assert( sizeOfSelf >= sizeof(MeshAdaptor) );
	self = (MeshAdaptor*)_MeshGenerator_New( MESHGENERATOR_PASSARGS );

	/* Virtual info */

	/* MeshAdaptor info */
	_MeshAdaptor_Init( self );

	return self;
}

void _MeshAdaptor_Init( MeshAdaptor* self ) {
	self->generator = NULL;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _MeshAdaptor_Delete( void* meshAdaptor ) {
	MeshAdaptor*	self = (MeshAdaptor*)meshAdaptor;

	/* Delete the parent. */
	_MeshGenerator_Delete( self );
}

void _MeshAdaptor_Print( void* meshAdaptor, Stream* stream ) {
	MeshAdaptor*	self = (MeshAdaptor*)meshAdaptor;
	
	/* Set the Journal for printing informations */
	Stream* meshAdaptorStream;
	meshAdaptorStream = Journal_Register( InfoStream_Type, "MeshAdaptorStream" );

	/* Print parent */
	Journal_Printf( stream, "MeshAdaptor (ptr): (%p)\n", self );
	_Stg_Component_Print( self, stream );
}

void* _MeshAdaptor_Copy( void* meshAdaptor, void* destProc_I, Bool deep, Name nameExt, PtrMap* ptrMap ) {
#if 0
	MeshAdaptor*	self = (MeshAdaptor*)meshAdaptor;
	MeshAdaptor*	newMeshAdaptor;
	PtrMap*	map = ptrMap;
	Bool	ownMap = False;

	/* Damn me for making copying so difficult... what was I thinking? */
	
	/* We need to create a map if it doesn't already exist. */
	if( !map ) {
		map = PtrMap_New( 10 );
		ownMap = True;
	}
	
	newMeshAdaptor = (MeshAdaptor*)_Mesh_Copy( self, destProc_I, deep, nameExt, map );
	
	/* Copy the virtual methods here. */

	/* Deep or shallow? */
	if( deep ) {
	}
	else {
	}
	
	/* If we own the map, get rid of it here. */
	if( ownMap ) Stg_Class_Delete( map );
	
	return (void*)newMeshAdaptor;
#endif

	return NULL;
}

void _MeshAdaptor_Construct( void* meshAdaptor, Stg_ComponentFactory* cf, void* data ) {
	MeshAdaptor*	self = (MeshAdaptor*)meshAdaptor;

	_MeshGenerator_Construct( self, cf, data );

	/* Read the source generator. */
	self->generator = Stg_ComponentFactory_ConstructByKey( cf, self->name, "sourceGenerator", 
							       MeshGenerator, True, data );
}

void _MeshAdaptor_Build( void* meshAdaptor, void* data ) {
	_MeshGenerator_Build( meshAdaptor, data );
}

void _MeshAdaptor_Initialise( void* meshAdaptor, void* data ) {
}

void _MeshAdaptor_Execute( void* meshAdaptor, void* data ) {
}

void _MeshAdaptor_Destroy( void* meshAdaptor, void* data ) {
}

/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void MeshAdaptor_SetGenerator( void* meshAdaptor, void* generator ) {
	MeshAdaptor*	self = (MeshAdaptor*)meshAdaptor;

	self->generator = generator;
}

/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

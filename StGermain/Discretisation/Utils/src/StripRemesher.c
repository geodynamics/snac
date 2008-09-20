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
** $Id: StripRemesher.c 2192 2004-10-15 02:45:38Z LukeHodkinson $
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
#include "SemiRegDeform.h"
#include "StripRemesher.h"


/* Textual name of this class */
const Type StripRemesher_Type = "StripRemesher";


/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

#define REMESHER_DEFARGS				\
	sizeof(StripRemesher),				\
	StripRemesher_Type,				\
	_StripRemesher_Delete,				\
	_StripRemesher_Print,				\
	NULL,						\
	(void*(*)(Name))_StripRemesher_DefaultNew,	\
	_StripRemesher_Construct,			\
	_StripRemesher_Build,				\
	_StripRemesher_Initialise,			\
	_StripRemesher_Execute,				\
	_StripRemesher_Destroy,				\
	name,						\
	False,						\
	_StripRemesher_SetMesh


StripRemesher* StripRemesher_New( Name name ) {
	return _StripRemesher_New( REMESHER_DEFARGS );
}


StripRemesher* _StripRemesher_New( CLASS_ARGS, 
				   COMPONENT_ARGS, 
				   REMESHER_ARGS )
{
	StripRemesher*	self;

	/* Allocate memory. */
	self = (StripRemesher*)_Remesher_New( _sizeOfSelf,
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

	/* StripRemesher info */
	if( initFlag ) {
		_StripRemesher_Init( self );
	}

	return self;
}


void StripRemesher_Init( StripRemesher* self ) {
	assert( 0 ); /* TODO */
#if 0
	/* General info */
	self->type = StripRemesher_Type;
	self->_sizeOfSelf = sizeof(StripRemesher);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _StripRemesher_Delete;
	self->_print = _StripRemesher_Print;
	self->_copy = NULL;
	_Stg_Class_Init( (Stg_Class*)self );
	
	/* StripRemesher info */
	_StripRemesher_Init( self );
#endif
}


void _StripRemesher_Init( StripRemesher* self ) {
	/* StripRemesher info */
	memset( &self->nDims, 
		0, 
		(size_t)&self->srd - (size_t)&self->nDims + sizeof(SemiRegDeform*) );
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _StripRemesher_Delete( void* stripRemesher ) {
	StripRemesher*	self = (StripRemesher*)stripRemesher;

	/* Delete the class itself */
	_StripRemesher_Free( self );

	/* Delete parent */
	_Stg_Component_Delete( stripRemesher );
}


void _StripRemesher_Print( void* stripRemesher, Stream* stream ) {
	StripRemesher*	self = (StripRemesher*)stripRemesher;
	Stream*		myStream;
	
	/* Set the Journal for printing informations */
	myStream = Journal_Register( InfoStream_Type, "StripRemesherStream" );

	/* Print parent */
	_Stg_Component_Print( self, stream );

	/* General info */
	Journal_Printf( myStream, "StripRemesher (ptr): (%p)\n", self );

	/* Virtual info */

	/* StripRemesher info */
}


StripRemesher* _StripRemesher_DefaultNew( Name name ) {
	return _StripRemesher_New( REMESHER_DEFARGS );
}


void _StripRemesher_Construct( void* stripRemesher, Stg_ComponentFactory* cf, void* data ) {
	StripRemesher*		self = (StripRemesher*)stripRemesher;
	Dictionary_Entry_Value*	dimLst;

	assert( self );
	assert( cf );
	assert( cf->componentDict );

	_Remesher_Construct( self, cf, data );

	/* Extract enabled dimensions. */
	dimLst = Dictionary_Entry_Value_GetMember( Dictionary_Get( cf->componentDict, self->name ), "dims" );
	if( dimLst ) {
		unsigned	dim_i;

		self->nDims = Dictionary_Entry_Value_GetCount( dimLst );
		self->deformDims = Memory_Alloc_Array_Unnamed( unsigned, self->nDims );
		for( dim_i = 0; dim_i < self->nDims; dim_i++ ) {
			self->deformDims[dim_i] = Dictionary_Entry_Value_AsBool( Dictionary_Entry_Value_GetElement( dimLst, dim_i ) );
		}
	}
}


void _StripRemesher_Build( void* stripRemesher, void* data ) {
	StripRemesher*	self = (StripRemesher*)stripRemesher;

	assert( self );

	if( !self->meshType ) {
		return;
	}

	assert( self->mesh );

	/* Build parent. */
	_Remesher_Build( self, data );

	/* Build the SRD. */
	StripRemesher_SetDims( self, self->deformDims );
}


void _StripRemesher_Initialise( void* stripRemesher, void* data ) {
	StripRemesher*	self = (StripRemesher*)stripRemesher;

	assert( self );

	/* Initialise parent. */
	_Remesher_Initialise( self, data );
}


void _StripRemesher_Execute( void* stripRemesher, void* data ) {
	StripRemesher*	self = (StripRemesher*)stripRemesher;

	assert( self );
	assert( self->mesh );
	/* TODO: remaining asserts */

	SemiRegDeform_Deform( self->srd );
}


void _StripRemesher_Destroy( void* stripRemesher, void* data ) {
	StripRemesher*	self = (StripRemesher*)stripRemesher;

	assert( self );

	/* TODO: If delete deletes, what does destroy do? */
}


void _StripRemesher_SetMesh( void* stripRemesher, Mesh* mesh ) {
	StripRemesher*	self = (StripRemesher*)stripRemesher;

	assert( self );

	/* Kill all internals. */
	_StripRemesher_Free( self );

	/* Store the mesh. */
	self->mesh = mesh;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void StripRemesher_SetDims( void* stripRemesher, Bool* dims ) {
	StripRemesher*	self = (StripRemesher*)stripRemesher;
	unsigned	d_i;

	assert( self );
	assert( self->mesh );
	assert( dims );

	if( self->srd ) {
		Stg_Class_Delete( self->srd );
	}

	self->srd = SemiRegDeform_New( "" );
	SemiRegDeform_SetMesh( self->srd, self->mesh );

	for( d_i = 0; d_i < self->srd->grm.nDims; d_i++ ) {
		IJK	ijkLow;
		IJK	ijkUpp;

		if( !dims[d_i] ) {
			continue;
		}

		_StripRemesher_BuildStrips( self->srd, 0, d_i, 
					    ijkLow, ijkUpp );
	}

	/* Build and initialise the SRD. */
	Build( self->srd, NULL, False );
	Initialise( self->srd, NULL, False );
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

void _StripRemesher_Free( StripRemesher* self ) {
	assert( self );

	if( self->srd ) {
		Stg_Class_Delete( self->srd );
		self->srd = NULL;
	}
}


void _StripRemesher_BuildStrips( SemiRegDeform* srd, unsigned dim, unsigned deformDim, 
				 IJK ijkLow, IJK ijkUpp )
{
	unsigned	n_i;

	if( dim < srd->grm.nDims ) {
		if( dim != deformDim ) {
			for( n_i = 0; n_i < srd->grm.nNodes[dim]; n_i++ ) {
				ijkLow[dim] = n_i;
				ijkUpp[dim] = n_i;
				_StripRemesher_BuildStrips( srd, dim + 1, deformDim, 
							    ijkLow, ijkUpp );
			}
		}
		else {
			ijkLow[dim] = 0;
			ijkUpp[dim] = srd->grm.nNodes[dim] - 1;
			_StripRemesher_BuildStrips( srd, dim + 1, deformDim, 
						    ijkLow, ijkUpp );
		}
	}
	else {
		unsigned	low, upp;

		GRM_Project( &srd->grm, ijkLow, &low );
		GRM_Project( &srd->grm, ijkUpp, &upp );
		SemiRegDeform_AddStrip( srd, low, upp );
	}
}

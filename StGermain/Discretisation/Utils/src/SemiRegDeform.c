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
** $Id: SemiRegDeform.c 2192 2004-10-15 02:45:38Z LukeHodkinson $
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
#include "Sync.h"
#include "SemiRegDeform.h"


/* Textual name of this class */
const Type SemiRegDeform_Type = "SemiRegDeform";


/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

SemiRegDeform* SemiRegDeform_DefaultNew( Name name ) {
	return _SemiRegDeform_New( 
		sizeof(SemiRegDeform), 
		SemiRegDeform_Type, 
		_SemiRegDeform_Delete, 
		_SemiRegDeform_Print, 
		NULL, 
		(Stg_Component_DefaultConstructorFunction*)SemiRegDeform_DefaultNew, 
		_SemiRegDeform_Construct, 
		_SemiRegDeform_Build, 
		_SemiRegDeform_Initialise, 
		_SemiRegDeform_Execute, 
		_SemiRegDeform_Destroy, 
		name );
}


SemiRegDeform* SemiRegDeform_New( Name name ) {
	return _SemiRegDeform_New( 
		sizeof(SemiRegDeform), 
		SemiRegDeform_Type, 
		_SemiRegDeform_Delete, 
		_SemiRegDeform_Print, 
		NULL, 
		(Stg_Component_DefaultConstructorFunction*)SemiRegDeform_DefaultNew, 
		_SemiRegDeform_Construct, 
		_SemiRegDeform_Build, 
		_SemiRegDeform_Initialise, 
		_SemiRegDeform_Execute, 
		_SemiRegDeform_Destroy, 
		name );
}


SemiRegDeform* _SemiRegDeform_New( SizeT					_sizeOfSelf, 
				   Type						type,
				   Stg_Class_DeleteFunction*			_delete,
				   Stg_Class_PrintFunction*			_print, 
				   Stg_Class_CopyFunction*			_copy, 
				   Stg_Component_DefaultConstructorFunction*	_defaultConstructor,
				   Stg_Component_ConstructFunction*		_construct,
				   Stg_Component_BuildFunction*			_build,
				   Stg_Component_InitialiseFunction*		_initialise,
				   Stg_Component_ExecuteFunction*		_execute,
				   Stg_Component_DestroyFunction*		_destroy, 
				   Name						name )
{
	SemiRegDeform*	self;
	
	/* Allocate memory. */
	self = (SemiRegDeform*)_Stg_Component_New(
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
		NON_GLOBAL );
	
	/* General info */
	
	/* Virtual info */
	
	/* SemiRegDeform info */
	_SemiRegDeform_Init( self );
	
	return self;
}


void SemiRegDeform_Init( SemiRegDeform* self, Name name ) {
	/* General info */
	self->type = SemiRegDeform_Type;
	self->_sizeOfSelf = sizeof(SemiRegDeform);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _SemiRegDeform_Delete;
	self->_print = _SemiRegDeform_Print;
	self->_copy = NULL;
	self->_defaultConstructor = (Stg_Component_DefaultConstructorFunction*)SemiRegDeform_DefaultNew;
	self->_construct = _SemiRegDeform_Construct;
	self->_build = _SemiRegDeform_Build;
	self->_execute = _SemiRegDeform_Execute;
	self->_destroy = _SemiRegDeform_Destroy;

	_Stg_Class_Init( (Stg_Class*)self );
	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );
	_Stg_Component_Init( (Stg_Component*)self );
	
	/* SemiRegDeform info */
	_SemiRegDeform_Init( self );
}


void _SemiRegDeform_Init( SemiRegDeform* self ) {
	/* General and Virtual info should already be set */
	
	/* SemiRegDeform info */
	self->grm.mesh = NULL;
	self->nStrips = 0;
	self->beginInds = NULL;
	self->endInds = NULL;
	self->conDims = NULL;
	self->sync = NULL;
	self->nRemotes = 0;
	self->remotes = NULL;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _SemiRegDeform_Delete( void* srd ) {
	SemiRegDeform*	self = (SemiRegDeform*)srd;
	
	/* Delete the class itself */
	_SemiRegDeform_FreeInternal( self );
	
	/* Delete parent */
	_Stg_Class_Delete( self );
}


void _SemiRegDeform_Print( void* srd, Stream* stream ) {
	SemiRegDeform*	self = (SemiRegDeform*)srd;
	Stream*	myStream;
	
	/* Set the Journal for printing informations */
	myStream = Journal_Register( InfoStream_Type, "SemiRegDeformStream" );
	
	/* Print parent */
	_Stg_Class_Print( self, stream );
	
	/* General info */
	Journal_Printf( myStream, "SemiRegDeform (ptr): (%p)\n", self );
	
	/* Virtual info */
	
	/* SemiRegDeform info */
}


void _SemiRegDeform_Construct( void* srd, Stg_ComponentFactory* cf, void* data ) {
}


void _SemiRegDeform_Build( void* srd, void* data ) {
}


void _SemiRegDeform_Initialise( void* srd, void* data ) {
	SemiRegDeform*	self = (SemiRegDeform*)srd;

	assert( self );


	/*
	** Validate the provided strips.  No two strips can occupy any of the same nodes in the same
	** dimension.
	*/

	/* TODO */


	/*
	** Initialise the synchronisation.
	*/

	_SemiRegDeform_SyncInit( self );
}

	
void _SemiRegDeform_Execute( void* srd, void* data ) {
}


void _SemiRegDeform_Destroy( void* srd, void* data ) {
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void SemiRegDeform_SetMesh( void* srd, Mesh* mesh ) {
	SemiRegDeform*	self = (SemiRegDeform*)srd;

	assert( !self->isInitialised );

	_SemiRegDeform_FreeInternal( self );
	RegMesh_Generalise( mesh, &self->grm );
}


void SemiRegDeform_AddStrip( void* srd, unsigned begin, unsigned end ) {
	SemiRegDeform*	self = (SemiRegDeform*)srd;
	unsigned short	conDim;
	IJK		inds[2];
	Bool		store;
	unsigned	s_i;

	assert( self->grm.mesh );
	assert( !self->isInitialised );


	/*
	** Ensure the specified strip has not already been added.
	*/

	for( s_i = 0; s_i < self->nStrips; s_i++ ) {
		if( self->beginInds[s_i] == begin && self->endInds[s_i] == end ) {
			assert( 0 );
		}
	}


	/*
	** Ensure the specified strip is one dimensionally valid.
	*/

	{
		Bool		found;
		unsigned	d_i;

		GRM_Lift( &self->grm, begin, inds[0] );
		GRM_Lift( &self->grm, end, inds[1] );

		/* Find the one dimension that is not in-line. */
		found = False;
		for( d_i = 0; d_i < self->grm.nDims; d_i++ ) {
			if( inds[0][d_i] != inds[1][d_i] ) {
				/* Check if we have found multiple connected dimensions. */
				assert( found == False );

				found = True;
				conDim = d_i;
			}
		}
	}


	/*
	** If the strip has no points stored locally then don't store on this processor.
	*/

	{
		unsigned	len = inds[1][conDim] - inds[0][conDim] + 1;
		IJK		cur;
		unsigned	n_i;

		store = False;
		memcpy( cur, inds[0], sizeof(IJK) );
		for( n_i = 0; n_i < len; n_i++ ) {
			unsigned	gInd;

			GRM_Project( &self->grm, cur, &gInd );
			if( Mesh_NodeMapGlobalToLocal( self->grm.mesh, gInd ) < self->grm.mesh->nodeLocalCount ) {
				store = True;
				break;
			}
			cur[conDim]++;
		}
	}

	if( !store ) {
		return;
	}


	/*
	** Store.
	*/

	self->beginInds = Memory_Realloc_Array( self->beginInds, unsigned, self->nStrips + 1 );
	self->endInds = Memory_Realloc_Array( self->endInds, unsigned, self->nStrips + 1 );
	self->conDims = Memory_Realloc_Array( self->conDims, unsigned, self->nStrips + 1 );
	self->beginInds[self->nStrips] = begin;
	self->endInds[self->nStrips] = end;
	self->conDims[self->nStrips] = conDim;
	self->nStrips++;
}


void _SemiRegDeform_SyncInit( void* srd ) {
	SemiRegDeform*	self = (SemiRegDeform*)srd;
	unsigned	nGlobals;
	unsigned	nLocals;
	unsigned*	locals;
	unsigned	nRequired;
	unsigned*	required;
	unsigned	loc_i, strip_i;

	assert( self );


	/*
	** Setup the synchronisation component.
	*/

	self->sync = Sync_New( "SemiRegDeform" );

	/* How many globals do we have? */
	nGlobals = self->grm.mesh->nodeGlobalCount;

	/* Build the set of local nodes. */
	nLocals = self->grm.mesh->nodeLocalCount;
	locals = Memory_Alloc_Array( unsigned, nLocals, "SemiRegDeform" );
	for( loc_i = 0; loc_i < nLocals; loc_i++ ) {
		locals[loc_i] = Mesh_NodeMapLocalToGlobal( self->grm.mesh, loc_i );
	}

	/* Build required indices. */
	nRequired = 0;
	required = Memory_Alloc_Array( unsigned, self->nStrips * 2, "SemiRegDeform" );
	for( strip_i = 0; strip_i < self->nStrips; strip_i++ ) {
		if( Mesh_NodeMapGlobalToLocal( self->grm.mesh, self->beginInds[strip_i] ) >= 
		    self->grm.mesh->nodeLocalCount )
		{
			required[nRequired++] = self->beginInds[strip_i];
		}

		if( Mesh_NodeMapGlobalToLocal( self->grm.mesh, self->endInds[strip_i] ) >= 
		    self->grm.mesh->nodeLocalCount )
		{
			required[nRequired++] = self->endInds[strip_i];
		}
	}
	required = Memory_Realloc_Array( required, unsigned, nRequired );

	/* Negotiate sources. */
	Sync_Negotiate( self->sync, 
			nGlobals, 
			locals, nLocals, 
			NULL, 0, 
			required, nRequired, 
			self->grm.mesh->layout->decomp->communicator );

	/* Free arrays. */
	FreeArray( locals );
	FreeArray( required );

	/* Allocate for sources. */
	self->nRemotes = self->sync->netSource;
	self->remotes = Memory_Alloc_Array( Coord, self->nRemotes, "SemiRegDeform" );

	/* Initialise transfer. */
	Sync_SetSplitArrays( self->sync, 
			     sizeof(Coord), 
			     sizeof(Coord), self->grm.mesh->nodeCoord, 
			     sizeof(Coord), self->remotes );
}


#define GET_VAL( ind )						\
	(((ind) < self->grm.mesh->nodeLocalCount) ? self->grm.mesh->nodeCoord[ind] : \
	 self->remotes[ind - self->grm.mesh->nodeLocalCount])


void SemiRegDeform_Deform( void* srd ) {
	SemiRegDeform*	self = (SemiRegDeform*)srd;

	assert( self );


	/*
	** Actually deform the specified strips.
	*/

	/* Import remote values. */
	Sync_SendRecv( self->sync );

	/* Interpolate each strip. */
	{
		unsigned*	begin;
		unsigned*	end;
		unsigned	strip_i;

		/* Allocate for the dimensions. */
		begin = Memory_Alloc_Array( unsigned, self->grm.nDims, "SemiRegDeform" );
		end = Memory_Alloc_Array( unsigned, self->grm.nDims, "SemiRegDeform" );

		for( strip_i = 0; strip_i < self->nStrips; strip_i++ ) {
			unsigned	len;
			unsigned	conDim;
			double		first, step;
			unsigned	node_i;

			/* Extract the basics. */
			GRM_Lift( &self->grm, self->beginInds[strip_i], begin );
			GRM_Lift( &self->grm, self->endInds[strip_i], end );
			conDim = self->conDims[strip_i];
			len = end[conDim] - begin[conDim] + 1;
			assert( len > 1 );
			first = GET_VAL( Sync_MapGlobal( self->sync, self->beginInds[strip_i] ) )[conDim];
			step = GET_VAL( Sync_MapGlobal( self->sync, self->endInds[strip_i] ) )[conDim];
			step = (step - first) / (len - 1);

			/* Loop and interpolate. */
			for( node_i = 1; node_i < len - 1; node_i++ ) {
				unsigned	ind;

				begin[conDim]++;
				GRM_Project( &self->grm, begin, &ind );
				if( Sync_MapGlobal( self->sync, ind ) != (unsigned)-1 ) {
					GET_VAL( Sync_MapGlobal( self->sync, ind ) )[conDim] = 
						first + (double)node_i * step;
				}
			}
		}
		FreeArray( begin );
		FreeArray( end );
	}
}


void RegMesh_Generalise( Mesh* mesh, GRM* grm ) {
	HexaMD*		decomp;
	unsigned	d_i;

	assert( mesh );
	assert( mesh->layout );
	assert( mesh->layout->decomp );

	decomp = (HexaMD*)mesh->layout->decomp;

	assert( decomp->type == HexaMD_Type );
	assert( grm );


	/*
	** Remove any dependance on dimension, storing necessary maps and information in the GRM
	** (Generalised Regular Mesh) structure.
	*/

	/* Calculate the number of topological dimensions and their mapping. */
	for( grm->nDims = 0, d_i = 0; d_i < 3; d_i++ ) {
		if( decomp->nodeGlobal3DCounts[d_i] > 1 ) {
			grm->nNodes[grm->nDims] = decomp->nodeGlobal3DCounts[d_i];
			grm->nDims++;
		}
	}

	/* Calculate the basis. */
	grm->basis[0] = 1;
	for( d_i = 1; d_i < grm->nDims; d_i++ ) {
		grm->basis[d_i] = grm->basis[d_i - 1] * grm->nNodes[d_i - 1];
	}

	/* Store the mesh. */
	grm->mesh = mesh;
}


void GRM_Lift( GRM* grm, unsigned ind, unsigned* dimInds ) {
	unsigned	rem;
	unsigned	d_i;

	assert( grm );
	assert( grm->nDims <= 3 );
	assert( dimInds );


	/*
	** Take a one dimensional array index and lift it into a regular mesh topological
	** space.
	*/

	rem = ind;
	for( d_i = grm->nDims; d_i > 0; d_i-- ) {
		unsigned short	dimInd = d_i - 1;
		div_t		divRes;

		divRes = div( rem, grm->basis[dimInd] );
		dimInds[dimInd] = divRes.quot;
		rem = divRes.rem;

		/* Ensure this is a valid lifting. */
		assert( dimInds[dimInd] < grm->nNodes[dimInd] );
	}
}


void GRM_Project( GRM* grm, unsigned* dimInds, unsigned* ind ) {
	unsigned short	d_i;

	assert( grm );
	assert( grm->nDims > 0 && grm->nDims <= 3 );
	assert( dimInds );
	assert( ind );


	/*
	** Project an n-dimensional set of topological indices into a one-dimensional, unique space.
	*/

	*ind = 0;
	for( d_i = 0; d_i < grm->nDims; d_i++ ) {
		assert( dimInds[d_i] < grm->nNodes[d_i] );
		*ind += dimInds[d_i] * grm->basis[d_i];
	}
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

void _SemiRegDeform_FreeInternal( void* srd ) {
	SemiRegDeform*	self = (SemiRegDeform*)srd;

	KillArray( self->beginInds );
	KillArray( self->endInds );
	KillArray( self->conDims );
	KillArray( self->remotes );
	if( self->sync ) {
		Stg_Class_Delete( self->sync );
		self->sync = NULL;
	}
}

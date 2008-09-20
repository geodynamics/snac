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
** $Id: SurfaceAdaptor.c 3584 2006-05-16 11:11:07Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"

#include "types.h"
#include "shortcuts.h"
#include "Grid.h"
#include "Decomp.h"
#include "Decomp_Sync.h"
#include "MeshTopology.h"
#include "MeshClass.h"
#include "MeshGenerator.h"
#include "MeshAdaptor.h"
#include "SurfaceAdaptor.h"


typedef double (SurfaceAdaptor_DeformFunc)( SurfaceAdaptor* self, Mesh* mesh, 
					    unsigned* globalSize, unsigned vertex, unsigned* vertexInds );


/* Textual name of this class */
const Type SurfaceAdaptor_Type = "SurfaceAdaptor";


/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

SurfaceAdaptor* SurfaceAdaptor_New( Name name ) {
	return _SurfaceAdaptor_New( sizeof(SurfaceAdaptor), 
				    SurfaceAdaptor_Type, 
				    _SurfaceAdaptor_Delete, 
				    _SurfaceAdaptor_Print, 
				    NULL, 
				    (void* (*)(Name))_SurfaceAdaptor_New, 
				    _SurfaceAdaptor_Construct, 
				    _SurfaceAdaptor_Build, 
				    _SurfaceAdaptor_Initialise, 
				    _SurfaceAdaptor_Execute, 
				    _SurfaceAdaptor_Destroy, 
				    name, 
				    NON_GLOBAL, 
				    SurfaceAdaptor_Generate );
}

SurfaceAdaptor* _SurfaceAdaptor_New( SURFACEADAPTOR_DEFARGS ) {
	SurfaceAdaptor* self;
	
	/* Allocate memory */
	assert( sizeOfSelf >= sizeof(SurfaceAdaptor) );
	self = (SurfaceAdaptor*)_MeshGenerator_New( MESHADAPTOR_PASSARGS );

	/* Virtual info */

	/* SurfaceAdaptor info */
	_SurfaceAdaptor_Init( self );

	return self;
}

void _SurfaceAdaptor_Init( SurfaceAdaptor* self ) {
	self->surfaceType = SurfaceAdaptor_SurfaceType_Invalid;
	memset( &self->info, 0, sizeof(SurfaceAdaptor_SurfaceInfo) );
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _SurfaceAdaptor_Delete( void* adaptor ) {
	SurfaceAdaptor*	self = (SurfaceAdaptor*)adaptor;

	/* Delete the parent. */
	_MeshGenerator_Delete( self );
}

void _SurfaceAdaptor_Print( void* adaptor, Stream* stream ) {
	SurfaceAdaptor*	self = (SurfaceAdaptor*)adaptor;
	
	/* Set the Journal for printing informations */
	Stream* adaptorStream;
	adaptorStream = Journal_Register( InfoStream_Type, "SurfaceAdaptorStream" );

	/* Print parent */
	Journal_Printf( stream, "SurfaceAdaptor (ptr): (%p)\n", self );
	_MeshGenerator_Print( self, stream );
}

void _SurfaceAdaptor_Construct( void* adaptor, Stg_ComponentFactory* cf, void* data ) {
	SurfaceAdaptor*	self = (SurfaceAdaptor*)adaptor;
	Dictionary*	dict;
	char*		surfaceType;

	assert( self );
	assert( cf );

	/* Call parent construct. */
	_MeshAdaptor_Construct( self, cf, data );

	/* Rip out the components structure as a dictionary. */
	dict = Dictionary_Entry_Value_AsDictionary( Dictionary_Get( cf->componentDict, self->name ) );

	/* What kind of surface do we want? */
	surfaceType = Stg_ComponentFactory_GetString( cf, self->name, "surfaceType", "" );
	if( !strcmp( surfaceType, "wedge" ) ) {
		self->surfaceType = SurfaceAdaptor_SurfaceType_Wedge;
		self->info.wedge.offs = Stg_ComponentFactory_GetDouble( cf, self->name, "offset", 0.0 );
		self->info.wedge.grad = Stg_ComponentFactory_GetDouble( cf, self->name, "gradient", 0.5 );
	}
	else if( !strcmp( surfaceType, "sine" ) || !strcmp( surfaceType, "cosine" ) ) {
		Dictionary_Entry_Value*	originList;

		if( !strcmp( surfaceType, "sine" ) )
			self->surfaceType = SurfaceAdaptor_SurfaceType_Sine;
		else
			self->surfaceType = SurfaceAdaptor_SurfaceType_Cosine;

		originList = Dictionary_Get( dict, "origin" );
		if( originList ) {
			unsigned	nDims;
			unsigned	d_i;

			nDims = Dictionary_Entry_Value_GetCount( originList );
			for( d_i = 0; d_i < nDims; d_i++ ) {
				Dictionary_Entry_Value*	val;

				val = Dictionary_Entry_Value_GetElement( originList, d_i );
				self->info.trig.origin[d_i] = Dictionary_Entry_Value_AsDouble( val );
			}
		}
		else
			memset( self->info.trig.origin, 0, sizeof(double) * 2 );

		self->info.trig.amp = Stg_ComponentFactory_GetDouble( cf, self->name, "amplitude", 1.0 );
		self->info.trig.freq = Stg_ComponentFactory_GetDouble( cf, self->name, "frequency", 1.0 );
	}
	else
		_SurfaceAdaptor_Init( self );
}

void _SurfaceAdaptor_Build( void* adaptor, void* data ) {
	_MeshAdaptor_Build( adaptor, data );
}

void _SurfaceAdaptor_Initialise( void* adaptor, void* data ) {
	_MeshAdaptor_Initialise( adaptor, data );
}

void _SurfaceAdaptor_Execute( void* adaptor, void* data ) {
}

void _SurfaceAdaptor_Destroy( void* adaptor, void* data ) {
}

/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void SurfaceAdaptor_Generate( void* adaptor, void* _mesh ) {
	SurfaceAdaptor*			self = (SurfaceAdaptor*)adaptor;
	Mesh*				mesh = (Mesh*)_mesh;
	SurfaceAdaptor_DeformFunc*	deformFunc;
	unsigned*			gSize;
	Grid*				grid;
	unsigned*			inds;
	unsigned			d_i, n_i;

	/* Build base mesh, which is assumed to be cartesian. */
	MeshGenerator_Generate( self->generator, mesh );

	/* If we're not 2D or 3D, forget about it. */
	if( mesh->topo->nDims != 2 && mesh->topo->nDims != 3 )
		return;

	/* What kind of surface do we want? */
	switch( self->surfaceType ) {
	case SurfaceAdaptor_SurfaceType_Wedge:
		deformFunc = SurfaceAdaptor_Wedge;
		break;
	case SurfaceAdaptor_SurfaceType_Sine:
		deformFunc = SurfaceAdaptor_Sine;
		break;
	case SurfaceAdaptor_SurfaceType_Cosine:
		deformFunc = SurfaceAdaptor_Cosine;
		break;
	default:
		break;
	};

	/* Extract the cartesian information. */
	gSize = (unsigned*)ExtensionManager_Get( mesh->info, mesh, 
						 ExtensionManager_GetHandle( mesh->info, "cartesianGlobalSize" ) );

	/* Build grid and space for indices. */
	grid = Grid_New();
	Grid_SetNDims( grid, mesh->topo->nDims );
	for( d_i = 0; d_i < mesh->topo->nDims; d_i++ )
		gSize[d_i]++;
	Grid_SetSizes( grid, gSize );
	for( d_i = 0; d_i < mesh->topo->nDims; d_i++ )
		gSize[d_i]--;
	inds = Memory_Alloc_Array_Unnamed( unsigned, mesh->topo->nDims );

	/* Loop over domain nodes. */
	for( n_i = 0; n_i < MeshTopology_GetDomainSize( mesh->topo, MT_VERTEX ); n_i++ ) {
		unsigned	gNode;
		double		height;

		gNode = MeshTopology_DomainToGlobal( mesh->topo, MT_VERTEX, n_i );
		Grid_Lift( grid, gNode, inds );

		/* Calculate a height percentage. */
		height = (double)inds[1] / (double)(gSize[1] - 1);

		/* Deform this node. */
		mesh->nodeCoord[n_i][1] += height * deformFunc( self, mesh, gSize, n_i, inds );
	}

	/* Free resources. */
	FreeArray( inds );
	FreeObject( grid );
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

double SurfaceAdaptor_Wedge( SurfaceAdaptor* self, Mesh* mesh, 
			     unsigned* globalSize, unsigned vertex, unsigned* vertexInds )
{
	if( mesh->nodeCoord[vertex][0] >= self->info.wedge.offs )
		return (mesh->nodeCoord[vertex][0] - self->info.wedge.offs) * self->info.wedge.grad;
	else
		return 0.0;
}

double SurfaceAdaptor_Sine( SurfaceAdaptor* self, Mesh* mesh, 
			    unsigned* globalSize, unsigned vertex, unsigned* vertexInds )
{
	double	dx, dy;
	double	rad;

	dx = mesh->nodeCoord[vertex][0] - self->info.trig.origin[0];
	rad = dx * dx;
	if( mesh->topo->nDims == 3 ) {
		dy = mesh->nodeCoord[vertex][1] - self->info.trig.origin[1];
		rad += dy * dy;
	}
	rad = sqrt( rad );

	return self->info.trig.amp * sin( self->info.trig.freq * rad );
}

double SurfaceAdaptor_Cosine( SurfaceAdaptor* self, Mesh* mesh, 
			      unsigned* globalSize, unsigned vertex, unsigned* vertexInds )
{
	double	dx, dy;
	double	rad;

	dx = mesh->nodeCoord[vertex][0] - self->info.trig.origin[0];
	rad = dx * dx;
	if( mesh->topo->nDims == 3 ) {
		dy = mesh->nodeCoord[vertex][1] - self->info.trig.origin[1];
		rad += dy * dy;
	}
	rad = sqrt( rad );

	return self->info.trig.amp * cos( self->info.trig.freq * rad );
}

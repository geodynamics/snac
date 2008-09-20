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
** $Id: LineParticleLayout.c 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"
#include "Discretisation/Mesh/Mesh.h"
#include "Discretisation/Utils/Utils.h"

#include "types.h"
#include "shortcuts.h"
#include "ParticleLayout.h"
#include "GlobalParticleLayout.h"
#include "LineParticleLayout.h"

#include "SwarmClass.h"
#include "StandardParticle.h"
#include "CellLayout.h"
#include "ElementCellLayout.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

const Type LineParticleLayout_Type = "LineParticleLayout";

LineParticleLayout* LineParticleLayout_New(
		Name                                             name,
		Dimension_Index                                  dim,
		Particle_Index                                   totalInitialParticles,
		Index                                            vertexCount,
		Coord*                                           vertexList )
{
	LineParticleLayout* self = (LineParticleLayout*) _LineParticleLayout_DefaultNew( name );
	_LineParticleLayout_Init( self, dim, totalInitialParticles, vertexCount, vertexList );
	return self;
}

LineParticleLayout* _LineParticleLayout_New( 
		SizeT                                            _sizeOfSelf,
		Type                                             type,
		Stg_Class_DeleteFunction*                        _delete,
		Stg_Class_PrintFunction*                         _print,
		Stg_Class_CopyFunction*                          _copy, 
		Stg_Component_DefaultConstructorFunction*        _defaultConstructor,
		Stg_Component_ConstructFunction*                 _construct,
		Stg_Component_BuildFunction*                     _build,
		Stg_Component_InitialiseFunction*                _initialise,
		Stg_Component_ExecuteFunction*                   _execute,
		Stg_Component_DestroyFunction*                   _destroy,
		ParticleLayout_SetInitialCountsFunction*         _setInitialCounts,
		ParticleLayout_InitialiseParticlesFunction*      _initialiseParticles,
		GlobalParticleLayout_InitialiseParticleFunction* _initialiseParticle,
		Name                                             name,
		Bool                                             initFlag )
{
	LineParticleLayout* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof( LineParticleLayout ) );
	self = (LineParticleLayout*)_GlobalParticleLayout_New( 
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
		_setInitialCounts,
		_initialiseParticles,
		_initialiseParticle,
		name,
		initFlag,
		GlobalCoordSystem,
		False,
		0,
		0.0 );

	return self;
}


void _LineParticleLayout_Init( 
		void*                                            particleLayout,
		Dimension_Index                                  dim,
		Particle_Index                                   totalInitialParticles,
		Index                                            vertexCount,
		Coord*                                           vertexList )
{
	LineParticleLayout*     self        = (LineParticleLayout*) particleLayout;
	double                  totalLength = 0.0;
	Index                   segment_I;
	double                  length;
	
	assert( totalInitialParticles >= 2 );
	assert( vertexCount >= 2 );

	self->dim = dim;
	self->vertexCount = vertexCount;
	self->vertexList = Memory_Alloc_Array( Coord, vertexCount, "vertexList" );
	memcpy( self->vertexList, vertexList, sizeof(Coord) * vertexCount );

	self->segmentLengthList = Memory_Alloc_Array( double, vertexCount - 1, "segmentLengthList" );
	for ( segment_I = 0 ; segment_I < self->vertexCount - 1 ; segment_I++ ) {
		length = StGermain_DistanceBetweenPoints( self->vertexList[ segment_I + 1 ], self->vertexList[ segment_I ], dim );
		self->segmentLengthList[ segment_I ] = length;

		totalLength += length;
	}

	self->dx = totalLength/( (double) totalInitialParticles - 1.0 );
	
	_GlobalParticleLayout_Init( self, GlobalCoordSystem, False, totalInitialParticles, 0.0 );
}


void* _LineParticleLayout_DefaultNew( Name name ) {
	return (void*)_LineParticleLayout_New( 
			sizeof(LineParticleLayout),
			LineParticleLayout_Type,
			_LineParticleLayout_Delete,
			_LineParticleLayout_Print,
			_LineParticleLayout_Copy,
			_LineParticleLayout_DefaultNew,
			_LineParticleLayout_Construct,
			_LineParticleLayout_Build,
			_LineParticleLayout_Initialise,
			_LineParticleLayout_Execute,
			_LineParticleLayout_Destroy,
			_GlobalParticleLayout_SetInitialCounts,
			_GlobalParticleLayout_InitialiseParticles,
			_LineParticleLayout_InitialiseParticle,
			name,
			False );
}

	
void _LineParticleLayout_Delete( void* particleLayout ) {
	LineParticleLayout* self = (LineParticleLayout*)particleLayout;

	Memory_Free( self->vertexList );
	
	_GlobalParticleLayout_Delete( self );

}

void _LineParticleLayout_Print( void* particleLayout, Stream* stream ) {
	LineParticleLayout* self  = (LineParticleLayout*)particleLayout;
	
	/* General info */
	Journal_Printf( stream, "LineParticleLayout (ptr): %p:\n", self );
	Stream_Indent( stream );
	
	/* Parent class info */
	_GlobalParticleLayout_Print( self, stream );
	
	/* LineParticleLayout */
	/*Journal_PrintArray( stream, self->startCoord, 3 );*/
	/*Journal_PrintArray( stream, self->endCoord, 3 );*/
	
	Stream_UnIndent( stream );
}


void* _LineParticleLayout_Copy( void* particleLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	LineParticleLayout*		self                    = (LineParticleLayout*)particleLayout;
	LineParticleLayout*		newLineParticleLayout;
	
	newLineParticleLayout = _GlobalParticleLayout_Copy( self, dest, deep, nameExt, ptrMap );

	/*memcpy( newLineParticleLayout->startCoord, self->startCoord, sizeof(Coord) );*/
	/*memcpy( newLineParticleLayout->endCoord, self->endCoord, sizeof(Coord) );*/

	return (void*)newLineParticleLayout;
}


void _LineParticleLayout_Construct( void* particleLayout, Stg_ComponentFactory *cf, void* data ){
	LineParticleLayout*     self                    = (LineParticleLayout*) particleLayout;
	Particle_Index          totalInitialParticles;
	Dimension_Index         dim;
	Coord*                  vertexList;
	Index                   vertexCount;
	Index                   vertex_I;
	Dictionary*             dictionary;
	Stream*                 stream;
	Dictionary_Entry_Value* entry;
	Dictionary_Entry_Value* list;
	double*                 coord;
	
	dictionary              = Dictionary_GetDictionary( cf->componentDict, self->name );
	/*stream                  = cf->infoStream;*/
	stream                  = Journal_MyStream( Info_Type, self );

	dim = Stg_ComponentFactory_GetRootDictUnsignedInt( cf, "dim", 0 );

	/* Read list of verticies of each of the different line segments from the dictionary */
	list = Dictionary_Get( dictionary, "verticies" );
	vertexCount = Dictionary_Entry_Value_GetCount( list );
	vertexList = Memory_Alloc_Array( Coord , vertexCount, "Vertex Array" );
	memset( vertexList, 0, vertexCount * sizeof(Coord) );

	Stream_Indent( stream );
	for ( vertex_I = 0 ; vertex_I < vertexCount ; vertex_I++) { 
		entry = Dictionary_Entry_Value_GetElement( list, vertex_I );
		coord = vertexList[vertex_I];
		/* Read Vertex */
		coord[ I_AXIS ] = Dictionary_Entry_Value_AsDouble( Dictionary_Entry_Value_GetMember( entry, "x"));
		coord[ J_AXIS ] = Dictionary_Entry_Value_AsDouble( Dictionary_Entry_Value_GetMember( entry, "y"));
		if (dim == 3 ) {
			coord[ K_AXIS ] = Dictionary_Entry_Value_AsDouble( Dictionary_Entry_Value_GetMember( entry, "z"));
		}

		/* Print Position */
		Journal_PrintfL( stream, 2, "(%0.3g, %0.3g, %0.3g)\n", coord[ I_AXIS ], coord[ J_AXIS ], coord[ K_AXIS ] );
	}
	Stream_UnIndent( stream );

	totalInitialParticles = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, "totalInitialParticles", 2 );

	_LineParticleLayout_Init( self, dim, totalInitialParticles, vertexCount, vertexList );

	/* Delete this vertexList because _LineParticleLayout_Init made a copy of it */
	Memory_Free( vertexList );
}
	
void _LineParticleLayout_Build( void* particleLayout, void* data ) {
}
void _LineParticleLayout_Initialise( void* particleLayout, void* data ) {
}
void _LineParticleLayout_Execute( void* particleLayout, void* data ) {
}
void _LineParticleLayout_Destroy( void* particleLayout, void* data ) {
}

void _LineParticleLayout_InitialiseParticle( void* particleLayout, void* _swarm, Particle_Index newParticle_I, void* _particle )
{
	LineParticleLayout*        self             = (LineParticleLayout*)particleLayout;
	Axis                       axis_I;
	Index                      segment_I;
	double                     lengthFromVertex = self->dx * (double) newParticle_I;
	double                     factor;
	double                     segmentLength    = 0.0;
	GlobalParticle*            particle         = (GlobalParticle*)_particle;

	/* Find which segment the particle is in */
	for ( segment_I = 0 ; segment_I < self->vertexCount - 1 ; segment_I++ ) {
		segmentLength = self->segmentLengthList[ segment_I ];
		if ( lengthFromVertex < self->segmentLengthList[ segment_I ] )
			break;
		lengthFromVertex -= segmentLength;
	}
	
	/* Check if we are the last particle */
	if ( segment_I == self->vertexCount - 1 ) {
		memcpy( particle->coord, self->vertexList[ segment_I ], sizeof(Coord) );
		return;
	}

	factor = lengthFromVertex / segmentLength;

	memcpy( particle->coord, self->vertexList[ segment_I ], sizeof(Coord) );
	for ( axis_I = 0 ; axis_I < self->dim ; axis_I++ ) {
		particle->coord[axis_I] += factor 
			* ( self->vertexList[ segment_I + 1 ][ axis_I ] - self->vertexList[ segment_I ][ axis_I ] )  ;
	}
}


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
** $Id: GLMesh.c 2453 2004-12-21 04:49:34Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>

/* This file was renamed to a C++ file because of this header under PDT-TAU */
extern "C" {
#include <StGermain/StGermain.h>
}

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#if defined(__APPLE__) && defined(__MACH__)
	#include <OpenGL/gl.h>	// Header File For The OpenGL32 Library
	#include <OpenGL/glu.h>	// Header File For The GLu32 Library
#else
	#include <GL/gl.h>	// Header File For The OpenGL32 Library
	#include <GL/glu.h>	// Header File For The GLu32 Library
#endif

#include "types.h"
#include "GLMesh.h"


/* Textual name of this class */
const Type GLMesh_Type = "GLMesh";


/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

GLMesh* GLMesh_New( void ) {
	return _GLMesh_New( 
		sizeof(GLMesh), 
		GLMesh_Type, 
		_GLMesh_Delete, 
		_GLMesh_Print,
		NULL );
}


void GLMesh_Init(
		GLMesh*						self )
{
	/* General info */
	self->type = GLMesh_Type;
	self->_sizeOfSelf = sizeof(GLMesh);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _GLMesh_Delete;
	self->_print = _GLMesh_Print;
	self->_copy = NULL;
	
	/* GLMesh info */
	_GLMesh_Init( self );
}


GLMesh* _GLMesh_New(
		SizeT						_sizeOfSelf, 
		Type						type,
		Stg_Class_DeleteFunction*				_delete,
		Stg_Class_PrintFunction*				_print,
		Stg_Class_CopyFunction*				_copy )
{
	GLMesh* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(GLMesh) );
	self = (GLMesh*)_Stg_Class_New( _sizeOfSelf, type, _delete, _print, _copy );

	/* General info */
	
	/* Virtual info */
	
	/* GLMesh info */
	_GLMesh_Init( self );
	
	return self;
}

void _GLMesh_Init(
		GLMesh*						self )
{
	self->vertCnt = 0;
	self->verts = NULL;
	self->edgeCnt = 0;
	self->edges = NULL;
	
	self->rankCnt = 0;
	self->localEdgeCnts = NULL;
	self->localEdges = NULL;
	self->shadowEdgeCnts = NULL;
	self->shadowEdges = NULL;
	self->vacantEdgeCnts = NULL;
	self->vacantEdges = NULL;
	
	self->localColor[0] = 0.0;
	self->localColor[1] = 0.0;
	self->localColor[2] = 1.0;
	self->shadowColor[0] = 0.2;
	self->shadowColor[1] = 0.2;
	self->shadowColor[2] = 0.6;
	self->vacantColor[0] = 0.6;
	self->vacantColor[1] = 0.6;
	self->vacantColor[2] = 0.6;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _GLMesh_Delete( void* glMesh ) {
	GLMesh* self = (GLMesh*)glMesh;
	
	_GLMesh_Destroy( self );
	
	/* Stg_Class_Delete parent */
	_Stg_Class_Delete( self );
}


void _GLMesh_Print( void* glMesh, Stream* stream ) {
	GLMesh* self = (GLMesh*)glMesh;
	
	/* Set the Journal for printing informations */
	Stream* glMeshStream = Journal_Register( InfoStream_Type, "GLMeshStream" );

	/* Print parent */
	_Stg_Class_Print( self, stream );
	
	/* General info */
	Journal_Printf( glMeshStream, "GLMesh (ptr): (%p)\n", self );
	
	/* Virtual info */
	
	/* GLMesh info */
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public member functions
*/

void GLMesh_BuildFromMesh( void* glMesh, MeshLayout* mesh ) {
	GLMesh*			self = (GLMesh*)glMesh;
	Geometry*		geometry;
	Node_GlobalIndex	point_I;
	Edge_Index		edge_I;
	ElementLayout*		elementLayout;
	Partition_Index		rank_I;
	
	/* Ensure everything is already freed */
	_GLMesh_Destroy( self );
	
	/* Copy the vertices */
	self->vertCnt = mesh->nodeLayout->nodeCount;
	self->verts = Memory_Alloc_Array( GLdouble, self->vertCnt * 3, "GLMesh->verts" );
	assert( self->verts );
	
	geometry = mesh->elementLayout->geometry;
	for( point_I = 0; point_I < mesh->nodeLayout->nodeCount; point_I++ ) {
		unsigned	vert_I = point_I * 3;
		Coord		point;
		
		geometry->pointAt( geometry, point_I, point );
		self->verts[vert_I] = (GLdouble)point[0] - 0.5;
		self->verts[vert_I + 1] = (GLdouble)point[1] - 0.5;
		self->verts[vert_I + 2] = (GLdouble)point[2] - 0.5;
	}
	
	/* Build the edges */
	elementLayout = mesh->elementLayout;
	self->edgeCnt = elementLayout->edgeCount;
	self->edges = Memory_Alloc_Array( unsigned, self->edgeCnt * 2, "GLMesh->edges" );
	assert( self->edges );

	for( edge_I = 0; edge_I < elementLayout->edgeCount; edge_I++ ) {
		unsigned	glEdge_I = edge_I * 2;
		Edge		edge;
		
		elementLayout->edgeAt( elementLayout, edge_I, edge );
		self->edges[glEdge_I] = (unsigned)edge[0];
		self->edges[glEdge_I + 1] = (unsigned)edge[1];
	}
	
	/* Build local edge indices */
	self->rankCnt = mesh->decomp->procsInUse;
	self->localEdgeCnts = Memory_Alloc_Array( unsigned, self->rankCnt, "GLMesh->localEdgeCnts" );
	memset( self->localEdgeCnts, 0, sizeof(unsigned) * self->rankCnt );
	self->localEdges = Memory_Alloc_Array( unsigned*, self->rankCnt, "GLMesh->localEdges" );
	memset( self->localEdges, 0, sizeof(unsigned*) * self->rankCnt );
	self->shadowEdgeCnts = Memory_Alloc_Array( unsigned, self->rankCnt, "GLMesh->localEdges" );
	memset( self->shadowEdgeCnts, 0, sizeof(unsigned) * self->rankCnt );
	self->shadowEdges = Memory_Alloc_Array( unsigned*, self->rankCnt, "GLMesh->shadowEdges" );
	memset( self->shadowEdges, 0, sizeof(unsigned*) * self->rankCnt );
	self->vacantEdgeCnts = Memory_Alloc_Array( unsigned, self->rankCnt, "GLMesh->vacantEdgeCnts" );
	memset( self->vacantEdgeCnts, 0, sizeof(unsigned) * self->rankCnt );
	self->vacantEdges = Memory_Alloc_Array( unsigned*, self->rankCnt, "GLMesh->vacantEdges" );
	memset( self->vacantEdges, 0, sizeof(unsigned*) * self->rankCnt );
	
	for( rank_I = 0; rank_I < self->rankCnt; rank_I++ ) {
		_GLMesh_BuildLocalEdges( self, mesh, rank_I );
		_GLMesh_BuildShadowEdges( self, mesh, rank_I );
		_GLMesh_BuildVacantEdges( self, mesh, rank_I );
	}
}


void GLMesh_RenderGlobal( void* glMesh ) {
	GLMesh*		self = (GLMesh*)glMesh;
	unsigned	edge_I;
	unsigned	vert_I;
	
	glColor3f( self->localColor[0], self->localColor[1], self->localColor[2] );
	
	/* Render vertices */
	glBegin( GL_POINTS );
	for( vert_I = 0; vert_I < self->vertCnt * 3; vert_I += 3 ) {
		glVertex3dv( &self->verts[vert_I] );
	}
	glEnd();
	
	/* Render edges */
	glBegin( GL_LINES );
	for( edge_I = 0; edge_I < self->edgeCnt * 2; edge_I += 2 ) {
		unsigned	vert_I = self->edges[edge_I] * 3;
		unsigned	vert_J = self->edges[edge_I + 1] * 3;
		
		glVertex3dv( &self->verts[vert_I] );
		glVertex3dv( &self->verts[vert_J] );
	}
	glEnd();
}


void GLMesh_RenderLocal( void* glMesh, Partition_Index rank ) {
	GLMesh*		self = (GLMesh*)glMesh;
	unsigned	lEdge_I;
	
	assert( rank < self->rankCnt );
	
	if( !self->localEdgeCnts[rank] || !self->localEdges[rank] ) {
		return;
	}
	
	glColor3f( self->localColor[0], self->localColor[1], self->localColor[2] );
	
	/* Render vertices */
	/*glBegin( GL_POINTS );
	for( vert_I = 0; vert_I < self->vertCnt * 3; vert_I += 3 ) {
		glVertex3dv( &self->verts[vert_I] );
	}
	glEnd();*/
	
	/* Render edges */
	glBegin( GL_LINES );
	for( lEdge_I = 0; lEdge_I < self->localEdgeCnts[rank]; lEdge_I++ ) {
		unsigned	edge_I = self->localEdges[rank][lEdge_I] * 2;
		unsigned	vert_I = self->edges[edge_I] * 3;
		unsigned	vert_J = self->edges[edge_I + 1] * 3;
		
		glVertex3dv( &self->verts[vert_I] );
		glVertex3dv( &self->verts[vert_J] );
	}
	glEnd();
}


void GLMesh_RenderShadow( void* glMesh, Partition_Index rank ) {
	GLMesh*		self = (GLMesh*)glMesh;
	unsigned	sEdge_I;
	
	assert( rank < self->rankCnt );
	
	if( !self->shadowEdgeCnts[rank] || !self->shadowEdges[rank] ) {
		return;
	}
	
	glColor3f( self->shadowColor[0], self->shadowColor[1], self->shadowColor[2] );
	
	/* Render vertices */
	/*glBegin( GL_POINTS );
	for( vert_I = 0; vert_I < self->vertCnt * 3; vert_I += 3 ) {
		glVertex3dv( &self->verts[vert_I] );
	}
	glEnd();*/
	
	/* Render edges */
	glBegin( GL_LINES );
	for( sEdge_I = 0; sEdge_I < self->shadowEdgeCnts[rank]; sEdge_I++ ) {
		unsigned	edge_I = self->shadowEdges[rank][sEdge_I] * 2;
		unsigned	vert_I = self->edges[edge_I] * 3;
		unsigned	vert_J = self->edges[edge_I + 1] * 3;
		
		glVertex3dv( &self->verts[vert_I] );
		glVertex3dv( &self->verts[vert_J] );
	}
	glEnd();
}


void GLMesh_RenderVacant( void* glMesh, Partition_Index rank ) {
	GLMesh*		self = (GLMesh*)glMesh;
	unsigned	vEdge_I;
	
	assert( rank < self->rankCnt );
	
	if( !self->vacantEdgeCnts || !self->vacantEdgeCnts[rank] || !self->vacantEdges || !self->vacantEdges[rank] ) {
		return;
	}
	
	glColor3f( self->vacantColor[0], self->vacantColor[1], self->vacantColor[2] );
	
	/* Render vertices */
	/*glBegin( GL_POINTS );
	for( vert_I = 0; vert_I < self->vertCnt * 3; vert_I += 3 ) {
		glVertex3dv( &self->verts[vert_I] );
	}
	glEnd();*/
	
	/* Render edges */
	glBegin( GL_LINES );
	for( vEdge_I = 0; vEdge_I < self->vacantEdgeCnts[rank]; vEdge_I++ ) {
		unsigned	edge_I = self->vacantEdges[rank][vEdge_I] * 2;
		unsigned	vert_I = self->edges[edge_I] * 3;
		unsigned	vert_J = self->edges[edge_I + 1] * 3;
		
		glVertex3dv( &self->verts[vert_I] );
		glVertex3dv( &self->verts[vert_J] );
	}
	glEnd();
}


void GLMesh_RenderRank( void* glMesh, Partition_Index rank ) {
	GLMesh*		self = (GLMesh*)glMesh;
	
	GLMesh_RenderLocal( self, rank );
	GLMesh_RenderShadow( self, rank );
	GLMesh_RenderVacant( self, rank );
}


/*--------------------------------------------------------------------------------------------------------------------------
** Private Member functions
*/

void _GLMesh_BuildLocalEdges( void* glMesh, MeshLayout* mesh, Partition_Index rank ) {
	GLMesh*		self = (GLMesh*)glMesh;
	Index		localElementCnt;
	Index*		localElementSet;
	
	IndexSet_GetMembers( mesh->decomp->localElementSets[rank], &localElementCnt, &localElementSet );
	
	if( localElementCnt ) {
		if( self->localEdges[rank] ) {
			Memory_Free( self->localEdges );
		}
		
		self->localEdgeCnts[rank] = ElementLayout_BuildEdgeSubset( mesh->elementLayout, 
									   localElementCnt, 
									   localElementSet, 
									   &self->localEdges[rank] );
	}
	
	Memory_Free( localElementSet );
}


void _GLMesh_BuildShadowEdges( void* glMesh, MeshLayout* mesh, Partition_Index rank ) {
	GLMesh*		self = (GLMesh*)glMesh;
	Index		shadowElementCnt;
	Index*		shadowElementSet;
	
	if( !mesh->decomp->shadowElementSets || !mesh->decomp->shadowElementSets[rank] ) {
		return;
	}
	
	IndexSet_GetMembers( mesh->decomp->shadowElementSets[rank], &shadowElementCnt, &shadowElementSet );
	
	if( shadowElementCnt ) {
		IndexSet*	localEdgeSet;
		IndexSet*	shadowEdgeSet;
		Index		lEdge_I;
		Index		sEdge_I;
		
		localEdgeSet = IndexSet_New( mesh->decomp->elementLayout->edgeCount );
		for( lEdge_I = 0; lEdge_I < self->localEdgeCnts[rank]; lEdge_I++ ) {
			IndexSet_Add( localEdgeSet, self->localEdges[rank][lEdge_I] );
		}
		
		if( self->shadowEdges[rank] ) {
			Memory_Free( self->shadowEdges );
		}
		self->shadowEdgeCnts[rank] = ElementLayout_BuildEdgeSubset( mesh->elementLayout, 
									   shadowElementCnt, 
									   shadowElementSet, 
									   &self->shadowEdges[rank] );
		shadowEdgeSet = IndexSet_New( mesh->decomp->elementLayout->edgeCount );
		for( sEdge_I = 0; sEdge_I < self->shadowEdgeCnts[rank]; sEdge_I++ ) {
			if( !IndexSet_IsMember( localEdgeSet, self->shadowEdges[rank][sEdge_I] ) ) {
				IndexSet_Add( shadowEdgeSet, self->shadowEdges[rank][sEdge_I] );
			}
		}
		
		Memory_Free( self->shadowEdges[rank] );
		IndexSet_GetMembers( shadowEdgeSet, &self->shadowEdgeCnts[rank], &self->shadowEdges[rank] );
		Stg_Class_Delete( shadowEdgeSet );
	}
	
	Memory_Free( shadowElementSet );
}


void _GLMesh_BuildVacantEdges( void* glMesh, MeshLayout* mesh, Partition_Index rank ) {
	GLMesh*		self = (GLMesh*)glMesh;
	ElementLayout*	elementLayout = mesh->decomp->elementLayout;
	IndexSet*	domainEdgeSet;
	IndexSet*	vacantEdgeSet;
	Index		gEdge_I;
	
	domainEdgeSet = IndexSet_New( elementLayout->edgeCount );
	
	if( self->localEdgeCnts && self->localEdgeCnts[rank] ) {
		unsigned	lEdge_I;
		
		for( lEdge_I = 0; lEdge_I < self->localEdgeCnts[rank]; lEdge_I++ ) {
			IndexSet_Add( domainEdgeSet, self->localEdges[rank][lEdge_I] );
		}
	}
	
	if( self->shadowEdgeCnts && self->shadowEdgeCnts[rank] ) {
		unsigned	sEdge_I;
		
		for( sEdge_I = 0; sEdge_I < self->shadowEdgeCnts[rank]; sEdge_I++ ) {
			IndexSet_Add( domainEdgeSet, self->shadowEdges[rank][sEdge_I] );
		}
	}
	
	vacantEdgeSet = IndexSet_New( elementLayout->edgeCount );
	
	for( gEdge_I = 0; gEdge_I < elementLayout->edgeCount; gEdge_I++ ) {
		if( !IndexSet_IsMember( domainEdgeSet, gEdge_I ) ) {
			IndexSet_Add( vacantEdgeSet, gEdge_I );
		}
	}
	
	IndexSet_GetMembers( vacantEdgeSet, &self->vacantEdgeCnts[rank], &self->vacantEdges[rank] );
	
	Stg_Class_Delete( domainEdgeSet );
	Stg_Class_Delete( vacantEdgeSet );
}


void _GLMesh_Destroy( void* glMesh ) {
	GLMesh*		self = (GLMesh*)glMesh;
	
	if( self->verts ) {
		Memory_Free( self->verts );
		self->verts = NULL;
	}
	
	if( self->edges ) {
		Memory_Free( self->edges );
		self->edges = NULL;
	}
	
	if( self->localEdgeCnts ) {
		Memory_Free( self->localEdgeCnts );
		self->localEdgeCnts = NULL;
	}
	
	if( self->localEdges ) {
		Partition_Index		rank_I;
		
		for( rank_I = 0; rank_I < self->rankCnt; rank_I++ ) {
			if( self->localEdges[rank_I] ) {
				Memory_Free( self->localEdges[rank_I] );
			}
		}
		Memory_Free( self->localEdges );
		self->localEdges = NULL;
	}
	
	if( self->shadowEdgeCnts ) {
		Memory_Free( self->shadowEdgeCnts );
		self->shadowEdgeCnts = NULL;
	}
	
	if( self->shadowEdges ) {
		Partition_Index		rank_I;
		
		for( rank_I = 0; rank_I < self->rankCnt; rank_I++ ) {
			if( self->shadowEdges[rank_I] ) {
				Memory_Free( self->shadowEdges[rank_I] );
			}
		}
		Memory_Free( self->shadowEdges );
		self->shadowEdges = NULL;
	}
	
	if( self->vacantEdgeCnts ) {
		Memory_Free( self->vacantEdgeCnts );
		self->vacantEdgeCnts = NULL;
	}
	
	if( self->vacantEdges ) {
		Partition_Index		rank_I;
		
		for( rank_I = 0; rank_I < self->rankCnt; rank_I++ ) {
			if( self->vacantEdges[rank_I] ) {
				Memory_Free( self->vacantEdges[rank_I] );
			}
		}
		Memory_Free( self->vacantEdges );
		self->vacantEdges = NULL;
	}
}

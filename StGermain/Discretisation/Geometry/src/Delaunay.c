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
*/
/** \file
**  Role:
** The Delaunay class computes the constrained delaunay triangulation of a set of points
** in 2 Dimensions along with the voronoi diagram of the point-set.
** Assumptions:
**
** Comments:
**
** The recursive divide and conquer algorithm has been implemented from the pseudo-code
** given in "Primitives for the manipulation of general subdivisions and the computation
** of voronoi diagrams" by Leonidas Guibas and Jorge Stolfi.
** ACM transtactions on graphics, Vol. 4, No. 2, April 1985, Pages 74-123
**
**
** $Id: Delaunay.c 3462 2006-02-19 06:53:24Z RaquibulHassan $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"


#include "units.h"
#include "types.h"
#include "QuadEdge.h"
#include "Delaunay.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <limits.h>

const Type Delaunay_Type="Delaunay";

#if !defined INFINITY
	#define INFINITY LONG_MAX
#endif

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	
/** Create a Delaunay */
Delaunay* Delaunay_DefaultNew( Name name )
{
	Delaunay *d = _Delaunay_New(
			sizeof( Delaunay ),
			Delaunay_Type,
			_Delaunay_Delete,
			_Delaunay_Print,
			_Delaunay_Copy,
			(Stg_Component_DefaultConstructorFunction*)Delaunay_DefaultNew,
			_Delaunay_Construct,
			_Delaunay_Build,
			_Delaunay_Initialise,
			_Delaunay_Execute,
			_Delaunay_Destroy,
			name,
			False,
			NULL,
			NULL,
			0,
			0,
			NULL );

	return d;
}
	
Delaunay* Delaunay_New(
	Name						name,
	Dictionary*					dictionary,
	CoordF						*sites,
	int							numSites,
	int							idOffset,
	DelaunayAttributes			*attr )
{
	Delaunay *d = _Delaunay_New(
			sizeof( Delaunay ),
			Delaunay_Type,
			_Delaunay_Delete,
			_Delaunay_Print,
			_Delaunay_Copy,
			(Stg_Component_DefaultConstructorFunction*)Delaunay_DefaultNew,
			_Delaunay_Construct,
			_Delaunay_Build,
			_Delaunay_Initialise,
			_Delaunay_Execute,
			_Delaunay_Destroy,
			name,
			True,
			dictionary,
			sites,
			numSites,
			idOffset,
			attr );
	
	return d;
}

	/** Initialise a Delaunay */
void Delaunay_Init(
	Delaunay*					self,
	Name						name,
	Dictionary*					dictionary,
	CoordF						*sites,
	int							numSites,
	int							idOffset,
	DelaunayAttributes			*attr )
{
	int i = 0;

	self->type = Delaunay_Type;
	self->_sizeOfSelf = sizeof( Delaunay );
	self->_deleteSelf = False;
	self->dictionary = dictionary;

	self->_delete = _Delaunay_Delete;
	self->_print = _Delaunay_Print;
	self->_copy = _Delaunay_Copy;
	self->_defaultConstructor = (Stg_Component_DefaultConstructorFunction*)Delaunay_DefaultNew;
	self->_construct = _Delaunay_Construct;
	self->_build = _Delaunay_Build;
	self->_initialise = _Delaunay_Initialise;
	self->_execute = _Delaunay_Execute;
	self->_destroy = _Delaunay_Destroy;

	self->attributes = attr;
	self->dictionary = dictionary;
	
	if( self->attributes->BuildBoundingTriangle ){
		self->numSites = numSites + 3;
	}
	else{
		self->numSites = numSites;
	}
	
	self->numInputSites = numSites;
	self->idOffset = idOffset;
	
	if( sites != NULL ){
		self->sites = Memory_Alloc_Array_Unnamed( Site, self->numSites );
		memset( self->boundingTriangle, 0, sizeof( self->boundingTriangle ) );

		for( i=0; i<self->numSites; i++ ){
			if( i < self->numInputSites ){
				self->sites[i].coord = &(sites[i]);
			}
			else{
				self->sites[i].coord = &(self->boundingTriangle[i%3]);
			}
			self->sites[i].id = i + self->idOffset;
		}
	}

	_Stg_Class_Init( (Stg_Class*)self );
	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );
	_Stg_Component_Init( (Stg_Component*)self );
	_Delaunay_Init( self );
}

/** Creation implementation */
Delaunay* _Delaunay_New(
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
	Dictionary					*dictionary,
	CoordF						*sites,
	int							numSites,
	int							idOffset,
	DelaunayAttributes			*attr )
{
	Delaunay *self = NULL;
	
	assert( _sizeOfSelf >= sizeof(Delaunay) );
	self = (Delaunay*)_Stg_Component_New( _sizeOfSelf, type, _delete, _print, _copy, _defaultConstructor,
			_construct, _build, _initialise, _execute, _destroy, name, NON_GLOBAL );

	self->points = sites;
	self->attributes = attr;
	self->dictionary = dictionary;

	self->numInputSites = numSites;
	self->idOffset = idOffset;
	
	if( initFlag ){
		_Delaunay_Init( self );
	}

	return self;
}

#define PI 3.1415926535897932384626
void Delaunay_FindMinMax( Site *sites, int count, float *minX, float *minY, float *maxX, float *maxY )
{
	int i = 0;
	
	*maxX = -INFINITY;
	*maxY = -INFINITY;
	
	*minX = INFINITY;
	*minY = INFINITY;

	if (sites == NULL ) return;
	
	for( i=0; i<count; i++ ){
		if( *maxX < (*(sites[i].coord))[0] ){
			*maxX = (*(sites[i].coord))[0];
		}
		if( *maxY < (*(sites[i].coord))[1] ){
			*maxY = (*(sites[i].coord))[1];
		}
		if( *minX > (*(sites[i].coord))[0] ){
			*minX = (*(sites[i].coord))[0];
		}
		if( *minY > (*(sites[i].coord))[1] ){
			*minY = (*(sites[i].coord))[1];
		}
	}
}

void _Delaunay_Init( Delaunay* self )
{
	float maxX, minX, maxY, minY;
	float centreX, centreY;
	float radius;
	int i = 0;
	CoordF *sites = NULL;
	
	assert( self );
	
	sites = self->points;
	
	if( self->attributes->BuildBoundingTriangle ){
		self->numSites = self->numInputSites + 3;
	}
	else{
		self->numSites = self->numInputSites;
	}

	if( sites != NULL ){
		self->sites = Memory_Alloc_Array_Unnamed( Site, self->numSites );
		memset( self->boundingTriangle, 0, sizeof( self->boundingTriangle ) );

		for( i=0; i<self->numSites; i++ ){
			if( i < self->numInputSites ){
				self->sites[i].coord = &(sites[i]);
			}
			else{
				self->sites[i].coord = &(self->boundingTriangle[i%3]);
			}
			self->sites[i].id = i + self->idOffset;
		}
	}
	
	centreX = 0; centreY = 0; 
	
	Delaunay_FindMinMax( self->sites, self->numSites, &minX, &minY, &maxX, &maxY );

	radius = (sqrt((maxX - minX) * (maxX - minX) + (maxY - minY) * (maxY - minY)));
			
	centreX = minX + (maxX - minX) / 2.0f;
	centreY = minY + (maxY - minY) / 2.0f;

	self->boundingTriangle[0][0] = centreX - tan(PI/3.0f)*radius;
	self->boundingTriangle[0][1] = centreY - radius;

	self->boundingTriangle[1][0] = centreX + tan(PI/3.0f)*radius;
	self->boundingTriangle[1][1] = centreY - radius;
			
	self->boundingTriangle[2][0] = centreX;
	self->boundingTriangle[2][1] = centreY + radius/cos(PI/3.0f);
}

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

/** Stg_Class_Delete() implementation */
void _Delaunay_Delete( void* delaunay )
{
	Delaunay *self = (Delaunay*)delaunay;
	
	assert( self );

	if( self->sites ){
		Memory_Free( self->sites );
	}
	
	if( self->qp ){
		Stg_Class_Delete( self->qp );
	}

	if( self->vp ){
		Stg_Class_Delete( self->vp );
	}

	if( self->triangleIndices ){
		Memory_Free( self->triangleIndices[0] );
		Memory_Free( self->triangleIndices );
	}

	if( self->triangleNeighbours ){
		Memory_Free( self->triangleNeighbours[0] );
		Memory_Free( self->triangleNeighbours );
	}

	if( self->numNeighbours ){
		Memory_Free( self->numNeighbours );
	}
	
	if( self->neighbours ){
		Memory_Free( self->neighbours[0] );
		Memory_Free( self->neighbours );
	}

	if( self->voronoiSides ){
		Memory_Free( self->voronoiSides[0] );
		Memory_Free( self->voronoiSides );
	}

	if( self->voronoiArea ){
		Memory_Free( self->voronoiArea );
	}

	if( self->hull ){
		Memory_Free( self->hull );
	}

	_Stg_Component_Delete( self );
}

/** Stg_Class_Print() implementation */
void _Delaunay_Print( void* delaunay, Stream* stream )
{
	Delaunay *self = ( Delaunay* )delaunay;
	
	assert( self );
	assert( stream );

	_Stg_Component_Print( self, stream );
	Journal_Printf( stream, "Delaunay (ptr): (%p)\n", self );
	
	Journal_Printf( stream, "\tNum Sites %d\n", self->numSites );
	Journal_Printf( stream, "\tNum Edges %d\n", self->numEdges );
	Journal_Printf( stream, "\tNum Triangles %d\n", self->numTriangles );
	Journal_Printf( stream, "\tNum Voronoi Vertices %d\n", self->numVoronoiVertices );
}

void *_Delaunay_Copy( void* delaunay, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap )
{
	return NULL;
}

void _Delaunay_Construct( void* delaunay, Stg_ComponentFactory* cf, void* data )
{
	Delaunay *self = NULL;
	Stg_ObjectList* pointerRegister;
	int idOffset = 0;
	CoordF *points = NULL;
	DelaunayAttributes *attr = NULL;
	int numSites = 0;

	self = (Delaunay*) delaunay;
	
	pointerRegister = Stg_ObjectList_Get( cf->registerRegister, "Pointer_Register" );
	
	assert( pointerRegister );

	points = Stg_ObjectList_Get( pointerRegister, "dataPoints" );
	attr = Stg_ObjectList_Get( pointerRegister, "delaunayAttributes" );

	numSites = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, "numSites", 0 );

	assert( points );
	assert( attr );
	assert( numSites );
	
	self->dictionary = cf->rootDict;
	self->points = points;
	self->attributes = attr;

	if( self->attributes->BuildBoundingTriangle ){
		self->numSites = numSites + 3;
	}
	else{
		self->numSites = numSites;
	}
	
	self->numInputSites = numSites;
	self->idOffset = idOffset;

	_Delaunay_Init( self );
	_Delaunay_Build( self, NULL );
}

void _Delaunay_Build( void* delaunay, void* data )
{
	Delaunay *self = ( Delaunay* )delaunay;
	DelaunayAttributes *attr = NULL;
    
	assert( self );
	
	self->qp = MemoryPool_New( QuadEdge, self->numSites * 3 );
	
	Delaunay_SortSites(self->sites, self->numSites);

    Delaunay_Recurse(self, 0, self->numSites, &self->leftMost, &self->rightMost);

	self->numEdges = self->qp->numElements - self->qp->numElementsFree;
	self->numFaces = self->numEdges - self->numSites + 2;
	self->numTriangles = 0;

	attr = self->attributes;

	if( attr->CreateVoronoiVertices ){
		self->vp = MemoryPool_New( VoronoiVertex, self->numSites * 2 );
	}
	
	Delaunay_FindHull( self );
	
	if( attr->BuildTriangleIndices ){
		Delaunay_BuildTriangleIndices( self );
	}
	
	if( attr->CreateVoronoiVertices ){
		Delaunay_BuildVoronoiVertices( self );
		self->numVoronoiVertices = self->vp->numElements - self->vp->numElementsFree;
	}
	
	Delaunay_FindNeighbours( self );
}

void _Delaunay_Initialise( void* delaunay, void* data )
{
	
}

void _Delaunay_Execute( void* delaunay, void* data )
{
	
}

void _Delaunay_Destroy( void* delaunay, void* data )
{
	
}

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

/* Function for heap sorting the input points in ascending x-coordinate */
void Delaunay_SortSites(Site *sites, int numSites )
{
   int gap, i, j;
   Site temp;

	for (gap = numSites/2; gap > 0; gap /= 2){
		for (i = gap; i < numSites; i++){
			for ( j = i-gap;
				j >= 0 && ( (*(sites[j].coord))[0] != (*(sites[j+gap].coord))[0] ?
					((*(sites[j].coord))[0] > (*(sites[j+gap].coord))[0]) : ((*(sites[j].coord))[1] > (*(sites[j+gap].coord))[1]));
				j -= gap) 
			{
				memcpy( &temp, sites+j, sizeof( Site ) );
				memcpy( sites+j, sites+j+gap, sizeof( Site ) );
				memcpy( sites+j+gap, &temp, sizeof( Site ) );
			}
		}
	}
}

/* Function to check if a point is to the right of an edge */
int RightOf(Site *s, QuadEdgeRef e)
{
	double result;

	CCW(s->coord, ((Site*)DEST(e))->coord, ((Site*)ORG(e))->coord, &result);

	return result > 0.0;
}

/* Function to check if a point is to the left of an edge */
int LeftOf(Site *s, QuadEdgeRef e)
{
	double result;
	CCW(s->coord, ((Site*)ORG(e))->coord, ((Site*)DEST(e))->coord, &result);

	return result > 0.0;
}

/* Function to check if a point is inside the circumcircle of three other points */
int InCircle(Site *a, Site *b, Site *c, Site *d)
{
	double x1 = (*(a->coord))[0], y1 = (*(a->coord))[1];
	double x2 = (*(b->coord))[0], y2 = (*(b->coord))[1];
	double x3 = (*(c->coord))[0], y3 = (*(c->coord))[1];
	double x4 = (*(d->coord))[0], y4 = (*(d->coord))[1];

	return ((y4-y1)*(x2-x3)+(x4-x1)*(y2-y3))*((x4-x3)*(x2-x1)-(y4-y3)*(y2-y1)) >
			((y4-y3)*(x2-x1)+(x4-x3)*(y2-y1))*((x4-x1)*(x2-x3)-(y4-y1)*(y2-y3));
}

/* This is the famous divide and conquer algorithm implemented from Guibas and Stolfi's 1985
 * paper. Refer to ACM Transcations on graphics, Vol. 4, No. 2, April 1985, Pages 74-123 */
void Delaunay_Recurse( Delaunay *delaunay, int sl, int sh, QuadEdgeRef *le, QuadEdgeRef *re )
{
	Site *sites = delaunay->sites;
	
	if (sh == sl+2) 
	{
		QuadEdgeRef a = MakeQuadEdge( delaunay->qp );
		ORG(a) = &sites[sl]; DEST(a) = &sites[sl+1];
		*le = a; *re = SYM(a);
	}
	else if (sh == sl+3) 
	{
		QuadEdgeRef a = MakeQuadEdge( delaunay->qp );
		QuadEdgeRef b = MakeQuadEdge( delaunay->qp );
		float ct;
		CCW(sites[sl].coord, sites[sl+1].coord, sites[sl+2].coord, &ct);
		SpliceQuadEdges(SYM(a), b);
		ORG(a) = &sites[sl]; DEST(a) = &sites[sl+1];
		ORG(b) = &sites[sl+1];  DEST(b) = &sites[sl+2];
		if (ct == 0.0) 
		{ *le = a; *re = SYM(b); }
		else
		{ QuadEdgeRef c = ConnectQuadEdges(delaunay->qp, b, a);
			if (ct > 0.0) 
			{ *le = a; *re = SYM(b); }
			else 
			{ *le = SYM(c); *re = c; }
		}
	}
	else
	{
		QuadEdgeRef ldo = 0, ldi = 0, rdi = 0, rdo = 0;
		QuadEdgeRef basel = 0, lcand = 0, rcand = 0;

		int sm = (sl+sh)/2;

		Delaunay_Recurse( delaunay, sl, sm, &ldo, &ldi );
		Delaunay_Recurse( delaunay, sm, sh, &rdi, &rdo);

		while (1) 
		{
			if (LeftOf(ORG(rdi), ldi)) ldi = LNEXT(ldi);
			else if (RightOf(ORG(ldi), rdi)) rdi = ONEXT(SYM(rdi));
			else break;
		}

		basel = ConnectQuadEdges(delaunay->qp, SYM(rdi), ldi);
		if (ORG(ldi) == ORG(ldo)) ldo = SYM(basel);
		if (ORG(rdi) == ORG(rdo)) rdo = basel;

		while (1) 
		{

			lcand = ONEXT(SYM(basel));
			if (RightOf(DEST(lcand), basel))
				while (InCircle(DEST(basel), ORG(basel), DEST(lcand), DEST(ONEXT(lcand)))) 
				{
					QuadEdgeRef t = ONEXT(lcand);
					
					DeleteQuadEdge(delaunay->qp, lcand);
					lcand = t;
				}

			rcand = OPREV(basel);
			if (RightOf(DEST(rcand), basel))
				while (InCircle(DEST(basel), ORG(basel), DEST(rcand), DEST(OPREV(rcand)))) 
				{
					QuadEdgeRef t = OPREV(rcand);

					DeleteQuadEdge(delaunay->qp, rcand);
					rcand = t;
				}

			if (!RightOf(DEST(lcand), basel) && !RightOf(DEST(rcand), basel)) break;

			if ( !RightOf(DEST(lcand), basel) ||
				( RightOf(DEST(rcand), basel) && 
				InCircle(DEST(lcand), ORG(lcand), ORG(rcand), DEST(rcand))))
				basel = ConnectQuadEdges(delaunay->qp, rcand, SYM(basel));
			else
				basel = ConnectQuadEdges(delaunay->qp, SYM(basel), SYM(lcand));
		}
		*le = ldo; *re = rdo;
	}
}

/* Function to find the convex hull of a triangulated set of points */
void Delaunay_FindHull( Delaunay *delaunay )
{
	QuadEdgeRef start = 0, le = 0;
	
	assert( delaunay );
	
	start = le = delaunay->leftMost;
	
	delaunay->hull = Memory_Alloc_Array_Unnamed( int, delaunay->numSites );
	memset( delaunay->hull, 0, sizeof( int ) * delaunay->numSites );
	
	do{
		delaunay->hull[((Site*)ORG(le))->id - delaunay->idOffset] = 1;
		delaunay->hull[((Site*)DEST(le))->id - delaunay->idOffset] = 1;
		le = RPREV(le);
	}while(le != start);
}

/* Function for generating triangle indices from a triangulation */
void Delaunay_BuildTriangleIndices( Delaunay *delaunay )
{
	int i = 0, triCount;
	QuadEdgeRef e = 0, eStart = 0, eOnext = 0, eLnext = 0;
	QuadEdge *edges = NULL;
	Site *sites = NULL;
	int maxEdges = 0;
	unsigned int **triIndices = NULL;
	int **edgeToTriangle = NULL;
	int index = 0;

	assert( delaunay );
	
	delaunay->triangleIndices = Memory_Alloc_Array_Unnamed( unsigned int*, delaunay->numFaces );
	delaunay->triangleIndices[0] = Memory_Alloc_Array_Unnamed( unsigned int, delaunay->numFaces * 3 );
	memset( delaunay->triangleIndices[0] , 0, sizeof(unsigned int) * delaunay->numFaces * 3 );
	
	if( delaunay->attributes->BuildTriangleNeighbours ){
		delaunay->triangleNeighbours = Memory_Alloc_Array_Unnamed( unsigned int*, delaunay->numFaces );
		delaunay->triangleNeighbours[0] = Memory_Alloc_Array_Unnamed( unsigned int, delaunay->numFaces * 3 );

		edgeToTriangle = Memory_Alloc_Array_Unnamed( int*, delaunay->qp->numElements );
		edgeToTriangle[0] = Memory_Alloc_Array_Unnamed( int, delaunay->qp->numElements * 2 );
	}
	
	for( i=0; i<delaunay->numFaces; i++ ){
		delaunay->triangleIndices[i] = delaunay->triangleIndices[0]+i*3;

		if( delaunay->attributes->BuildTriangleNeighbours ){
			delaunay->triangleNeighbours[i] = delaunay->triangleNeighbours[0]+i*3;
			
			delaunay->triangleNeighbours[i][0] = delaunay->numFaces-1;
			delaunay->triangleNeighbours[i][1] = delaunay->numFaces-1;
			delaunay->triangleNeighbours[i][2] = delaunay->numFaces-1;
		}
	}
	
	if( delaunay->attributes->BuildTriangleNeighbours ){
		for( i=0; i<delaunay->qp->numElements; i++ ){
			edgeToTriangle[i] = edgeToTriangle[0]+i*2;
			
			edgeToTriangle[i][0] = delaunay->numFaces-1;
			edgeToTriangle[i][1] = delaunay->numFaces-1;
		}
	}
		
	triIndices = delaunay->triangleIndices;
	
	edges = (QuadEdge*)delaunay->qp->elements;
	sites = delaunay->sites;
	maxEdges = delaunay->qp->numElements;
		
	for (i = 0; i < maxEdges; i++) {
		edges[i].count = 0;
	}

	triCount = 0;
	for (i = 0; i < maxEdges; i++) {
		
		e = eStart = (QuadEdgeRef)((void*)&(edges[i]));
		
		if( IS_FREE(e) )continue;
		
		do{
			eOnext = ONEXT(e);
			eLnext = LNEXT(e);
			
			if( (COUNT(e)<2) && (COUNT(LNEXT(e))<2) && (COUNT(eOnext)<2) ){
					if( ((((Site*)ORG(eLnext)) == ((Site*)DEST(e)))) &&
					 ((((Site*)DEST(eLnext)) == ((Site*)DEST(eOnext)))) ){
		
						if( delaunay->attributes->BuildBoundingTriangle ){
							if( (!( delaunay->hull[(((Site*)ORG(e))->id) - delaunay->idOffset] )) &&
								(!( delaunay->hull[(((Site*)DEST(e))->id) - delaunay->idOffset] )) && 
								(!( delaunay->hull[(((Site*)DEST(eOnext))->id) - delaunay->idOffset] )) )
							{
								triIndices[triCount][0] = (((Site*)ORG(e))->id);
								triIndices[triCount][1] = (((Site*)DEST(e))->id);
								triIndices[triCount][2] = (((Site*)DEST(eOnext))->id);

								if( delaunay->attributes->BuildTriangleNeighbours ){
									index = (int)(((QuadEdge*)((void*)e)) - (QuadEdge*)delaunay->qp->elements);
									edgeToTriangle[index][COUNT(e)] = triCount;
								
									index = (int)(((QuadEdge*)((void*)eOnext)) - (QuadEdge*)delaunay->qp->elements);
									edgeToTriangle[index][COUNT(eOnext)] = triCount;
								
									index = (int)(((QuadEdge*)((void*)eLnext)) - (QuadEdge*)delaunay->qp->elements);
									edgeToTriangle[index][COUNT(eLnext)] = triCount;
								}
								
								triCount++;
							}
						}
						else{
							triIndices[triCount][0] = (((Site*)ORG(e))->id);
							triIndices[triCount][1] = (((Site*)DEST(e))->id);
							triIndices[triCount][2] = (((Site*)DEST(eOnext))->id);
							
							if( delaunay->attributes->BuildTriangleNeighbours ){
								index = (int)(((QuadEdge*)((void*)e)) - (QuadEdge*)delaunay->qp->elements);
								edgeToTriangle[index][COUNT(e)] = triCount;
								
								index = (int)(((QuadEdge*)((void*)eOnext)) - (QuadEdge*)delaunay->qp->elements);
								edgeToTriangle[index][COUNT(eOnext)] = triCount;
								
								index = (int)(((QuadEdge*)((void*)eLnext)) - (QuadEdge*)delaunay->qp->elements);
								edgeToTriangle[index][COUNT(eLnext)] = triCount;
							}

							triCount++;
						}

						COUNT(e)++;
						COUNT(LNEXT(e))++;
						COUNT(eOnext)++;
					}
				}
			e = eOnext;
		}while( e != eStart );
	}
	
	delaunay->numTriangles = triCount;

	if( delaunay->attributes->BuildTriangleNeighbours ){
		int *triangleNeighbourCount = NULL;

		triangleNeighbourCount = Memory_Alloc_Array_Unnamed( int, delaunay->numFaces );
		memset( triangleNeighbourCount, 0, sizeof( int ) * delaunay->numFaces );
		
		for( i=0; i<delaunay->qp->numElements; i++ ){
			if( IS_FREE( (QuadEdgeRef)(&(delaunay->qp->elements[i*sizeof(QuadEdge)])) ) ) continue;

		
			if( edgeToTriangle[i][0] != (delaunay->numFaces-1) )
				delaunay->triangleNeighbours[edgeToTriangle[i][0]][triangleNeighbourCount[edgeToTriangle[i][0]]++] = edgeToTriangle[i][1];
		
			if( edgeToTriangle[i][1] != (delaunay->numFaces-1) )
				delaunay->triangleNeighbours[edgeToTriangle[i][1]][triangleNeighbourCount[edgeToTriangle[i][1]]++] = edgeToTriangle[i][0];
		}
		
		Memory_Free( edgeToTriangle );
		Memory_Free( triangleNeighbourCount );
	}
}

/* Function for calculating voronoi vertices */
void Delaunay_BuildVoronoiVertices( Delaunay *delaunay )
{
	int i = 0;
	QuadEdgeRef e = 0, eStart = 0, eOnext = 0, eLnext = 0;
	VoronoiVertex *new_voronoi_site = NULL;
	QuadEdge *edges = NULL;
	Site *sites = NULL;
	int maxEdges = 0;

	assert( delaunay );
	
	edges = (QuadEdge*)delaunay->qp->elements;
	sites = delaunay->sites;
	maxEdges = delaunay->qp->numElements;
		
	for (i = 0; i < maxEdges; i++) {
		edges[i].count = 0;
	}

	for (i = 0; i < maxEdges; i++) {
		
		e = eStart = (QuadEdgeRef)((void*)&(edges[i]));
		
		if( IS_FREE(e) )continue;
		
		new_voronoi_site = NULL;
		do{
			eOnext = ONEXT(e);
			eLnext = LNEXT(e);

			if( (COUNT(e)<2) && (COUNT(LNEXT(e))<2) && (COUNT(eOnext)<2) ){
					if( ((((Site*)ORG(eLnext)) == ((Site*)DEST(e)))) &&
					 ((((Site*)DEST(eLnext)) == ((Site*)DEST(eOnext)))) ){
						
						/* voronoi */
							
						new_voronoi_site = MemoryPool_NewObject( VoronoiVertex, delaunay->vp );
					
						/* Fiding the center of the circumcircle defined by org(e), dest(eonext) and dest(e)
						 * and retrieving the result via new_voronoi_site */
						CIRCUM_CIRCLE( ((Site*)ORG(e))->coord, ((Site*)DEST(eOnext))->coord, ((Site*)DEST(e))->coord, &(new_voronoi_site) );
						
						/* Assigning the new voronoi vertex to the associated edges */
						VDEST( e ) = new_voronoi_site;
						VORG( eOnext ) = new_voronoi_site;
						VDEST( eLnext ) = new_voronoi_site;
						
						/*         */
						
						/* Marking the edges so that we dont visit any edge more than twice */
						COUNT(e)++;
						COUNT(LNEXT(e))++;
						COUNT(eOnext)++;
					}
				}
			e = eOnext;
		}while( e != eStart );
	}
}

/* Function for finding the neighbours of each point, along with the sides
 * of the voronoi cells and the surface area of the voronoi cells */
void Delaunay_FindNeighbours( Delaunay *delaunay )
{
	int current_pos = 0, i = 0, count = 0, count1 = 0, numNeighboursSum;
	int *tempNumNeighbours = NULL;
	int maxEdges = 0, numSites = 0;
	Site *src, *dst;
	VoronoiVertex *vsrc, *vdst;
	float dist = 0.0f, diffx = 0.0f, diffy = 0.0f, voronoiAreaResult = 0.0;
	Site *sites = NULL;
	QuadEdge *edges = NULL;
	unsigned int *numNeighbours;
	int *hull;
	unsigned int **neighbours;
	float *voronoiArea, **sides;
	DelaunayAttributes *attr = NULL;

	assert( delaunay );

	attr = delaunay->attributes;
	sites = delaunay->sites;
	edges = (QuadEdge*)delaunay->qp->elements;
	maxEdges = delaunay->qp->numElements;
	numSites = delaunay->numSites;
	
	for (i = 0; i < maxEdges; i++) {
		edges[i].count = 0;
	}

	delaunay->numNeighbours = Memory_Alloc_Array_Unnamed( unsigned int, numSites );
	memset( delaunay->numNeighbours, 0, sizeof( unsigned int ) * numSites );
	numNeighbours = delaunay->numNeighbours;
	
	hull = delaunay->hull;

	numNeighboursSum = 0;
	for( i=0; i<maxEdges; i++ ){
		if( IS_FREE( (QuadEdgeRef)&(edges[i]) ) ) continue;
		
		if( edges[i].count == 0 ){
			src = (Site*)ORG((QuadEdgeRef)((void*)(&(edges[i]))));
			dst = (Site*)DEST((QuadEdgeRef)((void*)(&(edges[i]))));

			if( (src->id < (delaunay->numInputSites+delaunay->idOffset)) 
					&& (dst->id < (delaunay->numInputSites+delaunay->idOffset)) ){
				
				/* Finding the number of neighbours that each point has */
				numNeighbours[src->id - delaunay->idOffset]++;
				numNeighbours[dst->id - delaunay->idOffset]++;
			
				/* NumNeighboursSum is required for allocating just enough memory for storing the
				 * actual neighbours and the sides of the voronoi celss */
				numNeighboursSum += 2;

			}

			edges[i].count++;
		}
	}

	/* allocating memory */
	if( attr->CalculateVoronoiSurfaceArea ){
		delaunay->voronoiArea = Memory_Alloc_Array_Unnamed( float, numSites );
		memset( delaunay->voronoiArea, 0, sizeof( float ) * numSites );
	}
	
	if( attr->FindNeighbours ){
		delaunay->neighbours = Memory_Alloc_Array_Unnamed( unsigned int*, numSites );
		delaunay->neighbours[0] = Memory_Alloc_Array_Unnamed( unsigned int, numNeighboursSum );
		memset( delaunay->neighbours[0], 0, sizeof( unsigned int ) * numNeighboursSum );
	}
	
	if( attr->CalculateVoronoiSides ){
		delaunay->voronoiSides = Memory_Alloc_Array_Unnamed( float*, numSites );
		delaunay->voronoiSides[0] = Memory_Alloc_Array_Unnamed( float, numNeighboursSum );
		memset( delaunay->voronoiSides[0], 0, sizeof( float ) * numNeighboursSum );
	}

	tempNumNeighbours = Memory_Alloc_Array_Unnamed( int, numSites );
	memcpy( tempNumNeighbours, numNeighbours, sizeof( int ) * numSites );

	
	voronoiArea = delaunay->voronoiArea;
	neighbours = delaunay->neighbours;
	sides = delaunay->voronoiSides;
	
	current_pos = 0;
	for( i=0; i<numSites; i++ ){
		
		if( neighbours ) neighbours[i] = neighbours[0] + current_pos;
		if( sides ) sides[i] = sides[0] + current_pos;
		
		current_pos += numNeighbours[i];
	}
	
	for( i=0; i<maxEdges; i++ ){
		if( IS_FREE( (QuadEdgeRef)&(edges[i]) ) ) continue;

		src = (Site*)ORG((QuadEdgeRef)((void*)(&(edges[i]))));
		dst = (Site*)DEST((QuadEdgeRef)((void*)(&(edges[i]))));

		count = tempNumNeighbours[src->id - delaunay->idOffset];
		count1 = tempNumNeighbours[dst->id - delaunay->idOffset];
		
		if( edges[i].count == 1 ){

			if( (src->id < (delaunay->numInputSites+delaunay->idOffset)) 
					&& (dst->id < (delaunay->numInputSites+delaunay->idOffset)) ){

				vsrc = (VoronoiVertex*)VORG((QuadEdgeRef)((void*)(&(edges[i]))));
				vdst = (VoronoiVertex*)VDEST((QuadEdgeRef)((void*)(&(edges[i]))));
			
				if( vsrc && vdst ){
				
					/* calculating the length of the voronoi sides */
					if( attr->CalculateVoronoiSides ){
					
						diffx = ( vsrc->point[0] - vdst->point[0] );
						diffy = ( vsrc->point[1] - vdst->point[1] );
				
						dist = sqrt( diffx*diffx + diffy*diffy );

						sides[src->id - delaunay->idOffset][--count] = dist;
						sides[dst->id - delaunay->idOffset][--count1] = dist;
					}
				
					if( attr->CalculateVoronoiSurfaceArea ){
					
						/* calculating the voronoi surface area for each point, with the hull
						 * nodes having an infinite unbounded area */

						if( !(hull[src->id - delaunay->idOffset]) ){
							CCW( src->coord, &(vsrc->point), &(vdst->point), &voronoiAreaResult );
							voronoiArea[src->id - delaunay->idOffset] += FABS( voronoiAreaResult ) * 0.5;
						}
						else{
							voronoiArea[src->id - delaunay->idOffset] = INFINITY;
						}
					
						if( !(hull[dst->id - delaunay->idOffset]) ){
							CCW( dst->coord, &(vsrc->point), &(vdst->point), &voronoiAreaResult );
							voronoiArea[dst->id - delaunay->idOffset] += FABS( voronoiAreaResult ) * 0.5;
						}
						else{
							voronoiArea[dst->id - delaunay->idOffset] = INFINITY;
						}
					}
				}

				/* Storing the actual neighbours of each node */
				if( attr->FindNeighbours ){
					neighbours[src->id - delaunay->idOffset][--tempNumNeighbours[src->id - delaunay->idOffset]] = dst->id;
					neighbours[dst->id - delaunay->idOffset][--tempNumNeighbours[dst->id - delaunay->idOffset]] = src->id;
				}
			}
			edges[i].count++;
		}
	}
	Memory_Free(tempNumNeighbours);
}

/* Accessor functions below for accessing arrays inside the class */

unsigned int **Delaunay_GetTriangleIndices( Delaunay *delaunay )
{
	assert( delaunay );

	return delaunay->triangleIndices;
}

float **Delaunay_GetVoronoiSides( Delaunay *delaunay )
{
	assert( delaunay );

	return delaunay->voronoiSides;
}

float *Delaunay_GetSurfaceArea( Delaunay *delaunay )
{
	assert( delaunay );

	return delaunay->voronoiArea;
}

unsigned int *Delaunay_GetNumNeighbours( Delaunay *delaunay )
{
	assert( delaunay );

	return delaunay->numNeighbours;
}

unsigned int **Delaunay_GetNeighbours( Delaunay *delaunay )
{
	assert( delaunay );

	return delaunay->neighbours;
}

int *Delaunay_GetHull( Delaunay *delaunay )
{
	assert( delaunay );

	return delaunay->hull;
}

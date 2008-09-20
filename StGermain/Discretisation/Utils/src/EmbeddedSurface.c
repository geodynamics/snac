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
** $Id: EmbeddedSurface.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"
#include "Discretisation/Mesh/Mesh.h"

#include "types.h"
#include "EmbeddedSurface.h"

#include <stdio.h>
#include <string.h>
#include <math.h>


const Type EmbeddedSurface_Type = "EmbeddedSurface";


EmbeddedSurface *EmbeddedSurface_New(Mesh *surface)
{
	return _EmbeddedSurface_New(sizeof(EmbeddedSurface),
				    EmbeddedSurface_Type,
				    _EmbeddedSurface_Delete,
				    _EmbeddedSurface_Print,
				    surface);
}


EmbeddedSurface *_EmbeddedSurface_New(SizeT			_sizeOfSelf,
				      Type			type,
				      Stg_Class_DeleteFunction*	_delete,
				      Stg_Class_PrintFunction*	_print,
				      Mesh			*surface)
{
	EmbeddedSurface *self;
	
	/* Allocate memory */
	self = (EmbeddedSurface *)_Stg_Class_New(_sizeOfSelf,
		type,
		_delete,
		_print);
		
	/* General info */
	self->surface = surface;
	
	/* Virtual info */
	
	/* EmbeddedSurface info */
	_EmbeddedSurface_Init(self);
	
	return self;
}


void EmbeddedSurface_Init(EmbeddedSurface *self)
{
	_EmbeddedSurface_Init(self);
}


void _EmbeddedSurface_Init(EmbeddedSurface *self)
{
	/* Note: assumes surface is an irregular tri surf */
	Element_GlobalIndex	edc = self->surface->elementDomainCount;
	Node			*nodeAt;
	Coord			*node;
	Triangle_List		element = ((TriSurfTopology*)self->surface->layout->topology)->elementTbl;
	Coord			*edgeVector;
	Element_Index		*eltBndryCnt;
	Index			i;
	
	/* Build coord list */
	node = Memory_Alloc_Array( Coord, self->surface->nodeDomainCount, "node" );
	for (i = 0; i < self->surface->nodeDomainCount; i++)
	{
		nodeAt = Mesh_NodeAt(self->surface, i);
		node[i][0] = ((Advectable *)nodeAt)->coord[0];
		node[i][1] = ((Advectable *)nodeAt)->coord[1];
		node[i][2] = ((Advectable *)nodeAt)->coord[2];
	}

	/* Build edge list and edge face list */
	self->edgeCount = Edge_BuildList_FromTriangles(element, edc, &self->edge, &self->edgeElement);
	
	/* Generate element normals/planes */
	self->elementPlane = Memory_Alloc_Array( Plane, edc, "EmbeddedSurface->elementPlace" );
	for (i = 0; i < edc; i++)
	{
		Coord a, b;

		Vector_Sub(a, node[element[i][1]], node[element[i][0]]);
		Vector_Sub(b, node[element[i][2]], node[element[i][0]]);
		Vector_Cross(self->elementPlane[i], a, b);
		Vector_Norm(self->elementPlane[i], self->elementPlane[i]);
		self->elementPlane[i][3] = Vector_Dot(self->elementPlane[i], node[element[i][0]]);
	}
	
	/* Order the edge faces, ie. edgeElement[i][0] = left face, edgeElement[i][1] = right face */
	edgeVector = Memory_Alloc_Array( Coord, self->edgeCount, "edgeVector" );
	for (i = 0; i < self->edgeCount; i++)
	{
		Node_Index ni0 = self->edge[i][0];
		Node_Index ni1 = self->edge[i][1];
		Element_Index ei0 = self->edgeElement[i][0];
		Node_Index ni2 = element[ei0][0] != ni1 ? element[ei0][0] :
				 (element[ei0][1] != ni1 ? element[ei0][1] :
				 element[ei0][2]);
		Coord elementVector, cross;

		Vector_Sub(edgeVector[i], node[ni1], node[ni0]);
		Vector_Sub(elementVector, node[ni2], node[ni0]);
		Vector_Cross(cross, edgeVector[i], elementVector);
		if (Vector_Dot(cross, self->elementPlane[ei0]) >= 0.0)
		{
			self->edgeElement[i][0] = self->edgeElement[i][1];
			self->edgeElement[i][1] = ei0;
		}
	}
	
	/* Build tables for determining distance from surface to point */
	eltBndryCnt = Memory_Alloc_Array( Element_Index, edc, "eltBndryCnt" );
	memset(eltBndryCnt, 0, sizeof(Element_Index)*edc);
	
	self->elementBoundary = Memory_Alloc_Array( TriangleBoundary, edc, "EmbeddedSurface->elementBoundary" );

	for (i = 0; i < self->edgeCount; i++)
	{
		Coord plane;
		
		if (self->edgeElement[i][0] < edc)
		{
			Element_Index	eiLeft = self->edgeElement[i][0];
			Index		epiLeft = eltBndryCnt[eiLeft]++;

			Vector_Set(plane, self->elementPlane[eiLeft]);
			
			if (self->edgeElement[i][1] < edc)
			{
				Element_Index	eiRight = self->edgeElement[i][1];
				Index		epiRight = eltBndryCnt[eiRight]++;

				Vector_Add(plane, plane, self->elementPlane[eiRight]);
				
				Vector_Cross(self->elementBoundary[eiLeft][epiLeft], edgeVector[i], plane);
				Vector_Mult(self->elementBoundary[eiRight][epiRight], self->elementBoundary[eiLeft][epiLeft], -1.0);
				Vector_Norm(self->elementBoundary[eiLeft][epiLeft], self->elementBoundary[eiLeft][epiLeft]);
				Vector_Norm(self->elementBoundary[eiRight][epiRight], self->elementBoundary[eiRight][epiRight]);
				self->elementBoundary[eiLeft][epiLeft][3] = Vector_Dot(self->elementBoundary[eiLeft][epiLeft], node[self->edge[i][0]]);
				self->elementBoundary[eiRight][epiRight][3] = Vector_Dot(self->elementBoundary[eiRight][epiRight], node[self->edge[i][0]]);
			}
			else
			{
				Vector_Cross(self->elementBoundary[eiLeft][epiLeft], edgeVector[i], plane);
				Vector_Norm(self->elementBoundary[eiLeft][epiLeft], self->elementBoundary[eiLeft][epiLeft]);
				self->elementBoundary[eiLeft][epiLeft][3] = Vector_Dot(self->elementBoundary[eiLeft][epiLeft], node[self->edge[i][0]]);
			}
		}
		else
		{
			Element_Index	eiRight = self->edgeElement[i][1];
			Index		epiRight = eltBndryCnt[eiRight]++;

			Vector_Set(plane, self->elementPlane[eiRight]);
				
			Vector_Cross(self->elementBoundary[eiRight][epiRight], edgeVector[i], plane);
			Vector_Mult(self->elementBoundary[eiRight][epiRight], self->elementBoundary[eiRight][epiRight], -1.0);
			Vector_Norm(self->elementBoundary[eiRight][epiRight], self->elementBoundary[eiRight][epiRight]);
			self->elementBoundary[eiRight][epiRight][3] = Vector_Dot(self->elementBoundary[eiRight][epiRight], node[self->edge[i][0]]);
		}
	}
	
	if (eltBndryCnt) Memory_Free(eltBndryCnt);
	if (edgeVector) Memory_Free(edgeVector);
	if (node) Memory_Free(node);
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Generic virtual functions
*/

void _EmbeddedSurface_Delete(void *embeddedSurface)
{
	EmbeddedSurface	*self = (EmbeddedSurface *)embeddedSurface;
	
	if (self->edge) Memory_Free(self->edge);
	if (self->edgeElement) Memory_Free(self->edgeElement);
	if (self->elementPlane) Memory_Free(self->elementPlane);
	if (self->elementBoundary) Memory_Free(self->elementBoundary);
	
	/* Stg_Class_Delete parent class */
	_Stg_Class_Delete(embeddedSurface);
}


void _EmbeddedSurface_Print(void *embeddedSurface)
{
	EmbeddedSurface	*self = (EmbeddedSurface *)embeddedSurface;
	Element_GlobalIndex edc = self->surface->elementDomainCount;
	Index i, j;
	
	/* Set the Journal for printing informations */
	Stream* embeddedSurfaceStream = Journal_Register( InfoStream_Type,	"EmbeddedSurfaceStream");
	
	Journal_Printf( embeddedSurfaceStream, "EmbeddedSurface (%p):\n", self);
	Journal_Printf( embeddedSurfaceStream, "\tsurface: %p\n", self->surface);
	
	Journal_Printf( embeddedSurfaceStream, "\tedgeCount: %u\n", self->edgeCount);
	for (i = 0; i < self->edgeCount; i++)
		Journal_Printf( embeddedSurfaceStream, "\t\tedge[%u]: %u-->%u\n", i, self->edge[i][0], self->edge[i][1]);
	for (i = 0; i < self->edgeCount; i++)
	{
		Journal_Printf( embeddedSurfaceStream, "\t\tedgeElement[%u]: ", i);
		if (self->edgeElement[i][0] < edc)
			Journal_Printf( embeddedSurfaceStream, "left %03u, ", self->edgeElement[i][0]);
		else
			Journal_Printf( embeddedSurfaceStream, "left xxx, ");
		if (self->edgeElement[i][1] < edc)
			Journal_Printf( embeddedSurfaceStream, "right %03u\n", self->edgeElement[i][1]);
		else
			Journal_Printf( embeddedSurfaceStream, "right xxx\n");
	}
	
	for (i = 0; i < edc; i++)
		Journal_Printf( embeddedSurfaceStream, "\telementPlane[%u]: %.3fx + %.3fy + %.3fz = %.3f\n", i, self->elementPlane[i][0], 
			self->elementPlane[i][1], self->elementPlane[i][2], self->elementPlane[i][3]);
	for (i = 0; i < edc; i++)
		for (j = 0; j < 3; j++)
			Journal_Printf( embeddedSurfaceStream, "\telementBoundary[%u][%u]: %.3fx + %.3fy + %.3fz = %.3f\n", i, j, self->elementBoundary[i][j][0], 
				self->elementBoundary[i][j][1], self->elementBoundary[i][j][2], self->elementBoundary[i][j][3]);

	/* Print parent class */
	_Stg_Class_Print(embeddedSurface);
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Utility functions
*/

Element_GlobalIndex EmbeddedSurface_BuildIntersection(EmbeddedSurface *self, Element_GlobalIndex *intersect)
{
	MeshLayout		*spatial = ((IrregularMeshDecomp *)self->surface->layout->decomp)->_layout;
	Node			*node;
	Node_GlobalIndex	ndc = self->surface->nodeDomainCount;
	Element_GlobalIndex	be, intersectCnt = 0;
	Index i, j;

	for (i = 0; i < ndc; i++)
	{
		node = Mesh_NodeAt(self->surface, i);
		be = MeshLayout_ElementWithPoint(spatial, ((Advectable *)node)->coord);

		for (j = 0; j < intersectCnt; j++)
			if (intersect[j] == be) break;
		if (j < intersectCnt) continue;
		
		intersect[intersectCnt++] = be;
	}
	
	return intersectCnt;
}


double EmbeddedSurface_DistanceToPoint(EmbeddedSurface *self, Coord point)
{
	Element_GlobalIndex	edc = self->surface->elementDomainCount;
	double			dist[2];
	Bool			first = True;
	Index			i, j;

	for (i = 0; i < edc; i++)
	{
		for (j = 0; j < 3; j++)
		{
			dist[1] = Vector_Dot(self->elementBoundary[i][j], point);
			if (dist[1] < self->elementBoundary[i][j][3])
				break;
		}
		if (j < 3) continue;
		
		dist[1] = Vector_Dot(self->elementPlane[i], point);
		dist[1] = fabs(dist[1] - self->elementPlane[i][3]);
		
		if (!first)
		{
			if (dist[1] < dist[0])
				dist[0] = dist[1];
		}
		else
		{
			dist[0] = dist[1];
			first = False;
		}
	}
	
	if (first)
		Journal_Printf( (void*) stream,  *** Warning *** : Point in DistanceToPoint outside plane domain\n");
	
	return dist[0];
}

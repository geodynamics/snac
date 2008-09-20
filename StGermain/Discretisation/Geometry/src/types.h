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
**	Basic framework types.
**
** Assumptions:
**	None as yet.
**
** Comments:
**	None as yet.
**
** $Id: types.h 4026 2007-03-13 11:42:25Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Geometry_types_h__
#define __Discretisation_Geometry_types_h__


	typedef Index		NeighbourIndex;

	/** Describes a plane in 3D space, ie. Ax + By + Cz = d */
	typedef double		Plane[4];             
	/** Type used for indexing into lists of planes */ 
	typedef Index		Plane_Index;
	typedef Plane*		Plane_List;


	/** Stores three indices per triangle, each index refering to a coordinate stored in an external list */
	typedef Index		Triangle[3];
	/** Type used for indexing into lists of triangles */
	typedef Index		Triangle_Index;
	/** A type used to point to the head of a list of triangles */
	typedef Triangle*	Triangle_List;
	
	/** Stores the three planes needed to describe the 3D boundary of a triangle */
	typedef Plane		TriangleBoundary[3];

	/** Describes an edge between two points. Stored as two indices into an external coordinate list */
	typedef Index		Edge[2];
	/** Type used for indexing into lists of edges */
	typedef Index		Edge_Index;
	/** A type used to point to the head of a list of edges */
	typedef Edge*		Edge_List;

	/** Every edge can be touching at most two faces/elements/triangles. This type consists of two indices into
	 an external list of faces/elements/triangles describing the faces touched by an edge. */	
	typedef Index		EdgeFaces[2];
	typedef EdgeFaces*	EdgeFaces_List;
	typedef Index		EdgeFaces_Index;

	typedef double		Stg_Line[4];
	typedef Index		Line_Index;
	typedef Stg_Line*	Stg_Line_List;

	typedef Coord		Line3[2];
	
	
	typedef double		RMatrix[9];
	
	typedef Index		Dimension_Index;
	typedef Index		IJK[3];				/* ijk indices, positive, not constrained */
	typedef Index*		Indices;			/* array/list of indices */
	typedef enum Axis { I_AXIS=0, J_AXIS=1, K_AXIS=2 } Axis;

	typedef Coord*				Coord_List;
	typedef Index				Coord_Index;
	typedef long int 			QuadEdgeRef;

	typedef struct Topology			Topology;
	typedef struct IJKTopology		IJKTopology;
	typedef struct IJK6Topology		IJK6Topology;
	typedef struct IJK26Topology	IJK26Topology;
	typedef struct IrregTopology	IrregTopology;
	typedef struct Geometry			Geometry;
	typedef struct BlockGeometry	BlockGeometry;
	typedef struct RefinedRegionsGeometry	RefinedRegionsGeometry;
	typedef struct ShellGeometry	ShellGeometry;
	typedef struct IrregGeometry	IrregGeometry;
	typedef struct QuadEdge			QuadEdge;
	typedef struct QuadEdgePool		QuadEdgePool;
	typedef struct VoronoiVertexPool		VoronoiVertexPool;
	typedef struct Delaunay			Delaunay;
	typedef struct ParallelDelaunay	ParallelDelaunay;

	
#endif /* __Discretisation_Geometry_types_h__ */


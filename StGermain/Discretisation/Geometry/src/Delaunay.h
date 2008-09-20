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
**	Calculates the delaunay triangulation and the voronoi diangram of a set of points.
**
** Assumptions:
**
** Comments:
**
** $Id: Delaunay.h 3462 2006-02-19 06:53:24Z Raquibul Hassan$
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Geometry_Delaunay_h__
#define __Discretisation_Geometry_Delaunay_h__

	/* Virtual function types */
	
	/** Textual name of this class */
	extern const Type Delaunay_Type;

	typedef struct DelaunayAttributes_t{
		int	BuildBoundingTriangle;
		int BuildTriangleIndices;
		int BuildTriangleNeighbours;
		int CreateVoronoiVertices;
		int CalculateVoronoiSides;
		int CalculateVoronoiSurfaceArea;
		int FindNeighbours;
	}DelaunayAttributes;

	typedef struct Site_t{
		CoordF *coord;
		int id;
	}Site;

	/** Delaunay class contents (see Delaunay) */
	#define __Delaunay \
		__Stg_Component \
		Dictionary			*dictionary; \
		MemoryPool			*qp; \
		MemoryPool			*vp; \
		int					numSites; \
		int					numInputSites; \
		CoordF				*points; \
		Site				*sites; \
		int					idOffset; \
		CoordF				boundingTriangle[3]; \
		int					numEdges; \
		int					numVoronoiSites; \
		int					numTriangles; \
		int					numFaces; \
		unsigned int		**triangleIndices; \
		unsigned int		**triangleNeighbours; \
		int					numVoronoiVertices; \
		unsigned int		*numNeighbours; \
		unsigned int		**neighbours; \
		float				**voronoiSides; \
		float				*voronoiArea; \
		int					*hull; \
		QuadEdgeRef			rightMost; \
		QuadEdgeRef			leftMost; \
		DelaunayAttributes *attributes;
	
	struct Delaunay { __Delaunay };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	
	/** Create a Delaunay */
	Delaunay* Delaunay_DefaultNew( Name name );
	
	Delaunay* Delaunay_New(
		Name						name,
		Dictionary*					dictionary,
		CoordF						*sites,
		int							numSites,
		int							idOffset,
		DelaunayAttributes			*attr );
	
	/** Initialise a Delaunay */
	void Delaunay_Init(
		Delaunay*					self,
		Name						name,
		Dictionary*					dictionary,
		CoordF						*sites,
		int							numSites,
		int							idOffset,
		DelaunayAttributes			*attr );
	
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
		DelaunayAttributes			*attr );
	
	void _Delaunay_Init( Delaunay* self );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/** Stg_Class_Delete() implementation */
	void _Delaunay_Delete( void* delaunay );
	
	/** Stg_Class_Print() implementation */
	void _Delaunay_Print( void* delaunay, Stream* stream );
	
	void *_Delaunay_Copy( void* delaunay, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	void _Delaunay_Construct( void* delaunay, Stg_ComponentFactory* cf, void* data );
	
	void _Delaunay_Build( void* delaunay, void* data );
	
	void _Delaunay_Initialise( void* delaunay, void* data );
	
	void _Delaunay_Execute( void* delaunay, void* data );
	
	void _Delaunay_Destroy( void* delaunay, void* data );

	unsigned int **Delaunay_GetTriangleIndices( Delaunay *delaunay );
	float **Delaunay_GetVoronoiSides( Delaunay *delaunay );
	float *Delaunay_GetSurfaceArea( Delaunay *delaunay );
	unsigned int *Delaunay_GetNumNeighbours( Delaunay *delaunay );
	unsigned int **Delaunay_GetNeighbours( Delaunay *delaunay );
	int *Delaunay_GetHull( Delaunay *delaunay );
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/
	void Delaunay_SortSites(Site *sites, int numSites );
	int RightOf(Site *s, QuadEdgeRef e);
	int LeftOf(Site *s, QuadEdgeRef e);
	int InCircle(Site *a, Site *b, Site *c, Site *d);
	void Delaunay_Recurse( Delaunay *delaunay, int sl, int sh, QuadEdgeRef *le, QuadEdgeRef *re );
	void Delaunay_FindHull( Delaunay *delaunay );
	void Delaunay_BuildTriangleIndices( Delaunay *delaunay );
	void Delaunay_BuildVoronoiVertices( Delaunay *delaunay );
	void Delaunay_FindNeighbours( Delaunay *delaunay );
	void Delaunay_FindMinMax( Site *sites, int count, float *minX, float *minY, float *maxX, float *maxY );
	
#endif /* __Discretisation_Geometry_Delaunay_h__ */

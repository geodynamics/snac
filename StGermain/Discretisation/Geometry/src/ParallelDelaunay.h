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
**	Calculates the ParallelDelaunay triangulation and the voronoi diangram of a set of points.
**
** Assumptions:
**
** Comments:
**
** $Id: ParallelDelaunay.h 3462 2006-02-19 06:53:24Z Raquibul Hassan$
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Geometry_ParallelDelaunay_h__
#define __Discretisation_Geometry_ParallelDelaunay_h__

	/* Virtual function types */
	
	/** Textual name of this class */
	extern const Type ParallelDelaunay_Type;

	/** ParallelDelaunay class contents (see Delaunay) */
	#define __ParallelDelaunay \
		__Delaunay \
		CoordF		*localPoints; \
		int			*initialOrder; \
		int			leftProc; \
		int			rightProc; \
		int			numProcs; \
		BTree		*haloSites[2]; \
		int			numHaloSites[2]; \
		Delaunay	*localTriangulation; \
		int			*mappingTable[2]; \
		int			*mapGlobalToLocal; \
		int			*processor; \
		int			*processorLoad; \
		int			rank; \
		int			numLocalSites; \
		int			numTotalLocalSites; \
		MPI_Comm	*comm;
	struct ParallelDelaunay { __ParallelDelaunay };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	
	/** Create a ParallelDelaunay */
	ParallelDelaunay* ParallelDelaunay_DefaultNew( Name name );
	
	ParallelDelaunay* ParallelDelaunay_New(
		Name						name,
		Dictionary*					dictionary,
		CoordF						*sites,
		int							numSites,
		int							rank,
		int							numProcs,
		MPI_Comm					*comm,
		DelaunayAttributes	*attr );
	
	/** Initialise a ParallelDelaunay */
	void ParallelDelaunay_Init(
		ParallelDelaunay*					self,
		Name						name,
		Dictionary*					dictionary,
		CoordF						*sites,
		int							numSites,
		int							rank,
		int							numProcs,
		MPI_Comm					*comm,
		DelaunayAttributes			*attr );
	
	/** Creation implementation */
	ParallelDelaunay* _ParallelDelaunay_New(
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
		int							rank,
		int							numProcs,
		MPI_Comm					*comm,
		DelaunayAttributes			*attr );
	
	void _ParallelDelaunay_Init( ParallelDelaunay* self );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/** Stg_Class_Delete() implementation */
	void _ParallelDelaunay_Delete( void* pd );
	
	/** Stg_Class_Print() implementation */
	void _ParallelDelaunay_Print( void* pd, Stream* stream );
	
	void *_ParallelDelaunay_Copy( void* pd, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	void _ParallelDelaunay_Construct( void* pd, Stg_ComponentFactory* cf, void* data );
	
	void _ParallelDelaunay_Build( void* pd, void* data );
	
	void _ParallelDelaunay_Initialise( void* pd, void* data );
	
	void _ParallelDelaunay_Execute( void* pd, void* data );
	
	void _ParallelDelaunay_Destroy( void* pd, void* data );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/
	void ParallelDelaunay_BuildTriangleIndices( ParallelDelaunay *pd );
	void ParallelDelaunaySendEdge( QuadEdgeRef edge, int rank, MPI_Comm *comm, MPI_Request *req );
	QuadEdgeRef ParallelDelaunayRecvEdge( Delaunay *d, int rank, MPI_Comm *comm );
	QuadEdgeRef ParallelDelaunayFindLowestQuadEdge( ParallelDelaunay *pd, MPI_Comm *comm, int rank );
	void ParallelDelaunayMerge( ParallelDelaunay *pd, MPI_Comm *comm, int rank );
	int ParallelDelaunay_TranslateLocalToGlobal( ParallelDelaunay *self, int id );
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public Member functions
	*/
	
	void ParallelDelaunay_GatherTriangulation( ParallelDelaunay *pd );
	
#endif /* __Discretisation_Geometry_ParallelDelaunay_h__ */

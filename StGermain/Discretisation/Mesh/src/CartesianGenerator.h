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
**
** Assumptions:
**
** Invariants:
**
** Comments:
**
** $Id: CartesianGenerator.h 3584 2006-05-16 11:11:07Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisaton_Mesh_CartesianGenerator_h__
#define __Discretisaton_Mesh_CartesianGenerator_h__

	/** Textual name of this class */
	extern const Type CartesianGenerator_Type;

	/** Virtual function types */

	/** CartesianGenerator class contents */
	#define __CartesianGenerator		\
		/* General info */		\
		__MeshGenerator			\
						\
		/* Virtual info */		\
						\
		/* CartesianGenerator info */	\
		unsigned	maxDecompDims;	\
		unsigned*	minDecomp;	\
		unsigned*	maxDecomp;	\
		unsigned	shadowDepth;	\
		double*		crdMin;		\
		double*		crdMax;		\
						\
		Grid*		topoGrid;	\
		Grid*		procGrid;	\
		unsigned*	origin;		\
		unsigned*	range;

	struct CartesianGenerator { __CartesianGenerator };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/

	#define CARTESIANGENERATOR_DEFARGS	\
		MESHGENERATOR_DEFARGS

	#define CARTESIANGENERATOR_PASSARGS	\
		MESHGENERATOR_PASSARGS

	CartesianGenerator* CartesianGenerator_New( Name name );
	CartesianGenerator* _CartesianGenerator_New( CARTESIANGENERATOR_DEFARGS );
	void _CartesianGenerator_Init( CartesianGenerator* self );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _CartesianGenerator_Delete( void* generator );
	void _CartesianGenerator_Print( void* generator, Stream* stream );

	#define CartesianGenerator_Copy( self ) \
		(Mesh*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define CartesianGenerator_DeepCopy( self ) \
		(Mesh*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _CartesianGenerator_Copy( void* generator, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );

	void _CartesianGenerator_Construct( void* generator, Stg_ComponentFactory* cf, void* data );
	void _CartesianGenerator_Build( void* generator, void* data );
	void _CartesianGenerator_Initialise( void* generator, void* data );
	void _CartesianGenerator_Execute( void* generator, void* data );
	void _CartesianGenerator_Destroy( void* generator, void* data );

	void CartesianGenerator_Generate( void* generator, void* _mesh );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	void CartesianGenerator_SetTopologyParams( void* generator, unsigned nDims, unsigned* sizes, 
						   unsigned maxDecompDims, unsigned* minDecomp, unsigned* maxDecomp );
	void CartesianGenerator_SetGeometryParams( void* generator, double* min, double* max );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

	void CartesianGenerator_BuildDecomp( CartesianGenerator* self );
	void CartesianGenerator_RecurseDecomps( CartesianGenerator* self, 
						unsigned dim, unsigned max, 
						unsigned* nSubDomains, 
						unsigned* nPos, unsigned*** posNSubDomains );
	void CartesianGenerator_GenTopo( CartesianGenerator* self, MeshTopology* topo );
	void CartesianGenerator_GenElements( CartesianGenerator* self, MeshTopology* topo, Grid*** grids );
	void CartesianGenerator_GenVertices( CartesianGenerator* self, MeshTopology* topo, Grid*** grids );
	void CartesianGenerator_GenEdges( CartesianGenerator* self, MeshTopology* topo, Grid*** grids );
	void CartesianGenerator_GenEdges2D( CartesianGenerator* self, MeshTopology* topo, Grid*** grids );
	void CartesianGenerator_GenEdges3D( CartesianGenerator* self, MeshTopology* topo, Grid*** grids );
	void CartesianGenerator_GenFaces( CartesianGenerator* self, MeshTopology* topo, Grid*** grids );
	void CartesianGenerator_GenElementVertexInc( CartesianGenerator* self, MeshTopology* topo, Grid*** grids );
	void CartesianGenerator_GenVolumeFaceInc( CartesianGenerator* self, MeshTopology* topo, Grid*** grids );
	void CartesianGenerator_GenFaceEdgeInc( CartesianGenerator* self, MeshTopology* topo, Grid*** grids );
	void CartesianGenerator_GenEdgeVertexInc( CartesianGenerator* self, MeshTopology* topo, Grid*** grids );
	void CartesianGenerator_MapToDomain( CartesianGenerator* self, Decomp_Sync* sync, 
					     unsigned size, unsigned* nIncEls, unsigned** incEls );
	void CartesianGenerator_GenGeom( CartesianGenerator* self, Mesh* mesh );
	void CartesianGenerator_Destruct( CartesianGenerator* self );

#endif /* __Discretisaton_Mesh_CartesianGenerator_h__ */

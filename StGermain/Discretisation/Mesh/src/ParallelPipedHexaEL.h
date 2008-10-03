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
**	Manages the layout of elements in a mesh, where the user wishes to have regular
**	"box" elements. Automatically creates a "Block" Geometry, and an IJK26Topology.
**
** Assumptions:
**	At the moment, the calculation of minLocalThisPartition, maxLocalThisPartition
**	etc, and also the elementWithPoint() function, assumes the decomposition used
**	is a HexaMD. Would need to generalise them if this wasn't the case.
**
** Comments:
**
** $Id: ParallelPipedHexaEL.h 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisaton_Mesh_ParallelPipedHexaEL_h__
#define __Discretisaton_Mesh_ParallelPipedHexaEL_h__


	/* Virtual function types */
	
	/* Textual name of this class */
	extern const Type ParallelPipedHexaEL_Type;

	/* ParallelPipedHexaEL information */
	#define __ParallelPipedHexaEL \
		/* General info */ \
		__HexaEL \
		\
		/* Virtual info */ \
		\
		/* ParallelPipedHexaEL info ... */ \
		Coord			minLocalThisPartition; /** Bottom corner of local nodes */\
		Coord			maxLocalThisPartition; /** Top corner of local nodes */\
		Coord			minDomainThisPartition; /** Bottom corner of domain (inc shadow) nodes */\
		Coord			maxDomainThisPartition; /** Top corner of domain (inc shadow) nodes */\
		double			elementLengthEachDim[3]; /** Dimensions (l*w*h) of any given element */\
		Bool                    geometryWasCreatedInternally; /** Necessary due to component register */
		
	struct _ParallelPipedHexaEL { __ParallelPipedHexaEL };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	
	/* Create a ParallelPipedHexaEL */
	ParallelPipedHexaEL* ParallelPipedHexaEL_DefaultNew( Name name );
	
	ParallelPipedHexaEL* ParallelPipedHexaEL_New(
		Name						name,
		Dimension_Index             dim,
		Dictionary*					dictionary );

	/* Initialise a ParallelPipedHexaEL */
	void ParallelPipedHexaEL_Init(
		ParallelPipedHexaEL*				self,
		Name						name,
		Dimension_Index             dim,
		Dictionary*					dictionary );
	
	/* Creation implementation */
	ParallelPipedHexaEL* _ParallelPipedHexaEL_New(
		SizeT                                                   _sizeOfSelf, 
		Type                                                    type,
		Stg_Class_DeleteFunction*                               _delete,
		Stg_Class_PrintFunction*                                _print,
		Stg_Class_CopyFunction*                                 _copy, 
		Stg_Component_DefaultConstructorFunction*               _defaultConstructor,
		Stg_Component_ConstructFunction*                        _construct,
		Stg_Component_BuildFunction*                            _build,
		Stg_Component_InitialiseFunction*                       _initialise,
		Stg_Component_ExecuteFunction*                          _execute,
		Stg_Component_DestroyFunction*                          _destroy,
		Name                                                    name,
		Bool                                                    initFlag,
		ElementLayout_BuildFunction*                            build,		
		ElementLayout_BuildCornerIndicesFunction*               buildCornerIndices,
		ElementLayout_CornerElementCountFunction*               cornerElementCount,
		ElementLayout_BuildCornerElementsFunction*              buildCornerElements,
		ElementLayout_BuildEdgeIndicesFunction*                 buildEdgeIndices,
		ElementLayout_EdgeElementCountFunction*                 edgeElementCount,
		ElementLayout_BuildEdgeElementsFunction*                buildEdgeElements,
		ElementLayout_EdgeAtFunction*                           edgeAt,
		ElementLayout_GetStaticMinAndMaxLocalCoordsFunction*    getStaticMinAndMaxLocalCoords,
		ElementLayout_GetStaticMinAndMaxGlobalCoordsFunction*   getStaticMinAndMaxGlobalCoords,				
		ElementLayout_ElementWithPointFunction*                 elementWithPoint,
		Dimension_Index                                         dim,
		Dictionary*                                             dictionary );

	/* Initialisation implementation functions */
	void _ParallelPipedHexaEL_Init(
		ParallelPipedHexaEL*				self );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/* Stg_Class_Delete ParallelPipedHexaEL implementation */
	void _ParallelPipedHexaEL_Delete( void* parallelPipedHexaEL );
	
	/* Print ParallelPipedHexaEL implementation */
	void _ParallelPipedHexaEL_Print( void* parallelPipedHexaEL, Stream* stream );
	
	/* Copy */
	#define ParallelPipedHexaEL_Copy( self ) \
		(ParallelPipedHexaEL*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define ParallelPipedHexaEL_DeepCopy( self ) \
		(ParallelPipedHexaEL*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _ParallelPipedHexaEL_Copy( void* ppHexaEL, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	void _ParallelPipedHexaEL_Construct( void* ppHexaEL, Stg_ComponentFactory* cf, void* data );
	
	void _ParallelPipedHexaEL_Stg_ComponentBuild( void* ppHexaEL, void* data );
	
	void _ParallelPipedHexaEL_Initialise( void* ppHexaEL, void* data );
	
	void _ParallelPipedHexaEL_Execute( void* ppHexaEL, void* data );
	
	void _ParallelPipedHexaEL_Destroy( void* ppHexaEL, void* data );
	
	void _ParallelPipedHexaEL_Build( void* parallelPipedHexaEL, void* decomp );
	
	Bool _ParallelPipedHexaEL_GetStaticMinAndMaxLocalCoords( void* parallelPipedHexaEL, Coord min, Coord max );
	
	Bool _ParallelPipedHexaEL_GetStaticMinAndMaxGlobalCoords( void* parallelPipedHexaEL, Coord min, Coord max );

	Element_GlobalIndex _ParallelPipedHexaEL_ElementWithPoint( void* parallelPipedHexaEL, void* decomp, Coord point, void* mesh, 
								   PartitionBoundaryStatus boundaryStatus, unsigned nHints, unsigned* hints );
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public member functions
	*/
	
	void ParallelPipedHexaEL_UpdateGeometryPartitionInfo( void* parallelPipedHexaEL, void* decomp );
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/
	
	
#endif /* __Discretisaton_Mesh_ParallelPipedHexaEL_h__ */

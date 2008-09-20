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
** Comments:
**
** $Id: HexaMD.h 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisaton_Mesh_HexaMD_h__
#define __Discretisaton_Mesh_HexaMD_h__


	/* Virtual function types */
	
	/* Textual name of this class */
	extern const Type HexaMD_Type;

	/* HexaMD information */
	#define __HexaMD \
		/* General info */ \
		__MeshDecomp \
		\
		/* Virtual info */ \
		\
		/* HexaMD info ... */ \
		Index				numPartitionedDims; \
		Bool				partitionedAxis[3]; \
		Partition_IJK			partition3DCounts; \
		\
		Node_GlobalIJK			nodeGlobal3DCounts; \
		Node_LocalIJK*			nodeLocal3DCounts; \
		Node_GlobalIJK*			_nodeOffsets; \
		\
		Element_GlobalIJK		elementGlobal3DCounts; \
		Element_LocalIJK*		elementLocal3DCounts; \
		Element_DomainIJK		elementDomain3DCounts; \
		Element_GlobalIJK*		_elementOffsets;

	struct _HexaMD { __HexaMD };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	
	#define HexaMD_New( name, dictionary, communicator, eLayout, nLayout ) \
		HexaMD_New_All( name, dictionary, communicator, eLayout, nLayout, 1 )

	/* Create a HexaMD */
	
	/* Create a HexaMD */
	HexaMD* HexaMD_DefaultNew( Name name );
	
	HexaMD* HexaMD_New_All(
		Name							name,
		Dictionary*					dictionary,
		MPI_Comm					communicator,
		ElementLayout*					eLayout,
		NodeLayout*					nLayout,
		unsigned char					numPartitionedDims );
	
	/* Initialise a HexaMD */
	void HexaMD_Init(
		HexaMD*						self,
		Name							name,
		Dictionary*					dictionary,
		MPI_Comm					communicator,
		ElementLayout*					eLayout,
		NodeLayout*					nLayout,
		unsigned char					numPartitionedDims );

	/* Creation implementation */
	HexaMD* _HexaMD_New(
		SizeT						_sizeOfSelf, 
		Type						type,
		Stg_Class_DeleteFunction*				_delete,
		Stg_Class_PrintFunction*				_print,
		Stg_Class_CopyFunction*				_copy, 
		Stg_Component_DefaultConstructorFunction*	_defaultConstructor,
		Stg_Component_ConstructFunction*			_construct,
		Stg_Component_BuildFunction*				_build,
		Stg_Component_InitialiseFunction*			_initialise,
		Stg_Component_ExecuteFunction*				_execute,
		Stg_Component_DestroyFunction*				_destroy,
		Name									name,
		Bool									initFlag,
		MeshDecomp_Node_MapLocalToGlobalFunction*       nodeMapLocalToGlobal,
		MeshDecomp_Node_MapShadowToGlobalFunction*      nodeMapShadowToGlobal,
		MeshDecomp_Node_MapDomainToGlobalFunction*      nodeMapDomainToGlobal,
		MeshDecomp_Node_MapGlobalToLocalFunction*       nodeMapGlobalToLocal,
		MeshDecomp_Node_MapGlobalToShadowFunction*      nodeMapGlobalToShadow,
		MeshDecomp_Node_MapGlobalToDomainFunction*      nodeMapGlobalToDomain,
		MeshDecomp_Element_MapLocalToGlobalFunction*	elementMapLocalToGlobal,
		MeshDecomp_Element_MapShadowToGlobalFunction*	elementMapShadowToGlobal,
		MeshDecomp_Element_MapDomainToGlobalFunction*	elementMapDomainToGlobal,
		MeshDecomp_Element_MapGlobalToLocalFunction*	elementMapGlobalToLocal,
		MeshDecomp_Element_MapGlobalToShadowFunction*	elementMapGlobalToShadow,
		MeshDecomp_Element_MapGlobalToDomainFunction*	elementMapGlobalToDomain,
		MeshDecomp_Shadow_ProcCountFunction*		shadowProcCount,
		MeshDecomp_Shadow_BuildProcsFunction*		shadowBuildProcs,
		MeshDecomp_Shadow_ProcElementCountFunction*     shadowProcElementCount,
		MeshDecomp_Proc_WithElementFunction*		procWithElement,
		Dictionary*					dictionary,
		MPI_Comm					communicator,
		ElementLayout*					eLayout,
		NodeLayout*					nlayout,
		unsigned char					numPartitionedDims );

	/* Initialisation implementation functions */
	void _HexaMD_Init(
		HexaMD*						self,
		unsigned char					numPartitionedDims );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/* Stg_Class_Delete HexaMD implementation */
	void _HexaMD_Delete( void* hexaMD );
	
	/* Print HexaMD implementation */
	void _HexaMD_Print( void* hexaMD, Stream* stream );
	
	/* Copy */
	#define HexaMD_Copy( self ) \
		(HexaMD*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define HexaMD_DeepCopy( self ) \
		(HexaMD*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _HexaMD_Copy( void* hexaMD, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	void _HexaMD_Construct( void* hexaMD, Stg_ComponentFactory *cf, void* data );
	
	void _HexaMD_Build( void* hexaMD, void* data );
	
	void _HexaMD_Initialise( void* hexaMD, void* data );
	
	void _HexaMD_Execute( void* hexaMD, void* data );

	Partition_Index _HexaMD_Shadow_ProcCount( void* hexaMD );
	
	void _HexaMD_Shadow_BuildProcs( void* hexaMD, Partition_Index* procs );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public member functions
	*/
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/
	
	/** Function to calculate the set of Boundary elements of a given processor. The element
	    set given must be of size element global count */
	void _HexaMD_GetBoundarySet(
		HexaMD*            self,
		Processor_Index    proc_I,
		IndexSet*          boundaryElementSet );	    
	
	/** Function to calculate the shadow elements of a processor. Works for cases of shadow
		depth > 1 */
	void _HexaMD_BuildShadowSets( HexaMD* self, Processor_Index proc_I );
	

	/** Calculates the start and end of the lower shadow region in a given axis, taking into account periodicity.
		can be re-used for both nodes and elements by passing the appropriate parameters. */
	void _HexaMD_CalculateLowerShadowStartAndEnd(
		HexaMD*         self,
		Dimension_Index shadowAxis,
		MeshItemType    itemType,
		GlobalIndex     currProcMinInShadowAxis,
		GlobalIndex     globalCountInShadowAxis,            
		int*            startIndexInShadowAxis, /* needs to be an int rather than unsigned since it may be < zero in 
							* calcs */
		Index*          adjustedShadowDepth );

	/** Selecting start and end of upper shadow region, taking into account periodicity */
	void _HexaMD_CalculateUpperShadowStartAndEnd(
		HexaMD*         self,
		Dimension_Index shadowAxis,
		MeshItemType    itemType,
		GlobalIndex     currProcMaxInShadowAxis,
		GlobalIndex     globalCountInShadowAxis,            
		int*            startIndexInShadowAxis,
		Index*          adjustedShadowDepth );

	/** Fills the "corners" around a 2D or 3D block - used as part of setting up shadow nodes if more than
		one decomp dimension. Automatically handles periodicity in the outer block. */
	GlobalIndex _HexaMD_AddItemsInBlockCornersOutsideInnerBlockToSet( 
		HexaMD*         self,
		IJK             outerBlockMin,
		IJK             outerBlockCounts,
		IJK             innerBlockMin,
		IJK             innerBlockMax,
		IJK             itemGlobal3DCounts,   
		IndexSet*       itemSet );

	/** Utility function to fill an IndexSet representing global element/node indices with 1s for all
	     element/nodes that live inside a slice for a particular dimension. Slices can handle periodic meshes
	     	\param self HexaMD object
		\param sliceAxis the primary axis you wish to slice along
		\param startOfRange where the slice should start 
		\param widthOfSlice how wide the slice should be
		\param otherAxisMinimums IJK object containing the 3D coords of where the other parts of the slice start
			in the mesh
		\param otherAxisMaximums IJK object specifying where the other Axis end in space
		\param itemGlobal3DCounts 3D Count of global max in each dim
		\param itemSet the IndexSet to store the results in. */
	GlobalIndex  _HexaMD_AddItemsInSliceToSet(
		HexaMD*         self,
		Dimension_Index sliceAxis,
		Index           startOfRange,
		Index           widthOfSlice,
		IJK             otherAxisMinimums,
		IJK             otherAxisMaximums,
		IJK             itemGlobal3DCounts,   
		IndexSet*       itemSet );

	/** Decomposes the elements in a particular dimension, given the
	size of the partition and the number of processors available.
	
	If the number of processors doesn't divide evenly into the number of
	elements in the dimension to be decomposed on; the following occurs:

	If HexaMD::allowUnbalancing is True, then the remaining nodes will
	be distributed among the first few processors.

	If HexaMD::allowUnbalancing is False, then if and only if 
	HexaMD::allowUnusedCPUs is True, the number of processors
	to be used will be reduced until the division is even. Otherwise, an
	assert() and exit will occur.

	In either of the above 2 cases, the default behaviour is to decompose
	on nodes. Only if this is disallowed, or in the balanced case where
	the node partition is uneven, will partition on elements be used.

	TODO: Allow partition on node in the unbalanced case causes problems
	later on - not sure if it's a problem with this code or the
	way it's interpreted later.
	*/
	void _HexaMD_DecomposeDimension(
		HexaMD*				self,
		Element_GlobalIndex		numElementsThisDim,
		Partition_Index			procCount,
		Element_LocalIndex*		localElementCounts,
		Element_LocalIndex*		localElementOffsets );
	
	void _HexaMD_CalculateDomain3DCounts( HexaMD* self );
	
	void _HexaMD_Destroy( HexaMD* self, void *data );
	
#endif /* __Discretisaton_Mesh_HexaMD_h__ */

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
** $Id: types.h 3860 2006-10-14 18:12:14Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisaton_Mesh_types_h__
#define __Discretisaton_Mesh_types_h__
	
	/* Types user needs to define */
	typedef struct _Node			Node;
	typedef struct _Element			Element;

	/** Enum for functions that need to know type of mesh item they are operating on */
	typedef enum MeshItemType { ELEMENT_ITEM_TYPE, NODE_ITEM_TYPE } MeshItemType;

	/* Basic indexes */
	typedef unsigned int			GlobalIndex;			/* global 1D index, not constrained */
	typedef unsigned int			DomainIndex;			/* domain (local + shadow) 1D index, not 
											constrained */
	typedef unsigned int			LocalIndex;			/* local 1D index, not constrained */
	typedef unsigned int			ShadowIndex;			/* shadow 1D index, not constrained */

	/* Basic Node types */
	typedef GlobalIndex			Node_GlobalIndex;		/* global 1D, 0 -> globalNodeCount, > is error */
	typedef Index				Node_Index;			/* index, positive, not constrained */
	typedef DomainIndex			Node_DomainIndex;		/* domain 1D, 0 -> (local_shadow)NodeCount, > is 
											error */
	typedef LocalIndex			Node_LocalIndex;		/* local 1D, 0 -> localNodeCount, > is error */
	typedef ShadowIndex			Node_ShadowIndex;		/* shadow 1D, 0 -> shadowNodeCount, > is error */
	typedef Index				Node_ElementLocalIndex;		/* element local 1D, 0 -> count at given el.*/
	typedef Node*				Node_List;			/* array of Nodes */
	typedef Node_GlobalIndex		Node_GlobalIJK[3];
	typedef Node_LocalIndex			Node_LocalIJK[3];
	typedef Node_ShadowIndex		Node_ShadowIJK[3];
	typedef Node_DomainIndex		Node_DomainIJK[3];
	
	/* Basic Element types */
	typedef GlobalIndex			Element_GlobalIndex;		/* global 1D, 0 -> globalNodeCount, > is error  */
	typedef DomainIndex			Element_DomainIndex;		/* domain 1D, 0 -> (local_shadow)NodeCount, > is 
											error */
	typedef Element_GlobalIndex		Element_Index;			/* index, positive, not constrained */
	typedef LocalIndex			Element_LocalIndex;		/* local 1D, 0 -> localNodeCount, > is error */
	typedef ShadowIndex			Element_ShadowIndex;		/* shadow 1D, 0 -> shadowNodeCount, > is error */
	typedef Element*			Element_List;			/* array of Elements */
	typedef Element_GlobalIndex		Element_GlobalIJK[3];
	typedef Element_LocalIndex		Element_LocalIJK[3];
	typedef Element_ShadowIndex		Element_ShadowIJK[3];
	typedef Element_DomainIndex		Element_DomainIJK[3];
	
	/* Advanced Node types */
	typedef Node_DomainIndex*		Node_Neighbours;		/* array of domain indices of neighbouring nodes */
	typedef Index				Node_NeighbourIndex; 		/* index of a "node's neighbours" array */
	typedef Node_Neighbours*		Node_NeighboursList;		/* array of "node's neighbours" */
	typedef Node_NeighbourIndex*		Node_NeighbourIndexList;	/* array of counts of "node's neighbours" */
	
	typedef Element_DomainIndex*		Node_Elements;			/* array of domain indices of adjoining elements */
	typedef Index				Node_ElementIndex; 		/* index of a "node's elements" array */
	typedef Node_Elements*			Node_ElementsList;		/* array of "node's elements" */
	typedef Node_ElementIndex*		Node_ElementIndexList;		/* array of counts of "node's elements" */
	
	/* Advanced Element types */
	typedef Node_DomainIndex*		Element_Nodes;			/* array of domain indices of element nodes */
	typedef Index				Element_NodeIndex;		/* index of an "element's nodes" array */
	typedef Element_Nodes*			Element_NodesList;		/* array of "element's nodes" */
	typedef Element_NodeIndex*		Element_NodeIndexList;		/* array of counts of "element's nodes" */
	
	typedef Element_DomainIndex*		Element_Neighbours;		/* array of domain indices of neighbouring 
											elements */
	typedef Index				Element_NeighbourIndex;		/* index of an "element's neighbours" array */
	typedef Element_Neighbours*		Element_NeighboursList;		/* array of "element's neighbours" */
	typedef Element_NeighbourIndex*		Element_NeighbourIndexList;	/* array of counts of "element's neighbours" */
	
	typedef enum PartitionBoundaryStatus {
		EXCLUSIVE_UPPER_BOUNDARY,	/** The upper boundary of a processor's domain is not considered
						  * to be owned by that processor -except in the special case
						  * of the upper boundary of the entire mesh */
		INCLUSIVE_UPPER_BOUNDARY	/** The upper boundary of a processor's domain is always
						  * considered to be owned by the local processor. */
	} PartitionBoundaryStatus;
	
	/* Parallel types */
	typedef Index 				Partition_Index;			/* rank / domain partition index */
	typedef Partition_Index			PartitionIndex;
	typedef Partition_Index			Processor_Index;
	typedef Partition_Index			ProcessorIndex;
	typedef Partition_Index			Partition_IJK[3];
	/* For indexing processors by their neighbour number, not their
	global rank (ie in shadowing) */
	typedef Index				Neighbour_Index; 
	typedef Neighbour_Index			Nbr_Index;
	
	/* Classes */
	typedef struct Grid				Grid;
	typedef struct CommTopology			CommTopology;
	typedef struct Decomp				Decomp;
	typedef struct Decomp_Sync			Decomp_Sync;
	typedef struct Decomp_Sync_Claim		Decomp_Sync_Claim;
	typedef struct Decomp_Sync_Negotiate		Decomp_Sync_Negotiate;
	typedef struct DecompTransfer			DecompTransfer;
	typedef struct MeshTopology			MeshTopology;
	typedef struct MeshGenerator			MeshGenerator;
	typedef struct CartesianGenerator		CartesianGenerator;
	typedef struct MeshAdaptor			MeshAdaptor;
	typedef struct SurfaceAdaptor			SurfaceAdaptor;

	typedef struct ProcNbrInfo			ProcNbrInfo;
	typedef struct ShadowInfo			ShadowInfo;
	typedef struct _ElementLayout			ElementLayout;
	typedef struct _HexaEL				HexaEL;
	typedef struct _ParallelPipedHexaEL		ParallelPipedHexaEL;
	typedef struct _IrregEL				IrregEL;
	typedef struct _NodeLayout			NodeLayout;
	typedef struct _CornerNL			CornerNL;
	typedef struct _BodyNL				BodyNL;
	typedef struct MeshDecomp			MeshDecomp;
	typedef struct _HexaMD				HexaMD;
	typedef struct _StencilMD			StencilMD;
	typedef struct _DummyMD				DummyMD;
	typedef struct MeshGeometry			MeshGeometry;
	typedef struct _MeshLayout			MeshLayout;
	typedef struct _Mesh				Mesh;
	typedef struct _HMesh				HMesh;
	typedef struct _MeshCoarsener_Hexa		MeshCoarsener_Hexa;

	/* Streams */
	extern Stream*	Mesh_VerboseConfig;
	extern Stream*	Mesh_Debug; 	
	extern Stream*	Mesh_Warning; 
	extern Stream*	Mesh_Error;
	
#endif /* __Discretisaton_Mesh_types_h__ */

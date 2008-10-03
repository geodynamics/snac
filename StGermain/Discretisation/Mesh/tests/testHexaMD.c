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
** $Id: testHexaMD.c 3664 2006-07-04 04:26:57Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"
#include "Discretisation/Mesh/Mesh.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <assert.h>

struct _Node
{
	double temp;
};

struct _Element
{
	double temp;
};


void PrintDecompInfoOfHexaMD( HexaMD* decomp, int rank ) {	

	if( rank < decomp->procsInUse ) {
		Partition_Index		proc_I;
		Node_GlobalIndex	gNode_I;
		Node_LocalIndex		lNode_I;
		Node_DomainIndex	dNode_I;
		Element_GlobalIndex	gElt_I;
		Element_LocalIndex	lElt_I;
		Element_DomainIndex	dElt_I;
		Stream*                 outStream = Journal_Register( Info_Type, "testHexaMD" );
		
		printf( "Current processor is %d of %d in use (%d total).\n",
			rank, decomp->procsInUse, decomp->nproc );
		printf( "allowUnusedCPUs: %s\n", StG_BoolToStringMap[decomp->allowUnusedCPUs] );
		printf( "allowPartitionOnNode: %s\n", StG_BoolToStringMap[decomp->allowPartitionOnNode] );
		printf( "allowPartitionOnElement: %s\n", StG_BoolToStringMap[decomp->allowPartitionOnElement] );
		printf( "allowUnbalancing: %s\n", StG_BoolToStringMap[decomp->allowUnbalancing] );
		printf( "shadowDepth: %u\n", decomp->shadowDepth );
		printf( "procTopology:\n" );
		Stream_Indent( outStream );
		IJKTopology_PrintConcise( (IJKTopology*)decomp->procTopology, outStream );
		Stream_UnIndent( outStream );
		
		printf( "partitionedAxis: { %s, %s, %s }\n",
			StG_BoolToStringMap[decomp->partitionedAxis[0]], 
			StG_BoolToStringMap[decomp->partitionedAxis[1]],
			StG_BoolToStringMap[decomp->partitionedAxis[2]] );
		printf( "partitionCounts: { %u, %u, %u }\n", decomp->partition3DCounts[0], decomp->partition3DCounts[1], 
			decomp->partition3DCounts[2] );
		
		printf( "elementGlobalCounts: { %u, %u, %u }\n", decomp->elementGlobal3DCounts[0], decomp->elementGlobal3DCounts[1], 
			decomp->elementGlobal3DCounts[2] );
		printf( "elementGlobalCount: %u\n", decomp->elementGlobalCount ); 
		assert( decomp->elementGlobalCount == ( decomp->elementGlobal3DCounts[0] * decomp->elementGlobal3DCounts[1]
			* decomp->elementGlobal3DCounts[2] ) );
		for( proc_I = 0; proc_I < decomp->procsInUse; proc_I++ ) {
			printf( "\telementLocalCounts[%u]: { %u, %u, %u }\n", proc_I, decomp->elementLocal3DCounts[proc_I][0], 
				decomp->elementLocal3DCounts[proc_I][1], decomp->elementLocal3DCounts[proc_I][2] );
		}
		printf( "elementDomainCounts: { %u, %u, %u }\n", decomp->elementDomain3DCounts[0], decomp->elementDomain3DCounts[1], 
			decomp->elementDomain3DCounts[2] );
		
		printf( "nodeGlobalCounts: { %u, %u, %u }\n", decomp->nodeGlobal3DCounts[0], decomp->nodeGlobal3DCounts[1], 
			decomp->nodeGlobal3DCounts[2] );
		printf( "nodeGlobalCount: %u\n", decomp->nodeGlobalCount ); 
		assert( decomp->nodeGlobalCount == ( decomp->nodeGlobal3DCounts[0] * decomp->nodeGlobal3DCounts[1]
			* decomp->nodeGlobal3DCounts[2] ) );
		for( proc_I = 0; proc_I < decomp->procsInUse; proc_I++ ) {
			printf( "\tnodeLocalCounts[%u]: { %u, %u, %u }\n", proc_I, decomp->nodeLocal3DCounts[proc_I][0], 
				decomp->nodeLocal3DCounts[proc_I][1], decomp->nodeLocal3DCounts[proc_I][2] );
		}

		
		for (gElt_I = 0; gElt_I < decomp->elementGlobalCount; gElt_I++)
		{
			Element_LocalIndex	localElt;
			Element_ShadowIndex	shadowElt;
			Element_DomainIndex	domainElt;
			
			localElt = decomp->elementMapGlobalToLocal(decomp, gElt_I);
			shadowElt = decomp->elementMapGlobalToShadow(decomp, gElt_I);
			domainElt = decomp->elementMapGlobalToDomain(decomp, gElt_I);
			printf("Element, global: %u - ", gElt_I);
			if (localElt < decomp->elementLocalCount)
				printf("local: %u - ", localElt);
			else
				printf("local: X - ");
			if (shadowElt < decomp->elementShadowCount)
				printf("shadow: %u - ", shadowElt);
			else
				printf("shadow: X - ");
			if (domainElt < decomp->elementDomainCount)
				printf("domain: %u\n", domainElt);
			else
				printf("domain: X\n");
		}
		printf("\n");
		
		for (lElt_I = 0; lElt_I < decomp->elementLocalCount; lElt_I++)
		{
			Element_GlobalIndex	globalElement;
			
			globalElement = decomp->elementMapLocalToGlobal(decomp, lElt_I);
			printf("Element, local: %u - ", lElt_I);
			if (globalElement < decomp->elementGlobalCount)
				printf("global: %u\n", globalElement);
			else
				printf("global: X\n");
		}
		printf("\n");
		
		for (dElt_I = 0; dElt_I < decomp->elementDomainCount; dElt_I++)
		{
			Element_GlobalIndex	globalElement;
			
			globalElement = decomp->elementMapDomainToGlobal(decomp, dElt_I);
			printf("Element, domain: %u - ", dElt_I);
			if (globalElement < decomp->elementGlobalCount)
				printf("global: %u\n", globalElement);
			else
				printf("global: X\n");
		}
		printf("\n");
		
		for (gNode_I = 0; gNode_I < decomp->nodeGlobalCount; gNode_I++)
		{
			Node_LocalIndex		localNode;
			Node_ShadowIndex	shadowNode;
			Node_DomainIndex	domainNode;
			
			localNode = decomp->nodeMapGlobalToLocal(decomp, gNode_I);
			shadowNode = decomp->nodeMapGlobalToShadow(decomp, gNode_I);
			domainNode = decomp->nodeMapGlobalToDomain(decomp, gNode_I);
			printf("Node, global: %u - ", gNode_I);
			if (localNode < decomp->nodeLocalCount)
				printf("local: %u - ", localNode);
			else
				printf("local: X - ");
			if (shadowNode < decomp->nodeShadowCount)
				printf("shadow: %u - ", shadowNode);
			else
				printf("shadow: X - ");
			if (domainNode < decomp->nodeDomainCount)
				printf("domain: %u\n", domainNode);
			else
				printf("domain: X\n");
		}
		printf("\n");
		
		for (lNode_I = 0; lNode_I < decomp->nodeLocalCount; lNode_I++)
		{
			Node_GlobalIndex	globalNode;
			
			globalNode = decomp->nodeMapLocalToGlobal(decomp, lNode_I);
			printf("Node, local: %u - ", lNode_I);
			if (globalNode < decomp->nodeGlobalCount)
				printf("global: %u\n", globalNode);
			else
				printf("global: X\n");
		}
		printf("\n");
		
		for (dNode_I = 0; dNode_I < decomp->nodeDomainCount; dNode_I++)
		{
			Node_GlobalIndex globalNode;
			
			globalNode = decomp->nodeMapDomainToGlobal(decomp, dNode_I);
			printf("Node, domain: %u - ", dNode_I);
			if (globalNode < decomp->nodeGlobalCount)
				printf("global: %u\n", globalNode);
			else
				printf("global: X\n");
		}
		printf("\n");
	}
	else {
		printf( "Rank %u not in use.\n", rank );
	}	
}


Bool CheckDecompItemsAreDecomposedIdentically( HexaMD* decomp1, MeshItemType decomp1ItemType, 
		HexaMD* decomp2, MeshItemType decomp2ItemType, int rank )
{	

	if( rank < decomp1->procsInUse ) {
		int                     ii;
		Partition_Index		proc_I;
		Element_GlobalIndex	gItem_I;
		Dimension_Index         dim_I;
		Index*                  itemGlobal3DCounts[2];
		Element_LocalIJK*       itemLocal3DCounts[2];
		MeshDecomp_Node_MapGlobalToLocalFunction*   itemGlobalToLocalMap[2];
		MeshDecomp_Node_MapGlobalToShadowFunction*  itemGlobalToShadowMap[2];
		MeshDecomp_Node_MapGlobalToDomainFunction*  itemGlobalToDomainMap[2];
		HexaMD*                 decomp;
		MeshItemType            decompItemType;
		GlobalIndex             globalCount;
		
		printf("\tChecking general partitions etc\n" );
		for ( dim_I = 0; dim_I < 3; dim_I++ )
			assert( decomp1->partitionedAxis[dim_I] == decomp2->partitionedAxis[dim_I] );
		for ( dim_I = 0; dim_I < 3; dim_I++ )
			assert( decomp1->partition3DCounts[dim_I] == decomp2->partition3DCounts[dim_I] );

		/* Set up which tables to check betweenm depending on which item types we want to check are equal */
		for ( ii=0; ii < 2; ii++ ) {	
			if ( ii == 0 ) {
				decomp = decomp1;
				decompItemType = decomp1ItemType;
			}
			else {
				decomp = decomp2;
				decompItemType = decomp2ItemType;
			}

			if ( decompItemType == ELEMENT_ITEM_TYPE ) {
				itemGlobal3DCounts[ii] = decomp->elementGlobal3DCounts;
				itemLocal3DCounts[ii] = decomp->elementLocal3DCounts;
				itemGlobalToLocalMap[ii] = decomp->elementMapGlobalToLocal;
				itemGlobalToShadowMap[ii] = decomp->elementMapGlobalToShadow;
				itemGlobalToDomainMap[ii] = decomp->elementMapGlobalToDomain;
			}
			else {	/* NODE_ITEM_TYPE */
				itemGlobal3DCounts[ii] = decomp->nodeGlobal3DCounts;
				itemLocal3DCounts[ii] = decomp->nodeLocal3DCounts;
				itemGlobalToLocalMap[ii] = decomp->nodeMapGlobalToLocal;
				itemGlobalToShadowMap[ii] = decomp->nodeMapGlobalToShadow;
				itemGlobalToDomainMap[ii] = decomp->nodeMapGlobalToDomain;
			}
		}
		
		for ( dim_I = 0; dim_I < 3; dim_I++ )
			assert( itemGlobal3DCounts[0][dim_I] == itemGlobal3DCounts[1][dim_I] );

		for( proc_I = 0; proc_I < decomp1->procsInUse; proc_I++ ) {
			for ( dim_I = 0; dim_I < 3; dim_I++ )
				assert( itemLocal3DCounts[0][proc_I][dim_I] == itemLocal3DCounts[1][proc_I][dim_I] );
		}

		if ( ( decomp1ItemType == ELEMENT_ITEM_TYPE ) && ( decomp2ItemType == ELEMENT_ITEM_TYPE ) ) {
			for ( dim_I = 0; dim_I < 3; dim_I++ )
				assert( decomp1->elementDomain3DCounts[dim_I] == decomp2->elementDomain3DCounts[dim_I] );
		}		

		if ( decomp1ItemType == ELEMENT_ITEM_TYPE ) {
			globalCount = decomp1->elementGlobalCount;
		}
		else {
			globalCount = decomp1->nodeGlobalCount;
		}
		if ( ( decomp1ItemType == ELEMENT_ITEM_TYPE ) && ( decomp2ItemType == NODE_ITEM_TYPE ) ) {
			assert( decomp1->elementGlobalCount == decomp2->nodeGlobalCount );
		}
		if ( ( decomp1ItemType == NODE_ITEM_TYPE ) && ( decomp2ItemType == ELEMENT_ITEM_TYPE ) ) {
			assert( decomp1->nodeGlobalCount == decomp2->elementGlobalCount );
		}

		printf("\tChecking the %d individual items\n", globalCount );
		for (gItem_I = 0; gItem_I < globalCount; gItem_I++)
		{
			LocalIndex	localItem1;
			ShadowIndex	shadowItem1;
			DomainIndex	domainItem1;
			LocalIndex	localItem2;
			ShadowIndex	shadowItem2;
			DomainIndex	domainItem2;
			
			//printf("\tChecking item %d\n", gItem_I );
			localItem1 = (itemGlobalToLocalMap[0])(decomp1, gItem_I);
			shadowItem1 = (itemGlobalToShadowMap[0])(decomp1, gItem_I);
			domainItem1 = (itemGlobalToDomainMap[0])(decomp1, gItem_I);
			localItem2 = (itemGlobalToLocalMap[1])(decomp2, gItem_I);
			shadowItem2 = (itemGlobalToShadowMap[1])(decomp2, gItem_I);
			domainItem2 = (itemGlobalToDomainMap[1])(decomp2, gItem_I);

			assert( localItem1 == localItem2 );
			assert( shadowItem1 == shadowItem2 );
			assert( domainItem1 == domainItem2 );
		}
	}
	else {
		printf( "Rank %u not in use.\n", rank );
	}	

	printf("\tThe decomps matched for the given item types\n" );
	return True;
}


int main(int argc, char *argv[])
{
	MPI_Comm		CommWorld;
	int			rank;
	int			procCount;
	int			procToWatch;
	Dictionary*		dictionary;
	Topology*		nTopology;
	ElementLayout*		eLayout;
	NodeLayout*		nLayout;
	HexaMD*			decompCorner;
	HexaMD*			decompBody;
	Stream*                 stream;
	Index			decompDims;
	XML_IO_Handler*         ioHandler;
	Dimension_Index         dim_I;
	
	/* Initialise MPI, get world info */
	MPI_Init(&argc, &argv);
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size(CommWorld, &procCount);
	MPI_Comm_rank(CommWorld, &rank);

	Base_Init( &argc, &argv );
	
	DiscretisationGeometry_Init( &argc, &argv );
	DiscretisationShape_Init( &argc, &argv );
	DiscretisationMesh_Init( &argc, &argv );
	MPI_Barrier( CommWorld ); /* Ensures copyright info always come first in output */
	
	procToWatch = argc >= 2 ? atoi(argv[1]) : 0;

	Journal_Enable_TypedStream( DebugStream_Type, False );
	stream = Journal_Register( DebugStream_Type, HexaMD_Type );
	Stream_EnableBranch( stream, True );
	Stream_SetLevelBranch( stream, 3 );
	
	dictionary = Dictionary_New();

	Dictionary_Add( dictionary, "rank", Dictionary_Entry_Value_FromUnsignedInt( rank ) );
	Dictionary_Add( dictionary, "numProcessors", Dictionary_Entry_Value_FromUnsignedInt( procCount ) );
	Dictionary_Add( dictionary, "meshSizeI", Dictionary_Entry_Value_FromUnsignedInt( 5 ) );
	Dictionary_Add( dictionary, "meshSizeJ", Dictionary_Entry_Value_FromUnsignedInt( 5 ) );
	Dictionary_Add( dictionary, "meshSizeK", Dictionary_Entry_Value_FromUnsignedInt( 5 ) );
	Dictionary_Add( dictionary, "allowUnusedCPUs", Dictionary_Entry_Value_FromBool( True ) );
	Dictionary_Add( dictionary, "allowPartitionOnElement", Dictionary_Entry_Value_FromBool( False ) );
	Dictionary_Add( dictionary, "allowPartitionOnNode", Dictionary_Entry_Value_FromBool( True ) );
	Dictionary_Add( dictionary, "allowUnbalancing", Dictionary_Entry_Value_FromBool( False ) );
	Dictionary_Add( dictionary, "shadowDepth", Dictionary_Entry_Value_FromUnsignedInt( 1 ) );

	/* Moved afterwards to allow command line to over-ride */
	ioHandler = XML_IO_Handler_New();
	IO_Handler_ReadAllFromCommandLine( ioHandler, argc, argv, dictionary );

	decompDims = Dictionary_GetUnsignedInt_WithDefault( dictionary, "decompDims", 1 );
	
	nTopology = (Topology*)IJK6Topology_New( "IJK6Topology", dictionary );
	eLayout = (ElementLayout*)ParallelPipedHexaEL_New( "PPHexaEL", 3, dictionary );
	nLayout = (NodeLayout*)CornerNL_New( "CornerNL", dictionary, eLayout, nTopology );
	decompCorner = HexaMD_New_All( "HexaMD", dictionary, MPI_COMM_WORLD, eLayout, nLayout, decompDims );

	if( rank == procToWatch ) {
		printf( "Corner Node Layout\n" );
		PrintDecompInfoOfHexaMD( decompCorner, rank );
		printf( "\n" );
	}

	/* Do a run with body nodes */
	Stg_Class_Delete( nLayout );
	/* TODO: the following is a bit of a hack because of the weird way mesh size is defined by default in
	the dictionary (assumes a corner mesh ) */
	for ( dim_I = 0; dim_I < 3; dim_I++ ) {
		if ( ((IJKTopology*)nTopology)->size[dim_I] > 1 ) {
			((IJKTopology*)nTopology)->size[dim_I]--;
		}
	}	
	nLayout = (NodeLayout*)BodyNL_New( "BodyNL", dictionary, eLayout, nTopology );
	decompBody = HexaMD_New_All( "HexaMD", dictionary, MPI_COMM_WORLD, eLayout, nLayout, decompDims );
	if( rank == procToWatch ) {
		Bool    result;

		printf( "Body Node Layout\n" );
		//PrintDecompInfoOfHexaMD( decompBody, rank );
		printf( "Checking body node decomp has same element decomp as corner node decomp:\n" );
		result = CheckDecompItemsAreDecomposedIdentically( decompCorner, ELEMENT_ITEM_TYPE, 
			decompBody, ELEMENT_ITEM_TYPE, rank );
		if ( result == True ) printf( "\tPassed.\n" );
		else printf( "\tFailed.\n" );

		printf( "Checking body node decomp has same node decomp as it's element decomp:\n" );
		result = CheckDecompItemsAreDecomposedIdentically( decompBody, ELEMENT_ITEM_TYPE, 
			decompBody, NODE_ITEM_TYPE, rank );
		if ( result == True ) printf( "\tPassed.\n" );
		else printf( "\tFailed.\n" );
	}

	Stg_Class_Delete( decompBody );
	Stg_Class_Delete( decompCorner );
	Stg_Class_Delete( nLayout );
	Stg_Class_Delete( eLayout );
	Stg_Class_Delete( nTopology );
	Stg_Class_Delete( dictionary );
	
	DiscretisationMesh_Finalise();
	DiscretisationShape_Finalise();
	DiscretisationGeometry_Finalise();
	
	Base_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return EXIT_SUCCESS;
}

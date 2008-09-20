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
** $Id: testTriSurfTopology.c 3664 2006-07-04 04:26:57Z PatrickSunter $
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

int main( int argc, char *argv[] ) {
	MPI_Comm CommWorld;
	int rank;
	int numProcessors;
	int procToWatch;
	
	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size( CommWorld, &numProcessors );
	MPI_Comm_rank( CommWorld, &rank );
	
	Base_Init( &argc, &argv );
	
	DiscretisationGeometry_Init( &argc, &argv );
	DiscretisationShape_Init( &argc, &argv );
	DiscretisationMesh_Init( &argc, &argv );
	MPI_Barrier( CommWorld ); /* Ensures copyright info always come first in output */
	
	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}
	if( rank == procToWatch ) {
		TriSurfTopology*	mt;
		Dictionary*		dictionary;
		XML_IO_Handler*		io_handler = XML_IO_Handler_New();

		Node_GlobalIndex	nodeCnt;
		Node_NeighbourIndex	nodeNbrCnt;
		Node_Neighbours		nodeNbr;
		Node_ElementIndex	nodeEltCnt;
		Node_Elements		nodeElt;
		Element_GlobalIndex	eltCnt;
		Element_NeighbourIndex	elementNbrCnt;
		Element_Neighbours	elementNbr;
		Element_NodeIndex	elementNodeCnt;
		Element_Nodes		elementNode;
		Index			i, j;

		dictionary = Dictionary_New();
		IO_Handler_ReadAllFromFile(io_handler, "data/surface.xml", dictionary);

		mt = TriSurfTopology_New(dictionary, "imElements");
		Print(mt);

		nodeCnt = mt->nodeCount;
		eltCnt = mt->elementCount;

		for (i = 0; i < nodeCnt; i++)
		{
			nodeNbrCnt = mt->nodeNeighbourCount(mt, i);
			printf("Node global: %u, Neighbours: %u", i, nodeNbrCnt);
			if (nodeNbrCnt)
			{
				nodeNbr = Memory_Alloc_Array( Node_GlobalIndex, nodeNbrCnt, "nodeNbr" );
				mt->nodeBuildNeighbours(mt, i, nodeNbr);
				if (nodeNbr[0] < nodeCnt)
					printf(", {%u", nodeNbr[0]);
				else
					printf(", {X");
				for (j = 1; j < nodeNbrCnt; j++)
					if (nodeNbr[j] < nodeCnt)
						printf(", %u", nodeNbr[j]);
					else
						printf(", X");
				printf("}");
				Memory_Free(nodeNbr);
			}
			printf("\n");
		}
		printf("\n");

		for (i = 0; i < nodeCnt; i++)
		{
			nodeEltCnt = mt->nodeElementCount(mt, i);
			printf("Node global: %u, Elements: %u", i, nodeEltCnt);
			if (nodeEltCnt)
			{
				nodeElt = Memory_Alloc_Array( Element_DomainIndex, nodeEltCnt, "nodeElt" );
				mt->nodeBuildElements(mt, i, nodeElt);
				if (nodeElt[0] < eltCnt)
					printf(", {%u", nodeElt[0]);
				else
					printf(", {X");
				for (j = 1; j < nodeEltCnt; j++)
					if (nodeElt[j] < eltCnt)
						printf(", %u", nodeElt[j]);
					else
						printf(", X");
				printf("}");
				Memory_Free(nodeElt);
			}
			printf("\n");
		}
		printf("\n");

		for (i = 0; i < eltCnt; i++)
		{
			elementNbrCnt = mt->elementNeighbourCount(mt, i);
			printf("Element global: %u, Neighbours: %u", i, elementNbrCnt);
			if (elementNbrCnt)
			{
				elementNbr = Memory_Alloc_Array( Node_GlobalIndex, elementNbrCnt, "elementNbr" );
				mt->elementBuildNeighbours(mt, i, elementNbr);
				if (elementNbr[0] < eltCnt)
					printf(", {%u", elementNbr[0]);
				else
					printf(", {X");
				for (j = 1; j < elementNbrCnt; j++)
					if (elementNbr[j] < eltCnt)
						printf(", %u", elementNbr[j]);
					else
						printf(", X");
				printf("}");
				Memory_Free(elementNbr);
			}
			printf("\n");
		}
		printf("\n");

		for (i = 0; i < eltCnt; i++)
		{
			elementNodeCnt = mt->elementNodeCount(mt, i);
			printf("Element global: %u, Nodes: %u", i, elementNodeCnt);
			if (elementNodeCnt)
			{
				elementNode = Memory_Alloc_Array( Element_DomainIndex, elementNodeCnt, "elementNode" );
				mt->elementBuildNodes(mt, i, elementNode);
				if (elementNode[0] < nodeCnt)
					printf(", {%u", elementNode[0]);
				else
					printf(", {X");
				for (j = 1; j < elementNodeCnt; j++)
					if (elementNode[j] < nodeCnt)
						printf(", %u", elementNode[j]);
					else
						printf(", X");
				printf("}");
				Memory_Free(elementNode);
			}
			printf("\n");
		}
		printf("\n");

		Stg_Class_Delete(mt);
		Stg_Class_Delete(dictionary);
	}
	
	DiscretisationMesh_Finalise();
	DiscretisationShape_Finalise();
	DiscretisationGeometry_Finalise();
	
	Base_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0; /* success */
}

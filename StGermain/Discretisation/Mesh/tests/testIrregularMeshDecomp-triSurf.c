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
** $Id: testIrregularMeshDecomp-triSurf.c 3664 2006-07-04 04:26:57Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"
#include "Discretisation/Mesh/Mesh.h"

#include <stdio.h>
#include <mpi.h>


int main(int argc, char *argv[])
{
	int			rank;
	int			procCount;
	int			procToWatch;
	Dictionary*		dictionary;
	MeshTopology*		mt;
	MeshGeometry*		mg;
	MeshDecomp*		md;
	ShadowIndex		sd;
	XML_IO_Handler*		io_handler = XML_IO_Handler_New();
	
	/* Initialise MPI, get world info */
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &procCount);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	Base_Init( &argc, &argv );
	
	DiscretisationGeometry_Init( &argc, &argv );
	DiscretisationShape_Init( &argc, &argv );
	DiscretisationMesh_Init( &argc, &argv );
	MPI_Barrier( CommWorld ); /* Ensures copyright info always come first in output */
	
	procToWatch = argc >= 2 ? atoi(argv[1]) : 0;
	
	dictionary = Dictionary_New();
	Dictionary_Add( dictionary, "rank", Dictionary_Entry_Value_FromUnsignedInt( rank ) );
	Dictionary_Add( dictionary, "numProcessors", Dictionary_Entry_Value_FromUnsignedInt( procCount ) );
	Dictionary_Add( dictionary, "allowUnusedCPUs", Dictionary_Entry_Value_FromBool( True ) );
	Dictionary_Add( dictionary, "allowPartitionOnElement", Dictionary_Entry_Value_FromBool( True ) );
	Dictionary_Add( dictionary, "allowPartitionOnNode", Dictionary_Entry_Value_FromBool( True ) );
	Dictionary_Add( dictionary, "allowUnbalancing", Dictionary_Entry_Value_FromBool( False ) );
	Dictionary_Add( dictionary, "shadowDepth", Dictionary_Entry_Value_FromUnsignedInt( 0 ) );
	IO_Handler_ReadAllFromFile(io_handler, "data/surface.xml", dictionary);
	
	mt = (MeshTopology*)TriSurfTopology_New(dictionary, "imElements");
	mg = (MeshGeometry*)TriSurfGeometry_New(dictionary, "imNodes");
	
	for (sd = 0; sd < 2; sd++)
	{
		Dictionary_Set( dictionary, "shadowDepth", Dictionary_Entry_Value_FromUnsignedInt( sd ) );
	
		md = (MeshDecomp*)IrregularMeshDecomp_New(dictionary, MPI_COMM_WORLD, mt, mg);
					
		if (rank == procToWatch)
		{
			Node_GlobalIndex	gNode_I;
			Node_LocalIndex		lNode_I;
			Node_ShadowIndex	sNode_I;
			Node_DomainIndex	dNode_I;
			Element_GlobalIndex	gElt_I;
			Element_LocalIndex	lElt_I;
			Element_ShadowIndex	sElt_I;
			Element_DomainIndex	dElt_I;
						
			Print(md);
			printf("\n");
			
			for (gNode_I = 0; gNode_I < md->nodeGlobalCount; gNode_I++)
			{
				Node_LocalIndex		localNode;
				Node_ShadowIndex	shadowNode;
				Node_DomainIndex	domainNode;
							
				localNode = md->nodeMapGlobalToLocal(md, gNode_I);
				shadowNode = md->nodeMapGlobalToShadow(md, gNode_I);
				domainNode = md->nodeMapGlobalToDomain(md, gNode_I);
				printf("Node, global: %u - ", gNode_I);
				if (localNode < md->nodeLocalCount)
					printf("local: %u - ", localNode);
				else
					printf("local: X - ");
				if (shadowNode < md->nodeShadowCount)
					printf("shadow: %u - ", shadowNode);
				else
					printf("shadow: X - ");
				if (domainNode < md->nodeDomainCount)
					printf("domain: %u\n", domainNode);
				else
					printf("domain: X\n");
			}
			printf("\n");
						
			for (lNode_I = 0; lNode_I < md->nodeLocalCount; lNode_I++)
			{
				Node_GlobalIndex	globalNode;
				
				globalNode = md->nodeMapLocalToGlobal(md, lNode_I);
				printf("Node, local: %u - ", lNode_I);
				if (globalNode < md->nodeGlobalCount)
					printf("global: %u\n", globalNode);
				else
					printf("global: X\n");
			}
			printf("\n");
					
			for (sNode_I = 0; sNode_I < md->nodeShadowCount; sNode_I++)
			{
				Node_GlobalIndex	globalNode;
							
				globalNode = md->nodeMapShadowToGlobal(md, sNode_I);
				printf("Node, shadow: %u - ", sNode_I);
				if (globalNode < md->nodeGlobalCount)
					printf("global: %u\n", globalNode);
				else
					printf("global: X\n");
			}
			printf("\n");
						
			for (dNode_I = 0; dNode_I < md->nodeLocalCount; dNode_I++)
			{
				Node_GlobalIndex	globalNode;
					
				globalNode = md->nodeMapDomainToGlobal(md, dNode_I);
				printf("Node, domain: %u - ", dNode_I);
				if (globalNode < md->nodeGlobalCount)
					printf("global: %u\n", globalNode);
				else
					printf("global: X\n");
			}
			printf("\n");
						
			for (gElt_I = 0; gElt_I < md->elementGlobalCount; gElt_I++)
			{
				Element_LocalIndex	localElt;
				Element_ShadowIndex	shadowElt;
				Element_DomainIndex	domainElt;
							
				localElt = md->elementMapGlobalToLocal(md, gElt_I);
				shadowElt = md->elementMapGlobalToShadow(md, gElt_I);
				domainElt = md->elementMapGlobalToDomain(md, gElt_I);
				printf("Element, global: %u - ", gElt_I);
				if (localElt < md->elementLocalCount)
					printf("local: %u - ", localElt);
				else
					printf("local: X - ");
				if (shadowElt < md->elementShadowCount)
					printf("shadow: %u - ", shadowElt);
				else
					printf("shadow: X - ");
				if (domainElt < md->elementDomainCount)
					printf("domain: %u\n", domainElt);
				else
					printf("domain: X\n");
			}
			printf("\n");
						
			for (lElt_I = 0; lElt_I < md->elementLocalCount; lElt_I++)
			{
				Element_GlobalIndex	globalElement;
						
				globalElement = md->elementMapLocalToGlobal(md, lElt_I);
				printf("Element, local: %u - ", lElt_I);
				if (globalElement < md->elementGlobalCount)
					printf("global: %u\n", globalElement);
				else
					printf("global: X\n");
			}
			printf("\n");
						
			for (sElt_I = 0; sElt_I < md->elementShadowCount; sElt_I++)
			{
				Element_GlobalIndex	globalElement;
							
				globalElement = md->elementMapShadowToGlobal(md, sElt_I);
				printf("Element, shadow: %u - ", sElt_I);
				if (globalElement < md->elementGlobalCount)
					printf("global: %u\n", globalElement);
				else
					printf("global: X\n");
			}
			printf("\n");
						
			for (dElt_I = 0; dElt_I < md->elementLocalCount; dElt_I++)
			{
				Element_GlobalIndex	globalElement;
					
				globalElement = md->elementMapDomainToGlobal(md, dElt_I);
				printf("Element, domain: %u - ", dElt_I);
				if (globalElement < md->elementGlobalCount)
					printf("global: %u\n", globalElement);
				else
					printf("global: X\n");
			}
			printf("\n");
		}
		
		Stg_Class_Delete(md);
	}
	
	Stg_Class_Delete(mg);
	Stg_Class_Delete(mt);
	
	DiscretisationMesh_Finalise();
	DiscretisationShape_Finalise();
	DiscretisationGeometry_Finalise();
	
	Base_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0; /* success */
}

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
** $Id: MLGen.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "Base/Stg_Class/Stg_Class.h"
#include "Base/IO/IO.h"
#include "Base/Set/Set.h"
#include "Base/Geometry/Geometry.h"
#include "Base/Mesh/Mesh.h"
#include "Base/Utils/Utils.h"
#include "stdio.h"
#include "stdlib.h"
#include "mpi.h"
#include "string.h"


struct _Node {
	struct { __Advectable };
	double temp;
};

struct _Element {
	struct { __Advectable };
	double temp;
};


IJK		ijk = {2, 2, 2};
ShadowIndex	shadowDepth = 0;
Bool		useIrregular = False;
Bool		allowPartitionOnNode = True;
Bool		allowPartitionOnElement = True;
Bool		allowUnbalancing = False;


void dumpGLMesh(int numProcessors, int rank, MeshLayout *ml, FILE *fp)
{
	Node_GlobalIndex	*nltg, *nstg, *ndtg;
	Node_GlobalIndex	*ngtl, *ngts, *ngtd;
	Element_GlobalIndex	*eltg, *estg, *edtg;
	Element_GlobalIndex	*egtl, *egts, *egtd;
	Node_GlobalIndex	ngc;
	Node_LocalIndex		*nlc;
	Node_ShadowIndex	*nsc;
	Node_DomainIndex	*ndc;
	Element_GlobalIndex	egc;
	Element_LocalIndex	*elc;
	Element_ShadowIndex	*esc;
	Element_DomainIndex	*edc;
	Index			i;
	
	ngc = ml->decomp->nodeGlobalCount;
	nlc = Memory_Alloc_Array( Node_LocalIndex, numProcessors, "nlc" );
	nsc = Memory_Alloc_Array( Node_LocalIndex, numProcessors, "nsc" );
	ndc = Memory_Alloc_Array( Node_LocalIndex, numProcessors, "ndc" );
	ngtl = Memory_Alloc_Array( Node_GlobalIndex, ngc*numProcessors, "ngtl" );
	ngts = Memory_Alloc_Array( Node_GlobalIndex, ngc*numProcessors, "ngts" );
	ngtd = Memory_Alloc_Array( Node_GlobalIndex, ngc*numProcessors, "ngtd" );
	nltg = Memory_Alloc_Array( Node_GlobalIndex, ngc*numProcessors, "nltg" );
	nstg = Memory_Alloc_Array( Node_GlobalIndex, ngc*numProcessors, "nstg" );
	ndtg = Memory_Alloc_Array( Node_GlobalIndex, ngc*numProcessors, "ndtg" );
	egc = ml->decomp->elementGlobalCount;
	elc = Memory_Alloc_Array( Element_LocalIndex, numProcessors, "elc" );
	esc = Memory_Alloc_Array( Element_LocalIndex, numProcessors, "esc" );
	edc = Memory_Alloc_Array( Element_LocalIndex, numProcessors, "edc" );
	egtl = Memory_Alloc_Array( Element_GlobalIndex, egc*numProcessors, "egtl" );
	egts = Memory_Alloc_Array( Element_GlobalIndex, egc*numProcessors, "egts" );
	egtd = Memory_Alloc_Array( Element_GlobalIndex, egc*numProcessors, "egtd" );
	eltg = Memory_Alloc_Array( Element_GlobalIndex, egc*numProcessors, "eltg" );
	estg = Memory_Alloc_Array( Element_GlobalIndex, egc*numProcessors, "estg" );
	edtg = Memory_Alloc_Array( Element_GlobalIndex, egc*numProcessors, "edtg" );
	
	nlc[rank] = ml->decomp->nodeLocalCount;
	nsc[rank] = ml->decomp->nodeShadowCount;
	ndc[rank] = ml->decomp->nodeDomainCount;
	for (i = 0; i < ngc; i++)
	{
		ngtl[rank*ngc + i] = ml->decomp->nodeMapGlobalToLocal(ml->decomp, i);
		ngts[rank*ngc + i] = ml->decomp->nodeMapGlobalToShadow(ml->decomp, i);
		ngtd[rank*ngc + i] = ml->decomp->nodeMapGlobalToDomain(ml->decomp, i);
		nltg[rank*ngc + i] = ml->decomp->nodeMapLocalToGlobal(ml->decomp, i);
		nstg[rank*ngc + i] = ml->decomp->nodeMapShadowToGlobal(ml->decomp, i);
		ndtg[rank*ngc + i] = ml->decomp->nodeMapDomainToGlobal(ml->decomp, i);
	}
	elc[rank] = ml->decomp->elementLocalCount;
	esc[rank] = ml->decomp->elementShadowCount;
	edc[rank] = ml->decomp->elementDomainCount;
	for (i = 0; i < egc; i++)
	{
		egtl[rank*egc + i] = ml->decomp->elementMapGlobalToLocal(ml->decomp, i);
		egts[rank*egc + i] = ml->decomp->elementMapGlobalToShadow(ml->decomp, i);
		egtd[rank*egc + i] = ml->decomp->elementMapGlobalToDomain(ml->decomp, i);
		eltg[rank*egc + i] = ml->decomp->elementMapLocalToGlobal(ml->decomp, i);
		estg[rank*egc + i] = ml->decomp->elementMapShadowToGlobal(ml->decomp, i);
		edtg[rank*egc + i] = ml->decomp->elementMapDomainToGlobal(ml->decomp, i);
	}
	
	MPI_Gather(&nlc[rank], 1, MPI_UNSIGNED, nlc, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
	MPI_Gather(&nsc[rank], 1, MPI_UNSIGNED, nsc, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
	MPI_Gather(&ndc[rank], 1, MPI_UNSIGNED, ndc, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
	MPI_Gather(&ngtl[rank*ngc], ngc, MPI_UNSIGNED, ngtl, ngc, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
	MPI_Gather(&ngts[rank*ngc], ngc, MPI_UNSIGNED, ngts, ngc, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
	MPI_Gather(&ngtd[rank*ngc], ngc, MPI_UNSIGNED, ngtd, ngc, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
	MPI_Gather(&nltg[rank*ngc], ngc, MPI_UNSIGNED, nltg, ngc, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
	MPI_Gather(&nstg[rank*ngc], ngc, MPI_UNSIGNED, nstg, ngc, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
	MPI_Gather(&ndtg[rank*ngc], ngc, MPI_UNSIGNED, ndtg, ngc, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
	MPI_Gather(&elc[rank], 1, MPI_UNSIGNED, elc, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
	MPI_Gather(&esc[rank], 1, MPI_UNSIGNED, esc, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
	MPI_Gather(&edc[rank], 1, MPI_UNSIGNED, edc, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
	MPI_Gather(&egtl[rank*egc], egc, MPI_UNSIGNED, egtl, egc, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
	MPI_Gather(&egts[rank*egc], egc, MPI_UNSIGNED, egts, egc, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
	MPI_Gather(&egtd[rank*egc], egc, MPI_UNSIGNED, egtd, egc, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
	MPI_Gather(&eltg[rank*egc], egc, MPI_UNSIGNED, eltg, egc, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
	MPI_Gather(&estg[rank*egc], egc, MPI_UNSIGNED, estg, egc, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
	MPI_Gather(&edtg[rank*egc], egc, MPI_UNSIGNED, edtg, egc, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
	
	/* Dump GLMeshLayout file */
	if (rank == 0)
	{
		Node_GlobalIndex ngi;
		Element_GlobalIndex egi;
		Partition_Index pi;
	
		fprintf(fp, "%u\n", ngc);
		for (ngi = 0; ngi < ngc; ngi++)
		{
			Coord point;

			ml->geometry->nodeAt(ml->geometry, ngi, point);
			fprintf(fp, "%lf %lf %lf\n", point[0], point[1], point[2]);
		}
		
		for (pi = 0; pi < numProcessors; pi++)
		{
			fprintf(fp, "%u %u %u", nlc[pi], nsc[pi], ndc[pi]);
			for (ngi = 0; ngi < ngc; ngi++)
				fprintf(fp, " %u %u %u", ngtl[pi*ngc + ngi], ngts[pi*ngc + ngi], ngtd[pi*ngc + ngi]);
			for (ngi = 0; ngi < nlc[pi]; ngi++)
				fprintf(fp, " %u", nltg[pi*ngc + ngi]);
			for (ngi = 0; ngi < nsc[pi]; ngi++)
				fprintf(fp, " %u", nstg[pi*ngc + ngi]);
			for (ngi = 0; ngi < ndc[pi]; ngi++)
				fprintf(fp, " %u", ndtg[pi*ngc + ngi]);
			fprintf(fp, "\n");
		}
		
		for (ngi = 0; ngi < ngc; ngi++)
		{
			Node_NeighbourIndex nbrCnt = ml->topology->nodeNeighbourCount(ml->topology, ngi);
			Node_Neighbours nbr = Memory_Alloc_Array( Node_DomainIndex, nbrCnt, "nbr" );
			
			ml->topology->nodeBuildNeighbours(ml->topology, ngi, nbr);
			fprintf(fp, "%u", nbrCnt);
			for (i = 0; i < nbrCnt; i++)
				fprintf(fp, " %u", nbr[i]);
			fprintf(fp, "\n");
			
			if (nbr) Memory_Free(nbr);
		}
		
		fprintf(fp, "%u\n", egc);
		
		for (pi = 0; pi < numProcessors; pi++)
		{
			fprintf(fp, "%u %u %u", elc[pi], esc[pi], edc[pi]);
			for (egi = 0; egi < egc; egi++)
				fprintf(fp, " %u %u %u", egtl[pi*egc + egi], egts[pi*egc + egi], egtd[pi*egc + egi]);
			for (egi = 0; egi < elc[pi]; egi++)
				fprintf(fp, " %u", eltg[pi*egc + egi]);
			for (egi = 0; egi < esc[pi]; egi++)
				fprintf(fp, " %u", estg[pi*egc + egi]);
			for (egi = 0; egi < edc[pi]; egi++)
				fprintf(fp, " %u", edtg[pi*egc + egi]);
			fprintf(fp, "\n");
		}
		
		for (egi = 0; egi < egc; egi++)
		{
			Element_NodeIndex nodeCnt = ml->topology->elementNodeCount(ml->topology, egi);
			Element_Nodes nodes = Memory_Alloc_Array( Node_DomainIndex, nodeCnt, "nodes" );
			
			ml->topology->elementBuildNodes(ml->topology, egi, nodes);
			fprintf(fp, "%u", nodeCnt);
			for (i = 0; i < nodeCnt; i++)
				fprintf(fp, " %u", nodes[i]);
			fprintf(fp, "\n");
			
			if (nodes) Memory_Free(nodes);
		}
	}

	if (nlc) Memory_Free(nlc);
	if (nsc) Memory_Free(nsc);
	if (ndc) Memory_Free(ndc);
	if (ngtl) Memory_Free(ngtl);
	if (ngts) Memory_Free(ngts);
	if (ngts) Memory_Free(ngtd);
	if (nltg) Memory_Free(nltg);
	if (nstg) Memory_Free(nstg);
	if (ndtg) Memory_Free(ndtg);
	if (elc) Memory_Free(elc);
	if (esc) Memory_Free(esc);
	if (edc) Memory_Free(edc);
	if (egtl) Memory_Free(egtl);
	if (egts) Memory_Free(egts);
	if (egts) Memory_Free(egtd);
	if (eltg) Memory_Free(eltg);
	if (estg) Memory_Free(estg);
	if (edtg) Memory_Free(edtg);
}


void printHelp(void)
{
	printf("Incorrect arguments given.\n");
	printf("Current format:\n");
	printf("\t-d <isize>,<jsize>,<ksize> : Generate a regular mesh "
		"with given dimensions.\n" );
	printf("\t-s <shadow depth> : specify shadow depth.\n " );
	printf("\t-Pn+ : set allowPartitionOnNode = True.\n " );
	printf("\t-Pn- : set allowPartitionOnNode = False.\n " );
	printf("\t-Pe+ : set allowPartitionOnElement = True.\n " );
	printf("\t-Pe- : set allowPartitionOnElement = False.\n " );
	printf("\t-U+ : set allowUnbalancing = True.\n " );
	printf("\t-U- : set allowUnbalancing = False.\n " );
}


Bool parseCmdLine(int argc, char **argv)
{
	unsigned i;

	for (i = 1; i < argc; i++)
	{
		switch (argv[i][0])
		{
			case '-':
				if (argv[i][1] == 'd' && (i + 1) < argc)
				{
					if (sscanf(argv[++i], "%u,%u,%u", &ijk[0], &ijk[1], &ijk[2]) != EOF &&
					    ijk[0]*ijk[1]*ijk[2] > 0)
						break;
						
					printf("Error: invalid mesh dimensions\n");
					return False;
				}
				else if (argv[i][1] == 's' && (i + 1) < argc)
				{
					if (sscanf(argv[++i], "%u", &shadowDepth) != EOF)
						break;
						
					printf("Error: invalid shadow depth\n");
					return False;
				}
				else if (argv[i][1] == 'i')
				{
					useIrregular = True;
					break;
				}
				else if (!strcmp(&argv[i][1], "Pn+"))
				{
					allowPartitionOnNode = True;
					break;
				}
				else if (!strcmp(&argv[i][1], "Pn-"))
				{
					allowPartitionOnNode = False;
					break;
				}
				else if (!strcmp(&argv[i][1], "Pe+"))
				{
					allowPartitionOnElement = True;
					break;
				}
				else if (!strcmp(&argv[i][1], "Pe-"))
				{
					allowPartitionOnElement = False;
					break;
				}
				else if (!strcmp(&argv[i][1], "U+"))
				{
					allowUnbalancing = True;
					break;
				}
				else if (!strcmp(&argv[i][1], "U-"))
				{
					allowUnbalancing = False;
					break;
				}
			
			default:
				printHelp();
				return False;
		}
	}
	
	return True;
}


int main(int argc, char **argv)
{
	XML_IO_Handler 		*io_handler = XML_IO_Handler_New();
	MPI_Comm		CommWorld;
	int			rank;
	int			numProcessors;
	Dictionary		*dictionary;
	MeshTopology		*mt, *imt;
	MeshGeometry		*mg, *img;
	MeshDecomp		*md, *imd;
	MeshLayout		*rml, *iml;
	FILE			*fp;
	
	/* Initialise MPI, get world info */
	MPI_Init(&argc, &argv);
	MPI_Comm_dup(MPI_COMM_WORLD, &CommWorld);
	MPI_Comm_size(CommWorld, &numProcessors);
	MPI_Comm_rank(CommWorld, &rank);
	
	if (!parseCmdLine(argc, argv))
	{
		MPI_Finalize();
		return 0;
	}
	
	if (!allowPartitionOnNode && !allowPartitionOnElement)
	{
		printf("Error: invalid combination of partition switches\n");
		MPI_Finalize();
		return 0;
	}
	
	/* Read input */
	dictionary = Dictionary_New();
	Dictionary_Add( dictionary, "rank", Dictionary_Entry_Value_FromUnsignedInt( rank ) );
	Dictionary_Add( dictionary, "numProcessors", Dictionary_Entry_Value_FromUnsignedInt( numProcessors ) );

	Dictionary_Add( dictionary, "meshSizeI", Dictionary_Entry_Value_FromUnsignedInt( ijk[0] ) );
	Dictionary_Add( dictionary, "meshSizeJ", Dictionary_Entry_Value_FromUnsignedInt( ijk[1] ) );
	Dictionary_Add( dictionary, "meshSizeK", Dictionary_Entry_Value_FromUnsignedInt( ijk[2] ) );
	Dictionary_Add( dictionary, "allowUnusedCPUs", Dictionary_Entry_Value_FromBool( True ) );
	Dictionary_Add( dictionary, "allowPartitionOnElement", Dictionary_Entry_Value_FromBool( allowPartitionOnElement ) );
	Dictionary_Add( dictionary, "allowPartitionOnNode", Dictionary_Entry_Value_FromBool( allowPartitionOnNode ) );
	Dictionary_Add( dictionary, "allowUnbalancing", Dictionary_Entry_Value_FromBool( allowUnbalancing ) );
	Dictionary_Add( dictionary, "shadowDepth", Dictionary_Entry_Value_FromUnsignedInt( shadowDepth ) );
	IO_Handler_ReadAllFromFile(io_handler, "data/surface.xml", dictionary);

	mt = (MeshTopology *)HexaMeshTopology_New(dictionary);
	mg = (MeshGeometry *)HexaMeshGeometry_New(dictionary);
	md = (MeshDecomp *)RegularMeshDecomp_New(dictionary, MPI_COMM_WORLD, (HexaMeshTopology *)mt);
	rml = MeshLayout_New( mt, mg, md );
	
	if (!rank)
	{
		fp = fopen("ml.1", "wt");
		fprintf(fp, "%u\n", numProcessors);
	}

	dumpGLMesh(numProcessors, rank, rml, fp);
	
	if (useIrregular)
	{
		Element_GlobalIndex	esCnt, *intersect;
		EmbeddedSurface		*es;
		Mesh*			mesh;
		ExtensionManager_Register*	extensionMgr_Register;
		Index			i;
		
		if (!rank) fprintf(fp, "1\n");
		imt = (MeshTopology *)TriSurfTopology_New(dictionary, "imElements");
		img = (MeshGeometry *)TriSurfGeometry_New(dictionary, "imNodes");
		imd = (MeshDecomp *)IrregularMeshDecomp_New_FromMeshLayout(dictionary, MPI_COMM_WORLD, imt, img, rml);
		iml = MeshLayout_New( imt, img, imd );
		dumpGLMesh(numProcessors, rank, iml, fp);
		
		extensionMgr_Register = ExtensionManager_Register_New( );
		mesh = Mesh_New( iml, sizeof(Node), sizeof(Element), extensionMgr_Register, dictionary );
		Build( mesh, 0, False );
		Initialise( mesh, 0, False );
		
		if (!rank)
		{
			es = EmbeddedSurface_New(mesh);

			intersect = Memory_Alloc_Array( Element_GlobalIndex, ES_IntersectionMax(es), "intersect" );
			esCnt = EmbeddedSurface_BuildIntersection(es, intersect);
			fprintf(fp, "%u\n", esCnt);
			for (i = 0; i < esCnt; i++)
				fprintf(fp, "%u ", intersect[i]);
			if (intersect) Memory_Free(intersect);
		}
		
		Stg_Class_Delete(es);
		Stg_Class_Delete(mesh);
		Stg_Class_Delete(iml);
		Stg_Class_Delete(imd);
		Stg_Class_Delete(img);
		Stg_Class_Delete(imt);
	}
	else
		if (!rank) fprintf(fp, "0\n");
		
	if (!rank)
		fclose(fp);

	/* Destroy stuff */
	Stg_Class_Delete(rml);
	Stg_Class_Delete(md);
	Stg_Class_Delete(mg);
	Stg_Class_Delete(mt);
	Stg_Class_Delete( dictionary );

	/* Close off MPI */
	MPI_Finalize();

	return 0; /* success */
}

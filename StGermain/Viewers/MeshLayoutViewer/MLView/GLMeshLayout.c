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
** $Id: GLMeshLayout.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gl.h>
#include "GLMeshLayout.h"


void loadGLMesh(FILE *fp, unsigned procCnt, GLMesh *mesh)
{
	unsigned i;

	fscanf(fp, "%u", &mesh->nodeCnt);
	mesh->node = Memory_Alloc_Array( GLdouble, mesh->nodeCnt*3, "mesh->node");
	for (i = 0; i < mesh->nodeCnt; i++)
		fscanf(fp, "%lf %lf %lf", &mesh->node[i*3], &mesh->node[i*3 + 1], &mesh->node[i*3 + 2]);
		
	mesh->nodeLocalCnt = Memory_Alloc_Array( unsigned, procCnt, "mesh->nodeLocalCnt" );
	mesh->nodeShadowCnt = Memory_Alloc_Array( unsigned, procCnt, "mesh->nodeShadowCnt" );
	mesh->nodeDomainCnt = Memory_Alloc_Array( unsigned, procCnt, "mesh->nodeDomainCnt" );
	mesh->nodeGlobalToLocal = Memory_Alloc_2DArray( unsigned, procCnt, mesh->nodeCnt, "mesh->nodeGlobalToLocal" );
	mesh->nodeGlobalToShadow = Memory_Alloc_2DArray( unsigned, procCnt, mesh->nodeCnt, "mesh->nodeGlobalToDomain" );
	mesh->nodeGlobalToDomain = Memory_Alloc_2DArray( unsigned, procCnt, mesh->nodeCnt, "mesh->nodeGlobalToDomain" );
	mesh->nodeLocalToGlobal = Memory_Alloc_Array( unsigned*, procCnt, "mesh->nodeLocalToGlobal" );
	mesh->nodeShadowToGlobal = Memory_Alloc_Array( unsigned*, procCnt, "mesh->nodeShadowToGlobal" );
	mesh->nodeDomainToGlobal = Memory_Alloc_Array( unsigned *, procCnt, "mesh->nodeDomainToGlobal" );
	for (i = 0; i < procCnt; i++)
	{
		unsigned j;
		
		fscanf(fp, "%u", &mesh->nodeLocalCnt[i]);
		fscanf(fp, "%u", &mesh->nodeShadowCnt[i]);
		fscanf(fp, "%u", &mesh->nodeDomainCnt[i]);

		mesh->nodeLocalToGlobal[i] = Memory_Alloc_Array( unsigned, mesh->nodeLocalCnt[i], "mesh->nodeLocalToGlobal[]" );
		mesh->nodeShadowToGlobal[i] = Memory_Alloc_Array( unsigned, mesh->nodeShadowCnt[i], "mesh->nodeShadowToGlobal[]" );
		mesh->nodeDomainToGlobal[i] = Memory_Alloc_Array( unsigned, mesh->nodeDomainCnt[i], "mesh->nodeDomainToGlobal[]" );
		
		for (j = 0; j < mesh->nodeCnt; j++)
		{
			fscanf(fp, "%u", &mesh->nodeGlobalToLocal[i][j]);
			fscanf(fp, "%u", &mesh->nodeGlobalToShadow[i][j]);
			fscanf(fp, "%u", &mesh->nodeGlobalToDomain[i][j]);
		}
		for (j = 0; j < mesh->nodeLocalCnt[i]; j++)
			fscanf(fp, "%u", &mesh->nodeLocalToGlobal[i][j]);
		for (j = 0; j < mesh->nodeShadowCnt[i]; j++)
			fscanf(fp, "%u", &mesh->nodeShadowToGlobal[i][j]);
		for (j = 0; j < mesh->nodeDomainCnt[i]; j++)
			fscanf(fp, "%u", &mesh->nodeDomainToGlobal[i][j]);
	}
	
	mesh->nodeNeighbourCnt = Memory_Alloc_Array( unsigned, mesh->nodeCnt, "mesh->nodeNeighbourCnt" );
	mesh->nodeNeighbour = Memory_Alloc_Array( unsigned*, mesh->nodeCnt, "mesh->nodeNeighbour" );
	for (i = 0; i < mesh->nodeCnt; i++)
	{
		unsigned j;
	
		fscanf(fp, "%u", &mesh->nodeNeighbourCnt[i]);
		mesh->nodeNeighbour[i] = Memory_Alloc_Array( unsigned, mesh->nodeNeighbourCnt[i], "mesh->nodeNeighbour[]" );
		for (j = 0; j < mesh->nodeNeighbourCnt[i]; j++)
			fscanf(fp, "%u", &mesh->nodeNeighbour[i][j]);
	}
		
	mesh->edgeCnt = 0;
	mesh->edge = NULL;
	for (i = 0; i < mesh->nodeCnt; i++)
	{
		unsigned j;
		
		for (j = 0; j < mesh->nodeNeighbourCnt[i]; j++)
		{
			unsigned k;
				
			if (mesh->nodeNeighbour[i][j] >= mesh->nodeCnt) continue;
				
			for (k = 0; k < mesh->edgeCnt; k++)
				if ((mesh->edge[k*2] == mesh->nodeNeighbour[i][j] && mesh->edge[k*2 + 1] == i) ||
				    (mesh->edge[k*2] == i && mesh->edge[k*2 + 1] == mesh->nodeNeighbour[i][j]))
				{
					break;
				}
			if (k < mesh->edgeCnt) continue;
			
			mesh->edge = Memory_Alloc_Array(mesh->edge, unsigned, (mesh->edgeCnt + 1)*2 );
			mesh->edge[mesh->edgeCnt*2] = i;
			mesh->edge[mesh->edgeCnt*2 + 1] = mesh->nodeNeighbour[i][j];
			mesh->edgeCnt++;
		}
	}
	
	fscanf(fp, "%u", &mesh->elementCnt);
	mesh->elementLocalCnt = Memory_Alloc_Array( unsigned, procCnt, "mesh->elementLocalCnt" );
	mesh->elementShadowCnt = Memory_Alloc_Array( unsigned, procCnt, "mesh->elementShadowCnt" );
	mesh->elementDomainCnt = Memory_Alloc_Array( unsigned, procCnt, "mesh->elementDomainCnt" );
	mesh->elementGlobalToLocal = Memory_Alloc_2DArray( unsigned, procCnt, mesh->elementCnt, "mesh->elementGlobalToLocal" );
	mesh->elementGlobalToShadow = Memory_Alloc_2DArray( unsigned, procCnt, mesh->elementCnt, "mesh->elementGlobalToShadow" );
	mesh->elementGlobalToDomain = Memory_Alloc_2DArray(unsigned, procCnt, mesh->elementCnt, "mesh->elementGlobalToDomain" );
	mesh->elementLocalToGlobal = Memory_Alloc_Array( unsigned*, procCnt, "mesh->elementLocalToGlobal" );
	mesh->elementShadowToGlobal = Memory_Alloc_Array( unsigned*, procCnt, "mesh->elementShadowToGlobal" );
	mesh->elementDomainToGlobal = Memory_Alloc_Array( unsigned*, procCnt, "mesh->elementDomainToGlobal" );
	for (i = 0; i < procCnt; i++)
	{
		unsigned j;
		
		fscanf(fp, "%u", &mesh->elementLocalCnt[i]);
		fscanf(fp, "%u", &mesh->elementShadowCnt[i]);
		fscanf(fp, "%u", &mesh->elementDomainCnt[i]);

		mesh->elementLocalToGlobal[i] = Memory_Alloc_Array( unsigned*, mesh->elementLocalCnt[i],
			"mesh->elementLocalToGlobal[]" );
		mesh->elementShadowToGlobal[i] = Memory_Alloc_Array( unsigned*, mesh->elementShadowCnt[i],
			"mesh->elementShadowToGlobal[]" );
		mesh->elementDomainToGlobal[i] = Memory_Alloc_Array( unsigned*, mesh->elementDomainCnt[i],
			"mesh->elementDomainToGlobal[]" );
		
		for (j = 0; j < mesh->elementCnt; j++)
		{
			fscanf(fp, "%u", &mesh->elementGlobalToLocal[i][j]);
			fscanf(fp, "%u", &mesh->elementGlobalToShadow[i][j]);
			fscanf(fp, "%u", &mesh->elementGlobalToDomain[i][j]);
		}
		for (j = 0; j < mesh->elementLocalCnt[i]; j++)
			fscanf(fp, "%u", &mesh->elementLocalToGlobal[i][j]);
		for (j = 0; j < mesh->elementShadowCnt[i]; j++)
			fscanf(fp, "%u", &mesh->elementShadowToGlobal[i][j]);
		foy (j = 0; j < mesh->elementDomainCnt[i]; j++)
			fscanf(fp, "%u", &mesh->elementDomainToGlobal[i][j]);
	}
	
	mesh->elementNodeCnt = Memory_Alloc_Array( unsigned, mesh->elementCnt, "mesh->elementNodeCnt" );
	mesh->elementEdgeCnt = Memory_Alloc_Array( unsigned, mesh->elementCnt, "mesh->elementEdgeCnt" );
	mesh->elementNode = Memory_Alloc_Array( unsigned*, mesh->elementCnt, "mesh->elementNode" );
	mesh->elementEdge = Memory_Alloc_Array( unsigned*, mesh->elementCnt, "mesh->elementEdge" );
	for (i = 0; i < mesh->elementCnt; i++)
	{
		unsigned j;
	
		fscanf(fp, "%u", &mesh->elementNodeCnt[i]);
		mesh->elementNode[i] = Memory_Alloc_Array( unsigned, mesh->elementNodeCnt[i], "mesh->elementNode[]" );
		for (j = 0; j < mesh->elementNodeCnt[i]; j++)
			fscanf(fp, "%u", &mesh->elementNode[i][j]);
		
		mesh->elementEdgeCnt[i] = 0;
		mesh->elementEdge[i] = NULL;
		for (j = 0; j < mesh->edgeCnt; j++)
		{
			unsigned k, valid = 0;
			
			for (k = 0; k < mesh->elementNodeCnt[i]; k++)
			{
				if (mesh->edge[j*2] == mesh->elementNode[i][k] || mesh->edge[j*2 + 1] == mesh->elementNode[i][k])
				{
					if (++valid == 2)
					{
						mesh->elementEdge[i] = Memory_Realloc_Array( mesh->elementEdge[i], unsigned,
							(mesh->elementEdgeCnt[i] + 1) );
						mesh->elementEdge[i][mesh->elementEdgeCnt[i]] = j;
						mesh->elementEdgeCnt[i]++;
					}
				}
			}
		}
	}
}


void loadGLMeshLayout(char *filename, GLMeshLayout *glm)
{
	FILE *fp = fopen(filename, "rt");
	
	if (!fp)
	{
		printf("Error: couldn't locate %s\n", filename);
		memset(glm, 0, sizeof(GLMeshLayout));
		return;
	}
	
	fscanf(fp, "%u", &glm->procCnt);
	loadGLMesh(fp, glm->procCnt, &glm->baseMesh);
	glm->baseMesh.intersectCnt = 0;
	glm->baseMesh.intersect = NULL;

	fscanf(fp, "%u", &glm->childMeshCnt);
	if (glm->childMeshCnt)
	{
		unsigned i, j;

		glm->childMesh = Memory_Alloc_Array( GLMesh, glm->childMeshCnt, "glm->childMesh" );
		for (i = 0; i < glm->childMeshCnt; i++)
		{
			loadGLMesh(fp, glm->procCnt, &glm->childMesh[i]);
			
			fscanf(fp, "%u", &glm->childMesh[i].intersectCnt);
			glm->childMesh[i].intersect = Memory_Alloc_Array( unsigned,glm->childMesh[i].intersectCnt,
				"glm->childMesh[]" );
			for (j = 0; j < glm->childMesh[i].intersectCnt; j++)
				fscanf(fp, "%u", &glm->childMesh[i].intersect[j]);
		}
			
	}
	else
		glm->childMesh = NULL;

	fclose(fp);
}


void unloadGLMesh(GLMesh *mesh, unsigned procCnt)
{
	unsigned i;

	for (i = 0; i < procCnt; i++)
	{
		if (mesh->nodeGlobalToLocal[i]) Memory_Free(mesh->nodeGlobalToLocal[i]);
		if (mesh->nodeGlobalToShadow[i]) Memory_Free(mesh->nodeGlobalToShadow[i]);
		if (mesh->nodeGlobalToDomain[i]) Memory_Free(mesh->nodeGlobalToDomain[i]);
		if (mesh->nodeLocalToGlobal[i]) Memory_Free(mesh->nodeLocalToGlobal[i]);
		if (mesh->nodeShadowToGlobal[i]) Memory_Free(mesh->nodeShadowToGlobal[i]);
		if (mesh->nodeDomainToGlobal[i]) Memory_Free(mesh->nodeDomainToGlobal[i]);
		if (mesh->elementGlobalToLocal[i]) Memory_Free(mesh->elementGlobalToLocal[i]);
		if (mesh->elementGlobalToShadow[i]) Memory_Free(mesh->elementGlobalToShadow[i]);
		if (mesh->elementGlobalToDomain[i]) Memory_Free(mesh->elementGlobalToDomain[i]);
		if (mesh->elementLocalToGlobal[i]) Memory_Free(mesh->elementLocalToGlobal[i]);
		if (mesh->elementShadowToGlobal[i]) Memory_Free(mesh->elementShadowToGlobal[i]);
		if (mesh->elementDomainToGlobal[i]) Memory_Free(mesh->elementDomainToGlobal[i]);
	}
	for (i = 0; i < mesh->nodeCnt; i++)
	{
		if (mesh->nodeNeighbour[i]) Memory_Free(mesh->nodeNeighbour[i]);
	}
	for (i = 0; i < mesh->elementCnt; i++)
	{
		if (mesh->elementNode[i]) Memory_Free(mesh->elementNode[i]);
		if (mesh->elementEdge[i]) Memory_Free(mesh->elementEdge[i]);
	}
	if (mesh->nodeLocalCnt) Memory_Free(mesh->nodeLocalCnt);
	if (mesh->nodeShadowCnt) Memory_Free(mesh->nodeShadowCnt);
	if (mesh->nodeDomainCnt) Memory_Free(mesh->nodeDomainCnt);
	if (mesh->nodeGlobalToLocal) Memory_Free(mesh->nodeGlobalToLocal);
	if (mesh->nodeGlobalToShadow) Memory_Free(mesh->nodeGlobalToShadow);
	if (mesh->nodeGlobalToDomain) Memory_Free(mesh->nodeGlobalToDomain);
	if (mesh->nodeLocalToGlobal) Memory_Free(mesh->nodeLocalToGlobal);
	if (mesh->nodeShadowToGlobal) Memory_Free(mesh->nodeShadowToGlobal);
	if (mesh->nodeDomainToGlobal) Memory_Free(mesh->nodeDomainToGlobal);
	if (mesh->nodeNeighbourCnt) Memory_Free(mesh->nodeNeighbourCnt);
	if (mesh->nodeNeighbour) Memory_Free(mesh->nodeNeighbour);
	if (mesh->node) Memory_Free(mesh->node);
	if (mesh->elementLocalCnt) Memory_Free(mesh->elementLocalCnt);
	if (mesh->elementShadowCnt) Memory_Free(mesh->elementShadowCnt);
	if (mesh->elementDomainCnt) Memory_Free(mesh->elementDomainCnt);
	if (mesh->elementGlobalToLocal) Memory_Free(mesh->elementGlobalToLocal);
	if (mesh->elementGlobalToShadow) Memory_Free(mesh->elementGlobalToShadow);
	if (mesh->elementGlobalToDomain) Memory_Free(mesh->elementGlobalToDomain);
	if (mesh->elementLocalToGlobal) Memory_Free(mesh->elementLocalToGlobal);
	if (mesh->elementShadowToGlobal) Memory_Free(mesh->elementShadowToGlobal);
	if (mesh->elementDomainToGlobal) Memory_Free(mesh->elementDomainToGlobal);
	if (mesh->elementNodeCnt) Memory_Free(mesh->elementNodeCnt);
	if (mesh->elementNode) Memory_Free(mesh->elementNode);
	if (mesh->elementEdgeCnt) Memory_Free(mesh->elementEdgeCnt);
	if (mesh->elementEdge) Memory_Free(mesh->elementEdge);
	if (mesh->edge) Memory_Free(mesh->edge);
	if (mesh->intersect) Memory_Free(mesh->intersect);
}


void unloadGLMeshLayout(GLMeshLayout *glm)
{
	unsigned i;

	unloadGLMesh(&glm->baseMesh, glm->procCnt);
	for (i = 0; i < glm->childMeshCnt; i++)
		unloadGLMesh(&glm->childMesh[i], glm->procCnt);
}

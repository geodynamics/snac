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
** $Id: GLMeshLayout.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __GLMeshLayout_h__
#define __GLMeshLayout_h__


struct _GLMesh
{
	unsigned	nodeCnt;
	unsigned	*nodeLocalCnt;
	unsigned	*nodeShadowCnt;
	unsigned	*nodeDomainCnt;

	unsigned	**nodeGlobalToLocal;
	unsigned	**nodeGlobalToShadow;
	unsigned	**nodeGlobalToDomain;
	unsigned	**nodeLocalToGlobal;
	unsigned	**nodeShadowToGlobal;
	unsigned	**nodeDomainToGlobal;

	unsigned	*nodeNeighbourCnt;
	unsigned	**nodeNeighbour;
	
	GLdouble	*node;

	unsigned	elementCnt;
	unsigned	*elementLocalCnt;
	unsigned	*elementShadowCnt;
	unsigned	*elementDomainCnt;

	unsigned	**elementGlobalToLocal;
	unsigned	**elementGlobalToShadow;
	unsigned	**elementGlobalToDomain;
	unsigned	**elementLocalToGlobal;
	unsigned	**elementShadowToGlobal;
	unsigned	**elementDomainToGlobal;
	
	unsigned	*elementNodeCnt;
	unsigned	**elementNode;
	unsigned	*elementEdgeCnt;
	unsigned	**elementEdge;

	unsigned	edgeCnt;
	unsigned	*edge;
	
	unsigned	intersectCnt;
	unsigned	*intersect;
};
typedef struct _GLMesh GLMesh;


struct _GLMeshLayout
{
	unsigned	procCnt;
	GLMesh		baseMesh;
	unsigned	childMeshCnt;
	GLMesh		*childMesh;
};
typedef struct _GLMeshLayout GLMeshLayout;


void loadGLMeshLayout(char *filename, GLMeshLayout *glm);
void unloadGLMeshLayout(GLMeshLayout *glm);


#endif

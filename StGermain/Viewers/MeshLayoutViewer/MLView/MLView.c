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
** $Id: MLView.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <glut.h>
#include "GLMeshLayout.h"


enum _MeshScope
{
	localScope,
	shadowScope,
	domainScope
};
typedef enum _MeshScope MeshScope;


struct _GLMeshInstance
{
	unsigned char	*nodeIndex;
	unsigned char	*edgeIndex;
	unsigned	proc;
	MeshScope	scope;
	
	GLMesh		*mesh;
};
typedef struct _GLMeshInstance GLMeshInstance;


GLMeshLayout	meshLayout;
GLMeshInstance	*instance;

unsigned	instCnt = 0;
unsigned	curInst = 0;
GLdouble	oldx, oldy;
GLdouble	xrot = 0.0, yrot = 0.0;

GLuint		fontBase;
int 		scrWidth, scrHeight;


void glPrintf(char *fmt, ...)
{
	va_list args;
	char str[255];
	
	va_start(args, fmt);
	vsprintf(str, fmt, args);
	va_end(args);

	glListBase(fontBase);
	glCallLists(strlen(str), GL_UNSIGNED_BYTE, str);
}


void initGL(void)
{
//	GLfloat pda[3] = {0.0, 0.3, 0.0};
	int i;

	glClearColor(0.3, 0.3, 0.3, 1.0);
	glEnable(GL_CULL_FACE);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE);
	
	glPointSize(5.0);
//	glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, pda);

	fontBase = glGenLists(127);
	for (i = 0; i < 127; i++)
	{
		glNewList(fontBase + i, GL_COMPILE);
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, i);
		glEndList();
	}
}


void displayHandler(void)
{
	unsigned i, inst;
	unsigned cnt, *edge;
	GLdouble *node;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslated(0.0, 0.0, -2.5);
	glRotated(xrot, 0.0, 1.0, 0.0);
	glRotated(yrot, 1.0, 0.0, 0.0);
	glTranslated(-0.5, -0.5, -0.5);
	
	for (inst = 0; inst < instCnt; inst++)
	{
		node = instance[inst].mesh->node;
		edge = instance[inst].mesh->edge;

		cnt = instance[inst].mesh->nodeCnt;
		for (i = 0; i < cnt; i++)
		{
			if (instance[inst].nodeIndex[i])
			{
				glColor3f(1.0, 1.0, 1.0);
				glDisable(GL_BLEND);
			}
			else
			{
				glColor3f(0.2, 0.2, 0.2);
				glEnable(GL_BLEND);
			}

			glBegin(GL_POINTS);
			glVertex3d(node[i*3], node[i*3 + 1], node[i*3 + 2]);
			glEnd();
			
			glRasterPos3d(node[i*3] - 0.04, node[i*3 + 1] + 0.01, node[i*3 + 2] + 0.01);
			glPrintf("%03d", i);
		}
		
		cnt = instance[inst].mesh->edgeCnt;
		for (i = 0; i < cnt; i++)
		{
			if (instance[inst].edgeIndex[i])
			{
				glColor3f(0.0, 0.0, 1.0);
				glDisable(GL_BLEND);
			}
			else
			{
				glColor3f(0.2, 0.2, 0.2);
				glEnable(GL_BLEND);
			}

			glBegin(GL_LINES);
			glVertex3d(node[edge[i*2]*3], node[edge[i*2]*3 + 1], node[edge[i*2]*3 + 2]);
			glVertex3d(node[edge[i*2 + 1]*3], node[edge[i*2 + 1]*3 + 1], node[edge[i*2 + 1]*3 + 2]);
			glEnd();
		}
	}
	
	/* Display all text stuff */
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, scrWidth, 0, scrHeight);

	glColor3f(1.0, 1.0, 1.0);
	glRasterPos2i(10, scrHeight - 22);
	glPrintf("Mesh: ");
	switch (curInst)
	{
		case 0:
			glPrintf("base");
			break;
			
		default:
			glPrintf("child %u", curInst);
			break;
	}
	
	glRasterPos2i(10, scrHeight - 34);
	glPrintf("Scope: ");
	switch (instance[curInst].scope)
	{
		case (localScope):
			glPrintf("local");
			break;
		
		case (shadowScope):
			glPrintf("shadow");
			break;
		
		case (domainScope):
			glPrintf("domain");
			break;
	}
	
	glRasterPos2i(10, scrHeight - 46);
	glPrintf("Processor: ");
	if (instance[curInst].proc == meshLayout.procCnt)
		glPrintf("-");
	else
		glPrintf("%u", instance[curInst].proc);
	
	glPopMatrix();

	glutSwapBuffers();
}


void reshapeHandler(int width, int height)
{
	scrWidth = width;
	scrHeight = height;

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (GLdouble)width/(GLdouble)height, 0.5, 30.0);
}


void keyboardHandler(unsigned char key, int x, int y)
{
	int redraw = 0;
	unsigned i, j;

	switch (key)
	{
		case 'i':
			if (meshLayout.childMeshCnt)
			{
				for (i = 0; i < meshLayout.childMesh[0].intersectCnt; i++)
				{
					for (j = 0; j < meshLayout.baseMesh.elementNodeCnt[meshLayout.childMesh[0].intersect[i]]; j++)
					{
						instance[0].nodeIndex[meshLayout.baseMesh.elementNode[meshLayout.childMesh[0].intersect[i]][j]] = 1;
					}
					
					for (j = 0; j < meshLayout.baseMesh.elementEdgeCnt[meshLayout.childMesh[0].intersect[i]]; j++)
					{
						instance[0].edgeIndex[meshLayout.baseMesh.elementEdge[meshLayout.childMesh[0].intersect[i]][j]] = 1;
					}
				}
			}
			glutPostRedisplay();
			break;
			
		case ')':
		case '0':
			curInst = (curInst + 1)%(meshLayout.childMeshCnt + 1);
			glutPostRedisplay();
			break;
			
		case '(':
		case '9':
			curInst = curInst == 0 ? meshLayout.childMeshCnt : curInst - 1;
			glutPostRedisplay();
			break;

		case '}':
		case ']':
			instance[curInst].scope = instance[curInst].scope == localScope ? shadowScope :
						  instance[curInst].scope == shadowScope ? domainScope :
						  localScope;
			redraw = 1;
			break;
			
		case '{':
		case '[':
			instance[curInst].scope = instance[curInst].scope == localScope ? domainScope :
						  instance[curInst].scope == shadowScope ? localScope :
						  shadowScope;
			redraw = 1;
			break;
		
		case '>':
		case '.':
			instance[curInst].proc = (instance[curInst].proc + 1)%(meshLayout.procCnt + 1);
			redraw = 1;
			break;
			
		case '<':
		case ',':
			instance[curInst].proc = instance[curInst].proc == 0 ? meshLayout.procCnt : instance[curInst].proc - 1;
			redraw = 1;
			break;
	}
	
	if (redraw)
	{
		memset(instance[curInst].nodeIndex, 0, sizeof(unsigned char)*instance[curInst].mesh->nodeCnt);
		memset(instance[curInst].edgeIndex, 0, sizeof(unsigned char)*instance[curInst].mesh->edgeCnt);
	
		if (instance[curInst].proc < meshLayout.procCnt)
		{
			unsigned nodeCnt, elementCnt, *nodeMap, *elementMap;
			unsigned i;
			
			switch (instance[curInst].scope)
			{
				case 0:
					nodeCnt = instance[curInst].mesh->nodeLocalCnt[instance[curInst].proc];
					nodeMap = instance[curInst].mesh->nodeLocalToGlobal[instance[curInst].proc];
					elementCnt = instance[curInst].mesh->elementLocalCnt[instance[curInst].proc];
					elementMap = instance[curInst].mesh->elementLocalToGlobal[instance[curInst].proc];
					break;
					
				case 1:
					nodeCnt = instance[curInst].mesh->nodeShadowCnt[instance[curInst].proc];
					nodeMap = instance[curInst].mesh->nodeShadowToGlobal[instance[curInst].proc];
					elementCnt = instance[curInst].mesh->elementShadowCnt[instance[curInst].proc];
					elementMap = instance[curInst].mesh->elementShadowToGlobal[instance[curInst].proc];
					break;
			
				case 2:
					nodeCnt = instance[curInst].mesh->nodeDomainCnt[instance[curInst].proc];
					nodeMap = instance[curInst].mesh->nodeDomainToGlobal[instance[curInst].proc];
					elementCnt = instance[curInst].mesh->elementDomainCnt[instance[curInst].proc];
					elementMap = instance[curInst].mesh->elementDomainToGlobal[instance[curInst].proc];
					break;
				break;
			}
			
			for (i = 0; i < nodeCnt; i++)
				instance[curInst].nodeIndex[nodeMap[i]] = 1;
			for (i = 0; i < elementCnt; i++)
			{
				unsigned j;
		
				for (j = 0; j < instance[curInst].mesh->elementEdgeCnt[elementMap[i]]; j++)
					instance[curInst].edgeIndex[instance[curInst].mesh->elementEdge[elementMap[i]][j]] = 1;
			}
		}
		
		glutPostRedisplay();
	}
}


void specialHandler(int key, int x, int y)
{
}


void mouseHandler(int button, int state, int x, int y)
{
	oldx = x;
	oldy = y;
}


void motionHandler(int x, int y)
{
	int dx = x - oldx, dy = y - oldy;
		
	xrot += dx;
	yrot += dy;
	oldx = x;
	oldy = y;
	
	glutPostRedisplay();
}


int main(int argc, char **argv)
{
	int glutWin;
	unsigned i;
	
	/* Init GLUT */
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutWin = glutCreateWindow("MeshLayout Viewer");

	glutDisplayFunc(displayHandler);
	glutReshapeFunc(reshapeHandler);
	glutKeyboardFunc(keyboardHandler);
//	glutSpecialFunc(specialHandler);
	glutMouseFunc(mouseHandler);
	glutMotionFunc(motionHandler);
	
	/* Init OpenGL */
	initGL();
	
	/* Load mesh layout */
	if (argc > 1)
	{
		loadGLMeshLayout(argv[1], &meshLayout);
		
		instCnt = meshLayout.childMeshCnt + 1;
		instancy = Memory_Alloc_Array( GLMeshInstance, instCnt, "instance" );
		instance[0].nodeIndex = Memory_Alloc_Array( unsigned char, meshLayout.baseMesh.nodeCnt, "instance[0].nodeIndex" );
		memset(instance[0].nodeIndex, 0, sizeof(unsigned char)*meshLayout.baseMesh.nodeCnt);
		instance[0].edgeIndex = Memory_Alloc_Array( unsigned char, meshLayout.baseMesh.edgeCnt, "instance[0].edgeIndex" );
		memset(instance[0].edgeIndex, 0, sizeof(unsigned char)*meshLayout.baseMesh.edgeCnt);
		instance[0].proc = meshLayout.procCnt;
		instance[0].scope = localScope;
		for (i = 1; i < instCnt; i++)
		{
			instance[i].nodeIndex = Memory_Alloc_Array( unsigned char, meshLayout.childMesh[i - 1].nodeCnt,
				"instance[i].nodeIndex" );
			memset(instance[i].nodeIndex, 0, sizeof(unsigned char)*meshLayout.childMesh[i - 1].nodeCnt);
			instance[i].edgeIndex = Memory_Alloc_Array( unsigned char, meshLayout.childMesh[i - 1].edgeCnt,
				"instance[i].edgeIndex" );
			memset(instance[i].edgeIndex, 0, sizeof(unsigned char)*meshLayout.childMesh[i - 1].edgeCnt);
			instance[i].proc = meshLayout.procCnt;
			instance[i].scope = localScope;
		}
		instance[0].mesh = &meshLayout.baseMesh;
		for (i = 0; i < meshLayout.childMeshCnt; i++)
			instance[i + 1].mesh = &meshLayout.childMesh[i];
	}
	else
	{
		printf("Error: no GLMeshLayout file specified\n");
		glutDestroyWindow(glutWin);
		return 0;
	}

	glutMainLoop();

	glutDestroyWindow(glutWin);
		
	/* Stg_Class_Delete stuff */
	for (i = 0; i < instCnt; i++)
	{
		if (instance[i].nodeIndex) Memory_Free(instance[i].nodeIndex);
		if (instance[i].edgeIndex) Memory_Free(instance[i].edgeIndex);
	}
	if (instance) Memory_Free(instance);
	unloadGLMeshLayout(&meshLayout);
	glDeleteLists(fontBase, 127);
	
	return 0;
}

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
**	Concrete IJKTopology class for element or node with 6 neighbours
**	(only fully adjoining directions, no diagonals).
**
** Assumptions:
**
** Comments:
**
** $Id: GLMesh.h 2453 2004-12-21 04:49:34Z LukeHodkinson $
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Viewers_MeshViewer_GLMesh_h__
#define __Viewers_MeshViewer_GLMesh_h__

	/* Virtual function types */
	
	/* Textual name of this class */
	extern const Type GLMesh_Type;

	/** GLMesh class contents */
	#define __GLMesh \
		/* General info */ \
		__Stg_Class \
		\
		/* Stg_Class info */ \
		unsigned		vertCnt; \
		GLdouble*		verts; \
		unsigned		edgeCnt; \
		unsigned*		edges; \
		\
		unsigned		rankCnt; \
		unsigned*		localEdgeCnts; \
		unsigned**		localEdges; \
		unsigned*		shadowEdgeCnts; \
		unsigned**		shadowEdges; \
		unsigned*		vacantEdgeCnts; \
		unsigned**		vacantEdges; \
		\
		GLfloat			localColor[3]; \
		GLfloat			shadowColor[3]; \
		GLfloat			vacantColor[3]; \

	struct _GLMesh { __GLMesh };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	
	/** Create an GLMesh */
	GLMesh* GLMesh_New( void );
	
	/** Initialise an GLMesh */
	void GLMesh_Init(
		GLMesh*						self );
	
	/** Creation implementation */
	GLMesh* _GLMesh_New(
		SizeT						_sizeOfSelf, 
		Type						type,
		Stg_Class_DeleteFunction*				_delete,
		Stg_Class_PrintFunction*				_print,
		Stg_Class_CopyFunction*				_copy );
	
	/** Initialisation implementation */
	void _GLMesh_Init(
		GLMesh*						self );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/** Stg_Class_Delete() implementation */
	void _GLMesh_Delete( void* glMesh );
	
	/** Stg_Class_Print() implementation */
	void _GLMesh_Print( void* glMesh, Stream* stream );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public member functions
	*/
	
	void GLMesh_BuildFromMesh( void* glMesh, MeshLayout* mesh );
	
	void GLMesh_RenderGlobal( void* glMesh );
	
	void GLMesh_RenderLocal( void* glMesh, Partition_Index rank );
	
	void GLMesh_RenderShadow( void* glMesh, Partition_Index rank );
	
	void GLMesh_RenderVacant( void* glMesh, Partition_Index rank );
	
	void GLMesh_RenderRank( void* glMesh, Partition_Index rank );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/
	
	void _GLMesh_BuildLocalEdges( void* glMesh, MeshLayout* mesh, Partition_Index rank );
	
	void _GLMesh_BuildShadowEdges( void* glMesh, MeshLayout* mesh, Partition_Index rank );
	
	void _GLMesh_BuildVacantEdges( void* glMesh, MeshLayout* mesh, Partition_Index rank );
	
	void _GLMesh_Destroy( void* glMesh );
	
	
#endif /* __Viewers_MeshViewer_GLMesh_h__ */

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
**	Abstract class faciliting how mesh-based "modellers"/"solvers" are laid out and execute.
**
** Assumptions:
**
** Comments:
**	This is a class, of which subclasses can be defined, which can override default methods.
**	Currently built with only MeshPIC_Context in mind (for Snac).
**
** $Id: MeshContext.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __FD_MeshContext_h__
#define __FD_MeshContext_h__
	

	/* Textual name of this class */
	extern const Type MeshContext_Type;
	
	#define __MeshContext \
		/* General info */ \
		__AbstractContext \
		\
		/* Virtual info */ \
		\
		/* MeshContext info */ \
		MeshLayout*			meshLayout; \
		Mesh*				mesh; \
		ExtensionManager*		meshExtensionMgr;
	struct MeshContext { __MeshContext };
	
	
	/* No "MeshContext_New" and "MeshContext_Init" as this is an abstract class */
	
	/* Creation implementation / Virtual constructor */
	MeshContext* _MeshContext_New( 
		SizeT				sizeOfSelf,
		Type				type,
		Stg_Class_DeleteFunction*		_delete,
		Stg_Class_PrintFunction*		_print,
		Stg_Class_CopyFunction*		_copy, 
		Stg_Component_DefaultConstructorFunction*	_defaultConstructor,
		Stg_Component_ConstructFunction*			_construct,
		Stg_Component_BuildFunction*		_build,
		Stg_Component_InitialiseFunction*		_initialise,
		Stg_Component_ExecuteFunction*		_execute,
		Stg_Component_DestroyFunction*		_destroy,
		Name							name,
		Bool						initFlag,
		AbstractContext_SetDt*		_setDt,
		double				start,
		double				stop,
		MeshLayout*			meshLayout,
		SizeT				nodeSize,
		SizeT				elementSize,
		MPI_Comm			communicator,
		Dictionary*			dictionary );
	
	/* Initialisation implementation */
	void _MeshContext_Init( MeshContext* self, MeshLayout* meshLayout, SizeT nodeSize, SizeT elementSize );
	
	
	/* Stg_Class_Delete implementation */
	void _MeshContext_Delete( void* meshContext );
	
	/* Print implementation */
	void _MeshContext_Print( void* meshContext, Stream* stream);
	
	
	/* Build the mesh implementation */
	void _MeshContext_Build( Context* context, void* data );
	
	/* Initialise the mesh implementation */
	void _MeshContext_InitialConditions( Context* context, void* data );
	
#endif /* __FD_MeshContext_h__ */

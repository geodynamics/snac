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
**
** Assumptions:
**
** Invariants:
**
** Comments:
**
** $Id: Remesher.h 2225 1970-01-02 13:48:23Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __StGermain_Discretisation_Utils_Remesher_h__
#define __StGermain_Discretisation_Utils_Remesher_h__

	/* Textual name of this class. */
	extern const Type Remesher_Type;

	/* Virtual function types. */
	typedef void (Remesher_SetMeshFunc)( void* remesher, Mesh* mesh );

	/* Class contents. */
	#define __Remesher					\
		/* General info */				\
		__Stg_Component					\
								\
		/* Virtual info */				\
		Remesher_SetMeshFunc*	setMeshFunc;		\
								\
		/* Remesher info ... */				\
		char*			meshType;		\
		Mesh*			mesh;			\
		MPI_Comm		comm;

	struct Remesher { __Remesher };


	/*-----------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/

	#define CLASS_ARGS					\
		SizeT				_sizeOfSelf,	\
		Type				type,		\
		Stg_Class_DeleteFunction*	_delete,	\
		Stg_Class_PrintFunction*	_print, 	\
		Stg_Class_CopyFunction*		_copy

	#define COMPONENT_ARGS								\
		Stg_Component_DefaultConstructorFunction*	_defaultConstructor,	\
		Stg_Component_ConstructFunction*		_construct, 		\
		Stg_Component_BuildFunction*			_build, 		\
		Stg_Component_InitialiseFunction*		_initialise, 		\
		Stg_Component_ExecuteFunction*			_execute, 		\
		Stg_Component_DestroyFunction*			_destroy, 		\
		Name						name, 			\
		Bool						initFlag

	#define REMESHER_ARGS				\
		Remesher_SetMeshFunc*	setMeshFunc


	/* Creation implementation */
	Remesher* _Remesher_New( CLASS_ARGS, 
				 COMPONENT_ARGS, 
				 REMESHER_ARGS );

	/* Initialisation implementation functions */
	void _Remesher_Init( Remesher* self );


	/*-----------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _Remesher_Delete( void* remesher );
	void _Remesher_Print( void* remesher, Stream* stream );
	Remesher* _Remesher_DefaultNew( Name name );
	void _Remesher_Construct( void* remesher, Stg_ComponentFactory* cf, void* data );
	void _Remesher_Build( void* remesher, void* data );
	void _Remesher_Initialise( void* remesher, void* data );
	void _Remesher_Execute( void* remesher, void* data );
	void _Remesher_Destroy( void* remesher, void* data );


	/*-----------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	#define Remesher_SetMesh( self, mesh )		\
		(self)->setMeshFunc( self, mesh )


	/*-----------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

#endif

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
** Comments:
**
** $Id: MeshCoarsener_Hexa.h 2453 2004-12-21 04:49:34Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisaton_Mesh_MeshCoarsener_Hexa_h__
#define __Discretisaton_Mesh_MeshCoarsener_Hexa_h__


	/* Virtual function types */
	
	/* Textual name of this class */
	extern const Type MeshCoarsener_Hexa_Type;

	/* MeshCoarsener_Hexa information */
	#define __MeshCoarsener_Hexa \
		/* General info */ \
		__Stg_Component \
		Dictionary*				dictionary; \
		\
		/* Virtual info */ \
		\
		/* MeshCoarsener_Hexa info ... */ \
		Mesh*					mesh;
	struct _MeshCoarsener_Hexa { __MeshCoarsener_Hexa };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	
	/* Create a MeshCoarsener_Hexa */
	
	MeshCoarsener_Hexa* MeshCoarsener_Hexa_DefaultNew( Name name );
	
	#define MeshCoarsener_Hexa_New( name ) \
		MeshCoarsener_Hexa_New_Param( name, NULL)
	
	/* Create a MeshCoarsener_Hexa with all additional parameters */
	MeshCoarsener_Hexa* MeshCoarsener_Hexa_New_Param(
		Name						name,
		Dictionary*					dictionary );
	
	/* Initialise a MeshCoarsener_Hexa */
	void MeshCoarsener_Hexa_Init(
		MeshCoarsener_Hexa*				self,
		Name						name,
		Dictionary*					dictionary );
	
	/* Creation implementation */
	MeshCoarsener_Hexa* _MeshCoarsener_Hexa_New(
		SizeT						_sizeOfSelf, 
		Type						type,
		Stg_Class_DeleteFunction*				_delete,
		Stg_Class_PrintFunction*				_print,
		Stg_Class_CopyFunction*				_copy, 
		Stg_Component_DefaultConstructorFunction*	_defaultConstructor,
		Stg_Component_ConstructFunction*			_construct,
		Stg_Component_BuildFunction*		_build,
		Stg_Component_InitialiseFunction*		_initialise,
		Stg_Component_ExecuteFunction*		_execute,
		Stg_Component_DestroyFunction*		_destroy,
		Name							name,
		Bool						initFlag,
		Dictionary*					dictionary );
	
	/* Initialisation implementation functions */
	void _MeshCoarsener_Hexa_Init(
		MeshCoarsener_Hexa*				self );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/* Stg_Class_Delete meshCoarsener implementation */
	void _MeshCoarsener_Hexa_Delete( void* meshCoarsener );
	
	/* Print meshCoarsener implementation */
	void _MeshCoarsener_Hexa_Print( void* meshCoarsener, Stream* stream );
	
	void _MeshCoarsener_Hexa_Construct( void* meshCoarsener, Stg_ComponentFactory *cf, void* data );
	
	void _MeshCoarsener_Hexa_Build( void* meshCoarsener, void *data );
	
	void _MeshCoarsener_Hexa_Initialise( void* meshCoarsener, void *data );
	
	void _MeshCoarsener_Hexa_Execute( void* meshCoarsener, void *data );

	void _MeshCoarsener_Hexa_Destroy( void* meshCoarsener, void *data );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public member functions
	*/
	
	void MeshCoarsener_Hexa_Coarsen( void* meshCoarsener, 
					 void* hexaMesh, 
					 unsigned level, 
					 IndexMap** nodeFineToCoarse,
					 IndexMap** nodeCoarseToFine );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/
	
	void _MeshCoarsener_Hexa_CalcElementBaseAndLevel( void*			meshCoarsener, 
							  Element_Index		size, 
							  unsigned*		level, 
							  Element_Index*	base );
	
	
#endif /* __Discretisaton_Mesh_MeshCoarsener_Hexa_h__ */

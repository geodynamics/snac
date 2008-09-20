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
** $Id: DummyMD.h 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisaton_Mesh_DummyMD_h__
#define __Discretisaton_Mesh_DummyMD_h__


	/* Virtual function types */
	
	/* Textual name of this class */
	extern const Type DummyMD_Type;

	/* DummyMD information */
	#define __DummyMD \
		/* General info */ \
		__MeshDecomp \
		\
		/* Virtual info */ \
		\
		/* DummyMD info ... */
	struct _DummyMD { __DummyMD };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	
	/* Create a DummyMD */
	DummyMD* DummyMD_DefaultNew( Name name );
	
	DummyMD* DummyMD_New(
		Name						name,
		Dictionary*					dictionary,
		MPI_Comm					communicator,
		ElementLayout*					eLayout,
		NodeLayout*					nLayout );
	
	/* Initialise a DummyMD */
	void DummyMD_Init(
		DummyMD*					self,
		Name						name,
		Dictionary*					dictionary,
		MPI_Comm					communicator,
		ElementLayout*					eLayout,
		NodeLayout*					nLayout );
	
	/* Creation implementation */
	DummyMD* _DummyMD_New(
		SizeT						_sizeOfSelf, 
		Type						type,
		Stg_Class_DeleteFunction*				_delete,
		Stg_Class_PrintFunction*				_print,
		Stg_Class_CopyFunction*				_copy, 
		Stg_Component_DefaultConstructorFunction*	_defaultConstructor,
		Stg_Component_ConstructFunction*			_construct,
		Stg_Component_BuildFunction*				_build,
		Stg_Component_InitialiseFunction*			_initialise,
		Stg_Component_ExecuteFunction*				_execute,
		Stg_Component_DestroyFunction*				_destroy,
		Name									name,
		Bool									initFlag,
		MeshDecomp_Node_MapLocalToGlobalFunction*       nodeMapLocalToGlobal,
		MeshDecomp_Node_MapDomainToGlobalFunction*      nodeMapDomainToGlobal,
		MeshDecomp_Node_MapShadowToGlobalFunction*      nodeMapShadowToGlobal,
		MeshDecomp_Node_MapGlobalToLocalFunction*       nodeMapGlobalToLocal,
		MeshDecomp_Node_MapGlobalToDomainFunction*      nodeMapGlobalToDomain,
		MeshDecomp_Node_MapGlobalToShadowFunction*      nodeMapGlobalToShadow,
		MeshDecomp_Element_MapLocalToGlobalFunction*	elementMapLocalToGlobal,
		MeshDecomp_Element_MapDomainToGlobalFunction*	elementMapDomainToGlobal,
		MeshDecomp_Element_MapShadowToGlobalFunction*	elementMapShadowToGlobal,
		MeshDecomp_Element_MapGlobalToLocalFunction*	elementMapGlobalToLocal,
		MeshDecomp_Element_MapGlobalToDomainFunction*	elementMapGlobalToDomain,
		MeshDecomp_Element_MapGlobalToShadowFunction*	elementMapGlobalToShadow,
		MeshDecomp_Shadow_ProcCountFunction*		shadowProcCount,
		MeshDecomp_Shadow_BuildProcsFunction*		shadowBuildProcs,
		MeshDecomp_Shadow_ProcElementCountFunction*     shadowProcElementCount,
		MeshDecomp_Proc_WithElementFunction*		procWithElement,
		Dictionary*					dictionary,
		MPI_Comm					communicator,
		ElementLayout*					eLayout,
		NodeLayout*					nlayout );
	
	/* Initialisation implementation functions */
	void _DummyMD_Init(
		DummyMD*					self );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/* Stg_Class_Delete DummyMD implementation */
	void _DummyMD_Delete( void* dummyMD );
	
	/* Print DummyMD implementation */
	void _DummyMD_Print( void* dummyMD, Stream* stream );
	
	void _DummyMD_Construct( void* dummyMD, Stg_ComponentFactory *cf, void* data );
	
	void _DummyMD_Build( void* dummyMD, void* data );
	
	void _DummyMD_Initialise( void* dummyMD, void* data );
	
	void _DummyMD_Execute( void* dummyMD, void* data );
	
	void _DummyMD_Destroy( void* dummyMD, void* data );
	
	Partition_Index _DummyMD_Shadow_ProcCount( void* dummyMD );
	
	void _DummyMD_Shadow_BuildProcs( void* dummyMD, Partition_Index* procs );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public member functions
	*/
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/
	
#endif /* __Discretisaton_Mesh_DummyMD_h__ */

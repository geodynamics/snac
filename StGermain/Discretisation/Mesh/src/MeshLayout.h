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
** $Id: MeshLayout.h 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisaton_Mesh_MeshLayout_h__
#define __Discretisaton_Mesh_MeshLayout_h__


	/* Virtual function types */
	
	/* Textual name of this class */
	extern const Type MeshLayout_Type;

	/* MeshLayout information */
	#define __MeshLayout \
		/* General info */ \
		__Stg_Component \
		Stream*				debug; \
		\
		/* Virtual info */ \
		\
		/* MeshLayout info ... */ \
		ElementLayout*			elementLayout; \
		NodeLayout*			nodeLayout; \
		MeshDecomp*			decomp;
	struct _MeshLayout { __MeshLayout };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	
	/* Create a MeshLayout */
	MeshLayout* MeshLayout_DefaultNew( Name name );

	MeshLayout* MeshLayout_New(
		Name						name,
		ElementLayout*					elementLayout,
		NodeLayout*					nodeLayout,
		MeshDecomp*					decomp );

	void MeshLayout_LoadFromDict( void* meshLayout, Dictionary* subDict, Dictionary* dictionary, Stg_ObjectList* objList);
	
	/* Initialise a MeshLayout */
	void MeshLayout_Init(
		MeshLayout*					self,
		Name						name,
		ElementLayout*					elementLayout,
		NodeLayout*					nodeLayout,
		MeshDecomp*					decomp );
	
	/* Creation implementation */
	MeshLayout* _MeshLayout_New(
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
		Bool							initFlag,
		ElementLayout*					elementLayout,
		NodeLayout*					nodeLayout,
		MeshDecomp*					decomp );

	/* Initialisation implementation functions */
	void _MeshLayout_Init(
		MeshLayout*					self,
		ElementLayout*					elementLayout,
		NodeLayout*					nodeLayout,
		MeshDecomp*					decomp );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/* Stg_Class_Delete MeshLayout implementation */
	void _MeshLayout_Delete( void* meshLayout );
	
	/* Print MeshLayout implementation */
	void _MeshLayout_Print( void* meshLayout, Stream* stream );
	
	void _MeshLayout_Construct( void* meshLayout, Stg_ComponentFactory *cf, void* data );
	
	void _MeshLayout_Build( void* meshLayout, void *data );
	
	void _MeshLayout_Initialise( void* meshLayout, void *data );
	
	void _MeshLayout_Execute( void* meshLayout, void *data );
	
	void _MeshLayout_Destroy( void* meshLayout, void *data );
	
	/* Copy */
	#define MeshLayout_Copy( self ) \
		(MeshLayout*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define MeshLayout_DeepCopy( self ) \
		(MeshLayout*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _MeshLayout_Copy( void* meshLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public member functions
	*/
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/
	
	
#endif /* __Discretisaton_Mesh_MeshLayout_h__ */

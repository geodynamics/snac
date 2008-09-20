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
** $Id: HMesh.h 3584 2006-05-16 11:11:07Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisaton_Mesh_HMesh_h__
#define __Discretisaton_Mesh_HMesh_h__

	/** Textual name of this class */
	extern const Type HMesh_Type;

	/** Virtual function types */

	/** Mesh class contents */
	typedef struct {
		Mesh*		mesh;
		Bool		ownMesh;
		unsigned*	nodeUp;
		unsigned*	nodeDown;
		unsigned*	nodeGlobals;
		unsigned*	nElsUp;
		unsigned**	elUp;
		unsigned*	elDown;
		double		weldEpsilon;
	} HMesh_Level;

	#define __HMesh \
		/* General info */ \
		__Mesh \
		\
		/* Virtual info */ \
		\
		/* HMesh info */ \
		unsigned	nLevels; \
		unsigned	baseLevel; \
		HMesh_Level*	levels; \

	struct _HMesh { __HMesh };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/

	/* Create a HMesh */
	HMesh* HMesh_DefaultNew( Name name );

	HMesh* HMesh_New( Name		name,
			  void*		layout,
			  SizeT		_nodeSize,
			  SizeT		_elementSize,
			  void*		extension_Register,
			  Dictionary*	dictionary );

	/* Initialise a Mesh */
	void HMesh_Init( HMesh*		self,
			 Name		name,
			 void*		layout,
			 SizeT		_nodeSize,
			 SizeT		_elementSize,
			 void*		extension_Register,
			 Dictionary*	dictionary );
	
	/* Creation implementation */
	HMesh* _HMesh_New( SizeT					_sizeOfSelf, 
			   Type						type,
			   Stg_Class_DeleteFunction*			_delete,
			   Stg_Class_PrintFunction*			_print,
			   Stg_Class_CopyFunction*			_copy, 
			   Stg_Component_DefaultConstructorFunction*	_defaultConstructor,
			   Stg_Component_ConstructFunction*		_construct,
			   Stg_Component_BuildFunction*			_build,
			   Stg_Component_InitialiseFunction*		_initialise,
			   Stg_Component_ExecuteFunction*		_execute,
			   Stg_Component_DestroyFunction*		_destroy,
			   Name						name,
			   Bool						initFlag,
			   Mesh_Node_IsLocalFunction*			nodeIsLocal,
			   Mesh_Node_IsShadowFunction*			nodeIsShadow,
			   Mesh_Element_IsLocalFunction*		elementIsLocal,
			   Mesh_Element_IsShadowFunction*		elementIsShadow,
			   void*					layout,
			   SizeT					_nodeSize,
			   SizeT					_elementSize, 
			   void*					extension_Register,
			   Dictionary*					dictionary );

	void _HMesh_LoadFromDict( void*			mesh, 
				  Dictionary*		subDict, 
				  Dictionary*		dictionary, 
				  Stg_ObjectList*	objList);

	/* Initialisation implementation functions */
	void _HMesh_Init( HMesh* self );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _HMesh_Delete( void* mesh );
	void _HMesh_Print( void* mesh, Stream* stream );

	#define HMesh_Copy( self ) \
		(Mesh*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define HMesh_DeepCopy( self ) \
		(Mesh*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _HMesh_Copy( void* mesh, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );

	void _HMesh_Build( void* mesh, void* data );
	void _HMesh_Construct( void* mesh, Stg_ComponentFactory* cf, void* data );
	void _HMesh_Initialise( void* mesh, void* data );
	void _HMesh_Execute( void* mesh, void* data );
	void _HMesh_Destroy( void* mesh, void* data );
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	void HMesh_SetNLevels( HMesh* self, unsigned nLevels, unsigned baseLevel );
	void HMesh_SetMesh( HMesh* self, unsigned level, Mesh* mesh, Bool own );
	Mesh* HMesh_GenMesh( HMesh* self, unsigned level );
	Mesh* HMesh_GetMesh( HMesh* self, unsigned level );
	Mesh* HMesh_GetBaseMesh( HMesh* self );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

	char* HMesh_GenName( HMesh* self, unsigned level );
	void HMesh_BuildNodes( HMesh* self );
	void HMesh_BuildElements( HMesh* self );
	Bool HMesh_FindWeld( HMesh* self, unsigned level, Coord crd, unsigned* weld );
	void HMesh_CalcWeldEpsilons( HMesh* self );
	void HMesh_DeleteLevel( HMesh_Level* level );

#endif /* __Discretisaton_Mesh_HMesh_h__ */

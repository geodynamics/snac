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
**	Allows the user to lay out which Variables exist at each index in a structure (eg nodes of a mesh). Also allows
**	the user to "connect", through the use of the Variable class, where the the dofs that are conceptually
**	associated with each node actually reside in memory (e.g. it could be on a separate array, rather than on
**	the nodes themselves.
**
** Assumptions:
**
** Comments:
**	Note on terminology: we call this class a DofLayout because we consider Dofs (Degrees of Freedom)
**	as instantiations of a Variable onto some structure (i.e. we might say a given mesh has 30,000
**	velocity dofs if it has 10,000 nodes, each with a vx, vy, vz (Variables). )
**
**	This general DofLayout should have subclasses which add a set of dofs to each node of a mesh, etc.
**
** $Id: DofLayout.h 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Utils_DofLayout_h__
#define __Discretisation_Utils_DofLayout_h__
	
	/** Textual name of this class */
	extern const Type DofLayout_Type;
	
	/** DofLayout contents */
	#define __DofLayout \
		/* General info */ \
		__Stg_Component \
		\
		/* Virtual info */ \
		\
		/* Stg_Class info */ \
		/** Variable register containing the variables referenced in this layout. */ \
		Variable_Register*	_variableRegister; \
		\
		/** The number of items that are controlled by this dof layout, hence number of entries in each set. */ \
		Index			_numItemsInLayout; \
		/** The total number of variables referred to across all the indices in the dof layout. */ \
		Index			_totalVarCount; \
		/** Array of sets, 1 per variable, which record the items in the layout that have that variable as a dof. */ \
		IndexSet**		_variableEnabledSets; \
		/** Table which maps local storage indices of variables to indices into the Variable_Register. */ \
		Variable_Index*		_varIndicesMapping; \
		\
		/** Array containing number of dofs at each index (e.g. at each node in a mesh) */ \
		Dof_Index*		dofCounts; \
		/** 2D Array: for each index (e.g. each node), stores an array (of size dofCounts[i]) containing
		the indexes into the DofLayout::_variableRegister of the Variable s at that index. */ \
		Variable_Index**	varIndices; \

	/** Allows the user to lay out which Variables exist at each index in a structure (eg nodes of a mesh) - see
	 DofLayout.h for details. */
	struct _DofLayout { __DofLayout };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructor
	*/
	
	DofLayout*	DofLayout_DefaultNew( Name name );
	
	DofLayout*	DofLayout_New( Name name, Variable_Register* variableRegister, Index numItemsInLayout );
	
	void		DofLayout_Init(DofLayout* self, Name name, Variable_Register* variableRegister, Index numItemsInLayout );
	
	DofLayout*	_DofLayout_New( 
				SizeT						_sizeOfSelf, 
				Type						type,
				Stg_Class_DeleteFunction*				_delete,
				Stg_Class_PrintFunction*				_print,
				Stg_Class_CopyFunction*				_copy,
				Stg_Component_DefaultConstructorFunction*	_defaultConstructor,
				Stg_Component_ConstructFunction*			_construct,
				Stg_Component_BuildFunction*			_build,
				Stg_Component_InitialiseFunction*			_initialise,
				Stg_Component_ExecuteFunction*			_execute,
				Stg_Component_DestroyFunction*			_destroy,
				Name							name,
				Bool							initFlag,
				Variable_Register*				variableRegister,
				Index						numItemsInLayout);
	
	void _DofLayout_Init(void* dofLayout, Variable_Register* variableRegister, Index numItemsInLayout, Variable_Index baseVariableCount, Variable** baseVariableArray );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** General virtual functions
	*/
	
	/** Stg_Class_Delete() implementation */
	void	_DofLayout_Delete(void* dofLayout);
	
	/** Stg_Class_Print() implementation */
	void	_DofLayout_Print(void* dofLayout, Stream* stream);
	
	/* Copy */
	#define DofLayout_Copy( self ) \
		(DofLayout*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define DofLayout_DeepCopy( self ) \
		(DofLayout*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _DofLayout_Copy( void* dofLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/** Stg_Component_Construct() implementation */
	void _DofLayout_Construct( void* dofLayout, Stg_ComponentFactory* cf, void* data );
	
	/** Stg_Component_Build() implementation */
	void _DofLayout_Build( void* dofLayout, void* data );
	
	/** Stg_Component_Initialise() implementation */
	void _DofLayout_Initialise( void* dofLayout, void* data );
	
	/* Stg_Component_Execute() implementation */
	void _DofLayout_Execute( void* dofLayout, void* data );
	
	/* Stg_Component_Destroy() implementation */
	void _DofLayout_Destroy( void* dofLayout, void* data );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Macros
	*/
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	/** Adds a new Dof, by specifying the Variable index (into the Variable_Register) the dof is an instantiation of,
	and the index in the client's structure it applies to. An example might be (var index 0 ("vx"), node 100). */
	void	DofLayout_AddDof_ByVarIndex(void* dofLayout, Variable_Index varIndex, Index index);
	
	/** Adds a new Dof, by specifying the Variable name the dof is an instantiation of, and the index into the
	client's structure it applies to. An example might be (variable "vx", node 100).  */
	void	DofLayout_AddDof_ByVarName(void* dofLayout, Name varName, Index index);

	/** Gets a ptr to the Variable that lives at a particular index for a Dof */
	Variable* DofLayout_GetVariable(void* dofLayout, Index index, Dof_Index dofAtItemIndex );

	/** Shortcut macro to set a value of a particular dof, at a given index, without having to worry
	about the underlying variable interface. (Only have a "double" version as that's all we need so far */
	#define DofLayout_SetValueDouble( dofLayout, index, dofAtItemIndex, value ) \
		( Variable_SetValueDouble( DofLayout_GetVariable( dofLayout, index, dofAtItemIndex ), index, value ) )
	
	/** Shortcut macro to get a value of a particular dof, at a given index, without having to worry
	about the underlying variable interface. (Only have a "double" version as that's all we need so far */
	#define DofLayout_GetValueDouble( dofLayout, index, dofAtItemIndex ) \
		( Variable_GetValueDouble( DofLayout_GetVariable( dofLayout, index, dofAtItemIndex ), index ) )
	
	/** Utility function to set every dof's value to zero. */
	void DofLayout_SetAllToZero( void* dofLayout );
	
	/** Copies values from one dofLayout to another. Note this function requires the destination and
	source dofLayouts to have the same "shape". */
	void DofLayout_CopyValues( void* dofLayout, void* destDofLayout );
	
	/* Map the dofLayout to another set of indices */
	void DofLayout_Remap( void* dofLayout, Index newIndexCount, IndexMap* map );

	/** Adds each variable in this array to each item in the dof layout */
	void DofLayout_AddAllFromVariableArray( void* dofLayout, Variable_Index variableCount, Variable** variableArray ) ;

	/** Saves all variables used by this dofLayout to files */
	void DofLayout_SaveAllVariablesToFiles( void* dofLayout, char* prefixString, Processor_Index rank );

	/** Saves all variables used by this dofLayout to files */
	void DofLayout_LoadAllVariablesFromFiles( void* dofLayout, char* prefixString, Processor_Index rank );

#endif /* __Discretisation_Utils_DofLayout_h__ */

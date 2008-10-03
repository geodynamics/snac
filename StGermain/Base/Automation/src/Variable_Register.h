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
**	Instantiates the Variable_Register such that there is only one set of DOF descriptions (for example, all the nodes have the same
**	DOF configuration).
**
** Assumptions:
**	All "adding" is done before the Variable_Register_Build(...), which will be done in the mesh's initialisation.
**
** Comments:
**	You "add" DOF descriptors by using the Variable_Register_Add( variable_Register, name, offset ), where the "name" is the
**	textual name for the DOF that will be used in the I/O of the DOF (e.g. for initial and boundary conditions), and "offset" is
**	the memory offset of the DOF in the construct that it will be used (e.g. node.vel[0]-node ).
**
** $Id: Variable_Register.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Base_Automation_Variable_Register_h__
#define __Base_Automation_Variable_Register_h__
	
	
	extern const Type Variable_Register_Type;
	
	
	#define __Variable_Register \
		/* General info */ \
		__Stg_Class \
		\
		/* Virtual info */ \
		\
		/* Stg_Class info */ \
		/** Total number of variables counted */ \
		Variable_Index				count; \
		SizeT					_size; \
		SizeT					_delta; \
		Variable**				_variable; \
		
	struct _Variable_Register { __Variable_Register };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructor
	*/
	Variable_Register*	Variable_Register_New(void);
	
	void			Variable_Register_Init(Variable_Register* self);
	
	Variable_Register*	_Variable_Register_New( 
					SizeT			_sizeOfSelf,
					Type			type,
					Stg_Class_DeleteFunction*	_delete,
					Stg_Class_PrintFunction*	_print, 
					Stg_Class_CopyFunction*	_copy );
		
	void			_Variable_Register_Init(void* variable_Register);
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** General virtual functions
	*/
	
	void	_Variable_Register_Delete(void* variable_Register);
	
	void	_Variable_Register_Print(void* variable_Register, Stream* stream);

	void*	_Variable_Register_Copy( void* vr, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Macros
	*/
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Build functions
	*/
	
	/* Add a variable... do before building */
	Variable_Index	Variable_Register_Add(void* variable_Register, Variable* variable);
	
	void		Variable_Register_BuildAll(void* variable_Register);
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public Functions & Macros
	*/
	
	Variable_Index	Variable_Register_GetIndex(void* variable_Register, Name name);

	Variable*	Variable_Register_GetByName(void* variable_Register, Name name);

	Variable*	Variable_Register_GetByIndex( void* variable_Register, Variable_Index varIndex);	
	
#endif /* __Base_Automation_Variable_Register_h__ */

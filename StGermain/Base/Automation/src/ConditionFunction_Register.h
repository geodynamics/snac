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
**	Instantiates the ConditionFunction_Register such that there is only one set of DOF descriptions (for example, all the nodes have the same
**	DOF configuration).
**
** Assumptions:
**	All "adding" is done before the ConditionFunction_Register_Build(...), which will be done in the mesh's initialisation.
**
** Comments:
**	You "add" DOF descriptors by using the ConditionFunction_Register_Add( variable_Register, name, offset ), where the "name" is the
**	textual name for the DOF that will be used in the I/O of the DOF (e.g. for initial and boundary conditions), and "offset" is
**	the memory offset of the DOF in the construct that it will be used (e.g. node.vel[0]-node ).
**
** $Id: ConditionFunction_Register.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Base_Automation_ConditionFunction_Register_h__
#define __Base_Automation_ConditionFunction_Register_h__
	
	
	extern const Type ConditionFunction_Register_Type;
	
	
	#define __ConditionFunction_Register \
		/* General info */ \
		__Stg_Class \
		\
		/* Virtual info */ \
		\
		/* Stg_Class info */ \
		ConditionFunction_Index			count; \
		SizeT					_size; \
		SizeT					_delta; \
		ConditionFunction**			_cf; \
		
	struct _ConditionFunction_Register { __ConditionFunction_Register };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructor
	*/
	
	ConditionFunction_Register*	ConditionFunction_Register_New(void);
	
	void				ConditionFunction_Register_Init(ConditionFunction_Register* self);
	
	ConditionFunction_Register*	_ConditionFunction_Register_New( 
						SizeT			_sizeOfSelf,
						Type			type,
						Stg_Class_DeleteFunction*	_delete,
						Stg_Class_PrintFunction*	_print,
						Stg_Class_CopyFunction*	_copy );
		
	void				_ConditionFunction_Register_Init(void* conditionFunction_Register);
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** General virtual functions
	*/
	
	void	_ConditionFunction_Register_Delete( void* conditionFunction_Register );
	
	void	_ConditionFunction_Register_Print( void* conditionFunction_Register, Stream* stream );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Macros
	*/


	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/


	/*--------------------------------------------------------------------------------------------------------------------------
	** Build functions
	*/
	
	ConditionFunction_Index	ConditionFunction_Register_Add(void* conditionFunction_Register, ConditionFunction* cf);
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Functions
	*/
	
	ConditionFunction_Index	ConditionFunction_Register_GetIndex(void* conditionFunction_Register, Name name);

	void ConditionFunction_Register_PrintNameOfEachFunc( void* conditionFunction_Register, Stream* stream );

	
#endif /* __Base_Automation_ConditionFunction_Register_h__ */

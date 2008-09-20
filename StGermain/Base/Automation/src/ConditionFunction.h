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
**	Abstract class faciliting the Boundary Condition definition of this system. 
**	Meshs should only reference this... the base class, not the implementations.
**
** Assumptions:
**
** Comments:
**
** $Id: ConditionFunction.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Base_Automation_ConditionFunction_h__
#define __Base_Automation_ConditionFunction_h__


	typedef void	(ConditionFunction_ApplyFunc)	(Index index, Variable_Index var_I, void* context, void* result);
	

	extern const Type ConditionFunction_Type;
	
	
	#define __ConditionFunction \
		/* General info */ \
		__Stg_Class \
		\
		/* Virtual info */ \
		\
		/* Stg_Class info */ \
		Dictionary*				dictionary; \
		ConditionFunction_ApplyFunc*		apply; \
		Name					name; \
		
	struct _ConditionFunction { __ConditionFunction };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructor
	*/
	
	ConditionFunction*	ConditionFunction_New(ConditionFunction_ApplyFunc* apply, Name name);
	
	void			ConditionFunction_Init(ConditionFunction* self, ConditionFunction_ApplyFunc* apply, Name name);
	
	ConditionFunction*	_ConditionFunction_New( 
					SizeT				_sizeOfSelf, 
					Type				type,
					Stg_Class_DeleteFunction*		_delete,
					Stg_Class_PrintFunction*		_print, 
					Stg_Class_CopyFunction*		_copy, 
					ConditionFunction_ApplyFunc*	apply,
					Name				name);
	
	void			_ConditionFunction_Init(void* conditionFunction, ConditionFunction_ApplyFunc* apply, Name name);
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** General virtual functions
	*/

	void	_ConditionFunction_Delete(void* conditionFunction);
	
	void	_ConditionFunction_Print(void* conditionFunction, Stream* stream);
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Macros
	*/
	
	#define ConditionFunction_Apply(self, index, var_I, context, result) \
		(self)->apply((index), (var_I), (context), (result))
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Build functions
	*/
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Functions
	*/
	
	/* Apply the condition function */
	void	_ConditionFunction_Apply(void* conditionFunction, Index index, Variable_Index var_I, void* context, void* result);


#endif /* __Base_Automation_ConditionFunction_h__ */

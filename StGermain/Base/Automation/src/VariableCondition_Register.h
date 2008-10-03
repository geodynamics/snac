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
**	The original sizes need to be manually set by the user.... this whole system needs rethinking... it can be done better
**
** $Id: VariableCondition_Register.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Base_Automation_VariableCondition_Register_h__
#define __Base_Automation_VariableCondition_Register_h__
	

	/* Child classes must define these factories */
	typedef VariableCondition*	(VariableCondition_Register_FactoryFunc)	( Variable_Register* variable_Register, ConditionFunction_Register* conFunc_Register, Dictionary* dictionary, void* data );
	
	
	/** Textual name of this class */
	extern const Type VariableCondition_Register_Type;
	
	
	/** Global, default instantiation of the register... will be loaded with in built types (built in Init.c) */
	extern VariableCondition_Register* variableCondition_Register;
	
	
	/* Register entry info */
	#define __VariableCondition_Register_Entry \
		Type					type; \
		VariableCondition_Register_FactoryFunc*	factory;
	struct _VariableCondition_Register_Entry { __VariableCondition_Register_Entry };
	
	
	/* VariableCondition_Register info */
	#define __VariableCondition_Register \
		/* General info */ \
		__Stg_Class \
		\
		/* Virtual info */ \
		\
		/* VariableCondition_Register info */ \
		VariableCondition_Index			count; \
		SizeT					_size; \
		SizeT					_delta; \
		VariableCondition_Register_Entry*	entry;
	struct _VariableCondition_Register { __VariableCondition_Register };
	
	
	/* Create a new VariableCondition_Register */
	VariableCondition_Register* VariableCondition_Register_New(void);
	
	/* Initialise a VariableCondition_Register */
	void VariableCondition_Register_Init(
			void*				variableCondition_Register);
	
	/* Creation implementation / Virtual constructor */
	VariableCondition_Register* _VariableCondition_Register_New( 
		SizeT					_sizeOfSelf,
		Type					type,
		Stg_Class_DeleteFunction*			_delete,
		Stg_Class_PrintFunction*			_print, 
		Stg_Class_CopyFunction*			_copy );
	
	/* Initialisation implementation */
	void _VariableCondition_Register_Init( void* variableCondition_Register);
	
	
	/* Stg_Class_Delete implementation */
	void _VariableCondition_Register_Delete( void* variableCondition_Register );
	
	/* Print implementation */
	void _VariableCondition_Register_Print( void* variableCondition_Register, Stream* stream );
	
	
	/* Add a new entry */
	VariableCondition_Index VariableCondition_Register_Add( 
		void*					variableCondition_Register, 
		Type					type, 
		VariableCondition_Register_FactoryFunc*	factory );
	
	/* Get the handle to an entry */
	VariableCondition_Index VariableCondition_Register_GetIndex( void* variableCondition_Register, Type type );
	
	/* Get an entry from the register */
	#define VariableCondition_Register_At( variableCondition_Register, handle ) \
		( &(variableCondition_Register)->entry[(handle)] )
	VariableCondition_Register_Entry* _VariableCondition_Register_At( 
		void*					variableCondition_Register, 
		VariableCondition_Index			handle );
	
	/* Create a new instance of a type */
	VariableCondition* VariableCondition_Register_CreateNew(
				void* variableCondition_Register, 
				void* variable_Register, 
				void* conFunc_Register, 
				Type type, 
				void* dictionary, 
				void* data );
	
#endif /* __Base_Automation_VariableCondition_Register_h__ */

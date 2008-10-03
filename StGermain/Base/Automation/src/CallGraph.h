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
** $Id$
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Base_Automation_Stg_CallGraph_h__
#define __Base_Automation_Stg_CallGraph_h__
	
	/* Templates of virtual functions */
	typedef struct {
		void*			functionPtr;
		Type			name;			/* assume its a const char* (i.e. we just need to copy ptr */
		void*			parentPtr;
		Type			parentName;		/* assume its a const char* (i.e. we just need to copy ptr */
		double			time;
		SizeT			memory;
		Index			called;
		Index			returned;
	} _Stg_CallGraph_Entry;
	
	typedef struct __Stg_CallGraph_Stack _Stg_CallGraph_Stack;
	struct __Stg_CallGraph_Stack {
		void*			functionPtr;
		Type			name;			/* assume its a const char* (i.e. we just need to copy ptr */
		double			time;
		SizeT			memory;
		Index			entry_I;
		_Stg_CallGraph_Stack*	pop;
	};
	
	/* Textual name of this class */
	extern const Type Stg_CallGraph_Type;
	
	/* Stg_Component information */
	#define __Stg_CallGraph \
		/* General info */ \
		__Stg_Class \
		\
		/* Virtual info */ \
		\
		/* Stg_CallGraph info */ \
		_Stg_CallGraph_Stack*	_stack; \
		_Stg_CallGraph_Entry*	table; \
		Index			_tableSize; \
		Index			_tableCount;
	struct Stg_CallGraph { __Stg_CallGraph };
	
	
	/** StGermain call graph singleton */
	extern Stg_CallGraph* stgCallGraph;
	
	/** Create a new CallGraph */
	Stg_CallGraph* Stg_CallGraph_New();
	
	/** Initialise an existing CallGraph */
	void Stg_CallGraph_Init( void* callGraph );
	
	/* Creation implementation */
	Stg_CallGraph* _Stg_CallGraph_New( 
		SizeT						_sizeOfSelf, 
		Type						type,
		Stg_Class_DeleteFunction*			_delete,
		Stg_Class_PrintFunction*			_print, 
		Stg_Class_CopyFunction*				_copy );
	
	
	/* Class Administration members ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	/* Initialisation implementation */
	void _Stg_CallGraph_Init( Stg_CallGraph* self );
	
	/* Delete implementation */
	void _Stg_CallGraph_Delete( void* callGraph );
	
	/* Print implementation */
	void _Stg_CallGraph_Print( void* callGraph, Stream* stream );
	
	/* Copy implementation */
	void* _Stg_CallGraph_Copy( void* callGraph, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap );
	
	
	/* Public member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	/* Copy the component */
	#define Stg_CallGraph_Copy( self ) \
		(Stg_CallGraph*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define Stg_CallGraph_DeepCopy(self) \
		(Stg_CallGraph*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	/** Push a new function call onto the stack, and lodge trace */
	void Stg_CallGraph_Push( void* callGraph, void* functionPtr, Type name );
	
	/** Pop the function on the stack, and finalise trace */
	void Stg_CallGraph_Pop( void* callGraph );
	
#endif /* __Base_Automation_Stg_CallGraph_h__ */

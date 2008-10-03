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
**	Allows users to access FieldVariables based on their textual name,
**	or index.
**
** Assumptions:
**
** Comments:
**
**
** $Id: FieldVariable_Register.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Utils_FieldVariable_Register_h__
#define __Discretisation_Utils_FieldVariable_Register_h__
	
	
	extern const Type FieldVariable_Register_Type;
	
	#define __FieldVariable_Register \
		/* General info */ \
		__NamedObject_Register \
		\
		/* Virtual info */ \
		\
		/* Stg_Class info */ \

	struct FieldVariable_Register { __FieldVariable_Register };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructor
	*/
	
	FieldVariable_Register*	FieldVariable_Register_New( void );
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** General virtual functions
	*/
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/
	#define FieldVariable_Register_Add NamedObject_Register_Add

	#define FieldVariable_Register_GetIndex NamedObject_Register_GetIndex 

	#define FieldVariable_Register_GetByName( self, fieldVariableName ) \
		( (FieldVariable*) NamedObject_Register_GetByName( self, fieldVariableName ) ) 

	#define FieldVariable_Register_GetByIndex( self, fieldVariableIndex ) \
		( (FieldVariable*) NamedObject_Register_GetByIndex( self, fieldVariableIndex ) )

	#define FieldVariable_Register_PrintAllEntryNames NamedObject_Register_PrintAllEntryNames

#endif /* __Discretisation_Utils_FieldVariable_Register_h__ */

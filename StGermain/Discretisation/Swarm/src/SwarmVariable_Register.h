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
**	Allows users to access SwarmVariables based on their textual name,
**	or index.
**
** Assumptions:
**
** Comments:
**
**
** $Id: SwarmVariable_Register.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Swarm_SwarmVariable_Register_h__
#define __Discretisation_Swarm_SwarmVariable_Register_h__
	
	
	extern const Type SwarmVariable_Register_Type;
	
	#define __SwarmVariable_Register \
		/* General info */ \
		__NamedObject_Register \
		\
		/* Virtual info */ \
		\
		/* Stg_Class info */ \
		Variable_Register* variable_Register;

	struct SwarmVariable_Register { __SwarmVariable_Register };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructor
	*/
	
	SwarmVariable_Register*	SwarmVariable_Register_New( Variable_Register* variable_Register );
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** General virtual functions
	*/
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/
	#define SwarmVariable_Register_Add NamedObject_Register_Add

	#define SwarmVariable_Register_GetIndex NamedObject_Register_GetIndex 

	#define SwarmVariable_Register_GetByName( self, swarmVariableName ) \
		( (SwarmVariable*) NamedObject_Register_GetByName( self, swarmVariableName ) ) 

	#define SwarmVariable_Register_GetByIndex( self, swarmVariableIndex ) \
		( (SwarmVariable*) NamedObject_Register_GetByIndex( self, swarmVariableIndex ) )

	#define SwarmVariable_Register_PrintAllEntryNames NamedObject_Register_PrintAllEntryNames

#endif /* __Discretisation_Swarm_SwarmVariable_Register_h__ */

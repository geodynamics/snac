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
**
**
** Assumptions:
**
** Comments:
**
** $Id: Swarm_Register.h 2745 2005-05-10 08:12:18Z RaquibulHassan $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifndef __Discretisation_Swarm_Swarm_Register_h__
#define __Discretisation_Swarm_Swarm_Register_h__

	extern const Type Swarm_Register_Type;
	extern Swarm_Register* stgSwarm_Register;		/**< Swarm register singleton. */
	
	#define __Swarm_Register \
		/* General info */ \
		__Stg_Class \
		\
		/* Virtual info */ \
		\
		/* Class info */ \
		Stg_ObjectList*		swarmList;
		
	struct Swarm_Register { __Swarm_Register };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructor
	*/
	/* Creation implementation */
	Swarm_Register* _Swarm_Register_New( 
		SizeT					_sizeOfSelf, 
		Type					type,
		Stg_Class_DeleteFunction*		_delete,
		Stg_Class_PrintFunction*		_print, 
		Stg_Class_CopyFunction*			_copy
		);
	
	Swarm_Register* Swarm_Register_New();

	void _Swarm_Register_Init( Swarm_Register* self );
	
	void Swarm_Register_Init( Swarm_Register* self );

	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _Swarm_Register_Delete( void* swarmRegister );
	
	void _Swarm_Register_Print( void* swarmRegister, Stream* stream );
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Functions
	*/

	Index Swarm_Register_Add( Swarm_Register* self, void* swarm );
	
	Swarm* Swarm_Register_Get( Swarm_Register* self, Name name );
	Swarm* Swarm_Register_At( void* swarmRegister, Index index ) ;
	
	unsigned int Swarm_Register_GetCount( Swarm_Register* self );
	Index Swarm_Register_GetIndexFromPointer( Swarm_Register* self, void* ptr );
	Swarm_Register* Swarm_Register_GetSwarm_Register();

	void Swarm_Register_SaveAllRegisteredSwarms( Swarm_Register* self, void* context );

#endif /* __Discretisation_Swarm_Swarm_Register_h__ */



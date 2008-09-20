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
**	$Id: SwarmDump.h 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Swarm_SwarmDump_h__
#define __Discretisation_Swarm_SwarmDump_h__

	/** Textual name of this class */
	extern const Type SwarmDump_Type;
	
	/** SwarmDump contents */
	#define __SwarmDump \
		/* General info */ \
		__Stg_Component \
		\
		/* Virtual info */ \
		\
		/* Member info */ \
		Swarm**                                            swarmList;             \
		Index                                              swarmCount;            \
		Bool                                               newFileEachTime;

	struct SwarmDump { __SwarmDump };	

	/** Creation implementation */
	SwarmDump* SwarmDump_New(		
		Name                                               name,
		void*                                              context,
		Swarm**                                            swarmList,
		Index                                              swarmCount,
		Bool                                               newFileEachTime );
	
	SwarmDump* _SwarmDump_New(
		SizeT                                              _sizeOfSelf, 
		Type                                               type,
		Stg_Class_DeleteFunction*	                       _delete,
		Stg_Class_PrintFunction*	                       _print, 
		Stg_Class_CopyFunction*	                           _copy, 
		Stg_Component_DefaultConstructorFunction*          _defaultConstructor,
		Stg_Component_ConstructFunction*                   _construct,
		Stg_Component_BuildFunction*                       _build,
		Stg_Component_InitialiseFunction*                  _initialise,
		Stg_Component_ExecuteFunction*                     _execute,
		Stg_Component_DestroyFunction*                     _destroy,
		Name                                               name );

	/** Member initialisation implementation */
	void _SwarmDump_Init( 
		SwarmDump*                                         self,
		void*                                              context,
		Swarm**                                            swarmList,
		Index                                              swarmCount,
		Bool                                               newFileEachTime );
	
	void _SwarmDump_Delete( void* swarmDump ) ;
	void _SwarmDump_Print( void* _swarmDump, Stream* stream ) ;
	void* _SwarmDump_Copy( void* swarmDump, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	#define SwarmDump_Copy( self ) \
		(SwarmDump*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	
	void* _SwarmDump_DefaultNew( Name name );
	void _SwarmDump_Construct( void* swarmDump, Stg_ComponentFactory* cf, void* data ) ;
	void _SwarmDump_Build( void* swarmDump, void* data ) ;
	void _SwarmDump_Initialise( void* swarmDump, void* data ) ;
	void _SwarmDump_Execute( void* swarmDump, void* data ) ;
	void _SwarmDump_Destroy( void* swarmDump, void* data ) ;

	/** Virtual Function Wrappers */
	void SwarmDump_Execute( void* swarmDump, void* context ) ;

#endif /* __Discretisation_Swarm_SwarmDump_h__ */

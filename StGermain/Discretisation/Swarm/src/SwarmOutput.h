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
**	$Id: SwarmOutput.h 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Swarm_SwarmOutput_h__
#define __Discretisation_Swarm_SwarmOutput_h__

	/** Textual name of this class */
	extern const Type SwarmOutput_Type;
	
	typedef void    (SwarmOutput_PrintHeaderFunction) ( void* swarmOutput, Stream* stream, Particle_Index lParticle_I, void* data );
	typedef void    (SwarmOutput_PrintDataFunction) ( void* swarmOutput, Stream* stream, Particle_Index lParticle_I, void* data );
	
	/** SwarmOutput contents */
	#define __SwarmOutput \
		/* General info */ \
		__Stg_Component \
		\
		/* Virtual info */ \
		SwarmOutput_PrintHeaderFunction*                   _printHeader;          \
		SwarmOutput_PrintDataFunction*                     _printData;            \
		\
		/* Member info */ \
		Swarm*                                             swarm;                 \
		Name                                               baseFilename;          \
		ExtensionInfo_Index                                handle;                \
		Variable*                                          globalIndexVariable;   \
		Index                                              columnWidth;           \
		char*                                              borderString;          \
		char*                                              doubleFormatString;    \

	struct SwarmOutput { __SwarmOutput };	

	/** Creation implementation */
	SwarmOutput* SwarmOutput_New(		
		Name                                               name,
		void*                                              context,
		Swarm*                                             swarm,
		Name                                               baseFilename,
		unsigned int                                       columnWidth, 
		unsigned int                                       decimalLength,
		char*                                              borderString );		
	
	SwarmOutput* _SwarmOutput_New(
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
		SwarmOutput_PrintHeaderFunction*                   _printHeader,		
		SwarmOutput_PrintDataFunction*                     _printData,
		Name                                               name );

	/** Member initialisation implementation */
	void _SwarmOutput_Init( 
		SwarmOutput*                                       self,
		void*                                              context,
		Swarm*                                             swarm,
		Name                                               baseFilename,
		unsigned int                                       columnWidth, 
		unsigned int                                       decimalLength,
		char*                                              borderString );
	
	void _SwarmOutput_Delete( void* swarmOutput ) ;
	void _SwarmOutput_Print( void* _swarmOutput, Stream* stream ) ;
	void* _SwarmOutput_Copy( void* swarmOutput, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	#define SwarmOutput_Copy( self ) \
		(SwarmOutput*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	
	void* _SwarmOutput_DefaultNew( Name name );
	void _SwarmOutput_Construct( void* swarmOutput, Stg_ComponentFactory* cf, void* data ) ;
	void _SwarmOutput_Build( void* swarmOutput, void* data ) ;
	void _SwarmOutput_Initialise( void* swarmOutput, void* data ) ;
	void _SwarmOutput_Execute( void* swarmOutput, void* data ) ;
	void _SwarmOutput_Destroy( void* swarmOutput, void* data ) ;

	void _SwarmOutput_PrintHeader( void* swarmOutput, Stream* stream, Particle_Index lParticle_I, void* data ) ;
	void _SwarmOutput_PrintData( void* swarmOutput, Stream* stream, Particle_Index lParticle_I, void* data ) ;
	
	/** Virtual Function Wrappers */
	void SwarmOutput_Execute( void* swarmOutput, void* context ) ;
	void SwarmOutput_PrintHeader( void* swarmOutput, Stream* stream, Particle_Index lParticle_I, void* context ) ;
	void SwarmOutput_PrintData( void* swarmOutput, Stream* stream, Particle_Index lParticle_I, void* context ) ;

	void _SwarmOutput_SetGlobalIndicies( void* swarmOutput, void* data ) ;
	void SwarmOutput_PrintString( void* swarmOutput, Stream* stream, char* string ) ;
	void SwarmOutput_PrintDouble( void* swarmOutput, Stream* stream, double value ) ;
	#define SwarmOutput_PrintValue( swarmOutput, stream, value )\
		SwarmOutput_PrintDouble( swarmOutput, stream, (double) ( value ) )

#endif /* __Discretisation_Swarm_SwarmOutput_h__ */

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
**	Instantiates the ParticleLayout abstract class to a manually distributed particle layout.
**
** Assumptions:
**	Cell is a right-angled cuboid.
**
** Comments:
**
** $Id: FileParticleLayout.h 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Swarm_FileParticleLayout_h__
#define __Discretisation_Swarm_FileParticleLayout_h__
	

	/* Textual name of this class */
	extern const Type FileParticleLayout_Type;
	
	/* FileParticleLayout information */
	#define __FileParticleLayout \
		__GlobalParticleLayout \
		\
		Name                                             filename;    \
		FILE*                                            file;        \
		Stream*                                          errorStream;

	struct FileParticleLayout { __FileParticleLayout };
	
	/* Create a new FileParticleLayout and initialise */
	FileParticleLayout* FileParticleLayout_New( Name name, Name filename );

	/* Creation implementation / Virtual constructor */
	FileParticleLayout* _FileParticleLayout_New( 
		SizeT                                            _sizeOfSelf,
		Type                                             type,
		Stg_Class_DeleteFunction*                        _delete,
		Stg_Class_PrintFunction*                         _print,
		Stg_Class_CopyFunction*                          _copy, 
		Stg_Component_DefaultConstructorFunction*        _defaultConstructor,
		Stg_Component_ConstructFunction*                 _construct,
		Stg_Component_BuildFunction*                     _build,
		Stg_Component_InitialiseFunction*                _initialise,
		Stg_Component_ExecuteFunction*                   _execute,
		Stg_Component_DestroyFunction*                   _destroy,
		ParticleLayout_SetInitialCountsFunction*         _setInitialCounts,
		ParticleLayout_InitialiseParticlesFunction*      _initialiseParticles,
		GlobalParticleLayout_InitialiseParticleFunction* _initialiseParticle,
		Name                                             name,
		Bool                                             initFlag,
		Name                                             filename );
	
	void _FileParticleLayout_Init( void* particleLayout, Name filename );
	
	/* 'Stg_Class' Stuff */
	void _FileParticleLayout_Delete( void* particleLayout );
	void _FileParticleLayout_Print( void* particleLayout, Stream* stream );
	#define FileParticleLayout_Copy( self ) \
		(FileParticleLayout*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define FileParticleLayout_DeepCopy( self ) \
		(FileParticleLayout*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _FileParticleLayout_Copy( void* particleLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/* 'Stg_Component' Stuff */
	void* _FileParticleLayout_DefaultNew( Name name ) ;
	void _FileParticleLayout_Construct( void* particleLayout, Stg_ComponentFactory *cf, void* data );
	void _FileParticleLayout_Build( void* particleLayout, void* data );
	void _FileParticleLayout_Initialise( void* particleLayout, void* data );
	void _FileParticleLayout_Execute( void* particleLayout, void* data );
	void _FileParticleLayout_Destroy( void* particleLayout, void* data );
	
	void _FileParticleLayout_SetInitialCounts( void* particleLayout, void* _swarm ) ;
	void _FileParticleLayout_InitialiseParticles( void* particleLayout, void* _swarm ) ;
	void _FileParticleLayout_InitialiseParticle( void* particleLayout, void* swarm, Particle_Index newParticle_I, void* particle);

#endif /* __Discretisation_Swarm_FileParticleLayout_h__ */

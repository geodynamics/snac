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
** Invariants:
**
** Comments:
**
** $Id: MeshGenerator.h 3584 2006-05-16 11:11:07Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisaton_Mesh_MeshGenerator_h__
#define __Discretisaton_Mesh_MeshGenerator_h__

	/** Textual name of this class */
	extern const Type MeshGenerator_Type;

	/** Virtual function types */
	typedef void (MeshGenerator_GenerateFunc)( void* meshGenerator, void* _mesh );

	/** MeshGenerator class contents */
	#define __MeshGenerator					\
		/* General info */				\
		__Stg_Component					\
								\
		/* Virtual info */				\
		MeshGenerator_GenerateFunc*	generateFunc;	\
								\
		/* MeshGenerator info */			\
		MPI_Comm			comm;		\
		unsigned			nMeshes;	\
		Mesh**				meshes;

	struct MeshGenerator { __MeshGenerator };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/

	#define MESHGENERATOR_DEFARGS				\
		STG_COMPONENT_DEFARGS,				\
		MeshGenerator_GenerateFunc*	generateFunc

	#define MESHGENERATOR_PASSARGS			\
		STG_COMPONENT_PASSARGS, generateFunc

	MeshGenerator* _MeshGenerator_New( MESHGENERATOR_DEFARGS );
	void _MeshGenerator_Init( MeshGenerator* self );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _MeshGenerator_Delete( void* meshGenerator );
	void _MeshGenerator_Print( void* meshGenerator, Stream* stream );

	#define MeshGenerator_Copy( self ) \
		(Mesh*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define MeshGenerator_DeepCopy( self ) \
		(Mesh*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _MeshGenerator_Copy( void* meshGenerator, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );

	void _MeshGenerator_Construct( void* meshGenerator, Stg_ComponentFactory* cf, void* data );
	void _MeshGenerator_Build( void* meshGenerator, void* data );
	void _MeshGenerator_Initialise( void* meshGenerator, void* data );
	void _MeshGenerator_Execute( void* meshGenerator, void* data );
	void _MeshGenerator_Destroy( void* meshGenerator, void* data );

#ifndef NDEBUG
	#define MeshGenerator_Generate( self, mesh )			\
		(assert( self ), (self)->generateFunc( self, mesh ))
#else
	#define MeshGenerator_Generate( self, mesh )	\
		(self)->generateFunc( self, mesh )
#endif

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	void MeshGenerator_SetComm( void* meshGenerator, MPI_Comm comm );
	void MeshGenerator_AddMesh( void* meshGenerator, void* mesh );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

#endif /* __Discretisaton_Mesh_MeshGenerator_h__ */

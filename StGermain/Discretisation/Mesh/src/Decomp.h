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
** $Id: Decomp.h 3584 2006-05-16 11:11:07Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisaton_Mesh_Decomp_h__
#define __Discretisaton_Mesh_Decomp_h__

	/** Textual name of this class */
	extern const Type Decomp_Type;

	/** Virtual function types */

	/** Class contents */
	#define __Decomp				\
		/* General info */			\
		__Stg_Component				\
							\
		/* Virtual info */			\
							\
		/* Decomp info */			\
		MPI_Comm		comm;		\
							\
		unsigned		nGlobals;	\
		unsigned		nLocals;	\
		unsigned*		locals;		\
							\
		UIntMap*		glMap;		\
							\
		unsigned		nSyncs;		\
		Decomp_Sync**		syncs;

	struct Decomp { __Decomp };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/

	#define DECOMP_DEFARGS		\
		STG_COMPONENT_DEFARGS

	#define DECOMP_PASSARGS		\
		STG_COMPONENT_PASSARGS

	Decomp* Decomp_New( Name name );
	Decomp* _Decomp_New( DECOMP_DEFARGS );
	void _Decomp_Init( Decomp* self );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _Decomp_Delete( void* decomp );
	void _Decomp_Print( void* decomp, Stream* stream );

	#define Decomp_Copy( self ) \
		(Mesh*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define Decomp_DeepCopy( self ) \
		(Mesh*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _Decomp_Copy( void* decomp, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );

	void _Decomp_Construct( void* decomp, Stg_ComponentFactory* cf, void* data );
	void _Decomp_Build( void* decomp, void* data );
	void _Decomp_Initialise( void* decomp, void* data );
	void _Decomp_Execute( void* decomp, void* data );
	void _Decomp_Destroy( void* decomp, void* data );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	void Decomp_SetComm( void* decomp, MPI_Comm comm );
	void Decomp_SetLocals( void* decomp, unsigned nLocals, unsigned* locals );
	void Decomp_AddSync( void* decomp, Decomp_Sync* sync );
	void Decomp_RemoveSync( void* decomp, Decomp_Sync* sync );

	Bool Decomp_IsLocal( void* decomp, unsigned global );
	unsigned Decomp_GlobalToLocal( void* decomp, unsigned global );
	unsigned Decomp_LocalToGlobal( void* decomp, unsigned local );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

	void Decomp_BuildGLMap( Decomp* self );
	void Decomp_ValidateDomain( Decomp* self, unsigned* status );
	void Decomp_Destruct( Decomp* self );
	void Decomp_DestructLocals( Decomp* self );
	void Decomp_DestructSyncs( Decomp* self );

#endif /* __Discretisaton_Mesh_Decomp_h__ */

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
** $Id: Decomp_Sync.h 3584 2006-05-16 11:11:07Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisaton_Mesh_Decomp_Sync_h__
#define __Discretisaton_Mesh_Decomp_Sync_h__

	/** Textual name of this class */
	extern const Type Decomp_Sync_Type;

	/** Virtual function types */

	/** Mesh class contents */
	typedef struct {
		void*		snkArray;
		unsigned	snkStride;
		unsigned*	snkDisps;
		unsigned*	snkSizes;
		unsigned*	snkOffs;

		void*		srcArray;
		unsigned	srcStride;
		unsigned*	srcDisps;
		unsigned*	srcSizes;
		unsigned*	srcOffs;

		size_t		itemSize;
	} Decomp_Sync_Array;

	#define __Decomp_Sync				\
		/* General info */			\
		__Stg_Component				\
							\
		/* Virtual info */			\
							\
		/* Decomp_Sync info */			\
		Decomp*			decomp;		\
		CommTopology*		commTopo;	\
		RangeSet**		isects;		\
							\
		Decomp_Sync_Claim*	claim;		\
		Decomp_Sync_Negotiate*	negotiate;	\
							\
		unsigned		nRemotes;	\
		unsigned*		remotes;	\
		unsigned		nShared;	\
		unsigned*		shared;		\
		unsigned*		nSharers;	\
		unsigned**		sharers;	\
							\
		UIntMap*		grMap;		\
		UIntMap*		dsMap;		\
							\
		unsigned		netSrcs;	\
		unsigned*		nSrcs;		\
		unsigned**		srcs;		\
		unsigned		netSnks;	\
		unsigned*		nSnks;		\
		unsigned**		snks;		\
							\
		unsigned		nArrays;	\
		Decomp_Sync_Array**	arrays;

	struct Decomp_Sync { __Decomp_Sync };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/

	#define DECOMP_SYNC_DEFARGS	\
		STG_COMPONENT_DEFARGS

	#define DECOMP_SYNC_PASSARGS	\
		STG_COMPONENT_PASSARGS

	Decomp_Sync* Decomp_Sync_New( Name name );
	Decomp_Sync* _Decomp_Sync_New( DECOMP_SYNC_DEFARGS );
	void _Decomp_Sync_Init( Decomp_Sync* self );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _Decomp_Sync_Delete( void* sync );
	void _Decomp_Sync_Print( void* sync, Stream* stream );

	#define Decomp_Sync_Copy( self ) \
		(Mesh*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define Decomp_Sync_DeepCopy( self ) \
		(Mesh*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _Decomp_Sync_Copy( void* sync, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );

	void _Decomp_Sync_Construct( void* sync, Stg_ComponentFactory* cf, void* data );
	void _Decomp_Sync_Build( void* sync, void* data );
	void _Decomp_Sync_Initialise( void* sync, void* data );
	void _Decomp_Sync_Execute( void* sync, void* data );
	void _Decomp_Sync_Destroy( void* sync, void* data );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	void Decomp_Sync_SetDecomp( void* sync, Decomp* decomp );
	void Decomp_Sync_SetClaim( void* sync, Decomp_Sync_Claim* claim );
	void Decomp_Sync_SetNegotiate( void* sync, Decomp_Sync_Negotiate* negotiate );
	void Decomp_Sync_SetRemotes( void* sync, unsigned nRemotes, unsigned* remotes );
	void Decomp_Sync_Decompose( void* sync, unsigned nRequired, unsigned* required );

	unsigned Decomp_Sync_GetDomainSize( void* sync );
	Bool Decomp_Sync_IsDomain( void* sync, unsigned global );
	Bool Decomp_Sync_IsRemote( void* sync, unsigned domain );
	Bool Decomp_Sync_IsShared( void* sync, unsigned domain );
	unsigned Decomp_Sync_GlobalToDomain( void* sync, unsigned global );
	unsigned Decomp_Sync_DomainToGlobal( void* sync, unsigned domain );
	unsigned Decomp_Sync_DomainToShared( void* sync, unsigned domain );
	unsigned Decomp_Sync_SharedToDomain( void* sync, unsigned shared );

	Decomp_Sync_Array* Decomp_Sync_AddArray( void* sync, void* localArray, void* remoteArray, 
						 size_t localStride, size_t remoteStride, size_t itemSize );
	void Decomp_Sync_RemoveArray( void* sync, Decomp_Sync_Array* array );
	void Decomp_Sync_Sync( void* sync );
	void Decomp_Sync_SyncArray( void* self, Decomp_Sync_Array* array );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

	void Decomp_Sync_BuildIntersections( Decomp_Sync* self, unsigned nIndices, unsigned* indices );
	void Decomp_Sync_BuildShared( Decomp_Sync* self );
	void Decomp_Sync_BuildGRMap( Decomp_Sync* self );

	void Decomp_Sync_BuildArray( Decomp_Sync* self, Decomp_Sync_Array* array );
	void Decomp_Sync_Destruct( Decomp_Sync* self );
	void Decomp_Sync_DestructRemotes( Decomp_Sync* self );
	void Decomp_Sync_DestructArrays( Decomp_Sync* self );
	void Decomp_Sync_DestructArray( Decomp_Sync_Array* array );

#endif /* __Discretisaton_Mesh_Decomp_Sync_h__ */

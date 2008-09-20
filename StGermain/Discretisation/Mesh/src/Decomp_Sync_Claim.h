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
** $Id: Decomp_Sync_Claim.h 3584 2006-05-16 11:11:07Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisaton_Mesh_Decomp_Sync_Claim_h__
#define __Discretisaton_Mesh_Decomp_Sync_Claim_h__

	/** Textual name of this class */
	extern const Type Decomp_Sync_Claim_Type;

	/** Virtual function types */
	typedef void (Decomp_Sync_Claim_SelectFunc)( void* claim, Decomp_Sync* sync, unsigned nRequired, unsigned* required, 
						     unsigned* nLocals, unsigned** locals );

	/** Mesh class contents */
	#define __Decomp_Sync_Claim				\
		/* General info */				\
		__Stg_Component					\
								\
		/* Virtual info */				\
		Decomp_Sync_Claim_SelectFunc*	selectFunc;	\
								\
		/* Decomp_Sync_Claim info */

	struct Decomp_Sync_Claim { __Decomp_Sync_Claim };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/

	#define DECOMP_SYNC_CLAIM_DEFARGS			\
		STG_COMPONENT_DEFARGS,				\
		Decomp_Sync_Claim_SelectFunc*	selectFunc

	#define DECOMP_SYNC_CLAIM_PASSARGS		\
		STG_COMPONENT_PASSARGS, selectFunc

	Decomp_Sync_Claim* Decomp_Sync_Claim_New( Name name );
	Decomp_Sync_Claim* _Decomp_Sync_Claim_New( DECOMP_SYNC_CLAIM_DEFARGS );
	void _Decomp_Sync_Claim_Init( Decomp_Sync_Claim* self );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _Decomp_Sync_Claim_Delete( void* claim );
	void _Decomp_Sync_Claim_Print( void* claim, Stream* stream );

	#define Decomp_Sync_Claim_Copy( self ) \
		(Mesh*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define Decomp_Sync_Claim_DeepCopy( self ) \
		(Mesh*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _Decomp_Sync_Claim_Copy( void* claim, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );

	void _Decomp_Sync_Claim_Construct( void* claim, Stg_ComponentFactory* cf, void* data );
	void _Decomp_Sync_Claim_Build( void* claim, void* data );
	void _Decomp_Sync_Claim_Initialise( void* claim, void* data );
	void _Decomp_Sync_Claim_Execute( void* claim, void* data );
	void _Decomp_Sync_Claim_Destroy( void* claim, void* data );

	void Decomp_Sync_Claim_Select( void* claim, Decomp_Sync* sync, unsigned nRequired, unsigned* required, 
				       unsigned* nLocals, unsigned** locals );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

	void Decomp_Sync_Claim_ClaimOwnership( Decomp_Sync_Claim* self, CommTopology* topo, unsigned nRequired, unsigned* required, 
					       RangeSet* lSet, RangeSet** isects, 
					       unsigned* nLocals, unsigned** locals, unsigned* nRemotes, unsigned** remotes );
	void Decomp_Sync_Claim_BuildIndices( Decomp_Sync_Claim* self, unsigned nRequired, unsigned* required, RangeSet* claimed, 
					     unsigned* nLocals, unsigned** locals, unsigned* nRemotes, unsigned** remotes );

#endif /* __Discretisaton_Mesh_Decomp_Sync_Claim_h__ */

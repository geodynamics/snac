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
** $Id: Decomp_Sync_Negotiate.h 3584 2006-05-16 11:11:07Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisaton_Mesh_Decomp_Sync_Negotiate_h__
#define __Discretisaton_Mesh_Decomp_Sync_Negotiate_h__

	/** Textual name of this class */
	extern const Type Decomp_Sync_Negotiate_Type;

	/** Virtual function types */
	typedef void (Decomp_Sync_Negotiate_SelectFunc)( void* negotiate, Decomp_Sync* decomp );

	/** Mesh class contents */
	#define __Decomp_Sync_Negotiate					\
		/* General info */					\
		__Stg_Component						\
									\
		/* Virtual info */					\
		Decomp_Sync_Negotiate_SelectFunc*	selectFunc;	\
									\
		/* Decomp_Sync_Negotiate info */

	struct Decomp_Sync_Negotiate { __Decomp_Sync_Negotiate };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/

	#define DECOMP_SYNC_NEGOTIATE_DEFARGS				\
		STG_COMPONENT_DEFARGS,					\
		Decomp_Sync_Negotiate_SelectFunc*	selectFunc

	#define DECOMP_SYNC_NEGOTIATE_PASSARGS		\
		STG_COMPONENT_PASSARGS, selectFunc

	Decomp_Sync_Negotiate* Decomp_Sync_Negotiate_New( Name name );
	Decomp_Sync_Negotiate* _Decomp_Sync_Negotiate_New( DECOMP_SYNC_NEGOTIATE_DEFARGS );
	void _Decomp_Sync_Negotiate_Init( Decomp_Sync_Negotiate* self );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _Decomp_Sync_Negotiate_Delete( void* negotiate );
	void _Decomp_Sync_Negotiate_Print( void* negotiate, Stream* stream );

	#define Decomp_Sync_Negotiate_Copy( self ) \
		(Mesh*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define Decomp_Sync_Negotiate_DeepCopy( self ) \
		(Mesh*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _Decomp_Sync_Negotiate_Copy( void* negotiate, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );

	void _Decomp_Sync_Negotiate_Construct( void* negotiate, Stg_ComponentFactory* cf, void* data );
	void _Decomp_Sync_Negotiate_Build( void* negotiate, void* data );
	void _Decomp_Sync_Negotiate_Initialise( void* negotiate, void* data );
	void _Decomp_Sync_Negotiate_Execute( void* negotiate, void* data );
	void _Decomp_Sync_Negotiate_Destroy( void* negotiate, void* data );

	void Decomp_Sync_Negotiate_Select( void* negotiate, Decomp_Sync* decomp );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

	void Decomp_Sync_Negotiate_RemoteSearch( Decomp_Sync_Negotiate* self, Decomp_Sync* decomp, 
						 unsigned** nRemFound, unsigned*** remFound );

#endif /* __Discretisaton_Mesh_Decomp_Sync_Negotiate_h__ */

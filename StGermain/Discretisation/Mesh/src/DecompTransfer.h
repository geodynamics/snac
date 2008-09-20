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
** $Id: DecompTransfer.h 3584 2006-05-16 11:11:07Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisaton_Mesh_DecompTransfer_h__
#define __Discretisaton_Mesh_DecompTransfer_h__

	/** Textual name of this class */
	extern const Type DecompTransfer_Type;

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
	} DecompTransfer_Array;

	#define __DecompTransfer			\
		/* General info */			\
		__Stg_Component				\
							\
		/* Virtual info */			\
							\
		/* DecompTransfer info */		\
		Decomp*			decomps[2];	\
		unsigned		nIncIndices;	\
		unsigned*		incIndices[2];	\
		CommTopology*		commTopo;	\
							\
		unsigned		nLocalInds;	\
		unsigned*		srcLocalInds;	\
		unsigned*		dstLocalInds;	\
							\
		unsigned		netSrcs;	\
		unsigned*		nSrcs;		\
		unsigned**		srcs;		\
		unsigned		netSnks;	\
		unsigned*		nSnks;		\
		unsigned**		snks;		\
							\
		unsigned		nArrays;	\
		DecompTransfer_Array**	arrays;

	struct DecompTransfer { __DecompTransfer };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/

	#define DECOMPTRANSFER_DEFARGS	\
		STG_COMPONENT_DEFARGS

	#define DECOMPTRANSFER_PASSARGS	\
		STG_COMPONENT_PASSARGS

	DecompTransfer* DecompTransfer_New( Name name );
	DecompTransfer* _DecompTransfer_New( DECOMPTRANSFER_DEFARGS );
	void _DecompTransfer_Init( DecompTransfer* self );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _DecompTransfer_Delete( void* decompTransfer );
	void _DecompTransfer_Print( void* decompTransfer, Stream* stream );
	void _DecompTransfer_Construct( void* decompTransfer, Stg_ComponentFactory* cf, void* data );
	void _DecompTransfer_Build( void* decompTransfer, void* data );
	void _DecompTransfer_Initialise( void* decompTransfer, void* data );
	void _DecompTransfer_Execute( void* decompTransfer, void* data );
	void _DecompTransfer_Destroy( void* decompTransfer, void* data );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	void DecompTransfer_SetDecomps( void* decompTransfer, Decomp* fromDecomp, Decomp* toDecomp, UIntMap* mapping );
	DecompTransfer_Array* DecompTransfer_AddArray( void* decompTransfer, void* localArray, void* remoteArray, 
						       size_t localStride, size_t remoteStride, size_t itemSize );
	void DecompTransfer_RemoveArray( void* decompTransfer, DecompTransfer_Array* array );
	void DecompTransfer_Transfer( void* decompTransfer );
	void DecompTransfer_TransferArray( void* decompTransfer, DecompTransfer_Array* array );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

	void DecompTransfer_BuildTables( DecompTransfer* self );
	void DecompTransfer_BuildArray( DecompTransfer* self, DecompTransfer_Array* array );
	void DecompTransfer_Destruct( DecompTransfer* self );
	void DecompTransfer_DestructArrays( DecompTransfer* self );
	void DecompTransfer_DestructArray( DecompTransfer_Array* array );

#endif /* __Discretisaton_Mesh_DecompTransfer_h__ */

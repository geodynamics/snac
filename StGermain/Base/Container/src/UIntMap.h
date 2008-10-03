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
** $Id: UIntMap.h 3584 2006-05-16 11:11:07Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Base_Container_UIntMap_h__
#define __Base_Container_UIntMap_h__

	/** Textual name of this class */
	extern const Type UIntMap_Type;

	/** Virtual function types */

	/** Mesh class contents */
	#define __UIntMap				\
		/* General info */			\
		__Stg_Class				\
							\
		/* Virtual info */			\
							\
		/* UIntMap info */			\
		BTree*		btree;			\
		size_t		size;

	struct UIntMap { __UIntMap };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/

	#define UINTMAP_DEFARGS		\
		STG_CLASS_DEFARGS

	#define UINTMAP_PASSARGS	\
		STG_CLASS_PASSARGS

	UIntMap* UIntMap_New();
	UIntMap* _UIntMap_New( UINTMAP_DEFARGS );
	void _UIntMap_Init( UIntMap* self );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _UIntMap_Delete( void* map );
	void _UIntMap_Print( void* map, Stream* stream );

	#define UIntMap_Copy( self ) \
		(Mesh*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define UIntMap_DeepCopy( self ) \
		(Mesh*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _UIntMap_Copy( void* map, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	void UIntMap_Insert( void* map, unsigned key, unsigned val );
	void UIntMap_Clear( void* map );

	Bool UIntMap_HasKey( void* map, unsigned key );
	unsigned UIntMap_Map( void* map, unsigned key );
	void UIntMap_GetItems( void* map, unsigned* nItems, unsigned** keys, unsigned** values );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

	int UIntMap_DataCompare( void* left, void* right );
	void UIntMap_DataCopy( void** dstData, void* data, SizeT size );
	void UIntMap_DataDelete( void* data );
	void UIntMap_ParseNode( void* data, void* _parseStruct );

#endif /* __Base_Container_UIntMap_h__ */

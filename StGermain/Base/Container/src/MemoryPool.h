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
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/** \file
** <b>Role:</b>
**	Memory Pool class.
**
** <b>Assumptions:</b>
**	The functionality of this class is somewhat limited at this time. The memory pool class
**  can only handle homogeneous objects at the moment. Hence the user has to be careful
**  to make sure to use the pool class for allocating and releasing objects of the same type
**  that it was initialised to handle.
**
** <b>Comments:</b>
**	None
**
** $Id: MemoryPool.h 2087 2005-2-25 02:28:44Z RaquibulHassan $
**
**/

#ifndef __MemoryPool_h__
#define __MemoryPool_h__

	/** Textual name for List class. */
	extern const Type MemoryPool_Type;
	
	typedef struct Chunk_t{
		char *memory;
		unsigned int maxFree;
		unsigned int numFree;
	}Chunk;

	/** \def __List See __List */
	#define __MemoryPool \
		/* General info */ \
		__Stg_Class \
		\
		/* Virtual info */ \
		\
		/* MemoryPool info */ \
		SizeT	elementSize; \
		int		numInitialElements; \
		int		numElements; \
		int		numElementsFree; \
		int		numChunks; \
		int		delta; \
		Chunk	*chunks; \
		char	**pool;

	struct MemoryPool { __MemoryPool };
	
	/** Constructor interface. */
	#define MemoryPool_New( type, numElements, delta )\
		MemoryPool_NewFunc( sizeof(type), numElements, delta )
	
	MemoryPool* MemoryPool_NewFunc( SizeT elementSize, int numElements, int delta );

	MemoryPool* _MemoryPool_New(
				SizeT							_sizeOfSelf,
				Type							type,
				Stg_Class_DeleteFunction*			_delete,
				Stg_Class_PrintFunction*			_print,
				Stg_Class_CopyFunction*				_copy,
				int									elementSize,
				int									numElements,
				int									delta
				);

	
	/** Init interface. */
	void *MemoryPool_Init( MemoryPool* self );
	
	void _MemoryPool_Init( MemoryPool* self );
	
	/** Stg_Class_Delete interface. */
		/** Stg_Class delete function */
	void _MemoryPool_DeleteFunc ( void *memPool );
	
	/** Print interface. */
		/** Stg_Class print function */
	void _MemoryPool_PrintFunc ( void *memPool, Stream* stream );

	/** Public functions */
#define MemoryPool_NewObject( type, memPool ) \
	(type*)MemoryPool_NewObjectFunc( sizeof(type), memPool )
	
	void *MemoryPool_NewObjectFunc( SizeT elementSize, MemoryPool *memPool );
		
	Bool MemoryPool_DeleteObject( MemoryPool *memPool, void *object );

	void MemoryPool_Extend( MemoryPool *memPool );
	
	void MemPool_Shrink( MemoryPool *memPool );
	
	/** Private Functions */
	
#endif /* __MemoryPool_h__ */


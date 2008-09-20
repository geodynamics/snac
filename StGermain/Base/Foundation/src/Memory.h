/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003, Victorian Partnership for Advanced Computing (VPAC) Ltd, 110 Victoria Street, Melbourne, 3053 Australia.
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
**	A memory management module that allows analysis of memory usage to a fidelity chosen by the developer,
**	much like the Journal allows for streaming.
**
** <b>Assumptions</b>
**	The memory module is an itegral part of the Foundation module and will only begin operation after Foundation_Init() is
**	called. Likewise, the memory module will stop operation after Foundation_Finalise() is called.
**
**	Using Memory module functions outside while it is not in operation or disabled will most likely result in a segmentation
**	fault.
**
** <b>Comments</b>
**	Currently, the memory module is NOT thread safe. Future revisions of the module may possibly include this 
**	functionality.
**
** <b>Description</b>
**
**	The memory module is designed to extend traditional C malloc() and free() functions to include statistical recording 
**	and array allocation facilities. It contains a number of different functions which are used to replace malloc() and 
**	free(). Typically these functions accept additional parameters regarding statistical information.
**
**	Allocations are organised into <b>types</b> - a textual value, indicating the type of the data object being 
**	allocated. Each type, can be further sub-grouped into <b>names</b>, which is a textual tag to indicate  
**	particular instances or member variables. These are added as part of the allocation routine.
**
**	It also includes information such as the file, function and line number the allocation was made from, to allow easy
**	debugging.
**
**	The memory module provides facilities to perform general types of allocations - objects and arrays from 1D to 4D.
**	For performance reasons, a set of allocation functions (2D to 4D) can be allocated as a 1D array, and used via Access
**	macros. To conserve space, Complex arrays (2D to 3D) can also be created. Complex arrays basically allow the second
**	(and third) dimensions to have varying lengths. For example, a 2D complex array may have 3 rows, where the length of each
**	row is 4, 2 and 3 respectively.
**
**	Please visit the twiki page for details on how to use this module.
**
**	http://csd.vpac.org/twiki/bin/view/Stgermain/MemoryStats
**
** $Id: Memory.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**/

#ifndef __Base_Foundation_Memory_h__
#define __Base_Foundation_Memory_h__


	/** Textual name for Memory class. */
	extern const Type Memory_Type;

	/** Memory module singleton. */
	extern Memory* stgMemory;

	
	/**\def __Memory See Memory. */
	#define __Memory \
		Bool			enable;		/**< Run-time flag for enabling/disabling of statistics recording. */ \
		MemoryOpStamp		stamp;		/**< A "time"/operation stamp of the things done */ \
		MemoryField*		types;		/**< Contains all types, names are a sub field of types. */ \
		MemoryField*		files;		/**< Contains all files, functions are a field set of functions. */ \
		Index			ptrCount;	/**< Current number of pointers in array. */\
		Index			ptrSize;	/**< Size of the pointers array. */ \
		struct	BTree*		pointers;	/**< Large array listing all allocations made. */ \
		Pointer			ptrCache;	/**< A cache to speed up localised searches. */ \
		MemoryPointer*		memCache;	/**< The MemoryPointer associated with ptrCache. */ \
		void*			infoStream;	/**< A Stream object where general output will be directed to. */ \
		void*			errorStream;	/**< A Stream object where errors from by the Memory module is sent. */ \
		void*			debugStream;	/**< A Stream object where debuging info from the Memory module is send.*/ \
		unsigned long stgPeakMemory; \
		unsigned long stgCurrentMemory;
	struct Memory { __Memory };


	/** Creates a Memory instance. */
	Memory* Memory_Init();
	
	/** Deallocates the Memory singleton. */
	void Memory_Delete();	

	/** Enables/Disables Memory module for statistics recording. */
	void Memory_SetEnable( Bool enable );

	/* See implementations for _Memory_InternalMalloc and _Memory_InternalFree at the bottom of Memory.c */
	/* Replacement for malloc(), to insert MemoryTag in front of every alloc */
	void* _Memory_InternalMalloc( SizeT size );

	/* Replacement for realloc(), to insert MemoryTag in front of every alloc */
	void* _Memory_InternalRealloc( void* ptr, SizeT size );

	/* Replacement for free(), to free a pointer that has a MemoryTag */
	void _Memory_InternalFree( void* ptr );


	/*
	 * Macro interface.
	 *
	 * The Memory module is driven by marcos. This is used to:
	 * - Select which function/macro to use depending on whether module is enabled.
	 * - Insert file, function and line information automatically.
	 *
	 */

	#ifndef MEMORY_STATS
		/** Allocates a single instance of a given primitive or class. */
		#define Memory_Alloc( type, name ) \
			(type*) _Memory_Alloc_Macro( sizeof(type) )			
	#else		
		#define Memory_Alloc( type, name ) \
			(type*) _Memory_Alloc_Func( sizeof(type), #type, (name), __FILE__, __func__, __LINE__ )
	#endif		

	/** Allocates a single instance of a given primitive or class, without assigning a name to the allocation. */
	#define Memory_Alloc_Unnamed( type ) \
		Memory_Alloc( type, Name_Invalid )


	#ifndef MEMORY_STATS
		/** Allocates a given number of bytes, associating the allocation with a textual type and name. */
		#define Memory_Alloc_Bytes( size, typeName, name ) \
			_Memory_Alloc_Macro( (size) )
	#else
		#define Memory_Alloc_Bytes( size, typeName, name ) \
			_Memory_Alloc_Func( (size), (typeName), (name), __FILE__, __func__, __LINE__ )	
	#endif
		
	/** Allocates a given number of bytes, associating the allocation with a textual type only. */
	#define Memory_Alloc_Bytes_Unnamed( size, typeName ) \
		Memory_Alloc_Bytes( (size), (typeName), Name_Invalid )


	#ifndef MEMORY_STATS
		/** Allocates a 1D array of a primitive or class. */
		#define Memory_Alloc_Array( type, arrayLength, name ) \
			(type*) _Memory_Alloc_Array_Macro( sizeof(type), arrayLength )
	#else
		#define Memory_Alloc_Array( type, arrayLength, name ) \
			(type*) _Memory_Alloc_Array_Func( sizeof(type), (arrayLength), #type, (name), __FILE__, __func__, __LINE__ )
	#endif

	/** Allocates a 1D array of a primitive or class without a name. */
	#define Memory_Alloc_Array_Unnamed( type, arrayLength ) \
		Memory_Alloc_Array( type, (arrayLength), Name_Invalid )


	#ifndef MEMORY_STATS		
		/** Allocates a 1D array of items each of itemSize number of bytes. Allocation is given a textual type and name. */
		#define Memory_Alloc_Array_Bytes( itemSize, arrayLength, typeName, name ) \
			_Memory_Alloc_Array_Macro( (itemSize), (arrayLength) )
	#else
		#define Memory_Alloc_Array_Bytes( itemSize, arrayLength, typeName, name ) \
			_Memory_Alloc_Array_Func( (itemSize), (arrayLength), (typeName), (name), __FILE__, __func__, __LINE__ )
	#endif

	/** Allocates a 1D array of items each of itemSize number of bytes. Allocation is given a textual type only. */
	#define Memory_Alloc_Array_Bytes_Unnamed( itemSize, arrayLength, typeName ) \
		Memory_Alloc_Array_Bytes( (itemSize), (arrayLength), (typeName), Name_Invalid )


	/** Allocates a 2D array of any primative or class. */
	#define Memory_Alloc_2DArray( type, xLength, yLength, name ) \
		(type**) _Memory_Alloc_2DArray_Func( sizeof(type), (xLength), (yLength), #type, (name), __FILE__, __func__, __LINE__ )

	/** Allocates a 2D array of any primative or class without a name. */
	#define Memory_Alloc_2DArray_Unnamed( type, xLength, yLength ) \
		Memory_Alloc_2DArray( type, (xLength), (yLength), Name_Invalid )
		

	/** Allocates a 3D array of a primative or class. */
	#define Memory_Alloc_3DArray( type, xLength, yLength, zLength, name ) \
		(type***) _Memory_Alloc_3DArray_Func \
			( sizeof(type), (xLength), (yLength), (zLength), #type, (name), __FILE__, __func__, __LINE__ )
	
	/** Allocates a 3D array of a primative or class without a name. */
	#define Memory_Alloc_3DArray_Unnamed( type, xLength, yLength, zLength ) \
		Memory_Alloc_3DArray( type, (xLength), (yLength), (zLength), Name_Invalid )
		

	/** Allocates a 4D array of a primative or class. */
	#define Memory_Alloc_4DArray( type, xLength, yLength, zLength, wLength, name ) \
		(type****) _Memory_Alloc_4DArray_Func \
			( sizeof(type), (xLength), (yLength), (zLength), (wLength), #type, (name), __FILE__, __func__, __LINE__ )
	
	/** Allocates a 4D array of a primative or class without a name. */
	#define Memory_Alloc_4DArray_Unnamed( type, xLength, yLength, zLength, wLength ) \
		Memory_Alloc_4DArray( type, (xLength), (yLength), (zLength), (wLength), Name_Invalid )
	

	#ifndef MEMORY_STATS
		/** Allocates a 2D array from a 1D array. Use in conjunction with the Memory_Access2D marco. */
		#define Memory_Alloc_2DArrayAs1D( type, xLength, yLength, name ) \
			(type*) _Memory_Alloc_2DArrayAs1D_Macro( sizeof(type), (xLength), (yLength) )
	#else
		#define Memory_Alloc_2DArrayAs1D( type, xLength, yLength, name ) \
			(type*) _Memory_Alloc_2DArrayAs1D_Func \
				( sizeof(type), (xLength), (yLength), #type, (name), __FILE__, __func__, __LINE__ );
	#endif
	
	/** Allocates a 2D array from a 1D array without a name. Use in conjunction with the Memory_Access2D marco. */
	#define Memory_Alloc_2DArrayAs1D_Unnamed( type, xLength, yLength ) \
		Memory_Alloc_2DArrayAs1D( type, (xLength), (yLength), Name_Invalid );


	#ifndef MEMORY_STATS
		/** Allocates a 3D array from a 1D array. Use in conjunction with the Memory_Access3D marco. */
		#define Memory_Alloc_3DArrayAs1D( type, xLength, yLength, zLength, name ) \
			(type*) _Memory_Alloc_3DArrayAs1D_Macro( sizeof(type), (xLength), (yLength), (zLength) )
	#else
		#define Memory_Alloc_3DArrayAs1D( type, xLength, yLength, zLength, name ) \
			(type*) _Memory_Alloc_3DArrayAs1D_Func \
				( sizeof(type), (xLength), (yLength), (zLength), #type, (name), __FILE__, __func__, __LINE__ )
	#endif
	
	/** Allocates a 3D array from a 1D array without a name. Use in conjunction with the Memory_Access3D marco. */
	#define Memory_Alloc_3DArrayAs1D_Unnamed( type, xLength, yLength, zLength ) \
		Memory_Alloc_3DArrayAs1D( type, (xLength), (yLength), (zLength), Name_Invalid );

	
	#ifndef MEMORY_STATS
		/** Allocates a 4D array from a 1D array. Use in conjunction with the Memory_Access4D marco. */
		#define Memory_Alloc_4DArrayAs1D( type, xLength, yLength, zLength, wLength, name ) \
			(type*) _Memory_Alloc_4DArrayAs1D_Macro( sizeof(type), (xLength), (yLength), (zLength), (wLength) )
	#else
		#define Memory_Alloc_4DArrayAs1D( type, xLength, yLength, zLength, wLength, name ) \
			(type*) _Memory_Alloc_4DArrayAs1D_Func \
				( sizeof(type), (xLength), (yLength), (zLength), (wLength), #type, (name), __FILE__, __func__, __LINE__ )
	#endif

	/** Allocates a 4D array from a 1D array without a name. Use in conjunction with the Memory_Access4D marco. */
	#define Memory_Alloc_4DArrayAs1D_Unnamed( type, xLength, yLength, zLength, wLength ) \
		Memory_Alloc_4DArrayAs1D( type, (xLength), (yLength), (zLength), (wLength), Name_Invalid );


	/** Allocates a 2D complex array. Each array in the 2nd dimension may have varying lengths.
	 **
	 ** @param yLengths A set of lengths for the 2nd dimension.
	 **/
	#define Memory_Alloc_2DComplex( type, xLength, yLengths, name ) \
		(type**) _Memory_Alloc_2DComplex_Func( sizeof(type), (xLength), (yLengths), #type, (name), __FILE__, __func__, __LINE__ )
	
	/** Allocates a 2D complex array without a name. Each array in the 2nd dimension may have varying lengths.
	 **
	 ** @param yLengths A set of lengths for the 2nd dimension.
	 **/
	#define Memory_Alloc_2DComplex_Unnamed( type, xLength, yLengths ) \
		Memory_Alloc_2DComplex( type, (xLength), (yLengths), Name_Invalid )


	/** Allocates a 2D complex array used to represent 3rd dimension lengths in a 3D complex array.
	 **
	 ** @param yLengths A set of lengths for the 2nd dimension.
	 **/
	#define Memory_Alloc_3DSetup( xLength, yLengths ) \
		_Memory_Alloc_3DSetup_Func( (xLength), (yLengths), __FILE__, __func__, __LINE__ )
	
	
	/** Allocates a 3D complex array. Each array in the 2nd and 3rd dimension may have varying lengths.
	 **
	 ** @param yLengths A set of lengths for the 2nd dimension.
	 ** @param zLengths A matrix of lengths for the 3rd dimension. Allocated by Memory_Alloc_3DSetup.
	 **/
	#define Memory_Alloc_3DComplex( type, xLength, yLengths, zLengths, name ) \
		(type***) _Memory_Alloc_3DComplex_Func \
			( sizeof(type), (xLength), (yLengths), (zLengths), #type, (name), __FILE__, __func__, __LINE__ )

	/** Allocates a 3D complex array without a name. Each array in the 2nd and 3rd dimension may have varying lengths.
	 **
	 ** @param yLengths A set of lengths for the 2nd dimension.
	 ** @param zLengths A matrix of lengths for the 3rd dimension. Allocated by Memory_Alloc_3DSetup.
	 **/
	#define Memory_Alloc_3DComplex_Unnamed( type, xLength, yLengths, zLengths ) \
		Memory_Alloc_3DComplex( type, (xLength), (yLengths), (zLengths), Name_Invalid )


	#ifndef MEMORY_STATS
		/** Resizes the bytes of a single object. */
		#define Memory_Realloc( ptr, newSize ) \
			_Memory_Realloc_Macro( (ptr), (newSize) )
	#else
		#define Memory_Realloc( ptr, newSize ) \
			_Memory_Realloc_Func( (ptr), (newSize), Type_Invalid, __FILE__, __func__, __LINE__ )
	#endif

	#ifndef MEMORY_STATS
		/** Resizes a 1D array. */
		#define Memory_Realloc_Array( ptr, type, newLength ) \
			(type*) _Memory_Realloc_Array_Macro( (ptr), sizeof(type), (newLength) )
	#else
		#define Memory_Realloc_Array( ptr, type, newLength ) \
			(type*) _Memory_Realloc_Array_Func( (ptr), sizeof(type), (newLength), #type, __FILE__, __func__, __LINE__ )
	#endif

	#ifndef MEMORY_STATS
		/** Resizes a 1D array of items, where each item is of itemSize bytes. */
		#define Memory_Realloc_Array_Bytes( ptr, itemSize, newLength ) \
			_Memory_Realloc_Array_Macro( (ptr), (itemSize), (newLength) )
	#else
		#define Memory_Realloc_Array_Bytes( ptr, itemSize, newLength ) \
			_Memory_Realloc_Array_Func( (ptr), (itemSize), (newLength), Type_Invalid, __FILE__, __func__, __LINE__ )
	#endif

	/** Resizes a 2D array.
	 ** The integrity of the data is only maintained if enlarging the size of the array.
	 ** Assumes the original dimension of X is greater than 1.
	 **/
	#define Memory_Realloc_2DArray( ptr, type, newX, newY ) \
		(type**) _Memory_Realloc_2DArray_Func \
			( (ptr), sizeof(type), 0, 0, (newX), (newY), #type, __FILE__, __func__, __LINE__ )

	/** Resizes a 3D array.
	 ** The integrity of the data is only maintained if enlarging the size of the array.
	 ** Assumes the original dimension of X and Y is greater than 1.
	 **/
	#define Memory_Realloc_3DArray( ptr, type, newX, newY, newZ ) \
		(type***) _Memory_Realloc_3DArray_Func \
			( (ptr), sizeof(type), 0, 0, 0, (newX), (newY), (newZ), #type, __FILE__, __func__, __LINE__ )

	/** Resizes a 2D array.
	 ** The integrity of the data is only maintained if enlarging the size of the array.
	 ** Safe to used for when X is equal to 1.
	 **/
	#define Memory_Realloc_2DArraySafe( ptr, type, oldX, oldY, newX, newY ) \
		(type**) _Memory_Realloc_2DArray_Func \
			( (ptr), sizeof(type), (oldX), (oldY), (newX), (newY), #type, __FILE__, __func__, __LINE__ )

	/** Resizes a 3D array.
	 ** The integrity of the data is only maintained if enlarging the size of the array.
	 ** Safe to used for when X and/or Y is equal to 1.
	 **/
	#define Memory_Realloc_3DArraySafe( ptr, type, oldX, oldY, oldZ, newX, newY, newZ ) \
		(type***) _Memory_Realloc_3DArray_Func \
			( (ptr), sizeof(type), (oldX), (oldY), (oldZ), (newX), (newY), (newZ), #type, __FILE__, __func__, __LINE__ )
	



	/** Resizes a 2D array in 1D form. */
	#define Memory_Realloc_2DArrayAs1D( ptr, type, oldX, oldY, newX, newY ) \
		(type*) _Memory_Realloc_2DArrayAs1D_Func \
			( (ptr), sizeof(type), (oldX), (oldY), (newX), (newY), #type, __FILE__, __func__, __LINE__ )
		
	/** Resizes a 3D array in 1D form. */
	#define Memory_Realloc_3DArrayAs1D( ptr, type, oldX, oldY, oldZ, newX, newY, newZ ) \
		(type*) _Memory_Realloc_3DArrayAs1D_Func \
			( (ptr), sizeof(type), (oldX), (oldY), (oldZ), (newX), (newY), (newZ), #type, __FILE__, __func__, __LINE__ )

		
	#ifndef MEMORY_STATS
		/** Deallocates the memory of any allocation. */
		#define Memory_Free( ptr ) \
			_Memory_Free_Macro( (ptr) )
	#else
		#define Memory_Free( ptr ) \
			_Memory_Free_Func( (ptr) )
	#endif

	
	/** Displays a summary of the given type and name. */
	#define Memory_Print_Type_Name( type, name ) \
		Memory_Print_Type_Name_Func( #type, (name) );
		
	/** Displays a summary of the given type. */
	#define Memory_Print_Type( type ) \
		Memory_Print_Type_Func( #type );


	/** Deallocates all allocations of a given type and name. Can only be used if Memory Module is enabled. */
	#define Memory_Free_Type_Name( type, name ) \
		_Memory_Free_Type_Name_Func( #type, (name) );

	/** Deallocates all allocations of a given type. Can only be used if Memory Module is enabled. */
	#define Memory_Free_Type( type ) \
		_Memory_Free_Type_Func( #type );

	/** Retrives a value from a 2D array allocated as a 1D array. Use in conjunction with Memory_Alloc_2DArrayAs1D. */
	#define Memory_Access2D( array2D, x, y, yLength ) \
		array2D[ ( (x) * (yLength) ) + (y) ]

	/** Retrives a value from a 3D array allocated as a 1D array. Use in conjunction with Memory_Alloc_3DArrayAs1D. */
	#define Memory_Access3D( array3D, x, y, z, yLength, zLength ) \
		array3D[ ( (x) * (yLength) * (zLength) ) + ( (y) * (zLength) ) + (z) ]

	/** Retrives a value from a 4D array allocated as a 1D array. Use in conjunction with Memory_Alloc_4DArrayAs1D. */
	#define Memory_Access4D( array4D, x, y, z, w, yLength, zLength, wLength ) \
		array4D[ ( (x) * (yLength) * (zLength) * (wLength) ) + ( (y) * (zLength) * (wLength) ) + ( (z) * (wLength) ) + (w) ]


	/** Returns the number of bytes used in a 1D array. */
	SizeT Memory_Length_1DArray( SizeT itemSize, Index length );

	/** Returns the number of bytes used in a 2D array. */
	SizeT Memory_Length_2DArray( SizeT itemSize, Index xLength, Index yLength );
	
	/** Returns the number of bytes used in a 3D array. */
	SizeT Memory_Length_3DArray( SizeT itemSize, Index xLength, Index yLength, Index zLength );
	
	/** Returns the number of bytes used in a 4D array. */
	SizeT Memory_Length_4DArray( SizeT itemSize, Index xLength, Index yLength, Index zLength, Index wLength );

	/** Returns the number of bytes used in a 2D array allocated as 1D. */
	SizeT Memory_Length_2DAs1D( SizeT itemSize, Index xLength, Index yLength );
	
	/** Returns the number of bytes used in a 3D array allocated as 1D. */
	SizeT Memory_Length_3DAs1D( SizeT itemSize, Index xLength, Index yLength, Index zLength );
	
	/** Returns the number of bytes used in a 4D array allocated as 1D. */
	SizeT Memory_Length_4DAs1D( SizeT itemSize, Index xLength, Index yLength, Index zLength, Index wLength );

	/** Returns the number of bytes used in a 2D complex array. */
	SizeT Memory_Length_2DComplex( SizeT itemSize, Index xLength, Index* yLengths );
	
	/** Returns the number of bytes used in a 3D complex array. */
	SizeT Memory_Length_3DComplex( SizeT itemSize, Index xLength, Index* yLengths, Index** zLengths );

	/** Prints a summary by types and names in the system. */
	void Memory_Print_Summary();
	
	/** Prints a summary by types and names in the system. */
	void Memory_Print();
	
	/** Displays allocations which are currently still allocated. */
	void Memory_Print_Leak();
	
	typedef struct Memory_Print_Leak_Helper_Arg {
		Index printOptions;
		Index numLeaks;
		Index warningPrinted;
	} Memory_Print_Leak_Helper_Arg;
	
	void Memory_Print_Leak_Helper( void *memoryPointer, void *args );
	
	/** Displays a summary of the given type. */
	void Memory_Print_Type_Func( Type type );
	
	/** Displays a summary of the given type and name. */
	void Memory_Print_Type_Name_Func( Type type, Name name );
	
	/** Displays a summary of the given file. */
	void Memory_Print_File( char* fileName );
	
	/** Displays a summary of the given file and function. */
	void Memory_Print_File_Function( char* fileName, char* funcName );
	
	/** Displays a single allocation. */
	void Memory_Print_Pointer( void* ptr );


	/** \internal Allocates a single object. */
	void* _Memory_Alloc_Func(
		SizeT size,
		Type type,
		Name name,
		const char* fileName,
		const char* funcName,
		int lineNumber );

	/** \internal Allocates a 1D array. */
	void* _Memory_Alloc_Array_Func(
		SizeT itemSize,
		Index arrayLength,
		Type type,
		Name name,
		const char* fileName,
		const char* funcName,
		int lineNumber );
	
	/** \internal Allocates a 2D array. */
	void* _Memory_Alloc_2DArray_Func(
		SizeT itemSize,
		Index xLength,
		Index yLength,
		Type type,
		Name name,
		const char* fileName,
		const char* funcName,
		int lineNumber );
	
	/** \internal Allocates a 3D Array. */
	void* _Memory_Alloc_3DArray_Func(
		SizeT itemSize,
		Index xLength,
		Index yLength,
		Index zLength,
		Type type,
		Name name,
		const char* fileName,
		const char* funcName,
		int lineNumber );
	
	/** \internal Allocates a 4D Array. */
	void* _Memory_Alloc_4DArray_Func(
		SizeT itemSize,
		Index xLength,
		Index yLength,
		Index zLength,
		Index wLength,
		Type type,
		Name name,
		const char* fileName,
		const char* funcName,
		int lineNumber );

	/** \internal Allocates a 2D array from a 1D array. Use in conjunction with the Memory_Access2D marco. */
	void* _Memory_Alloc_2DArrayAs1D_Func(
		SizeT itemSize,
		Index xLength,
		Index yLength,
		Type type,
		Name name,
		const char* fileName,
		const char* funcName,
		int lineNumber );

	/** \internal Allocates a 3D array from a 1D array. Use in conjunction with the Memory_Access3D marco. */
	void* _Memory_Alloc_3DArrayAs1D_Func(
		SizeT itemSize,
		Index xLength,
		Index yLength,
		Index zLength,
		Type type,
		Name name,
		const char* fileName,
		const char* funcName,
		int lineNumber );

	/** \internal Allocates a 4D array from a 1D array. Use in conjunction with the Memory_Access4D marco. */
	void* _Memory_Alloc_4DArrayAs1D_Func(
		SizeT itemSize,
		Index xLength,
		Index yLength,
		Index zLength,
		Index wLength,
		Type type,
		Name name,
		const char* fileName,
		const char* funcName,
		int lineNumber );

	/** \internal Allocates a 2D complex array. Each array in the 2nd dimension may have varying lengths.
	 **
	 ** @param yLengths A set of lengths for the 2nd dimension.
	 **/
	void* _Memory_Alloc_2DComplex_Func(
		SizeT itemSize,
		Index xLength,
		Index* yLengths,
		Type type,
		Name name,
		const char* fileName,
		const char* funcName,
		int lineNumber );

	/** \internal Allocates a 2D complex array used to represent 3rd dimension lengths in a 3D complex array.
	 **
	 ** @param yLengths A set of lengths for the 2nd dimension.
	 **/
	Index** _Memory_Alloc_3DSetup_Func(
		Index xLength,
		Index* yLengths,
		const char* fileName,
		const char* funcName,
		int lineNumber );

	/** \internal Allocates a 3D complex array. Each array in the 2nd and 3rd dimension may have varying lengths.
	 **
	 ** @param yLengths A set of lengths for the 2nd dimension.
	 ** @param zLengths A matrix of lengths for the 3rd dimension. Allocated by Memory_Alloc_3DSetup.
	 **/
	void* _Memory_Alloc_3DComplex_Func(
		SizeT itemSize,
		Index xLength,
		Index* yLengths,
		Index** zLengths,
		Type type,
		Name name,
		const char* fileName,
		const char* funcName,
		int lineNumber );


	/** \internal Resizes the bytes of a single object. */
	void* _Memory_Realloc_Func(
		void* ptr,
		SizeT newSize,
		Type type,
		const char* fileName,
		const char* funcName,
		int lineNumber );

	/** \internal Resizes a 1D array. */
	void* _Memory_Realloc_Array_Func( 
		void* ptr, 
		SizeT itemSize, 
		Index newLength,
		Type type,
		const char* fileName,
		const char* funcName,
		int lineNumber );

	/** \internal Resizes a 2D array. */
	void* _Memory_Realloc_2DArray_Func(
		void* ptr,
		SizeT itemSize, 
		Index oldX,
		Index oldY,
		Index newX, 
		Index newY,
		Type type,
		const char* fileName,
		const char* funcName,
		int lineNumber );

	/** \internal Resizes a 3D array. */
	void* _Memory_Realloc_3DArray_Func( 
		void* ptr, 
		SizeT itemSize, 
		Index oldX,
		Index oldY,
		Index oldZ,
		Index newX, 
		Index newY, 
		Index newZ,
		Type type,
		const char* fileName,
		const char* funcName,
		int lineNumber );

	/** \internal Resizes a 2D array in 1D form. */
	void* _Memory_Realloc_2DArrayAs1D_Func( 
		void* ptr, 
		SizeT itemSize, 
		Index oldX, 
		Index oldY, 
		Index newX, 
		Index newY,
		Type type,
		const char* fileName,
		const char* funcName,
		int lineNumber );

	/** \internal Resizes a 3D array in 1D form. */
	void* _Memory_Realloc_3DArrayAs1D_Func( 
		void* ptr, 
		SizeT itemSize,
		Index oldX, 
		Index oldY, 
		Index oldZ, 
		Index newX, 
		Index newY, 
		Index newZ,
		Type type,
		const char* fileName,
		const char* funcName,
		int lineNumber );


	/** \internal Frees any allocation. */
	void _Memory_Free_Func( void* ptr );
	

	void _Memory_Free_Type_Name_Func( Type type, Name name );
	
	/** struct for packaging up the arguments of _Memory_Free_Type_Name_Func into a single argument */
	typedef struct Memory_Free_Type_Name_Func_Helper_Arg{
		Type type;
		Name name;
	}Memory_Free_Type_Name_Func_Helper_Arg;
	
	void _Memory_Free_Type_Name_Func_Helper( void *memoryPointer, void *args );
	
	/** \internal Deallocates all allocations of a given type. Can only be used if Memory Module is enabled. */	
	void _Memory_Free_Type_Func( Type type );
	void _Memory_Free_Type_Func_Helper ( void *memoryPointer, void *args );

	/** Queries the Memory module about whether or not the given address has been freed.
	 *  Returns True if allocated, False if freed or not found.
	 *  If memory module is disabled, this function simply returns True.
	 *
	 *  For debugging purposes only.
	 */
	Bool Memory_IsAllocated( void* ptr );

	/*
	 * Macro equivalents
	 *
	 * The following private functions are macro equivalents of some of the Memory module functions.
	 * Thesy are implemented to ensure that when the Memory module is disabled, minimal performance impact
	 * is ensured.
	 *
	 * Functions such as the normal 2D to 4D array and as well as the 2D and 3D complex allocations do not
	 * have macro equivalents due to the difficulty of creation. These operations only have a function implementation.
	 *
	 */

	#define _Memory_Alloc_Macro( size ) \
		_Memory_InternalMalloc( (size) )

	#define _Memory_Alloc_Array_Macro( itemSize, arrayLength ) \
		_Memory_InternalMalloc( (itemSize) * (arrayLength) )
	
	#define _Memory_Alloc_2DArrayAs1D_Macro( itemSize, xLength, yLength ) \
		_Memory_InternalMalloc( (itemSize) * (xLength) * (yLength) )
	
	#define _Memory_Alloc_3DArrayAs1D_Macro( itemSize, xLength, yLength, zLength ) \
		_Memory_InternalMalloc( (itemSize) * (xLength) * (yLength) * (zLength) )

	#define _Memory_Alloc_4DArrayAs1D_Macro( itemSize, xLength, yLength, zLength, wLength ) \
		_Memory_InternalMalloc( (itemSize) * (xLength) * (yLength) * (zLength) * (wLength) )
	
	#define _Memory_Realloc_Macro( ptr, newSize ) \
		_Memory_InternalRealloc( (ptr), (newSize) );

	#define _Memory_Realloc_Array_Macro( ptr, itemSize, newLength ) \
		_Memory_InternalRealloc( (ptr), (itemSize) * (newLength) );

	#define _Memory_Free_Macro( ptr ) \
		_Memory_InternalFree( (ptr) )
		
		
#endif /* __Base_Foundation_Memory_h__ */

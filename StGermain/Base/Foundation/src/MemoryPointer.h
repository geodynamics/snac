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
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/** \file
** <b>Role:</b>
**	Represents an allocated block of memory which is a given to a pointer.
**
** <b>Assumptions</b>
**	Memory Module is enabled.
**
** <b>Comments</b>
**	Do not use this class when the Memory Module is switched off.
**
** <b>Description</b>
**	This class is used to store statistical data of memory blocks allocated by the Memory Module.
**
** $Id: MemoryPointer.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**/

#ifndef __Base_Foundation_MemoryPointer_h__
#define __Base_Foundation_MemoryPointer_h__

	/** Textual name for MemoryPointer class. */
	extern const Type MemoryPointer_Type;

	/** Types of memory allocations in memory module. */
	typedef enum { 
		MEMORY_OBJECT,
		MEMORY_1DARRAY,
		MEMORY_2DARRAY,
		MEMORY_3DARRAY,
		MEMORY_4DARRAY,
		MEMORY_2DAS1D,
		MEMORY_3DAS1D,
		MEMORY_4DAS1D,
		MEMORY_2DCOMPLEX,
		MEMORY_3DSETUP,
		MEMORY_3DCOMPLEX
	} MemoryAllocType;
	
	/** Fields that are printable in MemoryPointer. A Binary Flag. */
	typedef enum {
		MEMORYPOINTER_PTR = 		0x0001,
		MEMORYPOINTER_TYPE = 		0x0002,
		MEMORYPOINTER_NAME = 		0x0004,
		MEMORYPOINTER_FILE = 		0x0008,
		MEMORYPOINTER_FUNC = 		0x0010,
		MEMORYPOINTER_LINE = 		0x0020,
		MEMORYPOINTER_ALLOCTYPE = 	0x0040,
		MEMORYPOINTER_ITEMSIZE = 	0x0080,
		MEMORYPOINTER_TOTALSIZE = 	0x0100,
		MEMORYPOINTER_LENGTH = 		0x0200,
		MEMORYPOINTER_ALL = 
			MEMORYPOINTER_PTR |
			MEMORYPOINTER_TYPE |
			MEMORYPOINTER_NAME |
			MEMORYPOINTER_FILE |
			MEMORYPOINTER_FUNC |
			MEMORYPOINTER_LINE |
			MEMORYPOINTER_ALLOCTYPE |
			MEMORYPOINTER_ITEMSIZE |
			MEMORYPOINTER_TOTALSIZE |
			MEMORYPOINTER_LENGTH
	} MemoryPointerColumn;
	
	typedef struct {
		Index	x;
		Index*	y;
		Index**	z;
	} MemoryPointer_LengthXYZ;
	
	typedef union {
		Index			oneD;
		Index			twoD[2];
		Index			threeD[3];
		Index			fourD[4];
		MemoryPointer_LengthXYZ	xyz;
	} MemoryPointer_Length;
	
	/** \def __MemoryPointer See MemoryPointer. */
	#define __MemoryPointer \
		Pointer			ptr; \
		Pointer			ptrReference; \
		MemoryPointerStatus	status; \
		MemoryOpStamp		stamp; \
		MemoryField*		type; \
		MemoryField*		name; \
		MemoryField*		file; \
		MemoryField*		func; \
		Index			line; \
		MemoryAllocType		allocType; \
		SizeT			itemSize; \
		SizeT			totalSize; \
		MemoryPointer_Length	length;
	struct MemoryPointer { __MemoryPointer };

	/** Constructs a MemoryPointer, registering the MemoryField(s) it is associated with.
	 **
	 ** The length must be manually set. This is because the members of the length union are set
	 ** depending on the allocation type.
	 **/
	MemoryPointer* MemoryPointer_New(
		Pointer ptr,
		MemoryOpStamp stamp,
		const char* type,
		const char* name,
		const char* file,
		const char* func,
		Index line,
		MemoryAllocType allocType,
		SizeT itemSize,
		SizeT totalSize );
	
	/** Initialises a MemoryPointer, registering the MemoryFields it is associated with. */
	void MemoryPointer_Init(
		MemoryPointer* memoryPointer,
		Pointer ptr,
		MemoryOpStamp stamp,
		const char* type,
		const char* name,
		const char* file,
		const char* func,
		Index line,
		MemoryAllocType allocType,
		SizeT itemSize,
		SizeT totalSize );
	
	/** Deletes a MemoryPointer. */
	void MemoryPointer_Delete( MemoryPointer* memoryPointer );
	void MemoryPointer_Delete_Helper( void* memoryPointer, void *args );

	/** Displays the contents of a MemoryPointer.
	 **
	 ** All fields are print at level 2, except for length of 2D and 3D complex arrays, which is printed at level 3.
	 **
	 ** @param columns A Bit flag of The fields to be displayed.
	 **/
	void MemoryPointer_Print( MemoryPointer* memoryPointer, MemoryPointerColumn columns );

		typedef struct MemoryPointer_Print_Type_Name_Func_Helper_Arg{
			Type			type;
			Name			name;
			MemoryPointerColumn	printOptions;
		}MemoryPointer_Print_Type_Name_Func_Helper_Arg;
	void MemoryPointer_Print_Type_Name_Func_Helper( void *memoryPointer, void *args );
	
		typedef struct MemoryPointer_Print_File_Func_Helper_Arg{
			char*			fileName;
			char*			funcName;
			MemoryPointerColumn	printOptions;
		}MemoryPointer_Print_File_Func_Helper_Arg;
	void MemoryPointer_Print_File_Func_Helper( void *memoryPointer, void *args );

	/** Displays all fields of a MemoryPointer. */
	#define MemoryPointer_PrintAll( memoryPointer ) MemoryPointer_Print( memoryPointer, MEMORYPOINTER_ALL );			
			
#endif /* __Base_Foundation_MemoryPointer_h__ */






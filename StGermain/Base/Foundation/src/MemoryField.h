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
**	A generic representation of a field in a MemoryPointer tuple which has a string type.
**
** <b>Assumptions</b>
**	Memory Module is enabled.
**
** <b>Comments</b>
**	Do not use this class when the Memory Module is switched off.
**
** <b>Description</b>
**	An instance of this class may either represent a value in Type, Name, File or Function fields, a container of a set of these
**	fields or both.
**
**	It holds statistical information of a field such as current, peak and total bytes allocated.
**
**	A field may have sub-fields, which forms a relationship with another field. (Heirarchical database.)
**
** $Id: MemoryField.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**/

#ifndef __Base_Foundation_MemoryField_h__
#define __Base_Foundation_MemoryField_h__

	/** Textual name for MemoryField class. */
	extern const Type MemoryField_Type;
	
	/** Special Name which can be used to ignore an allocation when calculating and querying the Memory database. */
	extern const Name Memory_IgnoreName;
	
	/** The fields which are printable in MemoryField. A binray flag. */
	typedef enum {
		MEMORYFIELD_VALUE =	0x0001,
		MEMORYFIELD_ALLOC =	0x0002,
		MEMORYFIELD_FREE = 	0x0004,
		MEMORYFIELD_CURRENT =	0x0008,
		MEMORYFIELD_PEAK = 	0x0010,
		MEMORYFIELD_TOTAL = 	0x0020,
		MEMORYFIELD_ALL = 
			MEMORYFIELD_VALUE | 
			MEMORYFIELD_ALLOC | 
			MEMORYFIELD_FREE | 
			MEMORYFIELD_CURRENT | 
			MEMORYFIELD_PEAK | 
			MEMORYFIELD_TOTAL
	} MemoryFieldColumn;

	/** \def __MemoryField See MemoryField. */
	#define __MemoryField \
		char*		value;			\
		Index		allocCount;		\
		Index		freeCount;		\
		SizeT		currentAllocation;	\
		SizeT		peakAllocation;		\
		SizeT		totalAllocation;	\
		Index		subCount;		\
		Index		subSize;		\
		MemoryField**	subFields;		\
		MemoryField*	memCache;		/**< A cache to speedup localised searches. */
	struct MemoryField { __MemoryField };

	/** Creates a new MemoryField with a value from a field. */
	MemoryField* MemoryField_New( const char* value );
	
	/** Initialises a MemoryField. */
	void _MemoryField_Init( MemoryField* memoryField, const char* value );
	
	/** Deallocates memory from a MemoryField. */
	void MemoryField_Delete( MemoryField* memoryField );


	/** Registers a sub MemoryField with this instance with the given value. If value exists, the existing field is returned. */
	MemoryField* MemoryField_Register( MemoryField* memoryField, const char* subValue );
	
	/** Updates the statisical information of this field. Should only be used for leaf fields. */
	void MemoryField_Update( MemoryField* memoryField, SizeT bytes );
	
	/** Updates statisical information of this field based on its children (branches and leaves). */
	void MemoryField_UpdateAsSumOfSubFields( MemoryField* memoryField );


	/** Displays the contents of a MemoryField.
	 **
	 ** @param columns A Bit flag of The fields to be displayed.
	 **/
	void MemoryField_Print( MemoryField* memoryField, MemoryFieldColumn columns );
	
	/** Displays all fields of a MemoryField. */
	#define MemoryField_PrintAll( memoryField ) \
		MemoryField_Print( memoryField, MEMORYFIELD_ALL ) 
	
	/** Displays a heading row for MemoryField printouts.
	 **
	 ** @param columns A Bit flag of The fields to be displayed.
	 **/
	void MemoryField_PrintHeader( const char* fieldName, MemoryFieldColumn columns );
	
	/** Displays all headings of a MemoryField. */
	#define MemoryField_PrintHeaderAll( fieldName ) \
		MemoryField_PrintHeader( fieldName, (MemoryFieldColumn)MEMORYFIELD_ALL )


	/** Displays a summary of this field and its children. */
	void MemoryField_PrintSummary( MemoryField* memoryField, const char* tableTitle );


	/** Sorts the children of this field lexographically in ascending order. */
	void MemoryField_Sort( MemoryField* memoryField );

	/** Compares two strings which can potentially NULL, lexographically. NULLs are considered smallest. */
	int MemoryField_StringCompare( const char* s1, const char* s2 );

#endif /* __Base_Foundation_MemoryField_h__ */






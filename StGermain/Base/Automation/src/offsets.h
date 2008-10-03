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
**	Macros for calculating offsets of members in structs and arrays.
**
** Assumptions:
**	None as yet.
**
** Comments:
**	None as yet.
**
** $Id: offsets.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Base_Automation_offsets_h__
#define __Base_Automation_offsets_h__

	/** Given a struct object and the name of a member, return the member */
	#define GetMember( structVar, member ) \
		( (structVar).member )

	/** Given an array object and an index into the array, return the indexed item */
	#define GetIndex( arrayVar, index ) \
		( (arrayVar)[(index)] )

	/** Given a struct object and the name of a member, work out the offset of that member in the struct. */
	#define GetOffsetOfMember( structVar, member ) \
		( (ArithPointer)&( GetMember( (structVar), member ) ) - (ArithPointer)&(structVar) )

	/** Given an array object and an index into the array, work out the offset of that index in the array */
	#define GetOffsetOfIndex( arrayVar, index ) \
		( (ArithPointer)&( GetIndex( (arrayVar), (index) ) ) - (ArithPointer)&(arrayVar) )

	/** Given a struct object, the name of an array member and an index into the array, work out the offset of that index in
	    the struct */
	#define GetOffsetOfMemberArrayIndex( structVar, arrayMember, index ) \
		( GetOffsetOfMember( (structVar), arrayMember ) + GetOffsetOfIndex( (structVar).arrayMember, (index) ) )

	/** Given an array object, the index of a struct item and a name of a member, work out the offset of that member in the
	    array */
	#define GetOffsetOfIndexStructMember( arrayVar, structIndex, member ) \
		( GetOffsetOfIndex( (arrayVar), (structIndex) ) + \
		  GetOffsetOfMember( GetIndex( (arrayVar), (structIndex) ), (index) ) )

#endif /* __Base_Automation_offsets_h__ */

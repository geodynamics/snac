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
**
** <b>Assumptions</b>
**
** <b>Comments</b>
**
** <b>Description</b>
**
** $Id: MemoryTag.h 2940 2005-05-15 00:53:19Z AlanLo $
**
**/

#ifndef __Base_Foundation_MemoryTag_h__
#define __Base_Foundation_MemoryTag_h__

	/* Memory Header that is inserted into every malloc
	 * As this grows, we can move into separate file.
	 *
	 * NOTE: Keep this guy aligned to 8-byte words to avoid misalign problems
	 *       (not doing as a macro because it may not resolve optimally and slow things down)
	 */
	#define __MemoryTag\
		unsigned long	instCount;		/**< Instance counter. */  \
		unsigned long   size;

	struct MemoryTag { __MemoryTag };


	#define Memory_CountGet( ptr ) ((MemoryTag*)( (ArithPointer)(ptr) - sizeof(MemoryTag) ))->instCount
	
	#define Memory_CountInc( ptr ) ( Memory_CountGet( ptr )++ )

	#define Memory_CountDec( ptr ) ( Memory_CountGet( ptr )-- )

	#define Memory_SizeGet( ptr ) ((MemoryTag*)( (ArithPointer)(ptr) - sizeof(MemoryTag) ))->size


#endif

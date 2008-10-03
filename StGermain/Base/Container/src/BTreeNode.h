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
**	Binary Tree class for objects.
**
** <b>Assumptions:</b>
**	None
**
** <b>Comments:</b>
**	None
**
** $Id: BTreeNode.h 2087 2004-11-15 02:28:44Z RaquibulHassan $
**
**/

#ifndef __Base_Foundation__BTreeNode_h__
#define __Base_Foundation__BTreeNode_h__

	/** Textual name for BTreeNode class. */
	extern const Type BTreeNode_Type;

	/** \def __BTreeNode See BTreeNode */
	#define __BTreeNode \
		/* General info */ \
		\
		/* Virtual info */ \
		\
		/* BTree info */ \
		Color				color; \
		void 				*data; \
		SizeT				sizeOfData; \
		struct BTreeNode	*left; \
		struct BTreeNode	*right; \
		struct BTreeNode	*parent; 

	struct BTreeNode { __BTreeNode };


	extern BTreeNode terminal;
	#define NIL &terminal


	/** Constructor interface. */
	BTreeNode *BTreeNode_New ( void );
	
	/** Init interface. */
	void _BTreeNode_Init( BTreeNode *self );
	
	/** Stg_Class_Delete interface. */
	
	/** Print interface. */

#endif /* __Base_Foundation__BTreeNode_h__ */


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

/* SPECIAL NOTE.
 *
 * This is a dummy header file, listing a subset of function prototypes in the Journal system.
 * The file has been constructed to allow the Memory Module to use the Journal, which resides in the IO directory.
 * IO is invisible to the Foundation directory.
 *
 * On compilation these types and functions are processed as undefined symbols, and will not be linked until
 * the Base directory is compiled into archive (.a) and dynamic libraries (.so).
 *
 * For this reason, JournalDummy.h must NOT be included in Foundation.h or Makefile.def as this will export it to become a part of
 * the StGermain framework.
 */

#ifndef __Base_Foundation_forwardDecl_h__
#define __Base_Foundation_forwardDecl_h__
	
	
	/* Forward Declaration of Stream ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	
	typedef struct Stream Stream;
	
	void Stream_Indent( void* );
	void Stream_UnIndent( void* );
	
	
	/* Forward Declaration of Journal ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	
	extern const Type Info_Type;
	extern const Type Error_Type;
	extern const Type Debug_Type;
	
	Stream* Journal_Register( const char*, const char* );
	
	int Journal_Printf( void*, char*, ... );
	int Journal_PrintfL( void*, unsigned int, char*, ... );
	int Journal_Firewall( int expression, void* stream, char* fmt, ... );
	
	#ifdef DEBUG
		#define Journal_DPrintf Journal_Printf
	#else
		#define Journal_DPrintf if (0) Journal_Printf
	#endif
	
	
	/* Forward Declaration of PtrMap ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	struct PtrMap* PtrMap_New( unsigned );	
	void PtrMap_Append( void* ptrMap, void* key, void* ptr );
	void* PtrMap_Find( void* ptrMap, void* key );
	
	
	/* Forward Declaration of BTree ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	
	typedef struct BTree BTree;
	typedef struct BTreeNode BTreeNode;
	
	typedef int (BTree_compareFunction)		(void*, void*);
	typedef void (BTree_dataCopyFunction)		(void**, void*, SizeT);
	typedef void (BTree_dataDeleteFunction)		(void*);
	typedef void (BTree_dataPrintFunction)		(void*, Stream *);
	typedef void (BTree_parseFunction)		(void*, void*);
	
	typedef enum BTreeProperty_t{
		BTREE_ALLOW_DUPLICATES,
		BTREE_NO_DUPLICATES
	}
	BTreeProperty;
	
	BTree* BTree_New( 
		BTree_compareFunction*		compareFunction,
		BTree_dataCopyFunction*		dataCopyFunction,
		BTree_dataDeleteFunction*	dataDeleteFunction,
		BTree_dataPrintFunction*	dataPrintFunction,
		BTreeProperty			property );
	
	void BTree_Delete( void* tree );
	void BTree_ParseTree( BTree *tree, BTree_parseFunction *parseFunction, void *args );
	BTreeNode *BTree_FindNode( BTree *tree, void *data );
	int BTree_InsertNode ( BTree *tree, void *newNodeData, SizeT );
	void BTree_DeleteNode( BTree *tree, BTreeNode *z );
	void BTree_SetCompareFunction ( BTree *tree, BTree_compareFunction *compareFunction );
	void* BTree_GetData( BTreeNode *node );
	
#endif

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
 **	Basic framework types.
 **
 ** <b>Assumptions:</b>
 **	None as yet.
 **
 ** <b>Comments:</b>
 **	None as yet.
 **
 ** $Id: shortcuts.h 2940 2005-05-15 00:53:19Z AlanLo $
 **
 **/

#ifndef __Base_Foundation_shortcuts_h__
#define __Base_Foundation_shortcuts_h__

/* If the system does not have these macros, define it here */
#ifndef MAX
#define MAX( a, b ) ( ( (a) > (b) ) ? (a) : (b) )
#endif
#ifndef MIN
#define MIN( a, b ) ( ( (a) < (b) ) ? (a) : (b) )
#endif	

#define FreeArray( ptr )			\
	if( ptr )				\
		Memory_Free( ptr )
	
	
#define FreeArray2D( nBlocks, ptr )					\
	do {								\
		if( ptr ) {						\
			unsigned	block_i;			\
			for( block_i = 0; block_i < nBlocks; block_i++ ) { \
				if( (ptr)[block_i] ) {			\
					Memory_Free( (ptr)[block_i] );	\
				}					\
			}						\
			Memory_Free( ptr );				\
		}							\
	} while( 0 )
	
	
#define KillArray( ptr )			\
	do {					\
		if( ptr ) {			\
			Memory_Free( ptr );	\
			(ptr) = NULL;		\
		}				\
	} while( 0 )
	
	
#define KillArray2D( nBlocks, ptr )					\
	do {								\
		if( ptr ) {						\
			unsigned	block_i;			\
			for( block_i = 0; block_i < nBlocks; block_i++ ) { \
				if( (ptr)[block_i] ) {			\
					Memory_Free( (ptr)[block_i] );	\
				}					\
			}						\
			Memory_Free( ptr );				\
			(ptr) = NULL;					\
		}							\
	} while( 0 )

#define DeleteArray( ptr, size )					\
	do {								\
		if( ptr ) {						\
			unsigned	__class_i;			\
			for( __class_i = 0; __class_i < size; __class_i++ ) \
				if( ptr[__class_i] ) Stg_Delete_Class( ptr[__class_i] ); \
			ptr = NULL;					\
		}							\
	} while( 0 )

#define DeleteOwnArray( ptr, own, size )				\
	do {								\
		if( ptr ) {						\
			unsigned	__class_i;			\
			for( __class_i = 0; __class_i < size; __class_i++ ) \
				if( ptr[__class_i] && own[__class_i] )	\
					Stg_Delete_Class( ptr[__class_i] ); \
			Memory_Free( ptr );				\
			Memory_Free( own );				\
			ptr = NULL;					\
			own = NULL;					\
		}							\
	} while( 0 )

#define FreeObject( obj ) \
	if( obj ) Stg_Class_Delete( obj )

#define KillObject( obj ) \
	if( obj ) (Stg_Class_Delete( obj ), (obj) = NULL)


#ifndef NDEBUG
#define insist( expr ) assert( expr )
#else
#define insist( expt ) expr
#endif
	
#endif /* __Base_Foundation_shortcuts_h__ */

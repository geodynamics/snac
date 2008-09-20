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
**	Kent Humphries, Software Engineer, VPAC. (kenth@vpac.org)
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
**	A tuple in the Dictionary database.
**
** <b>Assumptions:</b>
**	None.
**
** <b>Comments:</b>
**	- A copy of the entry key is created and deleted internally.
**	- If not NULL, a copy of the originalSource is created and deleted internally.
**	- Assumes ownership of the entry value, and will destroy it when Dictionary_Entry_Delete() is called.
**
** <b>Description:</b>
**	A data structure which holds a key-value-originalSource (third may be NULL) triplet for a dictionary.
**
** $Id: Dictionary_Entry.h 3743 2006-08-03 03:14:38Z KentHumphries $
**
**/

#ifndef __Base_IO_Dictionary_Entry_h__
#define __Base_IO_Dictionary_Entry_h__
	
	/** \def __Dictionary_Entry See Dictionary_Entry */
	#define __Dictionary_Entry  \
		Dictionary_Entry_Key		key; \
		Dictionary_Entry_Value*		value; \
		Dictionary_Entry_Source		source;

	struct _Dictionary_Entry { __Dictionary_Entry };

	/** Constant value used for Dictionary_Entry source when no source file was specified */
	#define NO_SOURCEFILE "created_in_code"
	
	
	/** Create a new Dictionary_Entry (assumes ownership of the value) */
	Dictionary_Entry* Dictionary_Entry_New( Dictionary_Entry_Key key, Dictionary_Entry_Value* value );

	/** Create a new Dictionary_Entry with a source file (assumes ownership of the value) */
	Dictionary_Entry* Dictionary_Entry_NewWithSource( Dictionary_Entry_Key key, Dictionary_Entry_Value* value, Dictionary_Entry_Source source );

	/** Initialise a Dictionary_Entry (assumes ownership of the value) */
	void Dictionary_Entry_Init( Dictionary_Entry* self, Dictionary_Entry_Key key, Dictionary_Entry_Value* value );
	
	/** Initialise a Dictionary_Entry with a source file (assumes ownership of the value) */
	void Dictionary_Entry_InitWithSource( Dictionary_Entry* self, Dictionary_Entry_Key key, Dictionary_Entry_Value* value, Dictionary_Entry_Source source );
	
	/** Destroy a Dictionary_Entry instance */
	void Dictionary_Entry_Delete( Dictionary_Entry* self );
	
	/** Print the contents of a Dictionary_Entry construct */
	void Dictionary_Entry_Print( Dictionary_Entry* self, Stream* stream );
	

	/** Compare a Dictionary_Entry to a key */
	Bool Dictionary_Entry_Compare( Dictionary_Entry* self, Dictionary_Entry_Key key );

	/** Compare a Dictionary_Entry to a key, and source */
	Bool Dictionary_Entry_CompareWithSource( Dictionary_Entry* self, Dictionary_Entry_Key key, Dictionary_Entry_Source source );

	/** Set/Replace an entry's value (assumes ownership of the value) */
	void Dictionary_Entry_Set( Dictionary_Entry* self, Dictionary_Entry_Value* value );
	
	/** Set/Replace an entry's value with new source file (assumes ownership of the value) */
	void Dictionary_Entry_SetWithSource( Dictionary_Entry* self, Dictionary_Entry_Value* value, Dictionary_Entry_Source source );
	
	/** Get an entry's value */
	Dictionary_Entry_Value* Dictionary_Entry_Get( Dictionary_Entry* self );

	/** Get an entry's originalSource */
	Dictionary_Entry_Source Dictionary_Entry_GetSource( Dictionary_Entry* self );
	
#endif /* __Base_IO_Dictionary_Entry_h__ */



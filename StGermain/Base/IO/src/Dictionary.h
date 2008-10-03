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
**	Facilitates a dictionary style repository of information.
**
** <b>Assumptions:</b>
**	calling Dictionary_Entry_Value_AsString puts the result into a temporary
**		buffer:- if you want to use it for other than printing, you should copy
**		it to your own string.
**
** <b>Comments:</b>
**	Made to be compatible/comparable/interchangable with Pythia's PropertySheets.
**	Currently, error handling is very basic - input values checked and set to
**	defaults if necessary, pointers checked, 0 returned when getting an invalid
**	value. Needs to be extended once we decide how we're simulating Journal.
**	Have made some facility to have the content as a void*... but not completed and not sure if necessary.
**	May need to add methods to remove entries from dictionaries and structs, but
**	not yet.
**	The Dictionary_Entry_Value_As... functions can accept the "self" as null, and will provide a zero-like or empty
**	string-like answer. This enables a simple line for accessing values (no need for error checking).
**
** $Id: Dictionary.h 3743 2006-08-03 03:14:38Z KentHumphries $
**
**/

#ifndef __Base_IO_Dictionary_h__
#define __Base_IO_Dictionary_h__
	
	typedef Index Dictionary_Index;		/**< Index type for Dictionary, */
	
	/** Enum to dictionary add/merging type */
	typedef enum { 
		Dictionary_MergeType_Append, 
		Dictionary_MergeType_Merge, 
		Dictionary_MergeType_Replace 
	} Dictionary_MergeType;
	
	
	/** Textual name for Dictionary class. */
	extern const Type Dictionary_Type;
	
	/* Virtual Function pointers */
	typedef void (Dictionary_AddFunction) (void* dictionary, Dictionary_Entry_Key key, Dictionary_Entry_Value* value );
	typedef void (Dictionary_AddWithSourceFunction) (void* dictionary, Dictionary_Entry_Key key, 
								Dictionary_Entry_Value* value, Dictionary_Entry_Source source );
	typedef Bool (Dictionary_SetFunction) (void* dictionary, Dictionary_Entry_Key key, Dictionary_Entry_Value* value );
	typedef Bool (Dictionary_SetWithSourceFunction) (void* dictionary, Dictionary_Entry_Key key, 
								Dictionary_Entry_Value* value, Dictionary_Entry_Source source );
	typedef Dictionary_Entry_Value*	(Dictionary_GetFunction) (void* dictionary, Dictionary_Entry_Key key );
	typedef Dictionary_Entry_Source	(Dictionary_GetSourceFunction) (void* dictionary, Dictionary_Entry_Key key );

	/** \def __Dictionary See Dictionary */
	#define __Dictionary \
		/* General info */ \
		__Stg_Class \
		\
		/* Virtual info */ \
		Dictionary_AddFunction*		add; \
		Dictionary_AddWithSourceFunction*	addWithSource; \
		Dictionary_SetFunction*		set; \
		Dictionary_SetWithSourceFunction*	setWithSource; \
		Dictionary_GetFunction*		get; \
		Dictionary_GetSourceFunction*	getSource; \
		\
		/* Dictionary info */ \
		Dictionary_Index		size; \
		Dictionary_Index		delta; \
		\
		Dictionary_Index		count; \
		Dictionary_Entry**		entryPtr; \
		\
		Stream*				debugStream;
	struct _Dictionary { __Dictionary };
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Dictionary
	*/
	
	/** Initialise a Dictionary construct */
	Dictionary* Dictionary_New( void );
	
	/** Constructor interface. */
	Dictionary* _Dictionary_New( 
		SizeT				_sizeOfSelf, 
		Type 				type, 
		Stg_Class_DeleteFunction* 		_delete,
		Stg_Class_PrintFunction* 		_print,
		Stg_Class_CopyFunction*		_copy, 
		Dictionary_AddFunction* 	add,
		Dictionary_AddWithSourceFunction*	addWithSource,
		Dictionary_SetFunction* 	set,
		Dictionary_SetWithSourceFunction*	setWithSource,
		Dictionary_GetFunction* 	get,
		Dictionary_GetSourceFunction*	getSource);
	
	/** Initialises a Dictionary. */
	void Dictionary_Init( Dictionary* self );
	
	/** Init interface. */
	void _Dictionary_Init( Dictionary* self );
	
	/** Stg_Class_Delete dictionary implementation */
	void _Dictionary_Delete( void* dictionary );
	
	/** Print dictionary implementation */
	void _Dictionary_Print( void* dictionary, Stream* stream );
	
	/** Concise dictionary info print */
	void Dictionary_PrintConcise( void* dictionary, Stream* stream );
	
	/** Add an entry to the dictionary... orignal implementation... appends keys */
	void Dictionary_Add( void* dictionary, Dictionary_Entry_Key key, Dictionary_Entry_Value* value );

	/** Add an entry to the dictionary... orignal implementation... appends keys, with source file */
	void Dictionary_AddWithSource( void* dictionary, Dictionary_Entry_Key key, 
					Dictionary_Entry_Value* value, Dictionary_Entry_Source source );
	
	/** Add an entry to the dictionary... specifying how the entry values are merged if key present already */
	Dictionary_Entry_Value* Dictionary_AddMerge( 
		void*						dictionary, 
		Dictionary_Entry_Key				key, 
		Dictionary_Entry_Value*				value,
		Dictionary_MergeType				mergeType );
	
	/** Add an entry to the dictionary... specifying how the entry values are merged if key present already */
	Dictionary_Entry_Value* Dictionary_AddMergeWithSource( 
		void*						dictionary, 
		Dictionary_Entry_Key				key, 
		Dictionary_Entry_Value*				value,
		Dictionary_MergeType				mergeType,
		Dictionary_Entry_Source				source );

	/** Set a value in the dictionary */
	Bool Dictionary_Set( void* dictionary, Dictionary_Entry_Key key, Dictionary_Entry_Value* value );
	
	/** Set a value in the dictionary */
	Bool Dictionary_SetWithSource( void* dictionary, Dictionary_Entry_Key key, 
					Dictionary_Entry_Value* value, Dictionary_Entry_Source source );

	/** Get a value from the dictionary */
	Dictionary_Entry_Value* Dictionary_Get( void* dictionary, Dictionary_Entry_Key key );

	/** Get a source from the dictionary */
	Dictionary_Entry_Source Dictionary_GetSource( void* dictionary, Dictionary_Entry_Key key );

	/** Get a value from the dictionary by index */
	Dictionary_Entry_Value* Dictionary_GetByIndex( void* dictionary, Dictionary_Index index );

	/** Get a value from the dictionary */
	Dictionary_Entry_Value* Dictionary_GetDefault( void* dictionary, Dictionary_Entry_Key key, Dictionary_Entry_Value* value );
	
	
	/** Add an entry to the dictionary implementation */
	void _Dictionary_Add( void* dictionary, Dictionary_Entry_Key key, Dictionary_Entry_Value* value );
	
	/** Add an entry to the dictionary implementation */
	void _Dictionary_AddWithSource( void* dictionary, Dictionary_Entry_Key key, 
					Dictionary_Entry_Value* value, Dictionary_Entry_Source source );
	
	/** Set a value in the dictionary implementation */
	Bool _Dictionary_Set( void* dictionary, Dictionary_Entry_Key key, Dictionary_Entry_Value* value );
	
	/** Set a value in the dictionary implementation */
	Bool _Dictionary_SetWithSource( void* dictionary, Dictionary_Entry_Key key, 
					Dictionary_Entry_Value* value, Dictionary_Entry_Source source );

	/** Get a value from the dictionary implementation */
	Dictionary_Entry_Value* _Dictionary_Get( void* dictionary, Dictionary_Entry_Key key );
	
	/** Get a source from the dictionary implementation */
	Dictionary_Entry_Source _Dictionary_GetSource( void* dictionary, Dictionary_Entry_Key key );
	
	Dictionary_Entry_Value* _Dictionary_GetByIndex( void* dictionary, Dictionary_Index index );
		
	/** Get an entry from the dictionary */
	Dictionary_Entry* Dictionary_GetEntry( void* dictionary, Dictionary_Entry_Key key );

	/** Get an entry from the dictionary by index */
	Dictionary_Entry* Dictionary_GetEntryByIndex( void* dictionary, Dictionary_Index index);

	/** Get count of entries in the dictionary */
	Index Dictionary_GetCount( void* dictionary );

	/** Loops over command line arguments and reads in values with format "--param=value" */
	void Dictionary_ReadAllParamFromCommandLine( void* dictionary, int argc, char* argv[] ) ;

#endif /* __Base_IO_Dictionary_h__ */

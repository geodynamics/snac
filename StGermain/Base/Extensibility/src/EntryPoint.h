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
** Doxygen information follows:
*/
/** \file
**  Role:
**	Entry-points are glorified "function pointer"s, but with one major advantage: the point can represent one or more functions,
**	changeable at run time. 
**
** Assumptions:
**	Name of the Entry-point and associated Hooks are constant (i.e. of type: Type).
**
** Comments:
**	Entry-points are glorified function-pointers, but with one major advantage: the point can represent one or more functions,
**	changeable at run time. Entry points, hence, provide the facility for dynamic control-flow behaviour, but in a manner
**	considerate to HPC applications. The term "Hook" refers to a named function-pointer that is associated to an Entry-point. To
**	that end, the Entry-point maintains a list of Hooks that are executed in order when the Entry-point is told to execute. The
**	insertion into this list is kept quite flexible, representing the types of manipulations code developers desire when 
**	thinking of extending or adapting the behaviour of a particular behaviour. These options include "associate this Hook to the
**	EntryPoint by": 
**		appending, 
**		prepending, 
**		replacing the entire list with, 
**		replacing a specific already associated hook with,
**		insert before already associated hook, and
**		insert after already associated hook.
**	Execution of an Entry-point has greater over-head than the execution of a function-pointer (typically C coder 
**	implementation of flexibility), which in turn has greater over-head than a hard-coded function (no flexibility, 
**	Fortranesque), which in turn has greater over-head than in-lining (the ultimate hero-codesque). The Entry-point is 
**	implemented to be as effecient at the execution phase as can be. The net result is an awkward calling convention, which is 
**	recommended to be wrapped if end users are coders, and you want to keep their pants dry.
**
** $Id: EntryPoint.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Base_Extensibility_EntryPoint_h__
#define __Base_Extensibility_EntryPoint_h__
	

	/* Templates of virtual functions */
	typedef Func_Ptr	(EntryPoint_GetRunFunction)	( void* self );
	
	/* Templates for default entry point type */
	typedef void        (EntryPoint_0_Cast)                         ();
	typedef void        (EntryPoint_0_CallCast)                     ( void* entryPoint );
	typedef void        (EntryPoint_VoidPtr_Cast)                   ( void* data0 );
	typedef void        (EntryPoint_VoidPtr_CallCast)               ( void* entryPoint, void* data0 );
	typedef void        (EntryPoint_2VoidPtr_Cast)	                ( void* data0, void* data1 );
	typedef void        (EntryPoint_2VoidPtr_CallCast)              ( void* entryPoint, void* data0, void* data1 );
	typedef void        (EntryPoint_3VoidPtr_Cast)	                ( void* data0, void* data1, void* data2 );
	typedef void        (EntryPoint_3VoidPtr_CallCast)              ( void* entryPoint, void* data0, void* data1, void* data2 );
	typedef double      (EntryPoint_Minimum_VoidPtr_Cast)           ( void* data0 );
	typedef double      (EntryPoint_Minimum_VoidPtr_CallCast)       ( void* entryPoint, void* data0 );
	typedef double      (EntryPoint_Maximum_VoidPtr_Cast)           ( void* data0 );
	typedef double      (EntryPoint_Maximum_VoidPtr_CallCast)       ( void* entryPoint, void* data0 );
	typedef double      (EntryPoint_Class_Minimum_VoidPtr_Cast)     ( void* reference, void* data0 );
	typedef double      (EntryPoint_Class_Minimum_VoidPtr_CallCast) ( void* entryPoint, void* reference, void* data0 );
	typedef double      (EntryPoint_Class_Maximum_VoidPtr_Cast)     ( void* reference, void* data0 );
	typedef double      (EntryPoint_Class_Maximum_VoidPtr_CallCast) ( void* entryPoint, void* reference, void* data0 );
	typedef void        (EntryPoint_Class_0_Cast)                   ( void* reference );
	typedef void        (EntryPoint_Class_0_CallCast)  	            ( void* entryPoint );
	typedef void        (EntryPoint_Class_VoidPtr_Cast)             ( void* reference, void* data0 );
	typedef void        (EntryPoint_Class_VoidPtr_CallCast)         ( void* entryPoint, void* data0 );

	#define 		EntryPoint_0_CastType                       0
	#define 		EntryPoint_VoidPtr_CastType                 (EntryPoint_0_CastType+1)
	#define 		EntryPoint_2VoidPtr_CastType                (EntryPoint_VoidPtr_CastType+1)
	#define 		EntryPoint_3VoidPtr_CastType                (EntryPoint_2VoidPtr_CastType+1)
	#define 		EntryPoint_Minimum_VoidPtr_CastType         (EntryPoint_3VoidPtr_CastType+1)
	#define 		EntryPoint_Maximum_VoidPtr_CastType         (EntryPoint_Minimum_VoidPtr_CastType+1)
	#define 		EntryPoint_Class_0_CastType                 (EntryPoint_Maximum_VoidPtr_CastType+1)
	#define			EntryPoint_Class_VoidPtr_CastType           (EntryPoint_Class_0_CastType+1)
	#define 		EntryPoint_Class_Minimum_VoidPtr_CastType   (EntryPoint_Class_VoidPtr_CastType+1)
	#define 		EntryPoint_Class_Maximum_VoidPtr_CastType   (EntryPoint_Class_Minimum_VoidPtr_CastType+1)
	#define 		EntryPoint_CastType_MAX                     (EntryPoint_Class_Maximum_VoidPtr_CastType+1)
	
	/** Textual name of this class */
	extern const Type EntryPoint_Type;
	
	/** EntryPoint info */
	#define __EntryPoint \
		/* General info */ \
		__Stg_Object \
		\
		/* Virtual info */ \
		EntryPoint_GetRunFunction*	_getRun; \
		\
		/* EntryPoint info */ \
		unsigned int			castType; \
		void*				run; \
		HookList*			hooks; \
		Hook*				alwaysFirstHook; \
		Hook*				alwaysLastHook;

	struct EntryPoint { __EntryPoint };
	
	/** Create a new EntryPoint */
	EntryPoint* EntryPoint_New( const Name name, unsigned int castType );
	
	/** Initialise an existing EntryPoint */
	void EntryPoint_Init( void* entryPoint, const Name name, unsigned int castType );
	
	/** Creation implementation */
	EntryPoint* _EntryPoint_New( 
		SizeT 				_sizeOfSelf, 
		Type 				type, 
		Stg_Class_DeleteFunction* 		_delete,
		Stg_Class_PrintFunction*		_print,
		Stg_Class_CopyFunction*		_copy, 
		EntryPoint_GetRunFunction*	_getRun,
		Name 				name, 
		unsigned int 			castType );
	
	/** Member Initialisation implementation */
	void _EntryPoint_Init( EntryPoint* self, unsigned int castType );
	
	/** Stg_Class_Delete() implementation */
	void _EntryPoint_Delete( void* entryPoint );
	
	/** Stg_Class_Print() implementation */
	void _EntryPoint_Print( void* entryPoint, Stream* stream );
	
	/* Copy */
	#define EntryPoint_Copy( self ) \
		(EntryPoint*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define EntryPoint_DeepCopy( self ) \
		(EntryPoint*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _EntryPoint_Copy( void* entryPoint, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/* VIRTUAL FUNCTIONS */

	
	/* Obtain the pointer the function that will run this entry point (i.e. the function that will call the many functions this
	 *  entry point */
	Func_Ptr EntryPoint_GetRun( void* entryPoint );
	
	/** Default GetRun implementation */
	Func_Ptr _EntryPoint_GetRun( void* entryPoint );
	
	/** Default entry point run... no arguements passed */
	void _EntryPoint_Run( void* entryPoint );
	
	/** Default entry point run... one void* arguement passed */
	void _EntryPoint_Run_VoidPtr( void* entryPoint, void* data0 );
	
	/** Default entry point run funcs... many void* arguments passed */
	void _EntryPoint_Run_2VoidPtr( void* entryPoint, void* data0, void* data1 );
	void _EntryPoint_Run_3VoidPtr( void* entryPoint, void* data0, void* data1, void* data2 ) ;

	/** Entry point run functions which calculates and returns the max or min of what each of the hooks returned */
	double _EntryPoint_Run_Minimum_VoidPtr( void* entryPoint, void* data0 ) ;
	double _EntryPoint_Run_Maximum_VoidPtr( void* entryPoint, void* data0 ) ;
	double _EntryPoint_Run_Class_Minimum_VoidPtr( void* entryPoint, void* data0 ) ;
	double _EntryPoint_Run_Class_Maximum_VoidPtr( void* entryPoint, void* data0 ) ;

	/** Runs a plain ClassHook */
	void _EntryPoint_Run_Class( void* entryPoint );
	void _EntryPoint_Run_Class_VoidPtr( void* entryPoint, void* data0 );

	
	/* Functions for Adding hooks */
	void _EntryPoint_PrependHook( void* entryPoint, Hook* hook );
	void _EntryPoint_PrependHook_AlwaysFirst( void* entryPoint, Hook* hook );
	void _EntryPoint_AppendHook( void* entryPoint, Hook* hook );
	void _EntryPoint_AppendHook_AlwaysLast( void* entryPoint, Hook* hook );
	void _EntryPoint_InsertHookBefore( void* entryPoint, Name hookToInsertBefore, Hook* hook );
	void _EntryPoint_InsertHookAfter( void* entryPoint, Name hookToInsertAfter, Hook* hook );
	void _EntryPoint_ReplaceAllHook( void* entryPoint, Hook* hook );
	void _EntryPoint_ReplaceHook( void* entryPoint, Name hookToReplace, Hook* hook );
	
	/* PUBLIC FUNCTIONS */
	
	/* Print concise function:- print information of interest when not debugging */
	void EntryPoint_PrintConcise( void* entryPoint, Stream* stream );
	
	/** Prepend a new Hook function at the start of an entry point's list of hooks to run. */
	void EntryPoint_Prepend( void* entryPoint, Name name, Func_Ptr funcPtr, char* addedBy );
	
	/** Prepend a new Hook function at the start of an entry point's list of hooks to run, specifying it
	should always remain the first hook. */
	void EntryPoint_Prepend_AlwaysFirst( void* entryPoint, Name name, Func_Ptr funcPtr, char* addedBy );
	
	/** Add a new Func_Ptr to an entry point, at the end of the current list of hooks. If there is a hook
	already specified to always come last, the new hook will be appended to come just before it. */
	void EntryPoint_Append( void* entryPoint, Name name, Func_Ptr funcPtr, char* addedBy );
	
	/** Add a new Func_Ptr to an entry point, at the end of the list of hooks - and make
	sure the entry point is always kept at the end. If this is called twice on
	the same entry point, an assert results. */
	void EntryPoint_Append_AlwaysLast( void* entryPoint, Name name, Func_Ptr funcPtr, char* addedBy );

	/** Add a new Hook function to an entry point, just before the specified hookToInsertBefore in the list of hooks.
	 * If the specified hook to insert before doesn't exist, then asserts with an error. */
	void EntryPoint_InsertBefore( void* entryPoint, Name hookToInsertBefore, Name name, Func_Ptr funcPtr, char* addedBy );
	
	/** Add a new Hook function to an entry point, just after the specified hookToInsertAfter in the list of hooks.
	 * If the specified hook to insert after doesn't exist, then asserts with an error. */
	void EntryPoint_InsertAfter( void* entryPoint, Name hookToInsertAfter, Name name, Func_Ptr funcPtr, char* addedBy );
	
	/** Replace a specific named hook with a new hook function.
	 * If the specified hook to replace doesn't exist, then asserts with an error. */
	void EntryPoint_Replace( void* entryPoint, Name hookToReplace, Name name, Func_Ptr funcPtr, char* addedBy );
	
	/** Replace all existing hooks of an entry point with a new hook. */
	void EntryPoint_ReplaceAll( void* entryPoint, Name name, Func_Ptr funcPtr, char* addedBy );


	/** Prepend a new Hook function at the start of an entry point's list of hooks to run. */
	void EntryPoint_PrependClassHook( void* entryPoint, Name name, Func_Ptr funcPtr, char* addedBy, void* reference );
	
	/** Prepend a new Hook function at the start of an entry point's list of hooks to run, specifying it
	should always remain the first hook. */
	void EntryPoint_PrependClassHook_AlwaysFirst( 
		void* entryPoint, 
		Name name, 
		Func_Ptr funcPtr, 
		char* addedBy, 
		void* reference );
	
	/** Add a new Func_Ptr to an entry point, at the end of the current list of hooks. If there is a hook
	already specified to always come last, the new hook will be appended to come just before it. */
	void EntryPoint_AppendClassHook( void* entryPoint, Name name, Func_Ptr funcPtr, char* addedBy, void* reference );
	
	/** Add a new Func_Ptr to an entry point, at the end of the list of hooks - and make
	sure the entry point is always kept at the end. If this is called twice on
	the same entry point, an assert results. */
	void EntryPoint_AppendClassHook_AlwaysLast( 
		void* entryPoint, 
		Name name, 
		Func_Ptr funcPtr, 
		char* addedBy, 
		void* reference );

	/** Add a new Hook function to an entry point, just before the specified hookToInsertBefore in the list of hooks.
	 * If the specified hook to insert before doesn't exist, then asserts with an error. */
	void EntryPoint_InsertClassHookBefore( 
		void* entryPoint, 
		Name hookToInsertBefore, 
		Name name, 
		Func_Ptr funcPtr, 
		char* addedBy, 
		void* reference );
	
	/** Add a new Hook function to an entry point, just after the specified hookToInsertAfter in the list of hooks.
	 * If the specified hook to insert after doesn't exist, then asserts with an error. */
	void EntryPoint_InsertClassHookAfter( 
		void* entryPoint, 
		Name hookToInsertAfter, 
		Name name, 
		Func_Ptr funcPtr, 
		char* addedBy, 
		void* reference );
	
	/** Replace a specific named hook with a new hook function.
	 * If the specified hook to replace doesn't exist, then asserts with an error. */
	void EntryPoint_ReplaceClassHook( 
		void* entryPoint, 
		Name hookToReplace, 
		Name name, 
		Func_Ptr funcPtr, 
		char* addedBy, 
		void* reference );
	
	/** Replace all existing hooks of an entry point with a new hook. */
	void EntryPoint_ReplaceAllClassHook( void* entryPoint, Name name, Func_Ptr funcPtr, char* addedBy, void* reference );


	/** Removes a hook from an entry point
	 * If the specified hook to remove doesn't exist, then asserts with an error. */
	void EntryPoint_Remove( void* entryPoint, Name name ) ;	

	/** "Purge" the entry point of all existing hooks, leaving it blank. */
	void EntryPoint_Purge( void* entryPoint );
	
	void EntryPoint_WarnIfNoHooks( void* entryPoint, const char* parentFunction );

	void EntryPoint_ErrorIfNoHooks( void* entryPoint, const char* parentFunction );
	
#endif /* __Base_Extensibility_EntryPoint_h__ */

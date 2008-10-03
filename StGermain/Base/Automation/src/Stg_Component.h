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
**	Abstract class for objects that are named.
**
** Assumptions:
**
** Comments:
**
** $Id: Stg_Component.h 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Base_Automation_Stg_Component_h__
#define __Base_Automation_Stg_Component_h__
	
	/* Templates of virtual functions */
	typedef void*				(Stg_Component_DefaultConstructorFunction)	( Name name );
	typedef void				(Stg_Component_ConstructFunction)		( void* component, 
												  Stg_ComponentFactory* cf,
												  void* data );
	typedef void				(Stg_Component_BuildFunction)			( void* component, void* data );
	typedef void				(Stg_Component_InitialiseFunction)		( void* component, void* data );
	typedef void				(Stg_Component_ExecuteFunction)			( void* component, void* data );
	typedef void				(Stg_Component_DestroyFunction)			( void* component, void* data );
	
	/* Textual name of this class */
	extern const Type Stg_Component_Type;
	
	
	/* Stg_Component information */
	#define __Stg_Component \
		/* General info */ \
		__Stg_Object \
		\
		/* Virtual info */ \
		Stg_Component_DefaultConstructorFunction*	_defaultConstructor; \
		Stg_Component_ConstructFunction*		_construct; \
		Stg_Component_BuildFunction*			_build; \
		Stg_Component_InitialiseFunction*		_initialise; \
		Stg_Component_ExecuteFunction*			_execute; \
		Stg_Component_DestroyFunction*			_destroy; \
		\
		/* Stg_Component info */ \
		Bool					isConstructed; \
		Bool					isBuilt; \
		Bool					isInitialised; \
		Bool					hasExecuted; \
		Bool					isDestroyed; \
		Type					constructType; \
		Type					buildType; \
		Type					initialiseType; \
		Type					executeType; \
		Type					destroyType;
	struct Stg_Component { __Stg_Component };

	#define STG_COMPONENT_DEFARGS							\
		STG_CLASS_DEFARGS,							\
		Stg_Component_DefaultConstructorFunction*	_defaultConstructor, 	\
		Stg_Component_ConstructFunction*		_construct, 		\
		Stg_Component_BuildFunction*			_build, 		\
		Stg_Component_InitialiseFunction*		_initialise, 		\
		Stg_Component_ExecuteFunction*			_execute, 		\
		Stg_Component_DestroyFunction*			_destroy, 		\
		Name						name, 			\
		AllocationType					nameAllocationType

	#define STG_COMPONENT_PASSARGS						\
		STG_CLASS_PASSARGS, _defaultConstructor, _construct, 		\
		_build, _initialise, _execute, _destroy, 			\
		name, nameAllocationType
	
	/* No Stg_Component_New or Stg_Component_Init as this is an abstract class */
	
	/* Creation implementation */
	Stg_Component* _Stg_Component_New( 
		SizeT						_sizeOfSelf, 
		Type						type,
		Stg_Class_DeleteFunction*			_delete,
		Stg_Class_PrintFunction*			_print, 
		Stg_Class_CopyFunction*				_copy, 
		Stg_Component_DefaultConstructorFunction*	_defaultConstructor,
		Stg_Component_ConstructFunction*		_construct,
		Stg_Component_BuildFunction*			_build,
		Stg_Component_InitialiseFunction*		_initialise,
		Stg_Component_ExecuteFunction*			_execute,
		Stg_Component_DestroyFunction*			_destroy,
		Name						name,
		AllocationType					nameAllocationType );
	
	
	/* Class Administration members ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	
	/* Initialisation implementation */
	void _Stg_Component_Init( Stg_Component* self );
	
	/* Stg_Class_Delete boundary condition layout implementation */
	void _Stg_Component_Delete( void* component );
	
	/* Print boundary condition layout implementation */
	void _Stg_Component_Print( void* component, Stream* stream );
	
	void* _Stg_Component_Copy( void* component, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap );
	
	
	/* Public member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	
	/* Copy the component */
	#define Stg_Component_Copy( self ) \
		(Stg_Component*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define Stg_Component_DeepCopy(self) \
		(Stg_Component*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	/** Contruct the component. Configure/setup the component. */
	void Stg_Component_Construct( void* component, Stg_ComponentFactory* cf, void* data, Bool force );
	
	/** Build the component: Take the configuration and instantiate the component (do all main mallocs, etc). */
	void Stg_Component_Build( void* component, void* data, Bool force );
	
	/** Initialise the component: Place in initial values. After this point the component is ready to execute. */
	void Stg_Component_Initialise( void* component, void* data, Bool force );
	
	/** Execute the component: Perform its task. */
	void Stg_Component_Execute( void* component, void* data, Bool force );
	
	/** Destroy the component: All resources used in the other phases are released. */
	void Stg_Component_Destroy( void* component, void* data, Bool force );
	
	/** Is the component constructed? (i.e. its configration/setup performed) */
	#define Stg_Component_IsConstructedMacro( component ) \
		((component)->isConstructed)
	Bool Stg_Component_IsConstructedFunc( void* component );
	#ifdef MACRO_AS_FUNC
		#define Stg_Component_IsConstructed Stg_Component_IsConstructedFunc
	#else
		#define Stg_Component_IsConstructed Stg_Component_IsConstructedMacro
	#endif
	#define IsConstructed Stg_Component_IsConstructed
	
	/** Is the component built? (i.e. instantiated) */
	#define Stg_Component_IsBuiltMacro( component ) \
		((component)->isBuilt)
	Bool Stg_Component_IsBuiltFunc( void* component );
	#ifdef MACRO_AS_FUNC
		#define Stg_Component_IsBuilt Stg_Component_IsBuiltFunc
	#else
		#define Stg_Component_IsBuilt Stg_Component_IsBuiltMacro
	#endif
	#define IsBuilt Stg_Component_IsBuilt
	
	/** Is the component initialised? (i.e. all initial values set) */
	#define Stg_Component_IsInitialisedMacro( component ) \
		((component)->isInitialised)
	Bool Stg_Component_IsInitialisedFunc( void* component );
	#ifdef MACRO_AS_FUNC
		#define Stg_Component_IsInitialised Stg_Component_IsInitialisedFunc
	#else
		#define Stg_Component_IsInitialised Stg_Component_IsInitialisedMacro
	#endif
	#define IsInitialised Stg_Component_IsInitialised
	
	/** Has the component executed? */
	#define Stg_Component_HasExecutedMacro( component ) \
		((component)->hasExecuted)
	Bool Stg_Component_HasExecutedFunc( void* component );
	#ifdef MACRO_AS_FUNC
		#define Stg_Component_HasExecuted Stg_Component_HasExecutedFunc
	#else
		#define Stg_Component_HasExecuted Stg_Component_HasExecutedMacro
	#endif
	#define HasExecuted Stg_Component_HasExecuted
	
	/** Is the component destroyed? */
	#define Stg_Component_IsDestroyedMacro( component ) \
		((component)->isDestroyed)
	Bool Stg_Component_IsDestroyedFunc( void* component );
	#ifdef MACRO_AS_FUNC
		#define Stg_Component_IsDestroyed Stg_Component_IsDestroyedFunc
	#else
		#define Stg_Component_IsDestroyed Stg_Component_IsDestroyedMacro
	#endif
	#define IsDestroyed Stg_Component_IsDestroyed

	/** Disowns the component from the current source, leaving the Live Stg_Component Register to 
	    destroy it */
	#define Stg_Component_Disown( component ) \
		( Memory_CountGet( component ) > 1 ? Memory_CountDec( component ) : Stg_Class_Delete( component ) )

	void Stg_Component_SetupStreamFromDictionary( void* component, Dictionary* dictionary );

	#define Stg_Component_CreateMeta( name, type ) \
		_Stg_Component_CreateMeta( name, type )
	Stg_ComponentMeta* _Stg_Component_CreateMeta( Name name, Type type );

	#define Stg_Component_Validate( component, type, dictionary ) \
		_Stg_Component_Validate( component, type, dictionary )
	Stg_ComponentMeta* _Stg_Component_Validate( void* component, Type type, Dictionary* dictionary );
	
#endif /* __Base_Automation_Stg_Component_h__ */

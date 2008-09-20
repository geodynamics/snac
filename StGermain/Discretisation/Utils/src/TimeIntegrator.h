/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003, Victorian Partnership for Advanced Computing (VPAC) Ltd, 110 Victoria Street, Melbourne, 3053, Australia.
**
** Authors:
**	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
**	David May, PhD Student Monash University, VPAC. (davidm@vpac.org)
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
**
** Assumptions:
**
** Comments:
**
** $Id: TimeIntegrator.h 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Utils_TimeIntegrator_h__
#define __Discretisation_Utils_TimeIntegrator_h__
	
	/* typedefs for virtual functions: */
	extern const Type TimeIntegrator_Type;
	
	/* TimeIntegrator information */
	#define __TimeIntegrator  \
		/* General info */ \
		__Stg_Component \
		\
		Stream*                                debug;                   \
		Stream*                                info;                   \
		NamedObject_Register*                  integrateeRegister;      \
		unsigned int                           order;                   \
		Bool                                   simultaneous;            \
		Name                                   _setupEPName;            \
		Name                                   _finishEPName;           \
		EntryPoint*                            setupEP;                 \
		Stg_ObjectList*                        setupData;               \
		EntryPoint*                            finishEP;                \
		Stg_ObjectList*                        finishData;              \
		double                                 time;
		  
	struct TimeIntegrator { __TimeIntegrator };
	
	/* Creation implementation / Virtual constructor */
	TimeIntegrator* TimeIntegrator_New( 
		Name                                       name,
		unsigned int                               order, 
		Bool                                       simultaneous, 
		EntryPoint_Register*                       entryPoint_Register,
		AbstractContext*                           context );

	TimeIntegrator* _TimeIntegrator_New(
		SizeT                                      _sizeOfSelf,
		Type                                       type,
		Stg_Class_DeleteFunction*                  _delete,
		Stg_Class_PrintFunction*                   _print,
		Stg_Class_CopyFunction*                    _copy, 
		Stg_Component_DefaultConstructorFunction*  _defaultConstructor,
		Stg_Component_ConstructFunction*           _construct,
		Stg_Component_BuildFunction*               _build,
		Stg_Component_InitialiseFunction*          _initialise,
		Stg_Component_ExecuteFunction*             _execute,
		Stg_Component_DestroyFunction*             _destroy,
		Name 							           name );
		
	void _TimeIntegrator_Init(	
		void*                                      timeIntegrator, 
		unsigned int                               order, 
		Bool                                       simultaneous, 
		EntryPoint_Register*                       entryPoint_Register,
		AbstractContext*                           context );

	/* 'Class' Virtual Functions */
	void _TimeIntegrator_Delete( void* timeIntegrator );
	void _TimeIntegrator_Print( void* timeIntegrator, Stream* stream );
	#define TimeIntegrator_Copy( self ) \
		(TimeIntegrator*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define TimeIntegrator_DeepCopy( self ) \
		(TimeIntegrator*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _TimeIntegrator_Copy( void* timeIntegrator, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/* 'Stg_Component' Virtual Functions */
	void* _TimeIntegrator_DefaultNew( Name name ) ;
	void _TimeIntegrator_Construct( void* timeIntegrator, Stg_ComponentFactory* cf, void* data ) ;
	void _TimeIntegrator_Build( void* timeIntegrator, void* data );
	void _TimeIntegrator_Initialise( void* timeIntegrator, void* data );
	void _TimeIntegrator_Execute( void* timeIntegrator, void* data );
	void _TimeIntegrator_Destroy( void* timeIntegrator, void* data ) ;

	/* +++ Private Functions +++ */
	void TimeIntegrator_UpdateClass( void* timeIntegrator, void* data ) ;
	void _TimeIntegrator_ExecuteEuler( void* timeIntegrator, void* data ) ;
	void _TimeIntegrator_ExecuteRK2( void* timeIntegrator, void* data ) ;
	void _TimeIntegrator_ExecuteRK4( void* timeIntegrator, void* data ) ;

	void _TimeIntegrator_ExecuteRK2Simultaneous( void* timeIntegrator, void* data ) ;
	void _TimeIntegrator_ExecuteRK4Simultaneous( void* timeIntegrator, void* data ) ;

	/* +++ Public Functions +++ */
	void TimeIntegrator_Add( void* timeIntegrator, void* _timeIntegratee ) ;
	#define TimeIntegrator_GetCount( self ) \
		((TimeIntegrator*)self)->integrateeRegister->objects->count
	#define TimeIntegrator_GetByIndex( self, index ) \
		( (TimeIntegratee*) NamedObject_Register_GetByIndex( ((TimeIntegrator*)self)->integrateeRegister, index ) )

	void TimeIntegrator_Setup( void* timeIntegrator ) ;
	void TimeIntegrator_AppendSetupEP( void* timeIntegrator, Name name, Func_Ptr funcPtr, char* addedBy, void* data ) ;
	void TimeIntegrator_PrependSetupEP( void* timeIntegrator, Name name, Func_Ptr funcPtr, char* addedBy, void* data ) ;

	void TimeIntegrator_Finalise( void* timeIntegrator ) ;
	void TimeIntegrator_AppendFinishEP( void* timeIntegrator, Name name, Func_Ptr funcPtr, char* addedBy, void* data ) ;
	void TimeIntegrator_PrependFinishEP( void* timeIntegrator, Name name, Func_Ptr funcPtr, char* addedBy, void* data ) ;

	void TimeIntegrator_InsertBeforeFinishEP( 
			void* timeIntegrator, 
			Name hookToInsertBefore,
			Name name, 
			Func_Ptr funcPtr, 
			char* addedBy, 
			void* data );

	void TimeIntegrator_InsertAfterFinishEP( 
			void* timeIntegrator, 
			Name hookToInsertAfter, 
			Name name, 
			Func_Ptr funcPtr, 
			char* addedBy, 
			void* data );
	
	void TimeIntegrator_SetTime( void* timeIntegrator, double time ) ;
	double TimeIntegrator_GetTime( void* timeIntegrator ) ;

	Variable* Variable_NewFromOld( Variable* oldVariable, Name name, Bool copyValues ) ;

#endif 

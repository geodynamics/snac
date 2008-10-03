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
**	Finite Element Constitutive Matrix object.
**
** Assumptions:
**
** Comments:
**
** $Id: TimeIntegratee.h 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Utils_TimeIntegratee_h__
#define __Discretisation_Utils_TimeIntegratee_h__
	
	typedef Bool (TimeIntegratee_CalculateTimeDerivFunction) ( void* timeIntegrator, Index array_I, double* timeDeriv );
	typedef void (TimeIntegratee_IntermediateFunction) ( void* timeIntegrator, Index array_I );

	extern const Type TimeIntegratee_Type;
	
	/* TimeIntegratee information */
	#define __TimeIntegratee  \
		/* General info */ \
		__Stg_Component \
		\
		/* Virtual info */ \
		TimeIntegratee_CalculateTimeDerivFunction* _calculateTimeDeriv;  \
		TimeIntegratee_IntermediateFunction*       _intermediate;  \
		/* Other info */ \
		TimeIntegrator*                            timeIntegrator;       \
		Variable*                                  variable;             \
		Index                                      dataCount;            \
		Stg_Component**                            data;                 \
		Bool                                       allowFallbackToFirstOrder; \
		Stream*                                    debug;                \
		
	struct TimeIntegratee { __TimeIntegratee };
	
	/* Creation implementation / Virtual constructor */
	TimeIntegratee* TimeIntegratee_New( 
		Name                                   name, 
		TimeIntegrator*                        timeIntegrator, 
		Variable*                              variable, 
		Index                                  dataCount, 
		Stg_Component**                        data,
		Bool                                   allowFallbackToFirstOrder );

	TimeIntegratee* _TimeIntegratee_New( 
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
		TimeIntegratee_CalculateTimeDerivFunction* _calculateTimeDeriv,
		TimeIntegratee_IntermediateFunction*       _intermediate,
		Name                                       name );

	void _TimeIntegratee_Init( 
		void*                                      timeIntegratee, 
		TimeIntegrator*                            timeIntegrator, 
		Variable*                                  variable, 
		Index                                      dataCount, 
		Stg_Component**                            data,
		Bool                                       allowFallbackToFirstOrder );
		
	/* 'Class' Virtual Functions */
	void _TimeIntegratee_Delete( void* timeIntegrator );
	void _TimeIntegratee_Print( void* timeIntegrator, Stream* stream );
	#define TimeIntegratee_Copy( self ) \
		(TimeIntegratee*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define TimeIntegratee_DeepCopy( self ) \
		(TimeIntegratee*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _TimeIntegratee_Copy( void* timeIntegrator, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/* 'Stg_Component' Virtual Functions */
	void* _TimeIntegratee_DefaultNew( Name name ) ;
	void _TimeIntegratee_Construct( void* timeIntegratee, Stg_ComponentFactory* cf, void* data ) ;
	void _TimeIntegratee_Build( void* timeIntegrator, void* data );
	void _TimeIntegratee_Initialise( void* timeIntegrator, void* data );
	void _TimeIntegratee_Execute( void* timeIntegrator, void* data );
	void _TimeIntegratee_Destroy( void* timeIntegratee, void* data );

	/* +++ Virtual Functions +++ */
	#define TimeIntegratee_CalculateTimeDeriv( timeIntegratee, array_I, timeDeriv ) \
		( ((TimeIntegratee*) timeIntegratee )->_calculateTimeDeriv( timeIntegratee, array_I, timeDeriv ) )
	#define TimeIntegratee_Intermediate( timeIntegratee, array_I ) \
		( ((TimeIntegratee*) timeIntegratee )->_intermediate( timeIntegratee, array_I ) )

	/* +++ Private Functions +++ */
	Bool _TimeIntegratee_AdvectionTimeDeriv( void* timeIntegratee, Index array_I, double* timeDeriv ) ;
	void _TimeIntegratee_Intermediate( void* timeIntegratee, Index array_I );
	void _TimeIntegratee_RewindToStartAndApplyFirstOrderUpdate( 
		TimeIntegratee* self,
		double*         arrayDataPtr,
		double*         startData,
		double          startTime,
		double          dt,
		double*         timeDeriv,
		Index           array_I );

	/* +++ Public Functions +++ */
	void TimeIntegratee_FirstOrder( void* timeIntegrator, Variable* startValue, double dt );
	void TimeIntegratee_SecondOrder( void* timeIntegrator, Variable* startValue, double dt );
	void TimeIntegratee_FourthOrder( void* timeIntegrator, Variable* startValue, double dt );

	void TimeIntegratee_StoreTimeDeriv( void* timeIntegratee, Variable* timeDeriv ) ;
	void TimeIntegratee_Add2TimesTimeDeriv( void* timeIntegratee, Variable* timeDerivVariable ) ;
	void TimeIntegratee_FourthOrderFinalStep( void* timeIntegratee, Variable* startData, Variable* timeDerivVariable, double dt ) ;

	#define TimeIntegratee_GetTime( timeIntegratee ) \
		TimeIntegrator_GetTime( ((TimeIntegratee*) timeIntegratee)->timeIntegrator ) 

#endif 

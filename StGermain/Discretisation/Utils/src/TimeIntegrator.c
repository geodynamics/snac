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
** $Id: TimeIntegrator.c 3851 2006-10-12 08:57:22Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"
#include "Discretisation/Mesh/Mesh.h"

#include "types.h"
#include "TimeIntegrator.h"
#include "TimeIntegratee.h"

#include <assert.h>
#include <string.h>

/* Textual name of this class */
const Type TimeIntegrator_Type = "TimeIntegrator";

TimeIntegrator* TimeIntegrator_New( 
		Name                                       name,
		unsigned int                               order, 
		Bool                                       simultaneous, 
		EntryPoint_Register*                       entryPoint_Register,
		AbstractContext*                           context )
{
	TimeIntegrator* self;

	self = _TimeIntegrator_DefaultNew( name );
	_TimeIntegrator_Init( self, order, simultaneous, entryPoint_Register, context );

	return self;
}


TimeIntegrator* _TimeIntegrator_New( 
		SizeT                                     _sizeOfSelf,
		Type                                      type,
		Stg_Class_DeleteFunction*                 _delete,
		Stg_Class_PrintFunction*                  _print,
		Stg_Class_CopyFunction*                   _copy, 
		Stg_Component_DefaultConstructorFunction* _defaultConstructor,
		Stg_Component_ConstructFunction*          _construct,
		Stg_Component_BuildFunction*              _build,
		Stg_Component_InitialiseFunction*         _initialise,
		Stg_Component_ExecuteFunction*            _execute,
		Stg_Component_DestroyFunction*            _destroy,
		Name 							          name )
{
	TimeIntegrator*	self;
	
	assert( _sizeOfSelf >= sizeof(TimeIntegrator) );
	
	/* General info */
	self = (TimeIntegrator*)_Stg_Component_New( 
			_sizeOfSelf, 
			type, 
			_delete,
			_print, 
			_copy,
			_defaultConstructor,
			_construct,
			_build,
			_initialise,
			_execute, 
			_destroy,
			name, 
			NON_GLOBAL );
	
	return self;
}

void _TimeIntegrator_Init( 
		void*                                      timeIntegrator, 
		unsigned int                               order, 
		Bool                                       simultaneous, 
		EntryPoint_Register*                       entryPoint_Register,
		AbstractContext*                           context )
{
	TimeIntegrator* self = (TimeIntegrator*)timeIntegrator;

	self->debug              = Journal_Register( Debug_Type, self->type );
	self->info               = Journal_Register( Info_Type, self->type );
		
	self->integrateeRegister = NamedObject_Register_New();
	self->order              = order;
	self->simultaneous       = simultaneous;

	/* Entry Point Stuff */
	Stg_asprintf( &self->_setupEPName, "%s-Setup", self->name );
	Stg_asprintf( &self->_finishEPName, "%s-Finish", self->name );
	self->setupEP  = EntryPoint_New( self->_setupEPName,  EntryPoint_VoidPtr_CastType );
	self->finishEP = EntryPoint_New( self->_finishEPName, EntryPoint_VoidPtr_CastType );

	if ( entryPoint_Register ) {
		EntryPoint_Register_Add( entryPoint_Register, self->setupEP );
		EntryPoint_Register_Add( entryPoint_Register, self->finishEP );
	}

	self->setupData = Stg_ObjectList_New();
	self->finishData = Stg_ObjectList_New();

	if ( context ) {
		EP_AppendClassHook( Context_GetEntryPoint( context, AbstractContext_EP_UpdateClass ), 
				TimeIntegrator_UpdateClass, self );
	}
}

void _TimeIntegrator_Delete( void* timeIntegrator ) {
	TimeIntegrator* self = (TimeIntegrator*)timeIntegrator;
	
	Journal_DPrintf( self->debug, "In %s for %s '%s'\n", __func__, self->type, self->name );
	Memory_Free( self->_setupEPName );
	Memory_Free( self->_finishEPName );
	
	Stg_Class_Delete( self->setupData );
	Stg_Class_Delete( self->finishData );
	
	/* Stg_Class_Delete parent*/
	_Stg_Component_Delete( self );
}

void _TimeIntegrator_Print( void* timeIntegrator, Stream* stream ) {
	TimeIntegrator* self = (TimeIntegrator*)timeIntegrator;

	/* General info */
	Journal_DPrintf( self->debug, "TimeIntegrator - '%s'\n", self->name );
	Journal_PrintPointer( stream, self );
	Stream_Indent( stream );
	
	/* Print parent */
	_Stg_Component_Print( self, stream );
	
	/* Virtual info */

	/* Regular Info */
	Stg_Class_Print( self->integrateeRegister, stream );
	
	Stream_UnIndent( stream );
}

void* _TimeIntegrator_Copy( void* timeIntegrator, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	TimeIntegrator*	self = (TimeIntegrator*)timeIntegrator;
	TimeIntegrator*	newTimeIntegrator;
	
	Journal_DPrintf( self->debug, "In %s for %s '%s'\n", __func__, self->type, self->name );
	/* TODO */ abort();

	return (void*)newTimeIntegrator;
}


void* _TimeIntegrator_DefaultNew( Name name ) {
	return (void*) _TimeIntegrator_New( 
		sizeof(TimeIntegrator), 
		TimeIntegrator_Type, 
		_TimeIntegrator_Delete,
		_TimeIntegrator_Print, 
		_TimeIntegrator_Copy, 
		_TimeIntegrator_DefaultNew, 
		_TimeIntegrator_Construct, 
		_TimeIntegrator_Build, 
		_TimeIntegrator_Initialise,
		_TimeIntegrator_Execute,
		_TimeIntegrator_Destroy, 
		name );
}

void _TimeIntegrator_Construct( void* timeIntegrator, Stg_ComponentFactory* cf, void* data ) {
	TimeIntegrator*          self        = (TimeIntegrator*)timeIntegrator;
	unsigned int             order;
	Bool                     simultaneous;
	EntryPoint_Register*     entryPoint_Register;
	AbstractContext*         context;

	/** Default for order changed to 2nd order (was 1st order) by Pat Sunter, 10 May 2006 */
	order          = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, "order", 2 );
	simultaneous   = Stg_ComponentFactory_GetBool( cf, self->name, "simultaneous", False );
	
	context = Stg_ComponentFactory_ConstructByName( cf, "context", AbstractContext, True, data );

	entryPoint_Register = Stg_ObjectList_Get( cf->registerRegister, "EntryPoint_Register" );
	assert( entryPoint_Register );

	_TimeIntegrator_Init( self, order, simultaneous, entryPoint_Register, context );
}

void _TimeIntegrator_Build( void* timeIntegrator, void* data ) {
	TimeIntegrator* self = (TimeIntegrator*)timeIntegrator;

	Journal_DPrintf( self->debug, "In %s for %s '%s'\n", __func__, self->type, self->name );

}

void _TimeIntegrator_Initialise( void* timeIntegrator, void* data ) {
	TimeIntegrator* self = (TimeIntegrator*)timeIntegrator;
	
	Journal_DPrintf( self->debug, "In %s for %s '%s'\n", __func__, self->type, self->name );

}

void _TimeIntegrator_Execute( void* timeIntegrator, void* data ) {
	TimeIntegrator*	self = (TimeIntegrator*)timeIntegrator;
	double wallTime;


	Journal_DPrintf( self->debug, "In %s for %s '%s'\n", __func__, self->type, self->name );

	wallTime = MPI_Wtime();

	/* Set function pointer */
	switch (self->order) {
		case 1:
			self->_execute = _TimeIntegrator_ExecuteEuler; 
			break;
		case 2:
			if (self->simultaneous) 
				self->_execute = _TimeIntegrator_ExecuteRK2Simultaneous; 
			else
				self->_execute = _TimeIntegrator_ExecuteRK2; 
			break;
		case 4:
			if (self->simultaneous) 
				self->_execute = _TimeIntegrator_ExecuteRK4Simultaneous; 
			else
				self->_execute = _TimeIntegrator_ExecuteRK4; 
			break;
		default:
			Journal_Firewall( False, Journal_Register( Error_Type, self->type ),
					"%s '%s' cannot handle order %u\n", self->type, self->name, self->order );
	}

	/* Call real function */
	
	Journal_Printf( self->info, "Time Integration\n" );
	self->_execute( self, data );
	Journal_Printf(self->info,"Time Integration - %.6g (secs)\n", MPI_Wtime()-wallTime);
	
}

void _TimeIntegrator_Destroy( void* timeIntegrator, void* data ) {
	TimeIntegrator*	self = (TimeIntegrator*)timeIntegrator;

	Journal_DPrintf( self->debug, "In %s for %s '%s'\n", __func__, self->type, self->name );
}

void TimeIntegrator_UpdateClass( void* timeIntegrator, void* data ) {
	TimeIntegrator*        self            = (TimeIntegrator*) timeIntegrator;
	double wallTime;
	
	wallTime = MPI_Wtime();
	Journal_Printf(self->info,"Time Integration\n");
	self->_execute( self, data );
	Journal_Printf(self->info,"Time Integration - %.4g (secs)\n", MPI_Wtime()-wallTime);	
}

/* +++ Private Functions +++ */
void _TimeIntegrator_ExecuteEuler( void* timeIntegrator, void* data ) {
	TimeIntegrator*        self            = (TimeIntegrator*) timeIntegrator;
	AbstractContext*       context         = (AbstractContext*) data;
	Index                  integratee_I;   
	Index                  integrateeCount = TimeIntegrator_GetCount( self );
	double                 dt              = AbstractContext_Dt( context );
	TimeIntegratee*        integratee;
	double wallTime;
	
	Journal_DPrintf( self->debug, "In %s for %s '%s'\n", __func__, self->type, self->name );

	/* Set Time */
	TimeIntegrator_SetTime( self, context->currentTime );
	
	TimeIntegrator_Setup( self );
	for ( integratee_I = 0 ; integratee_I < integrateeCount ; integratee_I++ ) {
		integratee = TimeIntegrator_GetByIndex( self, integratee_I );
	
		wallTime = MPI_Wtime();
		TimeIntegratee_FirstOrder( integratee, integratee->variable, dt );
		Journal_Printf(self->info,"\t1st order: %35s - %9.4f (secs)\n", integratee->name, MPI_Wtime()-wallTime);
	}
	TimeIntegrator_Finalise( self );
}

void _TimeIntegrator_ExecuteRK2( void* timeIntegrator, void* data ) {
	TimeIntegrator*        self            = (TimeIntegrator*) timeIntegrator;
	AbstractContext*       context         = (AbstractContext*) data;
	Index                  integratee_I;   
	Index                  integrateeCount = TimeIntegrator_GetCount( self );
	double                 dt              = AbstractContext_Dt( context );
	TimeIntegratee*        integratee;
	double wallTime;

	Journal_DPrintf( self->debug, "In %s for %s '%s'\n", __func__, self->type, self->name );

	
	wallTime = MPI_Wtime();
	TimeIntegrator_Setup( self );
	
	for ( integratee_I = 0 ; integratee_I < integrateeCount ; integratee_I++ ) {
		integratee = TimeIntegrator_GetByIndex( self, integratee_I );
				
		TimeIntegrator_SetTime( self, context->currentTime );
		
		wallTime = MPI_Wtime();
		TimeIntegratee_SecondOrder( integratee, integratee->variable, dt );
		Journal_Printf(self->info,"\t2nd order: %35s - %9.4f (secs)\n", integratee->name, MPI_Wtime()-wallTime);
		
	}
	
	TimeIntegrator_Finalise( self );
	
}


void _TimeIntegrator_ExecuteRK4( void* timeIntegrator, void* data ) {
	TimeIntegrator*        self            = (TimeIntegrator*) timeIntegrator;
	AbstractContext*       context         = (AbstractContext*) data;
	Index                  integratee_I;   
	Index                  integrateeCount = TimeIntegrator_GetCount( self );
	double                 dt              = AbstractContext_Dt( context );
	TimeIntegratee*        integratee;
	double wallTime;

	Journal_DPrintf( self->debug, "In %s for %s '%s'\n", __func__, self->type, self->name );

	TimeIntegrator_Setup( self );
	for ( integratee_I = 0 ; integratee_I < integrateeCount ; integratee_I++ ) {
		integratee = TimeIntegrator_GetByIndex( self, integratee_I );

		TimeIntegrator_SetTime( self, context->currentTime );
		wallTime = MPI_Wtime();
		TimeIntegratee_FourthOrder( integratee, integratee->variable, dt );
		Journal_Printf(self->info,"\t4th order: %35s - %9.4f (secs)\n", integratee->name, MPI_Wtime()-wallTime);
	}
	TimeIntegrator_Finalise( self );
}


void _TimeIntegrator_ExecuteRK2Simultaneous( void* timeIntegrator, void* data ) {
	TimeIntegrator*        self            = (TimeIntegrator*)timeIntegrator;
	AbstractContext*       context         = (AbstractContext*) data;
	Index                  integratee_I;   
	Index                  integrateeCount = TimeIntegrator_GetCount( self );
	double                 dt              = AbstractContext_Dt( context );
	TimeIntegratee*        integratee;
	Variable**             originalVariableList;

	Journal_DPrintf( self->debug, "In %s for %s '%s'\n", __func__, self->type, self->name );

	Journal_Firewall( 
			False,
			Journal_MyStream( Error_Type, self ),
			"Error in %s '%s' - This function is temporarily unavailable \n"
			"Please only use non-simultaneous update or only first order update.\n", 
			self->type, self->name );

	/* Set Time */
	TimeIntegrator_SetTime( self, context->currentTime );

	originalVariableList = Memory_Alloc_Array( Variable*, integrateeCount, "originalVariableList" );
	
	TimeIntegrator_Setup( self );
	for ( integratee_I = 0 ; integratee_I < integrateeCount ; integratee_I++ ) {
		integratee = TimeIntegrator_GetByIndex( self, integratee_I );
		Journal_Printf(self->info,"\t2nd order (simultaneous): %s\n", integratee->name);
		
		/* Store Original */
		originalVariableList[ integratee_I ] = Variable_NewFromOld( integratee->variable, "Original", True );

		/* Predictor Step */
		TimeIntegratee_FirstOrder( integratee, integratee->variable, 0.5 * dt );
	}
	TimeIntegrator_Finalise( self );
	
	/* Set Time */
	TimeIntegrator_SetTime( self, context->currentTime + 0.5 * dt );

	TimeIntegrator_Setup( self );
	for ( integratee_I = 0 ; integratee_I < integrateeCount ; integratee_I++ ) {
		integratee = TimeIntegrator_GetByIndex( self, integratee_I );

		/* Corrector Step */
		TimeIntegratee_FirstOrder( integratee, originalVariableList[ integratee_I ], dt );

		/* Free Original */
		Stg_Class_Delete( originalVariableList[ integratee_I ] );
	}
	TimeIntegrator_Finalise( self );
	Memory_Free( originalVariableList );
}

void _TimeIntegrator_ExecuteRK4Simultaneous( void* timeIntegrator, void* data ) {
	TimeIntegrator*        self            = (TimeIntegrator*)timeIntegrator;
	AbstractContext*       context         = (AbstractContext*) data;
	Index                  integratee_I;   
	Index                  integrateeCount = TimeIntegrator_GetCount( self );
	double                 dt              = AbstractContext_Dt( context );
	TimeIntegratee*        integratee;
	Variable**             originalVariableList;
	Variable**             timeDerivVariableList;

	Journal_DPrintf( self->debug, "In %s for %s '%s'\n", __func__, self->type, self->name );

	/* Set Time */
	TimeIntegrator_SetTime( self, context->currentTime );
	
	originalVariableList  = Memory_Alloc_Array( Variable*, integrateeCount, "originalVariableList" );
	timeDerivVariableList = Memory_Alloc_Array( Variable*, integrateeCount, "timeDerivVariableList" );

	/* First Step */
	TimeIntegrator_Setup( self );
	for ( integratee_I = 0 ; integratee_I < integrateeCount ; integratee_I++ ) {
		integratee = TimeIntegrator_GetByIndex( self, integratee_I );
		Journal_Printf(self->info,"\t2nd order (simultaneous): %s\n", integratee->name);

		/* Store Original Position Variable */
		originalVariableList[ integratee_I ]  = Variable_NewFromOld( integratee->variable, "Original", True );
		timeDerivVariableList[ integratee_I ] = Variable_NewFromOld( integratee->variable, "k1+2k2+2k3", False );

		/* Store k1 */
		TimeIntegratee_StoreTimeDeriv( integratee, timeDerivVariableList[ integratee_I ] );

		/* 1st Step */
		TimeIntegratee_FirstOrder( integratee, integratee->variable, 0.5 * dt );
	}
	TimeIntegrator_Finalise( self );
	
	/* Set Time */
	TimeIntegrator_SetTime( self, context->currentTime + 0.5 * dt );
	
	/* Second Step */
	TimeIntegrator_Setup( self );
	for ( integratee_I = 0 ; integratee_I < integrateeCount ; integratee_I++ ) {
		integratee = TimeIntegrator_GetByIndex( self, integratee_I );

		/* Add k2 */
		TimeIntegratee_Add2TimesTimeDeriv( integratee, timeDerivVariableList[ integratee_I ] );

		TimeIntegratee_FirstOrder( integratee, originalVariableList[ integratee_I ], 0.5 * dt );
	}
	TimeIntegrator_Finalise( self );

	TimeIntegrator_Setup( self );
	for ( integratee_I = 0 ; integratee_I < integrateeCount ; integratee_I++ ) {
		integratee = TimeIntegrator_GetByIndex( self, integratee_I );
		
		/* Add k3 */
		TimeIntegratee_Add2TimesTimeDeriv( integratee, timeDerivVariableList[ integratee_I ] );

		/* 3rd Step */
		TimeIntegratee_FirstOrder( integratee, originalVariableList[ integratee_I ], dt );
	}
	TimeIntegrator_Finalise( self );
	
	/* Set Time */
	TimeIntegrator_SetTime( self, context->currentTime + dt );
	
	TimeIntegrator_Setup( self );
	for ( integratee_I = 0 ; integratee_I < integrateeCount ; integratee_I++ ) {
		integratee = TimeIntegrator_GetByIndex( self, integratee_I );

		TimeIntegratee_FourthOrderFinalStep( integratee, originalVariableList[ integratee_I ], timeDerivVariableList[ integratee_I ], dt );

		/* Free Original */
		Stg_Class_Delete( timeDerivVariableList[ integratee_I ] );
		Stg_Class_Delete( originalVariableList[ integratee_I ] );
	}
	TimeIntegrator_Finalise( self );

	Memory_Free( originalVariableList );
	Memory_Free( timeDerivVariableList );
}



/* +++ Public Functions +++ */

void TimeIntegrator_Add( void* timeIntegrator, void* _timeIntegratee ) {
	TimeIntegrator*	self           = (TimeIntegrator*) timeIntegrator;
	TimeIntegratee*	timeIntegratee = (TimeIntegratee*)_timeIntegratee;

	NamedObject_Register_Add( self->integrateeRegister, timeIntegratee );
}

void TimeIntegrator_Setup( void* timeIntegrator ) {
	TimeIntegrator*        self            = (TimeIntegrator*)timeIntegrator;
	EntryPoint*            entryPoint      = self->setupEP;
	Hook_Index             hookIndex;
	double wallTime;
	
	/* Shouldn't this be using a call to a run function of the entry point class ? */ 
	for( hookIndex = 0; hookIndex < entryPoint->hooks->count; hookIndex++ ) {
		wallTime = MPI_Wtime();
		
		((EntryPoint_2VoidPtr_Cast*)((Hook*)entryPoint->hooks->data[hookIndex])->funcPtr)(
			self, Stg_ObjectList_At( self->setupData, hookIndex ) );
			
		Journal_Printf(self->info,"\t       EP: %35s - %9.4f (secs)\n",(entryPoint->hooks->data[hookIndex])->name,
			MPI_Wtime()-wallTime);	
	}		
}

void TimeIntegrator_AppendSetupEP( void* timeIntegrator, Name name, Func_Ptr funcPtr, char* addedBy, void* data ) {
	TimeIntegrator*        self            = (TimeIntegrator*)timeIntegrator;

	EntryPoint_Append( self->setupEP, name, funcPtr, addedBy );
	Stg_ObjectList_Append( self->setupData, data );
}

void TimeIntegrator_PrependSetupEP( void* timeIntegrator, Name name, Func_Ptr funcPtr, char* addedBy, void* data ) {
	TimeIntegrator*        self            = (TimeIntegrator*)timeIntegrator;

	EntryPoint_Prepend( self->setupEP, name, funcPtr, addedBy );
	Stg_ObjectList_Prepend( self->setupData, data );
}
	

void TimeIntegrator_Finalise( void* timeIntegrator ) {
	TimeIntegrator*        self            = (TimeIntegrator*)timeIntegrator;
	EntryPoint*            entryPoint      = self->finishEP;
	Hook_Index             hookIndex;
	double 				  wallTime;
	
	for( hookIndex = 0; hookIndex < entryPoint->hooks->count; hookIndex++ ) {
		wallTime = MPI_Wtime();
		
		((EntryPoint_2VoidPtr_Cast*)((Hook*)entryPoint->hooks->data[hookIndex])->funcPtr)(
			self, Stg_ObjectList_At( self->finishData, hookIndex ) );
		
		Journal_Printf(self->info,"\t       EP: %35s - %9.4f (secs)\n",(entryPoint->hooks->data[hookIndex])->name,
				MPI_Wtime()-wallTime);	


	}
}


void TimeIntegrator_AppendFinishEP( void* timeIntegrator, Name name, Func_Ptr funcPtr, char* addedBy, void* data ) {
	TimeIntegrator*        self            = (TimeIntegrator*)timeIntegrator;

	EntryPoint_Append( self->finishEP, name, funcPtr, addedBy );
	Stg_ObjectList_Append( self->finishData, data );
}

void TimeIntegrator_PrependFinishEP( void* timeIntegrator, Name name, Func_Ptr funcPtr, char* addedBy, void* data ) {
	TimeIntegrator*        self            = (TimeIntegrator*)timeIntegrator;

	EntryPoint_Prepend( self->finishEP, name, funcPtr, addedBy );
	Stg_ObjectList_Prepend( self->finishData, data );
}

void TimeIntegrator_InsertBeforeFinishEP( 
	void* timeIntegrator, 
	Name hookToInsertBefore,
	Name name, 
	Func_Ptr funcPtr, 
	char* addedBy, 
	void* data ) 
{
	TimeIntegrator*        self            = (TimeIntegrator*)timeIntegrator;

	if ( Stg_ObjectList_GetIndex( self->finishEP->hooks, hookToInsertBefore ) != (unsigned int)-1 ) {
		EntryPoint_InsertBefore( self->finishEP, hookToInsertBefore, name, funcPtr, addedBy );
		_Stg_ObjectList_InsertAtIndex( 
				self->finishData,
				Stg_ObjectList_GetIndex( self->finishEP->hooks, name ),
				data );
	}
	else {
		TimeIntegrator_AppendFinishEP( self, name, funcPtr, addedBy, data );
	}
}

void TimeIntegrator_InsertAfterFinishEP( 
	void* timeIntegrator, 
	Name hookToInsertAfter, 
	Name name, 
	Func_Ptr funcPtr, 
	char* addedBy, 
	void* data ) 
{
	TimeIntegrator*        self            = (TimeIntegrator*)timeIntegrator;

	if ( Stg_ObjectList_GetIndex( self->finishEP->hooks, hookToInsertAfter ) != (unsigned int)-1 ) {
		EntryPoint_InsertAfter( self->finishEP, hookToInsertAfter, name, funcPtr, addedBy );
		_Stg_ObjectList_InsertAtIndex(
				self->finishData,
				Stg_ObjectList_GetIndex( self->finishEP->hooks, name ),
				data );
	}
	else {
		TimeIntegrator_AppendFinishEP( self, name, funcPtr, addedBy, data );
	}
}

void TimeIntegrator_SetTime( void* timeIntegrator, double time ) {
	TimeIntegrator*        self            = (TimeIntegrator*)timeIntegrator;

	self->time = time;
}

double TimeIntegrator_GetTime( void* timeIntegrator ) {
	TimeIntegrator*        self            = (TimeIntegrator*)timeIntegrator;
	return self->time;
}

Variable* Variable_NewFromOld( Variable* oldVariable, Name name, Bool copyValues ) {
	Variable*         self;
	Index             array_I;
	SizeT             dataOffsets[] = { 0 };
	void*             myPtr;
	void*             oldPtr;

	Variable_Update( oldVariable );

	self = Variable_New(  
			name,
			1,
			dataOffsets,
			oldVariable->dataTypes,
			oldVariable->dataTypeCounts,
			NULL,
			0,
			oldVariable->arraySizePtr,
			NULL,
			NULL );

	self->allocateSelf = True;
	self->arrayPtrPtr = &self->arrayPtr;
	if ( oldVariable->isBuilt )
		Stg_Component_Build( self, NULL, True );
	if ( oldVariable->isInitialised )
		Stg_Component_Initialise( self, NULL, True );

	assert(self->offsetCount == 1);

	if ( copyValues ) {
		for ( array_I = 0 ; array_I < self->arraySize ; array_I++ ) {
			myPtr = Variable_GetStructPtr( self, array_I );
			oldPtr = Variable_GetStructPtr( oldVariable, array_I );
			memcpy( myPtr, oldPtr, self->dataSizes[0] ); 
		}
	}

	return self;
}

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
** $Id: AbstractContext.c 3857 2006-10-14 18:06:05Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdarg.h>
#include <mpi.h>  /* subsequent files need this */
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"
#include "Base/Automation/Automation.h"
#include "Base/Extensibility/Extensibility.h"

#include "units.h"
#include "types.h"
#include "shortcuts.h"
#include "AbstractContext.h"
#include "ContextEntryPoint.h"
#include "DictionaryCheck.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/* AbstractContext entry point names */
Type AbstractContext_EP_Construct =		"Context_Construct";
Type AbstractContext_EP_ConstructExtensions = 	"Context_ConstructExtensions";
Type AbstractContext_EP_Build = 		"Context_Build";
Type AbstractContext_EP_Initialise =		"Context_Initialise";
Type AbstractContext_EP_Execute =		"Context_Execute";
Type AbstractContext_EP_Destroy =		"Context_Destroy";
Type AbstractContext_EP_DestroyExtensions = 	"Context_DestroyExtensions";

Type AbstractContext_EP_Dt =			"Context_Dt";
Type AbstractContext_EP_Step =			"Context_Step";
Type AbstractContext_EP_UpdateClass =		"Context_UpdateClass";
Type AbstractContext_EP_Solve =			"Context_Solve";
Type AbstractContext_EP_Sync =			"Context_Sync";
Type AbstractContext_EP_FrequentOutput = 	"Context_FrequentOutput";
Type AbstractContext_EP_Dump =			"Context_Dump";
Type AbstractContext_EP_DumpClass =		"Context_DumpClass";
Type AbstractContext_EP_Save =			"Context_Save";
Type AbstractContext_EP_SaveClass =		"Context_SaveClass";


/* Dictionary entry names */
const Type AbstractContext_Dict_Components =	"components";
const Type AbstractContext_Dict_Applications =	"application_plugins";


/* Class stuff ********************************************************************************************************************/


/* Textual name of this class */
const Type AbstractContext_Type = "Context";
const Type AbstractContext_Type_Verbose = "Context-verbose";

AbstractContext* _AbstractContext_New(
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
		Bool						initFlag,
		AbstractContext_SetDt*				_setDt,
		double						startTime,
		double						stopTime,
		MPI_Comm					communicator,
		Dictionary*					dictionary )
{
	AbstractContext* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(AbstractContext) );
	self = (AbstractContext*)_Stg_Component_New( _sizeOfSelf, type, _delete, _print, _copy, _defaultConstructor, _construct, 
			_build, _initialise, _execute, _destroy, name, NON_GLOBAL );
	
	/* General info */
	self->dictionary = dictionary;
	
	/* Virtual info */
	self->_setDt = _setDt;
	
	if( initFlag ){
		_AbstractContext_Init( self, startTime, stopTime, communicator );
	}
	
	return self;
}

void _AbstractContext_Init(
		AbstractContext* 		self,
		double				startTime,
		double				stopTime,
		MPI_Comm			communicator )
{
	Stream* debug = Journal_Register( DebugStream_Type, AbstractContext_Type );
	Dictionary_Entry_Value* dictEntryVal = NULL;
	Dictionary *componentDict = NULL;
	char buf[80];
	
	/* General and Virtual info should already be set */
	
	/* AbstractContext info */
	self->isConstructed = True;
	self->communicator = communicator;
	MPI_Comm_rank( self->communicator, &self->rank );
	MPI_Comm_size( self->communicator, &self->nproc );
	self->debug = debug;
	if( self->rank == 0 ) {
		Journal_Printf( 
			debug, 
			"In: %s: self->communicator: %u, self->nproc: %u, self->rank %u\n", 
			__func__,
			self->communicator,
			self->nproc,
			self->rank );
	}
	self->info = Journal_Register( InfoStream_Type, AbstractContext_Type );
	self->verbose = Journal_Register( InfoStream_Type, AbstractContext_Type_Verbose );
	sprintf( buf, "journal.info.%s", AbstractContext_Type_Verbose );
	if( !Dictionary_Get( self->dictionary, buf ) ) {
		Journal_Enable_NamedStream( InfoStream_Type, AbstractContext_Type_Verbose, False );
	}
	/* Turn off the journal warning debug stream by default: even if debug is enabled in general */
	if( !Dictionary_Get( self->dictionary, "journal.debug.DictionaryWarning" ) ) {
		Journal_Enable_NamedStream( DebugStream_Type, "DictionaryWarning", False );
	}
	
	/* Set up the registers and plugin manager */
	self->CF = 0; /* gets built later */
	self->objectList = Stg_ObjectList_New();
	self->condFunc_Register = ConditionFunction_Register_New();
	self->variable_Register = Variable_Register_New();
	self->extensionMgr_Register = ExtensionManager_Register_New();
	self->extensionMgr = ExtensionManager_New_OfExistingObject( self->type, self );
	ExtensionManager_Register_Add( self->extensionMgr_Register, self->extensionMgr );
	self->pointer_Register = Stg_ObjectList_New();
	self->register_Register = Stg_ObjectList_New();

	self->plugins = PluginsManager_New( self->dictionary );
	PluginsManager_AddDirectory( self->plugins, "StGermain", LIB_DIR );
	
	/* Main input parameters */
	self->frequentOutputEvery = Dictionary_Entry_Value_AsUnsignedInt( 
		Dictionary_GetDefault( self->dictionary, "outputEvery", Dictionary_Entry_Value_FromUnsignedInt( 1 ) ) );
	self->dumpEvery = Dictionary_Entry_Value_AsUnsignedInt( 
		Dictionary_GetDefault( self->dictionary, "dumpEvery", Dictionary_Entry_Value_FromUnsignedInt( 10 ) ) );
	self->checkpointEvery = Dictionary_Entry_Value_AsUnsignedInt( 
		Dictionary_GetDefault( self->dictionary, "checkpointEvery", Dictionary_Entry_Value_FromUnsignedInt( 0 ) ) );
	self->experimentName = StG_Strdup( Dictionary_Entry_Value_AsString( 
		Dictionary_GetDefault( self->dictionary, "experimentName", Dictionary_Entry_Value_FromString( "experiment" ) ) ) );
	self->outputPath = StG_Strdup( Dictionary_Entry_Value_AsString( 
		Dictionary_GetDefault( self->dictionary, "outputPath", Dictionary_Entry_Value_FromString( "./" ) ) ) );

	if ( self->rank == 0 ) {
		if ( ! Stg_DirectoryExists( self->outputPath ) ) {
			Bool ret;

			if ( Stg_FileExists( self->outputPath ) ) {
				Journal_Firewall( 
					0, 
					self->info, 
					"outputPath '%s' is a file an not a directory! Exiting...\n", self->outputPath );
			}
			
			Journal_Printf( self->info, "outputPath '%s' does not exist, attempting to create...\n", self->outputPath );
			ret = Stg_CreateDirectory( self->outputPath );
			Journal_Firewall( ret, self->info, "Unable to create non-existing outputPath to '%s'\n", self->outputPath );
			/* else */
			Journal_Printf( self->info, "outputPath '%s' successfully created!\n", self->outputPath );
		}
	}

	if ( self->rank == 0 ) {
		XML_IO_Handler* ioHandler;
		char* inputfileRecord;

		Stream* s = Journal_Register( Info_Type, XML_IO_Handler_Type );

		/* Avoid confusing messages from XML_IO_Handler...turn it off temporarily */
		Bool isEnabled = Stream_IsEnable( s );
		Stream_EnableSelfOnly( s, False );

		ioHandler = XML_IO_Handler_New();

		/* Set file name */
		Stg_asprintf( &inputfileRecord, "%s/%s", self->outputPath, "input.xml" );

		IO_Handler_WriteAllToFile( ioHandler, inputfileRecord, self->dictionary );
		
		Stream_EnableSelfOnly( s, isEnabled );

		Stg_Class_Delete( ioHandler );
		Memory_Free( inputfileRecord );
	}


	/* Note: these try for deprecated keys "start", "end" and "stop" as well as new ones "startTime" and
		"stopTime" - Main.PatrickSunter - 4 November 2004 */
	dictEntryVal = Dictionary_Get( self->dictionary, "start" );
	if ( NULL == dictEntryVal ) {
		dictEntryVal = Dictionary_GetDefault( self->dictionary, "startTime",
			Dictionary_Entry_Value_FromDouble( startTime ) );
	}
	self->startTime = Dictionary_Entry_Value_AsDouble( dictEntryVal );

	dictEntryVal = Dictionary_Get( self->dictionary, "end" );
	if ( NULL == dictEntryVal ) {
		dictEntryVal = Dictionary_Get( self->dictionary, "stop" );
		if ( NULL == dictEntryVal ) {
			dictEntryVal = Dictionary_GetDefault( self->dictionary, "stopTime",
				Dictionary_Entry_Value_FromDouble( stopTime ) );
		}
	} 
	self->stopTime = Dictionary_Entry_Value_AsDouble( dictEntryVal );

	/* maxTimeSteps of 0 means no maximum applied */
	/* Note: these try for deprecated key "maxLoops" as well as new one "maxTimeSteps" - Main.PatrickSunter - 4 November 2004 */
	dictEntryVal = Dictionary_Get( self->dictionary, "maxLoops" );
	if ( NULL == dictEntryVal ) {
		dictEntryVal = Dictionary_GetDefault( self->dictionary, "maxTimeSteps",
			Dictionary_Entry_Value_FromUnsignedInt( 0 ) );
	}
	self->maxTimeSteps = Dictionary_Entry_Value_AsUnsignedInt( dictEntryVal );

	self->finalTimeStep = Dictionary_GetUnsignedInt_WithDefault( self->dictionary, "finalTimeStep", 0 );
	self->gracefulQuit = False;


	// TODO: does this need to be read from checkpoint file???
	self->currentTime = self->startTime;
	self->timeStep = 0;
	self->timeStepSinceJobRestart = 0;
	
	/* Read in the checkpointing info */
	self->restartTimestep = Dictionary_GetUnsignedInt_WithDefault( self->dictionary, "restartTimestep", 0 );
	self->checkPointPrefixString = Dictionary_GetString_WithDefault( self->dictionary, "checkPointPrefixString", "" );
	if ( self->restartTimestep != 0 ) {
		double dtFromFile;
		self->loadFromCheckPoint = True;
		self->timeStep = self->restartTimestep;
		_AbstractContext_LoadTimeInfoFromCheckPoint( self, self->restartTimestep, &dtFromFile );
	}
	else {
		self->loadFromCheckPoint = False;
	}

	/* Build the entryPoint table */
	self->entryPoint_Register = EntryPoint_Register_New(); 
	/* For the construct EP, override the run function such that the context/ptrToContext remain in sync in the loop. */
	self->constructK = Context_AddEntryPoint( 
		self, 
		ContextEntryPoint_New( AbstractContext_EP_Construct, EntryPoint_2VoidPtr_CastType ) );
	AbstractContext_GetEntryPoint( self, AbstractContext_EP_Construct )->_getRun = _AbstractContext_Construct_EP_GetRun;
	AbstractContext_GetEntryPoint( self, AbstractContext_EP_Construct )->run = 
		EntryPoint_GetRun( AbstractContext_GetEntryPoint( self, AbstractContext_EP_Construct ) );
	self->constructExtensionsK = Context_AddEntryPoint( 
		self, 
		ContextEntryPoint_New( AbstractContext_EP_ConstructExtensions, EntryPoint_VoidPtr_CastType ) );
	self->buildK = Context_AddEntryPoint( 
		self, 
		ContextEntryPoint_New( AbstractContext_EP_Build, EntryPoint_VoidPtr_CastType ) );
	self->initialiseK = Context_AddEntryPoint( 
		self, 
		ContextEntryPoint_New( AbstractContext_EP_Initialise, EntryPoint_VoidPtr_CastType ) );
	self->executeK = Context_AddEntryPoint( 
		self, 
		ContextEntryPoint_New( AbstractContext_EP_Execute, EntryPoint_VoidPtr_CastType ) );
	self->destroyK = Context_AddEntryPoint( 
		self, 
		ContextEntryPoint_New( AbstractContext_EP_Destroy, EntryPoint_VoidPtr_CastType ) );
	self->destroyExtensionsK = Context_AddEntryPoint( 
		self, 
		ContextEntryPoint_New( AbstractContext_EP_DestroyExtensions, EntryPoint_VoidPtr_CastType ) );
	
	self->dtK = Context_AddEntryPoint( 
		self, 
		ContextEntryPoint_New( AbstractContext_EP_Dt, ContextEntryPoint_Dt_CastType ) );
	self->stepK = Context_AddEntryPoint( 
		self, 
		ContextEntryPoint_New( AbstractContext_EP_Step, ContextEntryPoint_Step_CastType ) );
	self->solveK = Context_AddEntryPoint( 
		self, 
		ContextEntryPoint_New( AbstractContext_EP_Solve, EntryPoint_VoidPtr_CastType ) );
	self->updateClassK = Context_AddEntryPoint( 
		self, 
		ContextEntryPoint_New( AbstractContext_EP_UpdateClass, EntryPoint_Class_VoidPtr_CastType ) );
	self->syncK = Context_AddEntryPoint( 
		self, 
		ContextEntryPoint_New( AbstractContext_EP_Sync, EntryPoint_VoidPtr_CastType ) );
	self->frequentOutputK =	Context_AddEntryPoint( 
		self, 
		ContextEntryPoint_New( AbstractContext_EP_FrequentOutput, EntryPoint_VoidPtr_CastType ) );
	self->dumpK = Context_AddEntryPoint( 
		self, 
		ContextEntryPoint_New( AbstractContext_EP_Dump, EntryPoint_VoidPtr_CastType ) );
	self->dumpClassK = Context_AddEntryPoint( 
		self, 
		ContextEntryPoint_New( AbstractContext_EP_DumpClass, EntryPoint_Class_VoidPtr_CastType ) );
	self->saveK = Context_AddEntryPoint( 
		self, 
		ContextEntryPoint_New( AbstractContext_EP_Save, EntryPoint_VoidPtr_CastType ) );
	self->saveClassK = Context_AddEntryPoint( 
		self, 
		ContextEntryPoint_New( AbstractContext_EP_SaveClass, EntryPoint_Class_VoidPtr_CastType ) );
	
	/* add initial hooks */
	EntryPoint_Append( 
		AbstractContext_GetEntryPoint( self, AbstractContext_EP_Construct ),
		"default", 
		(Func_Ptr)_AbstractContext_Construct_Hook, 
		AbstractContext_Type );
	EntryPoint_Append( 
		AbstractContext_GetEntryPoint( self, AbstractContext_EP_Execute ),
		"default", 
		(Func_Ptr)_AbstractContext_Execute_Hook, 
		AbstractContext_Type );
	EntryPoint_Append( 
		AbstractContext_GetEntryPoint( self, AbstractContext_EP_Step ),
		"default", 
		(Func_Ptr)_AbstractContext_Step, 
		AbstractContext_Type );
	EntryPoint_Append( 
		AbstractContext_GetEntryPoint( self, AbstractContext_EP_Save ),
		"SaveTimeInfo", 
		(Func_Ptr)_AbstractContext_SaveTimeInfo, 
		AbstractContext_Type );

	Stg_ObjectList_ClassAppend( self->register_Register, (void*)self->extensionMgr_Register, "ExtensionManager_Register" );
	Stg_ObjectList_ClassAppend( self->register_Register, (void*)self->variable_Register, "Variable_Register" );
	Stg_ObjectList_ClassAppend( self->register_Register, (void*)self->condFunc_Register, "ConditionFunction_Register" );
	Stg_ObjectList_ClassAppend( self->register_Register, (void*)self->entryPoint_Register, "EntryPoint_Register" );
	Stg_ObjectList_ClassAppend( self->register_Register, (void*)self->pointer_Register, "Pointer_Register" );
	
	componentDict = Dictionary_GetDictionary( self->dictionary, "components" );

	/* Add functions to make sure all the live components are built and initialised */
	EntryPoint_Append( 
		Context_GetEntryPoint( self, AbstractContext_EP_Build ),
		"BuildAllLiveComponents", 
		AbstractContext_BuildAllLiveComponents, 
		AbstractContext_Type );
	EntryPoint_Append( 
		Context_GetEntryPoint( self, AbstractContext_EP_Initialise ),
		"InitialiseAllLiveComponents", 
		AbstractContext_InitialiseAllLiveComponents, 
		AbstractContext_Type );

	/* Check if we have been provided a constant to multiply our calculated dt values by. */
	self->dtFactor = Dictionary_GetDouble_WithDefault( self->dictionary, "timestepFactor", 1.0 );
}

void _AbstractContext_Delete( void* abstractContext ) {
	AbstractContext* self = (AbstractContext*)abstractContext;

	Stg_Class_Delete( self->objectList );
	Stg_Class_Delete( self->condFunc_Register );
	Stg_Class_Delete( self->variable_Register );

	Stg_Class_Delete( self->entryPoint_Register );

	Memory_Free( self->experimentName );
	Memory_Free( self->outputPath );
	
	Stg_Class_Delete( self->extensionMgr_Register );
	Stg_Class_Delete( self->plugins );
	
	/* Stg_Class_Delete parent */
	_Stg_Class_Delete( self );
}

void _AbstractContext_Print( void* abstractContext, Stream* stream ) {
	AbstractContext* self = (AbstractContext*)abstractContext;
	
	/* General info */
	Journal_Printf( (void*) stream, "AbstractContext (ptr)(%p):\n", self );
	Journal_Printf( (void*) stream, "\tdictionary (ptr): %p\n", self->dictionary );
	
	/* Virtual info */
	Journal_Printf( (void*) stream, "\t_setDt (ptr): %p\n", self->_setDt );
	
	/* AbstractContext info */
	Journal_Printf( (void*) stream, "\tcommunicator: %i\n", self->communicator );
	Journal_Printf( (void*) stream, "\tstartTime: %g\n", self->startTime );
	Journal_Printf( (void*) stream, "\tstopTime: %g\n", self->stopTime );
	Journal_Printf( (void*) stream, "\tcurrentTime: %g\n", self->currentTime );
	Journal_Printf( (void*) stream, "\ttimeStep: %u\n", self->timeStep );
	Journal_Printf( (void*) stream, "\ttimeStepSinceJobRestart: %u\n", self->timeStepSinceJobRestart );
	Journal_Printf( (void*) stream, "\tmaxTimeSteps: %u\n", self->maxTimeSteps );
	Journal_Printf( (void*) stream, "\tfinalTimeStep: %u\n", self->finalTimeStep );
	Journal_Printf( (void*) stream, "\toutputEvery: %u\n", self->frequentOutputEvery );
	Journal_Printf( (void*) stream, "\tdumpEvery: %u\n", self->dumpEvery );
	Journal_Printf( (void*) stream, "\tcheckpointEvery: %u\n", self->checkpointEvery );
	if( self->outputPath ) {
		Journal_Printf( (void*) stream, "\toutputPath: %s\n", self->outputPath );
	}
	else {
		Journal_Printf( (void*) stream, "\toutputPath: (null)\n" );
	}
	Journal_Printf( stream, "\tloadFromCheckPoint: %u\n", self->loadFromCheckPoint );
	Journal_Printf( stream, "\trestartTimestep: %u\n", self->restartTimestep );
	Journal_Printf( stream, "\tcheckPointPrefixString: %s\n", self->checkPointPrefixString );
	
	Print( self->entryPoint_Register, stream );
	
	Journal_Printf( (void*) stream, "\tconstructK: %u\n", self->constructK );
	Journal_Printf( (void*) stream, "\tconstructExtensionsK: %u\n", self->constructExtensionsK );
	Journal_Printf( (void*) stream, "\tbuildK: %u\n", self->buildK );
	Journal_Printf( (void*) stream, "\tinitialiseK: %u\n", self->initialiseK );
	Journal_Printf( (void*) stream, "\texecuteK: %u\n", self->executeK );
	Journal_Printf( (void*) stream, "\tdestroyK: %u\n", self->destroyK );
	Journal_Printf( (void*) stream, "\tdestroyExtensionsK: %u\n", self->destroyExtensionsK );
	
	Journal_Printf( (void*) stream, "\tdt: %u\n", self->dtK );
	Journal_Printf( (void*) stream, "\tstepK: %u\n", self->stepK );
	Journal_Printf( (void*) stream, "\tsolveK: %u\n", self->solveK );
	Journal_Printf( (void*) stream, "\tsyncK: %u\n", self->syncK );
	
	Print( self->extensionMgr_Register, stream );
	
	/* Print parent */
	_Stg_Class_Print( self, stream );
}


/* Construct EP override stuff ****************************************************************************************************/


Func_Ptr _AbstractContext_Construct_EP_GetRun( void* entryPoint ) {
	EntryPoint* self = (EntryPoint*)entryPoint;
	
	switch( self->castType ) {
		case EntryPoint_2VoidPtr_CastType:
			return (void*) _AbstractContext_Construct_EP_Run;
		
		default:
			return _EntryPoint_GetRun( self );
	}
	return 0;
}

void _AbstractContext_Construct_EP_Run( void* entryPoint, void* data0, void* data1 ) {
	/* This func should be _EntryPoint_Run_2VoidPtr, but with the synchronisation of context from ptrToContext after each
	 * hook. */
	EntryPoint*		self = (EntryPoint*)entryPoint;
	Hook_Index		hookIndex;
	AbstractContext* 	context = (AbstractContext*)data0;
	AbstractContext**	ptrToContext = (AbstractContext**)data1;
	
	
	#ifdef USE_PROFILE
		Stg_CallGraph_Push( stgCallGraph, _EntryPoint_Run_2VoidPtr, self->name );
	#endif
	
	for( hookIndex = 0; hookIndex < self->hooks->count; hookIndex++ ) {
		((EntryPoint_2VoidPtr_Cast*)((Hook*)self->hooks->data[hookIndex])->funcPtr)( context, ptrToContext );
		
		/* BIG difference to default run function... we need to "re-sync" the context with the potentially new one. Once
		    again this is only a HACK due to the implemented contexts NOT being extensions! What's also important, is
		    that the list of things added to this "the contruct" entry point may have changed, but changed on this "new"
		    context... need to "re-self" this EP. */
		context = *ptrToContext;
		self = KeyHandle( context, context->constructK );
//context->entryPoint_Register
	}
	
	#ifdef USE_PROFILE
		Stg_CallGraph_Pop( stgCallGraph );
	#endif
}


/* Component stuff ****************************************************************************************************************/


void _AbstractContext_Construct( void* context, Stg_ComponentFactory* cf, void* ptrToContext ) {
	AbstractContext* 	self = (AbstractContext*)context;
	AbstractContext**	ptrToSelf = (AbstractContext**)ptrToContext;
	Bool			isConstructed;
	
	Journal_Printf( self->debug, "In: %s\n", __func__ );
	Journal_Firewall( 
		self == *ptrToSelf, 
		Journal_Register( Error_Type, AbstractContext_Type ), 
		"ptrToContext is assumed to point to context\n" );

	#ifdef DEBUG
		Context_WarnIfNoHooks( self, self->buildK, __func__  );
	#endif
	
	/* Pre-mark the phase as complete as a default hook will attempt to build all live components (including this again) */
	isConstructed = self->isConstructed;
	self->isConstructed = True;

	/* Construct all the components. There should be quite few in there right now, however, the context will do a plugins
		load, which will add more and their dependencies. */
	KeyCall( self, self->constructK, EntryPoint_2VoidPtr_CallCast* )( KeyHandle(self,self->constructK), self, ptrToSelf );
	self = *ptrToSelf;

	/* Construct the list of plugins. This wont reconstruct the components already constructed in the last step. By seperating
		the plugins contstruction out of the component construction EP gives an opportunity to do a wrap up before
		plugins start */
	PluginsManager_ConstructPlugins( self->plugins, self->CF, ptrToSelf );

	/* Extensions are the last thing we want to do */
	KeyCall( self, self->constructExtensionsK, EntryPoint_VoidPtr_CallCast* )( KeyHandle(self,self->constructExtensionsK), self );
	
	if( self->rank == 0 ) 
		Context_PrintConcise( self, self->verbose );

	if ( True == Dictionary_GetBool_WithDefault( self->dictionary, "showJournalStatus", False ) ) {
		Journal_PrintConcise();	
	}	
	self->isConstructed = isConstructed;
}


void _AbstractContext_Build( void* context, void* data ) {
	AbstractContext* 	self = (AbstractContext*)context;
	Bool			isBuilt;
	
	Journal_Printf( self->debug, "In: %s\n", __func__ );

	#ifdef DEBUG
		Context_WarnIfNoHooks( self, self->buildK, __func__  );
	#endif
	
	/* Pre-mark the phase as complete as a default hook will attempt to build all live components (including this again) */
	isBuilt = self->isBuilt;
	self->isBuilt = True;
	KeyCall( self, self->buildK, EntryPoint_VoidPtr_CallCast* )( KeyHandle(self,self->buildK), self );
	self->isBuilt = isBuilt;
}


void _AbstractContext_Initialise( void* context, void* data ) {
	AbstractContext*	self = (AbstractContext*)context;
	Bool			isInitialised;
	
	Journal_Printf( self->debug, "In: %s\n", __func__ );

	#ifdef DEBUG
		AbstractContext_WarnIfNoHooks( self, self->initialiseK, __func__ );
	#endif
	
	/* Pre-mark the phase as complete as a default hook will attempt to initialise all live components (including this again) */
	isInitialised = self->isInitialised;
	self->isInitialised = True;
	KeyCall( self, self->initialiseK, EntryPoint_VoidPtr_CallCast* )( KeyHandle(self,self->initialiseK), self );
	self->isInitialised = isInitialised;
}


void _AbstractContext_Execute( void* context, void* data ) {
	AbstractContext* 	self = (AbstractContext*)context;
	Bool			hasExecuted;
	
	Journal_Printf( self->debug, "In: %s\n", __func__ );

	#if DEBUG
		AbstractContext_WarnIfNoHooks( self, self->executeK, __func__ );
	#endif
	
	/* Pre-mark the phase as complete as a default hook will attempt to initialise all live components (including this again) */
	hasExecuted = self->hasExecuted;
	self->hasExecuted = True;
	KeyCall( self, self->executeK, EntryPoint_VoidPtr_CallCast* )( KeyHandle(self,self->executeK), self );
	self->hasExecuted = hasExecuted;
}


void _AbstractContext_Destroy( void* context, void* data ) {
	AbstractContext*	self = (AbstractContext*)context;
	Bool			isDestroyed;
	
	Journal_Printf( self->debug, "In: %s\n", __func__ );

	/* Pre-mark the phase as complete as a default hook will attempt to initialise all live components (including this again) */
	isDestroyed = self->isDestroyed;
	self->isDestroyed = True;
	KeyCall( self, self->destroyExtensionsK, EntryPoint_VoidPtr_CallCast* )( KeyHandle(self,self->destroyExtensionsK), self );
	KeyCall( self, self->destroyK, EntryPoint_VoidPtr_CallCast* )( KeyHandle(self,self->destroyK), self );
	self->isDestroyed = isDestroyed;
}


/* Context public stuff ***********************************************************************************************************/


void AbstractContext_PrintConcise( void* abstractContext, Stream* stream ) {
	AbstractContext* self = (AbstractContext*)abstractContext;
	EntryPoint_Index entryPointIndex;

	Journal_Printf( stream, "Context: %s\n", self->type );
	for( entryPointIndex = 0; entryPointIndex < self->entryPoint_Register->count; entryPointIndex++ ) {
		EntryPoint_PrintConcise( EntryPoint_Register_At( self->entryPoint_Register, entryPointIndex ), stream );
	}
}


EntryPoint_Index AbstractContext_AddEntryPoint( 
		void*				abstractContext,
		void*				entryPoint )
{
	AbstractContext* self = (AbstractContext*)abstractContext;

	return EntryPoint_Register_Add( self->entryPoint_Register, entryPoint );
}


EntryPoint* AbstractContext_GetEntryPoint( 
		void*				abstractContext,
		const Name			entryPointName )
{
	AbstractContext*	self = (AbstractContext*)abstractContext;
	EntryPoint_Index	ep_I;

	/* Find the entry point */
	ep_I = EntryPoint_Register_GetHandle( self->entryPoint_Register, entryPointName );
	if( ep_I == (unsigned)-1 ) {
		return 0;
	}
	else {
		return EntryPoint_Register_At( self->entryPoint_Register, ep_I );
	}
}


Func_Ptr _AbstractContext_Call( void* abstractContext, Name entryPointName, void** epPtr ) {
	AbstractContext*	self = (AbstractContext*)abstractContext;
	EntryPoint_Index	ep_I;
	
	/* Find the entry point */
	ep_I = EntryPoint_Register_GetHandle( self->entryPoint_Register, entryPointName );
	if( ep_I == (unsigned)-1 ) {
		*epPtr = 0;
	}
	else {
		*epPtr = EntryPoint_Register_At( self->entryPoint_Register, ep_I );
	}
	
	/* ... and run it */
	if( *epPtr != 0 ) {
		return ((EntryPoint*) (*epPtr))->run;
	}
	return 0;
}


double AbstractContext_Dt( void* context ) {
	AbstractContext* self = (AbstractContext*)context;

	AbstractContext_ErrorIfNoHooks( self, self->dtK, __func__ );
	return KeyCall( self, self->dtK, ContextEntryPoint_Dt_CallCast* )( KeyHandle(self,self->dtK), self );
}

void AbstractContext_Step( void* context, double dt ) {
	AbstractContext* self = (AbstractContext*)context;
	#if DEBUG
		AbstractContext_WarnIfNoHooks( self, self->stepK, __func__ );
	#endif
	KeyCall( self, self->stepK, ContextEntryPoint_Step_CallCast* )( KeyHandle(self,self->stepK), self, dt );
}


void AbstractContext_WarnIfNoHooks( void* context, EntryPoint_Index epIndex, const char* caller ) {
	AbstractContext* self = (AbstractContext*)context;
	EntryPoint_WarnIfNoHooks( EntryPoint_Register_At( self->entryPoint_Register, epIndex ), caller );
}


void AbstractContext_ErrorIfNoHooks( void* context, EntryPoint_Index epIndex, const char* caller ) {
	AbstractContext* self = (AbstractContext*)context;
	EntryPoint_ErrorIfNoHooks( EntryPoint_Register_At( self->entryPoint_Register, epIndex ), caller );
}

void AbstractContext_FrequentOutput( void* context ) {
	AbstractContext* self = (AbstractContext*)context;

	KeyCall( self, self->frequentOutputK, EntryPoint_Class_VoidPtr_CallCast* )( 
			KeyHandle(self,self->frequentOutputK), self );
}

void AbstractContext_Dump( void* context ) {
	AbstractContext* self = (AbstractContext*)context;

	KeyCall( self, self->dumpK, EntryPoint_VoidPtr_CallCast* )(      
			KeyHandle(self,self->dumpK), self );
	KeyCall( self, self->dumpClassK, EntryPoint_Class_VoidPtr_CallCast* )(
			KeyHandle(self,self->dumpClassK), self );
}

void AbstractContext_Save( void* context ) {
	AbstractContext* self = (AbstractContext*)context;

	KeyCall( self, self->saveK, EntryPoint_VoidPtr_CallCast* )(     
			KeyHandle(self,self->saveK), self );
	KeyCall( self, self->saveClassK, EntryPoint_Class_VoidPtr_CallCast* )(      
			KeyHandle(self,self->saveClassK), self );
}


void AbstractContext_BuildAllLiveComponents( void* context ) {
	AbstractContext*       self = (AbstractContext*) context;	
	
	/* Build all on Live Stg_Component Register */
	if ( self->CF && self->CF->LCRegister )
		LiveComponentRegister_BuildAll( self->CF->LCRegister, self );
}


void AbstractContext_InitialiseAllLiveComponents( void* context ) {
	AbstractContext*       self = (AbstractContext*) context;	
	
	/* Build all on Live Stg_Component Register */
	if ( self->CF && self->CF->LCRegister )
		LiveComponentRegister_InitialiseAll( self->CF->LCRegister, self );
}


/* Context hooks ******************************************************************************************************************/


void _AbstractContext_Construct_Hook( void* context, void* ptrToContext ) {
	AbstractContext*		self = (AbstractContext*)context;
	AbstractContext**		ptrToSelf = (AbstractContext**)ptrToContext;
	Dictionary*			componentDict;
	Dictionary_Entry_Value*		apps;
	
	Journal_Printf( self->debug, "In: %s\n", __func__ );
	
	/* Obtain the components info from the global dictionary. Check that the dictionary has unique entry names. */
	if( (componentDict = Dictionary_GetDictionary( self->dictionary, AbstractContext_Dict_Components )) == NULL ) { 
		componentDict = Dictionary_New();
	}
	CheckDictionaryKeys( componentDict, "Component dictionary must have unique names\n" );
	
	/* The component factory should be specific to this context, so create it here unless is already exists (i.e. desired
	   behaviour has been purposely overridden). Add ourself to the global live register. */
	if( !self->CF ) {
		self->CF = Stg_ComponentFactory_New( self->dictionary, componentDict, self->register_Register );
	}
	LiveComponentRegister_Add( self->CF->LCRegister, (Stg_Component*)self );
	
	/* Obtain the applications  info from the global dictionary. Boot them up. */
	if( (apps = Dictionary_Get( self->dictionary, AbstractContext_Dict_Applications )) != NULL ) {
		int index; /* needs to be signed */
		
		/* Go through the list of entries, and attempt to load each plugin. NOTE: this will call the "Init" of the
		   application module. */
		for( index = 0; index < Dictionary_Entry_Value_GetCount( apps ); index++ ) {
			Dictionary_Entry_Value* application;
			
			application = Dictionary_Entry_Value_GetElement( apps, index );

			Journal_Firewall(
				PluginsManager_LoadPlugin( self->plugins, application->as.typeString, self ),
				Journal_Register( Error_Type, AbstractContext_Type ),
				"Error: Application Plugin %s not found. Ensure it is"
					" a valid plugin, and has been built.\n",
					application->as.typeString );
		}
		
		/* Now that the plugins are loaded (i.e. ALL application environments enabled), now RE-construct the context in the
		   reverse order. Ensuring that only the first (i.e. largest context) plugin builds a new context.
 		   NOTE: This mechanism completely sucks! The way Contexts are replaced is crap but works for now. It shows that
		   Context's should be implemented as extensions! */
		for( index = Dictionary_Entry_Value_GetCount( apps ) - 1; index >= 0; index-- ) {
			Dictionary_Entry_Value* application;
			
			application = Dictionary_Entry_Value_GetElement( apps, index );

			Journal_Firewall(
				PluginsManager_ConstructPlugin( self->plugins, application->as.typeString, self->CF, ptrToSelf ),
				Journal_Register( Error_Type, AbstractContext_Type ),
				"Error: Application Plugin %s could not be constructed."
					" Ensure it has a valid construction function.\n",
				      	application->as.typeString );
			
			if( self !=  (AbstractContext*)self->CF->LCRegister->componentList->data[0] ) {
				((AbstractContext*)self->CF->LCRegister->componentList->data[0])->plugins = self->plugins;
				self = (AbstractContext*)self->CF->LCRegister->componentList->data[0];
				Journal_Firewall( 
					(Bool)ptrToSelf, 
					Journal_Register( Error_Type, AbstractContext_Type ), 
					"Pointer to the context is needed, but set to NULL.\n" );
				*ptrToSelf = self;
			}
		}
	}
	
	/* Load the plugins ?? */
	PluginsManager_Load( self->plugins, context, self->dictionary );

	Stg_ComponentFactory_CreateComponents( self->CF );
	Stg_ComponentFactory_ConstructComponents( self->CF, ptrToSelf );
}


void _AbstractContext_Execute_Hook( Context* context ) {
	AbstractContext*   self = (AbstractContext*)context;
	double             dt = 0;
	double             dtLoadedFromFile = 0;
	
	if (self->maxTimeSteps) {
		Journal_Printf( self->info, "Run until %u timeSteps have been run\n", self->maxTimeSteps );
	}
	if (self->finalTimeStep ) {
		if (self->maxTimeSteps ) {
			Journal_Printf( self->info, "or " );
		}	
		else {
			Journal_Printf( self->info, "Run " );
		}
		Journal_Printf( self->info, "until absolute time step %u reached\n", self->stopTime );
	}
	
	if (self->stopTime) {
		if (self->maxTimeSteps || self->finalTimeStep ) {
			Journal_Printf( self->info, "or " );
		}	
		else {
			Journal_Printf( self->info, "Run " );
		}
		Journal_Printf( self->info, "until simulation time passes %g.\n", self->stopTime );
	}
	
	self->timeStepSinceJobRestart = 1;

	/* Set timeStep to 0 if not restarting, so that incrementing timestep below affects both
		regular and restart mode -- PatrickSunter - 18 June 2006 */
	if ( False == self->loadFromCheckPoint ) {
		self->timeStep = 0;
		self->currentTime = self->startTime;
	}
	
	self->timeStep++;

	while( !self->gracefulQuit ) {
		if ( ( True == self->loadFromCheckPoint ) &&
			( self->timeStep == self->restartTimestep + 1 ) )
		{ 
			/* Note: when checkpointing time info, we called AbstractContext_Dt( self )
			at the end of the step we were restarting from, which should be equivalent to the
			call here - and that calculation may be dependent on the solver info for that step,
			so we need to reload it here */
			_AbstractContext_LoadTimeInfoFromCheckPoint( self, self->restartTimestep, &dtLoadedFromFile );
			dt = dtLoadedFromFile;
		}	
		else {
			dt = self->dtFactor * AbstractContext_Dt( self );
		}

		AbstractContext_Step( self, dt );

		self->currentTime += dt;

		if ( self->frequentOutputEvery ) {
			if ( self->timeStep % self->frequentOutputEvery == 0 )
				AbstractContext_FrequentOutput( self );
		}	
		if ( self->dumpEvery ) {
			if ( self->timeStep % self->dumpEvery == 0 )
				AbstractContext_Dump( self );
		}	
		if ( self->checkpointEvery ) {
			if ( self->timeStep % self->checkpointEvery == 0 )
				AbstractContext_Save( self );
		}	

		if (self->maxTimeSteps && (self->timeStepSinceJobRestart >= self->maxTimeSteps)) break;
		if (self->finalTimeStep && (self->timeStep >= self->finalTimeStep)) break;
		if (self->stopTime && (self->currentTime >= self->stopTime)) break; 
		self->timeStep++;
		self->timeStepSinceJobRestart++;
	}
}


void _AbstractContext_Step( Context* context, double dt ) {
	AbstractContext* self = (AbstractContext*)context;
	
	/* This will make it clear where the timestep starts when several procs
	 * running. Figure this 1 synchronisation is ok since we are likely to
	 * have just synchronised while calculating timestep anyway. */
	MPI_Barrier( self->communicator );
	Journal_DPrintf( self->debug, "In: %s\n", __func__ );
	Journal_Printf( self->info, "TimeStep = %d, Start time = %.6g + %.6g prev timeStep dt\n",
		self->timeStep, self->currentTime, dt );

	if (self->loadFromCheckPoint) {
		Journal_Printf( self->info, "TimeStep since job restart = %d\n", self->timeStepSinceJobRestart );
	}

	#ifdef DEBUG
		Context_WarnIfNoHooks( self, self->solveK, __func__ );	
	#endif

	self->_setDt( self, dt );
	KeyCall( self, self->solveK, EntryPoint_VoidPtr_CallCast* )( KeyHandle(self,self->solveK), self );
	KeyCall( self, self->updateClassK, EntryPoint_Class_VoidPtr_CallCast* )( KeyHandle(self,self->updateClassK), self );
	KeyCall( self, self->syncK, EntryPoint_VoidPtr_CallCast* )( KeyHandle(self,self->syncK), self );
}


void _AbstractContext_LoadTimeInfoFromCheckPoint( Context* self, Index timeStep, double* dtLoadedFromFile ) {
	char*                  timeInfoFileName = NULL;
	FILE*                  timeInfoFile;		
	Stream*                errorStr = Journal_Register( Error_Type, self->type );

	timeInfoFileName = AbstractContext_GetTimeInfoFileNameForGivenTimeStep( self, timeStep ); 
	timeInfoFile = fopen( timeInfoFileName, "r" );
	Journal_Firewall( NULL != timeInfoFile, errorStr, "Error- in %s(), Couldn't find checkpoint time info file with "
		"filename \"%s\" - aborting.\n", __func__, timeInfoFileName );

	/* set currentTime and Dt loaded from file */
	fscanf( timeInfoFile, "%lg", &self->currentTime );
	fscanf( timeInfoFile, "%lg", dtLoadedFromFile );
	fclose( timeInfoFile );
	Memory_Free( timeInfoFileName );
}
		

void _AbstractContext_SaveTimeInfo( Context* context ) {
	AbstractContext*       self = context;	
	char*                  timeInfoFileName = NULL;
	FILE*                  timeInfoFile;	

	/* Only the master process needs to write this file */
	if ( 0 != self->rank ) return;

	timeInfoFileName = AbstractContext_GetTimeInfoFileNameForGivenTimeStep( self, self->timeStep ); 
	
	timeInfoFile = fopen( timeInfoFileName, "w" );

	if ( False == timeInfoFile ) {
		Stream*    errorStr = Journal_Register( Error_Type, self->type );
		Journal_Printf( errorStr, "Error- in %s(), Couldn't create checkpoint time info file with "
		"filename \"%s\" - aborting.\n", __func__, timeInfoFileName );
		exit(EXIT_FAILURE);
	}

	/* set currentTime and Dt loaded from file */
	fprintf( timeInfoFile, "%lg ", context->currentTime );
	fprintf( timeInfoFile, "%lg\n", AbstractContext_Dt( context ) );
	fclose( timeInfoFile );
	Memory_Free( timeInfoFileName );
}


Bool AbstractContext_CheckPointExists( void* context, Index timeStep ) {
	AbstractContext*       self = context;	
	char*                  timeInfoFileName = NULL;
	struct stat            statInfo;
	int                    statResult;

	timeInfoFileName = AbstractContext_GetTimeInfoFileNameForGivenTimeStep( self, self->timeStep ); 
	statResult = stat( timeInfoFileName, &statInfo );

	if ( 0 == statResult ) {
		return True;
	}
	else {
		return False;
	}
}


char* AbstractContext_GetTimeInfoFileNameForGivenTimeStep( void* context, Index timeStep ) {
	AbstractContext*       self = context;	
	char*                  timeInfoFileName = NULL;
	Index                  timeInfoStrLen = 0;

	timeInfoStrLen = strlen(self->outputPath) + 1 + 8 + 1 + 5 + 1 + 3 + 1;
	if ( strlen(self->checkPointPrefixString) > 0 ) {
		timeInfoStrLen += strlen(self->checkPointPrefixString) + 1;
	}
	timeInfoFileName = Memory_Alloc_Array( char, timeInfoStrLen, "timeInfoFileName" );

	if ( strlen(self->checkPointPrefixString) > 0 ) {
		sprintf( timeInfoFileName, "%s/%s.", self->outputPath, self->checkPointPrefixString );
	}
	else {
		sprintf( timeInfoFileName, "%s/", self->outputPath );
	}
	sprintf( timeInfoFileName, "%stimeInfo.%.5u.dat", timeInfoFileName, timeStep );

	return timeInfoFileName;
}

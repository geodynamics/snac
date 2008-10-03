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
**	Abstract class faciliting how "modellers"/"solvers" are laid out and execute.
**
** Assumptions:
**
** Comments:
**	This is a class, of which subclasses can be defined, which can override default methods.
**	Currently built with only MeshPIC_Context in mind (for Snac).
**
** $Id: AbstractContext.h 3857 2006-10-14 18:06:05Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Base_Context_AbstractContext_h__
#define __Base_Context_AbstractContext_h__
	

	/* Templates of virtual functions */
	typedef void		(AbstractContext_SetDt)				( void* self, double dt );
	
	
	/* Context_CallInfo info */
	#define __Base_Context_CallInfo \
		void*					functionPointer; \
		void*					entryPoint;
	struct Context_CallInfo { __Base_Context_CallInfo };
	
	
	/* AbstractContext entry point names */
	extern Type AbstractContext_EP_Construct;
	extern Type AbstractContext_EP_ConstructExtensions;
	extern Type AbstractContext_EP_Build;
	extern Type AbstractContext_EP_Initialise;
	extern Type AbstractContext_EP_Execute;
	extern Type AbstractContext_EP_Destroy;
	extern Type AbstractContext_EP_DestroyExtensions;
	extern Type AbstractContext_EP_Dt;
	extern Type AbstractContext_EP_Step;
	extern Type AbstractContext_EP_UpdateClass;
	extern Type AbstractContext_EP_Solve;
	extern Type AbstractContext_EP_Sync;
	extern Type AbstractContext_EP_FrequentOutput;
	extern Type AbstractContext_EP_Dump;
	extern Type AbstractContext_EP_DumpClass;
	extern Type AbstractContext_EP_Save;
	extern Type AbstractContext_EP_SaveClass;
	
	/* Textual name of this class */
	extern const Type AbstractContext_Type;
	extern const Type AbstractContext_Type_Verbose; /* Use for a particular info stream */
	
	/* AbstractContext info */
	#define __AbstractContext \
		/* General info */ \
		__Stg_Component \
		Dictionary*				dictionary; \
		\
		/* Virtual info */ \
		AbstractContext_SetDt*			_setDt; \
		\
		/* AbstractContext info */ \
		MPI_Comm				communicator; \
		int					rank; \
		int					nproc; \
		/** Start time for the simulation. */ \
		double					startTime; \
		/** Stop time for the simulation. Note that if this is 0, the sim will keep running unless a 
		max loops criterion is met. */ \
		double					stopTime; \
		double					currentTime; \
		unsigned int				timeStep; \
		double					dtFactor; \
		/** This additional timestep is necessary for checkpoint restart runs, so it can be compared against
			maxTimeSteps (which is now relative to job restart).*/ \
		unsigned int				timeStepSinceJobRestart; \
		/** Maximum number of time steps to run for. If set to 0, then this will  be ignored. */ \
		unsigned int				maxTimeSteps; \
		/** Final Time Step: last time step to run till, no matter if maxTimeSteps still has some
		     left in a checkpoint restart run. If 0 (the default), not active. */ \
		unsigned int				finalTimeStep; \
		Bool					gracefulQuit; \
		unsigned int				frequentOutputEvery; \
		unsigned int				dumpEvery; \
		unsigned int				checkpointEvery; \
		Name                                    experimentName; \
		char*					outputPath; \
		Bool                                    loadFromCheckPoint;      \
		unsigned int                            restartTimestep;         \
		char*                                   checkPointPrefixString;  \
		Stream*					info; \
		Stream*					verbose; \
		Stream*					debug; \
		\
		/* These are stored keys to entrypoints in the table, used for O(1) lookup (i.e. speed) */ \
		/* Contexts "are" Components implemented by entrypoints... there's an entry point per component phase */ \
		EntryPoint_Index			constructK; \
		EntryPoint_Index			constructExtensionsK; \
		EntryPoint_Index			buildK; \
		EntryPoint_Index			initialiseK; \
		EntryPoint_Index			executeK; \
		EntryPoint_Index			destroyK; \
		EntryPoint_Index			destroyExtensionsK; \
		\
		/* The following are not really part of an abstract context, and will one day be refactored somewhere else */ \
		EntryPoint_Index			dtK; \
		EntryPoint_Index			stepK; \
		EntryPoint_Index			updateClassK; \
		EntryPoint_Index			solveK; \
		EntryPoint_Index			syncK; \
		EntryPoint_Index			frequentOutputK; \
		EntryPoint_Index			dumpK; \
		EntryPoint_Index			dumpClassK; \
		EntryPoint_Index			saveK; \
		EntryPoint_Index			saveClassK; \
		\
		Stg_ObjectList*				objectList; \
		ConditionFunction_Register*		condFunc_Register; \
		Variable_Register*			variable_Register; \
		Pointer_Register*			pointer_Register; \
		EntryPoint_Register*			entryPoint_Register; \
		ExtensionManager_Register*		extensionMgr_Register; \
		ExtensionManager*			extensionMgr; \
		Register_Register*			register_Register; \
		Stg_ComponentFactory*			CF; \
		PluginsManager*				plugins;
		
	struct AbstractContext { __AbstractContext };
	
	/* Class stuff ************************************************************************************************************/
	
	/* No "AbstractContext_New" and "AbstractContext_Init" as this is an abstract class */
	
	/* Creation implementation / Virtual constructor */
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
		Dictionary*					dictionary );
	
	/* Initialisation implementation */
	void _AbstractContext_Init( 
		AbstractContext* 		self,
		double				startTime,
		double				stopTime,
		MPI_Comm			communicator );
	
	
	/* Stg_Class_Delete implementation */
	void _AbstractContext_Delete( void* abstractContext );
	
	/* Print implementation */
	void _AbstractContext_Print( void* abstractContext, Stream* stream );
	
	
	/* Component stuff ********************************************************************************************************/
	
	
	/* Construct the context ... connect and validate component connections, and initialise non-bulk internal values */
	void _AbstractContext_Construct( void* context, Stg_ComponentFactory* cf, void* data );
	
	/* Build the context ... allocates memory (builds arrays) */
	void _AbstractContext_Build( void* context, void* data );
	
	/* Initialise the context ... initialises memory (fills arrays with initial values) */
	void _AbstractContext_Initialise( void* context, void* data );
	
	/* Run the context ... perform the task this component is supposed to perform */
	void _AbstractContext_Execute( void* context, void* data );
	
	/* Destroy the context ... clean up / un build */
	void _AbstractContext_Destroy( void* context, void* data );
	
	
	/* Context public methods *************************************************************************************************/
	
	
	/* Print entry points utility */
	void AbstractContext_PrintConcise( void* abstractContext, Stream* stream );
	
	EntryPoint_Index AbstractContext_AddEntryPoint( void* abstractContext, void* entryPoint );
	
	/* Add an entry point. "castType" is ignored unless this is a new entry point. Returns a key to the entry. */
	EntryPoint* AbstractContext_GetEntryPoint( void* abstractContext, const Name entryPointName ); 
	
	/* Runs the AbstractContext_EP_FrequentOutput Entry Point */
	void AbstractContext_FrequentOutput( void* context ) ;
	
	/* Runs the AbstractContext_EP_Dump and AbstractContext_EP_DumpClass Entry Points */
	void AbstractContext_Dump( void* context ) ;
	
	/* Runs the AbstractContext_EP_Save Entry Point */
	void AbstractContext_Save( void* context ) ;

	/* Run an entry point... resolving from name (slower) */
	#define AbstractContext_Call( self, name, cast, handle )	((cast)_AbstractContext_Call( self, name, &handle ))
	Func_Ptr _AbstractContext_Call( void* abstractContext, Name name, void** handle );
	
	/* Run an entry point... resolving using key (slower) */
	#define AbstractContext_KeyHandle( self, key ) \
		EntryPoint_Register_At( (self)->entryPoint_Register, key )
	#define AbstractContext_KeyCall( self, key, cast ) \
		((cast)(EntryPoint_Register_At( (self)->entryPoint_Register, key )->run))
	
	/* Obtain the Dt */
	double AbstractContext_Dt( void* context );
	
	/* Step the solver */
	void AbstractContext_Step( void* context, double dt );
	
	/* function to warn if no hooks to an entrypoint defined */
	void AbstractContext_WarnIfNoHooks( void* context, EntryPoint_Index epIndex, const char* caller );
	
	/* function to error if no hooks to an entrypoint defined */
	void AbstractContext_ErrorIfNoHooks( void* context, EntryPoint_Index epIndex, const char* caller );
	
	void AbstractContext_BuildAllLiveComponents( void* context ) ;
	void AbstractContext_InitialiseAllLiveComponents( void* context ) ;

	Bool AbstractContext_CheckPointExists( void* context, Index timeStep );
	char* AbstractContext_GetTimeInfoFileNameForGivenTimeStep( void* context, Index timeStep );
	
	/* Context private methods ************************************************************************************************/
	
	
	/* Default construction hook, and overrides for the EP to handle the context/ptrToContext synchronisation */
	void _AbstractContext_Construct_Hook( void* context, void* ptrToContext );
	Func_Ptr _AbstractContext_Construct_EP_GetRun( void* entryPoint );
	void _AbstractContext_Construct_EP_Run( void* entryPoint, void* data0, void* data1 );
	
	/* Default construction hook */
	void _AbstractContext_Execute_Hook( Context* context );
	
	/* Step the solver implementation */
	void _AbstractContext_Step( Context* context, double dt );
	
	void _AbstractContext_LoadTimeInfoFromCheckPoint( Context* self, Index timeStep, double* dtLoadedFromFile );
	void _AbstractContext_SaveTimeInfo( Context* context );

#endif /* __Base_Context_AbstractContext_h__ */

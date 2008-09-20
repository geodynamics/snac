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
** This file may be distributed under the terms of the VPAC Public License
** as defined by VPAC of Australia and appearing in the file
** LICENSE.VPL included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
*/
/** \file
**  Role:
**
** Assumptions:
**
** Comments:
**
** $Id: SnacSync.h 2453 2004-12-21 04:49:34Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisaton_Utils_SnacSync_h__
#define __Discretisaton_Utils_SnacSync_h__

	extern const Type SnacSync_Type;

	/* Virtual function types */
	typedef void		(SnacSync_Select)( void* sync, unsigned nGlobals, unsigned* nFound, unsigned** found );
	
	/* Textual name of this class */

	/* SnacSync information */
	#define __SnacSync \
		/* General info */ \
		__Stg_Component \
		Dictionary*				dictionary; \
		\
		/* Virtual info */ \
		\
		/* SnacSync info ... */ \
		MPI_Comm					comm; \
		int						nProcs; \
		int						rank; \
		\
		SnacSync_Select*				select; \
		\
		unsigned					netSource; \
		unsigned*					nSource; \
		unsigned**				source; \
		\
		unsigned					netSink; \
		unsigned*					nSink; \
		unsigned**				sink; \
		\
		unsigned*					sourceDisps; \
		unsigned*					sourceSizes; \
		unsigned*					sinkDisps; \
		unsigned*					sinkSizes; \
		\
		unsigned*					sourceOffs; \
		unsigned*					sinkOffs; \
		size_t					itemSize;
	struct _SnacSync { __SnacSync };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	
	/* Create a SnacSync */
	
	void* SnacSync_DefaultNew( Name name );
	
	#define SnacSync_New( name ) \
		SnacSync_New_Param( NULL, name )
	
	/* Create a SnacSync with all additional parameters */
	SnacSync* SnacSync_New_Param(
		Dictionary*	dictionary,
		Name			name );
	
	/* Initialise a SnacSync */
	void SnacSync_Init(
		SnacSync*		self,
		Dictionary*	dictionary,
		Name			name );
	
	/* Creation implementation */
	SnacSync* _SnacSync_New(
		SizeT							_sizeOfSelf, 
		Type								type,
		Stg_Class_DeleteFunction*			_delete,
		Stg_Class_PrintFunction*				_print,
		Stg_Class_CopyFunction*				_copy, 
		Stg_Component_DefaultConstructorFunction*	_defaultConstructor,
		Stg_Component_ConstructFunction*			_construct,
		Stg_Component_BuildFunction*				_build,
		Stg_Component_InitialiseFunction*			_initialise,
		Stg_Component_ExecuteFunction*			_execute,
		Stg_Component_DestroyFunction*			_destroy,
		Name								name,
		Bool								initFlag,
		Dictionary*						dictionary );
	
	/* Initialisation implementation functions */
	void _SnacSync_Init( SnacSync* self );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/* Stg_Class_Delete sync implementation */
	void _SnacSync_Delete( void* sync );
	
	/* Print sync implementation */
	void _SnacSync_Print( void* sync, Stream* stream );
	
	void _SnacSync_Construct( void* sync, Stg_ComponentFactory *cf, void* data );
	
	void _SnacSync_Build( void* sync, void *data );
	
	void _SnacSync_Initialise( void* sync, void *data );
	
	void _SnacSync_Execute( void* sync, void *data );

	void _SnacSync_Destroy( void* sync, void *data );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public member functions
	*/
	
	void SnacSync_Negotiate( void*		sync, 
					 unsigned		nGlobals, 
					 unsigned		nLocals, 
					 unsigned*	locals, 
					 unsigned		nRequired, 
					 unsigned*	required, 
					 MPI_Comm		comm );
	
	void SnacSync_SendRecvInitialise( void*	sync, 
							size_t	itemSize, 
							unsigned*	sinkOffsets, 
							unsigned	sinkStride, 
							unsigned*	sourceOffsets, 
							unsigned	sourceStride );
	
	void SnacSync_SendRecv( void*	sync, 
					void*	srcArray, 
					void*	dstArray );

	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/
	
	void _SnacSync_FreePrivateArrays( void* sync );
	
	void _SnacSync_Select_MinimizeComms( void* sync, unsigned nGlobals, unsigned* nFound, unsigned** found );
	
	
#endif /* __Discretisaton_Utils_SnacSync_h__ */


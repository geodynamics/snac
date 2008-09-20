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
**
** Assumptions:
**
** Comments:
**
** $Id: Sync.h 2453 2004-12-21 04:49:34Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisaton_Utils_Sync_h__
#define __Discretisaton_Utils_Sync_h__


	/* Useful typedefs. */
	typedef unsigned char	byte_t;
	typedef enum {
		Sync_ArrayType_Domain, 
		Sync_ArrayType_Split, 
		Sync_ArrayType_None
	} Sync_ArrayType;

	extern const Type Sync_Type;

	/* Virtual function types */
	typedef void (Sync_Select)( void* sync, 
				    unsigned* nFound, unsigned** found );
	
	/* Textual name of this class */

	/* Sync information */
	#define __Sync \
		/* General info */ \
		__Stg_Component \
		Dictionary*				dictionary; \
		\
		/* Virtual info */ \
		\
		/* Sync info ... */ \
		MPI_Comm				comm; \
		int					nProcs; \
		int					rank; \
		\
		Sync_Select*				select; \
		\
		unsigned				nGlobal; \
		unsigned				nLocal; \
		unsigned*				local; \
		unsigned				nShared; \
		unsigned*				shared; \
		unsigned				nShadow; \
		unsigned*				shadow; \
		unsigned				nDomain; \
		unsigned*				domain; \
		\
		unsigned*		    		globalMap; \
		\
		unsigned				netSource; \
		unsigned*				nSource; \
		unsigned**				source; \
		unsigned				netSink; \
		unsigned*				nSink; \
		unsigned**				sink; \
		\
		unsigned*				sourceDisps; \
		unsigned*				sourceSizes; \
		unsigned*				sourceOffs; \
		void*					sourceArray; \
		unsigned*				sinkDisps; \
		unsigned*				sinkSizes; \
		unsigned*				sinkOffs; \
		void*					sinkArray; \
		\
		size_t					itemSize; \
		Sync_ArrayType				arrayType; \
		union { \
		      void*				array; \
		      struct { \
			    void*			srcArray; \
			    void*			snkArray; \
		      } splitArray; \
		} arrayInfo; 

	struct Sync { __Sync };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	
	/* Create a Sync */
	
	void* Sync_DefaultNew( Name name );
	
	#define Sync_New( name ) \
		Sync_New_Param( NULL, name )
	
	/* Create a Sync with all additional parameters */
	Sync* Sync_New_Param(
		Dictionary*	dictionary,
		Name		name );
	
	/* Initialise a Sync */
	void Sync_Init(	Sync*		self, 
			Dictionary*	dictionary, 
			Name		name );
	
	/* Creation implementation */
	Sync* _Sync_New( SizeT						_sizeOfSelf, 
			 Type						type,
			 Stg_Class_DeleteFunction*			_delete,
			 Stg_Class_PrintFunction*			_print,
			 Stg_Class_CopyFunction*			_copy, 
			 Stg_Component_DefaultConstructorFunction*	_defaultConstructor,
			 Stg_Component_ConstructFunction*		_construct,
			 Stg_Component_BuildFunction*			_build,
			 Stg_Component_InitialiseFunction*		_initialise,
			 Stg_Component_ExecuteFunction*			_execute,
			 Stg_Component_DestroyFunction*			_destroy,
			 Name						name,
			 Bool						initFlag,
			 Dictionary*					dictionary );
	
	/* Initialisation implementation functions */
	void _Sync_Init( Sync* self );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/* Stg_Class_Delete sync implementation */
	void _Sync_Delete( void* sync );
	
	/* Print sync implementation */
	void _Sync_Print( void* sync, Stream* stream );
	
	void _Sync_Construct( void* sync, Stg_ComponentFactory *cf, void* data );
	
	void _Sync_Build( void* sync, void *data );
	
	void _Sync_Initialise( void* sync, void *data );
	
	void _Sync_Execute( void* sync, void *data );

	void _Sync_Destroy( void* sync, void *data );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public member functions
	*/
	
	void Sync_Negotiate( void* sync, 
			     unsigned nGlobal, 
			     unsigned* local, unsigned nLocal, 
			     unsigned* shared, unsigned nShared, 
			     unsigned* shadow, unsigned nShadow, 
			     MPI_Comm comm );

	void Sync_SetDomainArray( void* sync, 
				  size_t itemSize, size_t stride, void* array );

	void Sync_SetSplitArrays( void* sync, 
				  size_t itemSize, 
				  size_t srcStride, void* srcArray, 
				  size_t snkStride, void* snkArray );

	unsigned Sync_GetNGlobal( void* sync );

	unsigned Sync_GetNLocal( void* sync );

	unsigned Sync_GetNShadow( void* sync );

	unsigned Sync_GetNDomain( void* sync );

	unsigned Sync_MapLocal( void* sync, unsigned lInd );

	unsigned Sync_MapRemote( void* sync, unsigned rInd );

	unsigned Sync_MapDomain( void* sync, unsigned dInd );

	unsigned Sync_MapGlobal( void* sync, unsigned gInd );

	Bool Sync_IsLocal( void* sync, unsigned gInd );

	Bool Sync_IsRemote( void* sync, unsigned gInd );

	void Sync_SendRecv( void* sync );

	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/
	
	void _Sync_Free( void* sync );
	
	void _Sync_Select_MinimizeComms( void* sync, unsigned* nFound, unsigned** found );

	void _Sync_MapAll( void* sync );

	void _Sync_BuildGlobalMap( void* sync );


	/*--------------------------------------------------------------------------------------------------------------------------
	** MPI Helper Functions.
	*/

	#define CHK_MPI( mpiFunc ) \
		if( mpiFunc != MPI_SUCCESS ) \
			fprintf( stderr, "Warning: MPI function returned failure...\n" )

	void MPI_ArrayGather1D( unsigned size, size_t itemSize, void* array, 
				unsigned** dstSizes, void** dstArray, 
				unsigned root, MPI_Comm comm );

	void MPI_ArrayAllgather( unsigned size, size_t itemSize, void* array, 
				 unsigned** dstSizes, void*** dstArray, 
				 MPI_Comm comm );

	void MPI_Array2DAlltoall( unsigned* sizes, size_t itemSize, void** array, 
				  unsigned** dstSizes, void*** dstArray, 
				  MPI_Comm comm );
	
#endif /* __Discretisaton_Utils_Sync_h__ */


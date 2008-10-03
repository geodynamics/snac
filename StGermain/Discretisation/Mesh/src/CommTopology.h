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
** Invariants:
**
** Comments:
**
** $Id: CommTopology.h 3584 2006-05-16 11:11:07Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisaton_Mesh_CommTopology_h__
#define __Discretisaton_Mesh_CommTopology_h__

	/** Textual name of this class */
	extern const Type CommTopology_Type;

	/** Virtual function types */

	/** Class contents */
	#define __CommTopology				\
		/* General info */			\
		__Stg_Component				\
							\
		/* Virtual info */			\
							\
		/* CommTopology info */			\
		MPI_Comm		comm;		\
		unsigned		nProcs;		\
		unsigned		rank; 		\
							\
		unsigned		nInc;		\
		unsigned*		inc;

	struct CommTopology { __CommTopology };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/

	#define COMMTOPOLOGY_DEFARGS	\
		STG_COMPONENT_DEFARGS

	#define COMMTOPOLOGY_PASSARGS	\
		STG_COMPONENT_PASSARGS

	CommTopology* CommTopology_New( Name name );
	CommTopology* _CommTopology_New( COMMTOPOLOGY_DEFARGS );
	void _CommTopology_Init( CommTopology* self );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _CommTopology_Delete( void* generator );
	void _CommTopology_Print( void* generator, Stream* stream );

	#define CommTopology_Copy( self )				\
		(Mesh*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define CommTopology_DeepCopy( self )				\
		(Mesh*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _CommTopology_Copy( void* generator, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );

	void _CommTopology_Construct( void* generator, Stg_ComponentFactory* cf, void* data );
	void _CommTopology_Build( void* generator, void* data );
	void _CommTopology_Initialise( void* generator, void* data );
	void _CommTopology_Execute( void* generator, void* data );
	void _CommTopology_Destroy( void* generator, void* data );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	void CommTopology_SetComm( void* commTopology, MPI_Comm comm );
	void CommTopology_SetIncidence( void* commTopology, unsigned nInc, unsigned* inc );

	void CommTopology_GetIncidence( void* commTopology, unsigned proc, 
					unsigned* nInc, unsigned** inc );
	void CommTopology_ReturnIncidence( void* commTopology, unsigned proc, 
					   unsigned* nInc, unsigned** inc );
	void CommTopology_Allgather( void* commTopology, 
				     unsigned srcSize, void* srcArray, 
				     unsigned** dstSizes, void*** dstArrays, 
				     unsigned itemSize );
	void CommTopology_Alltoall( void* commTopology, 
				    unsigned* srcSizes, void** srcArrays, 
				    unsigned** dstSizes, void*** dstArrays, 
				    unsigned itemSize );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

	int CommTopology_CmpRanks( const void* rank0, const void* rank1 );
	void CommTopology_Destruct( CommTopology* self );

#endif /* __Discretisaton_Mesh_CommTopology_h__ */

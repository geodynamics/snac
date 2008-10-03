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
** $Id: testVariable.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"
#include "Base/Automation/Automation.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


int main( int argc, char *argv[] ) {
	int		rank;
	int		procCount;
	int		procToWatch;
	Stream*		stream;
	
	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_size( MPI_COMM_WORLD, &procCount );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	
	BaseFoundation_Init( &argc, &argv );
	BaseIO_Init( &argc, &argv );
	BaseContainer_Init( &argc, &argv );
	BaseAutomation_Init( &argc, &argv );

	stream = Journal_Register( Info_Type, "testVariable" );
	
	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}
	if( rank == procToWatch ) {
		typedef double VectorD[3];
		typedef double VectorF[3];
		typedef struct {
			int		mass;
			VectorF		force;
			short		num;
			char*		info;
		} Particle;
		
		Variable_Register*	vr;
		Variable_Index		var_I;
		
		double*			temperature;
		VectorD*		velocity;
		Particle*		particle;
		Index			aSize[3] = { 16, 16, 16 };
		int			i;
		
		Particle		tmpParticle;
		Name			pNames[] = {
						"mass",
						"force",
						"info" };
		SizeT			pOffsets[] = { 0, 0, 0 };	/* Init later... */
		Variable_DataType	pDataTypes[] = {
						Variable_DataType_Int,
						Variable_DataType_Float,
						Variable_DataType_Pointer, };
		Index			pDtCounts[] = { 1, 3, 1 };
		static SizeT		pSize = sizeof(Particle);
		
		pOffsets[0] = (ArithPointer)&tmpParticle.mass - (ArithPointer)&tmpParticle;
		pOffsets[1] = (ArithPointer)&tmpParticle.force - (ArithPointer)&tmpParticle;
		pOffsets[2] = (ArithPointer)&tmpParticle.info - (ArithPointer)&tmpParticle;

		/* Construction phase --------------------------------------------------------------------------------------------*/
		vr = Variable_Register_New();
		Variable_NewScalar( "temperature", Variable_DataType_Double, &aSize[0], (void**)&temperature, vr );
		Variable_NewVector( "velocity", Variable_DataType_Double, 3, &aSize[1], (void**)&velocity, vr, "vx", "vy", "vz" );
		Variable_New( "particle", 3, pOffsets, pDataTypes, pDtCounts, pNames, &pSize, &aSize[2], (void**)&particle, vr );
		
		Journal_Printf( stream, "Variable construction is working fine.\n" );

		/* Build phase ---------------------------------------------------------------------------------------------------*/
		temperature = Memory_Alloc_Array( double, aSize[0], "temperature" );
		velocity = Memory_Alloc_Array( VectorD, aSize[1], "velocity" );
		particle = Memory_Alloc_Array( Particle, aSize[2], "array" );
		
		Variable_Register_BuildAll( vr );
		/*for( var_I = 0; var_I < vr->count; var_I++ ) {
			Print( vr->_variable[var_I], stream );
		}*/
		
		Journal_Printf( stream, "Variable building is working fine.\n" );
		
		/* Initialise phase ----------------------------------------------------------------------------------------------*/
		
		/* Execution phase -----------------------------------------------------------------------------------------------*/
		
		/* Test the Get and Set of a scalar double....................................................................... */
		/* Fill the temperature array with a known pattern of kinda random (bit filling) numbers. */
		for( i = 0; i < aSize[0]; i++ ) {
			temperature[i] = 1.0f / (aSize[0]+2) * (i+1); 
		}
		
		/* Check that Variable_GetValueDouble on the temperature Variable returns the right numbers */
		for( i = 0; i < aSize[0]; i++ ) {
			Variable*		var = Variable_Register_GetByName( vr, "temperature" );
			const double		tmp = 1.0f / (aSize[0]+2) * (i+1);
			
			Journal_Firewall( 
				Variable_GetValueDouble( var, i ) == tmp,
				Journal_Register( Error_Type, "testVariable" ),
				"Variable_GetValueDouble FAILED to get the correct value!\n" );
		}
		
		/* Fill the temperature Variable with another known pattern of kinda random (bit filling) numbers */
		for( i = 0; i < aSize[0]; i++ ) {
			Variable*		var = Variable_Register_GetByName( vr, "temperature" );
			
			Variable_SetValueDouble( var, i, 1.0f - ( 1.0f / (aSize[0]+2) * (i+1) ) );
		}
		
		/* Check that Variable_SetValueDouble on the temperature Variable set the right numbers */
		for( i = 0; i < aSize[0]; i++ ) {
			const double		tmp = 1.0f - 1.0f / (aSize[0]+2) * (i+1);
			
			Journal_Firewall( 
				temperature[i] == tmp,
				Journal_Register( Error_Type, "testVariable" ),
				"Variable_SetValueDouble FAILED to set the correct value!\n" );
		}
		
		Journal_Printf( stream, "Scalar-double Variable accessing is working fine.\n" );
		
		
		/* Test the Get and Set of a vector double....................................................................... */
		/* Fill the velocity array with a known pattern of kinda random (bit filling) numbers. */
		for( i = 0; i < aSize[1]; i++ ) {
			int			d;
			
			for( d = 0; d < 3; d++ ) {
				velocity[i][d] = 1.0f / ((aSize[1]*3)+2) * (i*3+d+1); 
			}
		}
		
		/* Check that Variable_GetPtrDouble on the velocity Variable returns the right numbers */
		for( i = 0; i < aSize[1]; i++ ) {
			Variable*		var = Variable_Register_GetByName( vr, "velocity" );
			int			d;
			
			for( d = 0; d < 3; d++ ) {
				const double 		tmp = 1.0f / ((aSize[1]*3)+2) * (i*3+d+1);
				
				Journal_Firewall( 
					Variable_GetValueAtDouble( var, i, d ) == tmp,
					Journal_Register( Error_Type, "testVariable" ),
					"Variable_GetValueDouble FAILED to get the correct value!\n" );
			}
		}
		
		/* Fill the variable Variable with another known pattern of kinda random (bit filling) numbers */
		for( i = 0; i < aSize[1]; i++ ) {
			Variable*		var = Variable_Register_GetByName( vr, "velocity" );
			int			d;
			
			for( d = 0; d < 3; d++ ) {
				Variable_SetValueAtDouble( var, i, d, 1.0f - ( 1.0f / ((aSize[1]*3)+2) * (i*3+d+1) ) );
			}
		}
		
		/* Check that Variable_SetValueDouble on the temperature Variable set the right numbers */
		for( i = 0; i < aSize[1]; i++ ) {
			int			d;
			
			for( d = 0; d < 3; d++ ) {
				const double		tmp = 1.0f - ( 1.0f / ((aSize[1]*3)+2) * (i*3+d+1) );
				
				Journal_Firewall( 
					velocity[i][d] == tmp,
					Journal_Register( Error_Type, "testVariable" ),
					"Variable_SetValueDouble FAILED to set the correct value!\n" );
			}
		}
		
		Journal_Printf( stream, "Vector-double Variable accessing is working fine.\n" );
		
/* TODO: try out vx, vy, vz, complex tests */
		
		/* Destruction phase ---------------------------------------------------------------------------------------------*/
		
		/* HACK: manually delete all the created Variables */
		for( var_I = 0; var_I < vr->count; var_I++ ) {
			Stg_Class_Delete( vr->_variable[var_I] );
		}
		
		Memory_Free( particle );
		Memory_Free( velocity );
		Memory_Free( temperature );
	}
	
	BaseAutomation_Finalise();
	BaseContainer_Finalise();
	BaseIO_Finalise();
	BaseFoundation_Finalise();

	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0; /* success */
}

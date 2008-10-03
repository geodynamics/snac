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
** Role:
**	Tests that the FeVariable class works correctly.
** Comments:
**	Tests all requirements of the FieldVariable interface, such as max & min values, interpolation etc,
**	plus any FE-specific additions made at the field Variable level.
**
** $Id: testTimeIntegration.c 3665 2006-07-04 04:56:29Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Base.h"

#include "Discretisation/Geometry/Geometry.h"
#include "Discretisation/Shape/Shape.h"
#include "Discretisation/Mesh/Mesh.h"
#include "Discretisation/Utils/Utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

double GetDt( void* context ) {
	return 0.1;
}

Bool ConstantTimeDeriv( void* timeIntegratee, Index array_I, double* timeDeriv ) {
	timeDeriv[0] = 2.0 * array_I;
	timeDeriv[1] = -1.0 * array_I;

	return True;
}
Bool ConstantTimeDeriv2( void* timeIntegratee, Index array_I, double* timeDeriv ) {
	timeDeriv[0] = -0.5 * array_I;
	timeDeriv[1] = 3.0 * array_I;
	
	return True;
}
Bool LinearTimeDeriv( void* timeIntegratee, Index array_I, double* timeDeriv ) {
	double time = TimeIntegratee_GetTime( timeIntegratee );

	timeDeriv[0] = 2.0 * array_I * time;
	timeDeriv[1] = -1.0 * array_I * time;
	
	return True;
}
Bool LinearTimeDeriv2( void* timeIntegratee, Index array_I, double* timeDeriv ) {
	double time = TimeIntegratee_GetTime( timeIntegratee );

	timeDeriv[0] = -0.5 * array_I * time;
	timeDeriv[1] = 3.0 * array_I * time;
	
	return True;
}
Bool CubicTimeDeriv( void* timeIntegratee, Index array_I, double* timeDeriv ) {
	double time = TimeIntegratee_GetTime( timeIntegratee );

	timeDeriv[0] = 2.0 * array_I * ( time * time * time - time*time );
	timeDeriv[1] = -1.0 * array_I * ( time * time * time - time*time );
	
	return True;
}
Bool CubicTimeDeriv2( void* timeIntegratee, Index array_I, double* timeDeriv ) {
	double time = TimeIntegratee_GetTime( timeIntegratee );

	timeDeriv[0] = -0.5 * array_I * ( time * time * time - time*time );
	timeDeriv[1] = 3.0 * array_I * ( time * time * time - time*time );
	
	return True;
}

TimeIntegratee_CalculateTimeDerivFunction* GetFunctionPtr( Name derivName ) {
	if ( strcasecmp( derivName, "Linear" ) == 0 )
		return LinearTimeDeriv;
	else if ( strcasecmp( derivName, "Linear2" ) == 0 )
		return LinearTimeDeriv2;
	else if ( strcasecmp( derivName, "Cubic" ) == 0 )
		return CubicTimeDeriv;
	else if ( strcasecmp( derivName, "Cubic2" ) == 0 )
		return CubicTimeDeriv2;
	else if ( strcasecmp( derivName, "Constant" ) == 0 )
		return ConstantTimeDeriv;
	else if ( strcasecmp( derivName, "Constant2" ) == 0 )
		return ConstantTimeDeriv2;
	else
		Journal_Firewall( 0 , Journal_Register( Error_Type, CURR_MODULE_NAME ),
				"Don't understand DerivName '%s'\n", derivName  );

	return NULL;
}

void TestContextType( void* timeIntegratee, Stg_Class* data ) {
	Stream* stream = Journal_Register (Info_Type, "myStream");

	Journal_Printf( stream, "In func %s\n", __func__ );
	assert( data->type == DiscretisationContext_Type ); 
}
void TestVariableType( void* timeIntegratee, Stg_Class* data ) {
	Stream* stream = Journal_Register (Info_Type, "myStream");

	Journal_Printf( stream, "In func %s\n", __func__ );
	assert( data->type == Variable_Type ); 
}
	
int main( int argc, char* argv[] ) {
	MPI_Comm                   CommWorld;
	int                        rank;
	int                        numProcessors;
	int                        procToWatch;
	XML_IO_Handler*            ioHandler;
	Dictionary*                dictionary;
	TimeIntegrator*            timeIntegrator;
	TimeIntegratee*            timeIntegratee;
	TimeIntegratee*            timeIntegrateeList[2];
	DiscretisationContext*     context;
	Variable*                  variable;
	Variable*                  variableList[2];
	double*                    array;
	double*                    array2;
	Index                      size0              = 11;
	Index                      size1              = 7;
	Index                      array_I;
	Index                      timestep           = 0;
	Index                      maxTimesteps       = 10;
	Stream*                    stream;
	Bool                       simultaneous;
	unsigned                   order;
	double                     error              = 0.0;
	Name                       derivName;
	double                     tolerance          = 0.001;
	Index                      integratee_I;
	Index                      integrateeCount    = 2;
	
	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size( CommWorld, &numProcessors );
	MPI_Comm_rank( CommWorld, &rank );
	
	Base_Init( &argc, &argv );
	
	DiscretisationGeometry_Init( &argc, &argv );
	DiscretisationShape_Init( &argc, &argv );
	DiscretisationMesh_Init( &argc, &argv );
	DiscretisationUtils_Init( &argc, &argv );
	MPI_Barrier( CommWorld ); /* Ensures copyright info always come first in output */
	
	stream = Journal_Register (Info_Type, "myStream");

	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}
	if( rank == procToWatch ) printf( "Watching rank: %i\n", rank );

	/* Create Context */
	ioHandler = XML_IO_Handler_New();
	dictionary = Dictionary_New();
	IO_Handler_ReadAllFromCommandLine( ioHandler, argc, argv, dictionary );
	context = _DiscretisationContext_New( 
			sizeof(DiscretisationContext), 
			DiscretisationContext_Type, 
			_DiscretisationContext_Delete, 
			_DiscretisationContext_Print,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			"discretisationContext",
			True,
			NULL,
			0,0,
			CommWorld, dictionary );
	ContextEP_Append( context, AbstractContext_EP_Dt, GetDt );

	printf ("!!! info %d\n", Stream_IsEnable( Journal_Register( Info_Type, "TimeIntegrator" ) ) );

	/* Create Stuff */
	order           = Dictionary_GetUnsignedInt_WithDefault( dictionary, "order", 1 );
	simultaneous    = Dictionary_GetBool_WithDefault( dictionary, "simultaneous", False );
	variableList[0] = Variable_NewVector( "testVariable",  Variable_DataType_Double, 2, &size0, (void**)&array, NULL );
	variableList[1] = Variable_NewVector( "testVariable2", Variable_DataType_Double, 2, &size1, (void**)&array2, NULL );
	timeIntegrator  = TimeIntegrator_New( "testTimeIntegrator", order, simultaneous, NULL, NULL );
	timeIntegrateeList[0] = TimeIntegratee_New( "testTimeIntegratee0", timeIntegrator, variableList[0],
		0, NULL, True );
	timeIntegrateeList[1] = TimeIntegratee_New( "testTimeIntegratee1", timeIntegrator, variableList[1],
		0, NULL, True );

	derivName = Dictionary_GetString( dictionary, "DerivName0" );
	timeIntegrateeList[0]->_calculateTimeDeriv = GetFunctionPtr( derivName );
	Journal_Printf( stream, "DerivName0 - %s\n", derivName );
	derivName = Dictionary_GetString( dictionary, "DerivName1" );
	timeIntegrateeList[1]->_calculateTimeDeriv = GetFunctionPtr( derivName );
	Journal_Printf( stream, "DerivName1 - %s\n", derivName );

	/* Print Stuff to file */
	Journal_PrintValue( stream, order );
	Journal_PrintBool( stream, simultaneous );

	/* Add stuff to EPs */
	TimeIntegrator_AppendSetupEP( timeIntegrator, "start1", TestContextType, CURR_MODULE_NAME, context );
	TimeIntegrator_AppendFinishEP( timeIntegrator, "finish1", TestVariableType, CURR_MODULE_NAME, variableList[0] );
	TimeIntegrator_PrependSetupEP( timeIntegrator, "start0", TestVariableType, CURR_MODULE_NAME, variableList[0] );
	TimeIntegrator_PrependFinishEP( timeIntegrator, "finish0", TestContextType, CURR_MODULE_NAME, context );

	/* Build */
	Stg_Component_Build( variableList[0], context, False );
	Stg_Component_Build( variableList[1], context, False );
	Stg_Component_Build( timeIntegrator, context, False );
	Stg_Component_Build( timeIntegrateeList[0], context, False );
	Stg_Component_Build( timeIntegrateeList[1], context, False );
	array = Memory_Alloc_Array( double, 2 * size0, "name" );
	array2 = Memory_Alloc_Array( double, 2 * size1, "name" );
	
	/* Initialise */
	memset( array, 0, sizeof(double) * 2 * size0 );
	memset( array2, 0, sizeof(double) * 2 * size1 );
	Stg_Component_Initialise( timeIntegrator, context, False );
	Stg_Component_Initialise( variableList[0], context, False );
	Stg_Component_Initialise( variableList[1], context, False );
	Stg_Component_Initialise( timeIntegrateeList[0], context, False );
	Stg_Component_Initialise( timeIntegrateeList[1], context, False );

	for ( timestep = 0.0 ; timestep < maxTimesteps ; timestep ++ ) {
		Journal_Printf( stream, "Step %u - Time = %.3g\n", timestep, context->currentTime );

		Stg_Component_Execute( timeIntegrator, context, True );
		context->currentTime += AbstractContext_Dt( context );

		for ( integratee_I = 0 ; integratee_I < integrateeCount ; integratee_I++ ) {
			timeIntegratee = timeIntegrateeList[ integratee_I ];
			variable       = variableList[ integratee_I ];
			for ( array_I = 0 ; array_I < variable->arraySize ; array_I++ ) {
				if ( timeIntegratee->_calculateTimeDeriv == ConstantTimeDeriv ) {
					error += fabs( Variable_GetValueAtDouble( variable, array_I, 0 ) - 2.0 * array_I * context->currentTime );
					error += fabs( Variable_GetValueAtDouble( variable, array_I, 1 ) + array_I * context->currentTime );
				}
				else if ( timeIntegratee->_calculateTimeDeriv == ConstantTimeDeriv2 ) {
					error += fabs( Variable_GetValueAtDouble( variable, array_I, 0 ) + 0.5 * array_I * context->currentTime );
					error += fabs( Variable_GetValueAtDouble( variable, array_I, 1 ) - 3 * array_I * context->currentTime );
				}
				else if ( timeIntegratee->_calculateTimeDeriv == LinearTimeDeriv ) {
					error += fabs( Variable_GetValueAtDouble( variable, array_I, 0 ) - 
							array_I * context->currentTime * context->currentTime );

					error += fabs( Variable_GetValueAtDouble( variable, array_I, 1 ) +
							0.5 * array_I * context->currentTime * context->currentTime );
				}
				else if ( timeIntegratee->_calculateTimeDeriv == LinearTimeDeriv2 ) {
					error += fabs( Variable_GetValueAtDouble( variable, array_I, 0 ) + 
							0.25 * array_I * context->currentTime * context->currentTime );

					error += fabs( Variable_GetValueAtDouble( variable, array_I, 1 ) -
							1.5 * array_I * context->currentTime * context->currentTime );
				}				
				else if ( timeIntegratee->_calculateTimeDeriv == CubicTimeDeriv ) {
					error += fabs( Variable_GetValueAtDouble( variable, array_I, 0 ) - 
							2.0 * array_I * ( 0.25 * pow( context->currentTime, 4.0 ) - pow( context->currentTime, 3.0)/3.0));

					error += fabs( Variable_GetValueAtDouble( variable, array_I, 1 ) +
							array_I * ( 0.25 * pow( context->currentTime, 4.0 ) - pow( context->currentTime, 3.0 )/3.0));
				}
				else if ( timeIntegratee->_calculateTimeDeriv == CubicTimeDeriv2 ) {
					error += fabs( Variable_GetValueAtDouble( variable, array_I, 0 ) + 
							0.5 * array_I * ( 0.25 * pow( context->currentTime, 4.0 ) - pow( context->currentTime, 3.0)/3.0));

					error += fabs( Variable_GetValueAtDouble( variable, array_I, 1 ) -
							3.0 * array_I * ( 0.25 * pow( context->currentTime, 4.0 ) - pow( context->currentTime, 3.0 )/3.0));
				}				
				else 
					Journal_Firewall( 0 , Journal_Register( Error_Type, CURR_MODULE_NAME ),
							"Don't understand _calculateTimeDeriv = %p\n", timeIntegratee->_calculateTimeDeriv );
			}
		}
	}
	
	if ( error < tolerance ) 
		Journal_Printf( stream, "Passed\n" );
	else
		Journal_Printf( stream, "Failed - Error = %lf\n", error );
	
	/* Destroy stuff */
	Memory_Free( array );
	Stg_Class_Delete( variable );
	Stg_Class_Delete( timeIntegrator );
	Stg_Class_Delete( timeIntegrateeList[0] );
	Stg_Class_Delete( timeIntegrateeList[1] );
	
	DiscretisationUtils_Finalise();
	DiscretisationMesh_Finalise();
	DiscretisationShape_Finalise();
	DiscretisationGeometry_Finalise();
	
	Base_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0; /* success */
}

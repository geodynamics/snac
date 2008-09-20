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
** $Id: testOperatorFieldVariable.c 3884 2006-10-26 05:26:19Z KathleenHumble $
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

/* Simulate Solid Body Rotation */
InterpolationResult dummyInterpolateValueAt( void* sdVariable, Coord coord, double* value ) {  
	value[0] = -coord[1];
	value[1] =  coord[0];
	value[2] =  coord[2];
	return LOCAL;
}

void dummyWithinElement( void* sdVariable, Element_DomainIndex dEl_I, Coord coord, double* value ) { return ; }
double dummyGetMinGlobalValue( void* sdVariable ) { return 0; }
double dummyGetMaxGlobalValue( void* sdVariable ) { return 1; }
void dummyGetMinAndMaxLocalCoords( void* sdVariable, Coord min, Coord max ) { 
	min[0] = -1.0;
	min[1] = -2.3;
	min[2] = -4.0;
	
	max[0] = 11.0;
	max[1] = 12.3;
	max[2] = 14.0;
	return ; 
}
void dummyGetMinAndMaxGlobalCoords( void* sdVariable, Coord min, Coord max ) {	min[0] = -1.0;
	min[0] = -111.0;
	min[1] = -112.3;
	min[2] = -114.0;
	
	max[0] = 111.0;
	max[1] = 112.3;
	max[2] = 114.0;
	return ; 
}


int main(int argc, char *argv[]) {
	MPI_Comm                    CommWorld;
	int                         rank;
	int                         procCount;
	Stream*                     stream;
	FieldVariable_Register*	    fV_Register;
	FieldVariable*              velocityField;
	OperatorFieldVariable* velSquared2D;
	OperatorFieldVariable* velSquared3D;
	OperatorFieldVariable* velMag2D;
	OperatorFieldVariable* velMag3D;
	double                      coord[3][3] = {{ 0.4 , 2.0 , 7.0 }, { -0.2 , 6.0 , 2.0 },{ 0.3 , -2.0 , -13.0 }} ;
	double                      value[3];
	Index                       index;
	Coord                       min, max;
	
	/* Initialise MPI, get world info */
	MPI_Init(&argc, &argv);
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size(CommWorld, &procCount);
	MPI_Comm_rank(CommWorld, &rank);
	
	Base_Init( &argc, &argv );
	
	DiscretisationGeometry_Init( &argc, &argv );
	DiscretisationShape_Init( &argc, &argv );
	DiscretisationMesh_Init( &argc, &argv );
	DiscretisationUtils_Init( &argc, &argv );
	MPI_Barrier( CommWorld ); /* Ensures copyright info always come first in output */

	stream = Journal_Register (Info_Type, "myStream");

	fV_Register = FieldVariable_Register_New();

	/* constructor  */
	velocityField = _FieldVariable_New( 
			sizeof(FieldVariable),
			FieldVariable_Type,
			_FieldVariable_Delete,
			_FieldVariable_Print,
			_FieldVariable_Copy,
			(Stg_Component_DefaultConstructorFunction*)FieldVariable_DefaultNew,
			_FieldVariable_Construct,
			_FieldVariable_Build,
			_FieldVariable_Initialise,
			_FieldVariable_Execute,
			_FieldVariable_Destroy,
			"Velocity", 
			True,
			dummyInterpolateValueAt,
			dummyGetMinGlobalValue, 
			dummyGetMaxGlobalValue,
			dummyGetMinAndMaxLocalCoords, 
			dummyGetMinAndMaxGlobalCoords, 
			3,
			3,
			False,
			MPI_COMM_WORLD,
			fV_Register );

	/* Construct Unary Operator Field Variables */
	velSquared2D = OperatorFieldVariable_NewUnary( 
			"VelocitySquaredField2D", 
			velocityField, 
			"VectorSquare" );
	velSquared2D->_operator->operandDofs = 2;

	velSquared3D = OperatorFieldVariable_NewUnary( 
			"VelocitySquaredField3D", 
			velocityField, 
			"VectorSquare" );

	velMag2D = OperatorFieldVariable_NewUnary( 
			"VelocityMagnitudeField2D", 
			velocityField,
			"Magnitude" );
	velMag2D->_operator->operandDofs = 2;

	velMag3D = OperatorFieldVariable_NewUnary( 
			"VelocityMagnitudeField3D",
			velocityField,
			"Magnitude" );

	/* Do Interpolations */
	for ( index = 0 ; index < 3 ; index++ ) {
		Journal_Printf( stream, "coord = ");
		StGermain_PrintVector( stream, coord[ index ], 3 );

		Journal_Printf( stream, "velocity = ");
		FieldVariable_InterpolateValueAt( velocityField, coord[ index ], value );
		StGermain_PrintVector( stream, value, 3 );

		Journal_Printf( stream, "velocity squared 2d = ");
		FieldVariable_InterpolateValueAt( velSquared2D, coord[ index ], value );
		StGermain_PrintVector( stream, value, 1 );
		Journal_Printf( stream, "velocity squared 3d = ");
		FieldVariable_InterpolateValueAt( velSquared3D, coord[ index ], value );
		StGermain_PrintVector( stream, value, 1 );

		Journal_Printf( stream, "velocity magnitude 2d = ");
		FieldVariable_InterpolateValueAt( velMag2D, coord[ index ], value );
		StGermain_PrintVector( stream, value, 1 );
		Journal_Printf( stream, "velocity magnitude 3d = ");
		FieldVariable_InterpolateValueAt( velMag3D, coord[ index ], value );
		StGermain_PrintVector( stream, value, 1 );
		Journal_Printf(stream, "\n");
	}

	Journal_Printf( stream , "testing min max local coords:\n" );
	Journal_Printf( stream, "velocity:\n");
	FieldVariable_GetMinAndMaxLocalCoords( velocityField, min, max );
	StGermain_PrintNamedVector( stream, min, 3 );
	StGermain_PrintNamedVector( stream, max, 3 );

	Journal_Printf( stream, "velocity squared 2d = \n");
	FieldVariable_GetMinAndMaxLocalCoords( velSquared2D, min, max );
	StGermain_PrintNamedVector( stream, min, 3 );
	StGermain_PrintNamedVector( stream, max, 3 );

	Journal_Printf( stream , "testing min max global coords:\n" );
	Journal_Printf( stream, "velocity:\n");
	FieldVariable_GetMinAndMaxGlobalCoords( velocityField, min, max );
	StGermain_PrintNamedVector( stream, min, 3 );
	StGermain_PrintNamedVector( stream, max, 3 );

	Journal_Printf( stream, "velocity squared 2d = \n");
	FieldVariable_GetMinAndMaxGlobalCoords( velSquared2D, min, max );
	StGermain_PrintNamedVector( stream, min, 3 );
	StGermain_PrintNamedVector( stream, max, 3 );

	/* Clean Up */
	Stg_Class_Delete(fV_Register);
	
	DiscretisationUtils_Finalise();
	DiscretisationMesh_Finalise();
	DiscretisationShape_Finalise();
	DiscretisationGeometry_Finalise();
	
	Base_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0; /* success */
}

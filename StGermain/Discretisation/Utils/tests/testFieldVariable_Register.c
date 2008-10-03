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
** $Id: testFieldVariable_Register.c 3884 2006-10-26 05:26:19Z KathleenHumble $
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

InterpolationResult dummyInterpolateValueAt( void* sdVariable, Coord coord, double* value ) { return OUTSIDE_GLOBAL; }
void dummyWithinElement( void* sdVariable, Element_DomainIndex dEl_I, Coord coord, double* value ) { return ; }
double dummyGetMinGlobalValue( void* sdVariable ) { return 0; }
double dummyGetMaxGlobalValue( void* sdVariable ) { return 1; }
void dummyGetMinAndMaxLocalCoords( void* sdVariable, Coord min, Coord max ) { return ; }
void dummyGetMinAndMaxGlobalCoords( void* sdVariable, Coord min, Coord max ) { return ; }


int main(int argc, char *argv[])
{
	MPI_Comm		CommWorld;
	int			rank;
	int			procCount;
	Stream*			stream;
	FieldVariable_Register*	fV_Register;
	FieldVariable*		testFVs[] = { NULL, NULL, NULL };
	Name			fvNames[] = { "testFV1", "testFV2", "testFV3" };
	Index			ii;
	Index			fV_Index;
	
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

	/* abstract class, so have to hack constructor  */
	for (ii=0; ii < 3; ii++ ) {
		testFVs[ii] = _FieldVariable_New( sizeof(FieldVariable), FieldVariable_Type, _Stg_Component_Delete,
			_Stg_Component_Print, NULL, (Stg_Component_DefaultConstructorFunction*)FieldVariable_DefaultNew, _Variable_Construct, _Variable_Build, 
			_Variable_Initialise, _Variable_Execute, _Variable_Destroy, fvNames[ii], True,
			dummyInterpolateValueAt, 
			dummyGetMinGlobalValue, dummyGetMaxGlobalValue,
			dummyGetMinAndMaxLocalCoords, dummyGetMinAndMaxGlobalCoords, 
			0, /* field components */
			3, /* dim */
			False,
			MPI_COMM_WORLD,
			fV_Register );
	}	

	Print( fV_Register, stream );

	/* test the get functions */
	fV_Index = FieldVariable_Register_GetIndex( fV_Register, fvNames[1] );
	Journal_Printf( stream, "\nIndex of fieldVar named \"%s\" = %d\n", fvNames[1], fV_Index );
	Journal_Printf( stream, "Index %d -> fieldVar with name \"%s\"\n", fV_Index,
		(FieldVariable_Register_GetByIndex( fV_Register, fV_Index ) )->name );
	Journal_Printf( stream, "Searching directly using name \"%s\" -> fieldVar with name \"%s\"\n\n", fvNames[1],
		(FieldVariable_Register_GetByName( fV_Register, fvNames[1] ) )->name );

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

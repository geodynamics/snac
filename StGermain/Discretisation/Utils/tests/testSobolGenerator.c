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
** $Id: testSobolGenerator.c 3555 2006-05-10 07:05:46Z PatrickSunter $
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

int main(int argc, char *argv[]) {
	MPI_Comm                    CommWorld;
	int                         rank;
	int                         procCount;
	Stream*                     stream;
	SobolGenerator*             sobolGenerator;
	Index                       index;
	Index                       sobol_I;
	int                         bit_I;
	char                        name[50];
	double                      result;
	
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
	
	Stream_RedirectFile_WithPrependedPath( stream, "output", "RightmostBit.dat" );
	Journal_Printf( stream, " *********************** Testing _SobolGenerator_FindRightmostZeroBit *******************\n" );
	for ( index = 0 ; index < 30 ; index++ ) {
		for ( bit_I = sizeof( Index ) * 4 - 1 ; bit_I >= 0 ; bit_I-- )
			Journal_Printf( stream, "%u", index & 1 << bit_I ? 1 : 0 );
		Journal_Printf( stream, " number %u: %u\n", index, _SobolGenerator_FindRightmostZeroBit( index ) );
	}
	
	/* constructor  */
	for ( sobol_I = 0 ; sobol_I < 6 ; sobol_I++ ) {
		sprintf( name, "testGenerator%u", sobol_I );
		Stream_RedirectFile_WithPrependedPath( stream, "output", name );
		sobolGenerator = SobolGenerator_NewFromTable( name );

		Stg_Class_Print( sobolGenerator, stream );

		Journal_Printf( stream," ****************** Testing SobolGenerator_GetDirectionalNumber ***************\n" );
		for ( index = 0 ; index < 30 ; index++ ) 
			SobolGenerator_GetDirectionalNumber( sobolGenerator, index );
		
		Stg_Class_Print( sobolGenerator, stream );

		/* Checking up to 200000 numbers - this number is arbitary - 
		 * it's only limited because we don't want file size to be huge
		 * This number is intentionally over 25535 = 2^16 - 1 because there was a time when numbers repeated after this */
		for ( index = 0 ; index < 200000 ; index++ ) {
			result = SobolGenerator_GetNextNumber(sobolGenerator);

			assert( fabs( result - SobolGenerator_GetNumberByIndex(sobolGenerator, index)) < 1e-8 );

			/* Only dump subset of data - this output criterion is completely arbitary */
			if ( index % 773 == 3 )
				Journal_Printf( stream, "%.4g\n", result );
		}

		Stg_Class_Delete( sobolGenerator );
	}

	DiscretisationUtils_Finalise();
	DiscretisationMesh_Finalise();
	DiscretisationShape_Finalise();
	DiscretisationGeometry_Finalise();
	
	Base_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0; /* success */
}


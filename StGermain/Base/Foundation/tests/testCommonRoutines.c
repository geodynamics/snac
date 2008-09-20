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
** $Id: testCommonRoutines.c 3706 2006-07-25 02:19:37Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "Base/Foundation/Foundation.h"
#include "Regresstor/libRegresstor/Regresstor.h"

#include "JournalWrappers.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <mpi.h>
#include <string.h>
#include <math.h>

void TestLMS( Stream* stream, char* string1, char* string2, Bool caseSensitive ) {
	Journal_Printf( stream, "Longest case-%ssensitive matching subsequence length of '%s' and '%s' is %d.\n", 
		( caseSensitive ? "" : "in" ), string1, string2, 
		Stg_LongestMatchingSubsequenceLength( string1, string2, caseSensitive ) );
}

void TestIsStringNumeric( Stream* stream, char* string ) {
	Journal_Printf( stream, "Stg_StringIsNumeric( '%s' ) = %s\n", 
			string, Stg_StringIsNumeric( string ) ? "True" : "False" );
}
void TestIsStringEmpty( Stream* stream, char* string ) {
	Journal_Printf( stream, "Stg_StringIsEmpty( '%s' ) = %s\n", 
			string, Stg_StringIsEmpty( string ) ? "True" : "False" );
}

int main(int argc, char *argv[])
{
	int			rank;
	int			procCount;
	int			procToWatch;
	Stream*			stream;

	/* Initialise MPI, get world info */
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &procCount);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	BaseFoundation_Init( &argc, &argv );

	RegressionTest_Init( "Base/Foundation/ObjectList" );

	stream = Journal_Register( "info", "myStream" );
	
	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}

	if ( rank == procToWatch ) {
		Index  nTestValues = 7;
		double testValues[] = {
			0.0,
			3.4562e-30,
			9.7324,
			97.654,
			104.321,
			-13762.1,
			0.0043253 };
		double tolerances[] = {
			1e-40,
			1e-40,
			1e-12,
			1e-12,
			1e-12,
			1e-12,
			1e-12 };
		double expectedRoundedToDecPlaces[7][4] = {
			{ 0.0, 0.0, 0.0, 0.0 },
			{ 0.0, 0.0, 0.0, 0.0 },
			{ 10., 9.7, 9.73, 9.732 },
			{ 98., 97.7, 97.65, 97.654 },
			{ 104., 104.3, 104.32, 104.321 },
			{ -13762., -13762.1, -13762.10, -13762.100 },
			{ 0.,    0.0, 0.00, 0.004 } };
		double expectedRoundedToSigFigs[7][4] = {
			{ 0.0, 0.0, 0.0, 0.0 },
			{ 0.0, 3e-30, 3.5e-30, 3.46e-30 },
			{ 0., 10, 9.7, 9.73 },
			{ 0., 100, 98, 97.7 },
			{ 0., 100, 100, 104 },
			{ 0., -10000, -14000, -13800 },
			{ 0., 0.004, 0.0043, 0.00433 } };
		double roundedValue;
		double errorMargin;
		Index testValue_I;
		Index nDecPlaces;
		Index nSigFigs;
		
		TestLMS( stream, "Acrobat", "BOAT", True );
		TestLMS( stream, "Abracadabra", "Yabbadabbadoo", True );
		TestLMS( stream, "Abracadabra", "Yabbadabbadoo", False );
		TestLMS( stream, "Python", "PythonShape", False );
		
		TestIsStringNumeric( stream, "nan" );
		TestIsStringNumeric( stream, "567" );
		TestIsStringNumeric( stream, "1.0e90" );
		TestIsStringNumeric( stream, "1e90e80" );
		TestIsStringNumeric( stream, ".asdfasdf" );
		TestIsStringNumeric( stream, ".0032" );
		TestIsStringNumeric( stream, ".0032.00" );
		
		TestIsStringEmpty( stream, "\t \n" );
		TestIsStringEmpty( stream, "asdf" );
		TestIsStringEmpty( stream, "    " );
		TestIsStringEmpty( stream, "    \n" );
		TestIsStringEmpty( stream, "  sdf  \n" );

		printf("\nTesting rounding to certain number of decimal places:\n");
		for ( testValue_I = 0; testValue_I < nTestValues; testValue_I++ ) {
			for ( nDecPlaces = 0; nDecPlaces <=3; nDecPlaces++ ) {
				roundedValue = StG_RoundDoubleToNDecimalPlaces(
					testValues[testValue_I], nDecPlaces );
				errorMargin = fabs( roundedValue -
					expectedRoundedToDecPlaces[testValue_I][nDecPlaces] );

				if ( errorMargin <= tolerances[testValue_I] ) {
					printf( "Value %8g rounded to %u dec places was within "
						"tolerance of expected value %8g.\n",
						testValues[testValue_I], nDecPlaces,
						expectedRoundedToDecPlaces[testValue_I][nDecPlaces] );
				}
				else {
					printf( "Value %8g rounded to %u dec places was not within "
						"tolerance of expected value %8g.\n",
						testValues[testValue_I], nDecPlaces,
						expectedRoundedToDecPlaces[testValue_I][nDecPlaces] );
					printf( "(error margin was %.8g)\n", errorMargin );
				}
			}
		}

		printf("\nTesting rounding to certain number of significant figures:\n");
		for ( testValue_I = 0; testValue_I < nTestValues; testValue_I++ ) {
			for ( nSigFigs = 1; nSigFigs <=3; nSigFigs++ ) {
				roundedValue = StG_RoundDoubleToNSigFigs(
					testValues[testValue_I], nSigFigs );
				errorMargin = fabs( roundedValue -
					expectedRoundedToSigFigs[testValue_I][nSigFigs] );
				if ( errorMargin <= tolerances[testValue_I] ) {
					printf( "Value %8g rounded to %u sig. figures was within "
						"tolerance of expected value %8g.\n",
						testValues[testValue_I], nSigFigs,
						expectedRoundedToSigFigs[testValue_I][nSigFigs] );
				}
				else {
					printf( "Value %8g rounded to %u sig. figures was not within "
						"tolerance of expected value %8g.\n",
						testValues[testValue_I], nSigFigs,
						expectedRoundedToSigFigs[testValue_I][nSigFigs] );
					printf( "(error margin was %.8g)\n", errorMargin );
				}
			}
		}
	}

	RegressionTest_Finalise();

	BaseFoundation_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0; /* success */
}

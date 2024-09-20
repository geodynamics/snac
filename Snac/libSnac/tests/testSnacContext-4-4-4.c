/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003,
**	Steve Quenette, 110 Victoria Street, Melbourne, Victoria, 3053, Australia.
**	Californian Institute of Technology, 1200 East California Boulevard, Pasadena, California, 91125, USA.
**	University of Texas, 1 University Station, Austin, Texas, 78712, USA.
**
** Authors:
**	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
**	Stevan M. Quenette, Visitor in Geophysics, Caltech.
**	Luc Lavier, Research Scientist, The University of Texas. (luc@utig.ug.utexas.edu)
**	Luc Lavier, Research Scientist, Caltech.
**
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2, or (at your option) any
** later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**
** Role:
**	Tests StrainRate function for values of element strainRate.
**
** $Id: testSnacContext-4-4-4.c 3230 2006-06-19 18:00:52Z EunseoChoi $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "TestUtilities.h"

#include <stdio.h>
#include <stdlib.h>

#define TOL 1.0e-09

extern int				numProcessors;
extern int				procToWatch;
extern int				rank;
extern MPI_Comm			CommWorld;
extern Dictionary*			dictionary;
extern Snac_Context*			snacContext;

int main( int argc, char* argv[] ) {

	SnacTest_SetUp( argc, argv );

	/* Step the context solver */
	AbstractContext_Step( snacContext, snacContext->dt );

	if( rank == procToWatch ) {
		Element_LocalIndex 	element_lI;
		Tetrahedra_Index tetra_I;

		printf( "dt: %g\n", snacContext->dt );
		/* The left half of elements should have zero strain rate. */
		for( element_lI = 0; element_lI < snacContext->mesh->elementLocalCount; element_lI +=2 ) {
			for (tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
				if( Snac_Element_At( snacContext, 0)->tetra[tetra_I].strainRate[0][0] != 0.0 ||
					Snac_Element_At( snacContext, 0)->tetra[tetra_I].strainRate[1][1] != 0.0 ||
					Snac_Element_At( snacContext, 0)->tetra[tetra_I].strainRate[2][2] != 0.0 ||
					Snac_Element_At( snacContext, 0)->tetra[tetra_I].strainRate[0][1] != 0.0 ||
					Snac_Element_At( snacContext, 0)->tetra[tetra_I].strainRate[0][2] != 0.0 ||
					Snac_Element_At( snacContext, 0)->tetra[tetra_I].strainRate[1][2] != 0.0 ) {
					printf( "Element %u, tetra %u, strainRate test failed: (%12g %12g %12g) (%12g %12g %12g)\n",
							element_lI,tetra_I,
							Snac_Element_At( snacContext, element_lI )->tetra[tetra_I].strainRate[0][0],
							Snac_Element_At( snacContext, element_lI )->tetra[tetra_I].strainRate[1][1],
							Snac_Element_At( snacContext, element_lI )->tetra[tetra_I].strainRate[2][2],
							Snac_Element_At( snacContext, element_lI )->tetra[tetra_I].strainRate[0][1],
							Snac_Element_At( snacContext, element_lI )->tetra[tetra_I].strainRate[0][2],
							Snac_Element_At( snacContext, element_lI )->tetra[tetra_I].strainRate[1][2] );
				}
			}
		}
		/* The right half of elements should have strain rate tensors equal to (0.02,0,0,0,0,0)
		   within a tolerance range. */
		for( element_lI = 1; element_lI < snacContext->mesh->elementLocalCount; element_lI +=2 ) {

			for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
				if( fabs(Snac_Element_At( snacContext, element_lI )->tetra[tetra_I].strainRate[0][0] - 0.02) > TOL ||
					fabs(Snac_Element_At( snacContext, element_lI )->tetra[tetra_I].strainRate[1][1]) > TOL ||
					fabs(Snac_Element_At( snacContext, element_lI )->tetra[tetra_I].strainRate[2][2]) > TOL ||
					fabs(Snac_Element_At( snacContext, element_lI )->tetra[tetra_I].strainRate[0][1]) > TOL ||
					fabs(Snac_Element_At( snacContext, element_lI )->tetra[tetra_I].strainRate[0][2]) > TOL ||
					fabs(Snac_Element_At( snacContext, element_lI )->tetra[tetra_I].strainRate[1][2]) > TOL )
					{
						printf( "Element %u, tetra %u, strainRate test failed: (%12g %12g %12g) (%12g %12g %12g)\n",
								element_lI,tetra_I,
								Snac_Element_At( snacContext, element_lI )->tetra[tetra_I].strainRate[0][0],
								Snac_Element_At( snacContext, element_lI )->tetra[tetra_I].strainRate[1][1],
								Snac_Element_At( snacContext, element_lI )->tetra[tetra_I].strainRate[2][2],
								Snac_Element_At( snacContext, element_lI )->tetra[tetra_I].strainRate[0][1],
								Snac_Element_At( snacContext, element_lI )->tetra[tetra_I].strainRate[0][2],
								Snac_Element_At( snacContext, element_lI )->tetra[tetra_I].strainRate[1][2] );
					}
			}
		}
	}

	SnacTest_TearDown();

	return 0; /* success */
}

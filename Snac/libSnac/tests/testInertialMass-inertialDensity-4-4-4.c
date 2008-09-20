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
**	Tests InertialMass function for ....
**
** $Id: testInertialMass-inertialDensity-4-4-4.c 1792 2004-07-30 05:42:39Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "mpi.h"
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "TestUtilities.h"

#include "stdio.h"
#include "stdlib.h"


extern int				numProcessors;
extern int				procToWatch;
extern int				rank;
extern MPI_Comm			CommWorld;
extern Dictionary*			dictionary;
extern Snac_Context*			snacContext;

int main( int argc, char* argv[] ) {
	Node_LocalIndex		nodeIndex;
	double				speedOfSound;	
	Tetrahedra_Index tetraIndex;
	Snac_InertialMassLocal inertialMassL0;
	Element_Index elementIndex;
	const Area globalMinArea = Tetrahedra_Max_Propagation_Length;
	
	SnacTest_SetUp( argc, argv );

	printf( "Global Min Projected Area set to: %g\n", globalMinArea );
	
	SnacTest_PrintElementZeroInfo();

	inertialMassL0.node_lI = 0;
	inertialMassL0.minProjectedArea = globalMinArea;
	Snac_InertialMass( (Context*)snacContext, &inertialMassL0 );
	printf( "Node: 0, InertialMass: %g\n", inertialMassL0.inertialMass );
	

#if 0
	/* For each element, compare to the first element's */
	for( elementIndex = 1; elementIndex < snacContext->mesh->elementLocalCount; elementIndex++ ) {
		Bool error;
		Snac_StrainRateLocal strainRateL;
		
		/* For each tetrahedra of the element */
		strainRateL.element_lI = elementIndex;
		Snac_StrainRate( (Context*)snacContext, &strainRateL );
		for( tetraIndex = 0, error = False; tetraIndex < Tetrahedra_Count; tetraIndex++ ) {
			if( strainRateL.elementMinProjectedArea != strainRateL0.elementMinProjectedArea ) {
				printf( "Element: %u, MinProjectedArea: %g\n", elementIndex, strainRateL.elementMinProjectedArea );
				error = True;
			}
		}
		
		if( error ) {
			printf( "Element: %u, Coords: (%g %g %g), (%g %g %g), (%g %g %g), (%g %g %g), (%g %g %g) (%g %g %g) (%g %g %g) (%g %g %g)\n",
				elementIndex,
				Snac_Element_Node_P( snacContext, elementIndex, 0 )->coord[0],
				Snac_Element_Node_P( snacContext, elementIndex, 0 )->coord[1],
				Snac_Element_Node_P( snacContext, elementIndex, 0 )->coord[2],
				Snac_Element_Node_P( snacContext, elementIndex, 1 )->coord[0],
				Snac_Element_Node_P( snacContext, elementIndex, 1 )->coord[1],
				Snac_Element_Node_P( snacContext, elementIndex, 1 )->coord[2],
				Snac_Element_Node_P( snacContext, elementIndex, 2 )->coord[0],
				Snac_Element_Node_P( snacContext, elementIndex, 2 )->coord[1],
				Snac_Element_Node_P( snacContext, elementIndex, 2 )->coord[2],
				Snac_Element_Node_P( snacContext, elementIndex, 3 )->coord[0],
				Snac_Element_Node_P( snacContext, elementIndex, 3 )->coord[1],
				Snac_Element_Node_P( snacContext, elementIndex, 3 )->coord[2],
				Snac_Element_Node_P( snacContext, elementIndex, 4 )->coord[0],
				Snac_Element_Node_P( snacContext, elementIndex, 4 )->coord[1],
				Snac_Element_Node_P( snacContext, elementIndex, 4 )->coord[2],
				Snac_Element_Node_P( snacContext, elementIndex, 5 )->coord[0],
				Snac_Element_Node_P( snacContext, elementIndex, 5 )->coord[1],
				Snac_Element_Node_P( snacContext, elementIndex, 5 )->coord[2],
				Snac_Element_Node_P( snacContext, elementIndex, 6 )->coord[0],
				Snac_Element_Node_P( snacContext, elementIndex, 6 )->coord[1],
				Snac_Element_Node_P( snacContext, elementIndex, 6 )->coord[2],
				Snac_Element_Node_P( snacContext, elementIndex, 7 )->coord[0],
				Snac_Element_Node_P( snacContext, elementIndex, 7 )->coord[1],
				Snac_Element_Node_P( snacContext, elementIndex, 7 )->coord[2] );
			printf( "Element: %u, Vel: (%g %g %g), (%g %g %g), (%g %g %g), (%g %g %g), (%g %g %g) (%g %g %g) (%g %g %g) (%g %g %g)\n",
				elementIndex,
				Snac_Element_Node_P( snacContext, elementIndex, 0 )->velocity[0],
				Snac_Element_Node_P( snacContext, elementIndex, 0 )->velocity[1],
				Snac_Element_Node_P( snacContext, elementIndex, 0 )->velocity[2],
				Snac_Element_Node_P( snacContext, elementIndex, 1 )->velocity[0],
				Snac_Element_Node_P( snacContext, elementIndex, 1 )->velocity[1],
				Snac_Element_Node_P( snacContext, elementIndex, 1 )->velocity[2],
				Snac_Element_Node_P( snacContext, elementIndex, 2 )->velocity[0],
				Snac_Element_Node_P( snacContext, elementIndex, 2 )->velocity[1],
				Snac_Element_Node_P( snacContext, elementIndex, 2 )->velocity[2],
				Snac_Element_Node_P( snacContext, elementIndex, 3 )->velocity[0],
				Snac_Element_Node_P( snacContext, elementIndex, 3 )->velocity[1],
				Snac_Element_Node_P( snacContext, elementIndex, 3 )->velocity[2],
				Snac_Element_Node_P( snacContext, elementIndex, 4 )->velocity[0],
				Snac_Element_Node_P( snacContext, elementIndex, 4 )->velocity[1],
				Snac_Element_Node_P( snacContext, elementIndex, 4 )->velocity[2],
				Snac_Element_Node_P( snacContext, elementIndex, 5 )->velocity[0],
				Snac_Element_Node_P( snacContext, elementIndex, 5 )->velocity[1],
				Snac_Element_Node_P( snacContext, elementIndex, 5 )->velocity[2],
				Snac_Element_Node_P( snacContext, elementIndex, 6 )->velocity[0],
				Snac_Element_Node_P( snacContext, elementIndex, 6 )->velocity[1],
				Snac_Element_Node_P( snacContext, elementIndex, 6 )->velocity[2],
				Snac_Element_Node_P( snacContext, elementIndex, 7 )->velocity[0],
				Snac_Element_Node_P( snacContext, elementIndex, 7 )->velocity[1],
				Snac_Element_Node_P( snacContext, elementIndex, 7 )->velocity[2] );
		}
		else {
			printf( "Element %u: has same values as element 0.\n", elementIndex );
		}		
	}
#endif
	
	SnacTest_TearDown();

	return 0; /* success */
}

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
**	Tests StrainRate function for values of element volume.
**
** $Id: testUpdateNode-mass-4-4-4.c 1792 2004-07-30 05:42:39Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "mpi.h"
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "TestUtilities.h"

#include "stdio.h"
#include "stdlib.h"


extern MPI_Comm			CommWorld;
extern int				rank;
extern int				numProcessors;
extern int				procToWatch;
extern Dictionary*			dictionary;
extern Snac_Context*			snacContext;

int main( int argc, char* argv[] ) {
	Node_LocalIndex		nodeIndex;
	double				speedOfSound;
	Mass				mass000, mass100, mass110, mass111;
	Mass				inertialMass000, inertialMass100, inertialMass110, inertialMass111;
	Force				force000, force100, force110, force111;
	Force				balance000, balance100, balance110, balance111;
	
	SnacTest_SetUp( argc, argv );

	SnacTest_PrintElementZeroInfo();
	
	SnacTest_UpdateElementsNodes( &speedOfSound );
	
	/* obtain the first corner node's info */
	nodeIndex = RegularMeshUtils_Node_Local3DTo1D( (HexaMD*)snacContext->mesh->layout->decomp, 0, 0, 0 );
	Snac_Force( (Context*)snacContext, nodeIndex, speedOfSound, &mass000, &inertialMass000, &force000, &balance000 );
	Snac_UpdateNodeMomentum( (Context*)snacContext, nodeIndex, inertialMass000, force000 );
	printf( "Corner   Node %3u (0,0,0): mass: %g\n", nodeIndex, mass000 );
	
	/* obtain the first edge node's info */
	nodeIndex = RegularMeshUtils_Node_Local3DTo1D( (HexaMD*)snacContext->mesh->layout->decomp, 1, 0, 0 );
	Snac_Force( (Context*)snacContext, nodeIndex, speedOfSound, &mass100, &inertialMass100, &force100, &balance100 );
	Snac_UpdateNodeMomentum( (Context*)snacContext, nodeIndex, inertialMass100, force100 );
	printf( "Edge     Node %3u (1,0,0): mass: %g\n", nodeIndex, mass100 );
	
	/* obtain the first suface node's info */
	nodeIndex = RegularMeshUtils_Node_Local3DTo1D( (HexaMD*)snacContext->mesh->layout->decomp, 1, 1, 0 );
	Snac_Force( (Context*)snacContext, nodeIndex, speedOfSound, &mass110, &inertialMass110, &force110, &balance110 );
	Snac_UpdateNodeMomentum( (Context*)snacContext, nodeIndex, inertialMass110, force110 );
	printf( "Surface  Node %3u (1,1,0): mass: %g\n", nodeIndex, mass110 );
	
	/* obtain the first internal node's info */
	nodeIndex = RegularMeshUtils_Node_Local3DTo1D( (HexaMD*)snacContext->mesh->layout->decomp, 1, 1, 1 );
	Snac_Force( (Context*)snacContext, nodeIndex, speedOfSound, &mass111, &inertialMass111, &force111, &balance111 );
	Snac_UpdateNodeMomentum( (Context*)snacContext, nodeIndex, inertialMass111, force111 );
	printf( "Internal Node %3u (1,1,1): mass: %g\n", nodeIndex, mass111 );
	
	
	/* For each node, compare to the appropriate node */
	for( nodeIndex = 1; nodeIndex < snacContext->mesh->nodeLocalCount; nodeIndex++ ) {
		IJK		ijk;
		const IJK	s = { 	((HexaMD*)snacContext->mesh->layout->decomp)->nodeLocal3DCounts[rank][0] - 1,
					((HexaMD*)snacContext->mesh->layout->decomp)->nodeLocal3DCounts[rank][1] - 1,
					((HexaMD*)snacContext->mesh->layout->decomp)->nodeLocal3DCounts[rank][2] - 1 };
		
		RegularMeshUtils_Node_Local1DTo3D( (HexaMD*)snacContext->mesh->layout->decomp, nodeIndex, &ijk[0], &ijk[1], 
			&ijk[2] );
		if( (ijk[0] == 0 || ijk[0] == s[0]) && (ijk[1] == 0 || ijk[1] == s[1]) && (ijk[2] == 0 || ijk[2] == s[2]) ) {
			Bool		error;
			Mass		mass;
			Mass		inertialMass;
			Force		force;
			Force		balance;
			
			error = False;		
			Snac_Force( (Context*)snacContext, nodeIndex, speedOfSound, &mass, &inertialMass, &force, &balance );
			Snac_UpdateNodeMomentum( (Context*)snacContext, nodeIndex, inertialMass, force );
			if( mass != mass000 ) {
				printf( "Corner   Node %3u: mass: %g\n", nodeIndex, mass );
				error = True;
			}
			
			if( error ) {
				printf( "Corner   Node %3u: Gives a differing answer from node {0, 0, 0}.\n", nodeIndex );
			}
			else {
				printf( "Corner   Node %3u: has same values as node {0, 0, 0}.\n", nodeIndex );
			}
		}
		else if( ((ijk[0] == 0 || ijk[0] == s[0]) && (ijk[1] == 0 || ijk[1] == s[1])) ||
			((ijk[0] == 0 || ijk[0] == s[0]) && (ijk[2] == 0 || ijk[2] == s[2])) ||
			((ijk[1] == 0 || ijk[1] == s[1]) && (ijk[2] == 0 || ijk[2] == s[2])) )
		{
			Bool 		error;
			Mass		mass;
			Mass		inertialMass;
			Force		force;
			Force		balance;
			
			error = False;		
			Snac_Force( (Context*)snacContext, nodeIndex, speedOfSound, &mass, &inertialMass, &force, &balance );
			Snac_UpdateNodeMomentum( (Context*)snacContext, nodeIndex, inertialMass, force );
			if( mass != mass100 ) {
				printf( "Edge     Node %3u: mass: %g\n", nodeIndex, mass );
				error = True;
			}
			
			if( error ) {
				printf( "Edge     Node %3u: Gives a differing answer from node {1, 0, 0}.\n", nodeIndex );
			}
			else {
				printf( "Edge     Node %3u: has same values as node {1, 0, 0}.\n", nodeIndex );
			}
		}
		else if( (ijk[0] == 0 || ijk[0] == s[0]) || (ijk[1] == 0 || ijk[1] == s[1]) || (ijk[2] == 0 || ijk[2] == s[2]) ) {
			Bool 		error;
			Mass		mass;
			Mass		inertialMass;
			Force		force;
			Force		balance;
			
			error = False;		
			Snac_Force( (Context*)snacContext, nodeIndex, speedOfSound, &mass, &inertialMass, &force, &balance );
			Snac_UpdateNodeMomentum( (Context*)snacContext, nodeIndex, inertialMass, force );
			if( mass != mass110 ) {
				printf( "Surface  Node %3u: mass: %g\n", nodeIndex, mass );
				error = True;
			}
			
			if( error ) {
				printf( "Surface  Node %3u: Gives a differing answer from node {1, 1, 0}.\n", nodeIndex );
			}
			else {
				printf( "Surface  Node %3u: has same values as node {1, 1, 0}.\n", nodeIndex );
			}
		}
		else {
			Bool 		error;
			Mass		mass;
			Mass		inertialMass;
			Force		force;
			Force		balance;
			
			error = False;		
			Snac_Force( (Context*)snacContext, nodeIndex, speedOfSound, &mass, &inertialMass, &force, &balance );
			Snac_UpdateNodeMomentum( (Context*)snacContext, nodeIndex, inertialMass, force );
			if( mass != mass111 ) {
				printf( "Internal Node %3u: mass: %g\n", nodeIndex, mass );
				error = True;
			}
			
			if( error ) {
				printf( "Internal Node %3u: Gives a differing answer from node {1, 1, 1}.\n", nodeIndex );
			}
			else {
				printf( "Internal Node %3u: has same values as node {1, 1, 1}.\n", nodeIndex );
			}
		}
	}
	
	SnacTest_TearDown();
	
	return 0; /* success */
}

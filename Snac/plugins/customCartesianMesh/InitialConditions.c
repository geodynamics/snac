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
** $Id: InitialConditions.c 1431 2004-05-18 07:19:21Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "InitialConditions.h"
#include "Register.h"

#include <string.h>
#include <math.h>
#ifndef PI
	#ifndef M_PIl
		#ifndef M_PI
			#define PI 3.1415927
		#else
			#define PI M_PI
		#endif
	#else
		#define PI M_PIl
	#endif
#endif

void _SnacCustomCartesian_InitialConditions( void* _context, void* data ) {
	Snac_Context*			context = (Snac_Context*)_context;
	Mesh*				mesh = context->mesh;
	MeshLayout*			layout = (MeshLayout*)mesh->layout;
	HexaMD*				decomp = (HexaMD*)layout->decomp;
	BlockGeometry*			geometry = (BlockGeometry*)layout->elementLayout->geometry;
	Node_GlobalIndex		node_gI;
	double                          new_x[decomp->nodeGlobal3DCounts[0]];
	const double                    reg_dx = (geometry->max[0]-geometry->min[0])/(decomp->nodeGlobal3DCounts[0] - 1);
	const double                    xm = geometry->min[0] + 0.3*(geometry->max[0]-geometry->min[0]);
	const double                    L = (geometry->max[0]-geometry->min[0])/20.0;

	#ifdef DEBUG
		printf( "In: %s\n", __func__ );
	#endif

	for( node_gI = 0; node_gI < decomp->nodeGlobal3DCounts[0]; node_gI++ ) {
		double                          reg_x;

		if(node_gI==0)
			new_x[node_gI] = geometry->min[0];
		else if(node_gI>0) {
			reg_x = geometry->min[0] + node_gI*reg_dx;
			new_x[node_gI] = new_x[node_gI-1] + reg_dx/(5.0/PI*atan((reg_x-xm)/L)+3.0);
		}
	}
	for( node_gI = 0; node_gI < context->mesh->nodeGlobalCount; node_gI++ ) {
		Node_LocalIndex			node_lI = _MeshDecomp_Node_GlobalToLocal1D( decomp, node_gI );
		Index				i_gI;
		Index				j_gI;
		Index				k_gI;
		Coord*				coord = 0;
		Coord				tmpCoord;

		/* If a local node, directly change the node coordinates and initial tpr, else use a temporary location */
		if( node_lI < context->mesh->nodeLocalCount ) { /* a local node */
			coord = Snac_NodeCoord_P( context, node_lI );
		}
		else {
			coord = &tmpCoord;
		}

		RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &i_gI, &j_gI, &k_gI );

		(*coord)[0] = new_x[i_gI];
		if(i_gI>0)
			(*coord)[0] *= geometry->max[0]/new_x[(decomp->nodeGlobal3DCounts[0] - 1)];
		//fprintf(stderr,"i_gI=%d coord=%e newX=%e min=%e max=%e\n",
		//	i_gI,(*coord)[0],new_x[i_gI],geometry->min[0],geometry->max[0]);

		#ifdef DEBUG
			printf( "\tnode_lI: %2u, node_gI: %2u, i: %2u, j: %2u, k: %2u, ",
				node_lI,
				node_gI,
				i_gI,
				j_gI,
				k_gI );
			printf( "x: %12g, y: %12g, z: %12g\n", (*coord)[0], (*coord)[1], (*coord)[2] );
		#endif
	}
}

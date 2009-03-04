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

void _SnacCylinderQuad_InitialConditions( void* _context, void* data ) {
	Snac_Context*			context = (Snac_Context*)_context;
	Mesh*				mesh = context->mesh;
	MeshLayout*			layout = (MeshLayout*)mesh->layout;
	HexaMD*				decomp = (HexaMD*)layout->decomp;
	BlockGeometry*			geometry = (BlockGeometry*)layout->elementLayout->geometry;
	Node_GlobalIndex		node_gI;
	double				ri,ro,ztop,zbot;
	double				alpha,d;
#ifdef DEBUG
	printf( "In: %s\n", __func__ );
#endif

	ri = Dictionary_Entry_Value_AsDouble(Dictionary_GetDefault( context->dictionary, "cylinder_ri", Dictionary_Entry_Value_FromDouble( 3.0f ) ) );
	ro = Dictionary_Entry_Value_AsDouble(Dictionary_GetDefault( context->dictionary, "cylinder_ro", Dictionary_Entry_Value_FromDouble( 10.0f ) ) );
	zbot = Dictionary_Entry_Value_AsDouble(Dictionary_GetDefault( context->dictionary, "cylinder_zbot", Dictionary_Entry_Value_FromDouble( 0.0f ) ) );
	ztop = Dictionary_Entry_Value_AsDouble(Dictionary_GetDefault( context->dictionary, "cylinder_ztop", Dictionary_Entry_Value_FromDouble( 1.0f ) ) );
	alpha = Dictionary_Entry_Value_AsDouble(Dictionary_GetDefault( context->dictionary, "cylinder_alpha", Dictionary_Entry_Value_FromDouble( 1.1f ) ) );

	d = (ro - ri)*(alpha-1.0)/(pow(alpha,decomp->nodeGlobal3DCounts[2]-1)-1.0);
	/* apply the cylindrical initial mesh */
	for( node_gI = 0; node_gI < context->mesh->nodeGlobalCount; node_gI++ ) {
		Node_LocalIndex			node_lI = _MeshDecomp_Node_GlobalToLocal1D( decomp, node_gI );
		Index				i_gI;
		Index				j_gI;
		Index				k_gI;
		Coord*				coord = 0;
		Coord				initialRTZ,tmpCoord;
		double				rsum = 0.0f;

		/* If a local node, directly change the node coordinates and initial tpr, else use a temporary location */
		if( node_lI < context->mesh->nodeLocalCount ) { /* a local node */
			coord = Snac_NodeCoord_P( context, node_lI );
		}
		else { /* Not a local node */
			coord = &tmpCoord;
		}

		RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &i_gI, &j_gI, &k_gI );

		/*               -----                   */
		/*         right |    \ front            */
		/*     z ^       --\   \                 */
		/*       |     back  |_ |                */
		/*       -->x       left                 */
		initialRTZ[0] = ri; // radius
		if(k_gI>0) rsum = (pow(alpha,k_gI)-1.0)/(alpha-1.0)*d;
		initialRTZ[0] += rsum;
		initialRTZ[1] = (90.0f/(decomp->nodeGlobal3DCounts[0]-1)*i_gI)*PI/180.0f; // theta
		initialRTZ[2] = ztop - (ztop-zbot)/(decomp->nodeGlobal3DCounts[1]-1)*j_gI; // Z

		(*coord)[0] = initialRTZ[0]*cos(initialRTZ[1]);
		(*coord)[1] = initialRTZ[2];
		(*coord)[2] = initialRTZ[0]*sin(initialRTZ[1]);

		if( node_gI == 0 ) {
			geometry->min[0] = geometry->max[0] = (*coord)[0];
			geometry->min[1] = geometry->max[1] = (*coord)[1];
			geometry->min[2] = geometry->max[2] = (*coord)[2];
		}
		else {
			if( geometry->min[0] > (*coord)[0] ) geometry->min[0] = (*coord)[0];
			if( geometry->min[1] > (*coord)[1] ) geometry->min[1] = (*coord)[1];
			if( geometry->min[2] > (*coord)[2] ) geometry->min[2] = (*coord)[2];
			if( geometry->max[0] < (*coord)[0] ) geometry->max[0] = (*coord)[0];
			if( geometry->max[1] < (*coord)[1] ) geometry->max[1] = (*coord)[1];
			if( geometry->max[2] < (*coord)[2] ) geometry->max[2] = (*coord)[2];
		}

		#ifdef DEBUG
			printf( "\tnode_lI: %2u, node_gI: %2u, i: %2u, j: %2u, k: %2u, r: %8g, theta: %8g, z: %8g,",
				node_lI,
				node_gI,
				i_gI,
				j_gI,
				k_gI,
				initialRTZ[0],
				initialRTZ[1],
				initialRTZ[2] );
			printf( "x: %12g, y: %12g, z: %12g\n", (*coord)[0], (*coord)[1], (*coord)[2] );
		#endif
	}
}

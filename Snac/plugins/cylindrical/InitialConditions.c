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

void _SnacCylindrical_InitialConditions( void* _context, void* data ) {
	Snac_Context*			context = (Snac_Context*)_context;
	Mesh*				mesh = context->mesh;
	MeshLayout*			layout = (MeshLayout*)mesh->layout;
	HexaMD*				decomp = (HexaMD*)layout->decomp;
	BlockGeometry*			geometry = (BlockGeometry*)layout->elementLayout->geometry;
	Node_GlobalIndex		node_gI, II;
	int				num_circ;
	double				xDelta;
	double				yDelta;
	double				zDelta;
	double				*r,*a;
	double				local_r;
	double				z;
	#ifdef DEBUG
		printf( "In: %s\n", __func__ );
	#endif

	num_circ = decomp->nodeGlobal3DCounts[0]/2;
/* 	fprintf(stderr,"num_circ=%d\n",num_circ); */
	r = (double *)malloc((num_circ+1)*sizeof(double));
	a = (double *)malloc((num_circ+1)*sizeof(double));
	r[0] = Dictionary_Entry_Value_AsDouble(Dictionary_GetDefault( context->dictionary, "cylinder_r", Dictionary_Entry_Value_FromDouble( 1.0f ) ) );
	z = Dictionary_Entry_Value_AsDouble(Dictionary_GetDefault( context->dictionary, "cylinder_z", Dictionary_Entry_Value_FromDouble( 2.0f ) ) );
	a[0] = sqrt(PI)/2.0f*r[0];

	for( II = 1; II<num_circ; II++ ) {
		r[II] = r[0] / (num_circ) * (num_circ - II);
		a[II] = sqrt(PI)/2.0f*r[II];
	}
	r[num_circ] = 0.0;
	a[num_circ] = 0.0;
	xDelta = 2.0f * a[0] / (decomp->nodeGlobal3DCounts[0] - 1);
	yDelta = z / (decomp->nodeGlobal3DCounts[1] - 1);
	zDelta = 2.0f * a[0] / (decomp->nodeGlobal3DCounts[2] - 1);
/* 	fprintf(stderr,"dX=%e dY=%e dZ=%e\n",xDelta,yDelta,zDelta); */
	/* apply the cylindrical initial mesh */
	for( node_gI = 0; node_gI < context->mesh->nodeGlobalCount; node_gI++ ) {
		Node_LocalIndex			node_lI = _MeshDecomp_Node_GlobalToLocal1D( decomp, node_gI );
		Index				i_gI;
		Index				j_gI;
		Index				k_gI;
		Index				local_Xindex,local_Zindex,local_index;
		Coord				XYZ;
		Coord*				coord = 0;
		Coord*				initialTPR = 0;
		Coord				tmpCoord;
		Coord				tmpInitialTPR;

		/* If a local node, directly change the node coordinates and initial tpr, else use a temporary location */
		if( node_lI < context->mesh->nodeLocalCount ) { /* a local node */
			coord = Snac_NodeCoord_P( context, node_lI );
			initialTPR = &tmpInitialTPR;
		}
		else { /* Not a local node */
			coord = &tmpCoord;
			initialTPR = &tmpInitialTPR;
		}

		RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &i_gI, &j_gI, &k_gI );

		if(fmod(decomp->nodeGlobal3DCounts[0],2)==0.0) {
			local_Xindex = (num_circ-1)-(int)(floor(fabs(i_gI-(decomp->nodeGlobal3DCounts[0]-1)/2.0)));
/* 			fprintf(stderr,"local_Xindex=%d num_circ=%d i_gI=%d %f fabs=%f abs_floor=%d\n",local_Xindex,num_circ,i_gI,(decomp->nodeGlobal3DCounts[0]-1)/2.0,fabs(i_gI-(decomp->nodeGlobal3DCounts[0]-1)/2.0),(int)(floor(fabs(i_gI-(decomp->nodeGlobal3DCounts[0]-1)/2.0)))); */
			local_Zindex = (num_circ-1)-floor(abs(k_gI-(decomp->nodeGlobal3DCounts[2]-1)/2.0));
			local_index = (local_Xindex > local_Zindex) ? local_Zindex : local_Xindex;
		}
		else {
			local_Xindex = num_circ-(int)(floor(fabs(i_gI-(decomp->nodeGlobal3DCounts[0]-1)/2.0)));
/* 			fprintf(stderr,"local_Xindex=%d num_circ=%d i_gI=%d %f fabs=%f abs_floor=%d\n",local_Xindex,num_circ,i_gI,(decomp->nodeGlobal3DCounts[0]-1)/2.0,fabs(i_gI-(decomp->nodeGlobal3DCounts[0]-1)/2.0),(int)(floor(fabs(i_gI-(decomp->nodeGlobal3DCounts[0]-1)/2.0)))); */
			local_Zindex = num_circ-floor(abs(k_gI-(decomp->nodeGlobal3DCounts[2]-1)/2.0));
			local_index = (local_Xindex > local_Zindex) ? local_Zindex : local_Xindex;
		}

		XYZ[0] = -1.0f * a[0] + i_gI * xDelta;
		XYZ[1] = j_gI * yDelta;
		XYZ[2] = -1.0f * a[0] + k_gI * zDelta;

		local_r = sqrt( XYZ[0]*XYZ[0] + XYZ[2]*XYZ[2] );
		if(local_r>0.0) {
			(*coord)[0] = r[local_index]/local_r * XYZ[0];
			(*coord)[1] = XYZ[1];
			(*coord)[2] = r[local_index]/local_r * XYZ[2];
		}
		else {
			(*coord)[0] = 0.0;
			(*coord)[1] = XYZ[1];
			(*coord)[2] = 0.0;
		}
/* 		fprintf(stderr,"ijk=%d %d %d  XYZ=%e %e %e TPR=%e %e %e\n", */
/* 			local_Xindex,local_Zindex,local_index,XYZ[0],XYZ[1],XYZ[2], */
/* 			(*coord)[0],(*coord)[1],(*coord)[2]); */
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
			printf( "\tnode_lI: %2u, node_gI: %2u, i: %2u, j: %2u, k: %2u, theta: %8g, phi: %8g, r: %8g,",
				node_lI,
				node_gI,
				i_gI,
				j_gI,
				k_gI,
				(*initialTPR)[0],
				(*initialTPR)[1],
				(*initialTPR)[2] );
			printf( "x: %12g, y: %12g, z: %12g\n", (*coord)[0], (*coord)[1], (*coord)[2] );
		#endif
	}
 	free( (void *)r );
 	free( (void *)a );
}

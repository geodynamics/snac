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
** $Id: InitialConditions.c 2498 2005-01-07 03:57:00Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "Mesh.h"
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

void _SnacSpherical_InitialConditions( void* _context, void* data ) {
	Snac_Context*			context = (Snac_Context*)_context;
	Mesh*				mesh = context->mesh;
	SnacSpherical_Mesh*		meshExt = ExtensionManager_Get(
						context->meshExtensionMgr,
						mesh,
						SnacSpherical_MeshHandle );
	MeshLayout*			layout = (MeshLayout*)mesh->layout;
	HexaMD*				decomp = (HexaMD*)layout->decomp;
	BlockGeometry*			geometry = (BlockGeometry*)layout->elementLayout->geometry;
	Node_GlobalIndex		node_gI;
	double				thetaDelta;
	double				phiDelta;
	double				rDelta;

	#ifdef DEBUG
		printf( "In: %s\n", __func__ );
	#endif

	thetaDelta = (meshExt->max[0] - meshExt->min[0]) / (decomp->nodeGlobal3DCounts[2] - 1);
	phiDelta = (meshExt->max[1] - meshExt->min[1]) / (decomp->nodeGlobal3DCounts[0] - 1);
	rDelta = (meshExt->max[2] - meshExt->min[2]) / (decomp->nodeGlobal3DCounts[1] - 1);

	/* apply the spherical initial mesh */
	for( node_gI = 0; node_gI < context->mesh->nodeGlobalCount; node_gI++ ) {
		Node_LocalIndex			node_lI = _MeshDecomp_Node_GlobalToLocal1D( decomp, node_gI );
		Index				i_gI;
		Index				j_gI;
		Index				k_gI;
		Coord				citcomTPR;
		Coord				citcomXYZ;
		Coord*				coord = 0;
		Coord*				initialTPR = 0;
		Coord				tmpCoord;
		Coord				tmpInitialTPR;

		/* If a local node, directly change the node coordinates and initial tpr, else use a temporary location */
		if( node_lI < context->mesh->nodeLocalCount ) { /* A local node */
			coord = Snac_NodeCoord_P( context, node_lI );
			initialTPR = &tmpInitialTPR;
		}
		else { /* Not a local node */
			coord = &tmpCoord;
			initialTPR = &tmpInitialTPR;
		}

		RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &i_gI, &j_gI, &k_gI );

		citcomTPR[0] = PI / 180.0f * ( (90.0f - meshExt->max[0]) + thetaDelta * k_gI );
		citcomTPR[1] = PI / 180.0f * ( meshExt->min[1] + phiDelta * i_gI );
		citcomTPR[2] = meshExt->min[2] + rDelta * j_gI;

		citcomXYZ[0] = citcomTPR[2] * sin( citcomTPR[0] ) * cos( citcomTPR[1] );
		citcomXYZ[1] = citcomTPR[2] * sin( citcomTPR[0] ) * sin( citcomTPR[1] );
		citcomXYZ[2] = citcomTPR[2] * cos( citcomTPR[0] );

		(*coord)[0] = citcomXYZ[0];
		(*coord)[1] = citcomXYZ[1];
		(*coord)[2] = citcomXYZ[2];
		(*initialTPR)[0] = citcomTPR[0];
		(*initialTPR)[1] = citcomTPR[1];
		(*initialTPR)[2] = citcomTPR[2];

		if( node_gI == 0 ) {
			geometry->min[0] = geometry->max[0] = (*coord)[1];
			geometry->min[1] = geometry->max[1] = (*coord)[2];
			geometry->min[2] = geometry->max[2] = (*coord)[0];
		}
		else {
			if( geometry->min[0] > (*coord)[0] ) geometry->min[0] = (*coord)[0];
			if( geometry->min[1] > (*coord)[1] ) geometry->min[1] = (*coord)[1];
			if( geometry->min[2] > (*coord)[2] ) geometry->min[2] = (*coord)[2];
			if( geometry->max[0] < (*coord)[0] ) geometry->max[0] = (*coord)[0];
			if( geometry->max[1] < (*coord)[1] ) geometry->max[1] = (*coord)[1];
			if( geometry->max[2] < (*coord)[2] ) geometry->max[2] = (*coord)[2];
		}
#if 0
		if( node_gI == 0 ) {
			geometry->min[0] = geometry->max[0] = (*initialTPR)[1];
			geometry->min[1] = geometry->max[1] = (*initialTPR)[2];
			geometry->min[2] = geometry->max[2] = (*initialTPR)[0];
		}
		else {
			if( geometry->min[0] > (*initialTPR)[1] ) geometry->min[0] = (*initialTPR)[1];
			if( geometry->min[1] > (*initialTPR)[2] ) geometry->min[1] = (*initialTPR)[2];
			if( geometry->min[2] > (*initialTPR)[0] ) geometry->min[2] = (*initialTPR)[0];
			if( geometry->max[0] < (*initialTPR)[1] ) geometry->max[0] = (*initialTPR)[1];
			if( geometry->max[1] < (*initialTPR)[2] ) geometry->max[1] = (*initialTPR)[2];
			if( geometry->max[2] < (*initialTPR)[0] ) geometry->max[2] = (*initialTPR)[0];
		}
#endif

#if 0
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
#endif
	}
}

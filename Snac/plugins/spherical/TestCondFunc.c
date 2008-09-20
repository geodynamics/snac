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
** $Id: TestCondFunc.c 1524 2004-06-07 18:21:50Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "Mesh.h"
#include "Register.h"
#include <stdio.h>
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
#define myr2sec (1.0e6*(365.0*24.0*3600))
#define deg2rad (PI/180.0)
#define rad2deg (180.0/PI)
#define omegaL (-0.75f*deg2rad/myr2sec)
#define omegaR (0.75f*deg2rad/myr2sec)

void _SnacSpherical_applyPlateVx( Index node_dI, Variable_Index var_I, void* _context, void* result ) {
	Snac_Context*			context = (Snac_Context*)_context;
	Mesh*				mesh = context->mesh;
	SnacSpherical_Mesh*		meshExt = ExtensionManager_Get(
						context->meshExtensionMgr,
						mesh,
						SnacSpherical_MeshHandle );
	MeshLayout*			layout = (MeshLayout*)mesh->layout;
	HexaMD*				decomp = (HexaMD*)layout->decomp;

	double*				velComponent = (double*)result;
	const Node_GlobalIndex		midI = (decomp->nodeGlobal3DCounts[0] + 1) / 2;
	IJK				ijk;
	Node_GlobalIndex		node_gI = _MeshDecomp_Node_LocalToGlobal1D( decomp, node_dI );
	Coord*                          coord = Snac_NodeCoord_P( context, node_dI );
	double                          lambda = deg2rad * (meshExt->min[1]+meshExt->max[1])*0.5f;
	double                          phi = deg2rad * 90.0f;
	double					omega;
	double					E[3],VX[3];

	E[0] = sin(PI/2.0f-phi)*cos(lambda);
	E[1] = sin(PI/2.0f-phi)*sin(lambda);
	E[2] = cos(PI/2.0f-phi);
	RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );

 	if(ijk[0] < (midI-1))
		omega = omegaL;
 	else
		omega = omegaR;

	VX[0] = omega*(E[1]*(*coord)[2] - E[2]*(*coord)[1]);
	VX[1] = omega*(-1.0f * E[0]*(*coord)[2] + E[2]*(*coord)[0]);
	VX[2] = omega*(E[0]*(*coord)[1] - E[1]*(*coord)[0]);

	(*velComponent) = VX[0];
}

void _SnacSpherical_applyPlateVy( Index node_dI, Variable_Index var_I, void* _context, void* result ) {
	Snac_Context*			context = (Snac_Context*)_context;
	Mesh*				mesh = context->mesh;
	SnacSpherical_Mesh*		meshExt = ExtensionManager_Get(
						context->meshExtensionMgr,
						mesh,
						SnacSpherical_MeshHandle );
	MeshLayout*			layout = (MeshLayout*)mesh->layout;
	HexaMD*				decomp = (HexaMD*)layout->decomp;

	double*				velComponent = (double*)result;
	const Node_GlobalIndex		midI = (decomp->nodeGlobal3DCounts[0] + 1) / 2;
	IJK				ijk;
	Node_GlobalIndex		node_gI = _MeshDecomp_Node_LocalToGlobal1D( decomp, node_dI );
	Coord*                          coord = Snac_NodeCoord_P( context, node_dI );
	double                          lambda = deg2rad * (meshExt->min[1]+meshExt->max[1])*0.5f;
	double                          phi = deg2rad * 90.0f;
	double					omega;
	double					E[3],VX[3];

	E[0] = sin(PI/2.0f-phi)*cos(lambda);
	E[1] = sin(PI/2.0f-phi)*sin(lambda);
	E[2] = cos(PI/2.0f-phi);
	RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );

 	if(ijk[0] < (midI-1))
		omega = omegaL;
 	else
		omega = omegaR;

	VX[0] = omega*(E[1]*(*coord)[2] - E[2]*(*coord)[1]);
	VX[1] = omega*(-1.0f * E[0]*(*coord)[2] + E[2]*(*coord)[0]);
	VX[2] = omega*(E[0]*(*coord)[1] - E[1]*(*coord)[0]);

	(*velComponent) = VX[1];
}

void _SnacSpherical_applyPlateVz( Index node_dI, Variable_Index var_I, void* _context, void* result ) {
	Snac_Context*			context = (Snac_Context*)_context;
	Mesh*				mesh = context->mesh;
	SnacSpherical_Mesh*		meshExt = ExtensionManager_Get(
						context->meshExtensionMgr,
						mesh,
						SnacSpherical_MeshHandle );
	MeshLayout*			layout = (MeshLayout*)mesh->layout;
	HexaMD*				decomp = (HexaMD*)layout->decomp;

	double*				velComponent = (double*)result;
	const Node_GlobalIndex		midI = (decomp->nodeGlobal3DCounts[0] + 1) / 2;
	IJK				ijk;
	Node_GlobalIndex		node_gI = _MeshDecomp_Node_LocalToGlobal1D( decomp, node_dI );
	Coord*                          coord = Snac_NodeCoord_P( context, node_dI );
	double                          lambda = deg2rad * (meshExt->min[1]+meshExt->max[1])*0.5f;
	double                          phi = deg2rad * 90.0f;
	double					omega;
	double					E[3],VX[3];

	E[0] = sin(PI/2.0f-phi)*cos(lambda);
	E[1] = sin(PI/2.0f-phi)*sin(lambda);
	E[2] = cos(PI/2.0f-phi);
	RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );

 	if(ijk[0] < (midI-1))
		omega = omegaL;
 	else
		omega = omegaR;

	VX[0] = omega*(E[1]*(*coord)[2] - E[2]*(*coord)[1]);
	VX[1] = omega*(-1.0f * E[0]*(*coord)[2] + E[2]*(*coord)[0]);
	VX[2] = omega*(E[0]*(*coord)[1] - E[1]*(*coord)[0]);

	(*velComponent) = VX[2];
}

void _SnacSpherical_applyInitialVx( Index node_dI, Variable_Index var_I, void* _context, void* result ) {
	Snac_Context*                   context = (Snac_Context*)_context;
	Mesh*                           mesh = context->mesh;
	SnacSpherical_Mesh*             meshExt = ExtensionManager_Get(
													context->meshExtensionMgr,
													mesh,
													SnacSpherical_MeshHandle );
	MeshLayout*                     layout = (MeshLayout*)mesh->layout;
	HexaMD*                         decomp = (HexaMD*)layout->decomp;

	double*                         velComponent = (double*)result;
	IJK                             ijk;
	Coord*                          coord = Snac_NodeCoord_P( context, node_dI );
	double                          r,lon,lat;
	double                          lambda = deg2rad * (meshExt->min[1]+meshExt->max[1])*0.5f;
	double                          phi = deg2rad * 90.0f;
	double                          E[3],VXL[3],VXR[3];
	Node_GlobalIndex                node_gI = _MeshDecomp_Node_LocalToGlobal1D( decomp, node_dI );
	RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );

	E[0] = sin(PI/2.0f-phi)*cos(lambda);
	E[1] = sin(PI/2.0f-phi)*sin(lambda);
	E[2] = cos(PI/2.0f-phi);

	r = sqrt((*coord)[0]*(*coord)[0] + (*coord)[1]*(*coord)[1] + (*coord)[2]*(*coord)[2]);
	lat = PI/2.0f - acos((*coord)[2] / r);

	lon = meshExt->min[1] * deg2rad; //85.0*deg2rad; //atan2((*coord)[1], (*coord)[0]);
	VXL[0] = omegaL*(E[1]*(*coord)[2] - E[2]*(*coord)[1]);
	VXL[1] = omegaL*(-1.0f * E[0]*(*coord)[2] + E[2]*(*coord)[0]);
	VXL[2] = omegaL*(E[0]*(*coord)[1] - E[1]*(*coord)[0]);

	lon = meshExt->max[1] * deg2rad; //85.0*deg2rad; //atan2((*coord)[1], (*coord)[0]);
	VXR[0] = omegaR*(E[1]*(*coord)[2] - E[2]*(*coord)[1]);
	VXR[1] = omegaR*(-1.0f * E[0]*(*coord)[2] + E[2]*(*coord)[0]);
	VXR[2] = omegaR*(E[0]*(*coord)[1] - E[1]*(*coord)[0]);

	(*velComponent) = VXL[0] + (VXR[0] - VXL[0])/(decomp->elementGlobal3DCounts[0])*ijk[0];
}

void _SnacSpherical_applyInitialVy( Index node_dI, Variable_Index var_I, void* _context, void* result ) {
	Snac_Context*                   context = (Snac_Context*)_context;
	Mesh*                           mesh = context->mesh;
	SnacSpherical_Mesh*             meshExt = ExtensionManager_Get(
													context->meshExtensionMgr,
													mesh,
													SnacSpherical_MeshHandle );
	MeshLayout*                     layout = (MeshLayout*)mesh->layout;
	HexaMD*                         decomp = (HexaMD*)layout->decomp;

	double*                         velComponent = (double*)result;
	IJK                             ijk;
	Coord*                          coord = Snac_NodeCoord_P( context, node_dI );
	double                          r,lon,lat;
	double                          lambda = deg2rad * (meshExt->min[1]+meshExt->max[1])*0.5f;
	double                          phi = deg2rad * 90.0f;
	double                          E[3],VXL[3],VXR[3];
	Node_GlobalIndex                node_gI = _MeshDecomp_Node_LocalToGlobal1D( decomp, node_dI );
	RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );

	E[0] = sin(PI/2.0f-phi)*cos(lambda);
	E[1] = sin(PI/2.0f-phi)*sin(lambda);
	E[2] = cos(PI/2.0f-phi);

	r = sqrt((*coord)[0]*(*coord)[0] + (*coord)[1]*(*coord)[1] + (*coord)[2]*(*coord)[2]);
	lat = PI/2.0f - acos((*coord)[2] / r);

	lon = meshExt->min[1] * deg2rad; //85.0*deg2rad; //atan2((*coord)[1], (*coord)[0]);
	VXL[0] = omegaL*(E[1]*(*coord)[2] - E[2]*(*coord)[1]);
	VXL[1] = omegaL*(-1.0f * E[0]*(*coord)[2] + E[2]*(*coord)[0]);
	VXL[2] = omegaL*(E[0]*(*coord)[1] - E[1]*(*coord)[0]);

	lon = meshExt->max[1] * deg2rad; //85.0*deg2rad; //atan2((*coord)[1], (*coord)[0]);
	VXR[0] = omegaR*(E[1]*(*coord)[2] - E[2]*(*coord)[1]);
	VXR[1] = omegaR*(-1.0f * E[0]*(*coord)[2] + E[2]*(*coord)[0]);
	VXR[2] = omegaR*(E[0]*(*coord)[1] - E[1]*(*coord)[0]);

	(*velComponent) = VXL[1] + (VXR[1] - VXL[1])/(decomp->elementGlobal3DCounts[0])*ijk[0];
}

void _SnacSpherical_applyInitialVz( Index node_dI, Variable_Index var_I, void* _context, void* result ) {
	Snac_Context*                   context = (Snac_Context*)_context;
	Mesh*                           mesh = context->mesh;
	SnacSpherical_Mesh*             meshExt = ExtensionManager_Get(
													context->meshExtensionMgr,
													mesh,
													SnacSpherical_MeshHandle );
	MeshLayout*                     layout = (MeshLayout*)mesh->layout;
	HexaMD*                         decomp = (HexaMD*)layout->decomp;

	double*                         velComponent = (double*)result;
	IJK                             ijk;
	Coord*                          coord = Snac_NodeCoord_P( context, node_dI );
	double                          r,lon,lat;
	double                          lambda = deg2rad * (meshExt->min[1]+meshExt->max[1])*0.5f;
	double                          phi = deg2rad * 90.0f;
	double                          E[3],VXL[3],VXR[3];
	Node_GlobalIndex                node_gI = _MeshDecomp_Node_LocalToGlobal1D( decomp, node_dI );
	RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );

	E[0] = sin(PI/2.0f-phi)*cos(lambda);
	E[1] = sin(PI/2.0f-phi)*sin(lambda);
	E[2] = cos(PI/2.0f-phi);

	r = sqrt((*coord)[0]*(*coord)[0] + (*coord)[1]*(*coord)[1] + (*coord)[2]*(*coord)[2]);
	lat = PI/2.0f - acos((*coord)[2] / r);

	lon = meshExt->min[1] * deg2rad; //85.0*deg2rad; //atan2((*coord)[1], (*coord)[0]);
	VXL[0] = omegaL*(E[1]*(*coord)[2] - E[2]*(*coord)[1]);
	VXL[1] = omegaL*(-1.0f * E[0]*(*coord)[2] + E[2]*(*coord)[0]);
	VXL[2] = omegaL*(E[0]*(*coord)[1] - E[1]*(*coord)[0]);

	lon = meshExt->max[1] * deg2rad; //85.0*deg2rad; //atan2((*coord)[1], (*coord)[0]);
	VXR[0] = omegaR*(E[1]*(*coord)[2] - E[2]*(*coord)[1]);
	VXR[1] = omegaR*(-1.0f * E[0]*(*coord)[2] + E[2]*(*coord)[0]);
	VXR[2] = omegaR*(E[0]*(*coord)[1] - E[1]*(*coord)[0]);

	(*velComponent) = VXL[2] + (VXR[2] - VXL[2])/(decomp->elementGlobal3DCounts[0])*ijk[0];
}

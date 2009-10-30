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
** $Id: TestCondFunc.c 1792 2004-07-30 05:42:39Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
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

static double Bell( double x ) {
	return exp( -((x) * (x)) * 10.0f );
}


void _SnacRemesher_TestCondFunc( Index node_dI, Variable_Index var_I, void* _context, void* result ) {
	Snac_Context*			context = (Snac_Context*)_context;
	MeshLayout*			meshLayout = (MeshLayout*)context->meshLayout;
	HexaMD*				decomp = (HexaMD*)meshLayout->decomp;

	double*				velComponent = (double*)result;
	IJK				ijk;
	double				x;
	Node_GlobalIndex		node_gI = _MeshDecomp_Node_LocalToGlobal1D( decomp, node_dI );
	
	RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );
	x = 1.0f / decomp->nodeGlobal3DCounts[0] * ijk[0] - 0.5;
	(*velComponent) = Bell( x ) * 5e-7;
}

void _SnacRemesher_XFunc( Index node_dI, Variable_Index var_I, void* _context, void* result ) {
	Snac_Context*			context = (Snac_Context*)_context;
	MeshLayout*			meshLayout = (MeshLayout*)context->meshLayout;
	HexaMD*				decomp = (HexaMD*)meshLayout->decomp;

	double*				velComponent = (double*)result;
	IJK				ijk;
	const Node_GlobalIndex		midI = (decomp->nodeGlobal3DCounts[0] + 1) / 2;
	Node_GlobalIndex		node_gI = _MeshDecomp_Node_LocalToGlobal1D( decomp, node_dI );
	
	RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );
	if( ijk[0] < (midI-1) ) {
		(*velComponent) += 1e-8;
	}
	else if( ijk[0] > (midI-1) ) {
		(*velComponent) -= 1e-7;
	}
	/* else: leave centre to be free */
}

void _SnacRemesher_YFunc( Index node_dI, Variable_Index var_I, void* _context, void* result ) {
	Snac_Context*			context = (Snac_Context*)_context;
	MeshLayout*			meshLayout = (MeshLayout*)context->meshLayout;
	HexaMD*				decomp = (HexaMD*)meshLayout->decomp;

	double*				velComponent = (double*)result;
	IJK				ijk;
	const Node_GlobalIndex		midI = (decomp->nodeGlobal3DCounts[0] + 1) / 2;
	Node_GlobalIndex		node_gI = _MeshDecomp_Node_LocalToGlobal1D( decomp, node_dI );
	
	RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );
	if( ijk[0] < (midI-1) ) {
		(*velComponent) += 5e-8;
	}
	else if( ijk[0] > (midI-1) ) {
		(*velComponent) -= 1e-8;
	}
	/* else: leave centre to be free */
}

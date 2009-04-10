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
** $Id: ConstructExtensions.c 2498 2005-01-07 03:57:00Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "Context.h"
#include "Register.h"
#include "ConstructExtensions.h"
#include <assert.h>
#include <limits.h>
#ifndef PATH_MAX
	#define PATH_MAX 1024
#endif

void _SnacVelocity_VariableCondition( Index node_dI, Variable_Index var_I, void* _context, void* result ){
	Snac_Context*			context = (Snac_Context*)_context;
	Mesh*				mesh = context->mesh;
	MeshLayout*			layout = (MeshLayout*)mesh->layout;
	HexaMD*				decomp = (HexaMD*)layout->decomp;
	
	double*				velComponent = (double*)result;
	IJK				ijk;
	Node_GlobalIndex		node_gI = _MeshDecomp_Node_LocalToGlobal1D( decomp, node_dI );
	Index              oneThird = (unsigned int)((11.0/30.0)*decomp->nodeGlobal3DCounts[0]);
	Index              twoThirds = (unsigned int)((19.0/30.0)*decomp->nodeGlobal3DCounts[0]);

	const double vmag = 1.4e-06;

	RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );
	
	if( ijk[0] < oneThird )
		(*velComponent) = -1.0*vmag;
	else if( ijk[0] >= oneThird && ijk[0] <= twoThirds )
		(*velComponent) =  -1.0*vmag + 2.0*vmag*((double)(ijk[0])-(double)oneThird)/((double)twoThirds-(double)oneThird);
	else
		(*velComponent) = vmag;
}

void _SnacPlastic_ConstructExtensions( void* _context, void* data ) {
	Snac_Context*				context = (Snac_Context*)_context;
	SnacPlastic_Context*			contextExt = ExtensionManager_Get(
							context->extensionMgr,
							context,
							SnacPlastic_ContextHandle );
	char					tmpBuf[PATH_MAX];

	#if DEBUG
		printf( "In %s()\n", __func__ );
	#endif

		ConditionFunction_Register_Add(
									   context->condFunc_Register,
									   ConditionFunction_New( _SnacVelocity_VariableCondition, "variableVelBC" ) );

	/* Prepare the dump and checkpoint file */
	sprintf( tmpBuf, "%s/plStrain.%u", context->outputPath, context->rank );
	if( (contextExt->plStrainOut = fopen( tmpBuf, "w+" )) == NULL ) {
		assert( contextExt->plStrainOut /* failed to open file for writing */ );
		abort();
	}
	sprintf( tmpBuf, "%s/plStrainCP.%u", context->outputPath, context->rank );
	if( (contextExt->plStrainCheckpoint = fopen( tmpBuf, "w+" )) == NULL ) {
		assert( contextExt->plStrainCheckpoint /* failed to open file for writing */ );
		abort();
	}
}

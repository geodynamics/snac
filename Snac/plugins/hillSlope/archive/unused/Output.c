/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003,
**	Steve Quenette, 110 Victoria Street, Melbourne, Victoria, 3053, Australia.
**	Californian Institute of Technology, 1200 East California Boulevard, Pasadena, California, 91125, USA.
**	University of Texas, 1 University Station, Austin, Texas, 78712, USA.
**
** Authors:
**           Colin Stark, Doherty Research Scientist, Lamont-Doherty Earth Observatory (cstark@ldeo.columbia.edu)
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
** $Id: Context.c $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "Output.h"
#include "Context.h"
#include "Node.h"
#include "Register.h"
#include <stdio.h>

void _SnacHillSlope_DumpShear( void* _context ) {
	Snac_Context*				context = (Snac_Context*) _context;
	Node_LocalIndex				node_lI;
	StressTensor*				stress = &element->tetra[tetra_I].stress;
	StrainTensor*				strain = &element->tetra[tetra_I].strain;
        Strain					plasticStrain = 0.0f;


			/* Compute elastic stress first */
			trace_strain = (*strain)[0][0] + (*strain)[1][1] + (*strain)[2][2];

			(*stress)[0][0] += (2.0f * material->mu) * (*strain)[0][0] + material->lambda * (trace_strain );
			(*stress)[1][1] += (2.0f * material->mu) * (*strain)[1][1] + material->lambda * (trace_strain );
			(*stress)[2][2] += (2.0f * material->mu) * (*strain)[2][2] + material->lambda * (trace_strain );
			(*stress)[0][1] += (2.0f * material->mu) * (*strain)[0][1];
			(*stress)[0][2] += (2.0f * material->mu) * (*strain)[0][2];
			(*stress)[1][2] += (2.0f * material->mu) * (*strain)[1][2];

			principal_stresses(stress,s,cn);



	if( context->timeStep == 0 || (context->timeStep - 1) % context->dumpEvery == 0 ) {
		#if DEBUG
			printf( "In %s()\n", __func__ );
		#endif

		for( node_lI = 0; node_lI < context->mesh->nodeLocalCount; node_lI++ ) {
			Snac_Node* 			node = Snac_Node_At( context, node_lI );
/* 			SnacHillSlope_Node*			nodeExt = ExtensionManager_Get( */
/* 											context->mesh->nodeExtensionMgr, */
/* 											node, */
/* 											SnacHillSlope_NodeHandle ); */
			float				shearStress = nodeExt->hillSlope;
			fwrite( &hillSlope, sizeof(float), 1, contextExt->hillSlopeOut );
		}
		fflush( contextExt->hillSlopeOut );
	}
}


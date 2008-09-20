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
**	Set up and Tear down code common to many tests.
**
** $Id: TestUtilities.c 3227 2006-06-16 23:42:14Z EunseoChoi $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "TestStress.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void SnacTest_Stress( void* _context, Element_LocalIndex element_lI ) {
	Snac_Context *snacContext = (Snac_Context *)_context;
	Tetrahedra_Index		tetra_I;
	Snac_Element*			element = Snac_Element_At( snacContext, element_lI );
	Stress				traceStress[Tetrahedra_Count];
	Stress				partialStress;
	Stress				sVolAvg;
	Stress				sOtherAvg;
	/* for adjusting density to changed pressure */
	Material_Index                  material_I = element->material_I;
	Snac_Material*          material = &snacContext->materialProperty[material_I];
	Density                 phsDensity = material->phsDensity; // node->density
	double          alpha = 0.0f;
	double          beta = 0.0f;
	Stress          pressure = 0.0f;
	double			elemVolume = ( Tetrahedra_Count > 5 )?(2.0*element->volume):element->volume;

	/* Calculate the strain for each tetrahedra. */
	for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
		/* Initialise the strain tensor for this tetrahedra to 0 */
		memset( element->tetra[tetra_I].strain, 0, sizeof(element->tetra[tetra_I].strain) );

		/* Calculate the strain from the strain rate for this tetrahedra   */
		element->tetra[tetra_I].strain[0][0] = element->tetra[tetra_I].strainRate[0][0] * snacContext->dt;
		element->tetra[tetra_I].strain[1][1] = element->tetra[tetra_I].strainRate[1][1] * snacContext->dt;
		element->tetra[tetra_I].strain[2][2] = element->tetra[tetra_I].strainRate[2][2] * snacContext->dt;
		element->tetra[tetra_I].strain[0][1] = element->tetra[tetra_I].strainRate[0][1] * snacContext->dt;
		element->tetra[tetra_I].strain[0][2] = element->tetra[tetra_I].strainRate[0][2] * snacContext->dt;
		element->tetra[tetra_I].strain[1][2] = element->tetra[tetra_I].strainRate[1][2] * snacContext->dt;
	}

	/* Calculate the stresses for each tetrahedra. */
	for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
		const double			a2 = material->lambda;
		const double			sh2 = 2.0 * material->mu;
		StressTensor*			stress = &element->tetra[tetra_I].stress;
		StrainTensor*			strain = &element->tetra[tetra_I].strain;

		double trace_strain = (*strain)[0][0] + (*strain)[1][1] + (*strain)[2][2];

		(*stress)[0][0] += sh2 * (*strain)[0][0] + a2 * (trace_strain );
		(*stress)[1][1] += sh2 * (*strain)[1][1] + a2 * (trace_strain );
		(*stress)[2][2] += sh2 * (*strain)[2][2] + a2 * (trace_strain );
		(*stress)[0][1] += sh2 * (*strain)[0][1];
		(*stress)[0][2] += sh2 * (*strain)[0][2];
		(*stress)[1][2] += sh2 * (*strain)[1][2];
	}

	partialStress = 0.0f;
	for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {

		/* Initialise the trace stress tensor for this tetrahedra to 0 */
		memset( &traceStress[tetra_I], 0, sizeof(Stress) );
		traceStress[tetra_I]=  (
			element->tetra[tetra_I].stress[0][0] +
			element->tetra[tetra_I].stress[1][1] +
			element->tetra[tetra_I].stress[2][2] ) / 3.0f;
		partialStress += traceStress[tetra_I] * element->tetra[tetra_I].volume;
	}

	sVolAvg = 0.0f;
	sOtherAvg = 0.0f;
	for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
		element->tetra[tetra_I].stress[0][0] += -1.0f * traceStress[tetra_I] + partialStress / elemVolume;
		element->tetra[tetra_I].stress[1][1] += -1.0f * traceStress[tetra_I] + partialStress / elemVolume;
		element->tetra[tetra_I].stress[2][2] += -1.0f * traceStress[tetra_I] + partialStress / elemVolume;

		sVolAvg +=
			element->tetra[tetra_I].stress[1][1] * element->tetra[tetra_I].stress[2][2] +
			element->tetra[tetra_I].stress[2][2] * element->tetra[tetra_I].stress[0][0] +
			element->tetra[tetra_I].stress[0][0] * element->tetra[tetra_I].stress[1][1];
		sOtherAvg +=
			element->tetra[tetra_I].stress[0][1] * element->tetra[tetra_I].stress[0][1] +
			element->tetra[tetra_I].stress[1][2] * element->tetra[tetra_I].stress[1][2] +
			element->tetra[tetra_I].stress[0][2] * element->tetra[tetra_I].stress[0][2];

		pressure = -1.0f * (element->tetra[tetra_I].stress[0][0]+element->tetra[tetra_I].stress[1][1]+element->tetra[tetra_I].stress[2][2])/3.0f;
	}
	sVolAvg /= Tetrahedra_Count;
	sOtherAvg /= Tetrahedra_Count;

	/* Calculate the element stress from the tetrahedra stress tensors */
	element->stress = 0.5f * sqrt( 0.5f * fabs( -1.0f * sVolAvg + sOtherAvg ) );

	/* update density with updated pressure */
	for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ )
		element->tetra[tetra_I].density = phsDensity * (1.0 - alpha * (element->tetra[tetra_I].avgTemp-material->reftemp) + beta * pressure);

}

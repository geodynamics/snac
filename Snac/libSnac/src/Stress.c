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
** $Id: Stress.c 3274 2007-03-27 20:25:29Z EunseoChoi $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>

#include "units.h"
#include "types.h"
#include "shortcuts.h"
#include "Material.h"
#include "Node.h"
#include "Tetrahedra.h"
#include "TetrahedraTables.h"
#include "Element.h"
#include "EntryPoint.h"
#include "Stress.h"
#include "Context.h"
#include <math.h>
#include <string.h>

#define Snac_Constitutive_CallEP( self, element_lI ) \
	(KeyCall( self, self->constitutiveK, Snac_Constitutive_CallCast* )( KeyHandle( self, self->constitutiveK ), self, element_lI ))

void Snac_Stress( void* context, Element_LocalIndex element_lI ) {
	Snac_Context*			self = (Snac_Context*)context;
	Tetrahedra_Index		tetra_I;
	Snac_Element*			element = Snac_Element_At( self, element_lI );
	Stress				traceStress[Tetrahedra_Count];
	Stress				partialStress;
	Stress				sVolAvg;
	Stress				sOtherAvg;
	/* for adjusting density to changed pressure */
	Material_Index                  material_I = element->material_I;
	Snac_Material*          material = &self->materialProperty[material_I];
	Density                 phsDensity = material->phsDensity; // node->density
	double          alpha = material->alpha;
	double          beta = material->beta;
	Stress          pressure = 0.0f;
	double			elemVolume = ( Tetrahedra_Count > 5 )?(2.0*element->volume):element->volume;

	/* Calculate the strain for each tetrahedra. */
	for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
		/* Initialise the strain tensor for this tetrahedra to 0 */
		memset( element->tetra[tetra_I].strain, 0, sizeof(element->tetra[tetra_I].strain) );

		/* Calculate the strain from the strain rate for this tetrahedra   */
		element->tetra[tetra_I].strain[0][0] = element->tetra[tetra_I].strainRate[0][0] * self->dt;
		element->tetra[tetra_I].strain[1][1] = element->tetra[tetra_I].strainRate[1][1] * self->dt;
		element->tetra[tetra_I].strain[2][2] = element->tetra[tetra_I].strainRate[2][2] * self->dt;
		element->tetra[tetra_I].strain[0][1] = element->tetra[tetra_I].strainRate[0][1] * self->dt;
		element->tetra[tetra_I].strain[0][2] = element->tetra[tetra_I].strainRate[0][2] * self->dt;
		element->tetra[tetra_I].strain[1][2] = element->tetra[tetra_I].strainRate[1][2] * self->dt;
	}

	/* Calculate the stresses for each tetrahedra. */
	Snac_Constitutive_CallEP( self, element_lI );

	partialStress = 0.0f;
	for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {

		Journal_DFirewall( 
				!isnan(element->tetra[tetra_I].stress[0][0]) &&
					!isinf(element->tetra[tetra_I].stress[0][0]),
				self->snacError,
				"Element %u: tetra: %u, stress tensor [0][0] is either nan or infinity", element_lI, tetra_I );
		Journal_DFirewall( 
				!isnan(element->tetra[tetra_I].stress[1][1]) &&
					!isinf(element->tetra[tetra_I].stress[1][1]),
				self->snacError,
				"Element %u: tetra: %u, stress tensor [1][1] is either nan or infinity", element_lI, tetra_I );
		Journal_DFirewall( 
				!isnan(element->tetra[tetra_I].stress[2][2]) &&
					!isinf(element->tetra[tetra_I].stress[2][2]),
				self->snacError,
				"Element %u: tetra: %u, stress tensor [2][2] is either nan or infinity", element_lI, tetra_I );
		Journal_DFirewall( 
				!isnan(element->tetra[tetra_I].stress[0][1]) &&
					!isinf(element->tetra[tetra_I].stress[0][1]),
				self->snacError,
				"Element %u: tetra: %u, stress tensor [0][1] is either nan or infinity", element_lI, tetra_I );
		Journal_DFirewall( 
				!isnan(element->tetra[tetra_I].stress[0][2]) &&
					!isinf(element->tetra[tetra_I].stress[0][2]),
				self->snacError,
				"Element %u: tetra: %u, stress tensor [0][2] is either nan or infinity", element_lI, tetra_I );
		Journal_DFirewall( 
				!isnan(element->tetra[tetra_I].stress[1][2]) &&
					!isinf(element->tetra[tetra_I].stress[1][2]),
				self->snacError,
				"Element %u: tetra: %u, stress tensor [1][2] is either nan or infinity", element_lI, tetra_I );

		/* Initialise the trace stress tensor for this tetrahedra to 0 */
		memset( &traceStress[tetra_I], 0, sizeof(Stress) );
		traceStress[tetra_I]=  (
			element->tetra[tetra_I].stress[0][0] +
			element->tetra[tetra_I].stress[1][1] +
			element->tetra[tetra_I].stress[2][2] ) / 3.0f;
		partialStress += traceStress[tetra_I] * element->tetra[tetra_I].volume;
	}

	sVolAvg   = 0.0f;
	sOtherAvg = 0.0f;
	pressure  = 0.0f;
	for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
		const double averageTraceStress = partialStress / elemVolume;
		element->tetra[tetra_I].stress[0][0] += -1.0f * traceStress[tetra_I] + averageTraceStress;
		element->tetra[tetra_I].stress[1][1] += -1.0f * traceStress[tetra_I] + averageTraceStress;
		element->tetra[tetra_I].stress[2][2] += -1.0f * traceStress[tetra_I] + averageTraceStress;

		sVolAvg +=
			(element->tetra[tetra_I].stress[1][1]-averageTraceStress) * (element->tetra[tetra_I].stress[2][2]-averageTraceStress) +
			(element->tetra[tetra_I].stress[2][2]-averageTraceStress) * (element->tetra[tetra_I].stress[0][0]-averageTraceStress) +
			(element->tetra[tetra_I].stress[0][0]-averageTraceStress) * (element->tetra[tetra_I].stress[1][1]-averageTraceStress);
		sOtherAvg +=
			element->tetra[tetra_I].stress[0][1] * element->tetra[tetra_I].stress[0][1] +
			element->tetra[tetra_I].stress[1][2] * element->tetra[tetra_I].stress[1][2] +
			element->tetra[tetra_I].stress[0][2] * element->tetra[tetra_I].stress[0][2];

		pressure += (element->tetra[tetra_I].stress[0][0]+element->tetra[tetra_I].stress[1][1]+element->tetra[tetra_I].stress[2][2])/3.0f;
	}
	sVolAvg /= Tetrahedra_Count;
	sOtherAvg /= Tetrahedra_Count;
	pressure /= Tetrahedra_Count;

	/* Calculate the element stress from the tetrahedra stress tensors */
	element->stress         = 0.5f * sqrt( 0.5f * fabs( -1.0f * sVolAvg + sOtherAvg ) );
	element->hydroPressure  = -1.0*pressure;

	/* To catch nan or inf in the stress values even in the optimised mode. */
	/* In the usual DEBUG mode, the above Journal_DFirewalls are sufficient. */
	/* -- EChoi 03/06/2009. */
#ifdef NDEBUG
	Journal_OFirewall( (!isnan(element->stress) && !isinf(element->stress)),
					   self->snacError, __FILE__, __func__, __LINE__,
					   "timeStep=%u rank=%u Element %u: The scalar measure of stress is either nan or infinity", 
					   self->timeStep, self->rank, element_lI );
#endif

	/* update density with updated pressure */
	for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ )
		element->tetra[tetra_I].density = phsDensity * (1.0 - alpha * (element->tetra[tetra_I].avgTemp-material->reftemp) + beta * pressure);

}

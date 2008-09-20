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
** $Id: StrainRate.c 3218 2006-06-14 20:42:31Z EunseoChoi $
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
#include "StrainRate.h"
#include "Context.h"
#include <math.h>
#include <string.h>

void Snac_StrainRate( void* context, Element_LocalIndex element_lI ) {
	Snac_Context*		self = (Snac_Context*)context;
	Tetrahedra_Index	tetra_I;
	Snac_Element*		element = Snac_Element_At( self, element_lI );
	StrainRate		traceStrainRate[Tetrahedra_Count];
	StrainRate		partialStrainRate;
	StrainRate		srVolAvg;
	StrainRate		srOtherAvg;
	double			elemVolume = ( Tetrahedra_Count > 5 )?(2.0*element->volume):element->volume;

	/* Calculate the strain rate for each tetrahedra. */
	for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
		/* Initialise the strain rate tensor for this tetrahedra to 0 */
		memset( &element->tetra[tetra_I].strainRate, 0, sizeof(element->tetra[tetra_I].strainRate) );

		/* Calculate tetrahedra strain rate (1st pass)*/
		element->tetra[tetra_I].strainRate[0][0] = -1.0f * (
			Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][0])->velocity[0] *
				element->tetra[tetra_I].surface[1].normal[0] *
				element->tetra[tetra_I].surface[1].area +
			Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][1])->velocity[0] *
				element->tetra[tetra_I].surface[2].normal[0] *
				element->tetra[tetra_I].surface[2].area +
			Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][2])->velocity[0] *
				element->tetra[tetra_I].surface[3].normal[0] *
				element->tetra[tetra_I].surface[3].area +
			Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][3])->velocity[0] *
				element->tetra[tetra_I].surface[0].normal[0] *
				element->tetra[tetra_I].surface[0].area ) /
			3.0f / element->tetra[tetra_I].volume;
		element->tetra[tetra_I].strainRate[1][1] = -1.0f * (
			Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][0])->velocity[1] *
				element->tetra[tetra_I].surface[1].normal[1] *
				element->tetra[tetra_I].surface[1].area +
			Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][1])->velocity[1] *
				element->tetra[tetra_I].surface[2].normal[1] *
				element->tetra[tetra_I].surface[2].area +
			Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][2])->velocity[1] *
				element->tetra[tetra_I].surface[3].normal[1] *
				element->tetra[tetra_I].surface[3].area +
			Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][3])->velocity[1] *
				element->tetra[tetra_I].surface[0].normal[1] *
				element->tetra[tetra_I].surface[0].area ) /
			3.0f / element->tetra[tetra_I].volume;
		element->tetra[tetra_I].strainRate[2][2] = -1.0f * (
			Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][0])->velocity[2] *
				element->tetra[tetra_I].surface[1].normal[2] *
				element->tetra[tetra_I].surface[1].area +
			Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][1])->velocity[2] *
				element->tetra[tetra_I].surface[2].normal[2] *
				element->tetra[tetra_I].surface[2].area +
			Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][2])->velocity[2] *
				element->tetra[tetra_I].surface[3].normal[2] *
				element->tetra[tetra_I].surface[3].area +
			Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][3])->velocity[2] *
				element->tetra[tetra_I].surface[0].normal[2] *
				element->tetra[tetra_I].surface[0].area ) /
			3.0f / element->tetra[tetra_I].volume;
		element->tetra[tetra_I].strainRate[0][1] = -0.5f * (
			Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][0])->velocity[0] *
				element->tetra[tetra_I].surface[1].normal[1] *
				element->tetra[tetra_I].surface[1].area +
			Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][1])->velocity[0] *
				element->tetra[tetra_I].surface[2].normal[1] *
				element->tetra[tetra_I].surface[2].area +
			Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][2])->velocity[0] *
				element->tetra[tetra_I].surface[3].normal[1] *
				element->tetra[tetra_I].surface[3].area +
			Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][3])->velocity[0] *
				element->tetra[tetra_I].surface[0].normal[1] *
				element->tetra[tetra_I].surface[0].area +
			Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][0])->velocity[1] *
				element->tetra[tetra_I].surface[1].normal[0] *
				element->tetra[tetra_I].surface[1].area +
			Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][1])->velocity[1] *
				element->tetra[tetra_I].surface[2].normal[0] *
				element->tetra[tetra_I].surface[2].area +
			Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][2])->velocity[1] *
				element->tetra[tetra_I].surface[3].normal[0] *
				element->tetra[tetra_I].surface[3].area +
			Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][3])->velocity[1] *
				element->tetra[tetra_I].surface[0].normal[0] *
				element->tetra[tetra_I].surface[0].area ) /
			3.0f / element->tetra[tetra_I].volume;
		element->tetra[tetra_I].strainRate[0][2] = -0.5f * (
			Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][0])->velocity[0] *
				element->tetra[tetra_I].surface[1].normal[2] *
				element->tetra[tetra_I].surface[1].area +
			Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][1])->velocity[0] *
				element->tetra[tetra_I].surface[2].normal[2] *
				element->tetra[tetra_I].surface[2].area +
			Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][2])->velocity[0] *
				element->tetra[tetra_I].surface[3].normal[2] *
				element->tetra[tetra_I].surface[3].area +
			Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][3])->velocity[0] *
				element->tetra[tetra_I].surface[0].normal[2] *
				element->tetra[tetra_I].surface[0].area +
			Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][0])->velocity[2] *
				element->tetra[tetra_I].surface[1].normal[0] *
				element->tetra[tetra_I].surface[1].area +
			Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][1])->velocity[2] *
				element->tetra[tetra_I].surface[2].normal[0] *
				element->tetra[tetra_I].surface[2].area +
			Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][2])->velocity[2] *
				element->tetra[tetra_I].surface[3].normal[0] *
				element->tetra[tetra_I].surface[3].area +
			Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][3])->velocity[2] *
				element->tetra[tetra_I].surface[0].normal[0] *
				element->tetra[tetra_I].surface[0].area ) /
			3.0f / element->tetra[tetra_I].volume;
		element->tetra[tetra_I].strainRate[1][2] = -0.5f * (
			Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][0])->velocity[1] *
				element->tetra[tetra_I].surface[1].normal[2] *
				element->tetra[tetra_I].surface[1].area +
			Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][1])->velocity[1] *
				element->tetra[tetra_I].surface[2].normal[2] *
				element->tetra[tetra_I].surface[2].area +
			Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][2])->velocity[1] *
				element->tetra[tetra_I].surface[3].normal[2] *
				element->tetra[tetra_I].surface[3].area +
			Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][3])->velocity[1] *
				element->tetra[tetra_I].surface[0].normal[2] *
				element->tetra[tetra_I].surface[0].area +
			Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][0])->velocity[2] *
				element->tetra[tetra_I].surface[1].normal[1] *
				element->tetra[tetra_I].surface[1].area +
			Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][1])->velocity[2] *
				element->tetra[tetra_I].surface[2].normal[1] *
				element->tetra[tetra_I].surface[2].area +
			Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][2])->velocity[2] *
				element->tetra[tetra_I].surface[3].normal[1] *
				element->tetra[tetra_I].surface[3].area +
			Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][3])->velocity[2] *
				element->tetra[tetra_I].surface[0].normal[1] *
				element->tetra[tetra_I].surface[0].area ) /
			3.0f / element->tetra[tetra_I].volume;

		Journal_DFirewall( 
			!isnan(Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][0])->velocity[0]), 
			self->snacError,
			"Element %u: tetra: %u, element-node: %u, velocity[0] is nan", element_lI, tetra_I, TetraToNode[tetra_I][0] );
		Journal_DFirewall( 
			!isnan(Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][1])->velocity[0]),
			self->snacError,
			"Element %u: tetra: %u, element-node: %u, velocity[0] is nan", element_lI, tetra_I, TetraToNode[tetra_I][1] );
		Journal_DFirewall( 
			!isnan(Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][2])->velocity[0]),
			self->snacError,
			"Element %u: tetra: %u, element-node: %u, velocity[0] is nan", element_lI, tetra_I, TetraToNode[tetra_I][2] );
		Journal_DFirewall( 
			!isnan(Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][3])->velocity[0]),
			self->snacError,
			"Element %u: tetra: %u, element-node: %u, velocity[0] is nan", element_lI, tetra_I, TetraToNode[tetra_I][3] );
		Journal_DFirewall( 
			!isnan(Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][0])->velocity[1]),
			self->snacError,
			"Element %u: tetra: %u, element-node: %u, velocity[0] is nan", element_lI, tetra_I, TetraToNode[tetra_I][0] );
		Journal_DFirewall( 
			!isnan(Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][1])->velocity[1]),
			self->snacError,
			"Element %u: tetra: %u, element-node: %u, velocity[1] is nan", element_lI, tetra_I, TetraToNode[tetra_I][1] );
		Journal_DFirewall( 
			!isnan(Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][2])->velocity[1]),
			self->snacError,
			"Element %u: tetra: %u, element-node: %u, velocity[1] is nan", element_lI, tetra_I, TetraToNode[tetra_I][2] );
		Journal_DFirewall( 
			!isnan(Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][3])->velocity[1]),
			self->snacError,
			"Element %u: tetra: %u, element-node: %u, velocity[1] is nan", element_lI, tetra_I, TetraToNode[tetra_I][3] );
		Journal_DFirewall( 
			!isnan(Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][0])->velocity[2]),
			self->snacError,
			"Element %u: tetra: %u, element-node: %u, velocity[2] is nan", element_lI, tetra_I, TetraToNode[tetra_I][0] );
		Journal_DFirewall( 
			!isnan(Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][1])->velocity[2]),
			self->snacError,
			"Element %u: tetra: %u, element-node: %u, velocity[2] is nan", element_lI, tetra_I, TetraToNode[tetra_I][1] );
		Journal_DFirewall( 
			!isnan(Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][2])->velocity[2]),
			self->snacError,
			"Element %u: tetra: %u, element-node: %u, velocity[2] is nan", element_lI, tetra_I, TetraToNode[tetra_I][2] );
		Journal_DFirewall( 
			!isnan(Snac_Element_Node_P( self, element_lI, TetraToNode[tetra_I][3])->velocity[2]),
			self->snacError,
			"Element %u: tetra: %u, element-node: %u, velocity[2] is nan", element_lI, tetra_I, TetraToNode[tetra_I][3] );

		Journal_DFirewall( 
			!isnan(element->tetra[tetra_I].volume) && !isinf(element->tetra[tetra_I].volume),
			self->snacError,
			"Element %u: tetra: %u, volume is either nan or infinity", element_lI, tetra_I );

		Journal_DFirewall( 
			!isnan(element->tetra[tetra_I].strainRate[0][0]) &&
				!isinf(element->tetra[tetra_I].strainRate[0][0]),
			self->snacError,
			"Element %u: tetra: %u, strain-rate tensor [0][0] is either nan or infinity", element_lI, tetra_I );
		Journal_DFirewall( 
			!isnan(element->tetra[tetra_I].strainRate[1][1]) &&
				!isinf(element->tetra[tetra_I].strainRate[1][1]),
			self->snacError,
			"Element %u: tetra: %u, strain-rate tensor [1][1] is either nan or infinity", element_lI, tetra_I );
		Journal_DFirewall( 
			!isnan(element->tetra[tetra_I].strainRate[2][2]) &&
				!isinf(element->tetra[tetra_I].strainRate[2][2]),
			self->snacError,
			"Element %u: tetra: %u, strain-rate tensor [2][2] is either nan or infinity", element_lI, tetra_I );
		Journal_DFirewall( 
			!isnan(element->tetra[tetra_I].strainRate[0][1]) &&
				!isinf(element->tetra[tetra_I].strainRate[0][1]),
			self->snacError,
			"Element %u: tetra: %u, strain-rate tensor [0][1] is either nan or infinity", element_lI, tetra_I );
		Journal_DFirewall( 
			!isnan(element->tetra[tetra_I].strainRate[0][2]) &&
				!isinf(element->tetra[tetra_I].strainRate[0][2]),
			self->snacError,
			"Element %u: tetra: %u, strain-rate tensor [0][2] is either nan or infinity", element_lI, tetra_I );
		Journal_DFirewall( 
			!isnan(element->tetra[tetra_I].strainRate[1][2]) &&
				!isinf(element->tetra[tetra_I].strainRate[1][2]),
			self->snacError,
			"Element %u: tetra: %u, strain-rate tensor [1][2] is either nan or infinity", element_lI, tetra_I );
	}

	/*   Mixed Discretization: Calculate the volumique average of the first invariant of each strain rates of each Tetrahedra */
	partialStrainRate = 0.0f;
	for( tetra_I = 0; tetra_I < Tetrahedra_Count;  tetra_I++ ) {
		/* Initialise the trace strain rate tensor for this tetrahedra to 0 */
		memset( &traceStrainRate[tetra_I], 0, sizeof(StrainRate) );
		traceStrainRate[tetra_I] = (
			element->tetra[tetra_I].strainRate[0][0] +
			element->tetra[tetra_I].strainRate[1][1] +
			element->tetra[tetra_I].strainRate[2][2] ) / 3.0f;
		partialStrainRate += traceStrainRate[tetra_I] * element->tetra[tetra_I].volume;
	}

	srVolAvg = 0.0f;
	srOtherAvg = 0.0f;
	for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
		element->tetra[tetra_I].strainRate[0][0] += -1.0f * traceStrainRate[tetra_I] + partialStrainRate / elemVolume;
		element->tetra[tetra_I].strainRate[1][1] += -1.0f * traceStrainRate[tetra_I] + partialStrainRate / elemVolume;
		element->tetra[tetra_I].strainRate[2][2] += -1.0f * traceStrainRate[tetra_I] + partialStrainRate / elemVolume;

		srVolAvg +=
			element->tetra[tetra_I].strainRate[1][1] * element->tetra[tetra_I].strainRate[2][2] +
			element->tetra[tetra_I].strainRate[2][2] * element->tetra[tetra_I].strainRate[0][0] +
			element->tetra[tetra_I].strainRate[0][0] * element->tetra[tetra_I].strainRate[1][1];
		srOtherAvg +=
			element->tetra[tetra_I].strainRate[0][1] * element->tetra[tetra_I].strainRate[0][1] +
			element->tetra[tetra_I].strainRate[1][2] * element->tetra[tetra_I].strainRate[1][2] +
			element->tetra[tetra_I].strainRate[0][2] * element->tetra[tetra_I].strainRate[0][2];
	}
	srVolAvg /= Tetrahedra_Count;
	srOtherAvg /= Tetrahedra_Count;

	/* Calculate the element strain rate from the tetrahedra strain rate tensors */
	element->strainRate = 0.5f * sqrt( 0.5f * fabs( -1.0f * srVolAvg + srOtherAvg ) );
}

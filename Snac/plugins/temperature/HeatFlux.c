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
 ** $Id: HeatFlux.c 3265 2006-11-15 21:43:39Z EunseoChoi $
 **
 **~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "units.h"
#include "types.h"
#include "Context.h"
#include "Element.h"
#include "Node.h"
#include "Register.h"
#include "HeatFlux.h"
#include "VariableConditions.h"
#include <string.h>
#include <math.h>

void SnacTemperature_LoopElements( void* _context ) {
	Snac_Context*			context = (Snac_Context*)_context;
	Element_LocalIndex		element_lI;

	Journal_Printf( context->debug, "In %s(): Updating heat flux of all elements.\n", __func__ );

	for( element_lI = 0; element_lI < context->mesh->elementLocalCount; element_lI++ ) {
		Snac_HeatFlux( context, element_lI );
	}

	SnacTemperature_HeatFluxBC( context );
}


void Snac_HeatFlux( void* _context, Element_LocalIndex element_lI ) {
	Snac_Context*					context = (Snac_Context*)_context;
	Tetrahedra_Index				tetra_I;
	Snac_Element*					element = Snac_Element_At( context, element_lI );
	SnacTemperature_Element*		elementExt = ExtensionManager_Get(
			context->mesh->elementExtensionMgr,
			element,
			SnacTemperature_ElementHandle );

    const Snac_Material* 			material = &context->materialProperty[element->material_I];
	const double					bulkm = material->lambda + 2.0f * material->mu/3.0f;
	const double					alpha = material->alpha;
	double							temp = 0.0f, temp0 = 0.0f;

	/* Calculate the Heat Flux for each tetrahedra. */
	for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
		Snac_Node*			contributingNode[4] = {
			Snac_Element_Node_P( context, element_lI, TetraToNode[tetra_I][0] ),
			Snac_Element_Node_P( context, element_lI, TetraToNode[tetra_I][1] ),
			Snac_Element_Node_P( context, element_lI, TetraToNode[tetra_I][2] ),
			Snac_Element_Node_P( context, element_lI, TetraToNode[tetra_I][3] )
		};
		SnacTemperature_Node*		contributingNodeExt[4] = {
			ExtensionManager_Get(
					context->mesh->nodeExtensionMgr,
					contributingNode[0],
					SnacTemperature_NodeHandle ),
			ExtensionManager_Get(
					context->mesh->nodeExtensionMgr,
					contributingNode[1],
					SnacTemperature_NodeHandle ),
			ExtensionManager_Get(
					context->mesh->nodeExtensionMgr,
					contributingNode[2],
					SnacTemperature_NodeHandle ),
			ExtensionManager_Get(
					context->mesh->nodeExtensionMgr,
					contributingNode[3],
					SnacTemperature_NodeHandle ),
		};

		/* Calculate tetrahedra heat flux (1st pass)*/
		elementExt->heatFlux[tetra_I][0] = 1.0f * material->thermal_conduct * (
				contributingNodeExt[0]->temperature *
				element->tetra[tetra_I].surface[1].normal[0] *
				element->tetra[tetra_I].surface[1].area +
				contributingNodeExt[1]->temperature *
				element->tetra[tetra_I].surface[2].normal[0] *
				element->tetra[tetra_I].surface[2].area +
				contributingNodeExt[2]->temperature *
				element->tetra[tetra_I].surface[3].normal[0] *
				element->tetra[tetra_I].surface[3].area +
				contributingNodeExt[3]->temperature *
				element->tetra[tetra_I].surface[0].normal[0] *
				element->tetra[tetra_I].surface[0].area ) /
			3.0f / element->tetra[tetra_I].volume;
		elementExt->heatFlux[tetra_I][1] = 1.0f * material->thermal_conduct * (
				contributingNodeExt[0]->temperature *
				element->tetra[tetra_I].surface[1].normal[1] *
				element->tetra[tetra_I].surface[1].area +
				contributingNodeExt[1]->temperature *
				element->tetra[tetra_I].surface[2].normal[1] *
				element->tetra[tetra_I].surface[2].area +
				contributingNodeExt[2]->temperature *
				element->tetra[tetra_I].surface[3].normal[1] *
				element->tetra[tetra_I].surface[3].area +
				contributingNodeExt[3]->temperature *
				element->tetra[tetra_I].surface[0].normal[1] *
				element->tetra[tetra_I].surface[0].area ) /
			3.0f / element->tetra[tetra_I].volume;
		elementExt->heatFlux[tetra_I][2] = 1.0f * material->thermal_conduct * (
				contributingNodeExt[0]->temperature *
				element->tetra[tetra_I].surface[1].normal[2] *
				element->tetra[tetra_I].surface[1].area +
				contributingNodeExt[1]->temperature *
				element->tetra[tetra_I].surface[2].normal[2] *
				element->tetra[tetra_I].surface[2].area +
				contributingNodeExt[2]->temperature *
				element->tetra[tetra_I].surface[3].normal[2] *
				element->tetra[tetra_I].surface[3].area +
				contributingNodeExt[3]->temperature *
				element->tetra[tetra_I].surface[0].normal[2] *
				element->tetra[tetra_I].surface[0].area ) /
			3.0f / element->tetra[tetra_I].volume;

		/* thermal stress */
		if( context->computeThermalStress ) {
			temp = 0.25f * ( contributingNodeExt[0]->temperature + contributingNodeExt[1]->temperature + contributingNodeExt[2]->temperature + contributingNodeExt[3]->temperature );
			temp0 = 0.25f * ( contributingNodeExt[0]->temperature0 + contributingNodeExt[1]->temperature0 + contributingNodeExt[2]->temperature0 + contributingNodeExt[3]->temperature0 );
			/* Thermal stresses (alfa_v = 3.e-5 1/K) */
			/* elementExt->thermalStress[tetra_I] = -1.0f*alpha*bulkm*(context->timeStep==1?(1.0*(temp-1300.0)):(temp-temp0)); */
			elementExt->thermalStress[tetra_I] = -1.0f*alpha*bulkm*(temp-temp0);
			Journal_Firewall( (!isinf(elementExt->thermalStress[tetra_I]) && !isnan(elementExt->thermalStress[tetra_I])), " timeStep %d thermalStress for element %d tetra %d (T: %e %e) is inf or nan!!\n",context->timeStep,element_lI,tetra_I,temp,temp0);
		}
	}
}

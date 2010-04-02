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
** $Id: Heat.c 3272 2007-02-07 23:40:54Z EunseoChoi $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "units.h"
#include "types.h"
#include "Heat.h"
#include "Context.h"
#include "Element.h"
#include "Node.h"
#include "Register.h"
#include "VariableConditions.h"
#include <math.h>
#include <assert.h>

void SnacTemperature_LoopNodes( void* _context ) {
	Snac_Context*			context = (Snac_Context*)_context;
	Node_LocalIndex			node_lI;

	Journal_Printf( context->debug, "In %s(): updating temperature of all nodes.\n", __func__ );
	Journal_Printf( context->snacInfo, "In %s(): updating temperature of all nodes.\n", __func__ );

	for( node_lI = 0; node_lI < context->mesh->nodeLocalCount; node_lI++ )  {
		Snac_Heat( context, node_lI, 0 );
	}

	/* update tetra average temp to recompute density later in Snac_Stress(). */
	UpdateAverageTemp_LoopElements( context );

	SnacTemperature_BoundaryConditions( context );
}

void Snac_Heat( void* _context, Node_LocalIndex node_lI, double sourceterm ) {
	Snac_Context*			context = (Snac_Context*)_context;
	SnacTemperature_Context*	contextExt = ExtensionManager_Get(
						context->extensionMgr,
						context,
						SnacTemperature_ContextHandle );
	Node_ElementIndex		nodeElement_I;
	Node_ElementIndex		nodeElementCount;
	Snac_Node*			node = Snac_Node_At( context, node_lI );
	SnacTemperature_Node*		nodeExt = ExtensionManager_Get( context->mesh->nodeExtensionMgr, node, SnacTemperature_NodeHandle );
	double				energy=0.0f, source=0.0f, dt_thermal_to_mech=0.0f;
	double				lumpVolume=0.0f;

	/* Calculate Energy at the nodes */
	nodeElementCount = context->mesh->nodeElementCountTbl[node_lI];
	for( nodeElement_I = 0; nodeElement_I < nodeElementCount; nodeElement_I++ )
	{ /* for each element of this node */
		Element_LocalIndex		element_lI = context->mesh->nodeElementTbl[node_lI][nodeElement_I];

		/* Nodes on the meshes boundary do not have all 8 neighbouring elements... ignore these */
		if( element_lI < context->mesh->elementDomainCount ) {
			Snac_Element*			element = Snac_Element_At( context, element_lI );
			const Snac_Material* material = &context->materialProperty[element->material_I];
			SnacTemperature_Element*	elementExt = ExtensionManager_Get(
								context->mesh->elementExtensionMgr,
								element,
								SnacTemperature_ElementHandle );
			Index				index;

			for( index = 0; index < Node_Element_Tetrahedra_Count; index++ ) {
				const Tetrahedra_Index			tetra_I = NodeToTetra[nodeElement_I][index];
				const Tetrahedra_Surface_Index		surface_I = NodeToSurface[nodeElement_I][index];

				/* Element info shortcuts */
				const Snac_Element_Tetrahedra		tetra = element->tetra[tetra_I];
				HeatFlux*				heatFlux = &elementExt->heatFlux[tetra_I];
				const Snac_Element_Tetrahedra_Surface	surface = tetra.surface[surface_I];
				const Normal*				normal = &surface.normal;

				/* Incorporate this surface's heat flux into the energy */
				/* the last factor of 2 is needed for two layouts */
				energy +=
					( (*heatFlux)[0] * (*normal)[0] +
					  (*heatFlux)[1] * (*normal)[1] +
					  (*heatFlux)[2] * (*normal)[2] ) * surface.area / 6.0f;

				/* sourceterm is a volumetric heat source */
				source += sourceterm * tetra.volume / 4.0f;

				/* density calculation below is redundant but kept as a safety measure. */
				lumpVolume += material->heatCapacity *
					(
					 (
					  (tetra.density==0.0)?
					  (material->phsDensity*(1.0-material->alpha*(tetra.avgTemp-material->reftemp) + material->beta * element->hydroPressure)):
					  tetra.density
					  ) * tetra.volume
					 ) / 8.0f;
			}
			/* Update temperature at the Node */
			/* we are using mechanical time step, context->dt.
			   In most cases, dt_mech << dt_therm, so it should be o.k.
			   However, just in case, we assert that the ratio of dt_thermal to dt_mech
			   is larger than one */
			dt_thermal_to_mech = sqrt( (material->lambda+2.0f*material->mu) / material->phsDensity ) *
				context->minLengthScale / (material->thermal_conduct / (material->heatCapacity*material->phsDensity) );
			Journal_Firewall( (dt_thermal_to_mech > 1.0 ),
							  context->snacError, "In %s: rank=%d timeStep=%d Check parameters: lambda=%e mu=%e rho=%e dL=%e k=%e Cp=%e\n",
							  __func__, context->rank,context->timeStep, material->lambda,material->mu,
							  material->phsDensity,context->minLengthScale,material->thermal_conduct,material->heatCapacity);
		}
	}

	nodeExt->temperature0 = nodeExt->temperature;
	nodeExt->temperature += (energy + source) * -1.0f * context->dt / lumpVolume;

}


void UpdateAverageTemp_LoopElements( void* _context )
{

	Snac_Context*           context = (Snac_Context*)_context;
	Element_LocalIndex      element_lI;
	Tetrahedra_Index	    tetra_I;
	Index                   tetraNode_I;

	for( element_lI = 0; element_lI < context->mesh->elementLocalCount; element_lI++ ) {
		Snac_Element*		element = Snac_Element_At( context, element_lI );

		for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
			Snac_Element_Tetrahedra*		tetra = &element->tetra[tetra_I];

			tetra->avgTemp = 0.0;
			for(tetraNode_I = 0; tetraNode_I < Tetrahedra_Point_Count; tetraNode_I++ ) {
				Snac_Node* contributingNode = Snac_Element_Node_P(
					context,
					element_lI,
					TetraToNode[tetra_I][tetraNode_I] );
				SnacTemperature_Node* temperatureNodeExt = ExtensionManager_Get(
					context->mesh->nodeExtensionMgr,
					contributingNode,
					SnacTemperature_NodeHandle );

				tetra->avgTemp += temperatureNodeExt->temperature/(1.0f*Tetrahedra_Point_Count);
			}
		}
	}
}

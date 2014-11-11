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
** $Id: Constitutive.c 3274 2007-03-27 20:25:29Z EunseoChoi $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "math.h"

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "units.h"
#include "types.h"
#include "Element.h"
#include "Register.h"
#include "Constitutive.h"
#include "Snac/Temperature/Temperature.h"

#include <assert.h>

void _SnacMaxwell_Constitutive( void* _context, Element_LocalIndex element_lI ) {
	Snac_Context*			context = (Snac_Context*)_context;
	Snac_Element*			element = Snac_Element_At( context, element_lI );
	SnacMaxwell_Element*		elementExt = ExtensionManager_Get(
						context->mesh->elementExtensionMgr,
						element,
						SnacMaxwell_ElementHandle );
	SnacTemperature_Element* temperatureElement = ExtensionManager_Get(
						context->mesh->elementExtensionMgr,
						element,
						SnacTemperature_ElementHandle );
	const Snac_Material*		material = &context->materialProperty[element->material_I];

	EntryPoint* 			temperatureEP;
	temperatureEP = Context_GetEntryPoint( context,	"Snac_EP_LoopElementsEnergy" );

	/* If this is a Maxwell material, calculate its stress. */
	if( material->rheology & Snac_Material_Maxwell ) {
		Tetrahedra_Index		tetra_I;

		for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
			const double			bulkm = material->lambda + 2.0f * material->mu/3.0f;
			StressTensor*			stress = &element->tetra[tetra_I].stress;
			StrainTensor*			strain = &element->tetra[tetra_I].strain;
			Maxwell*			viscosity = &elementExt->viscosity[tetra_I];
			double				straind0,straind1,straind2,stressd0,stressd1,stressd2;
			double				trace_strain;
			double				trace_stress;
			double				temp;
			double				vic1;
			double				vic2;
			double				VolumicStress;
			double				rviscosity=material->refvisc;
                        double                          rmu= material->mu;
			double				srJ2;
			double				avgTemp;

			Node_LocalIndex			node_lI;
			/* For now reference values of viscosity, second invariant of deviatoric */
			/* strain rate and reference temperature  are being hard wired ( these specific */
			/* values are from paper by Hall et. al., EPSL, 2003 */
			double				rstrainrate = material->refsrate;
			double				rTemp = material->reftemp;
			double				H = material->activationE; // kJ/mol
			double				srexponent = material->srexponent;
			const double			R=8.31448;  // J/mol/K
#if 0
			double				a2 = material->lambda;
			double				sh2 = 2.0 * material->mu;
			double				s00,s11,s22,s01,s02,s12;

			/*	elastic update */
		/* 	s00=(*stress)[0][0] + sh2 * (*strain)[0][0] + a2 * (trace_strain ); */
/*  			s11=(*stress)[1][1] + sh2 * (*strain)[1][1] + a2 * (trace_strain ); */
/* 			s22=(*stress)[2][2] + sh2 * (*strain)[2][2] + a2 * (trace_strain ); */
/* 			s01=(*stress)[0][1] + sh2 * (*strain)[0][1]; */
/* 			s02=(*stress)[0][2] + sh2 * (*strain)[0][2]; */
/* 			s12=(*stress)[1][2] + sh2 * (*strain)[1][2]; */
#endif
			if( context->computeThermalStress ) {
				(*stress)[0][0] += temperatureElement->thermalStress[tetra_I];
				(*stress)[1][1] += temperatureElement->thermalStress[tetra_I];
				(*stress)[2][2] += temperatureElement->thermalStress[tetra_I];
			}

			trace_stress = (*stress)[0][0] + (*stress)[1][1] + (*stress)[2][2];
			trace_strain = element->tetra[tetra_I].volume/element->tetra[tetra_I].old_volume-1.0f;

			/* Deviatoric Stresses and Strains */
			straind0 =  (*strain)[0][0] -  (trace_strain) / 3.0f;
			straind1 =  (*strain)[1][1] -  (trace_strain) / 3.0f;
			straind2 =  (*strain)[2][2] -  (trace_strain) / 3.0f;

			stressd0 =  (*stress)[0][0] -  (trace_stress) / 3.0f;
			stressd1 =  (*stress)[1][1] -  (trace_stress) / 3.0f;
			stressd2 =  (*stress)[2][2] -  (trace_stress) / 3.0f;
			if( temperatureEP ) {
				srJ2 = sqrt(fabs(straind1*straind2+straind2*straind0+straind0*straind1 -(*strain)[0][1]*(*strain)[0][1]-(*strain)[0][2]*(*strain)[0][2]-(*strain)[1][2]*(*strain)[1][2]))/context->dt;
				if(srJ2 == 0.0f) srJ2 = rstrainrate; // temporary. should be vmax/length_scale

				avgTemp=0.0;
				for(node_lI=0; node_lI<4; node_lI++) {
					Snac_Node* contributingNode = Snac_Element_Node_P( context, element_lI, TetraToNode[tetra_I][node_lI] );
					SnacTemperature_Node* temperatureNodeExt = ExtensionManager_Get(
												 context->mesh->nodeExtensionMgr,contributingNode,
												 SnacTemperature_NodeHandle );

					avgTemp += 0.25 * temperatureNodeExt->temperature;
					assert( !isnan(avgTemp) && !isinf(avgTemp) );
				}

				(*viscosity)= rviscosity*pow((srJ2/rstrainrate),(1./srexponent-1.))
					*exp(H/R*(1./(avgTemp+273.15)-1./(rTemp+273.15)));
				if((*viscosity) < material->vis_min) (*viscosity) = material->vis_min;
				if((*viscosity) > material->vis_max) (*viscosity) = material->vis_max;

				if( isnan((*viscosity)) || isinf((*viscosity))) {
					fprintf(stderr,"rvisc=%e Erattio=%e pow(E)=%e, dT=%e exp=%e\n",
						rviscosity,(srJ2/rstrainrate),pow((srJ2/rstrainrate),(1./srexponent-1.)),
						exp(H/R*(1./(avgTemp+273.15)-1./(rTemp+273.15))),(1./(avgTemp+273.15)-1./(rTemp+273.15)) );
				}
				assert(!isnan((*viscosity)) && !isinf((*viscosity)));
			}
			else
				(*viscosity) = rviscosity;
			assert(!isnan((*viscosity)) && !isinf((*viscosity)));

                       /*   *viscosity=material->refvisc; */
			/* Non dimensional parameters elastic/viscous */
			temp = rmu / (2.0f* (*viscosity)) * context->dt;


			vic1 = 1.0f - temp;
			vic2 = 1.0f / (1.0f + temp);
                       /*   printf("temp=%g\t rmu=%g\t viscosity=%g\t\n",temp,rmu,*viscosity); */
/*                          printf("trace_stress=%g\t trace_strain=%g\t vic1=%g\t vic2=%g\n",trace_stress,trace_strain,vic1,vic2); */

			/* Deviatoric Stress Update */

			stressd0 =  (stressd0 * vic1 + 2.0f * rmu * straind0) * vic2 ;
			stressd1 =  (stressd1 * vic1 + 2.0f * rmu * straind1) * vic2 ;
			stressd2 =  (stressd2 * vic1 + 2.0f * rmu * straind2) * vic2 ;

			(*stress)[0][1] =((*stress)[0][1] * vic1 + 2.0f * rmu * (*strain)[0][1]) * vic2;
			(*stress)[0][2] =((*stress)[0][2] * vic1 + 2.0f * rmu * (*strain)[0][2]) * vic2;
			(*stress)[1][2] =((*stress)[1][2] * vic1 + 2.0f * rmu * (*strain)[1][2]) * vic2;

			/* Isotropic stress is elastic,
			   WARNING:volumic Strain may be better defined as
			   volumique change in the mesh */
			VolumicStress = trace_stress / 3.0f + bulkm * trace_strain;

			(*stress)[0][0] = stressd0 + VolumicStress;
			(*stress)[1][1] = stressd1 + VolumicStress;
			(*stress)[2][2] = stressd2 + VolumicStress;
		}
	}
}

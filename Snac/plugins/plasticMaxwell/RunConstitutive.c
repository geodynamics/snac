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
** $Id: RunConstitutive.c 3274 2007-03-27 20:25:29Z EunseoChoi $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "Register.h"
#include "RunConstitutive.h"

#include <string.h>
#include <assert.h>

void _SnacPlasticMaxwell_RunConstitutive(
		void*					entryPoint,
		void*					_context,
		Element_LocalIndex			element_lI )
{
	Snac_EntryPoint*			self = (Snac_EntryPoint*)entryPoint;
	Snac_Context*				context = (Snac_Context*)_context;
	Snac_Element*				element = Snac_Element_At( context, element_lI );
	const Snac_Material*			material = &context->materialProperty[element->material_I];
	Hook_Index				hookIndex;
	Snac_TetraStressTensor*			stress = &(Snac_Element_At( (Snac_Context*)context, element_lI )->tetraStress);
	Snac_TetraStressTensor			stressPlastic;
	Snac_TetraStressTensor			stressMaxwell;
	Bool					didPlastic = False;
	Bool					didMaxwell = False;
	double                                  getSecondInvariant();
	double                                  sII_visc, sII_plas;

	for( hookIndex = 0; hookIndex < self->hooks->count; hookIndex++ ) {
		/* TODO: Removal of strcmp is possible if Hook_New uses GLOBAL... meaning all names to entry point hooks must
		   must be global const variables... need to chat with team first. */
		if(
			material->rheology & Snac_Material_Plastic &&
			!strcmp( NamedObject_GetName( self->hooks->data[hookIndex] ), "SnacPlastic" ) )
		{
			memcpy( stressPlastic, *stress, sizeof(Snac_TetraStressTensor) );
			(*(Snac_Constitutive_Cast*)((Hook*)self->hooks->data[hookIndex])->funcPtr)
				( context, element_lI );
			didPlastic = True;
		}
		else if(
			material->rheology & Snac_Material_Maxwell &&
			!strcmp( NamedObject_GetName( self->hooks->data[hookIndex] ), "SnacMaxwell" ) )
		{
			memcpy( stressMaxwell, *stress, sizeof(Snac_TetraStressTensor) );
			(*(Snac_Constitutive_Cast*)((Hook*)self->hooks->data[hookIndex])->funcPtr)
				( context, element_lI );
			didMaxwell = True;
		}
		else {
			(*(Snac_Constitutive_Cast*)((Hook*)self->hooks->data[hookIndex])->funcPtr)
				( context, element_lI );
		}
	}

	if( didPlastic && didMaxwell ) {
		sII_visc = getSecondInvariant(stressMaxwell);
		sII_plas = getSecondInvariant(stressPlastic);
		assert( !isnan(sII_visc) && !isinf(sII_visc) );
		assert( !isnan(sII_plas) && !isinf(sII_plas) );

		if(sII_visc <= sII_plas) {
			memcpy( *stress, stressMaxwell, sizeof(Snac_TetraStressTensor) );
			element->irheology = -1;
		}
		else {
			memcpy( *stress, stressPlastic, sizeof(Snac_TetraStressTensor) );
			element->irheology = 1;
		}
	}
	else if( didPlastic ) {
		memcpy( *stress, stressPlastic, sizeof(Snac_TetraStressTensor) );
		element->irheology = 1;
	}
	else if( didMaxwell ) {
		memcpy( *stress, stressMaxwell, sizeof(Snac_TetraStressTensor) );
		element->irheology = -1;
	}
	/* else: let the other updates (e.g. Elastic) be the value */
}


double getSecondInvariant(Snac_TetraStressTensor stress)
{

	double s00,s11,s22,s01,s02,s12;
	double spVolAvg,spOtherAvg;
	Tetrahedra_Index		tetra_I;

	s00 = s11 = s22 = s01 = s02 = s12 = 0.0f;
	for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
		s00 += stress[tetra_I][0][0]/Tetrahedra_Count;
		s11 += stress[tetra_I][1][1]/Tetrahedra_Count;
		s22 += stress[tetra_I][2][2]/Tetrahedra_Count;
		s01 += stress[tetra_I][0][1]/Tetrahedra_Count;
		s02 += stress[tetra_I][0][2]/Tetrahedra_Count;
		s12 += stress[tetra_I][1][2]/Tetrahedra_Count;
	}
	spVolAvg = s00*s11 + s11*s22 + s22*s00;
	spOtherAvg = s01*s01 + s12*s12 + s02*s02;
	return fabs(-1.0f*spVolAvg + spOtherAvg);
}

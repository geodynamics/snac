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
 
#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "Register.h"
#include "Elastic.h"

void _SnacElastic_Constitutive( void* _context, Element_LocalIndex element_dI ) {
	Snac_Context*			context = (Snac_Context*)_context;
	Snac_Element*			element = Snac_Element_At( context, element_dI );
	const Snac_Material*		material = &context->materialProperty[element->material_I];
	double				trace_strain;

	/* If this is a Elastic material, calculate its stress. */
	if( material->rheology & Snac_Material_Elastic ) {
		Tetrahedra_Index		tetra_I;
		
		for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
			const double			a2 = material->lambda;
			const double			sh2 = 2.0 * material->mu;
			StressTensor*			stress = &element->tetra[tetra_I].stress;
			StrainTensor*			strain = &element->tetra[tetra_I].strain;
			
			trace_strain = (*strain)[0][0] + (*strain)[1][1] + (*strain)[2][2];

			(*stress)[0][0] += sh2 * (*strain)[0][0] + a2 * (trace_strain ); 
			(*stress)[1][1] += sh2 * (*strain)[1][1] + a2 * (trace_strain ); 
			(*stress)[2][2] += sh2 * (*strain)[2][2] + a2 * (trace_strain ); 
			(*stress)[0][1] += sh2 * (*strain)[0][1]; 
			(*stress)[0][2] += sh2 * (*strain)[0][2]; 
			(*stress)[1][2] += sh2 * (*strain)[1][2]; 
		}
	}
}

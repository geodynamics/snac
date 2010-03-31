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
** $Id: Force.c 3274 2007-03-27 20:25:29Z EunseoChoi $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>

#include "units.h"
#include "types.h"
#include "shortcuts.h"
#include "Material.h"
#include "Tetrahedra.h"
#include "TetrahedraTables.h"
#include "Node.h"
#include "Element.h"
#include "Context.h"
#include "Stress.h"
#include "Force.h"
#include <math.h>
#include <string.h>
#include <assert.h>

int forceCompare( const void* e1, const void* e2 );

void Snac_Force(
		void*				context,
		Node_LocalIndex			node_lI,
		double				speedOfSound,
		Mass*				mass,
		Mass*				inertialMass,
		Force*				force,
		Force*				balance )
{
	Snac_Context*			self = (Snac_Context*)context;
	Node_ElementIndex		nodeElement_I;
	Node_ElementIndex		nodeElementCount;
	const double			factor4 = 1.0f / 4.0f;
	double				dirnorm;
	double				dir2centr[3];
	const double			gravity = self->gravity;

	if(self->spherical) {
		Coord*	coord = Snac_NodeCoord_P( self, node_lI );
		dirnorm = sqrt( (*coord)[0]*(*coord)[0] + (*coord)[1]*(*coord)[1] + (*coord)[2]*(*coord)[2] );
		dir2centr[0] = (*coord)[0] / dirnorm;
		dir2centr[1] = (*coord)[1] / dirnorm;
		dir2centr[2] = (*coord)[2] / dirnorm;
	}

	/* Calculate real and inertial masses. Order of summation matters due to floating point round-off, hence we implement it
	    three different ways. */
	*inertialMass = 0.0f;
	*mass = 0.0f;
	nodeElementCount = self->mesh->nodeElementCountTbl[node_lI];
	memset( *force, 0, sizeof(Force) );
	memset( *balance, 0, sizeof(Force) );
	if( self->forceCalcType == Snac_Force_Complete ) {

		for( nodeElement_I = 0; nodeElement_I < nodeElementCount; nodeElement_I++ ) { /* for each element of this node */
			Element_DomainIndex		element_lI = self->mesh->nodeElementTbl[node_lI][nodeElement_I];

			/* Nodes on the meshes boundary do not have all 8 neighbouring elements... but they are stored in
			   in the domain */
			if( element_lI < self->mesh->elementDomainCount ) {
				Snac_Element*			element = Snac_Element_At( self, element_lI );
				Material_Index			material_I = element->material_I;
				const Snac_Material*	material = &self->materialProperty[material_I];
				const Density			inertialDensity = (material->lambda + 2.0f * material->mu )/(speedOfSound * speedOfSound);
				Index					index;

				/* for each tetra that refers to this node */
				for( index = 0; index < Node_Element_Tetrahedra_Count;index++ ) {
					const Tetrahedra_Index				tetra_I = NodeToTetra[nodeElement_I][index];
					const Tetrahedra_Surface_Index		surface_I = NodeToSurface[nodeElement_I][index];

					/* Element info shortcuts */
					const Snac_Element_Tetrahedra*			tetra = &element->tetra[tetra_I];
					const StressTensor*						stress = (const StressTensor*) &tetra->stress;
					const Snac_Element_Tetrahedra_Surface*	surface = &tetra->surface[surface_I];
					const Normal*							normal = &surface->normal;
					const Density 							effDensity = tetra->density;

					/* Work out the mass contributions */
					if( self->dtType == Snac_DtType_Dynamic ) {

						*inertialMass += factor4 * inertialDensity * element->tetra[tetra_I].volume;
						Journal_DFirewall(
							!isnan( *inertialMass ) && !isinf( *inertialMass ),
							self->snacError,
							"forceCalc: Complete, dtType: Dynamic, element_lI: %u, inertialDensity is either nan or inf", element_lI );
					}
					else if( self->dtType == Snac_DtType_Constant ) {
						double				alpha1 = (material->lambda + 2.0f*material->mu);
						int					dim;
						double				temp;
						double				area_sum = 0.0;

						for( dim = 0; dim < 3; dim++ ) {
							temp = ((*normal)[dim] * surface->area) * ((*normal)[dim] * surface->area);
							if( temp > area_sum ) {
								area_sum = temp;
							}
						}
						*inertialMass += alpha1 * area_sum / (9.0f * tetra->volume) * (self->dt * self->dt);
						Journal_DFirewall(
							!isnan( *inertialMass ) && !isinf( *inertialMass ),
							self->snacError,
							"forceCalc: Complete, dtType: Constant, element_lI: %u, inertialDensity is either nan or inf", element_lI );
					}
					else if( self->dtType == Snac_DtType_Wave ) {
						*inertialMass += factor4 * effDensity * element->tetra[tetra_I].volume;
						Journal_DFirewall(
							!isnan( *inertialMass ) && !isinf( *inertialMass ),
							self->snacError,
							"forceCalc: Complete, dtType: Courant, element_lI: %u, inertialDensity is either nan or inf", element_lI );
					}
					else {
						Journal_DFirewall(
							0,
							self->snacError,
							"forceCalc: Complete, dtType: Unknown. Aborting.\n");
					}
					/*ccccc*/

					/* Incorporate this surface's stresses into the force */
					(*force)[0] +=
						( (*stress)[0][0] * (*normal)[0] +
						  (*stress)[0][1] * (*normal)[1] +
						  (*stress)[0][2] * (*normal)[2] ) * surface->area / 6.0f;
					(*force)[1] +=
						( (*stress)[0][1] * (*normal)[0] +
						  (*stress)[1][1] * (*normal)[1] +
						  (*stress)[1][2] * (*normal)[2] ) * surface->area / 6.0f;
					(*force)[2] +=
						( (*stress)[0][2] * (*normal)[0] +
						  (*stress)[1][2] * (*normal)[1] +
						  (*stress)[2][2] * (*normal)[2] ) * surface->area / 6.0f;
					
					if(self->spherical) {
						(*force)[0] -= dir2centr[0] * (gravity * effDensity * tetra->volume / 8.0f);
						(*force)[1] -= dir2centr[1] * (gravity * effDensity * tetra->volume / 8.0f);
						(*force)[2] -= dir2centr[2] * (gravity * effDensity * tetra->volume / 8.0f);
					}
					else {
						(*force)[1] -= gravity * effDensity * tetra->volume / 8.0f;
					}

					(*balance)[0] += fabs( (*force)[0] );
					(*balance)[1] += fabs( (*force)[1] );
					(*balance)[2] += fabs( (*force)[2] );

					*mass += effDensity * tetra->volume / 8.0;
				}
			}
		}
	}
	else { /* "quick" */
		Journal_Firewall( 0, self->snacError, "ForcalCalcType, \"complete\" shold be used !!\n");
	}
}

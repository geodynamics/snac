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
** $Id: VariableConditions.c 1410 2004-05-17 00:49:44Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "Force.h"
#include "Register.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>


/*==========================================================================================*/
/*  Bottom support force ( a.k.a., Archimed's force, or Winkler foundation ) term is added  */
/*==========================================================================================*/
void _SnacWinklerForce_Apply(
		void*				_context,
		Node_LocalIndex			node_lI,
		double				speedOfSound,
		Mass*				mass,
		Mass*				inertialMass,
		Force*				force,
		Force*				balance )
{
	Snac_Context*			context = (Snac_Context*)_context;
	MeshLayout*			meshLayout = (MeshLayout*)context->meshLayout;
	double				dhE, rho_g, area;
	double                          normal[3];
	Node_ElementIndex		nodeElement_I, nodeElementCount;
	const double			factor2 = ( context->forceCalcType == Snac_Force_Complete ) ? (1.0f) : (1.0f/2.0f);
	const double			factor3 = 1.0f / 3.0f;
	const double			factor4 = 1.0f / 4.0f;

	/* loop over all the elements surrounding node_dI */
	if( context->gravity > 0.0 ) {
		HexaMD*			decomp = (HexaMD*)meshLayout->decomp;
		Node_GlobalIndex	node_gI = context->mesh->nodeL2G[node_lI];
		IJK			ijk;

		RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );
		if(ijk[1]==0) {
			nodeElementCount = context->mesh->nodeElementCountTbl[node_lI];
			for( nodeElement_I = 0; nodeElement_I < nodeElementCount; nodeElement_I++ ) {
				Element_LocalIndex		element_dI = context->mesh->nodeElementTbl[node_lI][nodeElement_I];
				if( element_dI < context->mesh->elementDomainCount ) {
					Snac_Element*		element = Snac_Element_At( context, element_dI );

					rho_g = 3300.0f * context->gravity;
					if(nodeElement_I == 2) {
						dhE = factor3 * ( Snac_Element_NodeCoord( context, element_dI, 1 )[1] +
								  Snac_Element_NodeCoord( context, element_dI, 4 )[1] +
								  Snac_Element_NodeCoord( context, element_dI, 5 )[1] );
						area = Tetrahedra_SurfaceArea( Snac_Element_NodeCoord( context, element_dI, 1 ),
									       Snac_Element_NodeCoord( context, element_dI, 4 ),
									       Snac_Element_NodeCoord( context, element_dI, 5 ) );
						Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_dI, 1 ),
									  Snac_Element_NodeCoord( context, element_dI, 4 ),
									  Snac_Element_NodeCoord( context, element_dI, 5 ),
									  &normal );
						(*force)[0] += factor2 * ( (element->hydroPressure - rho_g * (dhE - element->rzbo)) * area * normal[0] );
						(*force)[1] += factor2 * ( (element->hydroPressure - rho_g * (dhE - element->rzbo)) * area * normal[1] );
						(*force)[2] += factor2 * ( (element->hydroPressure - rho_g * (dhE - element->rzbo)) * area * normal[2] );
					}
					if(nodeElement_I == 3) {
						dhE = factor3 * ( Snac_Element_NodeCoord( context, element_dI, 0 )[1] +
								  Snac_Element_NodeCoord( context, element_dI, 4 )[1] +
								  Snac_Element_NodeCoord( context, element_dI, 5 )[1] );
						area = Tetrahedra_SurfaceArea( Snac_Element_NodeCoord( context, element_dI, 0 ),
									       Snac_Element_NodeCoord( context, element_dI, 4 ),
									       Snac_Element_NodeCoord( context, element_dI, 5 ) );
						Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_dI, 0 ),
									  Snac_Element_NodeCoord( context, element_dI, 4 ),
									  Snac_Element_NodeCoord( context, element_dI, 5 ),
									  &normal );
						(*force)[0] += factor2 * ( (element->hydroPressure - rho_g * (dhE - element->rzbo)) * area * normal[0] );
						(*force)[1] += factor2 * ( (element->hydroPressure - rho_g * (dhE - element->rzbo)) * area * normal[1] );
						(*force)[2] += factor2 * ( (element->hydroPressure - rho_g * (dhE - element->rzbo)) * area * normal[2] );
					}
					if(nodeElement_I == 6) {
						dhE = factor3 * ( Snac_Element_NodeCoord( context, element_dI, 0 )[1] +
								  Snac_Element_NodeCoord( context, element_dI, 5 )[1] +
								  Snac_Element_NodeCoord( context, element_dI, 1 )[1] );
						area = Tetrahedra_SurfaceArea( Snac_Element_NodeCoord( context, element_dI, 0 ),
									       Snac_Element_NodeCoord( context, element_dI, 5 ),
									       Snac_Element_NodeCoord( context, element_dI, 1 ) );
						Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_dI, 0 ),
									  Snac_Element_NodeCoord( context, element_dI, 5 ),
									  Snac_Element_NodeCoord( context, element_dI, 1 ),
									  &normal );
						(*force)[0] += factor2 * ( (element->hydroPressure - rho_g * (dhE - element->rzbo)) * area * normal[0] );
						(*force)[1] += factor2 * ( (element->hydroPressure - rho_g * (dhE - element->rzbo)) * area * normal[1] );
						(*force)[2] += factor2 * ( (element->hydroPressure - rho_g * (dhE - element->rzbo)) * area * normal[2] );
					}
					if(nodeElement_I == 7) {
						dhE = factor3 * ( Snac_Element_NodeCoord( context, element_dI, 0 )[1] +
								  Snac_Element_NodeCoord( context, element_dI, 4 )[1] +
								  Snac_Element_NodeCoord( context, element_dI, 1 )[1] );
						area = Tetrahedra_SurfaceArea( Snac_Element_NodeCoord( context, element_dI, 0 ),
									       Snac_Element_NodeCoord( context, element_dI, 4 ),
									       Snac_Element_NodeCoord( context, element_dI, 1 ) );
						Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_dI, 0 ),
									  Snac_Element_NodeCoord( context, element_dI, 4 ),
									  Snac_Element_NodeCoord( context, element_dI, 1 ),
									  &normal );
						(*force)[0] += factor2 * ( (element->hydroPressure - rho_g * (dhE - element->rzbo)) * area * normal[0] );
						(*force)[1] += factor2 * ( (element->hydroPressure - rho_g * (dhE - element->rzbo)) * area * normal[1] );
						(*force)[2] += factor2 * ( (element->hydroPressure - rho_g * (dhE - element->rzbo)) * area * normal[2] );
					}
				}
			}
		} /* end if if(ijk[1] == ) */
	}
}



void _SnacWinklerForce_Apply_Spherical(
		void*				_context,
		Node_LocalIndex			node_lI,
		double				speedOfSound,
		Mass*				mass,
		Mass*				inertialMass,
		Force*				force,
		Force*				balance )
{
	Snac_Context*			context = (Snac_Context*)_context;
	MeshLayout*			meshLayout = (MeshLayout*)context->meshLayout;
	double				rho_g;
	double				normal1[3],normal2[3],normal3[3],normal4[3];
	double				dF[3],dF1[3],dF2[3],dF3[3],dF4[3];
	Node_ElementIndex		nodeElement_I, nodeElementCount;
	const double			factor2 = 1.0f / 2.0f;
	const double			factor4 = 1.0f / 4.0f;
	double				r1,r2,r3,r4,S1,S2,S3,S4,A1,A2,A3,A4;
	Index				dimI;
	
	/* loop over all the elements surrounding node_dI */
	if( context->gravity > 0.0 ) {
		HexaMD*			decomp = (HexaMD*)meshLayout->decomp;
		Node_GlobalIndex	node_gI = context->mesh->nodeL2G[node_lI];
		IJK			ijk,lijk;

		RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );
		RegularMeshUtils_Node_Local1DTo3D( decomp, node_lI, &lijk[0], &lijk[1], &lijk[2] );
		if(ijk[1]==0) {
			nodeElementCount = context->mesh->nodeElementCountTbl[node_lI];
			for( nodeElement_I = 0; nodeElement_I < nodeElementCount; nodeElement_I++ ) {
				Element_LocalIndex		element_dI = context->mesh->nodeElementTbl[node_lI][nodeElement_I];

				if( element_dI < context->mesh->elementDomainCount ) {
					rho_g = 3300.0f * context->gravity;
					if(nodeElement_I == 2 || nodeElement_I == 3 || nodeElement_I == 6 || nodeElement_I == 7 ) {
						r1 = sqrt( Snac_Element_NodeCoord( context, element_dI, 0 )[0]*Snac_Element_NodeCoord( context, element_dI, 0 )[0] + Snac_Element_NodeCoord( context, element_dI, 0 )[1]*Snac_Element_NodeCoord( context, element_dI, 0 )[1] + Snac_Element_NodeCoord( context, element_dI, 0 )[2]*Snac_Element_NodeCoord( context, element_dI, 0 )[2] );
						r2 = sqrt( Snac_Element_NodeCoord( context, element_dI, 1 )[0]*Snac_Element_NodeCoord( context, element_dI, 1 )[0] + Snac_Element_NodeCoord( context, element_dI, 1 )[1]*Snac_Element_NodeCoord( context, element_dI, 1 )[1] + Snac_Element_NodeCoord( context, element_dI, 1 )[2]*Snac_Element_NodeCoord( context, element_dI, 1 )[2] );
						r3 = sqrt( Snac_Element_NodeCoord( context, element_dI, 4 )[0]*Snac_Element_NodeCoord( context, element_dI, 4 )[0] + Snac_Element_NodeCoord( context, element_dI, 4 )[1]*Snac_Element_NodeCoord( context, element_dI, 4 )[1] + Snac_Element_NodeCoord( context, element_dI, 4 )[2]*Snac_Element_NodeCoord( context, element_dI, 4 )[2] );
						r4 = sqrt( Snac_Element_NodeCoord( context, element_dI, 5 )[0]*Snac_Element_NodeCoord( context, element_dI, 5 )[0] + Snac_Element_NodeCoord( context, element_dI, 5 )[1]*Snac_Element_NodeCoord( context, element_dI, 5 )[1] + Snac_Element_NodeCoord( context, element_dI, 5 )[2]*Snac_Element_NodeCoord( context, element_dI, 5 )[2] );
						S1 = context->density*context->gravity*(Spherical_RMax-r1) - rho_g * fabs(Spherical_RMin - r1);
						S2 = context->density*context->gravity*(Spherical_RMax-r2) - rho_g * fabs(Spherical_RMin - r2);
						S3 = context->density*context->gravity*(Spherical_RMax-r3) - rho_g * fabs(Spherical_RMin - r3);
						S4 = context->density*context->gravity*(Spherical_RMax-r4) - rho_g * fabs(Spherical_RMin - r4);

						A1 = Tetrahedra_SurfaceArea( Snac_Element_NodeCoord( context, element_dI, 1 ),
									     Snac_Element_NodeCoord( context, element_dI, 4 ),
									     Snac_Element_NodeCoord( context, element_dI, 5 ) );
						A2 = Tetrahedra_SurfaceArea( Snac_Element_NodeCoord( context, element_dI, 1 ),
									     Snac_Element_NodeCoord( context, element_dI, 0 ),
									     Snac_Element_NodeCoord( context, element_dI, 4 ) );
						A3 = Tetrahedra_SurfaceArea( Snac_Element_NodeCoord( context, element_dI, 0 ),
									     Snac_Element_NodeCoord( context, element_dI, 4 ),
									     Snac_Element_NodeCoord( context, element_dI, 5 ) );
						A4 = Tetrahedra_SurfaceArea( Snac_Element_NodeCoord( context, element_dI, 0 ),
									     Snac_Element_NodeCoord( context, element_dI, 5 ),
									     Snac_Element_NodeCoord( context, element_dI, 1 ) );
						Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_dI, 1 ),
									  Snac_Element_NodeCoord( context, element_dI, 4 ),
									  Snac_Element_NodeCoord( context, element_dI, 5 ),
									  &normal1 );
						Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_dI, 1 ),
									  Snac_Element_NodeCoord( context, element_dI, 0 ),
									  Snac_Element_NodeCoord( context, element_dI, 4 ),
									  &normal2 );
						Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_dI, 0 ),
									  Snac_Element_NodeCoord( context, element_dI, 4 ),
									  Snac_Element_NodeCoord( context, element_dI, 5 ),
									  &normal3 );
						Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_dI, 0 ),
									  Snac_Element_NodeCoord( context, element_dI, 5 ),
									  Snac_Element_NodeCoord( context, element_dI, 1 ),
									  &normal4 );
						for(dimI=0;dimI<3;dimI++) {
							dF1[dimI] = 1.0/3.0 * (S2 + S3 + S4) * normal1[dimI] * A1;
							dF2[dimI] = 1.0/3.0 * (S2 + S1 + S3) * normal2[dimI] * A2;
							dF3[dimI] = 1.0/3.0 * (S1 + S3 + S4) * normal3[dimI] * A3;
							dF4[dimI] = 1.0/3.0 * (S1 + S4 + S2) * normal4[dimI] * A4;
						}
						for(dimI=0;dimI<3;dimI++) {
							dF[dimI] =  factor2 * (dF1[dimI] + dF2[dimI] + dF3[dimI] + dF4[dimI] );
						}
						(*force)[0] += factor4 * dF[0];
						(*force)[1] += factor4 * dF[1];
						(*force)[2] += factor4 * dF[2];
					}
				}
			}
		}
	}
}


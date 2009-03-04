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

void _SnacCylinderQuad_Force_Apply(
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
	double				area, normal1[3], normal2[3], normal[3];
	Node_ElementIndex		nodeElement_I, nodeElementCount;
	const double			factor4 = 1.0f / 4.0f;
	const double			Pressure = Dictionary_Entry_Value_AsDouble(Dictionary_GetDefault( context->dictionary, "cylinder_innerP", Dictionary_Entry_Value_FromDouble( 1.0e+06 ) ) );

	/* loop over all the elements surrounding node_dI */
	HexaMD*			decomp = (HexaMD*)meshLayout->decomp;
	Node_GlobalIndex	node_gI = context->mesh->nodeL2G[node_lI];
	IJK			ijk;

	RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );

	if(ijk[2]==0) { // inner wall of the cylinder
		nodeElementCount = context->mesh->nodeElementCountTbl[node_lI];
		for( nodeElement_I = 0; nodeElement_I < nodeElementCount; nodeElement_I++ ) {
			Element_LocalIndex		element_lI = context->mesh->nodeElementTbl[node_lI][nodeElement_I];

			if( element_lI < context->mesh->elementDomainCount ) {
				area = Tetrahedra_SurfaceArea( Snac_Element_NodeCoord( context, element_lI, 0 ),
											   Snac_Element_NodeCoord( context, element_lI, 1 ),
											   Snac_Element_NodeCoord( context, element_lI, 2 ) ) +
					Tetrahedra_SurfaceArea( Snac_Element_NodeCoord( context, element_lI, 0 ),
											Snac_Element_NodeCoord( context, element_lI, 2 ),
											Snac_Element_NodeCoord( context, element_lI, 3 ) );
				Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_lI, 0 ),
										  Snac_Element_NodeCoord( context, element_lI, 1 ),
										  Snac_Element_NodeCoord( context, element_lI, 2 ),
										  &normal1 );
				Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_lI, 0 ),
										  Snac_Element_NodeCoord( context, element_lI, 2 ),
										  Snac_Element_NodeCoord( context, element_lI, 3 ),
										  &normal2 );
				
				normal[0] = 0.5f * ( normal1[0] + normal2[0] );
				normal[1] = 0.5f * ( normal1[1] + normal2[1] );
				normal[2] = 0.5f * ( normal1[2] + normal2[2] );
				
				(*force)[0] += factor4 * ( Pressure * area * normal[0] );
				(*force)[1] += factor4 * ( Pressure * area * normal[1] );
				(*force)[2] += factor4 * ( Pressure * area * normal[2] );
				
				(*balance)[0] += fabs( (*force)[0] );
				(*balance)[1] += fabs( (*force)[1] );
				(*balance)[2] += fabs( (*force)[2] );
			}
		}
	}
}

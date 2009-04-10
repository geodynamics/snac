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
#include "InitialConditions.h"
#include "Snac/Temperature/Temperature.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void _SnacWinklerForce_Apply_North( void* _context, Node_LocalIndex	node_lI, Force* force, Force* balance );
void _SnacWinklerForce_Apply_South( void* _context, Node_LocalIndex	node_lI, Force* force, Force* balance );
void _SnacWinklerForce_Apply_East( void* _context, Node_LocalIndex	node_lI, Force* force, Force* balance );
void _SnacWinklerForce_Apply_West( void* _context, Node_LocalIndex	node_lI, Force* force, Force* balance );
void _SnacWinklerForce_Apply_Spherical_North( void* _context, Node_LocalIndex	node_lI, Force* force, Force* balance );
void _SnacWinklerForce_Apply_Spherical_South( void* _context, Node_LocalIndex	node_lI, Force* force, Force* balance );
double getRadius( Coord coord );

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
	double                          normal[3];
	Node_ElementIndex		nodeElement_I, nodeElementCount;
	const double			factor4 = 1.0f / 4.0f;
    double                  Fy;
	Snac_Node*			node = Snac_Node_At( context, node_lI );
	Coord*				coord = Snac_NodeCoord_P( context, node_lI );
	SnacTemperature_Node* temperatureNodeExt = ExtensionManager_Get( context->mesh->nodeExtensionMgr, node, SnacTemperature_NodeHandle );
	double          nodeT =temperatureNodeExt->temperature;

	/* loop over all the elements surrounding node_dI */
	if( context->gravity > 0.0 ) {
		HexaMD*			decomp = (HexaMD*)meshLayout->decomp;
		Node_GlobalIndex	node_gI = context->mesh->nodeL2G[node_lI];
		IJK			ijk;

		RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );
		if(ijk[1]==0) {

			nodeElementCount = context->mesh->nodeElementCountTbl[node_lI];
			for( nodeElement_I = 0; nodeElement_I < nodeElementCount; nodeElement_I++ ) {
				Element_LocalIndex		element_lI = context->mesh->nodeElementTbl[node_lI][nodeElement_I];
				if( element_lI < context->mesh->elementDomainCount ) {
					Snac_Element*		element = Snac_Element_At( context, element_lI );
					Material_Index          material_I = element->material_I;
					Snac_Material*          material = &context->materialProperty[material_I];
					Density                 phsDensity = material->phsDensity; /* node->density */
					Density                 mantleDensity = 3300.0f;
					double          alpha = material->alpha;
					double          beta = material->beta;
					double          drosub = 0.0f;

					double p_est = context->pisos + 0.5f * ( phsDensity + drosub ) * context->gravity * ( (*coord)[1] - element->rzbo );
					double rosubg = context->gravity * ( phsDensity + drosub ) * ( 1.0 - alpha * (nodeT-material->reftemp) + beta * p_est );
					double press_norm = 0.0f;
					double area=0.0f, dhE=0.0f;
					Normal* normal1;
					Normal* normal2;
					Normal* normal3;
					Normal* normal4;

					Snac_Element_Tetrahedra*		tetra;
					Snac_Element_Tetrahedra_Surface*	surface;

					dhE = factor4 * ( Snac_Element_NodeCoord( context, element_lI, 0 )[1] +
									  Snac_Element_NodeCoord( context, element_lI, 1 )[1] +
									  Snac_Element_NodeCoord( context, element_lI, 4 )[1] +
									  Snac_Element_NodeCoord( context, element_lI, 5 )[1] );

					tetra = &element->tetra[1];
					surface = &tetra->surface[0];
					normal1 = &surface->normal;
					area += surface->area;

					tetra = &element->tetra[2];
					surface = &tetra->surface[1];
					normal2 = &surface->normal;
					area += surface->area;

					tetra = &element->tetra[6];
					surface = &tetra->surface[2];
					normal3 = &surface->normal;
					area += surface->area;

					tetra = &element->tetra[8];
					surface = &tetra->surface[2];
					normal4 = &surface->normal;
					area += surface->area;

					normal[0] = factor4 * ( (*normal1)[0] + (*normal2)[0] + (*normal3)[0] + (*normal4)[0] );
					normal[1] = -1.0f * factor4 * ( (*normal1)[1] + (*normal2)[1] + (*normal3)[1] + (*normal4)[1] );
					normal[2] = factor4 * ( (*normal1)[2] + (*normal2)[2] + (*normal3)[2] + (*normal4)[2] );
					area *= 0.5f;

					/* compute spring force due to the change in displacement */
					/* if dh > 0, F < 0; dh < 0, F > 0. */
					/* So, bottom surface goes up, Force in -y direction; down, F acts in +y direction */
					/* Adjust all the signs to be consistent with this principle. */
					/* Let's make isostatic pressure positive and normals to point to the direction of action, +y. */
					/* dP should then have the same sign with dh. */
					press_norm = context->pisos + rosubg * ( element->rzbo - dhE );
					(*force)[0] += factor4 * ( press_norm * area * normal[0] );
					(*force)[1] += factor4 * ( press_norm * area * normal[1] );
					(*force)[2] += factor4 * ( press_norm * area * normal[2] );
				}
			}
			if( context->restartTimestep == 0 ) {
				if( context->timeStep == 1 ) {
					Fy = (*force)[1];
					if(Fy != 0.0)
						node->residualFr = Fy;
				}
			}
			(*force)[1] -= node->residualFr;
		} /* end if if(ijk[1] == ) */
	}
/* 	_SnacWinklerForce_Apply_North( context, node_lI, force, balance ); */
/* 	_SnacWinklerForce_Apply_South( context, node_lI, force, balance ); */
/* 	_SnacWinklerForce_Apply_East( context, node_lI, force, balance ); */
/* 	_SnacWinklerForce_Apply_West( context, node_lI, force, balance ); */
}

void _SnacWinklerForce_Apply_South( void* _context, Node_LocalIndex	node_lI, Force* force, Force* balance )
{
	Snac_Context*			context = (Snac_Context*)_context;
	MeshLayout*			meshLayout = (MeshLayout*)context->meshLayout;
	double				dhE, area;
	Node_ElementIndex		nodeElement_I, nodeElementCount;
	const double			factor4 = 1.0f / 4.0f;
    double                  Fz;
	Snac_Node*			node = Snac_Node_At( context, node_lI );

	/* loop over all the elements surrounding node_dI */
	if( context->gravity > 0.0 ) {
		HexaMD*			decomp = (HexaMD*)meshLayout->decomp;
		Node_GlobalIndex	node_gI = context->mesh->nodeL2G[node_lI];
		IJK			ijk;

		RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );
		/* For Tibet problem */
        if(ijk[2]==decomp->nodeGlobal3DCounts[2]-1) {
            double                          r1,r2,r3,r4;
            double                          normal[3],normal1[3],normal2[3],normal3[3],normal4[3];
            nodeElementCount = context->mesh->nodeElementCountTbl[node_lI];
            for( nodeElement_I = 0; nodeElement_I < nodeElementCount; nodeElement_I++ ) {
                Element_LocalIndex		element_lI = context->mesh->nodeElementTbl[node_lI][nodeElement_I];
                if( element_lI < context->mesh->elementDomainCount ) {
                    if(nodeElement_I == 0 || nodeElement_I == 1 || nodeElement_I == 2 || nodeElement_I == 3) {
                        r1 = Snac_Element_NodeCoord( context, element_lI, 4 )[1];
                        r2 = Snac_Element_NodeCoord( context, element_lI, 5 )[1];
                        r3 = Snac_Element_NodeCoord( context, element_lI, 6 )[1];
                        r4 = Snac_Element_NodeCoord( context, element_lI, 7 )[1];

                        dhE = factor4 * ( r1 + r2 + r3 + r4 );
                        area = Tetrahedra_SurfaceArea( Snac_Element_NodeCoord( context, element_lI, 4 ),
                                                       Snac_Element_NodeCoord( context, element_lI, 6 ),
                                                       Snac_Element_NodeCoord( context, element_lI, 7 ) ) +
                            Tetrahedra_SurfaceArea( Snac_Element_NodeCoord( context, element_lI, 4 ),
                                                    Snac_Element_NodeCoord( context, element_lI, 5 ),
                                                    Snac_Element_NodeCoord( context, element_lI, 6 ) );
                        Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_lI, 4 ),
                                                  Snac_Element_NodeCoord( context, element_lI, 6 ),
                                                  Snac_Element_NodeCoord( context, element_lI, 7 ),
                                                  &normal1 );
                        Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_lI, 4 ),
                                                  Snac_Element_NodeCoord( context, element_lI, 5 ),
                                                  Snac_Element_NodeCoord( context, element_lI, 6 ),
                                                  &normal2 );
                        Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_lI, 5 ),
                                                  Snac_Element_NodeCoord( context, element_lI, 7 ),
                                                  Snac_Element_NodeCoord( context, element_lI, 4 ),
                                                  &normal3 );
                        Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_lI, 5 ),
                                                  Snac_Element_NodeCoord( context, element_lI, 6 ),
                                                  Snac_Element_NodeCoord( context, element_lI, 7 ),
                                                  &normal4 );
                        normal[0] = factor4 * ( normal1[0] + normal2[0] + normal3[0] + normal4[0]);
                        normal[1] = factor4 * ( normal1[1] + normal2[1] + normal3[1] + normal4[1]);
                        normal[2] = factor4 * ( normal1[2] + normal2[2] + normal3[2] + normal4[2] );
                        (*force)[0] -= factor4 * ( context->density * context->gravity * (0.0-dhE) * area * normal[0] );
                        (*force)[1] -= factor4 * ( context->density * context->gravity * (0.0-dhE) * area * normal[1] );
                        (*force)[2] -= factor4 * ( context->density * context->gravity * (0.0-dhE) * area * normal[2] );
                    }
                }
            }
            /*ccccc*/
            if(context->timeStep==1) {
                Fz = (*force)[2];
                if(Fz != 0.0)
                    node->residualFt = Fz;
            }
            (*force)[2] -= node->residualFt;
            /*ccccc*/
        } /* end of if( ijk[0] == nox ) */
	}
}

void _SnacWinklerForce_Apply_North( void* _context, Node_LocalIndex	node_lI, Force* force, Force* balance )
{
	Snac_Context*			context = (Snac_Context*)_context;
	MeshLayout*			meshLayout = (MeshLayout*)context->meshLayout;
	double				dhE, area;
	Node_ElementIndex		nodeElement_I, nodeElementCount;
	const double			factor4 = 1.0f / 4.0f;
    double                  Fz;
	Snac_Node*			node = Snac_Node_At( context, node_lI );

	/* loop over all the elements surrounding node_dI */
	if( context->gravity > 0.0 ) {
		HexaMD*			decomp = (HexaMD*)meshLayout->decomp;
		Node_GlobalIndex	node_gI = context->mesh->nodeL2G[node_lI];
		IJK			ijk;

		RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );
		/* For Tibet problem */
        if(ijk[2]==0) {
            double                          r1,r2,r3,r4;
            double                          normal[3],normal1[3],normal2[3],normal3[3],normal4[3];
            nodeElementCount = context->mesh->nodeElementCountTbl[node_lI];
            for( nodeElement_I = 0; nodeElement_I < nodeElementCount; nodeElement_I++ ) {
                Element_LocalIndex		element_lI = context->mesh->nodeElementTbl[node_lI][nodeElement_I];
                if( element_lI < context->mesh->elementDomainCount ) {
                    if(nodeElement_I == 4 || nodeElement_I == 5 || nodeElement_I == 6 || nodeElement_I == 7) {
                        r1 = Snac_Element_NodeCoord( context, element_lI, 0 )[1];
                        r2 = Snac_Element_NodeCoord( context, element_lI, 1 )[1];
                        r3 = Snac_Element_NodeCoord( context, element_lI, 2 )[1];
                        r4 = Snac_Element_NodeCoord( context, element_lI, 3 )[1];

                        dhE = factor4 * ( r1 + r2 + r3 + r4 );
                        area = Tetrahedra_SurfaceArea( Snac_Element_NodeCoord( context, element_lI, 0 ),
                                                       Snac_Element_NodeCoord( context, element_lI, 3 ),
                                                       Snac_Element_NodeCoord( context, element_lI, 2 ) ) +
                            Tetrahedra_SurfaceArea( Snac_Element_NodeCoord( context, element_lI, 0 ),
                                                    Snac_Element_NodeCoord( context, element_lI, 2 ),
                                                    Snac_Element_NodeCoord( context, element_lI, 1 ) );
                        Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_lI, 0 ),
                                                  Snac_Element_NodeCoord( context, element_lI, 3 ),
                                                  Snac_Element_NodeCoord( context, element_lI, 2 ),
                                                  &normal1 );
                        Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_lI, 0 ),
                                                  Snac_Element_NodeCoord( context, element_lI, 2 ),
                                                  Snac_Element_NodeCoord( context, element_lI, 1 ),
                                                  &normal2 );
                        Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_lI, 0 ),
                                                  Snac_Element_NodeCoord( context, element_lI, 3 ),
                                                  Snac_Element_NodeCoord( context, element_lI, 1 ),
                                                  &normal3 );
                        Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_lI, 1 ),
                                                  Snac_Element_NodeCoord( context, element_lI, 3 ),
                                                  Snac_Element_NodeCoord( context, element_lI, 2 ),
                                                  &normal4 );
                        normal[0] = factor4 * ( normal1[0] + normal2[0] + normal3[0] + normal4[0]);
                        normal[1] = factor4 * ( normal1[1] + normal2[1] + normal3[1] + normal4[1]);
                        normal[2] = factor4 * ( normal1[2] + normal2[2] + normal3[2] + normal4[2] );
                        (*force)[0] -= factor4 * ( context->density * context->gravity * (0.0-dhE) * area * normal[0] );
                        (*force)[1] -= factor4 * ( context->density * context->gravity * (0.0-dhE) * area * normal[1] );
                        (*force)[2] -= factor4 * ( context->density * context->gravity * (0.0-dhE) * area * normal[2] );
                    }
                }
            }
            /*ccccc*/
            if(context->timeStep==1) {
                Fz = (*force)[2];
                if(Fz != 0.0)
                    node->residualFt = Fz;
            }
            (*force)[2] -= node->residualFt;
            /*ccccc*/
        } /* end of if( ijk[0] == nox ) */
	}
}

void _SnacWinklerForce_Apply_East( void* _context, Node_LocalIndex	node_lI, Force* force, Force* balance )
{
	Snac_Context*			context = (Snac_Context*)_context;
	MeshLayout*			meshLayout = (MeshLayout*)context->meshLayout;
	double				dhE, area;
	Node_ElementIndex		nodeElement_I, nodeElementCount;
	const double			factor4 = 1.0f / 4.0f;
    double                  Fz;
	Snac_Node*			node = Snac_Node_At( context, node_lI );

	/* loop over all the elements surrounding node_dI */
	if( context->gravity > 0.0 ) {
		HexaMD*			decomp = (HexaMD*)meshLayout->decomp;
		Node_GlobalIndex	node_gI = context->mesh->nodeL2G[node_lI];
		IJK			ijk;

		RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );
		/* For Tibet problem */
        if(ijk[0]==decomp->nodeGlobal3DCounts[0]-1) {
            double                          r1,r2,r3,r4;
            double                          normal[3],normal1[3],normal2[3],normal3[3],normal4[3];
            nodeElementCount = context->mesh->nodeElementCountTbl[node_lI];
            for( nodeElement_I = 0; nodeElement_I < nodeElementCount; nodeElement_I++ ) {
                Element_LocalIndex		element_lI = context->mesh->nodeElementTbl[node_lI][nodeElement_I];
                if( element_lI < context->mesh->elementDomainCount ) {
                    if(nodeElement_I == 0 || nodeElement_I == 3 || nodeElement_I == 4 || nodeElement_I == 7) {
                        r1 = Snac_Element_NodeCoord( context, element_lI, 1 )[1];
                        r2 = Snac_Element_NodeCoord( context, element_lI, 2 )[1];
                        r3 = Snac_Element_NodeCoord( context, element_lI, 5 )[1];
                        r4 = Snac_Element_NodeCoord( context, element_lI, 6 )[1];

                        dhE = factor4 * ( r1 + r2 + r3 + r4 );
                        area = Tetrahedra_SurfaceArea( Snac_Element_NodeCoord( context, element_lI, 1 ),
                                                       Snac_Element_NodeCoord( context, element_lI, 2 ),
                                                       Snac_Element_NodeCoord( context, element_lI, 6 ) ) +
                            Tetrahedra_SurfaceArea( Snac_Element_NodeCoord( context, element_lI, 1 ),
                                                    Snac_Element_NodeCoord( context, element_lI, 6 ),
                                                    Snac_Element_NodeCoord( context, element_lI, 5 ) );
                        Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_lI, 1 ),
                                                  Snac_Element_NodeCoord( context, element_lI, 2 ),
                                                  Snac_Element_NodeCoord( context, element_lI, 6 ),
                                                  &normal1 );
                        Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_lI, 1 ),
                                                  Snac_Element_NodeCoord( context, element_lI, 6 ),
                                                  Snac_Element_NodeCoord( context, element_lI, 5 ),
                                                  &normal2 );
                        Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_lI, 1 ),
                                                  Snac_Element_NodeCoord( context, element_lI, 2 ),
                                                  Snac_Element_NodeCoord( context, element_lI, 5 ),
                                                  &normal3 );
                        Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_lI, 2 ),
                                                  Snac_Element_NodeCoord( context, element_lI, 6 ),
                                                  Snac_Element_NodeCoord( context, element_lI, 5 ),
                                                  &normal4 );
                        normal[0] = factor4 * ( normal1[0] + normal2[0] + normal3[0] + normal4[0]);
                        normal[1] = factor4 * ( normal1[1] + normal2[1] + normal3[1] + normal4[1]);
                        normal[2] = factor4 * ( normal1[2] + normal2[2] + normal3[2] + normal4[2] );
                        (*force)[0] -= factor4 * ( context->density * context->gravity * (0.0-dhE) * area * normal[0] );
                        (*force)[1] -= factor4 * ( context->density * context->gravity * (0.0-dhE) * area * normal[1] );
                        (*force)[2] -= factor4 * ( context->density * context->gravity * (0.0-dhE) * area * normal[2] );
                    }
                }
            }
            /*ccccc*/
            if(context->timeStep==1) {
                Fz = (*force)[0];
                if(Fz != 0.0)
                    node->residualFt = Fz;
            }
            (*force)[0] -= node->residualFt;
            /*ccccc*/
        } /* end of if( ijk[0] == nox ) */
	}
}

void _SnacWinklerForce_Apply_West( void* _context, Node_LocalIndex	node_lI, Force* force, Force* balance )
{
	Snac_Context*			context = (Snac_Context*)_context;
	MeshLayout*			meshLayout = (MeshLayout*)context->meshLayout;
	double				dhE, area;
	Node_ElementIndex		nodeElement_I, nodeElementCount;
	const double			factor4 = 1.0f / 4.0f;
    double                  Fz;
	Snac_Node*			node = Snac_Node_At( context, node_lI );

	/* loop over all the elements surrounding node_dI */
	if( context->gravity > 0.0 ) {
		HexaMD*			decomp = (HexaMD*)meshLayout->decomp;
		Node_GlobalIndex	node_gI = context->mesh->nodeL2G[node_lI];
		IJK			ijk;

		RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );
		/* For Tibet problem */
        if(ijk[0]==0) {
            double                          r1,r2,r3,r4;
            double                          normal[3],normal1[3],normal2[3],normal3[3],normal4[3];
            nodeElementCount = context->mesh->nodeElementCountTbl[node_lI];
            for( nodeElement_I = 0; nodeElement_I < nodeElementCount; nodeElement_I++ ) {
                Element_LocalIndex		element_lI = context->mesh->nodeElementTbl[node_lI][nodeElement_I];
                if( element_lI < context->mesh->elementDomainCount ) {
                    if(nodeElement_I == 1 || nodeElement_I == 2 || nodeElement_I == 5 || nodeElement_I == 6) {
                        r1 = Snac_Element_NodeCoord( context, element_lI, 0 )[1];
                        r2 = Snac_Element_NodeCoord( context, element_lI, 3 )[1];
                        r3 = Snac_Element_NodeCoord( context, element_lI, 4 )[1];
                        r4 = Snac_Element_NodeCoord( context, element_lI, 7 )[1];

                        dhE = factor4 * ( r1 + r2 + r3 + r4 );
                        area = Tetrahedra_SurfaceArea( Snac_Element_NodeCoord( context, element_lI, 0 ),
                                                       Snac_Element_NodeCoord( context, element_lI, 3 ),
                                                       Snac_Element_NodeCoord( context, element_lI, 7 ) ) +
                            Tetrahedra_SurfaceArea( Snac_Element_NodeCoord( context, element_lI, 0 ),
                                                    Snac_Element_NodeCoord( context, element_lI, 7 ),
                                                    Snac_Element_NodeCoord( context, element_lI, 4 ) );
                        Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_lI, 0 ),
                                                  Snac_Element_NodeCoord( context, element_lI, 3 ),
                                                  Snac_Element_NodeCoord( context, element_lI, 7 ),
                                                  &normal1 );
                        Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_lI, 0 ),
                                                  Snac_Element_NodeCoord( context, element_lI, 7 ),
                                                  Snac_Element_NodeCoord( context, element_lI, 4 ),
                                                  &normal2 );
                        Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_lI, 0 ),
                                                  Snac_Element_NodeCoord( context, element_lI, 3 ),
                                                  Snac_Element_NodeCoord( context, element_lI, 4 ),
                                                  &normal3 );
                        Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_lI, 3 ),
                                                  Snac_Element_NodeCoord( context, element_lI, 4 ),
                                                  Snac_Element_NodeCoord( context, element_lI, 7 ),
                                                  &normal4 );
                        normal[0] = factor4 * ( normal1[0] + normal2[0] + normal3[0] + normal4[0]);
                        normal[1] = factor4 * ( normal1[1] + normal2[1] + normal3[1] + normal4[1]);
                        normal[2] = factor4 * ( normal1[2] + normal2[2] + normal3[2] + normal4[2] );
                        (*force)[0] -= factor4 * ( context->density * context->gravity * (0.0-dhE) * area * normal[0] );
                        (*force)[1] -= factor4 * ( context->density * context->gravity * (0.0-dhE) * area * normal[1] );
                        (*force)[2] -= factor4 * ( context->density * context->gravity * (0.0-dhE) * area * normal[2] );
                    }
                }
            }
            /*ccccc*/
            if(context->timeStep==1) {
                Fz = (*force)[0];
                if(Fz != 0.0)
                    node->residualFt = Fz;
            }
            (*force)[0] -= node->residualFt;
            /*ccccc*/
        } /* end of if( ijk[0] == nox ) */
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
	Node_ElementIndex		nodeElement_I, nodeElementCount;
	const double			factor4 = 1.0f / 4.0f;
	double				radius,theta,phi;
	Snac_Node*			node = Snac_Node_At( context, node_lI );
	Coord*				coord = Snac_NodeCoord_P( context, node_lI );
	SnacTemperature_Node* temperatureNodeExt = ExtensionManager_Get( context->mesh->nodeExtensionMgr, node, SnacTemperature_NodeHandle );
	double          nodeT =temperatureNodeExt->temperature;
	double          Fr;
	float           sphF[3];
	HexaMD*			decomp = (HexaMD*)meshLayout->decomp;
	Node_GlobalIndex	node_gI = context->mesh->nodeL2G[node_lI];
	IJK			ijk;

	RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );

	radius = sqrt( (*coord)[0]*(*coord)[0] + (*coord)[1]*(*coord)[1] + (*coord)[2]*(*coord)[2] );
	theta = acos((*coord)[2]/radius);
	phi = atan2((*coord)[1],(*coord)[0]);
	/* loop over all the elements surrounding node_dI */
	if( context->gravity > 0.0 ) {
		if(ijk[1]==0) {
			nodeElementCount = context->mesh->nodeElementCountTbl[node_lI];
			for( nodeElement_I = 0; nodeElement_I < nodeElementCount; nodeElement_I++ ) {
				Element_LocalIndex		element_lI = context->mesh->nodeElementTbl[node_lI][nodeElement_I];
				if( element_lI < context->mesh->elementDomainCount ) {
					Snac_Element*		element = Snac_Element_At( context, element_lI );
					Material_Index          material_I = element->material_I;
					Snac_Material*          material = &context->materialProperty[material_I];
					Density                 phsDensity = material->phsDensity; /* node->density */
					Density                 mantleDensity = 3300.0f;
					double          alpha = material->alpha;
					double          beta = material->beta;
					double          drosub = 0.0f;

					double p_est = context->pisos + 0.5f * ( phsDensity + drosub ) * context->gravity * ( radius - Spherical_RMin );
					double rosubg = context->gravity * ( phsDensity + drosub ) * ( 1.0 - alpha * (nodeT-material->reftemp) + beta * p_est );
					double press_norm = 0.0f;
					float normal[3];
					Normal* normal1;
					Normal* normal2;
					Normal* normal3;
					Normal* normal4;
					double r1,r2,r3,r4;
					double dhE, area=0.0f, mag_normal;
					Snac_Element_Tetrahedra*		tetra;
					Snac_Element_Tetrahedra_Surface*	surface;

					r1 = getRadius( Snac_Element_NodeCoord( context, element_lI, 0 ) );
					r2 = getRadius( Snac_Element_NodeCoord( context, element_lI, 1 ) );
					r3 = getRadius( Snac_Element_NodeCoord( context, element_lI, 5 ) );
					r4 = getRadius( Snac_Element_NodeCoord( context, element_lI, 4 ) );
					dhE = 0.25f * ( r1 + r2 + r3 + r4 );

					tetra = &element->tetra[1];
					surface = &tetra->surface[0];
					normal1 = &surface->normal;
					area += surface->area;

					tetra = &element->tetra[2];
					surface = &tetra->surface[1];
					normal2 = &surface->normal;
					area += surface->area;

					tetra = &element->tetra[6];
					surface = &tetra->surface[2];
					normal3 = &surface->normal;
					area += surface->area;

					tetra = &element->tetra[8];
					surface = &tetra->surface[2];
					normal4 = &surface->normal;
					area += surface->area;

					normal[0] = factor4 * ( (*normal1)[0] + (*normal2)[0] + (*normal3)[0] + (*normal4)[0] );
					normal[1] = -1.0f * factor4 * ( (*normal1)[1] + (*normal2)[1] + (*normal3)[1] + (*normal4)[1] );
					normal[2] = factor4 * ( (*normal1)[2] + (*normal2)[2] + (*normal3)[2] + (*normal4)[2] );
					mag_normal = sqrt( normal[0]*normal[0] + normal[1]*normal[1] + normal[2]*normal[2] );
					area *= 0.5f;

					press_norm = context->pisos + rosubg * ( Spherical_RMin - dhE );
					(*force)[0] += factor4 * ( press_norm * area * normal[0] );
					(*force)[1] += factor4 * ( press_norm * area * normal[1] );
					(*force)[2] += factor4 * ( press_norm * area * normal[2] );
				}
			}
		}
	}
	if( context->restartTimestep == 0 ) {
		if( context->timeStep == 1 ) {
			Fr = (*force)[0]*sin(theta)*cos(phi) + (*force)[1]*sin(theta)*sin(phi) + (*force)[2]*cos(theta);
			node->residualFr = Fr;
		}
	}

	sphF[0] = (*force)[0]*sin(theta)*cos(phi) + (*force)[1]*sin(theta)*sin(phi) + (*force)[2]*cos(theta);
	sphF[1] = (*force)[0]*cos(theta)*cos(phi) + (*force)[1]*cos(theta)*sin(phi) - (*force)[2]*sin(theta);
	sphF[2] = -1.0f * (*force)[0]*sin(phi) + (*force)[1]*cos(phi);
	sphF[0] -= node->residualFr;
	(*force)[0] = sphF[0]*sin(theta)*cos(phi) + sphF[1]*cos(theta)*cos(phi) - sphF[2]*sin(phi);
	(*force)[1] = sphF[0]*sin(theta)*sin(phi) + sphF[1]*cos(theta)*sin(phi) + sphF[2]*cos(phi);
	(*force)[2] = sphF[0]*cos(theta) - sphF[1]*sin(theta);

	_SnacWinklerForce_Apply_Spherical_North( context, node_lI, force, balance );
	_SnacWinklerForce_Apply_Spherical_South( context, node_lI, force, balance );
}

void _SnacWinklerForce_Apply_Spherical_North( void* _context, Node_LocalIndex	node_lI, Force* force, Force* balance )
{
	Snac_Context*			context = (Snac_Context*)_context;
	MeshLayout*			meshLayout = (MeshLayout*)context->meshLayout;
	double				area;
	double                          normal[3];
	Node_ElementIndex		nodeElement_I, nodeElementCount;
	const double			factor2 = 1.0f / 2.0f;
	double				radius,theta,phi;
	Snac_Node*			node = Snac_Node_At( context, node_lI );
	Coord*				coord = Snac_NodeCoord_P( context, node_lI );
	double              Ft, sphF[3];

	radius = sqrt( (*coord)[0]*(*coord)[0] + (*coord)[1]*(*coord)[1] + (*coord)[2]*(*coord)[2] );
	theta = acos((*coord)[2]/radius);
	phi = atan2((*coord)[1],(*coord)[0]);

	if( context->gravity > 0.0 ) {
		HexaMD*			decomp = (HexaMD*)meshLayout->decomp;
		Node_GlobalIndex	node_gI = context->mesh->nodeL2G[node_lI];
		IJK			ijk;

		RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );
		if( ijk[2] == 0 ) {
			nodeElementCount = context->mesh->nodeElementCountTbl[node_lI];
			for( nodeElement_I = 0; nodeElement_I < nodeElementCount; nodeElement_I++ ) {
				Element_LocalIndex		element_lI = context->mesh->nodeElementTbl[node_lI][nodeElement_I];
				if( element_lI < context->mesh->elementDomainCount ) {
					Snac_Element*		element = Snac_Element_At( context, element_lI );

					if(nodeElement_I == 4) {
						area = Tetrahedra_SurfaceArea( Snac_Element_NodeCoord( context, element_lI, 1 ),
									       Snac_Element_NodeCoord( context, element_lI, 2 ),
									       Snac_Element_NodeCoord( context, element_lI, 3 ) );
						Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_lI, 1 ),
									  Snac_Element_NodeCoord( context, element_lI, 2 ),
									  Snac_Element_NodeCoord( context, element_lI, 3 ),
									  &normal );
					}
					if(nodeElement_I == 5) {
						area = Tetrahedra_SurfaceArea( Snac_Element_NodeCoord( context, element_lI, 0 ),
													   Snac_Element_NodeCoord( context, element_lI, 2 ),
													   Snac_Element_NodeCoord( context, element_lI, 3 ) );
						Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_lI, 0 ),
									  Snac_Element_NodeCoord( context, element_lI, 2 ),
									  Snac_Element_NodeCoord( context, element_lI, 3 ),
									  &normal );
					}
					if(nodeElement_I == 6) {
						area = Tetrahedra_SurfaceArea( Snac_Element_NodeCoord( context, element_lI, 0 ),
									       Snac_Element_NodeCoord( context, element_lI, 1 ),
									       Snac_Element_NodeCoord( context, element_lI, 3 ) );
						Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_lI, 0 ),
									  Snac_Element_NodeCoord( context, element_lI, 1 ),
									  Snac_Element_NodeCoord( context, element_lI, 3 ),
									  &normal );
					}
					if(nodeElement_I == 7) {
						area = Tetrahedra_SurfaceArea( Snac_Element_NodeCoord( context, element_lI, 0 ),
									       Snac_Element_NodeCoord( context, element_lI, 1 ),
									       Snac_Element_NodeCoord( context, element_lI, 2 ) );
						Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_lI, 0 ),
									  Snac_Element_NodeCoord( context, element_lI, 1 ),
									  Snac_Element_NodeCoord( context, element_lI, 2 ),
									  &normal );
					}
					(*force)[0] -= factor2 * ( element->hydroPressure * area * normal[0] );
					(*force)[1] -= factor2 * ( element->hydroPressure * area * normal[1] );
					(*force)[2] -= factor2 * ( element->hydroPressure * area * normal[2] );
				}
			}
            if(context->timeStep==1) {
                Ft = (*force)[0]*cos(theta)*cos(phi) + (*force)[1]*cos(theta)*sin(phi) - (*force)[2]*sin(theta);
                if(Ft != 0.0)
                    node->residualFt = Ft;
            }
			sphF[0] = (*force)[0]*sin(theta)*cos(phi) + (*force)[1]*sin(theta)*sin(phi) + (*force)[2]*cos(theta);
            sphF[1] = (*force)[0]*cos(theta)*cos(phi) + (*force)[1]*cos(theta)*sin(phi) - (*force)[2]*sin(theta);
			sphF[2] = -1.0f * (*force)[0]*sin(phi) + (*force)[1]*cos(phi);
/* 			sphF[1] -= node->residualFt; */
			sphF[1] = 0.0f;
            (*force)[0] = sphF[0]*sin(theta)*cos(phi) + sphF[1]*cos(theta)*cos(phi) - sphF[2]*sin(phi);
            (*force)[1] = sphF[0]*sin(theta)*sin(phi) + sphF[1]*cos(theta)*sin(phi) + sphF[2]*cos(phi);
            (*force)[2] = sphF[0]*cos(theta) - sphF[1]*sin(theta);
		}
	}
}

void _SnacWinklerForce_Apply_Spherical_South( void* _context, Node_LocalIndex	node_lI, Force* force, Force* balance )
{
	Snac_Context*			context = (Snac_Context*)_context;
	MeshLayout*			meshLayout = (MeshLayout*)context->meshLayout;
	double				area;
	double                          normal[3];
	Node_ElementIndex		nodeElement_I, nodeElementCount;
	const double			factor2 = 1.0f / 2.0f;
	double				radius,theta,phi;
	Snac_Node*			node = Snac_Node_At( context, node_lI );
	Coord*				coord = Snac_NodeCoord_P( context, node_lI );
	double              Ft, sphF[3];

	radius = sqrt( (*coord)[0]*(*coord)[0] + (*coord)[1]*(*coord)[1] + (*coord)[2]*(*coord)[2] );
	theta = acos((*coord)[2]/radius);
	phi = atan2((*coord)[1],(*coord)[0]);

	if( context->gravity > 0.0 ) {
		HexaMD*			decomp = (HexaMD*)meshLayout->decomp;
		Node_GlobalIndex	node_gI = context->mesh->nodeL2G[node_lI];
		IJK			ijk;

		RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );
		if( ijk[2] == decomp->nodeGlobal3DCounts[2]-1 ) {
			nodeElementCount = context->mesh->nodeElementCountTbl[node_lI];
			for( nodeElement_I = 0; nodeElement_I < nodeElementCount; nodeElement_I++ ) {
				Element_LocalIndex		element_lI = context->mesh->nodeElementTbl[node_lI][nodeElement_I];
				if( element_lI < context->mesh->elementDomainCount ) {
					Snac_Element*		element = Snac_Element_At( context, element_lI );

					if(nodeElement_I == 0) {
						area = Tetrahedra_SurfaceArea( Snac_Element_NodeCoord( context, element_lI, 5 ),
									       Snac_Element_NodeCoord( context, element_lI, 6 ),
									       Snac_Element_NodeCoord( context, element_lI, 7 ) );
						Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_lI, 5 ),
									  Snac_Element_NodeCoord( context, element_lI, 6 ),
									  Snac_Element_NodeCoord( context, element_lI, 7 ),
									  &normal );
					}
					if(nodeElement_I == 1) {
						area = Tetrahedra_SurfaceArea( Snac_Element_NodeCoord( context, element_lI, 4 ),
									       Snac_Element_NodeCoord( context, element_lI, 6 ),
									       Snac_Element_NodeCoord( context, element_lI, 7 ) );
						Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_lI, 4 ),
									  Snac_Element_NodeCoord( context, element_lI, 6 ),
									  Snac_Element_NodeCoord( context, element_lI, 7 ),
									  &normal );
					}
					if(nodeElement_I == 2) {
						area = Tetrahedra_SurfaceArea( Snac_Element_NodeCoord( context, element_lI, 4 ),
									       Snac_Element_NodeCoord( context, element_lI, 5 ),
									       Snac_Element_NodeCoord( context, element_lI, 7 ) );
						Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_lI, 4 ),
									  Snac_Element_NodeCoord( context, element_lI, 5 ),
									  Snac_Element_NodeCoord( context, element_lI, 7 ),
									  &normal );
					}
					if(nodeElement_I == 3) {
						area = Tetrahedra_SurfaceArea( Snac_Element_NodeCoord( context, element_lI, 4 ),
									       Snac_Element_NodeCoord( context, element_lI, 5 ),
									       Snac_Element_NodeCoord( context, element_lI, 6 ) );
						Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_lI, 4 ),
									  Snac_Element_NodeCoord( context, element_lI, 5 ),
									  Snac_Element_NodeCoord( context, element_lI, 6 ),
									  &normal );
					}
					(*force)[0] += factor2 * ( element->hydroPressure * area * normal[0] );
					(*force)[1] += factor2 * ( element->hydroPressure * area * normal[1] );
					(*force)[2] += factor2 * ( element->hydroPressure * area * normal[2] );
				}
			}
            if(context->timeStep==1) {
                Ft = (*force)[0]*cos(theta)*cos(phi) + (*force)[1]*cos(theta)*sin(phi) - (*force)[2]*sin(theta);
                if(Ft != 0.0)
                    node->residualFt = Ft;
            }
            sphF[0] = (*force)[0]*sin(theta)*cos(phi) + (*force)[1]*sin(theta)*sin(phi) + (*force)[2]*cos(theta);
            sphF[1] = (*force)[0]*cos(theta)*cos(phi) + (*force)[1]*cos(theta)*sin(phi) - (*force)[2]*sin(theta);
            sphF[2] = -1.0f * (*force)[0]*sin(phi) + (*force)[1]*cos(phi);
/*             sphF[0] -= node->residualFt; */
            sphF[1] = 0.0f;
            (*force)[0] = sphF[0]*sin(theta)*cos(phi) + sphF[1]*cos(theta)*cos(phi) - sphF[2]*sin(phi);
            (*force)[1] = sphF[0]*sin(theta)*sin(phi) + sphF[1]*cos(theta)*sin(phi) + sphF[2]*cos(phi);
            (*force)[2] = sphF[0]*cos(theta) - sphF[1]*sin(theta);
		}
	}
}

double getRadius( Coord coord )
{
	return sqrt( coord[0]*coord[0] + coord[1]*coord[1] + coord[2]*coord[2] );
}

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

void _SnacTractionBC_Force_Apply(
		void*				_context,
		Node_LocalIndex			node_lI,
		double				speedOfSound,
		Mass*				mass,
		Mass*				inertialMass,
		Force*				force,
		Force*				balance )
{
	Snac_Context*			context = (Snac_Context*)_context;
	MeshLayout*				meshLayout = (MeshLayout*)context->meshLayout;
	double					area, normal1[3], normal2[3], normal[3];
	Node_ElementIndex		nodeElement_I, nodeElementCount;
	const double			factor4 = 1.0f / 4.0f;

	/* loop over all the elements surrounding node_dI */
	HexaMD*					decomp = (HexaMD*)meshLayout->decomp;
	Node_GlobalIndex		node_gI = context->mesh->nodeL2G[node_lI];
	IJK						ijk;
	
	double 				 	pressure;
	StressTensor 			stress;
	Force					traction;	

	pressure = 1.0e+08;
	stress[0][0] = 1.0e+08;
	stress[0][1] = 0.0;
	stress[0][2] = 0.0;
	stress[1][0] = 0.0;
	stress[1][1] = 0.0;
	stress[1][2] = 0.0;
	stress[2][0] = 0.0;
	stress[2][1] = 0.0;
	stress[2][2] = 0.0;
	
	RegularMeshUtils_Node_1DTo3D( decomp, node_gI, &ijk[0], &ijk[1], &ijk[2] );

#if 0
	if(ijk[0]==0) { /* for the "left" wall. */
		nodeElementCount = context->mesh->nodeElementCountTbl[node_lI];
		for( nodeElement_I = 0; nodeElement_I < nodeElementCount; nodeElement_I++ ) {
			Element_LocalIndex		element_lI = context->mesh->nodeElementTbl[node_lI][nodeElement_I];

			if( element_lI < context->mesh->elementDomainCount ) {
				/* for the "left" face of the elements on the "left" wall. */
				area = Tetrahedra_SurfaceArea( Snac_Element_NodeCoord( context, element_lI, 0 ),
											   Snac_Element_NodeCoord( context, element_lI, 7 ),
											   Snac_Element_NodeCoord( context, element_lI, 3 ) ) +
					Tetrahedra_SurfaceArea( Snac_Element_NodeCoord( context, element_lI, 0 ),
											Snac_Element_NodeCoord( context, element_lI, 4 ),
											Snac_Element_NodeCoord( context, element_lI, 7 ) );
				Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_lI, 0 ),
										  Snac_Element_NodeCoord( context, element_lI, 7 ),
										  Snac_Element_NodeCoord( context, element_lI, 3 ),
										  &normal1 );
				Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_lI, 0 ),
										  Snac_Element_NodeCoord( context, element_lI, 4 ),
										  Snac_Element_NodeCoord( context, element_lI, 7 ),
										  &normal2 );
#endif
#if 0
	if(ijk[1]==0) { /* for the "bottom" wall. */
		nodeElementCount = context->mesh->nodeElementCountTbl[node_lI];
		for( nodeElement_I = 0; nodeElement_I < nodeElementCount; nodeElement_I++ ) {
			Element_LocalIndex		element_lI = context->mesh->nodeElementTbl[node_lI][nodeElement_I];

			if( element_lI < context->mesh->elementDomainCount ) {
				/* for the "bottom" face of the elements on the "bottom" wall. */
				area = Tetrahedra_SurfaceArea( Snac_Element_NodeCoord( context, element_lI, 0 ),
											   Snac_Element_NodeCoord( context, element_lI, 1 ),
											   Snac_Element_NodeCoord( context, element_lI, 5 ) ) +
					Tetrahedra_SurfaceArea( Snac_Element_NodeCoord( context, element_lI, 0 ),
											Snac_Element_NodeCoord( context, element_lI, 5 ),
											Snac_Element_NodeCoord( context, element_lI, 4 ) );
				Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_lI, 0 ),
										  Snac_Element_NodeCoord( context, element_lI, 1 ),
										  Snac_Element_NodeCoord( context, element_lI, 5 ),
										  &normal1 );
				Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_lI, 0 ),
										  Snac_Element_NodeCoord( context, element_lI, 5 ),
										  Snac_Element_NodeCoord( context, element_lI, 4 ),
										  &normal2 );
#endif
#if 0
	if(ijk[1]==0) { /* for the "bottom" wall. */
		nodeElementCount = context->mesh->nodeElementCountTbl[node_lI];
		for( nodeElement_I = 0; nodeElement_I < nodeElementCount; nodeElement_I++ ) {
			Element_LocalIndex		element_lI = context->mesh->nodeElementTbl[node_lI][nodeElement_I];

			if( element_lI < context->mesh->elementDomainCount ) {
				/* for the "bottom" face of the elements on the "bottom" wall. */
				area = Tetrahedra_SurfaceArea( Snac_Element_NodeCoord( context, element_lI, 0 ),
											   Snac_Element_NodeCoord( context, element_lI, 1 ),
											   Snac_Element_NodeCoord( context, element_lI, 5 ) ) +
					Tetrahedra_SurfaceArea( Snac_Element_NodeCoord( context, element_lI, 0 ),
											Snac_Element_NodeCoord( context, element_lI, 5 ),
											Snac_Element_NodeCoord( context, element_lI, 4 ) );
				Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_lI, 0 ),
										  Snac_Element_NodeCoord( context, element_lI, 1 ),
										  Snac_Element_NodeCoord( context, element_lI, 5 ),
										  &normal1 );
				Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_lI, 0 ),
										  Snac_Element_NodeCoord( context, element_lI, 5 ),
										  Snac_Element_NodeCoord( context, element_lI, 4 ),
										  &normal2 );
#endif
#if 0
	if(ijk[1]==(decomp->nodeGlobal3DCounts[1]-1)) { /* for the "top" wall. */
		nodeElementCount = context->mesh->nodeElementCountTbl[node_lI];
		for( nodeElement_I = 0; nodeElement_I < nodeElementCount; nodeElement_I++ ) {
			Element_LocalIndex		element_lI = context->mesh->nodeElementTbl[node_lI][nodeElement_I];

			if( element_lI < context->mesh->elementDomainCount ) {
				/* for the "top" face of the elements on the "top" wall. */
				area = Tetrahedra_SurfaceArea( Snac_Element_NodeCoord( context, element_lI, 2 ),
											   Snac_Element_NodeCoord( context, element_lI, 3 ),
											   Snac_Element_NodeCoord( context, element_lI, 7 ) ) +
					Tetrahedra_SurfaceArea( Snac_Element_NodeCoord( context, element_lI, 2 ),
											Snac_Element_NodeCoord( context, element_lI, 7 ),
											Snac_Element_NodeCoord( context, element_lI, 6 ) );
				Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_lI, 2 ),
										  Snac_Element_NodeCoord( context, element_lI, 3 ),
										  Snac_Element_NodeCoord( context, element_lI, 7 ),
										  &normal1 );
				Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_lI, 2 ),
										  Snac_Element_NodeCoord( context, element_lI, 7 ),
										  Snac_Element_NodeCoord( context, element_lI, 6 ),
										  &normal2 );
#endif
#if 0
	if(ijk[2]==0)) { /* for the "back" wall. */
		nodeElementCount = context->mesh->nodeElementCountTbl[node_lI];
		for( nodeElement_I = 0; nodeElement_I < nodeElementCount; nodeElement_I++ ) {
			Element_LocalIndex		element_lI = context->mesh->nodeElementTbl[node_lI][nodeElement_I];

			if( element_lI < context->mesh->elementDomainCount ) {
				/* for the "back" face of the elements on the "back" wall. */
				area = Tetrahedra_SurfaceArea( Snac_Element_NodeCoord( context, element_lI, 0 ),
											   Snac_Element_NodeCoord( context, element_lI, 2 ),
											   Snac_Element_NodeCoord( context, element_lI, 1 ) ) +
					Tetrahedra_SurfaceArea( Snac_Element_NodeCoord( context, element_lI, 0 ),
											Snac_Element_NodeCoord( context, element_lI, 3 ),
											Snac_Element_NodeCoord( context, element_lI, 2 ) );
				Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_lI, 0 ),
										  Snac_Element_NodeCoord( context, element_lI, 2 ),
										  Snac_Element_NodeCoord( context, element_lI, 1 ),
										  &normal1 );
				Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_lI, 0 ),
										  Snac_Element_NodeCoord( context, element_lI, 3 ),
										  Snac_Element_NodeCoord( context, element_lI, 2 ),
										  &normal2 );
#endif
#if 0
	if(ijk[2]==(decomp->nodeGlobal3DCounts[2]-1)) { /* for the "front" wall. */
		nodeElementCount = context->mesh->nodeElementCountTbl[node_lI];
		for( nodeElement_I = 0; nodeElement_I < nodeElementCount; nodeElement_I++ ) {
			Element_LocalIndex		element_lI = context->mesh->nodeElementTbl[node_lI][nodeElement_I];

			if( element_lI < context->mesh->elementDomainCount ) {
				/* for the "front" face of the elements on the "front" wall. */
				area = Tetrahedra_SurfaceArea( Snac_Element_NodeCoord( context, element_lI, 4 ),
											   Snac_Element_NodeCoord( context, element_lI, 5 ),
											   Snac_Element_NodeCoord( context, element_lI, 6 ) ) +
					Tetrahedra_SurfaceArea( Snac_Element_NodeCoord( context, element_lI, 4 ),
											Snac_Element_NodeCoord( context, element_lI, 6 ),
											Snac_Element_NodeCoord( context, element_lI, 7 ) );
				Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_lI, 4 ),
										  Snac_Element_NodeCoord( context, element_lI, 5 ),
										  Snac_Element_NodeCoord( context, element_lI, 6 ),
										  &normal1 );
				Tetrahedra_SurfaceNormal( Snac_Element_NodeCoord( context, element_lI, 4 ),
										  Snac_Element_NodeCoord( context, element_lI, 6 ),
										  Snac_Element_NodeCoord( context, element_lI, 7 ),
										  &normal2 );
#endif
	if(ijk[0]==(decomp->nodeGlobal3DCounts[0]-1)) { /* for the "right" wall. */
		nodeElementCount = context->mesh->nodeElementCountTbl[node_lI];
		for( nodeElement_I = 0; nodeElement_I < nodeElementCount; nodeElement_I++ ) {
			Element_LocalIndex		element_lI = context->mesh->nodeElementTbl[node_lI][nodeElement_I];

			if( element_lI < context->mesh->elementDomainCount ) {
				/* for the "right" face of the elements on the "right" wall. */
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
				
				normal[0] = 0.5f * ( normal1[0] + normal2[0] );
				normal[1] = 0.5f * ( normal1[1] + normal2[1] );
				normal[2] = 0.5f * ( normal1[2] + normal2[2] );
				
#if 0
				/* Directly applying tractions [N/m^2]. */
				(*force)[0] += factor4 * area * traction[0];
				(*force)[1] += factor4 * area * traction[1];
				(*force)[2] += factor4 * area * traction[2];
#endif
#if 0
				/* When a pressure is the source. */
				(*force)[0] += factor4 * ( pressure * area * normal[0] );
				(*force)[1] += factor4 * ( pressure * area * normal[1] );
				(*force)[2] += factor4 * ( pressure * area * normal[2] );
#endif
				/* When a full stress tensor is the source. */
				(*force)[0] += factor4 * area *
					( stress[0][0]*normal[0]+stress[0][1]*normal[1]+stress[0][2]*normal[2] );
				(*force)[1] += factor4 * area *
					( stress[1][0]*normal[0]+stress[1][1]*normal[1]+stress[1][2]*normal[2] );
				(*force)[2] += factor4 * area *
					( stress[2][0]*normal[0]+stress[2][1]*normal[1]+stress[2][2]*normal[2] );
				
				(*balance)[0] += fabs( (*force)[0] );
				(*balance)[1] += fabs( (*force)[1] );
				(*balance)[2] += fabs( (*force)[2] );
			}
		}
	}
}

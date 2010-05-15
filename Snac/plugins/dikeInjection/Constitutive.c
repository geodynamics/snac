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
#include "Snac/ViscoPlastic/ViscoPlastic.h"
#include "types.h"
#include "Context.h"
#include "Constitutive.h"
#include "Register.h"
#include <math.h>
#include <string.h>
#include <assert.h>

#ifndef PI
#ifndef M_PIl
#ifndef M_PI
#define PI 3.14159265358979323846
#else
#define PI M_PI
#endif
#else
#define PI M_PIl
#endif
#endif

//#define DEBUG

void SnacDikeInjection_Constitutive( void* _context, Element_LocalIndex element_lI ) {

	Snac_Context			*context = (Snac_Context*)_context;
	SnacDikeInjection_Context*	contextExt = ExtensionManager_Get(
												context->extensionMgr,
												context,
												SnacDikeInjection_ContextHandle );
	Snac_Element			*element = Snac_Element_At( context, element_lI );
	SnacViscoPlastic_Element* viscoplasticElement = ExtensionManager_Get( context->mesh->elementExtensionMgr, element, SnacViscoPlastic_ElementHandle );
	const Snac_Material		*material = &context->materialProperty[element->material_I];
	
	/* make local copies. */
	double startX = contextExt->startX;
	double endX = contextExt->endX;
	double startZ = contextExt->startZ;
	double endZ = contextExt->endZ;
	double dX = endX-startX;
	double dZ = endZ-startZ;
	double elem_dX = 0.0;
	double epsilon_xx = 0.0;
	Tetrahedra_Index	tetra_I;

	/* Some convenience stuffs. */
	MeshLayout*			meshLayout = (MeshLayout*)context->meshLayout;
	HexaMD*				decomp = (HexaMD*)meshLayout->decomp;
	IJK				ijk;
	Element_GlobalIndex		element_gI = _MeshDecomp_Element_LocalToGlobal1D( decomp, element_lI );
	RegularMeshUtils_Element_1DTo3D( decomp, element_gI, &ijk[0], &ijk[1], &ijk[2] );
	
	elem_dX = 0.25*( 
					(Snac_Element_NodeCoord( context, element_lI, 1)[0]-Snac_Element_NodeCoord( context, element_lI, 0)[0]) + 
					(Snac_Element_NodeCoord( context, element_lI, 2)[0]-Snac_Element_NodeCoord( context, element_lI, 3)[0]) + 
					(Snac_Element_NodeCoord( context, element_lI, 5)[0]-Snac_Element_NodeCoord( context, element_lI, 4)[0]) + 
					(Snac_Element_NodeCoord( context, element_lI, 6)[0]-Snac_Element_NodeCoord( context, element_lI, 7)[0]) 
					 );
	fprintf(stderr,"elem_dX=%e dikeWidth=%e\n",elem_dX,contextExt->dikeWidth);
	epsilon_xx = (contextExt->injectionRate*context->dt)/elem_dX;
	/*	epsilon_xx = (contextExt->injectionRate*context->dt)/contextExt->dikeWidth; */
	
	for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
		Coord baryCenter;
		double distance = 0.0;
		double numer = 0.0;
		double denom = 1.0;
		Node_LocalIndex node_lI;
		unsigned int dim;

		/*
		  First decide whther this tet is a part of the dike.
		*/

		/* compute barycenter. */
		for( dim=0; dim < 3; dim++ ) 
			baryCenter[dim] = 0.0;
		for(node_lI=0; node_lI<4; node_lI++) {
			Coord* tetNodeCoord = Snac_Element_NodeCoord( context, element_lI, TetraToNode[tetra_I][node_lI] );
			for( dim=0; dim < 3; dim++ ) 
				baryCenter[dim] += 0.25 * (*tetNodeCoord)[dim];
		}

		/* The following is the general formula for distance from a line to a point. */
		numer = fabs( dX*(startZ-baryCenter[2])-(startX-baryCenter[0])*dZ );
		denom = sqrt( dX*dX + dZ*dZ );
		assert( denom > 0.0 );
		distance = numer/denom;

		/* 
		   If part of the dike, adjust stresses. 

		   Note that although parameters can define a ridge with an arbitrary orientation,
		   the following stress mods assume ridges are parallel with z-axis.
		   One can implement tensor rotation when necessary.
		 */
		if( (distance <= contextExt->dikeWidth) && (baryCenter[1] >= contextExt->dikeDepth) ) {
			StressTensor*		stress = &element->tetra[tetra_I].stress;

/* 			fprintf(stderr,"el=%d (%d %d %d) tet=%d (%e %e %e) distance=%e startX=%e width=%e\n", */
/* 					element_lI,ijk[0],ijk[1],ijk[2],tetra_I, */
/* 					baryCenter[0], baryCenter[1], baryCenter[2], */
/* 					distance,startX,contextExt->dikeWidth); */
			(*stress)[0][0] -= (material->lambda + 2.0f * material->mu) * epsilon_xx;
			(*stress)[1][1] -= material->lambda * epsilon_xx;
			(*stress)[2][2] -= material->lambda * epsilon_xx;
			
			/* Also assuming viscoplastic rheology is used. */
			viscoplasticElement->plasticStrain[tetra_I] = 0.0;
		}
	}
}

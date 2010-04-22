/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003,
**	Pururav Thoutireddy,
**	Californian Institute of Technology, 1200 East California Boulevard, Pasadena, California, 91125, USA.
**	110 Victoria Street, Melbourne, Victoria, 3053, Australia.
**	University of Texas, 1 University Station, Austin, Texas, 78712, USA.
**
** Authors:
**	Pururav Thoutireddy, Staff Scientist, Caltech
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
** $Id: RemeshElements.c 3259 2006-11-09 20:06:31Z EunseoChoi $
*/
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "EntryPoint.h"
#include "Mesh.h"
#include "Context.h"
#include "Remesh.h"
#include "Register.h"
#include "Utils.h"

#include <string.h>
#include <math.h>
#include <assert.h>
#include <float.h>


void _SnacRemesher_InterpolateElements( void* _context ) {
	Snac_Context*			context = (Snac_Context*)_context;
	SnacRemesher_Context*	contextExt = ExtensionManager_Get( context->extensionMgr,
															   context,
															   SnacRemesher_ContextHandle );
	Mesh*				mesh = context->mesh;
	Element_LocalIndex	newElt_i;
	unsigned			tet_i;
	
	/* Loop over elements. */
	for( newElt_i = 0; newElt_i < mesh->elementLocalCount; newElt_i++ ) 
		/* Loop over tets. */
		for( tet_i = 0; tet_i < Tetrahedra_Count; tet_i++ ) 
			SnacRemesher_InterpolateElement( context, contextExt, newElt_i, tet_i );
}

void _SnacRemesher_UpdateElements( void* _context ) {

	Snac_Context*			context = (Snac_Context*)_context;
	Element_LocalIndex	element_lI;

	/* Update all the elements, and in the process work out this processor's minLengthScale */
	for( element_lI = 0; element_lI < context->mesh->elementLocalCount; element_lI++ ) {
		double elementMinLengthScale;
		Tetrahedra_Index	tetra_I;
		Snac_Element*	element = Snac_Element_At( context, element_lI );
 		float materialInd = 0.0; 
/* 		Material_Index materialInd = 0;  */

		for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
 			materialInd += (1.0f*element->tetra[tetra_I].material_I)/((float)Tetrahedra_Count); 
/* 			materialInd += (1.0*element->tetra[tetra_I].material_I+0.1); */
		}
		/* Ad-hoc rounding-off. Notet that 0.6 is not a typo. 
		   There is a fundamental issue with mapping integer field during remeshing and 
		   the usual roundoff causes too severe artificial diffusin. 
		   0.6 is a conservative choice (than 0.5) to reduce the artefact, but it is not guaranteed to work always. */
		element->material_I = ((materialInd>0.6)?1:0);
/* 		element->material_I = (Material_Index)(materialInd/10+0.5); */
#if 0
		KeyCall( context, context->updateElementK, Snac_UpdateElementMomentum_CallCast* )
			( KeyHandle(context,context->updateElementK),
			  context,
			  element_lI,
			  &elementMinLengthScale );
		if( elementMinLengthScale < context->minLengthScale ) {
			context->minLengthScale = elementMinLengthScale;
		}
#endif
	}
}


/*
** Interpolate an element's tetrahedra.
*/
void _SnacRemesher_InterpolateElement( void* _context, 
									   Element_LocalIndex	dstEltInd, 
									   Tetrahedra_Index	dstTetInd ) 
{
	Snac_Context*	context = (Snac_Context*)_context;
	Snac_Element*	dstElt = Snac_Element_At( context, dstEltInd );
	Index i,j;
	double strain[6];
	double stress[6];
	float materialInd;
	double density;

	/* Compute averages of recovered fields for this tet.*/
	for(j=0;j<6;j++) {
		strain[j] = 0.0;
		stress[j] = 0.0;
		for(i=0;i<Tetrahedra_Point_Count;i++) {
			Index dstNodeNum = Element_Node_I(context, dstEltInd, TetraToNode[dstTetInd][i]);
 			Snac_Node* dstNode = Snac_Node_At( context, dstNodeNum );
 			strain[j] += 0.25f * dstNode->strainSPR[j];
			stress[j] += 0.25f * dstNode->stressSPR[j];
			/* The next two lines can possibly replace the above codes, but need testing. */
			/* strain[j] += (0.25f*Snac_Element_Node_P(context, dstEltInd, TetraToNode[dstTetInd][i])->strainSPR[j]); */
			/* stress[j] += (0.25f*Snac_Element_Node_P(context, dstEltInd, TetraToNode[dstTetInd][i])->stressSPR[j]); */
		}
	}
	materialInd = 0.0;
	for(i=0;i<Tetrahedra_Point_Count;i++) {
		Index dstNodeNum = Element_Node_I(context, dstEltInd, TetraToNode[dstTetInd][i]);
		Snac_Node* dstNode = Snac_Node_At( context, dstNodeNum );
		materialInd += 0.25f*(dstNode->material_ISPR);
	}

	density = 0.0;
	for(i=0;i<Tetrahedra_Point_Count;i++) {
		Index dstNodeNum = Element_Node_I(context, dstEltInd, TetraToNode[dstTetInd][i]);
		Snac_Node* dstNode = Snac_Node_At( context, dstNodeNum );
		density += 0.25f*(dstNode->densitySPR);
	}

	/* Assign the computed averages to the tets. */
	dstElt->tetra[dstTetInd].strain[0][0] = strain[0];
	dstElt->tetra[dstTetInd].strain[1][1] = strain[1];
	dstElt->tetra[dstTetInd].strain[2][2] = strain[2];
	dstElt->tetra[dstTetInd].strain[0][1] = strain[3];
	dstElt->tetra[dstTetInd].strain[0][2] = strain[4];
	dstElt->tetra[dstTetInd].strain[1][2] = strain[5];
	dstElt->tetra[dstTetInd].strain[1][0] = strain[3];
	dstElt->tetra[dstTetInd].strain[2][0] = strain[4];
	dstElt->tetra[dstTetInd].strain[2][1] = strain[5];

	dstElt->tetra[dstTetInd].stress[0][0] = stress[0];
	dstElt->tetra[dstTetInd].stress[1][1] = stress[1];
	dstElt->tetra[dstTetInd].stress[2][2] = stress[2];
	dstElt->tetra[dstTetInd].stress[0][1] = stress[3];
	dstElt->tetra[dstTetInd].stress[0][2] = stress[4];
	dstElt->tetra[dstTetInd].stress[1][2] = stress[5];
	dstElt->tetra[dstTetInd].stress[1][0] = stress[3];
	dstElt->tetra[dstTetInd].stress[2][0] = stress[4];
	dstElt->tetra[dstTetInd].stress[2][1] = stress[5];

	/* There is a fundamental issue with mapping integer field for multiple phases during remeshing. 
	   Note that the following roundoff is not guaranteed to work always.  */
  	dstElt->tetra[dstTetInd].material_I = ((log10(materialInd)>0.5)?1:0);

 	dstElt->tetra[dstTetInd].density = density; 

#if 0
 	if( dstElt->material_I == 2 ) 
 		fprintf(stderr,"el=%d tet=%d elmatI=%d matI=%e (%d) (%e %e %e %e)\n",dstEltInd,dstTetInd,dstElt->material_I,materialInd, 
 				dstElt->tetra[dstTetInd].material_I,
				Snac_Element_Node_P(context, dstEltInd, TetraToNode[dstTetInd][0])->material_ISPR,
				Snac_Element_Node_P(context, dstEltInd, TetraToNode[dstTetInd][1])->material_ISPR,
				Snac_Element_Node_P(context, dstEltInd, TetraToNode[dstTetInd][2])->material_ISPR,
				Snac_Element_Node_P(context, dstEltInd, TetraToNode[dstTetInd][3])->material_ISPR
				); 
#endif
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003,
**	Pururav Thoutireddy, 
**	Californian Institute of Technology, 1200 East California Boulevard, Pasadena, California, 91125, USA.
**      110 Victoria Street, Melbourne, Victoria, 3053, Australia.
**	University of Texas, 1 University Station, Austin, Texas, 78712, USA.
**
** Authors:
**	Pururav Thoutireddy, Staff Scientist, Caltech
**      Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
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
** $Id: Remesh.c 3250 2006-10-23 06:15:18Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "Snac/Remesher/Remesher.h"
#include "Snac/Remesher/Mesh.h"
#include "types.h"
#include "Element.h"
#include "Remesh.h"
#include "Register.h"

void _SnacViscoPlastic_InterpolateElement(  void*				 	_context, 
											Element_LocalIndex	 	dstEltInd, 
											Tetrahedra_Index	 	dstTetInd, 
											SnacRemesher_Element*	dstElements, 
											Element_DomainIndex 	srcEltInd, 
											Tetrahedra_Index		srcTetInd )
{
	Snac_Context* 			context = (Snac_Context*)_context;
	Mesh*					mesh = context->mesh;
	SnacRemesher_Mesh*		meshExt = ExtensionManager_Get( context->meshExtensionMgr,
															mesh, 
															SnacRemesher_MeshHandle );
	HexaMD*					decomp = (HexaMD*)mesh->layout->decomp;
	SnacRemesher_Element*	dstElt = &dstElements[dstEltInd];
	Element_DomainIndex 	eltdI[8],eldI,eldJ,eldK;
	Index 					coef_I;
	Element_DomainIndex		neldI =  decomp->elementDomain3DCounts[0];
	Element_DomainIndex		neldJ =  decomp->elementDomain3DCounts[1];
	Element_DomainIndex		neldK =  decomp->elementDomain3DCounts[2];
	enum					{ threeD, xy, undefined } geomType;
	
#ifdef DEBUG
	printf( "element_lI: %u, fromElement_lI: %u\n", dstElementInd, srcElementInd );
#endif

	geomType = undefined;
	if( neldI>1 && neldJ>1 && neldK>1 ) geomType = threeD;
	else if( neldI>1 && neldJ>1 && neldK==1 ) geomType = xy;
	
	switch(geomType) {
	case undefined:
		Journal_Firewall( 0, context->snacError, "Remeshing is currently allowed only for xy or threeD!!\n");
	case threeD:
		/* Decompose srcEltInd into ijk indexes. */
		eldI = (srcEltInd % neldI);
		eldJ = (((srcEltInd-eldI)/neldI) % neldJ);
		eldK = ((srcEltInd-eldI-eldJ*neldI)/(neldI*neldJ));

		/* Eight-node hex defined on the old barycenter grid. */
		eltdI[0] = eldI     + eldJ*neldI     + eldK*neldI*neldJ;
		eltdI[1] = (eldI+1) + eldJ*neldI     + eldK*neldI*neldJ;
		eltdI[2] = (eldI+1) + (eldJ+1)*neldI + eldK*neldI*neldJ;
		eltdI[3] = eldI     + (eldJ+1)*neldI + eldK*neldI*neldJ;
		eltdI[4] = eldI     + eldJ*neldI     + (eldK+1)*neldI*neldJ;
		eltdI[5] = (eldI+1) + eldJ*neldI     + (eldK+1)*neldI*neldJ;
		eltdI[6] = (eldI+1) + (eldJ+1)*neldI + (eldK+1)*neldI*neldJ;
		eltdI[7] = eldI     + (eldJ+1)*neldI + (eldK+1)*neldI*neldJ;

		dstElt->plasticStrain[dstTetInd] = 0.0;
		for(coef_I=0;coef_I<4;coef_I++) {
			/* The actual src elements are the four apexes of a tet (srcTetInd) in the old barycenter grid. */
			Snac_Element* 				srcElt = Snac_Element_At( context, 
																  meshExt->orderedToDomain[eltdI[TetraToNode[srcTetInd][coef_I]]] );
			SnacViscoPlastic_Element*	srcEltExt = ExtensionManager_Get(
																		 context->mesh->elementExtensionMgr,
																		 srcElt,
																		 SnacViscoPlastic_ElementHandle );
			/* Weights are associated only with destination element but not on the tet level. 
			   So, "dstTetInd" is used in both source and destination terms. */
			dstElt->plasticStrain[dstTetInd] += meshExt->barcord[dstEltInd].L[coef_I]*srcEltExt->plasticStrain[dstTetInd];
		}
		break;
	case xy:
		/* Decompose srcEltInd into ijk indexes. */
		eldI = (srcEltInd % neldI);
		eldJ = (((srcEltInd-eldI)/neldI) % neldJ);
		
		/* Eight-node hex defined on the old barycenter grid. */
		eltdI[0] = eldI     + eldJ*neldI;     
		eltdI[1] = (eldI+1) + eldJ*neldI;
		eltdI[2] = (eldI+1) + (eldJ+1)*neldI;
		eltdI[3] = eldI     + (eldJ+1)*neldI;
		
		dstElt->plasticStrain[dstTetInd] = 0.0;
		for(coef_I=0;coef_I<3;coef_I++) {
			/* The actual src elements are the four apexes of a tet (srcTetInd) in the old barycenter grid. */
			Snac_Element* 			srcElt = Snac_Element_At( context,
															  meshExt->orderedToDomain[eltdI[TriToNode[srcTetInd][coef_I]]] );
			SnacViscoPlastic_Element*	srcEltExt = ExtensionManager_Get(
																	 context->mesh->elementExtensionMgr,
																	 srcElt,
																	 SnacViscoPlastic_ElementHandle );
			/* Weights are associated only with destination element but not on the tet level.
			   So, "dstTetInd" is used in both source and destination terms. */
			dstElt->plasticStrain[dstTetInd] += meshExt->barcord[dstEltInd].L[coef_I]*srcEltExt->plasticStrain[dstTetInd];
		}
		break;
	}
}

/*
  Copy interpolated values stored in newElement array 
  back to the original element array. 
*/
void _SnacViscoPlastic_CopyElement( void* 					_context, 
									Element_LocalIndex		eltInd, 
									Tetrahedra_Index		tetInd, 
									SnacRemesher_Element*	srcEltArray )
{

	Snac_Context*				context = (Snac_Context*)_context;
	SnacRemesher_Element* 		srcElt = &srcEltArray[eltInd];
	Snac_Element* 				dstElt = Snac_Element_At( context, eltInd );
	SnacViscoPlastic_Element*	dstEltExt = ExtensionManager_Get(
									context->mesh->elementExtensionMgr,
									dstElt,
									SnacViscoPlastic_ElementHandle );

	dstEltExt->plasticStrain[tetInd] = srcElt->plasticStrain[tetInd];

}

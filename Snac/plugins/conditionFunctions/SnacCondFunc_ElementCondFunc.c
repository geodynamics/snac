/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003,
**      Steve Quenette, 110 Victoria Street, Melbourne, Victoria, 3053, Australia.
**      Californian Institute of Technology, 1200 East California Boulevard, Pasadena, California, 91125, USA.
**      University of Texas, 1 University Station, Austin, Texas, 78712, USA.
**
** Authors:
**      Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
**      Stevan M. Quenette, Visitor in Geophysics, Caltech.
**      Luc Lavier, Research Scientist, The University of Texas. (luc@utig.ug.utexas.edu)
**      Luc Lavier, Research Scientist, Caltech.
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
** $Id: SnacCondFunc_InitialCondition.c 3192 2006-06-14 03:07:59Z LaetitiaLePourhiet $
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "SnacCondFunc_ElementCondFunc.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

/*
  Assuming this cond func is called in the Element IC 
  to assign spatially varying amaterial_I. 

  Make this a template for other situations.
*/
void _SnacCondFunc_AssignPhaseID( Index element_lI, Variable_Index var_I, void* _context, void* result ){
	Snac_Context*		context = (Snac_Context*)_context;
	Snac_Element*		element = Snac_Element_At( context, element_lI );
	Material_Index*		material_I = (Material_Index*)result;
	
	/* Stuff for convenience */
	Mesh*				mesh = context->mesh;
	MeshLayout*			layout = (MeshLayout*)mesh->layout;
	HexaMD*				decomp = (HexaMD*)layout->decomp;
	IJK					ijk;
	Element_GlobalIndex	element_gI = _MeshDecomp_Element_LocalToGlobal1D( decomp, element_lI );
	Index				oneThird = (unsigned int)((11.0/30.0)*decomp->elementGlobal3DCounts[0]);
	Index				twoThirds = (unsigned int)((19.0/30.0)*decomp->elementGlobal3DCounts[0]);

	RegularMeshUtils_Element_1DTo3D( decomp, element_gI, &ijk[0], &ijk[1], &ijk[2] );

	fprintf(stderr,"Running %s\n",__func__);
	
	if( ijk[0] < oneThird )
		(*material_I) = 0;
	else if( ijk[0] >= oneThird && ijk[0] <= twoThirds )
		(*material_I) = 1;
	else
		(*material_I) = 0;
}

void _SnacCondFunc_DeadSea( Index element_lI, Variable_Index var_I, void* _context, void* result ){
	Snac_Context*		context = (Snac_Context*)_context;
	Snac_Element*		element = Snac_Element_At( context, element_lI );
	Material_Index*		material_I = (Material_Index*)result;
	
	/* Stuff for convenience */
	Mesh*				mesh = context->mesh;
	MeshLayout*			layout = (MeshLayout*)mesh->layout;
	HexaMD*				decomp = (HexaMD*)layout->decomp;
	IJK				ijk;
    	Element_GlobalIndex		global_I_range = decomp->elementGlobal3DCounts[0];
    	Element_GlobalIndex		global_K_range = decomp->elementGlobal3DCounts[2];

	Element_GlobalIndex		element_gI = _MeshDecomp_Element_LocalToGlobal1D( decomp, element_lI );
        unsigned int			matID=0, tetra_I;	

	RegularMeshUtils_Element_1DTo3D( decomp, element_gI, &ijk[0], &ijk[1], &ijk[2] );

	if( (ijk[0] < (global_I_range-ijk[2])) &&
		(ijk[2] < global_K_range/4) )
		matID=1;
	if((ijk[0]==(global_I_range-ijk[2]-1)) &&
		(ijk[2] < (global_K_range/4)) )
		matID=2;
	if( (ijk[0]==global_I_range/2) &&
		(ijk[2]>=global_K_range/4) )
		matID=2;
	if( ((ijk[0]>=global_I_range/2)&&(ijk[0]<=global_I_range*3/4))
		&&
		(ijk[2]==global_K_range/4) )
		matID=2;

	(*material_I)=matID;

	for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
		element->tetra[tetra_I].material_I=matID;
	}
}

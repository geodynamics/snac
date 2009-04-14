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
** $Id: InitialConditions.c 2150 2004-10-07 22:18:03Z EunseoChoi $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "InitialConditions.h"
#include "Register.h"
#include "Snac/ViscoPlastic/ViscoPlastic.h"
#include "Snac/Plastic/Plastic.h"

void _SnacVPSeeds_InitialConditions( void* _context, void* data ) {
	Snac_Context*			    context = (Snac_Context*)_context;
	Mesh*				        mesh = context->mesh;
	MeshLayout*			        layout = (MeshLayout*)mesh->layout;
	HexaMD*				        decomp = (HexaMD*)layout->decomp;
	Element_LocalIndex		    element_lI;
    Element_GlobalIndex         element_gI;
    int                         index_I,index_J,global_I_range,global_J_range,global_K_range;
    IJK				            ijk;
    Snac_Element*			    element;
    const Snac_Material*	    material;
    Tetrahedra_Index		    tetra_I;
    SnacViscoPlastic_Element*	viscoplasticElement;
    float                       rand_num;
    int                         NUM_SEED;
    int                         incZ = 0;

	int			restart = 0;
	Dictionary_Entry_Value* pluginsList;
	Dictionary_Entry_Value* plugin;

	if( context->restartTimestep > 0 )
		return;

#ifdef DEBUG
    printf( "In: %s\n", __func__ );
#endif

    srand( (unsigned int)time(NULL) );
    global_I_range = decomp->elementGlobal3DCounts[0]/4;
    global_J_range = 1; //decomp->elementGlobal3DCounts[1];
    global_K_range = 1; //decomp->elementGlobal3DCounts[2]; //10;
    NUM_SEED = decomp->elementGlobal3DCounts[2] * global_I_range * 0.02;

    for(index_I = 0; index_I < NUM_SEED; index_I++) {
        rand_num = rand()/(RAND_MAX+1.0);
        ijk[2] = decomp->elementGlobal3DCounts[2]*rand_num;
        rand_num = rand()/(RAND_MAX+1.0);
        ijk[0] = global_I_range * rand_num;
        ijk[0] = decomp->elementGlobal3DCounts[0]/2 - (ijk[0]-global_I_range/2);

        incZ = 0;
        //ijk[2] -= global_K_range/2;
        while( incZ < global_K_range && ijk[2] >=0 && ijk[2] < decomp->elementGlobal3DCounts[2] ) {
            ijk[1] = decomp->elementGlobal3DCounts[1]-1;
            for(index_J = 0; index_J < global_J_range; index_J++) {
                element_gI = ijk[0] + decomp->elementGlobal3DCounts[0]*ijk[1] + decomp->elementGlobal3DCounts[0]*decomp->elementGlobal3DCounts[1]*ijk[2];
                element_lI = Mesh_ElementMapGlobalToLocal( mesh, element_gI );

                if(element_lI < mesh->elementLocalCount) {

                    element = Snac_Element_At( context, element_lI );
                    material = &context->materialProperty[element->material_I];
                    viscoplasticElement = ExtensionManager_Get(
                                                               mesh->elementExtensionMgr,
                                                               element,
                                                               SnacViscoPlastic_ElementHandle );
                    for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
                        viscoplasticElement->plasticStrain[tetra_I] = 1.1 * material->plstrain[1];

                        Journal_Printf(
                                       context->snacVerbose,
                                       "timeStep=%d ijk=%d %d %d plasticE=%e\n",
                                       context->timeStep,
                                       ijk[0],
                                       ijk[1],
                                       ijk[2],
                                       viscoplasticElement->plasticStrain[tetra_I] );
                    }
                    //fprintf(stderr,"me=%d seeds(%d/%d) ijk=%d %d %d\n",context->rank,index_I,NUM_SEED,ijk[0],ijk[1],ijk[2]);
                }
                ijk[1]--;
            }
            ijk[2]++;
            incZ++;
        }
    }
}

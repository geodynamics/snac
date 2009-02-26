s/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
**  Mods by:
**           Colin Stark, Doherty Research Scientist, Lamont-Doherty Earth Observatory (cstark@ldeo.columbia.edu)
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
** $Id: InitialConditions.c  $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <math.h>

#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "Context.h"
#include "Register.h"
#include "InitialConditions.h"
#include "Snac/Plastic/Plastic.h"


#ifndef PI
	#ifndef M_PIl
		#ifndef M_PI
			#define PI 3.1415927
		#else
			#define PI M_PI
		#endif
	#else
		#define PI M_PIl
	#endif
#endif

#define min(a,b) ((a)<(b) ? (a):(b))

#ifndef PATH_MAX
	#define PATH_MAX 1024
#endif


//#define DEBUG

void _SnacHillSlope_InitialConditions( void* _context, void* data ) {
	Snac_Context*			context = (Snac_Context*)_context;
	SnacHillSlope_Context*		contextExt = ExtensionManager_Get(
							context->extensionMgr,
							context,
							SnacHillSlope_ContextHandle );
	Mesh*				mesh = context->mesh;
	MeshLayout*			layout = (MeshLayout*)mesh->layout;
	HexaMD*				decomp = (HexaMD*)layout->decomp;
	BlockGeometry*			geometry = (BlockGeometry*)layout->elementLayout->geometry;
	Node_GlobalIndex		node_g;
	Node_GlobalIndex		node_gI;
	Node_GlobalIndex		node_gJ;
	double                          new_x[decomp->nodeGlobal3DCounts[0]];
	const double                    reg_dx = (geometry->max[0]-geometry->min[0])/(decomp->nodeGlobal3DCounts[0] - 1);
	double                          new_y[decomp->nodeGlobal3DCounts[1]];
	const double                    reg_dy = (geometry->max[1]-geometry->min[1])/(decomp->nodeGlobal3DCounts[1] - 1);
	double                          new_xMax;
	double                          new_xMin;
	double                          new_yMax;
	double                          new_yMin;
	double                          slope = contextExt->slopeAngle * M_PI/180.0;
/* 	unsigned int                    rngSeed = contextExt->rngSeed; */
/* 	double                          fractionWeakPoints = contextExt->fractionWeakPoints; */

	int				restart = 0;
	Dictionary_Entry_Value* 	pluginsList;
	Dictionary_Entry_Value* 	plugin;

	Element_LocalIndex		element_lI;
	Element_GlobalIndex       	element_gI;
	int                   	        index_I,index_J,global_I_range,global_J_range,global_K_range;
	IJK				ijk;
	Snac_Element*			element;
	const Snac_Material*	    	material;
	Tetrahedra_Index		tetra_I;
	SnacPlastic_Element*		plasticElement;
	float                       	rand_num;
	int                         	NUM_SEED;
	int                         	incZ = 0;

	pluginsList = PluginsManager_GetPluginsList( context->dictionary );
	if (pluginsList) {
		plugin = Dictionary_Entry_Value_GetFirstElement(pluginsList);
		while ( plugin ) {
			if ( 0 == strcmp( Dictionary_Entry_Value_AsString( plugin ),
					  "SnacRestart" ) ) {
				restart = 1;
				break;
			}
			plugin = plugin->next;
		}
	}
	if( restart )
		return;


	#ifdef DEBUG
		printf( "In: %s\n", __func__ );
		fprintf(stderr, "Slope angle = %g degrees\n", slope/(M_PI/180.0));
	#endif

	/*  Report HillSlope plugin variables picked up (?) from xml parameter file */
	Journal_Printf( context->snacInfo, "\n\tSlope angle = %g degrees\n", contextExt->slopeAngle );
/* 	Journal_Printf( context->snacInfo, "\tRNG seed = %u\n", contextExt->rngSeed ); */
/* 	Journal_Printf( context->snacInfo, "\tFraction of weak points = %g\n\n", contextExt->fractionWeakPoints ); */


	for( node_gI = 0; node_gI < decomp->nodeGlobal3DCounts[0]; node_gI++ )
		new_x[node_gI] = geometry->min[0] + node_gI*reg_dx;
	
	for( node_gJ = 0; node_gJ < decomp->nodeGlobal3DCounts[1]; node_gJ++ )
		new_y[node_gJ] = geometry->min[1] + node_gJ*reg_dy;
	
	/*
	for( node_gJ = 0; node_gJ < decomp->nodeGlobal3DCounts[1]; node_gJ++ ) {
		double                          reg_y;
		if(node_gJ==0)
			new_y[node_gJ] = geometry->min[1];
		else if(node_gJ>0) {
			reg_y = geometry->min[1] + node_gJ*reg_dy;
			new_y[node_gJ] = new_y[node_gJ-1] + reg_dy/(5.0/PI*atan((reg_y-ym)/Ly)+3.0);
		}
	}
	*/
	

	/*
	 *  Assume highest point in x,y is at the last mesh node
	 */
	new_xMin = new_x[0];
	new_xMax = new_x[(decomp->nodeGlobal3DCounts[0] - 1)];
	new_yMin = new_y[0];
	new_yMax = new_y[(decomp->nodeGlobal3DCounts[1] - 1)];  /*+ tan(mesh_slope)*geometry->max[0]; */
	/*
	 *  Loop across all nodes in mesh
	 *    - total of x*y*z nodes is given by context->mesh->nodeGlobalCount+1
	 */
	for( node_g = 0; node_g < context->mesh->nodeGlobalCount; node_g++ ) {
		Node_LocalIndex			node_l = _MeshDecomp_Node_GlobalToLocal1D( decomp, node_g );
		Index				i_g;
		Index				j_g;
		Index				k_g;
		Coord*				coord = 0;
		Coord				tmpCoord;

		/* If a local node, directly change the node coordinates and initial tpr, else use a temporary location */
		if( node_l < context->mesh->nodeLocalCount ) { /* a local node */
			coord = Snac_NodeCoord_P( context, node_l );
		}
		else {
			coord = &tmpCoord;
		}

		RegularMeshUtils_Node_1DTo3D( decomp, node_g, &i_g, &j_g, &k_g );

		(*coord)[0] = new_x[i_g];
		(*coord)[0] *= geometry->max[0]/new_xMax;

		(*coord)[1] = new_y[j_g] + tan(slope)*(*coord)[0];
		(*coord)[1] *= geometry->max[1]/new_yMax;

		//fprintf(stderr,"i_g=%d coord=%e newX=%e min=%e max=%e\n",
		//	i_g,(*coord)[0],new_x[i_g],geometry->min[0],geometry->max[0]);

		#ifdef DEBUG
			printf( "\tnode_l: %2u, node_g: %2u, i: %2u, j: %2u, k: %2u, ",
				node_l,
				node_g,
				i_g,
				j_g,
				k_g );
			printf( "x: %12g, y: %12g, z: %12g\n", (*coord)[0], (*coord)[1], (*coord)[2] );
		#endif
	}

/* 	srand( rngSeed ); */
	srand( 1 );
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
			plasticElement = ExtensionManager_Get(
							      mesh->elementExtensionMgr,
							      element,
							      SnacPlastic_ElementHandle );
			for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
			    plasticElement->plasticStrain[tetra_I] = 1.1 * material->plstrain[1];
			    
			    Journal_Printf(
					   context->snacInfo,
					   "timeStep=%d ijkt=%d %d %d %d  plasticE=%e\n",
					   context->timeStep,
					   ijk[0],
					   ijk[1],
					   ijk[2],
					   tetra_I,
					   plasticElement->plasticStrain[tetra_I] );
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

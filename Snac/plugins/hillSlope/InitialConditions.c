/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003,
**	Steve Quenette, 110 Victoria Street, Melbourne, Victoria, 3053, Australia.
**	Californian Institute of Technology, 1200 East California Boulevard, Pasadena, California, 91125, USA.
**	University of Texas, 1 University Station, Austin, Texas, 78712, USA.
**
** Authors:
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
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "Context.h"
#include "Register.h"
#include "InitialConditions.h"
#include "Snac/Plastic/Plastic.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
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
#define min(a,b) ((a)<(b) ? (a):(b))
#ifndef PATH_MAX
	#define PATH_MAX 1024
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif


#define DEBUG

void _SnacHillSlope_InitialConditions( void* _context, void* data ) {
    Snac_Context		*context = (Snac_Context*)_context;
    SnacHillSlope_Context	*contextExt = ExtensionManager_Get(context->extensionMgr,
								   context,
								   SnacHillSlope_ContextHandle );
    Mesh			*mesh = context->mesh;
    MeshLayout			*layout = (MeshLayout*)mesh->layout;
    HexaMD			*decomp = (HexaMD*)layout->decomp;
    BlockGeometry		*geometry = (BlockGeometry*)layout->elementLayout->geometry;
    Node_GlobalIndex		node_g, node_gI, node_gJ;

    const int			full_I_node_range=decomp->nodeGlobal3DCounts[0];
    const int			full_J_node_range=decomp->nodeGlobal3DCounts[1];

    double			new_x[full_I_node_range], new_y[full_J_node_range];
    double			reg_dx, reg_dy, new_xMax, new_xMin, new_xHalf, midOffset, old_yMax, new_yMax, new_yMin;

    double			slopeAngle = contextExt->slopeAngle * M_PI/180.0;
    double			resolveHeight = geometry->max[1]-contextExt->resolveDepth;

    double			xLeftFlatRamp = (contextExt->leftFlatFraction<0.0 ? 0.0 : 
					     (contextExt->leftFlatFraction>1.0 ? 1.0 : contextExt->leftFlatFraction));
    double			xRightRampFlat = (contextExt->rightFlatFraction<0.0 ? 
					     1.0-contextExt->leftFlatFraction : (contextExt->rightFlatFraction>1.0 ? 
										  1.0 : contextExt->rightFlatFraction));
    const double		smoothFactor = contextExt->rampFlatSmoothFactor;

    int				restart = FALSE;
    Dictionary_Entry_Value 	*pluginsList, *plugin;
#ifdef DEBUG
    int imax=0;
#endif

    pluginsList = PluginsManager_GetPluginsList( context->dictionary );
    if (pluginsList) {
	plugin = Dictionary_Entry_Value_GetFirstElement(pluginsList);
	while ( plugin ) {
	    if ( 0 == strcmp( Dictionary_Entry_Value_AsString( plugin ),
			      "SnacRestart" ) ) {
		restart = TRUE;
		break;
	    }
	    plugin = plugin->next;
	}
    }
    if( restart ) {
	fprintf(stderr, "Restarting: thus bailing from hillSlope/InitialConditions.c to avoid overwriting the mesh geometry\n");
	return;
    }

    
#ifdef DEBUG
    printf( "In: %s\n", __func__ );
#endif
    //    fprintf(stderr, "Slope angle = %g degrees\n", slopeAngle/(M_PI/180.0));

    /*  Report HillSlope plugin variables picked up (?) from xml parameter file */
    Journal_Printf( context->snacInfo, "\nSlope angle = %g degrees\n", slopeAngle/(M_PI/180.0) );

    reg_dx = (geometry->max[0]-geometry->min[0])/(double)(full_I_node_range-1);
    reg_dy = (geometry->max[1]-geometry->min[1])/(double)(full_J_node_range-1);
    /*
     *  Decide at what height (prior to shearing) to switch from rombhoid to parallelopiped geometry, 
     *     i.e., at what depth to resolve using "regular" elements parallel to the surface
     *           versus "irregular" elements deformed to fit to a horizontal bottom
     * If contextExt->resolveDepth is negative, resolveHeight is set to zero and the whole domain is "regular"
     */
    if(contextExt->resolveDepth<0.0 || resolveHeight<0.0) {
	resolveHeight = 0.0;
    } else if(resolveHeight>geometry->max[1]) {
	resolveHeight = geometry->max[1];
    }
    for( node_gI = 0; node_gI < full_I_node_range; node_gI++ )
	new_x[node_gI] = geometry->min[0] + node_gI*reg_dx;
	
    for( node_gJ = 0; node_gJ <= (full_J_node_range-1); node_gJ++ )
	new_y[node_gJ] = geometry->min[1] + node_gJ*reg_dy;
/*     if((full_J_node_range-1)<4) { */
/* 	for( node_gJ = 0; node_gJ <= (full_J_node_range-1); node_gJ++ ) */
/* 	    new_y[node_gJ] = geometry->min[1] + node_gJ*reg_dy; */
/*     } else { */
/* 	for( node_gJ = 0; node_gJ <= (full_J_node_range-1)/4; node_gJ++ ) */
/* 	    new_y[node_gJ] = geometry->min[1] + node_gJ*reg_dy*(stretchFactor/(1.0+stretchFactor)); */
/* 	for(            ; node_gJ <= (full_J_node_range-1); node_gJ++ ) */
/* 	    new_y[node_gJ] = geometry->min[1] + ((full_J_node_range-1)/4)*reg_dy*(stretchFactor/(1.0+stretchFactor))  */
/* 		+ (node_gJ-((full_J_node_range-1)/4))*reg_dy*(1.0/(1.0+stretchFactor)); */
/*     } */
    /*
     *  Assume highest point in x,y is at the last mesh node
     */
    new_xMin = new_x[0];
    new_xMax = new_x[full_I_node_range-1];
    new_xHalf = new_xMin+(new_xMax-new_xMin)/2.0;
    new_yMin = new_y[0];
    old_yMax = new_yMax = new_y[full_J_node_range-1];  /*+ tan(mesh_slope)*geometry->max[0]; */

    xLeftFlatRamp=new_xMin+(new_xMax-new_xMin)*xLeftFlatRamp;
    xRightRampFlat=new_xMin+(new_xMax-new_xMin)*xRightRampFlat;

    fprintf(stderr, "Left flat <= %g -> %g\n", contextExt->leftFlatFraction, xLeftFlatRamp);
    fprintf(stderr, "Right flat >= %g -> %g\n", contextExt->rightFlatFraction, xRightRampFlat);
    fprintf(stderr, "Ramp-flat smoothness = %g\n", smoothFactor);
    fprintf(stderr, "Flat/ramp/flat ranges:  %g <-> %g <-> %g <-> %g\n", new_xMin, xLeftFlatRamp, xRightRampFlat, new_xMax);
    /*
     *  Loop across all nodes in mesh
     *    - total of x*y*z nodes is given by context->mesh->nodeGlobalCount+1
     */
    for( node_g = 0; node_g < context->mesh->nodeGlobalCount; node_g++ ) {
	Node_LocalIndex			node_l = Mesh_NodeMapGlobalToLocal( mesh, node_g );
	Index				i_g;
	Index				j_g;
	Index				k_g;
	Coord*				coord = 0;
	Coord				tmpCoord;

	/* 
	 *  If a local node, directly change the node coordinates else use a temporary location
	 */
	if( node_l < context->mesh->nodeLocalCount ) { /* a local node */
	    coord = Snac_NodeCoord_P( context, node_l );
	}
	else {
	    /*
	     *  If the node is running on another CPU, use dummy ptr
	     *   - wouldn't it be better to just skip (continue) to next node?
	     */
	    coord = &tmpCoord;
	}
	/*
	 *  Convert 1d mesh node index to 3d i,j,k mesh node position
	 */
	RegularMeshUtils_Node_1DTo3D( decomp, node_g, &i_g, &j_g, &k_g );

	/*
	 *  Do nothing to x coordinate
	 */
	(*coord)[0] = new_x[i_g];

	/*exp(-smoothFactor*(xRightRampFlat-new_x[i_g])/old_yMax)
	 *  Transform y coordinates by shearing mesh parallel to vertical axis
	 */
	(*coord)[1] = new_y[j_g];
	midOffset=( (xRightRampFlat-(xRightRampFlat-new_xHalf)*( 1.0-exp(-smoothFactor*(xRightRampFlat-new_xHalf)/old_yMax) )) 
		    - (new_xHalf-xLeftFlatRamp)*( 1.0-exp(-smoothFactor*(new_xHalf-xLeftFlatRamp)/old_yMax) ) )*tan(slopeAngle);
	if(new_x[i_g]<=new_xHalf) {
	    /* LHS */
	    (*coord)[1] +=
		( (new_x[i_g]<xLeftFlatRamp ? 0 :  
		   (new_x[i_g]-xLeftFlatRamp)*( 1.0-exp(-smoothFactor*(new_x[i_g]-xLeftFlatRamp)/old_yMax) )*tan(slopeAngle) )
		  *(new_y[j_g]>=resolveHeight ? 1.0 : new_y[j_g]/resolveHeight) );
	} else {
	    /* RHS */
	    (*coord)[1] +=
		( (new_x[i_g]>=xRightRampFlat ? xRightRampFlat*tan(slopeAngle)-midOffset :  
		   (xRightRampFlat-(xRightRampFlat-new_x[i_g])*( 1.0-exp(-smoothFactor*(xRightRampFlat-new_x[i_g])/old_yMax) ))*tan(slopeAngle) - midOffset)
		  *(new_y[j_g]>=resolveHeight ? 1.0 : new_y[j_g]/resolveHeight) );

	}
	/*
	 *  Track maximum y as mesh deforms
	 */
	if((*coord)[1]>new_yMax) new_yMax=(*coord)[1];
	
#ifdef DEBUG
	fprintf( stderr, "\tnode_l: %2u, node_g: %2u, i: %2u, j: %2u, k: %2u, ",
		 node_l,
		 node_g,
		 i_g,
		 j_g,
		 k_g );
	fprintf( stderr, "x: %12g, y: %12g, z: %12g\n", (*coord)[0], (*coord)[1], (*coord)[2] );
#endif
    }
    /*
     *  Reset vertical max
     */
    geometry->max[1] = new_yMax;

}



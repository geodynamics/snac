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
 ** $Id: Track.c $
 **
 **~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "Context.h"
#include "Register.h"
#include "Track.h"
#include <string.h>
#include <math.h>

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

//#define DEBUG

void SnacHillSlope_Track( void* _context ) {
	Snac_Context			*context = (Snac_Context*)_context;
	SnacHillSlope_Context		*contextExt = ExtensionManager_Get(
							context->extensionMgr,
							context,
							SnacHillSlope_ContextHandle );
	Mesh				*mesh = context->mesh;
	MeshLayout			*layout = (MeshLayout*)mesh->layout;
	HexaMD				*decomp = (HexaMD*)layout->decomp;
	BlockGeometry			*geometry = (BlockGeometry*)layout->elementLayout->geometry;
	const int			full_I_node_range=decomp->nodeGlobal3DCounts[0];
	const int			full_J_node_range=decomp->nodeGlobal3DCounts[1];
	const int			full_K_node_range=decomp->nodeGlobal3DCounts[2];
	int                   	        index_I,index_J,index_K;

	static double			*yGridOldPtr, *yGridOlderPtr;
	double				max_yVelocity, max_yAcceln;
	static double			unit_yVelocity=0.0, unit_yAcceln=0.0;
	static double			old_max_yVelocity=0.0, old_max_yAcceln=0.0;
/* 	static double			min_yVelocity=0.0, min_yAcceln=0.0; */
	static char			fallingFlag=FALSE;

	const double			trackLevel=(double)contextExt->trackLevel;
	const double			startThreshold=(double)(contextExt->startThreshold<=1? 
					       (contextExt->startThreshold>=0 ? contextExt->startThreshold : 0) : 1);
	const double			stopThreshold=(double)(contextExt->stopThreshold<=1? 
					       (contextExt->stopThreshold>=0 ? contextExt->stopThreshold : 0) : 1);
	

/* 	if (context->timeStep % context->dumpEvery == 0) { */
/* 	    Journal_Printf( context->snacInfo,"timeStep=%d (in track)\n", context->timeStep ); */
/* 	} */
	/*
	 *  Set up a tracking grids (slices of mesh) to allow t instance to be compared with t-1, t-2 instances
	 */
	if(context->timeStep==1) {
	    yGridOldPtr=(double *)malloc((size_t)(full_I_node_range*full_K_node_range*sizeof(double)));
	    yGridOlderPtr=(double *)malloc((size_t)(full_I_node_range*full_K_node_range*sizeof(double)));
	    for(index_I = 0; index_I < full_I_node_range; index_I++) {
		for(index_K = 0; index_K < full_K_node_range; index_K++) {
		    *(yGridOlderPtr+index_I + full_I_node_range*index_K)=-1.0E19;
		    *(yGridOldPtr+index_I + full_I_node_range*index_K)=-1.0E19;
		}
	    }
	}

	/*
	 * Compare t instance with t-1, t-2 instances, calculating slice vel, acceln and identifying maxima
	 */
	max_yVelocity=0.0;
	max_yAcceln=0.0;
	for(index_I = 0; index_I < full_I_node_range; index_I++) {
	    for(index_K = 0; index_K < full_K_node_range; index_K++) {
		Node_GlobalIndex		node_gI;
		Node_LocalIndex			node_lI;
		Coord				*coordPtr = 0;
		double				node_yElevation, node_yVelocity, node_yAcceln;
		double				*tmp_yGridOldPtr, *tmp_yGridOlderPtr;

		index_J=(int)(((double)full_J_node_range-1.0)*(1.0-trackLevel));
		node_gI = index_I + full_I_node_range*index_J + full_I_node_range*full_J_node_range*index_K;
		node_lI = Mesh_NodeMapGlobalToLocal( mesh, node_gI );
/* 		Journal_Printf( context->snacInfo,"\t\t%d,%d,%d -> %d ->%d \n", index_I,index_J,index_K,node_gI,node_lI ); */
/* 		node_lI = _MeshDecomp_Node_GlobalToLocal1D( decomp, node_gI ); */
/* 		Journal_Printf( context->snacInfo,"\t\t%d,%d,%d -> %d ->%d \n", index_I,index_J,index_K,node_gI,node_lI ); */

		/* If a local node, read its elevation, if not, give a dummy value */
		if( node_lI < context->mesh->nodeLocalCount ) { /* a local node */
		    coordPtr = Snac_NodeCoord_P( context, node_lI );
		    node_yElevation = (*coordPtr)[1]+(*coordPtr)[1]+(*coordPtr)[1];
		}
		else {
		    node_yElevation=-1.0E29;
		}
		tmp_yGridOldPtr = yGridOldPtr+index_I + full_I_node_range*index_K;
		tmp_yGridOlderPtr = yGridOlderPtr+index_I + full_I_node_range*index_K;
		/*
		 * Start assessing the vertical surface motions after two time steps
		 */
		node_yVelocity = node_yElevation-*tmp_yGridOldPtr;
		node_yAcceln = node_yVelocity-(*tmp_yGridOldPtr-*tmp_yGridOlderPtr);
		if(context->timeStep>=3){
		    if(fabs(node_yVelocity)>max_yVelocity)
			max_yVelocity = fabs(node_yVelocity);
		    if(fabs(node_yAcceln)>max_yAcceln)
			max_yAcceln = fabs(node_yAcceln);
/* 		    if(fabs(node_yVelocity)<min_yVelocity && fabs(node_yVelocity)>0.0) */
/* 			min_yVelocity = fabs(node_yVelocity); */
/* 		    if(fabs(node_yAcceln)<min_yAcceln && fabs(node_yAcceln)>0.0) */
/* 			min_yAcceln = fabs(node_yAcceln); */
		}
/* 		Journal_Printf( context->snacInfo, */
/* 				"%d,%d,%d -> %d ->%d   :   y=%0.8f  /  oy=%0.8f  /  ooy=%0.8f   ->  v%g (%g)  ,  a=%g (%g)\n",  */
/* 				index_I,index_J,index_K,node_gI,node_lI,   */
/* 				node_yElevation, *tmp_yGridOldPtr, *tmp_yGridOlderPtr, */
/* 				node_yVelocity,max_yVelocity, node_yAcceln,max_yAcceln */
/* 				); */
		/*
		 * Record this elevation field and push previous back to "OlderPtr" array
		 */
		*tmp_yGridOlderPtr = *tmp_yGridOldPtr;
		*tmp_yGridOldPtr = node_yElevation;
	    }
	}
/* 	if(!contextExt->startedTrackingFlag && unit_yVelocity>min_yVelocity && min_yVelocity<1.0) */
/* 	    unit_yVelocity = min_yVelocity;		 */
/* 	if(!contextExt->startedTrackingFlag && unit_yAcceln>min_yAcceln && min_yAcceln<1.0) */
/* 	    unit_yAcceln = min_yAcceln;	 */
	if(unit_yVelocity==0.0 && max_yVelocity>0.0)
	    unit_yVelocity = max_yVelocity;		
	if(unit_yAcceln==0.0 && max_yAcceln>0.0)
	    unit_yAcceln = max_yAcceln;	
	if(!contextExt->startedTrackingFlag && max_yVelocity>=unit_yVelocity && context->timeStep>=4) 
	    contextExt->startedTrackingFlag=TRUE;

	/*
	 *  Decide whether to stop or to continue simulation
	 */
	if(contextExt->startedTrackingFlag && !contextExt->elasticStabilizedFlag){
#ifdef DEBUG
	    fprintf(stderr,"t=%d:  elasticStabilizedFlag=%d   startedTrackingFlag=%d\n",
		    context->timeStep, contextExt->elasticStabilizedFlag, contextExt->startedTrackingFlag ); 
#endif
	    fallingFlag = CheckFallingFn(max_yVelocity,max_yAcceln,old_max_yVelocity,old_max_yAcceln);
	    if(CheckStabilizingFn(max_yVelocity/unit_yVelocity, 
				  max_yAcceln/unit_yAcceln, stopThreshold, fallingFlag)==TRUE
	       && context->maxTimeSteps!=context->timeStep) {
		/*
		 *  Stabilizing!  Therefore terminate processing at the next time step
		 */
		contextExt->elasticStabilizedFlag=TRUE;
		if(contextExt->solveElasticEqmOnlyFlag)
		    context->maxTimeSteps=context->timeStep+context->dumpEvery;

/* 		Journal_Printf( context->snacInfo,"Stabilizing (falling?=%d) on level %d (%g)\n", */
/* 				fallingFlag, index_J, trackLevel ); */
	    } else {
		/*
		 *  Still working...
		 */
/* 		if (context->timeStep % context->dumpEvery == 0) { */
/* 		    Journal_Printf( context->snacInfo,"Changing (rising?=%d) on level %d (%g) (t steps=%d/%d)\n", */
/* 				    fallingFlag, index_J, trackLevel,  */
/* 				    context->timeStep, context->maxTimeSteps ); */
/* 		} */
	    }
	}


/* 	if (context->timeStep % context->dumpEvery == 0) { */
/* 	    Journal_Printf( context->snacInfo,"\tTracking?=%d  unit vel=%g,  unit accel=%g\n", */
/* 			    contextExt->startedTrackingFlag, */
/* 			    unit_yVelocity/geometry->max[1], unit_yAcceln/geometry->max[1] */
/* 			    ); */
/* 	    Journal_Printf( context->snacInfo,"\tmax vel=%g (was %g)  ,  max accel=%g (was %g)\n",  */
/* 			    max_yVelocity/(unit_yVelocity>0.0?unit_yVelocity:1), */
/* 			    old_max_yVelocity/(unit_yVelocity>0.0?unit_yVelocity:1),  */
/* 			    max_yAcceln/(unit_yAcceln>0.0?unit_yAcceln:1),  */
/* 			    old_max_yAcceln/(unit_yAcceln>0.0?unit_yAcceln:1) */
/* 			    ); */
/* 	    Journal_Printf( context->snacInfo,"\tfalling?=%d  tracking?=%d  stabilized?=%d\n", */
/* 			    fallingFlag, contextExt->startedTrackingFlag, contextExt->elasticStabilizedFlag); */
/* 	} */


	/*
	 * Record the current mesh slice velocity and acceln for use next iteration
	 */
	old_max_yVelocity = max_yVelocity;
	old_max_yAcceln = max_yAcceln;

}


/*
 *----------------------------------------------------------------------
 *
 * CheckFallingFn --
 *
 *      Decide whether solution is heading away from or towards equilibrium
 *
 * Returns:
 *	True=falling/False=rising
 *
 * Side effects:
 *      None
 *
 *----------------------------------------------------------------------
 */

char 
CheckFallingFn(double velocity, double acceln, double old_velocity, double old_acceln)
{
    /*
     *  Return TRUE if either velocity or acceln is dropping or constant 
     */
/*     fprintf(stderr, "%g->%g   %g->%g\n", old_velocity,velocity,old_acceln,acceln); */
    return ( ((velocity<old_velocity) && (acceln<old_acceln)) ? TRUE : FALSE );
}


/*
 *----------------------------------------------------------------------
 *
 * CheckStabilizingFn --
 *
 *      Decide whether solution  is stabilizing
 *
 * Returns:
 *	True=stabilizing/False=still changing
 *
 * Side effects:
 *      None
 *
 *----------------------------------------------------------------------
 */

char 
CheckStabilizingFn(double velocity, double acceln, double stopThreshold, char fallingFlag)
{
    return ( fallingFlag & (velocity<=stopThreshold /* && acceln<stopThreshold */ ? TRUE : FALSE) );
}

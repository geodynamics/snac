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
** $Id: RemeshCoords.c 3173 2005-11-21 23:47:09Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "Mesh.h"
#include "Context.h"
#include "Remesh.h"
#include "Register.h"

#include <math.h>
#include <assert.h>
#include <string.h>
#include <float.h>


void _SnacRemesher_NewCoords( void* _context ) {
	Snac_Context*			context = (Snac_Context*)_context;
	SnacRemesher_Context*	contextExt = ExtensionManager_Get(
							context->extensionMgr,
							context,
							SnacRemesher_ContextHandle );
	Mesh*				mesh = context->mesh;
	SnacRemesher_Mesh*		meshExt = ExtensionManager_Get(
							context->meshExtensionMgr,
							mesh,
							SnacRemesher_MeshHandle );
	HexaMD*				decomp = (HexaMD*)mesh->layout->decomp;

	void _SnacRemesher_RemapSurface( void*				_context,
							   unsigned			nSurfNodes,
							   Node_LocalIndex*		surfNodeMap,
							   unsigned			nTris,
							   Node_DomainIndex*	triNodeMap );


	/*
	** This rewrite is based on the following assumptions:
	**	1. Only the top and bottom internal nodes are free, the rest will be restored to their initial positions.
	**	2. ... that's it for now.
	*/


	/*
	** The left, right, front and back wall nodes will already be stored.  Also, the x and z components of the bulk,
	** top and bottom nodes will also be pre-calculated.  So all that remains is to calculate the y components.
	*/


	/*
	** Performing a bottom surface remap is only necessary if bottomRestore is false.  This is because the newNodeCoords
	** array in meshExt will already have the correct bottom values in there by default.
	*/

	if( contextExt->bottomRestore == False ) {
		_SnacRemesher_RemapSurface( context,
							   meshExt->nBotInternNodes,
							   meshExt->botInternToLocal,
							   meshExt->nBotTriNodes,
							   meshExt->botTriToDomain );
	}


	/*
	** Remap the top surface.
	*/

	_SnacRemesher_RemapSurface( context,
						   meshExt->nTopInternNodes,
						   meshExt->topInternToLocal,
						   meshExt->nTopTriNodes,
						   meshExt->topTriToDomain );


	/*
	** The bulk of the mesh requires some communication, it needs to know both the terminals of
	** the y-axis for each y-oriented line.
	*/

	{
		double*	remoteYCoords;
		SnacSync*	sync = meshExt->sync;
		unsigned	yLine_i;

		/* Prepare an array to receive any y coords we need. */
		if( sync->netSource > 0 ) {
			remoteYCoords = Memory_Alloc_Array( double, sync->netSource, "SnacRemesher" );
		}
		else {
			remoteYCoords = NULL;
		}

		/* Send and receive. */
		SnacSync_SendRecv( sync,
				   &meshExt->newNodeCoords[0][1],
				   remoteYCoords );

		/* Now that we have any required remote y coords, interpolate the local bulk nodes. */
		for( yLine_i = 0; yLine_i < meshExt->nYLines; yLine_i++ ) {
			double	top;
			double	bot;
			double	h;
			unsigned	ylNode_i;

			/* Find the top coordinate. */
			if( meshExt->yLineUTerm[yLine_i] < mesh->nodeLocalCount ) {
				Node_LocalIndex	lNodeInd = meshExt->yLineUTerm[yLine_i];

				top = meshExt->newNodeCoords[lNodeInd][1];
			}
			else {
				unsigned	remInd = meshExt->yLineUTerm[yLine_i] - mesh->nodeLocalCount;

				top = remoteYCoords[remInd];
			}

			/* Find the bottom coordinate. */
			if( meshExt->yLineLTerm[yLine_i] < mesh->nodeLocalCount ) {
				Node_LocalIndex	lNodeInd = meshExt->yLineLTerm[yLine_i];

				bot = meshExt->newNodeCoords[lNodeInd][1];
			}
			else {
				unsigned	remInd = meshExt->yLineLTerm[yLine_i] - mesh->nodeLocalCount;

				bot = remoteYCoords[remInd];
			}

			/* Calculate the interval. */
			h = (top - bot) / (double)(decomp->nodeGlobal3DCounts[1] - 1);

			/* Interpolate all the local nodes in this y-line. */
			for( ylNode_i = 0; ylNode_i < meshExt->yLineNYInds[yLine_i]; ylNode_i++ ) {
				Node_LocalIndex	lNodeInd = meshExt->yLineLocalInds[yLine_i][ylNode_i];
				unsigned			yInd = meshExt->yLineYInds[yLine_i][ylNode_i];

				meshExt->newNodeCoords[lNodeInd][1] = bot + h * (double)yInd;
			}
		}

		/* Free up resources. */
		FreeArray( remoteYCoords );
	}
}


void _SnacRemesher_RemapSurface( void*				_context,
						   unsigned			nSurfNodes,
						   Node_LocalIndex*		surfNodeMap,
						   unsigned			nTriNodes,
						   Node_DomainIndex*	triNodeMap )
{
	Snac_Context*			context = (Snac_Context*)_context;
	Mesh*				mesh = context->mesh;
	SnacRemesher_Mesh*		meshExt = ExtensionManager_Get(
							context->meshExtensionMgr,
							mesh,
							SnacRemesher_MeshHandle );
	unsigned				surfNode_i;

	Bool _SnacRemesher_PointInTri( Coord pnt, Coord a, Coord b, Coord c );
	void _SnacRemesher_TriBarycenter( Coord tri[3], Coord pnt, Coord dst );


	if( nSurfNodes == 0 ) {
		return;
	}


	/*
	** Loop over the nodes to be mapped (nodeInds) and determine which surface element of the old
	** mesh it falls in.  Then, interpolate the position there using barycentric coordinates.
	*/

	for( surfNode_i = 0; surfNode_i < nSurfNodes; surfNode_i++ ) {
		Node_LocalIndex	lNodeInd = surfNodeMap[surfNode_i];
		Coord			newCoord;
		unsigned			triNode_i;

		/* Grab the new x and z coords. */
		newCoord[0] = meshExt->newNodeCoords[lNodeInd][0];
		newCoord[1] = 0.0;
		newCoord[2] = meshExt->newNodeCoords[lNodeInd][2];

		/* Find the triangle it's in and interpolate. */
		for( triNode_i = 0; triNode_i < nTriNodes; triNode_i += 3 ) {
			Coord	tri[3];

			/* Collect the coords of the tri and clear the y component. */
			Vector_Set( tri[0], mesh->nodeCoord[triNodeMap[triNode_i + 0]] );
			tri[0][1] = 0.0;
			Vector_Set( tri[1], mesh->nodeCoord[triNodeMap[triNode_i + 1]] );
			tri[1][1] = 0.0;
			Vector_Set( tri[2], mesh->nodeCoord[triNodeMap[triNode_i + 2]] );
			tri[2][1] = 0.0;

			if( _SnacRemesher_PointInTri( newCoord, tri[0], tri[1], tri[2] ) == True ) {
				Coord	center;

				/* Calculate the barycentric coords of the new point in the triangle, then
				   interpolate. */
				_SnacRemesher_TriBarycenter( tri, newCoord, center );
				newCoord[1] = center[0] * mesh->nodeCoord[triNodeMap[triNode_i + 0]][1] +
						    center[1] * mesh->nodeCoord[triNodeMap[triNode_i + 1]][1] +
						    center[2] * mesh->nodeCoord[triNodeMap[triNode_i + 2]][1];

				/* Update the y-axis of the new node coords. */
				meshExt->newNodeCoords[lNodeInd][1] = newCoord[1];

				break;
			}
		}

		/* Note that if the new coord couldn't be projected onto the triangulated surface (the mesh has either
		   contracted too far, drifted too far or there is not enough shadow depth), then the current y coord will
		   be left as is. */
	}
}


/*
** Two functions for determining whether a point is inside a triangle.  Probably not the fastest
** way of doing this but it is independant of triangle winding so can be used without a whole lot
** of extra consideration.  Additionally, it may be better to use a 'winding' style solution so that
** if the surface to be projected onto has folded then we can know if we are projecting onto an
** unreachable triangle.
*/

Bool _SnacRemesher_PointInTri( Coord pnt, Coord a, Coord b, Coord c ) {
	Bool _SnacRemesher_SameSide( Coord pnt, Coord ref, Coord from, Coord to );

	if( _SnacRemesher_SameSide( pnt, a, b, c ) &&
	    _SnacRemesher_SameSide( pnt, b, a, c ) &&
	    _SnacRemesher_SameSide( pnt, c, a, b ) )
	{
		return True;
	}
	else {
		return False;
	}
}


Bool _SnacRemesher_SameSide( Coord pnt, Coord ref, Coord from, Coord to ) {
	Coord	line;
	Coord	cp[2];

	Vector_Sub( line, to, from );
	Vector_Sub( cp[0], pnt, from );
	Vector_Sub( cp[1], ref, from );
	Vector_Cross( cp[0], line, cp[0] );
	Vector_Cross( cp[1], line, cp[1] );

	return (Vector_Dot( cp[0], cp[1] ) >= -1.0e-16) ? True : False;
}


/*
** Calculates the barycenter of a triangle with respect to some point.
*/

void _SnacRemesher_TriBarycenter( Coord tri[3], Coord pnt, Coord dst ) {
	double	a = tri[0][0] - tri[2][0];
	double	b = tri[1][0] - tri[2][0];
	double	c = tri[2][0] - pnt[0];
	double	d = tri[0][1] - tri[2][1];
	double	e = tri[1][1] - tri[2][1];
	double	f = tri[2][1] - pnt[1];
	double	g = tri[0][2] - tri[2][2];
	double	h = tri[1][2] - tri[2][2];
	double	i = tri[2][2] - pnt[2];

	dst[0] = (b * (f + i) - c * (e + h)) / (a * (e + h) - b * (d + g));
	dst[1] = (a * (f + i) - c * (d + g)) / (b * (d + g) - a * (e + h));
	dst[2] = 1.0 - dst[0] - dst[1];
}







#if 0
/*
** Removed but not deleted while rewriting the whole damn thing.
**	Luke, 31/06/2005
*/



#define DIM 3
#define NODES_PER_ELEMENT 8

#ifndef MAX
	#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
	#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

double isLargerThanZero( double, double );
double isLargerThanZero2( double, double, double );
double getSignedArea( Coord );
void xyz2tprCoord( Coord, Coord* );
void tpr2xyzCoord( Coord, Coord* );
void xyz2tprVel( Coord, Coord, Coord* );
void tpr2xyzVel( Coord, Coord, Coord* );



void _SnacRemesher_NewCoords_Cartesian( void* _context ) {
	Snac_Context*			context = (Snac_Context*)_context;
	SnacRemesher_Context*			contextExt = ExtensionManager_Get(
                                                                      context->extensionMgr,
                                                                      context,
                                                                      SnacRemesher_ContextHandle );
	Mesh*				mesh = context->mesh;
	SnacRemesher_Mesh*		meshExt = ExtensionManager_Get(
                                                           context->meshExtensionMgr,
                                                           mesh,
                                                           SnacRemesher_MeshHandle );
	HexaMD*				decomp = (HexaMD*)mesh->layout->decomp;
	Node_LocalIndex			index;
	int*				ind;
	Index				d;
	Element_LocalIndex      element_lK,element_lI,element_lN;
	Node_LocalIndex         node_lK,node_lI,node_lII,node_lN;
	Element_LocalIndex		nElementX=decomp->elementLocal3DCounts[decomp->rank][0];
	Element_LocalIndex		nElementY=decomp->elementLocal3DCounts[decomp->rank][1];
	Element_LocalIndex		nElementZ=decomp->elementLocal3DCounts[decomp->rank][2];
	Node_LocalIndex		    nNodeX=decomp->nodeLocal3DCounts[decomp->rank][0];
	Node_LocalIndex		    nNodeY=decomp->nodeLocal3DCounts[decomp->rank][1];
	Node_LocalIndex		    nNodeZ=decomp->nodeLocal3DCounts[decomp->rank][2];
	Element_NodeIndex		elementNodeCount;
	double				xc[DIM*NODES_PER_ELEMENT];
	PartitionIndex			rn_I;
	int                             count;
	int			        en_II[4] = { 3, 2, 6, 7};
	int			        nn[2][3] = { {3, 7, 2}, {2, 7, 6} };
	unsigned int                       All_passed = 0;
	double                             TOL = 0.0f;
    const int                          shadowDepth = ( decomp->shadowDepth > 3) ? decomp->shadowDepth : 3;
	const int                          numSearchElement = 2*shadowDepth-1;
	const int                          numSearchDepth = shadowDepth-1;

	Journal_DPrintf( context->debug, "In: %s\n", __func__ );

	ind = (int*)malloc( sizeof(int) * nNodeX * nNodeZ );
	memset( ind, 0, sizeof(int) * nNodeX * nNodeZ );

	while(!All_passed) {
		/* Calculate the new node coordintates for the top-internal nodes. */
		for( node_lK = 0; node_lK < nNodeZ; node_lK++ )
			for( node_lII = 0; node_lII < nNodeX; node_lII++ ) {
				Element_NodeIndex		en_N;
				double				elembbox[2][DIM];
				Node_LocalIndex**		elementNodeTbl = context->mesh->elementNodeTbl;
				Tetrahedra_Index		tetra_I;
				Coord				initialNodeCoord;
				Element_LocalIndex      searchElement_lI,searchElement_lK;
				IJK                             ijk;

				node_lI = node_lII + (nNodeY-1)*nNodeX + node_lK*nNodeX*nNodeY;
				RegularMeshUtils_Node_Local1DTo3D( decomp, node_lI, &ijk[0], &ijk[1], &ijk[2] );
				element_lI = (ijk[0] <= 1)? 0 : (ijk[0]-1);
				element_lK = (ijk[2] <= 1)? 0 : (ijk[2]-1);

				if( ind[ijk[0]+ijk[2]*nNodeX] ) {
					continue;
				}

				initialNodeCoord[0] = meshExt->initialNodeCoord[node_lI][0];
				initialNodeCoord[1] = 0.0f;
				initialNodeCoord[2] = meshExt->initialNodeCoord[node_lI][2];

				for(searchElement_lK=0;searchElement_lK<numSearchElement;searchElement_lK++) {
					if( (element_lK+searchElement_lK-numSearchDepth) < 0 || (element_lK+searchElement_lK-numSearchDepth) >= nElementZ )
						continue;
					for(searchElement_lI=0;searchElement_lI<numSearchElement;searchElement_lI++) {
						if( (element_lI+searchElement_lI-numSearchDepth) < 0 || (element_lI+searchElement_lI-numSearchDepth) >= nElementX )
							continue;
						element_lN = (element_lI+searchElement_lI-numSearchDepth) + (nElementY-1)*nElementX +
							(element_lK+searchElement_lK-numSearchDepth)*nElementY*nElementX;

						elementNodeCount = context->mesh->elementNodeCountTbl[element_lN];
						if( elementNodeCount != (unsigned)NODES_PER_ELEMENT ) {
							printf( "elementNodeCount != NODES_PER_ELEMENT  element_lN = %d", element_lN );
							assert( 0 );
						}

						if( ind[ijk[0]+ijk[2]*nNodeX] )
							break;

						/* create a local copy of the veocity and coords for each of the element's nodes of the old mesh */
						for( en_N = 0; en_N < 4; en_N++) {
							node_lN = elementNodeTbl[element_lN][en_II[en_N]];
							for( d = 0; d < DIM; d++ ) {
								xc[en_II[en_N]*DIM+d] = Mesh_CoordAt( context->mesh, node_lN )[d];
							}
						}

						/* Calculate element's bounding box */
						for( d = 0; d < DIM; d++ ) {
							elembbox[0][d] = DBL_MAX;
							elembbox[1][d] = DBL_MIN;
						}
						for( en_N = 0; en_N < 4; en_N++ ) {
							for( d = 0; d < DIM; ++d ) {
								elembbox[0][d] = MIN( elembbox[0][d], xc[en_II[en_N]*DIM+d] );
								elembbox[1][d] = MAX( elembbox[1][d], xc[en_II[en_N]*DIM+d] );
							}
						}

						/* If new mesh's current node is outside the bounding box, next loop */
						if( initialNodeCoord[0] < elembbox[0][0]-0.5f*(elembbox[1][0]-elembbox[0][0]) ||
							initialNodeCoord[0] > elembbox[1][0]+0.5f*(elembbox[1][0]-elembbox[0][0]) ||
							initialNodeCoord[2] < elembbox[0][2]-0.5f*(elembbox[1][2]-elembbox[0][2]) ||
							initialNodeCoord[2] > elembbox[1][2]+0.5f*(elembbox[1][2]-elembbox[0][2]) )
							{
								//assert(0);
								continue;
							}
						/* if the current coord is very close to the initial ones, just copy those initial values rather than bothering to compute determinents and adding error */
						if( ( meshExt->initialNodeCoord[node_lI][0] >= mesh->nodeCoord[node_lI][0] - TOL*(elembbox[1][0]-elembbox[0][0]) && meshExt->initialNodeCoord[node_lI][0] <= mesh->nodeCoord[node_lI][0] + TOL*(elembbox[1][0]-elembbox[0][0]) ) &&
							( meshExt->initialNodeCoord[node_lI][1] >= mesh->nodeCoord[node_lI][1] - TOL*(elembbox[1][1]-elembbox[0][1]) && meshExt->initialNodeCoord[node_lI][1] <= mesh->nodeCoord[node_lI][1] + TOL*(elembbox[1][1]-elembbox[0][1]) ) &&
							( meshExt->initialNodeCoord[node_lI][2] >= mesh->nodeCoord[node_lI][2] - TOL*(elembbox[1][2]-elembbox[0][2]) && meshExt->initialNodeCoord[node_lI][2] <= mesh->nodeCoord[node_lI][2] + TOL*(elembbox[1][2]-elembbox[0][2]) ) ) {
							meshExt->newNodeCoord[node_lI][0] = meshExt->initialNodeCoord[node_lI][0];
							meshExt->newNodeCoord[node_lI][1] = Mesh_CoordAt( context->mesh, node_lI )[1];
							meshExt->newNodeCoord[node_lI][2] = meshExt->initialNodeCoord[node_lI][2];
							ind[ijk[0]+ijk[2]*nNodeX] = 1;
							continue;
						}

						/* if the current coordinate is far enough from the initial position,
						   try to interpolate surface topo. */
						/* loop over 2 sub triangles in a rectangular element, work out the triangle it is in, and then interpolate */
						for( tetra_I = 0; tetra_I < 2; tetra_I++ ) {
							double				x1[DIM];
							double				x2[DIM];
							double				x3[DIM];
							double				y1,y2,y3;
							double				dett;
							double				det[3];
							Coord				axisA,axisB,axisC;
							unsigned int		found=0;

							for( d = 0; d < DIM; d++ ) {
								x1[d] = xc[nn[tetra_I][0]*DIM+d];
								x2[d] = xc[nn[tetra_I][1]*DIM+d];
								x3[d] = xc[nn[tetra_I][2]*DIM+d];
							}
							y1 = x1[1];
							y2 = x2[1];
							y3 = x3[1];

							x1[1] = x2[1] = x3[1] = 0.0f;
							Vector_Sub( axisA, x2, x1 );
							Vector_Sub( axisB, x3, x1 );
							Vector_Cross( axisC, axisA, axisB );
							dett = 0.5*getSignedArea( axisC );
							dett = isLargerThanZero2(dett, dett, TOL);

							Vector_Sub( axisA, x2, initialNodeCoord );
							Vector_Sub( axisB, x3, initialNodeCoord );
							Vector_Cross( axisC, axisA, axisB );
							det[0] = 0.5*getSignedArea( axisC );
							det[0] = isLargerThanZero2(det[0], dett, TOL);
							Vector_Sub( axisA, x3, initialNodeCoord );
							Vector_Sub( axisB, x1, initialNodeCoord );
							Vector_Cross( axisC, axisA, axisB );
							det[1] = 0.5*getSignedArea( axisC );
							det[1] = isLargerThanZero2(det[1], dett, TOL);
							Vector_Sub( axisA, x1, initialNodeCoord );
							Vector_Sub( axisB, x2, initialNodeCoord );
							Vector_Cross( axisC, axisA, axisB );
							det[2] = 0.5*getSignedArea( axisC );
							det[2] = isLargerThanZero2(det[2], dett, TOL);
							dett = det[0] + det[1] + det[2];
							//if( dett <= -1.0e-10 || dett == 0.0)
							//    fprintf(stderr,"me=%d element_lI=%d node_lI=%d TOL=%e det=%e %e %e %e\ninitialCoord=%e %e %e x1=%e %e %e x2=%e %e %e x3=%e %e %e\n",
							//            context->rank,element_lI,node_lI,TOL,det[0],det[1],det[2],dett,initialNodeCoord[0],initialNodeCoord[1],initialNodeCoord[2],
							//            x1[0],x1[1],x1[2],x2[0],x2[1],x2[2],x3[0],x3[1],x3[2]);
							//assert( dett > -1.0e-10 && dett != 0.0);
							//fprintf(stderr,"Top coord: element_lI=%d node_lI=%d tetra_I=%d det's=%e %e %e %e\n",
							//element_lI,node_lI,tetra_I,dett,det[0],det[1],det[2]);
							//fprintf(stderr,"tetra_I=%d dett=%e det0=%e det1=%e det2=%e\n",tetra_I,dett,det[0],det[1],det[2]);


							if( dett < 0 )	{
								//assert(0);
								continue;
							}
							/* found if all det are greater than zero */
							if( det[0] >= 0.0f && det[1] >= 0.0f && det[2] >= 0.0f ) {
								found = 1;
							}
							if( found ) {
								double				shape[3];
								Index				tNode_I;

								/* mark, such that we dont do it again */
								ind[ijk[0]+ijk[2]*nNodeX] = 1;

								/* Calculate the shape funcs */

								for( tNode_I = 0; tNode_I < 3; tNode_I++ ) {
									shape[tNode_I] = det[tNode_I] / dett;
								}
								//fprintf(stderr,"dett=%e det0=%e det1=%e det2=%e shape=%e %e %e\n",dett,det[0],det[1],det[2],shape[0],shape[1],shape[2]);

								meshExt->newNodeCoord[node_lI][0] = initialNodeCoord[0];
								meshExt->newNodeCoord[node_lI][1] = y1*shape[0] + y2*shape[1] + y3*shape[2];
								//meshExt->newNodeCoord[node_lI][1] -= TOL*fabs(meshExt->newNodeCoord[node_lI][1]);
								meshExt->newNodeCoord[node_lI][2] = initialNodeCoord[2];

								break;
							}
#if 0
							if(node_lII==0 && node_lK==0)
								fprintf(stderr,"Found? %d timeStep=%d me=%d element_lI=%d node_lI=%d tetra_I=%d\nxc=(%e %e %e) (%e %e %e)(%e %e %e) initialX=%e %e %e\nbbox=(%e %e) (%e %e)\ndett=%e %e %e %e\n\n",
										found,context->timeStep,context->rank,element_lN,node_lI,tetra_I,
										x1[0],x1[1],x1[2],x2[0],x2[1],x2[2],x3[0],x3[1],x3[2],
                                            initialNodeCoord[0],initialNodeCoord[1],initialNodeCoord[2],
                                            elembbox[0][0],elembbox[1][0],elembbox[0][2],elembbox[1][2],
										det[0],det[1],det[2],dett);
#endif
						}
					}
				}
				/*
				** Loop over shadow elements
				*/
				for( rn_I = 0; rn_I < meshExt->neighbourRankCount; rn_I++ ) {
					Element_ShadowIndex		element_sN;

                    /* Loop over all the shadow elements to find one including node_lI */
                    for( element_sN = 0; element_sN < meshExt->shadowElementRemoteCount[rn_I]; element_sN++ ) {
                        Element_GlobalIndex		gElement_N = meshExt->shadowElementRemoteArray[rn_I][element_sN];
                        Node_Index*			shadowElementNodesN = (Node_Index*)malloc( sizeof(Node_Index) * NODES_PER_ELEMENT );
                        Index				en_N;
                        double				elembbox[2][DIM];
                        Node_GlobalIndex	elementNodeN[8];
                        IJK                 eijkN;

                        /* skip irrelevant elements or exit the loop once the interpolation is done (ind == 1). */
                        RegularMeshUtils_Element_1DTo3D( decomp, gElement_N, &eijkN[0], &eijkN[1], &eijkN[2] );
                        if(eijkN[1] != decomp->elementGlobal3DCounts[1]-1)
                            continue;
                        if(ind[ijk[0]+ijk[2]*nNodeX] == 1)
                            break;

                        /* Figure out node index */
                        _HexaEL_BuildCornerIndices( context->mesh->layout->elementLayout, gElement_N, elementNodeN );
                        for( en_N = 0; en_N < 8; en_N++ ) {
                            Node_GlobalIndex	node_gN = elementNodeN[en_N];
                            Index			    node_lN = Mesh_NodeMapGlobalToLocal( context->mesh, node_gN );
                            unsigned int		found = 0;
                            if( node_lN < meshExt->nodeLocalCount ) {
                                shadowElementNodesN[en_N] = node_lN;
                                found = 1;
                                elementNodeN[en_N] = node_lN;
                            }
                            else {
                                Node_ShadowIndex		node_sI;
                                for( node_sI = 0; node_sI < meshExt->shadowNodeRemoteCount[rn_I]; node_sI++ ) {
                                    if( node_gN == meshExt->shadowNodeRemoteArray[rn_I][node_sI] ) {
                                        shadowElementNodesN[en_N] =
                                            context->mesh->nodeGlobalCount +
                                            node_sI;
                                        found = 1;
                                        break;
                                    }
                                }
                            }
                            assert( found ); /* we expected this to be a shadow node, but wasn't in list! */
                        }

                        elementNodeCount = (unsigned)NODES_PER_ELEMENT; /* Assume all shadow elements have 8 nodes per el */
                        if( elementNodeCount != (unsigned)NODES_PER_ELEMENT ) {
                            printf( "elementNodeCount != NODES_PER_ELEMENT  element_sN = %d", element_sN );
                            assert( 0 );
                        }

                        /* create a local copy of the veocity and coords for each of the element's nodes of the old mesh */
                        for( en_N = 0; en_N < 4; en_N++) {
                            node_lN = shadowElementNodesN[en_II[en_N]];
                            if( node_lN < context->mesh->nodeDomainCount ) {
                                for( d = 0; d < DIM; d++ ) {
                                    xc[en_II[en_N]*DIM+d] = Mesh_CoordAt( context->mesh, node_lN )[d];
                                }
                            }
                            else {
                                node_lN -= context->mesh->nodeGlobalCount;
                                for( d = 0; d < DIM; d++ ) {
                                    xc[en_II[en_N]*DIM+d] = meshExt->shadowNodeRemoteCoord[rn_I][node_lN][d];
                                }
                            }
                        }

                        /* Calculate element's bounding box */
                        for( d = 0; d < DIM; d++ ) {
                            elembbox[0][d] = DBL_MAX;
                            elembbox[1][d] = DBL_MIN;
                        }
                        for( en_N = 0; en_N < 4; en_N++ ) {
                            for( d = 0; d < DIM; ++d ) {
                                elembbox[0][d] = MIN( elembbox[0][d], xc[en_II[en_N]*DIM+d] );
                                elembbox[1][d] = MAX( elembbox[1][d], xc[en_II[en_N]*DIM+d] );
                            }
                        }

                        /* If new mesh's current node is outside the bounding box, next loop */
                        if( initialNodeCoord[0] < elembbox[0][0]-0.5f*(elembbox[1][0]-elembbox[0][0]) ||
                            initialNodeCoord[0] > elembbox[1][0]+0.5f*(elembbox[1][0]-elembbox[0][0]) ||
                            initialNodeCoord[2] < elembbox[0][2]-0.5f*(elembbox[1][2]-elembbox[0][2]) ||
                            initialNodeCoord[2] > elembbox[1][2]+0.5f*(elembbox[1][2]-elembbox[0][2]) )
                            {
                                //assert(0);
                                continue;
                            }

                        /* if the current coordinate is far enough fromt the initial position,
                           try to interpolate surface topo. */
                        /* loop over 2 sub triangles in a rectangular element, work out the triangle it is in, and then interpolate */
                        for( tetra_I = 0; tetra_I < 2; tetra_I++ ) {
                            double				x1[DIM];
                            double				x2[DIM];
                            double				x3[DIM];
                            double				y1,y2,y3;
                            double				dett;
                            double				det[3];
							Coord				axisA,axisB,axisC;
							unsigned int		found=0;

                            for( d = 0; d < DIM; d++ ) {
                                x1[d] = xc[nn[tetra_I][0]*DIM+d];
                                x2[d] = xc[nn[tetra_I][1]*DIM+d];
                                x3[d] = xc[nn[tetra_I][2]*DIM+d];
                            }
                            y1 = x1[1];
                            y2 = x2[1];
                            y3 = x3[1];
                            x1[1] = x2[1] = x3[1] = 0.0f;

                            Vector_Sub( axisA, x2, x1 );
                            Vector_Sub( axisB, x3, x1 );
                            Vector_Cross( axisC, axisA, axisB );
                            dett = 0.5*getSignedArea( axisC );
                            dett = isLargerThanZero2(dett, dett, TOL);


                            Vector_Sub( axisA, x2, initialNodeCoord );
                            Vector_Sub( axisB, x3, initialNodeCoord );
                            Vector_Cross( axisC, axisA, axisB );
                            det[0] = 0.5*getSignedArea( axisC );
                            det[0] = isLargerThanZero2(det[0], dett, TOL);
                            Vector_Sub( axisA, x3, initialNodeCoord );
                            Vector_Sub( axisB, x1, initialNodeCoord );
                            Vector_Cross( axisC, axisA, axisB );
                            det[1] = 0.5*getSignedArea( axisC );
                            det[1] = isLargerThanZero2(det[1], dett, TOL);
                            Vector_Sub( axisA, x1, initialNodeCoord );
                            Vector_Sub( axisB, x2, initialNodeCoord );
                            Vector_Cross( axisC, axisA, axisB );
                            det[2] = 0.5*getSignedArea( axisC );
                            det[2] = isLargerThanZero2(det[2], dett, TOL);
                            dett = det[0] + det[1] + det[2];

                            if( dett < 0 )	{
                                printf( "Determinant evaluation is wrong node=%d\t xt[0]=%g \t xt[1]=%g \t xt[t]=%g\n",
                                        node_lI,
                                        initialNodeCoord[0],
                                        initialNodeCoord[1],
                                        initialNodeCoord[2]);
                                continue;
                                //assert(0);
                            }
                            /* found if all det are greater than zero */
                            if( det[0] >= 0.0f && det[1] >= 0.0f && det[2] >= 0.0f ) {
                                found = 1;
                            }
                            if( found ) {
                                double				shape[3];
                                Index				tNode_I;

                                /* mark, such that we dont do it again */
                                ind[ijk[0]+ijk[2]*nNodeX] = 1;

                                /* Calculate the shape funcs */
                                for( tNode_I = 0; tNode_I < 3; tNode_I++ ) {
                                    shape[tNode_I] = det[tNode_I] / dett;
                                }

                                meshExt->newNodeCoord[node_lI][0] = initialNodeCoord[0];
                                meshExt->newNodeCoord[node_lI][1] = y1*shape[0] + y2*shape[1] + y3*shape[2];
                                meshExt->newNodeCoord[node_lI][2] = initialNodeCoord[2];

								break;
                            }
#if 0
                            if(context->rank==1 && node_lI==44)
                                fprintf(stderr,"Shadow Found? %d ind=%d timeStep=%d me=%d gElment_N=%d node_lI=%d tetra_I=%d\nxc=(%e %e %e) (%e %e %e)(%e %e %e) initialX=%e %e %e\nbbox=(%e %e) (%e %e)\ndett=%e %e %e %e\n\n",
                                        found,ind[ijk[0]+ijk[2]*nNodeX],context->timeStep,context->rank,gElement_N,node_lI,tetra_I,
                                        x1[0],x1[1],x1[2],x2[0],x2[1],x2[2],x3[0],x3[1],x3[2],
                                        initialNodeCoord[0],initialNodeCoord[1],initialNodeCoord[2],
                                        elembbox[0][0],elembbox[1][0],elembbox[0][2],elembbox[1][2],
                                        det[0],det[1],det[2],dett);
#endif
                        }
                    }
                }
            }
		All_passed = 1;
		count = 0;
		for( node_lK = 0; node_lK < nNodeZ; node_lK++ )
			for( node_lII = 0; node_lII < nNodeX; node_lII++ ) {
				node_lI=node_lII+(nNodeY-1)*nNodeX+node_lK*nNodeX*nNodeY;
				if( ind[node_lII+node_lK*nNodeX] == 0 ) {
					count++;
					All_passed = 0;
					fprintf(stderr,"me=%d TOP failed node_lI=%d TOL=%e\n",context->rank,node_lI,TOL);
				}
			}
		if(TOL==0.0)TOL = 1.0e-10;
		else TOL *= 10.0;
        Journal_Firewall( TOL <= 1.0e-03, context->snacError," In: %s, timeStep=%d rank=%d Increased tolerance for Top: %e count=%d\n", __func__, context->timeStep, context->rank, TOL,count );
		Journal_DPrintf( context->debug, " In: %s, rank=%d Increased tolerance for Top: %e count=%d\n", __func__, context->rank, TOL,count );
	}
	/* Calculate the new node coordintates for the bottom-internal nodes. */
    if(contextExt->bottomRestore) {
        for( index = 0; index < meshExt->bottomInternalCount; index++ ) {
			node_lI = meshExt->bottomInternalArray[index];
			meshExt->newNodeCoord[node_lI][0] = meshExt->initialNodeCoord[node_lI][0];
			meshExt->newNodeCoord[node_lI][1] = meshExt->initialNodeCoord[node_lI][1];
			meshExt->newNodeCoord[node_lI][2] = meshExt->initialNodeCoord[node_lI][2];
        }
    }
    else {
        en_II[0] = 0;	en_II[1] = 1;	en_II[2] = 5;	en_II[3] = 4;
        nn[0][0] = 0; nn[0][1] = 4; nn[0][2] = 1;
        nn[1][0] = 1; nn[1][1] = 4; nn[1][2] = 5;
        count = 0;
        memset( ind, 0, sizeof( int ) * nNodeX * nNodeZ );

        All_passed = 0;
        TOL= 0.0f;
        while(!All_passed) {
			for( node_lK = 0; node_lK < nNodeZ; node_lK++ )
				for( node_lII = 0; node_lII < nNodeX; node_lII++ ) {
                    Element_NodeIndex		en_N;
                    double				elembbox[2][DIM];
					Coord               initialNodeCoord;
					Node_LocalIndex**		elementNodeTbl = context->mesh->elementNodeTbl;
					Tetrahedra_Index		tetra_I;
                    Element_LocalIndex      searchElement_lI,searchElement_lK;
					IJK                             ijk;

					node_lI = node_lII + node_lK*nNodeX*nNodeY;
					RegularMeshUtils_Node_Local1DTo3D( decomp, node_lI, &ijk[0], &ijk[1], &ijk[2] );
					element_lI = (ijk[0] <= 1)? 0 : (ijk[0]-1);
					element_lK = (ijk[2] <= 1)? 0 : (ijk[2]-1);

					if( ind[ijk[0]+ijk[2]*nNodeX] ) {
						continue;
					}

					initialNodeCoord[0] = meshExt->initialNodeCoord[node_lI][0];
					initialNodeCoord[1] = 0.0f;
					initialNodeCoord[2] = meshExt->initialNodeCoord[node_lI][2];

					for(searchElement_lK=0;searchElement_lK<numSearchElement;searchElement_lK++) {
						if( (element_lK+searchElement_lK-numSearchDepth) < 0 || (element_lK+searchElement_lK-numSearchDepth) >= nElementZ )
							continue;
						for(searchElement_lI=0;searchElement_lI<numSearchElement;searchElement_lI++) {
							if( (element_lI+searchElement_lI-numSearchDepth) < 0 || (element_lI+searchElement_lI-numSearchDepth) >= nElementX )
								continue;
							element_lN = (element_lI+searchElement_lI-numSearchDepth) + (element_lK+searchElement_lK-numSearchDepth)*nElementY*nElementX;

							elementNodeCount = context->mesh->elementNodeCountTbl[element_lN];
							if( elementNodeCount != (unsigned)NODES_PER_ELEMENT ) {
								printf( "elementNodeCount != NODES_PER_ELEMENT  element_lN = %d", element_lN );
								assert( 0 );
							}

							if( ind[ijk[0]+ijk[2]*nNodeX] )
								break;

							/* create a local copy of the veocity and coords for each of the element's nodes of the old mesh */
							for( en_N = 0; en_N < 4; en_N++) {
								node_lN = elementNodeTbl[element_lN][en_II[en_N]];
								for( d = 0; d < DIM; d++ ) {
									xc[en_II[en_N]*DIM+d] = Mesh_CoordAt( context->mesh, node_lN )[d];
								}
							}

							/* Calculate element's bounding box */
							for( d = 0; d < DIM; d++ ) {
								elembbox[0][d] = DBL_MAX;
								elembbox[1][d] = DBL_MIN;
							}
							for( en_N = 0; en_N < 4; en_N++ ) {
								for( d = 0; d < DIM; ++d ) {
									elembbox[0][d] = MIN( elembbox[0][d], xc[en_II[en_N]*DIM+d] );
									elembbox[1][d] = MAX( elembbox[1][d], xc[en_II[en_N]*DIM+d] );
								}
							}
							/* If new mesh's current node is outside the bounding box, next loop */
							if( initialNodeCoord[0] < elembbox[0][0]-0.5f*(elembbox[1][0]-elembbox[0][0]) ||
								initialNodeCoord[0] > elembbox[1][0]+0.5f*(elembbox[1][0]-elembbox[0][0]) ||
								initialNodeCoord[2] < elembbox[0][2]-0.5f*(elembbox[1][2]-elembbox[0][2]) ||
								initialNodeCoord[2] > elembbox[1][2]+0.5f*(elembbox[1][2]-elembbox[0][2]) )
								{
									//assert(0);
									continue;
								}

							/* if the current coordinate is far enough fromt the initial position,
							   try to interpolate surface topo. */
							/* loop over 2 sub triangles in a rectangular element, work out the triangle it is in, and then interpolate */
							for( tetra_I = 0; tetra_I < 2; tetra_I++ ) {
								double				x1[DIM];
								double				x2[DIM];
								double				x3[DIM];
								double				y1,y2,y3;
								double				dett;
								double				det[3];
								Coord				axisA,axisB,axisC;
								unsigned int		found=0;

								for( d = 0; d < DIM; d++ ) {
									x1[d] = xc[nn[tetra_I][0]*DIM+d];
									x2[d] = xc[nn[tetra_I][1]*DIM+d];
									x3[d] = xc[nn[tetra_I][2]*DIM+d];
								}
								y1 = x1[1];
								y2 = x2[1];
								y3 = x3[1];

								x1[1] = x2[1] = x3[1] = 0.0f;
								Vector_Sub( axisA, x2, x1 );
								Vector_Sub( axisB, x3, x1 );
								Vector_Cross( axisC, axisA, axisB );
								dett = 0.5*getSignedArea( axisC );
								dett = isLargerThanZero2(dett, dett, TOL);

								Vector_Sub( axisA, x2, initialNodeCoord );
								Vector_Sub( axisB, x3, initialNodeCoord );
								Vector_Cross( axisC, axisA, axisB );
								det[0] = 0.5*getSignedArea( axisC );
								det[0] = isLargerThanZero2(det[0], dett, TOL);
								Vector_Sub( axisA, x3, initialNodeCoord );
								Vector_Sub( axisB, x1, initialNodeCoord );
								Vector_Cross( axisC, axisA, axisB );
								det[1] = 0.5*getSignedArea( axisC );
								det[1] = isLargerThanZero2(det[1], dett, TOL);
								Vector_Sub( axisA, x1, initialNodeCoord );
								Vector_Sub( axisB, x2, initialNodeCoord );
								Vector_Cross( axisC, axisA, axisB );
								det[2] = 0.5*getSignedArea( axisC );
								det[2] = isLargerThanZero2(det[2], dett, TOL);
								assert( dett > -1.0e-10 && dett != 0.0);
								dett = det[0] + det[1] + det[2];

								if( dett < 0 )	{
									continue;
									//assert(0);
								}
								/* found if all det are greater than zero */
								if( det[0] >= 0.0f && det[1] >= 0.0f && det[2] >= 0.0f ) {
									found = 1;
								}
								if( found ) {
									double				shape[3];
									Index				tNode_I;

									/* mark, such that we dont do it again */
									RegularMeshUtils_Node_Local1DTo3D( decomp, node_lI, &ijk[0], &ijk[1], &ijk[2] );
									ind[ijk[0]+ijk[2]*nNodeX] = 1;

									/* Calculate the shape funcs */
									for( tNode_I = 0; tNode_I < 3; tNode_I++ ) {
										shape[tNode_I] = det[tNode_I] / dett;
									}

									meshExt->newNodeCoord[node_lI][0] = initialNodeCoord[0];
									meshExt->newNodeCoord[node_lI][1] = y1*shape[0] + y2*shape[1] + y3*shape[2];
									meshExt->newNodeCoord[node_lI][2] = initialNodeCoord[2];

									break;
								}
#if 0
								fprintf(stderr,"Found? %d timeStep=%d me=%d element_lI=%d node_lI=%d tetra_I=%d\nxc=(%e %e %e) (%e %e %e)(%e %e %e) initialX=%e %e %e\nbbox=(%e %e) (%e %e)\ndett=%e %e %e %e\n\n",
										found,context->timeStep,context->rank,element_lN,node_lI,tetra_I,
										x1[0],x1[1],x1[2],x2[0],x2[1],x2[2],x3[0],x3[1],x3[2],
										initialNodeCoord[0],initialNodeCoord[1],initialNodeCoord[2],
										elembbox[0][0],elembbox[1][0],elembbox[0][2],elembbox[1][2],
										det[0],det[1],det[2],dett);
#endif
							}
						}
					}
					/*
					** Loop over shadow elements
					*/
					for( rn_I = 0; rn_I < meshExt->neighbourRankCount; rn_I++ ) {
						Element_ShadowIndex		element_sN;

                        /* Loop over all the shadow elements to find one including node_lI */
                        for( element_sN = 0; element_sN < meshExt->shadowElementRemoteCount[rn_I]; element_sN++ ) {
                            Element_GlobalIndex		gElement_N = meshExt->shadowElementRemoteArray[rn_I][element_sN];
                            Node_Index*			shadowElementNodesN = (Node_Index*)malloc( sizeof(Node_Index) * NODES_PER_ELEMENT );
                            Index				en_N;
                            double				elembbox[2][DIM];
                            Node_GlobalIndex	elementNodeN[8];
                            IJK                 eijkN;

                            /* skip irrelevant elements or exit the loop once the interpolation is done (ind == 1). */
                            RegularMeshUtils_Element_1DTo3D( decomp, gElement_N, &eijkN[0], &eijkN[1], &eijkN[2] );
                            if(eijkN[1] != 0)
                                continue;
                            if(ind[ijk[0]+ijk[2]*nNodeX] == 1)
                                break;

                            /* Figure out node index */
                            _HexaEL_BuildCornerIndices( context->mesh->layout->elementLayout, gElement_N, elementNodeN );
                            for( en_N = 0; en_N < 8; en_N++ ) {
                                Node_GlobalIndex     node_gN = elementNodeN[en_N];
                                Node_LocalIndex      node_lN = Mesh_NodeMapGlobalToLocal( context->mesh, node_gN );
                                unsigned int         found = 0;
                                if( node_lN < meshExt->nodeLocalCount ) {
                                    shadowElementNodesN[en_N] = node_lN;
                                    found = 1;
                                    elementNodeN[en_N] = node_lN;
                                }
                                else {
                                    Node_ShadowIndex		node_sI;
                                    for( node_sI = 0; node_sI < meshExt->shadowNodeRemoteCount[rn_I]; node_sI++ ) {
                                        if( node_gN == meshExt->shadowNodeRemoteArray[rn_I][node_sI] ) {
                                            shadowElementNodesN[en_N] =
                                                context->mesh->nodeGlobalCount +
                                                node_sI;
                                            found = 1;
                                            break;
                                        }
                                    }
                                }
                                assert( found ); /* we expected this to be a shadow node, but wasn't in list! */
                            }

                            elementNodeCount = (unsigned)NODES_PER_ELEMENT; /* Assume all shadow elements have 8 nodes per el */
                            if( elementNodeCount != (unsigned)NODES_PER_ELEMENT ) {
                                printf( "elementNodeCount != NODES_PER_ELEMENT  element_sN = %d", element_sN );
                                assert( 0 );
                            }

                            /* create a local copy of the veocity and coords for each of the element's nodes of the old mesh */
                            for( en_N = 0; en_N < 4; en_N++) {
                                node_lN = shadowElementNodesN[en_II[en_N]];
                                if( node_lN < context->mesh->nodeDomainCount ) {
                                    for( d = 0; d < DIM; d++ ) {
                                        xc[en_II[en_N]*DIM+d] = Mesh_CoordAt( context->mesh, node_lN )[d];
                                    }
                                }
                                else {
                                    node_lN -= context->mesh->nodeGlobalCount;
                                    for( d = 0; d < DIM; d++ ) {
                                        xc[en_II[en_N]*DIM+d] = meshExt->shadowNodeRemoteCoord[rn_I][node_lN][d];
                                    }
                                }
                            }

                            /* Calculate element's bounding box */
                            for( d = 0; d < DIM; d++ ) {
                                elembbox[0][d] = DBL_MAX;
                                elembbox[1][d] = DBL_MIN;
                            }
                            for( en_N = 0; en_N < 4; en_N++ ) {
                                for( d = 0; d < DIM; ++d ) {
                                    elembbox[0][d] = MIN( elembbox[0][d], xc[en_II[en_N]*DIM+d] );
                                    elembbox[1][d] = MAX( elembbox[1][d], xc[en_II[en_N]*DIM+d] );
                                }
                            }
                            /* If new mesh's current node is outside the bounding box, next loop */
                            if( initialNodeCoord[0] < elembbox[0][0]-0.5f*(elembbox[1][0]-elembbox[0][0]) ||
                                initialNodeCoord[0] > elembbox[1][0]+0.5f*(elembbox[1][0]-elembbox[0][0]) ||
                                initialNodeCoord[2] < elembbox[0][2]-0.5f*(elembbox[1][2]-elembbox[0][2]) ||
                                initialNodeCoord[2] > elembbox[1][2]+0.5f*(elembbox[1][2]-elembbox[0][2]) )
                                {
                                    //assert(0);
                                    continue;
                                }

                            /* if the current coord is very close to the initial ones, just copy those initial values rather than bothering to compute determinents and adding error */
                            if( ( initialNodeCoord[0] >= Mesh_CoordAt( context->mesh, node_lI )[0] - TOL*(elembbox[1][0]-elembbox[0][0]) && initialNodeCoord[0] <= Mesh_CoordAt( context->mesh, node_lI )[0] + TOL*(elembbox[1][0]-elembbox[0][0]) ) &&
                                ( initialNodeCoord[1] >= Mesh_CoordAt( context->mesh, node_lI )[1] - TOL*(elembbox[1][1]-elembbox[0][1]) && initialNodeCoord[1] <= Mesh_CoordAt( context->mesh, node_lI )[1] + TOL*(elembbox[1][1]-elembbox[0][1]) ) &&
                                ( initialNodeCoord[2] >= Mesh_CoordAt( context->mesh, node_lI )[2] - TOL*(elembbox[1][2]-elembbox[0][2]) && initialNodeCoord[2] <= Mesh_CoordAt( context->mesh, node_lI )[2] + TOL*(elembbox[1][2]-elembbox[0][2]) ) ) {
                                meshExt->newNodeCoord[node_lI][0] = initialNodeCoord[0];
                                meshExt->newNodeCoord[node_lI][1] = Mesh_CoordAt( context->mesh, node_lI )[1];
                                meshExt->newNodeCoord[node_lI][2] = initialNodeCoord[2];
                                ind[ijk[0]+ijk[2]*nNodeX] = 1;
                                continue;
                            }

                            /* if the current coordinate is far enough fromt the initial position,
                               try to interpolate surface topo. */
                            /* loop over 2 sub triangles in a rectangular element, work out the triangle it is in, and then interpolate */
                            for( tetra_I = 0; tetra_I < 2; tetra_I++ ) {
                                double				x1[DIM];
                                double				x2[DIM];
                                double				x3[DIM];
                                double				y1,y2,y3;
                                double				dett;
                                double				det[3];
								Coord				axisA,axisB,axisC;
								unsigned int		found=0;

                                for( d = 0; d < DIM; d++ ) {
                                    x1[d] = xc[nn[tetra_I][0]*DIM+d];
                                    x2[d] = xc[nn[tetra_I][1]*DIM+d];
                                    x3[d] = xc[nn[tetra_I][2]*DIM+d];
                                }
                                y1 = x1[1];
                                y2 = x2[1];
                                y3 = x3[1];
                                x1[1] = x2[1] = x3[1] = 0.0f;

                                Vector_Sub( axisA, x2, x1 );
                                Vector_Sub( axisB, x3, x1 );
                                Vector_Cross( axisC, axisA, axisB );
                                dett = 0.5*getSignedArea( axisC );
                                dett = isLargerThanZero2(dett, dett, TOL);


                                Vector_Sub( axisA, x2, initialNodeCoord );
                                Vector_Sub( axisB, x3, initialNodeCoord );
                                Vector_Cross( axisC, axisA, axisB );
                                det[0] = 0.5*getSignedArea( axisC );
                                det[0] = isLargerThanZero2(det[0], dett, TOL);
                                Vector_Sub( axisA, x3, initialNodeCoord );
                                Vector_Sub( axisB, x1, initialNodeCoord );
                                Vector_Cross( axisC, axisA, axisB );
                                det[1] = 0.5*getSignedArea( axisC );
                                det[1] = isLargerThanZero2(det[1], dett, TOL);
                                Vector_Sub( axisA, x1, initialNodeCoord );
                                Vector_Sub( axisB, x2, initialNodeCoord );
                                Vector_Cross( axisC, axisA, axisB );
                                det[2] = 0.5*getSignedArea( axisC );
                                det[2] = isLargerThanZero2(det[2], dett, TOL);
                                dett = det[0] + det[1] + det[2];

                                if( dett < 0 )	{
                                    continue;
                                }

                                /* found if all det are greater than zero */
                                if( det[0] >= 0.0f && det[1] >= 0.0f && det[2] >= 0.0f ) {
                                    found = 1;
                                }
                                if( found ) {
                                    double				shape[3];
                                    Index				tNode_I;

                                    /* mark, such that we dont do it again */
                                    ind[ijk[0]+ijk[2]*nNodeX] = 1;
                                    count++;

                                    /* Calculate the shape funcs */
                                    for( tNode_I = 0; tNode_I < 3; tNode_I++ ) {
                                        shape[tNode_I] = det[tNode_I] / dett;
                                    }

                                    meshExt->newNodeCoord[node_lI][0] = initialNodeCoord[0];
                                    meshExt->newNodeCoord[node_lI][1] = y1*shape[0] + y2*shape[1] + y3*shape[2];
                                    meshExt->newNodeCoord[node_lI][2] = initialNodeCoord[2];

									break;
                                }
                            }
                        }
                    }
                }
			All_passed = 1;
			count = 0;
			for( node_lK = 0; node_lK < nNodeZ; node_lK++ )
				for( node_lII = 0; node_lII < nNodeX; node_lII++ ) {
					node_lI=node_lII+(nNodeY-1)*nNodeX+node_lK*nNodeX*nNodeY;
					if( ind[node_lII+node_lK*nNodeX] == 0 ) {
						All_passed = 0;
						count++;
						fprintf(stderr,"me=%d BOT failed node_lI=%d TOL=%e\n",context->rank,node_lI,TOL);
					}
				}
			if(TOL==0.0)TOL = 1.0e-10;
			else TOL *= 10.0;
			Journal_Firewall( TOL <= 1.0e-03, context->snacError," In: %s, timeStep=%d rank=%d Increased tolerance for Bottom: %e count=%d\n", __func__, context->timeStep, context->rank, TOL,count );
			Journal_DPrintf( context->debug, " In: %s, rank=%d Increased tolerance for Bottom: %e count=%d\n", __func__, context->rank, TOL, count );
		}
	}
	free( ind );

	/* Calculate the new node coordintates for the wall nodes. */
	if( meshExt->meshType == -1 ) {
		assert( 0 );
	}
	else { /* SnacRemesher_Cartesian */
		for( index = 0; index < meshExt->wallCount; index++ ) {
			Node_LocalIndex				node_lI;
			IJK					ijk;
			Node_LocalIndex				nodeTop_lI, nodeBottom_lI;
			const Index 				topI = decomp->nodeLocal3DCounts[decomp->rank][1];

			node_lI = meshExt->wallArray[index];
			RegularMeshUtils_Node_Local1DTo3D( decomp, node_lI, &ijk[0], &ijk[1], &ijk[2] );

			/* The wall node positions revert to the initial position */
			meshExt->newNodeCoord[node_lI][0] = meshExt->initialNodeCoord[node_lI][0];
			meshExt->newNodeCoord[node_lI][2] = meshExt->initialNodeCoord[node_lI][2];
			nodeBottom_lI = RegularMeshUtils_Node_Local3DTo1D( decomp, ijk[0], 0, ijk[2] );
			nodeTop_lI = RegularMeshUtils_Node_Local3DTo1D( decomp, ijk[0], topI - 1, ijk[2] );
			meshExt->newNodeCoord[node_lI][1] =
				meshExt->newNodeCoord[nodeBottom_lI][1] +
				( meshExt->newNodeCoord[nodeTop_lI][1] - meshExt->newNodeCoord[nodeBottom_lI][1] ) /
				(topI - 1 ) *
				ijk[1];
		}
	}

	for( index = 0; index < meshExt->internalCount; index++ ) {
		Node_LocalIndex				node_lI;
		IJK					ijk;
		Node_LocalIndex				nodeBottom_lI;
		Node_LocalIndex				nodeTop_lI;
		HexaMD*					decomp = (HexaMD*)mesh->layout->decomp;
		const Index 				topI = decomp->nodeLocal3DCounts[decomp->rank][1];

		/* TODO... parallel version! */
		node_lI = meshExt->internalArray[index];
		RegularMeshUtils_Node_Local1DTo3D( decomp, node_lI, &ijk[0], &ijk[1], &ijk[2] );
		/* TODO... Local? where's the global equivalent? */
		nodeBottom_lI = RegularMeshUtils_Node_Local3DTo1D( decomp, ijk[0], 0, ijk[2] );
		nodeTop_lI = RegularMeshUtils_Node_Local3DTo1D( decomp, ijk[0], topI - 1, ijk[2] );

		meshExt->newNodeCoord[node_lI][0] = meshExt->initialNodeCoord[node_lI][0];
		meshExt->newNodeCoord[node_lI][1] =
			meshExt->newNodeCoord[nodeBottom_lI][1] +
			( meshExt->newNodeCoord[nodeTop_lI][1] - meshExt->newNodeCoord[nodeBottom_lI][1] ) /
			(topI - 1 ) *
			ijk[1];
		meshExt->newNodeCoord[node_lI][2] = meshExt->initialNodeCoord[node_lI][2];
	}
}

void _SnacRemesher_NewCoords_Spherical( void* _context ) {
	Snac_Context*			context = (Snac_Context*)_context;
	SnacRemesher_Context*			contextExt = ExtensionManager_Get(
							context->extensionMgr,
							context,
							SnacRemesher_ContextHandle );
	Mesh*				mesh = context->mesh;
	SnacRemesher_Mesh*		meshExt = ExtensionManager_Get(
						context->meshExtensionMgr,
						mesh,
						SnacRemesher_MeshHandle );
	HexaMD*				decomp = (HexaMD*)mesh->layout->decomp;
	Node_LocalIndex			index;
	Node_LocalIndex			node_lI,node_lII,node_lK,node_lN;
	int*				ind;
	Index				d;
	Element_LocalIndex		element_lK,element_lJ,element_lI,element_lN;
	Element_LocalIndex		nElementX=decomp->elementLocal3DCounts[decomp->rank][0];
	Element_LocalIndex		nElementY=decomp->elementLocal3DCounts[decomp->rank][1];
	Element_LocalIndex		nElementZ=decomp->elementLocal3DCounts[decomp->rank][2];
	Node_LocalIndex		        nNodeX=decomp->nodeLocal3DCounts[decomp->rank][0];
	Node_LocalIndex		        nNodeY=decomp->nodeLocal3DCounts[decomp->rank][1];
	Node_LocalIndex		        nNodeZ=decomp->nodeLocal3DCounts[decomp->rank][2];
	Element_NodeIndex		elementNodeCount;
	double				xc[DIM*NODES_PER_ELEMENT];
	double				xcs[DIM*NODES_PER_ELEMENT];
	PartitionIndex			rn_I;
	int                             count;
	int			        en_II[4] = { 3, 2, 6, 7};
	int			        nn[2][3] = { {3, 7, 2}, {2, 7, 6} };
	unsigned int                    All_passed = 0;
	double                          TOL = 0.0f;
    const int                          shadowDepth = ( decomp->shadowDepth > 3) ? decomp->shadowDepth : 3;
	const int                          numSearchElement = 2*shadowDepth-1;
	const int                          numSearchDepth = shadowDepth-1;

	Journal_DPrintf( context->debug, "In: %s\n", __func__ );

	ind = (int*)malloc( sizeof( int ) * nNodeX * nNodeZ );
	memset( ind, 0, sizeof( int ) * nNodeX * nNodeZ );

	while(!All_passed) {
		/* Calculate the new node coordintates for the top-internal nodes. */
		for( node_lK = 0; node_lK < nNodeZ; node_lK++ )
			for( node_lII = 0; node_lII < nNodeX; node_lII++ ) {

				Element_NodeIndex		en_N;
				double				elembbox[2][DIM];
				Coord                           temp1,temp2;
				Node_LocalIndex**		elementNodeTbl = context->mesh->elementNodeTbl;
				Tetrahedra_Index		tetra_I;
				Coord				initialNodeCoord;
				Coord				initialNodeCoordS;
                Element_LocalIndex      searchElement_lI,searchElement_lK;
				IJK                             ijk;

				/* Ensure we haven't already done this node */
				node_lI = node_lII + (nNodeY-1)*nNodeX + node_lK*nNodeX*nNodeY;
				RegularMeshUtils_Node_Local1DTo3D( decomp, node_lI, &ijk[0], &ijk[1], &ijk[2] );
				element_lI = (ijk[0] <= 1)? 0 : (ijk[0]-1);
				element_lK = (ijk[2] <= 1)? 0 : (ijk[2]-1);

				if( ind[ijk[0]+ijk[2]*nNodeX] ) {
					continue;
				}

				initialNodeCoord[0] = meshExt->initialNodeCoord[node_lI][0];
				initialNodeCoord[1] = meshExt->initialNodeCoord[node_lI][1];
				initialNodeCoord[2] = meshExt->initialNodeCoord[node_lI][2];
				xyz2tprCoord( initialNodeCoord, &initialNodeCoordS );
				initialNodeCoordS[1] = 0.0f;

				for(searchElement_lK=0;searchElement_lK<numSearchElement;searchElement_lK++) {
					if( (element_lK+searchElement_lK-numSearchDepth) < 0 || (element_lK+searchElement_lK-numSearchDepth) >= nElementZ )
						continue;
					for(searchElement_lI=0;searchElement_lI<numSearchElement;searchElement_lI++) {
						if( (element_lI+searchElement_lI-numSearchDepth) < 0 || (element_lI+searchElement_lI-numSearchDepth) >= nElementX )
							continue;
						element_lN = (element_lI+searchElement_lI-numSearchDepth) + (nElementY-1)*nElementX +
							(element_lK+searchElement_lK-numSearchDepth)*nElementY*nElementX;

						elementNodeCount = context->mesh->elementNodeCountTbl[element_lN];
						if( elementNodeCount != (unsigned)NODES_PER_ELEMENT ) {
							printf( "elementNodeCount != NODES_PER_ELEMENT  element_lN = %d", element_lN );
							assert( 0 );
						}

						if( ind[ijk[0]+ijk[2]*nNodeX] )
							break;

						/* create a local copy of the veocity and coords for each of the element's nodes of the old mesh */
						for( en_N = 0; en_N < 4; en_N++) {
							node_lN = elementNodeTbl[element_lN][en_II[en_N]];
							for( d = 0; d < DIM; d++ ) {
								xc[en_II[en_N]*DIM+d] = Mesh_CoordAt( context->mesh, node_lN )[d];
							}
							// (0,1,2) <=> (x,y,z) <=> (fi,r,theta)
							temp1[0] = xc[en_II[en_N]*DIM+0];
							temp1[1] = xc[en_II[en_N]*DIM+1];
							temp1[2] = xc[en_II[en_N]*DIM+2];
							xyz2tprCoord( temp1, &temp2 );
							xcs[en_II[en_N]*DIM+0] = temp2[0];
							xcs[en_II[en_N]*DIM+1] = temp2[1];
							xcs[en_II[en_N]*DIM+2] = temp2[2];
						}

						/* Calculate element's bounding box */
						for( d = 0; d < DIM; d++ ) {
							elembbox[0][d] = DBL_MAX;
							elembbox[1][d] = DBL_MIN;
						}
						for( en_N = 0; en_N < 4; en_N++ ) {
							for( d = 0; d < DIM; ++d ) {
								elembbox[0][d] = MIN( elembbox[0][d], xcs[en_II[en_N]*DIM+d] );
								elembbox[1][d] = MAX( elembbox[1][d], xcs[en_II[en_N]*DIM+d] );
							}
						}
						/* If new mesh's current node is outside the bounding box, next loop */
						if( initialNodeCoordS[0] < elembbox[0][0]-0.5f*(elembbox[1][0]-elembbox[0][0]) ||
							initialNodeCoordS[0] > elembbox[1][0]+0.5f*(elembbox[1][0]-elembbox[0][0]) ||
							initialNodeCoordS[2] < elembbox[0][2]-0.5f*(elembbox[1][2]-elembbox[0][2]) ||
							initialNodeCoordS[2] > elembbox[1][2]+0.5f*(elembbox[1][2]-elembbox[0][2]) )
							{
								//assert(0);
								continue;
							}

						/* if the current coordinate is far enough from the initial position,
						   try to interpolate surface topo. */
						/* loop over 2 sub triangles in a rectangular element, work out the triangle it is in, and then interpolate */
						for( tetra_I = 0; tetra_I < 2; tetra_I++ ) {
							double				x1[DIM];
							double				x2[DIM];
							double				x3[DIM];
							double				y1,y2,y3;
							double				dett;
							double				det[3];
							Coord				axisA,axisB,axisC;
							unsigned int		found=0;

							for( d = 0; d < DIM; d++ ) {
								x1[d] = xcs[nn[tetra_I][0]*DIM+d];
								x2[d] = xcs[nn[tetra_I][1]*DIM+d];
								x3[d] = xcs[nn[tetra_I][2]*DIM+d];
							}
							y1 = x1[1];
							y2 = x2[1];
							y3 = x3[1];
							x1[1] = x2[1] = x3[1] = 0.0f;

							Vector_Sub( axisA, x2, x1 );
							Vector_Sub( axisB, x3, x1 );
							Vector_Cross( axisC, axisA, axisB );
							dett = 0.5*getSignedArea( axisC );
							dett = isLargerThanZero2(dett, dett, TOL);

							Vector_Sub( axisA, x2, initialNodeCoordS );
							Vector_Sub( axisB, x3, initialNodeCoordS );
							Vector_Cross( axisC, axisA, axisB );
							det[0] = 0.5*getSignedArea( axisC );
							det[0] = isLargerThanZero2(det[0], dett, TOL);
							Vector_Sub( axisA, x3, initialNodeCoordS );
							Vector_Sub( axisB, x1, initialNodeCoordS );
							Vector_Cross( axisC, axisA, axisB );
							det[1] = 0.5*getSignedArea( axisC );
							det[1] = isLargerThanZero2(det[1], dett, TOL);
							Vector_Sub( axisA, x1, initialNodeCoordS );
							Vector_Sub( axisB, x2, initialNodeCoordS );
							Vector_Cross( axisC, axisA, axisB );
							det[2] = 0.5*getSignedArea( axisC );
							det[2] = isLargerThanZero2(det[2], dett, TOL);
							dett = det[0] + det[1] + det[2];

							if( dett < 0 )	{
								continue;
							}
							/* found if all det are greater than zero */
							if( det[0] >= 0.0f && det[1] >= 0.0f && det[2] >= 0.0f ) {
								found = 1;
							}
							if( found ) {
								double				shape[3];
								Index				tNode_I;
								/* mark, such that we dont do it again */
								ind[ijk[0]+ijk[2]*nNodeX] = 1;

								/* Calculate the shape funcs */

								for( tNode_I = 0; tNode_I < 3; tNode_I++ ) {
									shape[tNode_I] = det[tNode_I] / dett;
								}

								x1[0] = initialNodeCoordS[0];
								x1[1] = y1*shape[0] + y2*shape[1] + y3*shape[2];
								x1[2] = initialNodeCoordS[2];
								tpr2xyzCoord( x1,&meshExt->newNodeCoord[node_lI] );

								break;
							}
#if 0
							if(context->rank==0 && node_lI==0 && TOL <= 1.0e-7)
								fprintf(stderr,"Found? %d TOL=%e timeStep=%d me=%d element_lI=%d node_lI=%d tetra_I=%d\nxc=(%e %e %e) (%e %e %e)(%e %e %e) initialX=%e %e %e\nbbox=(%e %e) (%e %e)\ndett=%e %e %e %e min=%e max=%e result=%e\n\n",
										found,TOL,context->timeStep,context->rank,element_lN,node_lI,tetra_I,
										x1[0],x1[1],x1[2],x2[0],x2[1],x2[2],x3[0],x3[1],x3[2],
										initialNodeCoordS[0],initialNodeCoordS[1],initialNodeCoordS[2],
										elembbox[0][0],elembbox[1][0],elembbox[0][2],elembbox[1][2],
										det[0],det[1],det[2],dett,-1.0*fabs(dett)*TOL,fabs(dett)*TOL,
										isLargerThanZero2(det[0],dett,TOL));
#endif
						}
					}
				}
				/*
				** Loop over shadow elements
				*/
				for( rn_I = 0; rn_I < meshExt->neighbourRankCount; rn_I++ ) {
					Element_ShadowIndex		element_sN;

                    /* Loop over all the shadow elements to find one including node_lI */
                    for( element_sN = 0; element_sN < meshExt->shadowElementRemoteCount[rn_I]; element_sN++ ) {
                        Element_GlobalIndex		gElement_N = meshExt->shadowElementRemoteArray[rn_I][element_sN];
                        Node_Index*			shadowElementNodesN = (Node_Index*)malloc( sizeof(Node_Index) * NODES_PER_ELEMENT );
                        Index				en_N;
                        double				elembbox[2][DIM];
                        Node_GlobalIndex	elementNodeN[8];
                        IJK                 eijkN;

                        /* skip irrelevant elements or exit the loop once the interpolation is done (ind == 1). */
                        RegularMeshUtils_Element_1DTo3D( decomp, gElement_N, &eijkN[0], &eijkN[1], &eijkN[2] );
                        if(eijkN[1] != decomp->elementGlobal3DCounts[1]-1)
                            continue;
                        if(ind[ijk[0]+ijk[2]*nNodeX] == 1)
                            break;

                        /* Figure out node index */
                        _HexaEL_BuildCornerIndices( context->mesh->layout->elementLayout, gElement_N, elementNodeN );
                        for( en_N = 0; en_N < 8; en_N++ ) {
                            Node_GlobalIndex	node_gN = elementNodeN[en_N];
                            unsigned int		found = 0;
                            node_lN = Mesh_NodeMapGlobalToLocal( context->mesh, node_gN );
                            if( node_lN < meshExt->nodeLocalCount ) {
                                shadowElementNodesN[en_N] = node_lN;
                                found = 1;
                                elementNodeN[en_N] = node_lN;
                            }
                            else {
                                Node_ShadowIndex		node_sI;
                                for( node_sI = 0; node_sI < meshExt->shadowNodeRemoteCount[rn_I]; node_sI++ ) {
                                    if( node_gN == meshExt->shadowNodeRemoteArray[rn_I][node_sI] ) {
                                        shadowElementNodesN[en_N] =
                                            context->mesh->nodeGlobalCount +
                                            node_sI;
                                        found = 1;
                                        break;
                                    }
                                }
                            }
                            assert( found ); /* we expected this to be a shadow node, but wasn't in list! */
                        }

                        elementNodeCount = (unsigned)NODES_PER_ELEMENT; /* Assume all shadow elements have 8 nodes per el */
                        if( elementNodeCount != (unsigned)NODES_PER_ELEMENT ) {
                            printf( "elementNodeCount != NODES_PER_ELEMENT  element_sN = %d", element_sN );
                            assert( 0 );
                        }

                        /* create a local copy of the veocity and coords for each of the element's nodes of the old mesh */
                        for( en_N = 0; en_N < 4; en_N++) {
                            node_lN = shadowElementNodesN[en_II[en_N]];
                            if( node_lN < context->mesh->nodeDomainCount ) {
                                for( d = 0; d < DIM; d++ ) {
                                    xc[en_II[en_N]*DIM+d] = Mesh_CoordAt( context->mesh, node_lN )[d];
                                }
                            }
                            else {
                                node_lN -= context->mesh->nodeGlobalCount;
                                for( d = 0; d < DIM; d++ ) {
                                    xc[en_II[en_N]*DIM+d] = meshExt->shadowNodeRemoteCoord[rn_I][node_lN][d];
                                }
                            }
                            // (0,1,2) <=> (x,y,z) <=> (fi,r,theta)
                            temp1[0] = xc[en_II[en_N]*DIM+0];
                            temp1[1] = xc[en_II[en_N]*DIM+1];
                            temp1[2] = xc[en_II[en_N]*DIM+2];
                            xyz2tprCoord( temp1, &temp2 );
                            xcs[en_II[en_N]*DIM+0] = temp2[0];
                            xcs[en_II[en_N]*DIM+1] = temp2[1];
                            xcs[en_II[en_N]*DIM+2] = temp2[2];
                        }

                        /* Calculate element's bounding box */
                        for( d = 0; d < DIM; d++ ) {
                            elembbox[0][d] = DBL_MAX;
                            elembbox[1][d] = DBL_MIN;
                        }
                        for( en_N = 0; en_N < 4; en_N++ ) {
                            for( d = 0; d < DIM; ++d ) {
                                elembbox[0][d] = MIN( elembbox[0][d], xcs[en_II[en_N]*DIM+d] );
                                elembbox[1][d] = MAX( elembbox[1][d], xcs[en_II[en_N]*DIM+d] );
                            }
                        }

                        /* If new mesh's current node is outside the bounding box, next loop */
                        if( initialNodeCoordS[0] < elembbox[0][0]-0.5f*(elembbox[1][0]-elembbox[0][0]) ||
                            initialNodeCoordS[0] > elembbox[1][0]+0.5f*(elembbox[1][0]-elembbox[0][0]) ||
                            initialNodeCoordS[2] < elembbox[0][2]-0.5f*(elembbox[1][2]-elembbox[0][2]) ||
                            initialNodeCoordS[2] > elembbox[1][2]+0.5f*(elembbox[1][2]-elembbox[0][2]) )
                            {
                                continue;
                            }

                        /* if the current coord is very close to the initial ones, just copy those initial values rather than bothering to compute determinents and adding error */
                        if( ( initialNodeCoord[0] >= Mesh_CoordAt( context->mesh, node_lI )[0] - TOL*(elembbox[1][0]-elembbox[0][0]) && initialNodeCoord[0] <= Mesh_CoordAt( context->mesh, node_lI )[0] + TOL*(elembbox[1][0]-elembbox[0][0]) ) &&
                            ( initialNodeCoord[1] >= Mesh_CoordAt( context->mesh, node_lI )[1] - TOL*(elembbox[1][1]-elembbox[0][1]) && initialNodeCoord[1] <= Mesh_CoordAt( context->mesh, node_lI )[1] + TOL*(elembbox[1][1]-elembbox[0][1]) ) &&
                            ( initialNodeCoord[2] >= Mesh_CoordAt( context->mesh, node_lI )[2] - TOL*(elembbox[1][2]-elembbox[0][2]) && initialNodeCoord[2] <= Mesh_CoordAt( context->mesh, node_lI )[2] + TOL*(elembbox[1][2]-elembbox[0][2]) ) ) {
                            meshExt->newNodeCoord[node_lI][0] = initialNodeCoord[0];
                            meshExt->newNodeCoord[node_lI][1] = Mesh_CoordAt( context->mesh, node_lI )[1];
                            meshExt->newNodeCoord[node_lI][2] = initialNodeCoord[2];
                            ind[ijk[0]+ijk[2]*nNodeX] = 1;
                            continue;
                        }

                        /* if the current coordinate is far enough fromt the initial position,
                           try to interpolate surface topo. */
                        /* loop over 2 sub triangles in a rectangular element, work out the triangle it is in, and then interpolate */
                        for( tetra_I = 0; tetra_I < 2; tetra_I++ ) {
                            double				x1[DIM];
                            double				x2[DIM];
                            double				x3[DIM];
                            double				y1,y2,y3;
                            double				dett;
                            double				det[3];
							Coord				axisA,axisB,axisC;
							unsigned int		found=0;

                            for( d = 0; d < DIM; d++ ) {
                                x1[d] = xcs[nn[tetra_I][0]*DIM+d];
                                x2[d] = xcs[nn[tetra_I][1]*DIM+d];
                                x3[d] = xcs[nn[tetra_I][2]*DIM+d];
                            }
                            y1 = x1[1];
                            y2 = x2[1];
                            y3 = x3[1];
                            x1[1] = x2[1] = x3[1] = 0.0f;

                            Vector_Sub( axisA, x2, x1 );
                            Vector_Sub( axisB, x3, x1 );
                            Vector_Cross( axisC, axisA, axisB );
                            dett = 0.5*getSignedArea( axisC );
                            dett = isLargerThanZero2(dett, dett, TOL);


                            Vector_Sub( axisA, x2, initialNodeCoordS );
                            Vector_Sub( axisB, x3, initialNodeCoordS );
                            Vector_Cross( axisC, axisA, axisB );
                            det[0] = 0.5*getSignedArea( axisC );
                            det[0] = isLargerThanZero2(det[0], dett, TOL);
                            Vector_Sub( axisA, x3, initialNodeCoordS );
                            Vector_Sub( axisB, x1, initialNodeCoordS );
                            Vector_Cross( axisC, axisA, axisB );
                            det[1] = 0.5*getSignedArea( axisC );
                            det[1] = isLargerThanZero2(det[1], dett, TOL);
                            Vector_Sub( axisA, x1, initialNodeCoordS );
                            Vector_Sub( axisB, x2, initialNodeCoordS );
                            Vector_Cross( axisC, axisA, axisB );
                            det[2] = 0.5*getSignedArea( axisC );
                            det[2] = isLargerThanZero2(det[2], dett, TOL);
                            dett = det[0] + det[1] + det[2];
                            assert( dett > -1.0e-10 && dett != 0.0);

                            if( dett < 0 )	{
                                continue;
                            }
                            /* found if all det are greater than zero */
                            if( det[0] >= 0.0f && det[1] >= 0.0f && det[2] >= 0.0f ) {
                                found = 1;
                            }
                            if( found ) {
                                double				shape[3];
                                Index				tNode_I;

                                /* mark, such that we dont do it again */
                                ind[ijk[0]+ijk[2]*nNodeX] = 1;

                                /* Calculate the shape funcs */
                                for( tNode_I = 0; tNode_I < 3; tNode_I++ ) {
                                    shape[tNode_I] = det[tNode_I] / dett;
                                }

                                x1[0] = initialNodeCoordS[0];
                                x1[1] = y1*shape[0] + y2*shape[1] + y3*shape[2];
                                x1[2] = initialNodeCoordS[2];
                                tpr2xyzCoord( x1, &meshExt->newNodeCoord[node_lI] );
                            }
#if 0
                            if(context->rank==2 && node_lI==594)
                                fprintf(stderr,"Shadow Found? %d TOL=%e timeStep=%d me=%d element_lI=%d node_lI=%d tetra_I=%d\nxc=(%e %e %e) (%e %e %e)(%e %e %e) initialX=%e %e %e\nbbox=(%e %e) (%e %e)\ndett=%e %e %e %e\n\n",
                                        found,TOL,context->timeStep,context->rank,element_lN,node_lI,tetra_I,
                                        x1[0],x1[1],x1[2],x2[0],x2[1],x2[2],x3[0],x3[1],x3[2],
                                        initialNodeCoordS[0],initialNodeCoordS[1],initialNodeCoordS[2],
                                        elembbox[0][0],elembbox[1][0],elembbox[0][2],elembbox[1][2],
                                        det[0],det[1],det[2],dett);
#endif
                        }
                    }
                }
			}
		All_passed = 1;
		count = 0;
		for( node_lK = 0; node_lK < nNodeZ; node_lK++ )
			for( node_lII = 0; node_lII < nNodeX; node_lII++ ) {
				node_lI=node_lII+(nNodeY-1)*nNodeX+node_lK*nNodeX*nNodeY;
				if( ind[node_lII+node_lK*nNodeX] == 0 ) {
					count++;
					All_passed = 0;
				}
			}
		if(TOL==0.0) TOL = 1.0e-10;
		else TOL *= 10.0;
		Journal_DPrintf( context->debug," In: %s, rank=%d Increased tolerance for Top: %e count=%d\n", __func__, context->rank, TOL,count );
        Journal_Firewall( TOL <= 1.0e-03, context->snacError," In: %s, timeStep=%d rank=%d Increased tolerance for Top: %e count=%d\n", __func__, context->timeStep, context->rank, TOL,count );
	}

	/* Calculate the new node coordintates for the bottom-internal nodes. */
    if(contextExt->bottomRestore) {
        for( index = 0; index < meshExt->bottomInternalCount; index++ ) {
			node_lI = meshExt->bottomInternalArray[index];
			meshExt->newNodeCoord[node_lI][0] = meshExt->initialNodeCoord[node_lI][0];
			meshExt->newNodeCoord[node_lI][1] = meshExt->initialNodeCoord[node_lI][1];
			meshExt->newNodeCoord[node_lI][2] = meshExt->initialNodeCoord[node_lI][2];
        }
    }
    else {
        en_II[0] = 0;	en_II[1] = 1;	en_II[2] = 5;	en_II[3] = 4;
        nn[0][0] = 0; nn[0][1] = 4; nn[0][2] = 1;
        nn[1][0] = 1; nn[1][1] = 4; nn[1][2] = 5;
        memset( ind, 0, sizeof( int ) * nNodeX * nNodeZ );

        All_passed = 0;
        TOL= 0.0f;
        while(!All_passed) {
			for( node_lK = 0; node_lK < nNodeZ; node_lK++ )
				for( node_lII = 0; node_lII < nNodeX; node_lII++ ) {
                    Element_NodeIndex		en_N;
                    double				elembbox[2][DIM];
                    Coord                           temp1,temp2;
                    Node_LocalIndex**		elementNodeTbl = context->mesh->elementNodeTbl;
                    Element_LocalIndex      searchElement_lI,searchElement_lK;
					Tetrahedra_Index		tetra_I;
					Coord				initialNodeCoord;
					Coord				initialNodeCoordS;
					IJK                             ijk;

					node_lI = node_lII + node_lK*nNodeX*nNodeY;
					RegularMeshUtils_Node_Local1DTo3D( decomp, node_lI, &ijk[0], &ijk[1], &ijk[2] );
					element_lI = (ijk[0] <= 1)? 0 : (ijk[0]-1);
					element_lK = (ijk[2] <= 1)? 0 : (ijk[2]-1);

					if( ind[ijk[0]+ijk[2]*nNodeX] ) {
						continue;
					}

					initialNodeCoord[0] = meshExt->initialNodeCoord[node_lI][0];
					initialNodeCoord[1] = meshExt->initialNodeCoord[node_lI][1];
					initialNodeCoord[2] = meshExt->initialNodeCoord[node_lI][2];
					xyz2tprCoord( initialNodeCoord, &initialNodeCoordS );
					initialNodeCoordS[1] = 0.0f;

					for(searchElement_lK=0;searchElement_lK<numSearchElement;searchElement_lK++) {
						if( (element_lK+searchElement_lK-numSearchDepth) < 0 || (element_lK+searchElement_lK-numSearchDepth) >= nElementZ )
							continue;
						for(searchElement_lI=0;searchElement_lI<numSearchElement;searchElement_lI++) {
							if( (element_lI+searchElement_lI-numSearchDepth) < 0 || (element_lI+searchElement_lI-numSearchDepth) >= nElementX )
								continue;
							element_lN = (element_lI+searchElement_lI-numSearchDepth) + (element_lK+searchElement_lK-numSearchDepth)*nElementY*nElementX;

							elementNodeCount = context->mesh->elementNodeCountTbl[element_lN];
							if( elementNodeCount != (unsigned)NODES_PER_ELEMENT ) {
								printf( "elementNodeCount != NODES_PER_ELEMENT  element_lN = %d", element_lN );
								assert( 0 );
							}

							if( ind[ijk[0]+ijk[2]*nNodeX] )
								break;

							for( en_N = 0; en_N < 4; en_N++ ) {
								node_lN = elementNodeTbl[element_lN][en_II[en_N]];
								for( d = 0; d < DIM; d++ ) {
									xc[en_II[en_N]*DIM+d] = Mesh_CoordAt( context->mesh, node_lN )[d];
								}
								// (0,1,2) <=> (x,y,z) <=> (fi,r,theta)
								temp1[0] = xc[en_II[en_N]*DIM+0];
								temp1[1] = xc[en_II[en_N]*DIM+1];
								temp1[2] = xc[en_II[en_N]*DIM+2];
								xyz2tprCoord( temp1, &temp2 );
								xcs[en_II[en_N]*DIM+0] = temp2[0];
								xcs[en_II[en_N]*DIM+1] = temp2[1];
								xcs[en_II[en_N]*DIM+2] = temp2[2];
							}

							/* Calculate element's bounding box */
							for( d = 0; d < DIM; d++ ) {
								elembbox[0][d] = DBL_MAX;
								elembbox[1][d] = DBL_MIN;
							}
							for( en_N = 0; en_N < 4; en_N++ ) {
								for( d = 0; d < DIM; ++d ) {
									elembbox[0][d] = MIN( elembbox[0][d], xcs[en_II[en_N]*DIM+d] );
									elembbox[1][d] = MAX( elembbox[1][d], xcs[en_II[en_N]*DIM+d] );
								}
							}
							/* If new mesh's current node is outside the bounding box, next loop */
							if( initialNodeCoordS[0] < elembbox[0][0]-0.5f*(elembbox[1][0]-elembbox[0][0]) ||
								initialNodeCoordS[0] > elembbox[1][0]+0.5f*(elembbox[1][0]-elembbox[0][0]) ||
								initialNodeCoordS[2] < elembbox[0][2]-0.5f*(elembbox[1][2]-elembbox[0][2]) ||
								initialNodeCoordS[2] > elembbox[1][2]+0.5f*(elembbox[1][2]-elembbox[0][2]) )
								{
									continue;
								}

							/* if the current coordinate is far enough fromt the initial position,
							   try to interpolate surface topo. */
							/* loop over 2 sub triangles in a rectangular element, work out the triangle it is in, and then interpolate */
							for( tetra_I = 0; tetra_I < 2; tetra_I++ ) {
								double				x1[DIM];
								double				x2[DIM];
								double				x3[DIM];
								double				y1,y2,y3;
								double				dett;
								double				det[3];
								Coord				axisA,axisB,axisC;
								unsigned int		found=0;

								for( d = 0; d < DIM; d++ ) {
									x1[d] = xcs[nn[tetra_I][0]*DIM+d];
									x2[d] = xcs[nn[tetra_I][1]*DIM+d];
									x3[d] = xcs[nn[tetra_I][2]*DIM+d];
								}
								y1 = x1[1];
								y2 = x2[1];
								y3 = x3[1];
								x1[1] = x2[1] = x3[1] = 0.0f;

								Vector_Sub( axisA, x2, x1 );
								Vector_Sub( axisB, x3, x1 );
								Vector_Cross( axisC, axisA, axisB );
								dett = 0.5*getSignedArea( axisC );
								dett = isLargerThanZero2(dett, dett, TOL);

								Vector_Sub( axisA, x2, initialNodeCoordS );
								Vector_Sub( axisB, x3, initialNodeCoordS );

								Vector_Cross( axisC, axisA, axisB );
								det[0] = 0.5*getSignedArea( axisC );
								det[0] = isLargerThanZero2(det[0], dett, TOL);
								Vector_Sub( axisA, x3, initialNodeCoordS );
								Vector_Sub( axisB, x1, initialNodeCoordS );
								Vector_Cross( axisC, axisA, axisB );
								det[1] = 0.5*getSignedArea( axisC );
								det[1] = isLargerThanZero2(det[1], dett, TOL);
								Vector_Sub( axisA, x1, initialNodeCoordS );
								Vector_Sub( axisB, x2, initialNodeCoordS );
								Vector_Cross( axisC, axisA, axisB );
								det[2] = 0.5*getSignedArea( axisC );
								det[2] = isLargerThanZero2(det[2], dett, TOL);
								dett = det[0] + det[1] + det[2];

								if( dett <= 0 )	{
									printf( "Determinant evaluation is wrong node=%d\t xt[0]=%g \t xt[1]=%g \t xt[t]=%g\n",
											en_N,
											initialNodeCoordS[0],
											initialNodeCoordS[1],
											initialNodeCoordS[2]);
									//assert(0);
									continue;
								}
								/* found if all det are greater than zero */
								if( det[0] >= 0.0f && det[1] >= 0.0f && det[2] >= 0.0f ) {
									found = 1;
								}
								if( found ) {
									double				shape[3];
									Index				tNode_I;

									/* mark, such that we dont do it again */
									ind[ijk[0]+ijk[2]*nNodeX] = 1;

									/* Calculate the shape funcs */
									for( tNode_I = 0; tNode_I < 3; tNode_I++ ) {
										shape[tNode_I] = det[tNode_I] / dett;
									}

									x1[0] = initialNodeCoordS[0];
									x1[1] = y1*shape[0] + y2*shape[1] + y3*shape[2];
									x1[2] = initialNodeCoordS[2];
									tpr2xyzCoord( x1, &meshExt->newNodeCoord[node_lI] );

									break;
								}
#if 0
								if(context->rank==0 && node_lI==0)
									fprintf(stderr,"Found? %d timeStep=%d me=%d element_lI=%d node_lI=%d tetra_I=%d\nxc=(%e %e %e) (%e %e %e)(%e %e %e) initialX=%e %e %e\nbbox=(%e %e) (%e %e)\ndett=%e %e %e %e\n\n",
											found,context->timeStep,context->rank,element_lN,node_lI,tetra_I,
											x1[0],x1[1],x1[2],x2[0],x2[1],x2[2],x3[0],x3[1],x3[2],
											initialNodeCoordS[0],initialNodeCoordS[1],initialNodeCoordS[2],
											elembbox[0][0],elembbox[1][0],elembbox[0][2],elembbox[1][2],
											det[0],det[1],det[2],dett);
#endif
							}
						}
					}
					/*
					** Loop over shadow elements
					*/
					for( rn_I = 0; rn_I < meshExt->neighbourRankCount; rn_I++ ) {
						Element_ShadowIndex		element_sN;

                        /* Loop over all the shadow elements to find one including node_lI */
                        for( element_sN = 0; element_sN < meshExt->shadowElementRemoteCount[rn_I]; element_sN++ ) {
                            Element_GlobalIndex		gElement_N = meshExt->shadowElementRemoteArray[rn_I][element_sN];
                            Node_Index*			shadowElementNodesN = (Node_Index*)malloc( sizeof(Node_Index) * NODES_PER_ELEMENT );
                            Index				en_N;
                            double				elembbox[2][DIM];
                            Node_GlobalIndex	elementNodeN[8];
                            IJK                 eijkN;

                            /* skip irrelevant elements or exit the loop once the interpolation is done (ind == 1). */
                            RegularMeshUtils_Element_1DTo3D( decomp, gElement_N, &eijkN[0], &eijkN[1], &eijkN[2] );
                            if(eijkN[1] != 0)
                                continue;
                            if(ind[ijk[0]+ijk[2]*nNodeX] == 1)
                                break;

                            /* Figure out node index */
                            _HexaEL_BuildCornerIndices( context->mesh->layout->elementLayout, gElement_N, elementNodeN );
                            for( en_N = 0; en_N < 8; en_N++ ) {
                                Node_GlobalIndex	node_gN = elementNodeN[en_N];
                                unsigned int		found = 0;
                                node_lN = Mesh_NodeMapGlobalToLocal( context->mesh, node_gN );
                                if( node_lN < meshExt->nodeLocalCount ) {
                                    shadowElementNodesN[en_N] = node_lN;
                                    found = 1;
                                    elementNodeN[en_N] = node_lN;
                                }
                                else {
                                    Node_ShadowIndex		node_sI;
                                    for( node_sI = 0; node_sI < meshExt->shadowNodeRemoteCount[rn_I]; node_sI++ ) {
                                        if( node_gN == meshExt->shadowNodeRemoteArray[rn_I][node_sI] ) {
                                            shadowElementNodesN[en_N] =
                                                context->mesh->nodeGlobalCount +
                                                node_sI;
                                            found = 1;
                                            break;
                                        }
                                    }
                                }
                                assert( found ); /* we expected this to be a shadow node, but wasn't in list! */
                            }

                            elementNodeCount = (unsigned)NODES_PER_ELEMENT; /* Assume all shadow elements have 8 nodes per el */
                            if( elementNodeCount != (unsigned)NODES_PER_ELEMENT ) {
                                printf( "elementNodeCount != NODES_PER_ELEMENT  element_sN = %d", element_sN );
                                assert( 0 );
                            }


                            /* create a local copy of the veocity and coords for each of the element's nodes of the old mesh */
                            for( en_N = 0; en_N < 4; en_N++) {
                                node_lN = shadowElementNodesN[en_II[en_N]];
                                if( node_lN < context->mesh->nodeDomainCount ) {
                                    for( d = 0; d < DIM; d++ ) {
                                        xc[en_II[en_N]*DIM+d] = Mesh_CoordAt( context->mesh, node_lN )[d];
                                    }
                                }
                                else {
                                    node_lN -= context->mesh->nodeGlobalCount;
                                    for( d = 0; d < DIM; d++ ) {
                                        xc[en_II[en_N]*DIM+d] = meshExt->shadowNodeRemoteCoord[rn_I][node_lN][d];
                                    }
                                }
                                // (0,1,2) <=> (x,y,z) <=> (fi,r,theta)
                                temp1[0] = xc[en_II[en_N]*DIM+0];
                                temp1[1] = xc[en_II[en_N]*DIM+1];
                                temp1[2] = xc[en_II[en_N]*DIM+2];
                                xyz2tprCoord( temp1, &temp2 );
                                xcs[en_II[en_N]*DIM+0] = temp2[0];
                                xcs[en_II[en_N]*DIM+1] = temp2[1];
                                xcs[en_II[en_N]*DIM+2] = temp2[2];
                            }

                            /* Calculate element's bounding box */
                            for( d = 0; d < DIM; d++ ) {
                                elembbox[0][d] = DBL_MAX;
                                elembbox[1][d] = DBL_MIN;
                            }
                            for( en_N = 0; en_N < 4; en_N++ ) {
                                for( d = 0; d < DIM; ++d ) {
                                    elembbox[0][d] = MIN( elembbox[0][d], xcs[en_II[en_N]*DIM+d] );
                                    elembbox[1][d] = MAX( elembbox[1][d], xcs[en_II[en_N]*DIM+d] );
                                }
                            }
                            /* If new mesh's current node is outside the bounding box, next loop */
                            if( initialNodeCoordS[0] < elembbox[0][0]-0.5f*(elembbox[1][0]-elembbox[0][0]) ||
                                initialNodeCoordS[0] > elembbox[1][0]+0.5f*(elembbox[1][0]-elembbox[0][0]) ||
                                initialNodeCoordS[2] < elembbox[0][2]-0.5f*(elembbox[1][2]-elembbox[0][2]) ||
                                initialNodeCoordS[2] > elembbox[1][2]+0.5f*(elembbox[1][2]-elembbox[0][2]) )
                                {
                                    continue;
                                }

                            /* if the current coord is very close to the initial ones, just copy those initial values rather than bothering to compute determinents and adding error */
                            if( ( initialNodeCoord[0] >= Mesh_CoordAt( context->mesh, node_lI )[0] - TOL*(elembbox[1][0]-elembbox[0][0]) && initialNodeCoord[0] <= Mesh_CoordAt( context->mesh, node_lI )[0] + TOL*(elembbox[1][0]-elembbox[0][0]) ) &&
                                ( initialNodeCoord[1] >= Mesh_CoordAt( context->mesh, node_lI )[1] - TOL*(elembbox[1][1]-elembbox[0][1]) && initialNodeCoord[1] <= Mesh_CoordAt( context->mesh, node_lI )[1] + TOL*(elembbox[1][1]-elembbox[0][1]) ) &&
                                ( initialNodeCoord[2] >= Mesh_CoordAt( context->mesh, node_lI )[2] - TOL*(elembbox[1][2]-elembbox[0][2]) && initialNodeCoord[2] <= Mesh_CoordAt( context->mesh, node_lI )[2] + TOL*(elembbox[1][2]-elembbox[0][2]) ) ) {
                                meshExt->newNodeCoord[node_lI][0] = initialNodeCoord[0];
                                meshExt->newNodeCoord[node_lI][1] = Mesh_CoordAt( context->mesh, node_lI )[1];
                                meshExt->newNodeCoord[node_lI][2] = initialNodeCoord[2];
                                ind[ijk[0]+ijk[2]*nNodeX] = 1;
                                continue;
                            }

                            /* if the current coordinate is far enough fromt the initial position,
                               try to interpolate surface topo. */
                            /* loop over 2 sub triangles in a rectangular element, work out the triangle it is in, and then interpolate */
                            for( tetra_I = 0; tetra_I < 2; tetra_I++ ) {
                                double				x1[DIM];
                                double				x2[DIM];
                                double				x3[DIM];
                                double				y1,y2,y3;
                                double				dett;
                                double				det[3];
								Coord				axisA,axisB,axisC;
								unsigned int		found=0;

                                for( d = 0; d < DIM; d++ ) {
                                    x1[d] = xcs[nn[tetra_I][0]*DIM+d];
                                    x2[d] = xcs[nn[tetra_I][1]*DIM+d];
                                    x3[d] = xcs[nn[tetra_I][2]*DIM+d];
                                }
                                y1 = x1[1];
                                y2 = x2[1];
                                y3 = x3[1];
                                x1[1] = x2[1] = x3[1] = 0.0f;

                                Vector_Sub( axisA, x2, x1 );
                                Vector_Sub( axisB, x3, x1 );
                                Vector_Cross( axisC, axisA, axisB );
                                dett = 0.5*getSignedArea( axisC );
                                dett = isLargerThanZero2(dett, dett, TOL);

                                Vector_Sub( axisA, x2, initialNodeCoordS );
                                Vector_Sub( axisB, x3, initialNodeCoordS );
                                Vector_Cross( axisC, axisA, axisB );
                                det[0] = 0.5*getSignedArea( axisC );
                                det[0] = isLargerThanZero2(det[0], dett, TOL);
                                Vector_Sub( axisA, x3, initialNodeCoordS );
                                Vector_Sub( axisB, x1, initialNodeCoordS );
                                Vector_Cross( axisC, axisA, axisB );
                                det[1] = 0.5*getSignedArea( axisC );
                                det[1] = isLargerThanZero2(det[1], dett, TOL);
                                Vector_Sub( axisA, x1, initialNodeCoordS );
                                Vector_Sub( axisB, x2, initialNodeCoordS );
                                Vector_Cross( axisC, axisA, axisB );
                                det[2] = 0.5*getSignedArea( axisC );
                                det[2] = isLargerThanZero2(det[2], dett, TOL);
                                dett = det[0] + det[1] + det[2];

                                assert( dett > 0.0);
                                if( dett <= 0 )	{
									continue;
                                }
                                /* found if all det are greater than zero */
                                if( det[0] >= 0.0f && det[1] >= 0.0f && det[2] >= 0.0f ) {
                                    found = 1;
                                }
                                if( found ) {
                                    double				shape[3];
                                    Index				tNode_I;

                                    /* mark, such that we dont do it again */
                                    RegularMeshUtils_Node_Local1DTo3D( decomp, node_lI, &ijk[0], &ijk[1], &ijk[2] );
                                    ind[ijk[0]+ijk[2]*nNodeX] = 1;

                                    /* Calculate the shape funcs */
                                    for( tNode_I = 0; tNode_I < 3; tNode_I++ ) {
                                        shape[tNode_I] = det[tNode_I] / dett;
                                    }

                                    x1[0] = initialNodeCoordS[0];
                                    x1[1] = y1*shape[0] + y2*shape[1] + y3*shape[2];
                                    x1[2] = initialNodeCoordS[2];
                                    tpr2xyzCoord( x1, &meshExt->newNodeCoord[node_lI] );
                                }
#if 0
                                if(context->rank==0 && node_lI==0)
                                    fprintf(stderr,"Shadow Found? %d timeStep=%d me=%d element_lI=%d node_lI=%d tetra_I=%d\nxc=(%e %e %e) (%e %e %e)(%e %e %e) initialX=%e %e %e\nbbox=(%e %e) (%e %e)\ndett=%e %e %e %e\n\n",
                                            found,context->timeStep,context->rank,element_lN,node_lI,tetra_I,
                                            x1[0],x1[1],x1[2],x2[0],x2[1],x2[2],x3[0],x3[1],x3[2],
                                            initialNodeCoordS[0],initialNodeCoordS[1],initialNodeCoordS[2],
                                            elembbox[0][0],elembbox[1][0],elembbox[0][2],elembbox[1][2],
                                            det[0],det[1],det[2],dett);
#endif
                            }
                        }
                    }
                }
			All_passed = 1;
			count = 0;
			for( node_lK = 0; node_lK < nNodeZ; node_lK++ )
				for( node_lII = 0; node_lII < nNodeX; node_lII++ ) {
					node_lI=node_lII+(nNodeY-1)*nNodeX+node_lK*nNodeX*nNodeY;
					if( ind[node_lII+node_lK*nNodeX] == 0 ) {
						All_passed = 0;
						count++;
						//fprintf(stderr,"me=%d failed Bottom nodes=%d\n",context->rank,node_lI);
					}
				}
			if(TOL==0.0)TOL = 1.0e-10;
			else TOL *= 10.0;
			Journal_DPrintf( context->debug," In: %s, rank=%d Increased tolerance for Bottom: %e count=%d\n", __func__, context->rank, TOL, count );
			Journal_Firewall( TOL <= 1.0e-03, context->snacError," In: %s, timeStep=%d rank=%d Increased tolerance for Bottom: %e count=%d\n", __func__, context->timeStep, context->rank, TOL,count );
		}
	}
	free( ind );

	/* Calculate the new node coordintates for the wall nodes. */
	if( meshExt->meshType == -1 ) {
		assert( 0 );
	}
	else {
		for( index = 0; index < meshExt->wallCount; index++ ) {
			Node_LocalIndex				node_lI;
			IJK					ijk;
			Node_LocalIndex				nodeTop_lI, nodeBottom_lI;
			const Index 				topI = decomp->nodeLocal3DCounts[decomp->rank][1];
			Coord                                   temp,tempTop,tempBottom;
			node_lI = meshExt->wallArray[index];
			RegularMeshUtils_Node_Local1DTo3D( decomp, node_lI, &ijk[0], &ijk[1], &ijk[2] );

			/* The wall node positions revert to the initial position */
			nodeBottom_lI = RegularMeshUtils_Node_Local3DTo1D( decomp, ijk[0], 0, ijk[2] );
			nodeTop_lI = RegularMeshUtils_Node_Local3DTo1D( decomp, ijk[0], topI - 1, ijk[2] );

			xyz2tprCoord( meshExt->initialNodeCoord[node_lI], &temp );
			xyz2tprCoord( meshExt->newNodeCoord[nodeTop_lI], &tempTop );
			xyz2tprCoord( meshExt->newNodeCoord[nodeBottom_lI], &tempBottom );

			temp[1] = tempBottom[1] + (tempTop[1] - tempBottom[1])/(topI - 1 )*ijk[1];

			tpr2xyzCoord( temp, &meshExt->newNodeCoord[node_lI] );
		}
	}

	/* Calculate the new node coordintates for the internal nodes. */
	for( index = 0; index < meshExt->internalCount; index++ ) {
		Node_LocalIndex				node_lI;
		IJK					ijk;
		Node_LocalIndex				nodeBottom_lI, nodeLeft_lI, nodeFront_lI;
		Node_LocalIndex				nodeTop_lI, nodeRight_lI, nodeBack_lI;
		const Index 				topI = decomp->nodeLocal3DCounts[decomp->rank][1];
		const Index				rightI = decomp->nodeLocal3DCounts[decomp->rank][0];
		const Index				frontI = decomp->nodeLocal3DCounts[decomp->rank][2];
		Coord                                   temp,tempTop,tempBottom,tempLeft,tempRight,tempBack,tempFront;

		/* TODO... parallel version! */
		node_lI = meshExt->internalArray[index];
		RegularMeshUtils_Node_Local1DTo3D( decomp, node_lI, &ijk[0], &ijk[1], &ijk[2] );
		/* TODO... Local? where's the global equivalent? */
		nodeBottom_lI = RegularMeshUtils_Node_Local3DTo1D( decomp, ijk[0], 0, ijk[2] );
		nodeTop_lI = RegularMeshUtils_Node_Local3DTo1D( decomp, ijk[0], topI - 1, ijk[2] );
		nodeLeft_lI = RegularMeshUtils_Node_Local3DTo1D( decomp, 0, ijk[1], ijk[2] );
		nodeRight_lI = RegularMeshUtils_Node_Local3DTo1D( decomp, rightI - 1, ijk[1], ijk[2] );
		nodeBack_lI = RegularMeshUtils_Node_Local3DTo1D( decomp, ijk[0], ijk[1], 0 );
		nodeFront_lI = RegularMeshUtils_Node_Local3DTo1D( decomp, ijk[0], ijk[1], frontI - 1 );
		xyz2tprCoord( meshExt->newNodeCoord[nodeTop_lI], &tempTop );
		xyz2tprCoord( meshExt->newNodeCoord[nodeBottom_lI], &tempBottom );
		xyz2tprCoord( meshExt->initialNodeCoord[nodeLeft_lI], &tempLeft );
		xyz2tprCoord( meshExt->initialNodeCoord[nodeRight_lI], &tempRight );
		xyz2tprCoord( meshExt->initialNodeCoord[nodeBack_lI], &tempBack );
		xyz2tprCoord( meshExt->initialNodeCoord[nodeFront_lI], &tempFront );

		temp[0] = tempLeft[0] + (tempRight[0] - tempLeft[0])/(rightI - 1 )*ijk[0];
		temp[1] = tempBottom[1] + (tempTop[1] - tempBottom[1])/(topI - 1 )*ijk[1];
		temp[2] = tempBack[2] + (tempFront[2] - tempBack[2])/(frontI - 1 )*ijk[2];

		tpr2xyzCoord( temp, &meshExt->newNodeCoord[node_lI] );
	}
}

double isLargerThanZero(double a, double TOL)
{
	if(a>=-1.0*TOL && a<=TOL)
		return 0.0f;
	else
		return a;
}
double isLargerThanZero2(double a, double refval, double TOL)
{
	if(a>=-1.0*fabs(refval)*TOL && a<=fabs(refval)*TOL)
		return 0.0f;
	else
		return a;
}
double getSignedArea( Coord a )
{
	if(a[1] >= 0.0)
		return Vector_Mag(a);
	else
		return -1.0f*Vector_Mag(a);
}
void xyz2tprCoord( Coord x, Coord* dest )
{
	(*dest)[0] = atan2(x[1],x[0]);
	(*dest)[1] = sqrt(x[0]*x[0]+x[1]*x[1]+x[2]*x[2]);
	(*dest)[2] = acos(x[2]/(*dest)[1]);
}
void tpr2xyzCoord( Coord s, Coord* dest )
{
	(*dest)[0] = s[1]*sin(s[2])*cos(s[0]);
	(*dest)[1] = s[1]*sin(s[2])*sin(s[0]);
	(*dest)[2] = s[1]*cos(s[2]);
}
void xyz2tprVel( Coord x, Coord vx, Coord* dest )
{
	Coord s;
	xyz2tprCoord( x, &s );
	(*dest)[0] = -1.0f*vx[0]*sin(s[0])+vx[1]*cos(s[0]);
	(*dest)[1] = vx[0]*sin(s[2])*cos(s[0])+vx[1]*sin(s[2])*sin(s[0])+vx[2]*cos(s[2]);
	(*dest)[2] = vx[0]*cos(s[2])*cos(s[0])+vx[1]*cos(s[2])*sin(s[0])-vx[2]*sin(s[2]);
}
void tpr2xyzVel( Coord s, Coord vs, Coord* dest )
{
	(*dest)[0] = vs[1]*sin(s[2])*cos(s[0])+vs[2]*cos(s[2])*cos(s[0])-vs[0]*sin(s[0]);
	(*dest)[1] = vs[1]*sin(s[2])*sin(s[0])+vs[2]*cos(s[2])*sin(s[0])+vs[0]*cos(s[0]);
	(*dest)[2] = vs[1]*cos(s[2])-vs[2]*sin(s[2]);
}
#endif

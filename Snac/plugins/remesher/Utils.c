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
** $Id: Utils.c 2749 2005-03-06 06:43:47Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdio.h>
#include <mpi.h>
#include <string.h>

#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>

#include "Snac/Snac.h"
#include "types.h"
#include "EntryPoint.h"
#include "Mesh.h"
#include "Context.h"
#include "Remesh.h"
#include "Register.h"


Node_DomainIndex findClosestNode( void* _context, Coord point, Node_LocalIndex refNodeInd ) {
	Snac_Context*			context = (Snac_Context*)_context;
	Mesh*				mesh = context->mesh;
	Topology*				nodeTopo = mesh->layout->nodeLayout->topology;
	Node_DomainIndex		curNode = refNodeInd;
	Bool					done = False;
	
	
	/*
	** Search through the mesh's old coordinates to find the closest node to the new node 'newNode'.
	*/
	
	do {
		unsigned			nNbrs;
		Node_DomainIndex*	nbrs;
		Coord			tmp;
		double			minDist;
		Node_DomainIndex	minNode;
		unsigned			nbr_i;
		
		/* Collect the neighbours of the current node. */
		{
			Node_GlobalIndex	gNodeInd;
			
			gNodeInd = Mesh_NodeMapDomainToGlobal( mesh, curNode );
			nNbrs = Topology_NeighbourCount( nodeTopo, gNodeInd );
			if( nNbrs ) {
				nbrs = Memory_Alloc_Array( Node_DomainIndex, nNbrs, "SnacRemesher" );
				Topology_BuildNeighbours( nodeTopo, curNode, nbrs );
			}
			else {
				nbrs = NULL;
			}
		}
		
		/* Convert neighbours to domain indices. */
		for( nbr_i = 0; nbr_i < nNbrs; nbr_i++ ) {
			nbrs[nbr_i] = Mesh_NodeMapGlobalToDomain( mesh, nbrs[nbr_i] );
		}
		
		/* Get the distance to the current node. */
		Vector_Sub( tmp, point, mesh->nodeCoord[curNode] );
		minDist = Vector_Mag( tmp );
		minNode = curNode;
		
		/* Loop over neighbours to find the closest node. */
		for( nbr_i = 0; nbr_i < nNbrs; nbr_i++ ) {
			double	dist;
			
			if( nbrs[nbr_i] >= mesh->nodeDomainCount ) {
				continue;
			}
			
			Vector_Sub( tmp, point, mesh->nodeCoord[nbrs[nbr_i]] );
			dist = Vector_Mag( tmp );
			
			if( dist < minDist ) {
				minDist = dist;
				minNode = nbrs[nbr_i];
			}
		}
		
		/* Have we found the closest node? */
		if( minNode == curNode ) {
			done = True;
		}
		else {
			curNode = minNode;
		}
		
		/* Free the neighbour array. */
		FreeArray( nbrs );
	}
	while( !done );
	
	return curNode;
}


/*
** Determine if 'point' is inside the element corresponding to 'dElementInd'.
*/

Bool pointInElement( void* _context, Coord point, Element_DomainIndex dElementInd ) {
	Snac_Context*		context = (Snac_Context*)_context;
	Mesh*			mesh = context->mesh;
	NodeLayout*		nLayout = mesh->layout->nodeLayout;
	unsigned		nEltNodes;
	Node_DomainIndex*	eltNodes;
	Coord			crds[8];
	double			bc[4];
	unsigned		inds[4];
	unsigned		eltNode_i;

	/* Extract the element's nodes, of which there should be eight. */
	{
		Element_GlobalIndex	gEltInd;
		
		nEltNodes = 8;
		eltNodes = Memory_Alloc_Array( Node_DomainIndex, nEltNodes, "SnacRemesher" );
		gEltInd = Mesh_ElementMapDomainToGlobal( mesh, dElementInd );
		nLayout->buildElementNodes( nLayout, gEltInd, eltNodes );
	}
	
	/* Convert the node indices back to domain values. */
	for( eltNode_i = 0; eltNode_i < nEltNodes; eltNode_i++ ) {
		eltNodes[eltNode_i] = Mesh_NodeMapGlobalToDomain( mesh, eltNodes[eltNode_i] );
	}

	/* Copy coordinates. */
	for( eltNode_i = 0; eltNode_i < nEltNodes; eltNode_i++ )
		memcpy( crds[eltNode_i], mesh->nodeCoord[eltNodes[eltNode_i]], sizeof(Coord) );

	if( _HexaEL_FindTetBarycenter( crds, point, bc, inds, INCLUSIVE_UPPER_BOUNDARY, NULL, 0 ) ) {
		return True;
	}

	return False;

#if 0

	unsigned				plane_i;

	
	const unsigned			planeInds[6][3] = { { 0, 4, 1 }, { 3, 2, 7 }, { 0, 3, 4 }, 
										{ 1, 5, 2 }, { 0, 1, 3 }, { 5, 4, 6 } };
	
	/* As we are dealing with hexahedral meshes with Snac, there will be six planes to check. */
	for( plane_i = 0; plane_i < 6; plane_i++ ) {
		Coord	norm;
		double	dist;
		
		/* Build the plane normal. */
		{
			Node_DomainIndex	inds[3];
			Coord			tmpA, tmpB;
			
			inds[0] = eltNodes[planeInds[plane_i][0]];
			inds[1] = eltNodes[planeInds[plane_i][1]];
			inds[2] = eltNodes[planeInds[plane_i][2]];
			
			Vector_Sub( tmpA, mesh->nodeCoord[inds[1]], mesh->nodeCoord[inds[0]] );
			Vector_Sub( tmpB, mesh->nodeCoord[inds[2]], mesh->nodeCoord[inds[0]] );
			Vector_Cross( norm, tmpA, tmpB );
			Vector_Norm( norm, norm );
		}
		
		/* Calc distance. */
		dist = Vector_Dot( norm, mesh->nodeCoord[planeInds[plane_i][0]] );
		
		/* Check which side of the plane our new point is. */
		if( Vector_Dot( norm, point ) <= dist ) {
			/* It is inside, so we may continue. */
			continue;
		}
		else {
			/* It's outside, so this is the wrong element. */
			break;
		}
	}
	
	/* Free the element nodes array. */
	FreeArray( eltNodes );
	
	/* Return appropriately. */
	
	return (plane_i < 6) ? False : True;
#endif
}

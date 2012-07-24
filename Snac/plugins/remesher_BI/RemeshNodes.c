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
** $Id: RemeshNodes.c 3259 2006-11-09 20:06:31Z EunseoChoi $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
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
#include <assert.h>
#include <float.h>


void _SnacRemesher_InterpolateNodes( void* _context ) {
	Snac_Context*		context = (Snac_Context*)_context;
	Mesh*				mesh = context->mesh;
	SnacRemesher_Mesh*	meshExt = ExtensionManager_Get( context->meshExtensionMgr,
															mesh,
															SnacRemesher_MeshHandle );
	NodeLayout*			nLayout = mesh->layout->nodeLayout;
	Node_LocalIndex		newNode_i;
	IndexSet*			extNodes;

	void interpolateNode( void* _context, Node_LocalIndex newNodeInd, Element_DomainIndex dEltInd );
	void SPR( void* _context );

	/*
	** Free any owned arrays that may still exist from the last node interpolation.
	*/

	FreeArray( meshExt->externalNodes );
	meshExt->nExternalNodes = 0;

	/*
	** Scoot over all the new nodes and find the old element in which each one resides, then interpolate.
	*/

	/* Create an index set for storing any external nodes. */
	extNodes = IndexSet_New( mesh->nodeLocalCount );

	for( newNode_i = 0; newNode_i < mesh->nodeLocalCount; newNode_i++ ) {
		Node_LocalIndex		dNodeInd;
		unsigned				nElements;
		Element_DomainIndex*	elements;
		Coord				newPoint;
		unsigned				elt_i;

		/* Extract the new node's coordinate. */
		Vector_Set( newPoint, meshExt->newNodeCoords[newNode_i] );

		/* Find the closest old node. */
		dNodeInd = findClosestNode( context, newPoint, newNode_i );

		/* Grab incident elements. */
		{
			Node_GlobalIndex	gNodeInd;

			gNodeInd = Mesh_NodeMapDomainToGlobal( mesh, dNodeInd );
			nElements = nLayout->nodeElementCount( nLayout, gNodeInd );
			if( nElements ) {
				elements = Memory_Alloc_Array( Element_DomainIndex, nElements, "SnacRemesher" );
				nLayout->buildNodeElements( nLayout, gNodeInd, elements );
			}
			else {
				elements = NULL;
			}
		}

		/* Convert global element indices to domain. */
		for( elt_i = 0; elt_i < nElements; elt_i++ ) {
			elements[elt_i] = Mesh_ElementMapGlobalToDomain( mesh, elements[elt_i] );
		}

		/* Which of the incident elements contains the node? */
		for( elt_i = 0; elt_i < nElements; elt_i++ ) {
			if( elements[elt_i] >= mesh->elementDomainCount ) {
				continue;
			}

			if( pointInElement( context, newPoint, elements[elt_i] ) ) {
				break;
			}
		}

		/* Did we find the element? */
		if( elt_i < nElements ) {
			/* If so, call a function to locate the tetrahedra and interpolate. */
			interpolateNode( context, newNode_i, elements[elt_i] );
		}
		else {
			/* If not, then the new node finds itself outside the old mesh.  In this scenario, we cannot interpolate
			   the nodal values with any accuracy (without knowing more about the physical problem).  So, we will leave
			   the node with its old values and mark this node as not being interpolated so the user may deal with it. */

			/* Stash the node index. */
			IndexSet_Add( extNodes, newNode_i );

			/* Copy across the old value. Note that this should be done using some other provided copy method. */
			memcpy( (unsigned char*)meshExt->newNodes + newNode_i * mesh->nodeExtensionMgr->finalSize,
				(unsigned char*)mesh->node + newNode_i * mesh->nodeExtensionMgr->finalSize,
				mesh->nodeExtensionMgr->finalSize );
			/* assert(0); */
		}

		/* Free element array. */
		FreeArray( elements );
	}

	/* Dump the external nodes and delete the set. */
	IndexSet_GetMembers( extNodes, &meshExt->nExternalNodes, &meshExt->externalNodes );
	Stg_Class_Delete( extNodes );
}


void interpolateNode( void* _context, Node_LocalIndex newNodeInd, Element_DomainIndex dEltInd ) {
	Snac_Context*			context = (Snac_Context*)_context;
	SnacRemesher_Context*	contextExt = ExtensionManager_Get( context->extensionMgr,
								   context,
								   SnacRemesher_ContextHandle );
	Mesh*				mesh = context->mesh;
	SnacRemesher_Mesh*		meshExt = ExtensionManager_Get( context->meshExtensionMgr,
									mesh,
									SnacRemesher_MeshHandle );
	NodeLayout*			nLayout = mesh->layout->nodeLayout;
	unsigned				nEltNodes;
	Node_DomainIndex*		eltNodes;
	Coord				newNodeCoord;
	Coord			crds[8];
	double			weights[4];
	unsigned		tetNodeInds[4];
	unsigned		eltNode_i;

	/* Extract the element's node indices.  Note that there should always be eight of these. */
	{
		Element_GlobalIndex	gEltInd;

		nEltNodes = 8;
		eltNodes = Memory_Alloc_Array( Node_DomainIndex, nEltNodes, "SnacRemesher" );
		gEltInd = Mesh_ElementMapDomainToGlobal( mesh, dEltInd );
		nLayout->buildElementNodes( nLayout, gEltInd, eltNodes );
	}

	/* Convert global node indices to domain. */
	{
		unsigned	eltNode_i;

		for( eltNode_i = 0; eltNode_i < nEltNodes; eltNode_i++ ) {
			eltNodes[eltNode_i] = Mesh_NodeMapGlobalToDomain( mesh, eltNodes[eltNode_i] );
		}
	}

	/* Extract the new node's coordinate. */
	Vector_Set( newNodeCoord, meshExt->newNodeCoords[newNodeInd] );

	/* Copy coordinates. */
	for( eltNode_i = 0; eltNode_i < nEltNodes; eltNode_i++ )
		memcpy( crds[eltNode_i], mesh->nodeCoord[eltNodes[eltNode_i]], sizeof(Coord) );

	if( !_HexaEL_FindTetBarycenter( crds, newNodeCoord, weights, tetNodeInds, INCLUSIVE_UPPER_BOUNDARY, NULL, 0 ) )
		abort();

	SnacRemesher_InterpolateNode( context, contextExt,
				      newNodeInd, dEltInd, 0,
				      tetNodeInds, weights,
				      meshExt->newNodes );

	/* Free the element node array. */
	FreeArray( eltNodes );
}


/*
** Interpolate an individual node using a cartesian method or a spherical method.
*/

void _SnacRemesher_InterpolateNode( void* _context,
				    unsigned nodeInd, unsigned elementInd, unsigned tetInd,
				    unsigned* tetNodeInds, double* weights,
				    Snac_Node* dstNodes )
{
	Snac_Context*	context = (Snac_Context*)_context;
	Mesh*		mesh = context->mesh;
	NodeLayout*			nLayout = mesh->layout->nodeLayout;
	Snac_Node*	dstNode = (Snac_Node*)ExtensionManager_At( context->mesh->nodeExtensionMgr,
								   dstNodes,
								   nodeInd );
	unsigned	tetNode_i;

	/* Extract the element's node indices.  Note that there should always be eight of these. */
	Node_DomainIndex*	eltNodes;
	unsigned int nEltNodes = 8;
	{
		Element_GlobalIndex	gEltInd;

		eltNodes = Memory_Alloc_Array( Node_DomainIndex, nEltNodes, "SnacRemesher" );
		gEltInd = Mesh_ElementMapDomainToGlobal( mesh, elementInd );
		nLayout->buildElementNodes( nLayout, gEltInd, eltNodes );
	}

	/* Convert global node indices to local. */
	{
		unsigned	eltNode_i;

		for( eltNode_i = 0; eltNode_i < nEltNodes; eltNode_i++ ) {
			eltNodes[eltNode_i] = Mesh_NodeMapGlobalToDomain( mesh, eltNodes[eltNode_i] );
		}
	}

	/* Clear the velocity. */
	dstNode->velocity[0] = 0.0;
	dstNode->velocity[1] = 0.0;
	dstNode->velocity[2] = 0.0;


	/* Loop over each contributing node. */
	for( tetNode_i = 0; tetNode_i < 4; tetNode_i++ ) {
		Snac_Node*	srcNode;

		/* Where is this contibution coming from? */

		srcNode = Snac_Node_At( context, eltNodes[tetNodeInds[tetNode_i]] );

		/* Add the contribution. */ // TODO: Do spherical part.
		dstNode->velocity[0] += srcNode->velocity[0] * weights[tetNode_i];
		dstNode->velocity[1] += srcNode->velocity[1] * weights[tetNode_i];
		dstNode->velocity[2] += srcNode->velocity[2] * weights[tetNode_i];

		/* fprintf(stderr,"\t src vel[%d]=%e %e %e\n",tetNode_i,srcNode->velocity[0],srcNode->velocity[1],srcNode->velocity[2]); */
		/* if(sqrt(dstNode->velocity[0]*dstNode->velocity[0]+dstNode->velocity[1]*dstNode->velocity[1]+dstNode->velocity[2]*dstNode->velocity[2])>=5e-10) { */
		/* 	fprintf(stderr,"node=%d in element %d tet nodes: %d %d %d %d weights: %e %e %e %e\n",nodeInd,elementInd, */
		/* 			tetNodeInds[0],tetNodeInds[1],tetNodeInds[2],tetNodeInds[3], */
		/* 			weights[0],weights[1],weights[2],weights[3]); */
		/* 	fprintf(stderr,"\t after vel=%e %e %e\n",dstNode->velocity[0],dstNode->velocity[1],dstNode->velocity[2]); */
		/* } */

	}
}

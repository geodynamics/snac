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
** $Id: Remesh.c 3260 2006-11-13 06:36:32Z EunseoChoi $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "Node.h"
#include "Remesh.h"
#include "Register.h"


void _SnacTemperature_InterpolateNode( void* _context, 
				       unsigned nodeInd, unsigned elementInd, unsigned tetInd, 
				       unsigned* tetNodeInds, double* weights, 
				       Snac_Node* newNodes )
{
	Snac_Context*                   context = (Snac_Context*)_context;
	Mesh*				mesh = context->mesh;
	NodeLayout*			nLayout = mesh->layout->nodeLayout;
	Snac_Node*			dstNode = (Snac_Node*)ExtensionManager_At( mesh->nodeExtensionMgr, 
										newNodes, 
										nodeInd );
	SnacTemperature_Node*		dstNodeExt = ExtensionManager_Get( mesh->nodeExtensionMgr, 
									dstNode, 
									SnacTemperature_NodeHandle );
	Snac_Node*			srcNode;
	SnacTemperature_Node*		srcNodeExt;
	unsigned			tetNode_i;
	
#ifdef DEBUG
	printf( "node_lI: %u\n", nodeInd );
#endif

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

	/* Zero out on first addition */
/* 	fprintf(stderr,"before T=%e \t",dstNodeExt->temperature); */
	dstNodeExt->temperature = 0;

	for( tetNode_i = 0; tetNode_i < 4; tetNode_i++ ) {
		srcNode = Snac_Node_At( context, eltNodes[tetNodeInds[tetNode_i]] );
		srcNodeExt = ExtensionManager_Get( mesh->nodeExtensionMgr, 
										   srcNode, 
										   SnacTemperature_NodeHandle );

		dstNodeExt->temperature += srcNodeExt->temperature * weights[tetNode_i];
	}
/* 	fprintf(stderr,"%e (%e %e %e %e)\t",srcNodeExt->temperature, */
/* 		weights[0],weights[1],weights[2],weights[3]); */
/* 	fprintf(stderr,"after T=%e \n",dstNodeExt->temperature); */
}

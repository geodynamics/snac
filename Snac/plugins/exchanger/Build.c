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
** $Id: Build.c 1176 2004-04-05 17:09:16Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <string.h>

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "ExchangerVC.h"
#include "Context.h"
#include "Register.h"
#include "Build.h"

void _SnacExchanger_Build( void* _context, void* data ) {
	Snac_Context*				context = (Snac_Context*)_context;
	SnacExchanger_Context*			contextExt = ExtensionManager_Get(
							context->extensionMgr,
							context,
							SnacExchanger_ContextHandle );
	Node_LocalIndex			node_lI;
	double*				tmp;
	int				s;
	IndexSet_Index			index;
	IndexSet*			walls[5] = { RegularMeshUtils_CreateLocalInGlobalLeftSet( context->mesh ), 
						     RegularMeshUtils_CreateLocalInGlobalRightSet( context->mesh ), 
						     RegularMeshUtils_CreateLocalInGlobalFrontSet( context->mesh ), 
						     RegularMeshUtils_CreateLocalInGlobalBackSet( context->mesh ), 
						     RegularMeshUtils_CreateLocalInGlobalBottomSet( context->mesh ) };
	Index				wall_I;

	#ifdef DEBUG
		printf( "In: %s\n", __func__ );
	#endif

	/* Because this VC relies on the mesh to be built (for context->mesh->nodeLocalCount, etc), we must do this in the build
	    phase, i.e. after _MeshContext_Build. However, the VC itself, context->velocityBC, will be built by Snac_Build... so we
	    must ensure this is called imbetween them. Work out which nodes are on the  */
	contextExt->bcs->valuesSet = IndexSet_New( context->mesh->nodeLocalCount );
	for( node_lI = 0; node_lI < context->mesh->nodeLocalCount; node_lI++ ) {
		/* If the node is on any but the top wall, add to the set. The top wall is ( sizeIJK[1] - 1 ). */
		for( wall_I = 0; wall_I < 5; wall_I++ ) {
			if( IndexSet_IsMember( walls[wall_I], node_lI ) ) {
				IndexSet_Add( contextExt->bcs->valuesSet, node_lI );
				break;
			}
		}
	}
	for( wall_I = 0; wall_I < 5; wall_I++ ) {
		Stg_Class_Delete( walls[wall_I] );
	}
	IndexSet_GetMembers( contextExt->bcs->valuesSet, &contextExt->bcs->valuesSize, &contextExt->bcs->valuesIndices );
	#ifdef DEBUG
		printf( "Number of nodes on coupled interface: %u\n", contextExt->bcs->valuesSize );
		for( index = 0; index < contextExt->bcs->valuesSize; index++ ) {
			printf( "\tcoupled node %u is node: %u\n", index, contextExt->bcs->valuesIndices[index] );
		}
	#endif

	contextExt->bcs->values = (VariableCondition_Value*)malloc( sizeof(VariableCondition_Value) * contextExt->bcs->valuesSize );
	memset( contextExt->bcs->values, 0, sizeof(VariableCondition_Value) * contextExt->bcs->valuesSize );

	s = 3;
	/* TODO:   HACK... 3...should be gotten from something outside this module! trying to be generic */
	tmp = (double*)malloc( sizeof(double) * contextExt->bcs->valuesSize * s );
	memset( tmp, 0, sizeof(double) * contextExt->bcs->valuesSize * s );
	#ifdef DEBUG2
		printf( "Chunk memory for BC's asDoubleArrays: %p to %p\n", tmp, (char*)tmp + (sizeof(double) * contextExt->bcs->valuesSize * s) );
	#endif

	for( index = 0; index < contextExt->bcs->valuesSize; index++ ) {
		contextExt->bcs->values[index].type = VC_ValueType_DoubleArray;
		contextExt->bcs->values[index].as.typeArray.size = s;
		contextExt->bcs->values[index].as.typeArray.array = (double*)((char*)tmp + (sizeof(double) * 3 * index));
	}
	for( index = 0; index < contextExt->bcs->valuesSize; index++ ) {
		#ifdef DEBUG2
			printf( "contextExt->bcs->values[%u] { \n\ttype: %u, \n\tarraySize: %u, \n\tasDoubleArray: %p {%g %g %g}\n}\n",
				index,
				contextExt->bcs->values[index].type,
				contextExt->bcs->values[index].as.typeArray.size,
				contextExt->bcs->values[index].as.typeArray.array,
				contextExt->bcs->values[index].as.typeArray.array[0],
				contextExt->bcs->values[index].as.typeArray.array[1],
				contextExt->bcs->values[index].as.typeArray.array[2] );
			fflush( stdout );
		#endif
	}
}

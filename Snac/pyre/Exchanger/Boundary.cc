// /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// **
// ** Copyright (C), 2003,
// **	Steve Quenette, 110 Victoria Street, Melbourne, Victoria, 3053, Australia.
// **	Californian Institute of Technology, 1200 East California Boulevard, Pasadena, California, 91125, USA.
// **	University of Texas, 1 University Station, Austin, Texas, 78712, USA.
// **
// ** Authors:
// **	Eh Tan, ?, Caltech (tan2@gps.caltech.edu)
// **	Pururav Thoutireddy, ?, Caltech (puru@cacr.caltech.edu)
// **	Eun-seo Choi, ?, Caltech (ces74@gps.caltech.edu)
// **	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
// **	Stevan M. Quenette, Visitor in Geophysics, Caltech.
// **	Luc Lavier, Research Scientist, The University of Texas. (luc@utig.ug.utexas.edu)
// **	Luc Lavier, Research Scientist, Caltech.
// **
// ** This program is free software; you can redistribute it and/or modify it
// ** under the terms of the GNU General Public License as published by the
// ** Free Software Foundation; either version 2, or (at your option) any
// ** later version.
// **
// ** This program is distributed in the hope that it will be useful,
// ** but WITHOUT ANY WARRANTY; without even the implied warranty of
// ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// ** GNU General Public License for more details.
// **
// ** You should have received a copy of the GNU General Public License
// ** along with this program; if not, write to the Free Software
// ** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
// **
// ** $Id: Boundary.cc 1328 2004-05-02 16:42:38Z EunseoChoi $
// **
// **~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <memory>
#include <algorithm>
#include <limits>
#include <vector>
extern "C" {
	#include <mpi.h>
	#include <StGermain/StGermain.h>
	#include <StGermain/FD/FD.h>
	#include <Snac/Snac.h>

	/* Required module */
	#include "Snac/Exchanger/Exchanger.h"
}
#include "Boundary.h"
#include "journal/firewall.h"
#include "journal/debug.h"
#include "journal/warning.h"
#include "journal/error.h"


Boundary::Boundary() :
	Exchanger::Boundary()
{}


Boundary::Boundary( const void* _context ) :
	Exchanger::Boundary(), SnacExchanger_ContextHandle( (unsigned)-1 )
{
	journal::debug_t	debug( "Boundary" );
	debug << journal::at(__HERE__) << journal::endl;

	Snac_Context*		context = (Snac_Context*)_context;
	MeshLayout*             meshLayout = (MeshLayout*)context->meshLayout;
	HexaMD*			decomp = (HexaMD*)meshLayout->decomp;

	/* Obtain the handle to the Snac exchanger plugin. If it were linked in, the following variable will be an available global
	   variable. Done this way so that the exchangerBC plugin doesn't need to be linked in. */
	SnacExchanger_ContextHandle = ExtensionManager_GetHandle( context->extensionMgr, "SnacExchanger" );
	debug << journal::at(__HERE__) << "SnacExchanger_ContextHandle: " << SnacExchanger_ContextHandle << journal::endl;

	/* Ensure the mesh is built... counts are not availble until this is availble. If context is a valid pointer, this
	   mesh will be too. */
	if( !context->mesh->isBuilt ) {
		journal::firewall_t firewall( "Boundary" );
		firewall << journal::at(__HERE__) << "Snac mesh has NOT been built yet!" << journal::endl;
	}

	/* The exchanger assumes Snac will have no shadowing... this is ok... It'll never need to. This should never come up */
	/*if( context->mesh->nodeDomainCount != context->mesh->nodeLocalCount ) {
		journal::firewall_t firewall( "Boundary" );
		firewall << journal::at(__HERE__) << "context->mesh->nodeDomainCount != context->mesh->nodeDomainCount ("
			<< context->mesh->nodeDomainCount << "!=" << context->mesh->nodeLocalCount << ")... this module "
			<< "assumes no shadowing" << journal::endl;\
	}*/

	int maxNodes = context->mesh->nodeLocalCount - ( (decomp->nodeLocal3DCounts[decomp->rank][0] - 2)
		* (decomp->nodeLocal3DCounts[decomp->rank][1] - 2)
		* (decomp->nodeLocal3DCounts[decomp->rank][2] - 2) );
	X_.reserve( maxNodes );
	nodeID_.reserve( maxNodes );
	normal_.reserve( maxNodes );

	initX( _context );
	initBBox( _context );
	bbox_.print( "Boundary-BBox" );

	X_.shrink();
	X_.print( "Snac_Boundary-X" );
	nodeID_.shrink();
	nodeID_.print( "Boundary-nodeID" );
	normal_.shrink();
	normal_.print("Boundary-normal");
}


Boundary::~Boundary()
{}


// private functions

void Boundary::initBBox(const void* _context) {
	journal::debug_t	debug( "Boundary" );
	Snac_Context*		context = (Snac_Context*)_context;

	double xmax, xmin,ymax, ymin,zmax, zmin;

	xmax = ymax = zmax = std::numeric_limits<double>::min();
	xmin = ymin = zmin = std::numeric_limits<double>::max();

	debug << journal::at(__HERE__) << journal::endl
	      << "\tcontext: " << context << journal::endl
	      << "\tcontext->mesh->nodeLocalCount: " << context->mesh->nodeLocalCount << journal::endl;

	/* Obtain the bounding box for Snac's mesh by querying each node of the mesh. This is only valid after it has been built...
	   two checks 1) Mesh has been built, 2) Mesh has been initialised. */
	if( context->mesh->isBuilt ) { /* 1 */
		if( context->mesh->isInitialised ) { /* 2 */
			for( Node_LocalIndex node_lI = 0; node_lI < context->mesh->nodeLocalCount; node_lI++ ) {
#if 0
				if(Mesh_CoordAt( context->mesh, node_lI )[2] < 0.0 ) {
					debug<<"Z coords: "<<node_lI<<"/"<<context->mesh->nodeLocalCount<<" "<<Mesh_CoordAt( context->mesh, node_lI )[2]<<journal::endl;
				}
#endif
				xmax = std::max( xmax,  Mesh_CoordAt( context->mesh, node_lI )[0] );
				xmin = std::min( xmin,  Mesh_CoordAt( context->mesh, node_lI )[0] );
				ymax = std::max( ymax,  Mesh_CoordAt( context->mesh, node_lI )[1] );
				ymin = std::min( ymin,  Mesh_CoordAt( context->mesh, node_lI )[1] );
				zmax = std::max( zmax,  Mesh_CoordAt( context->mesh, node_lI )[2] );
				zmin = std::min( zmin,  Mesh_CoordAt( context->mesh, node_lI )[2] );
			}

			bbox_[0][0] = xmin;
			bbox_[1][0] = xmax;
			bbox_[0][1] = ymin;
			bbox_[1][1] = ymax;
			bbox_[0][2] = zmin;
			bbox_[1][2] = zmax;

			debug << "Snac's initial BBox\n"
			      << bbox_[0][0] << " " << bbox_[1][0] << "\n"
			      << bbox_[0][1] << " " << bbox_[1][1] << "\n"
			      << bbox_[0][2] << " " << bbox_[1][2] << journal::endl;
		}
		else {
			journal::firewall_t firewall( "Boundary" );
			firewall << journal::at(__HERE__) << "Snac's mesh has NOT been initialised yet!" << journal::endl;
		}
	}
	else {
		journal::firewall_t firewall( "Boundary" );
		firewall << journal::at(__HERE__) << "Snac's mesh has NOT been built yet!" << journal::endl;
	}
}


void Boundary::initX( const void* _context ) {
	journal::debug_t debug("Boundary");

	Snac_Context*		context = (Snac_Context*)_context;
	SnacExchanger_Context*		contextExt = (SnacExchanger_Context*)ExtensionManager_Get( context->extensionMgr, context, SnacExchanger_ContextHandle );
	ExchangerVC*			eVC = contextExt->bcs;

	for( unsigned int index = 0; index < eVC->valuesSize; index++ ) {
		Node_LocalIndex		node_lI = eVC->valuesIndices[index];
		std::vector<double>	x( Exchanger::DIM );

// 		debug << "in Snac's InitX: " << index<< " node_lI = " <<node_lI
// 		      << journal::endl;

		for( int d = 0; d < Exchanger::DIM; d++ ) {
			x[d] = Mesh_CoordAt( context->mesh, node_lI )[d];
		}
		X_.push_back( x );
		nodeID_.push_back( node_lI );
	}
}

bool Boundary::isOnBoundary( const void* _context , unsigned int i, unsigned int j, unsigned int k ) const {

	Snac_Context*		context = (Snac_Context*)_context;
	MeshLayout*             meshLayout = (MeshLayout*)context->meshLayout;
	HexaMD*			decomp = (HexaMD*)meshLayout->decomp;

	/* Map i,j,k  to 1D index... call the other isOnBoundary */
	return isOnBoundary( context, RegularMeshUtils_Node_Local3DTo1D( decomp, i, j, k ) );
}

bool Boundary::isOnBoundary( const void* _context , unsigned int node_dI ) const {
	journal::debug_t debug("Boundary");
	Snac_Context*		context = (Snac_Context*)_context;
	SnacExchanger_Context*	contextExt = (SnacExchanger_Context*)ExtensionManager_Get(
					context->extensionMgr,
					context,
					SnacExchanger_ContextHandle );
	Index			index;

	/* To know whether this is a couplin boundary node, refer to the Snac exchanger plugin... its stored on the velocity BC.
	   But, this is only valid after it has been built... two checks 1) Mesh has been built, 2) bcs nodes have been set. */
	if( context->mesh->isBuilt ) { /* 1 */
		if( contextExt->bcs->valuesSize ) { /* 2 */
			for( index = 0; index < contextExt->bcs->valuesSize; index++ ) {
				if( node_dI == contextExt->bcs->valuesIndices[index] ) {
					debug << "Deciding onBoundary: node_dI = "<<node_dI<<journal::endl;
					return true;
				}
			}
		}
		else {
			journal::firewall_t firewall( "Boundary" );
			firewall << journal::at(__HERE__)
			         << "The Snac exchanger plugin has not yet specified/built-the coupling boundary!" << journal::endl;
		}
	}
	else {
		journal::firewall_t firewall( "Boundary" );
		firewall << journal::at(__HERE__) << "Snac's Mesh has NOT been built yet!" << journal::endl;
	}

	return false;
}

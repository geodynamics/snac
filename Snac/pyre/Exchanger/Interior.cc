/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003,
**	Steve Quenette, 110 Victoria Street, Melbourne, Victoria, 3053, Australia.
**	Californian Institute of Technology, 1200 East California Boulevard, Pasadena, California, 91125, USA.
**	University of Texas, 1 University Station, Austin, Texas, 78712, USA.
**
** Authors:
**	Eh Tan, ?, Caltech (tan2@gps.caltech.edu)
**	Pururav Thoutireddy, ?, Caltech (puru@cacr.caltech.edu)
**	Eun-seo Choi, ?, Caltech (ces74@gps.caltech.edu)
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
** $Id: Interior.cc 1279 2004-04-26 21:59:27Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <memory>
#include <algorithm>
#include <limits>
#include <vector>
#include "mpi.h"
extern "C" {
	#include <mpi.h>
	#include <StGermain/StGermain.h>
	#include <StGermain/FD/FD.h>
	#include <Snac/Snac.h>
}
#include "Interior.h"
#include "journal/debug.h"

Interior::Interior() :
	Exchanger::BoundedMesh()
{}



Interior::Interior( const Exchanger::BoundedBox& remoteBBox,
		    const void* _context ) :
	Exchanger::BoundedMesh()
{
	journal::debug_t debug("Snac-Exchanger");
	debug << journal::at(__HERE__) << journal::endl;
	Snac_Context* context = (Snac_Context*)_context;

	bbox_ = remoteBBox;
	bbox_.print(" Snac-Interior-BBox" );

	X_.reserve( context->mesh->nodeLocalCount );
	nodeID_.reserve( context->mesh->nodeLocalCount );

	initX( context );

	X_.shrink();
	X_.print( "Interior-X" );

	nodeID_.shrink();
	nodeID_.print( "Interior-nodeID" );
}


Interior::~Interior()
{}


void Interior::initX( const void* _context ) {
	Snac_Context*		context = (Snac_Context*)_context;
	MeshLayout*             meshLayout = (MeshLayout*)context->meshLayout;
	HexaMD*			decomp = (HexaMD*)meshLayout->decomp;

	/* Build the array of indices and coordinates for the coupled nodes. Note: CitComS' and hence Exchanger's node orderings
	   are different by axis labelling only. Snac is generic Y-up (x to right, z at you), with ascending IJK for 3D to 1D
	   mappings, Exchanger's is X-down (y to right, z-down) ascending JIK (i think, SQ). */
	for( Index j = 0; j < decomp->nodeLocal3DCounts[decomp->rank][1]; j++ ) {
		for( Index i = 0; i < decomp->nodeLocal3DCounts[decomp->rank][0]; i++ ) {
			for( Index k = 0; k < decomp->nodeLocal3DCounts[decomp->rank][2]; k++ ) {
				journal::debug_t	debug( "Interior" );
				Node_LocalIndex		node_lI = RegularMeshUtils_Node_Local3DTo1D( decomp, i, j, k );
				std::vector<double>	x( Exchanger::DIM );

				for( int d = 0; d < Exchanger::DIM; d++ ) {
					x[d] = Mesh_CoordAt( context->mesh, node_lI )[d];
				}
				if( isInside( x, bbox_ ) ) {
					X_.push_back( x );
					nodeID_.push_back( node_lI );
				}
			}
		}
	}
}

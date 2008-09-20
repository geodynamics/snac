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
// ** $Id: Source.cc 1340 2004-05-03 12:50:49Z EunseoChoi $
// **
// **~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <memory>
#include <vector>
#include "mpi.h"
extern "C" {
	#include <mpi.h>
	#include <StGermain/StGermain.h>
	#include <StGermain/FD/FD.h>
	#include <Snac/Snac.h>
}
#include "journal/debug.h"
#include "Exchanger/BoundedMesh.h"
#include "SnacInterpolator.h"

using Exchanger::Array2D;
using Exchanger::BoundedBox;
using Exchanger::BoundedMesh;
using Exchanger::DIM;
using Exchanger::STRESS_DIM;

#include "SnacSource.h"

SnacSource::SnacSource( MPI_Comm comm,
			int sinkRank,
			BoundedMesh& mesh,
			const BoundedBox& mybbox,
			const void* __context ) :
	Exchanger::Source(comm, sinkRank, mesh, mybbox ),
	_context( __context )
{
	journal::debug_t debug("Snac_Source");
	debug << journal::at(__HERE__) << journal::endl;

	init( mesh, mybbox);
}


SnacSource::~SnacSource()
{}


void SnacSource::interpolateTemperature( Array2D<double,1>& T ) const
{
	journal::debug_t debug( "Snac_Exchanger" );
	debug << journal::at(__HERE__)	<< journal::endl;

	if( size() ) {
		snac_interp->interpolateT( T, _context );
	}
}


void SnacSource::interpolateStress( Array2D<double,STRESS_DIM>& S ) const
{
	journal::debug_t debug( "Snac_Exchanger" );
	debug << journal::at(__HERE__)	<< journal::endl;

	if( size() ) {
		snac_interp->interpolateS( S, _context );
	}
}


void SnacSource::interpolateVelocity( Array2D<double,DIM>& V ) const
{
	if( size() ) {
		snac_interp->interpolateV( V, _context );
	}
}


void SnacSource::createInterpolator(const BoundedMesh& mesh)
{
	snac_interp = new SnacInterpolator(mesh, _context, meshNode_);
}

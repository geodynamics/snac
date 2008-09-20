// -*- C++ -*-
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//  <LicenseText>
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//

#include "journal/debug.h"
#include "Exchanger/BoundedMesh.h"
#include "Exchanger/Sink.h"
#include "VTInlet.h"
extern "C" {
	#include <mpi.h>
	#include <StGermain/StGermain.h>
	#include <StGermain/FD/FD.h>
	#include <Snac/Snac.h>

	/* Required module */
	#include "Snac/Exchanger/Exchanger.h"
}

using Exchanger::Array2D;
using Exchanger::BoundedMesh;
using Exchanger::DIM;
using Exchanger::Sink;


VTInlet::VTInlet(const BoundedMesh& boundedMesh,
		 const Sink& sink,
		 void* __context) :
    Inlet(boundedMesh, sink),
    _context( __context),
    v(sink.size()),
    v_old(sink.size()),
    t(sink.size()),
    t_old(sink.size())
{
    journal::debug_t debug("Snac-Exchanger");
    debug << journal::at(__HERE__) << journal::endl;

    setVBCFlag();
    setTBCFlag();
}


VTInlet::~VTInlet()
{}


void VTInlet::recv()
{
    journal::debug_t debug("Snac-Exchanger");
    debug << journal::at(__HERE__) << journal::endl;

    // store bc from previous timestep
    t.swap(t_old);
    v.swap(v_old);

    sink.recv(t, v);

    t.print("Snac-VTInlet-T");
    v.print("Snac-VTInlet-V");
}


void VTInlet::impose()
{
    imposeV();
    imposeT();
}


// private functions

void VTInlet::setVBCFlag()
{
    // Because CitcomS' default side BC is reflecting,
    // here we should change to velocity BC.
    for(int i=0; i<mesh.size(); i++) {
    }
}


void VTInlet::setTBCFlag()
{
    for(int i=0; i<mesh.size(); i++) {
    }
}


void VTInlet::imposeV()
{
    journal::debug_t debug("Snac-Exchanger");
    debug << journal::at(__HERE__) << journal::endl;

    journal::debug_t debugBC("Snac-VTInlet-imposeV");
    debugBC << journal::at(__HERE__);

    double N1, N2;
    getTimeFactors(N1, N2);

    // impose velocity interpolated using N1, N2

    debugBC << journal::endl;
}


void VTInlet::imposeT()
{
    journal::debug_t debug("Snac-Exchanger");
    debug << journal::at(__HERE__) << journal::endl;

    journal::debug_t debugBC("Snac-VTInlet-imposeT");
    debugBC << journal::at(__HERE__);

    double N1, N2;
    getTimeFactors(N1, N2);

    // impose velocity interpolated using N1, N2

    debugBC << journal::endl;
}


// version
// $Id: VTInlet.cc,v 1.5 2004/05/11 07:55:30 tan2 Exp $

// End of file

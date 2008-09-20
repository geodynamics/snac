// -*- C++ -*-
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//  <LicenseText>
//
//  Purpose:
//  Replace local temperture field by received values. Note that b.c. is not
//  affected.
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//

#include "journal/debug.h"
#include "Exchanger/BoundedMesh.h"
#include "Exchanger/Sink.h"
#include "TInlet.h"

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
using Exchanger::Sink;


TInlet::TInlet(const BoundedMesh& boundedMesh,
	       const Sink& sink,
	       const void* _context) :
    Inlet(boundedMesh, sink),
    t(sink.size())
{
    journal::debug_t debug("Snac-Exchanger");
    debug << journal::at(__HERE__) << journal::endl;
}


TInlet::~TInlet()
{}


void TInlet::recv()
{
    journal::debug_t debug("Snac-Exchanger");
    debug << journal::at(__HERE__) << journal::endl;

    sink.recv(t);

    t.print("Snac-TInlet-T");
}


void TInlet::impose()
{
    journal::debug_t debug("Snac-Exchanger");
    debug << journal::at(__HERE__) << journal::endl;

    const int m = 1;
    for(int i=0; i<sink.size(); i++) {
	int n = sink.meshNode(i);
	// assign values
    }
}


// private functions


// version
// $Id: TInlet.cc,v 1.1 2004/05/11 07:55:30 tan2 Exp $

// End of file

// -*- C++ -*-
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//  <LicenseText>
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//

#include "journal/debug.h"
#include "SnacSource.h"
#include "TOutlet.h"
extern "C" {
	#include <mpi.h>
	#include <StGermain/StGermain.h>
	#include <StGermain/FD/FD.h>
	#include <Snac/Snac.h>

	/* Required module */
	#include "Snac/Exchanger/Exchanger.h"
}

TOutlet::TOutlet(const SnacSource& source,
		 void* _context) :
    Outlet(source),
    t(source.size())
{
    journal::debug_t debug("Snac-Exchanger");
    debug << journal::at(__HERE__) << journal::endl;
}


TOutlet::~TOutlet()
{}


void TOutlet::send()
{
    journal::debug_t debug("Snac-Exchanger");
    debug << journal::at(__HERE__) << journal::endl;

    source.interpolateTemperature(t);
    t.print("Snac-TOutlet-T");

    source.send(t);
}


// private functions



// version
// $Id: TOutlet.cc,v 1.1 2004/05/11 07:55:30 tan2 Exp $

// End of file

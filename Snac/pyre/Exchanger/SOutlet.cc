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
#include "SOutlet.h"

using Exchanger::Array2D;
using Exchanger::DIM;
using Exchanger::STRESS_DIM;


SOutlet::SOutlet(const SnacSource& source,
				   void* _context) :
    Outlet(source),
    s(source.size())
{
    journal::debug_t debug("Snac-Exchanger");
    debug << journal::at(__HERE__) << journal::endl;
}


SOutlet::~SOutlet()
{}


void SOutlet::send()
{
    journal::debug_t debug("Snac-Exchanger");
    debug << journal::at(__HERE__) << journal::endl;

    source.interpolateStress(s);
    s.print("Snac-SOutlet-S");

    source.send(s);
}


// version
// $Id: SOutlet.cc,v 1.3 2004/05/25 00:29:30 tan2 Exp $

// End of file

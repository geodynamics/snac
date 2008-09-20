// -*- C++ -*-
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//  <LicenseText>
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//

#include "Convertor.h"
#include "journal/debug.h"


void Convertor::init(bool dimensional, bool transformational,void* _context)
{
    journal::debug_t debug("Snac-Exchanger");
    debug << journal::at(__HERE__) << journal::endl;

    si = 0;
    cart = 0;
    inited = true;

}


// version
// $Id: Convertor.cc,v 1.5 2004/05/11 07:55:30 tan2 Exp $

// End of file

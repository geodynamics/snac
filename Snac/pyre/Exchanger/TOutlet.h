// -*- C++ -*-
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//  <LicenseText>
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//

#if !defined(pySnacExchanger_TOutlet_h)
#define pySnacExchanger_TOutlet_h

#include "Exchanger/Outlet.h"

class SnacSource;


class TOutlet : public Exchanger::Outlet {
    void* _context;
    Exchanger::Array2D<double,1> t;

public:
    TOutlet(const SnacSource& source,
	    void* _context);
    virtual ~TOutlet();

    virtual void send();

private:
    // disable copy c'tor and assignment operator
    TOutlet(const TOutlet&);
    TOutlet& operator=(const TOutlet&);

};


#endif

// version
// $Id: TOutlet.h,v 1.1 2004/05/11 07:55:30 tan2 Exp $

// End of file

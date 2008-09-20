// -*- C++ -*-
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//  <LicenseText>
//
// Role:
//     send stress, velocity and temperature to sink
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//

#if !defined(pySnacExchanger_SOutlet_h)
#define pySnacExchanger_SOutlet_h

#include "Exchanger/Outlet.h"

class SnacSource;


class SOutlet : public Exchanger::Outlet {
    void* _context;
    Exchanger::Array2D<double,Exchanger::STRESS_DIM> s;
    Exchanger::Array2D<double,1> t;

public:
    SOutlet(const SnacSource& source,
	    void* _context);
    virtual ~SOutlet();

    virtual void send();

private:
    // disable copy c'tor and assignment operator
    SOutlet(const SOutlet&);
    SOutlet& operator=(const SOutlet&);

};


#endif

// version
// $Id: SOutlet.h,v 1.2 2004/05/11 07:55:30 tan2 Exp $

// End of file

// -*- C++ -*-
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//  <LicenseText>
//
// Role:
//     impose velocity and temperature as b.c.
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//

#if !defined(pySnacExchanger_TInlet_h)
#define pySnacExchanger_TInlet_h

#include "Exchanger/Array2D.h"
#include "Exchanger/DIM.h"
#include "Exchanger/Inlet.h"

class TInlet : public Exchanger::Inlet {
    Exchanger::Array2D<double,1> t;

public:
    TInlet(const Exchanger::BoundedMesh& boundedMesh,
	   const Exchanger::Sink& sink,
	   const void* _context);

    virtual ~TInlet();

    virtual void recv();
    virtual void impose();

private:

};


#endif

// version
// $Id: TInlet.h,v 1.1 2004/05/11 07:55:30 tan2 Exp $

// End of file

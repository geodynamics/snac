// -*- C++ -*-
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//  <LicenseText>
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//

#if !defined(pySnacExchanger_Convertor_h)
#define pySnacExchanger_Convertor_h

#include "Exchanger/Convertor.h"

// singleton class

class Convertor : public Exchanger::Convertor {

public:
    ~Convertor();

    static void init(bool dimensional, bool transformational,void* _context);

private:
    Convertor();

    // disable
    Convertor(const Convertor&);
    Convertor& operator=(const Convertor&);

};


#endif

// version
// $Id: Convertor.h,v 1.3 2004/05/11 07:55:30 tan2 Exp $

// End of file

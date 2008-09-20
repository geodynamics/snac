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

#if !defined(pySnacExchanger_VInlet_h)
#define pySnacExchanger_VInlet_h

#include "Exchanger/Array2D.h"
#include "Exchanger/DIM.h"
#include "Exchanger/Inlet.h"
extern "C" {
	#include <mpi.h>
	#include <StGermain/StGermain.h>
	#include <StGermain/FD/FD.h>
	#include <Snac/Snac.h>
}

class VInlet : public Exchanger::Inlet {
	Exchanger::Array2D<double,Exchanger::DIM> v;
	Exchanger::Array2D<double,Exchanger::DIM> v_old;
	ExtensionManager_Index SnacExchangerForceBC_ContextHandle; // aleviates having to link against the module itself
	ExtensionManager_Index SnacExchanger_ContextHandle; // aleviates having to link against the module itself
	void* _context;

public:
	VInlet(const Exchanger::BoundedMesh& boundedMesh,
	       const Exchanger::Sink& sink,
	       void* _context);

	virtual ~VInlet();

	virtual void recv();
	virtual void impose();
	virtual void storeVold();
	virtual void readVold();

private:
	void imposeV();
	void storeVold_();
	void readVold_();
};


#endif

// version
// $Id: VTInlet.h,v 1.3 2004/05/11 07:55:30 tan2 Exp $

// End of file

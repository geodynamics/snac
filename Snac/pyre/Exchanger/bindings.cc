// -*- C++ -*-
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//  <LicenseText>
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//

#include <Python.h>

#include "Exchanger/exchangers.h"
#include "exchangers.h"
#include "inlets_outlets.h"
#include "misc.h"          // miscellaneous methods

#include "bindings.h"

// the method table

struct PyMethodDef pyExchanger_methods[] = {

    // from misc.h

    {PySnacExchanger_copyright__name__,
     PySnacExchanger_copyright,
     METH_VARARGS,
     PySnacExchanger_copyright__doc__},

    // from inlets_outlets.h

    {PySnacExchanger_TInlet_create__name__,
     PySnacExchanger_TInlet_create,
     METH_VARARGS,
     PySnacExchanger_TInlet_create__doc__},

    {PySnacExchanger_VInlet_create__name__,
     PySnacExchanger_VInlet_create,
     METH_VARARGS,
     PySnacExchanger_VInlet_create__doc__},

    {PySnacExchanger_VInlet_storeVold__name__,
     PySnacExchanger_VInlet_storeVold,
     METH_VARARGS,
     PySnacExchanger_VInlet_storeVold__doc__},

    {PySnacExchanger_VInlet_readVold__name__,
     PySnacExchanger_VInlet_readVold,
     METH_VARARGS,
     PySnacExchanger_VInlet_readVold__doc__},

    {PySnacExchanger_VTInlet_create__name__,
     PySnacExchanger_VTInlet_create,
     METH_VARARGS,
     PySnacExchanger_VTInlet_create__doc__},

    {PySnacExchanger_TOutlet_create__name__,
     PySnacExchanger_TOutlet_create,
     METH_VARARGS,
     PySnacExchanger_TOutlet_create__doc__},

    {PySnacExchanger_SOutlet_create__name__,
     PySnacExchanger_SOutlet_create,
     METH_VARARGS,
     PySnacExchanger_SOutlet_create__doc__},

    // from exchangers.h

    {PySnacExchanger_createBoundary__name__,
     PySnacExchanger_createBoundary,
     METH_VARARGS,
     PySnacExchanger_createBoundary__doc__},

    {PySnacExchanger_createEmptyBoundary__name__,
     PySnacExchanger_createEmptyBoundary,
     METH_VARARGS,
     PySnacExchanger_createEmptyBoundary__doc__},

    {PySnacExchanger_createEmptyInterior__name__,
     PySnacExchanger_createEmptyInterior,
     METH_VARARGS,
     PySnacExchanger_createEmptyInterior__doc__},

    {PySnacExchanger_createGlobalBoundedBox__name__,
     PySnacExchanger_createGlobalBoundedBox,
     METH_VARARGS,
     PySnacExchanger_createGlobalBoundedBox__doc__},

    {PySnacExchanger_createInterior__name__,
     PySnacExchanger_createInterior,
     METH_VARARGS,
     PySnacExchanger_createInterior__doc__},

    {PySnacExchanger_initConvertor__name__,
     PySnacExchanger_initConvertor,
     METH_VARARGS,
     PySnacExchanger_initConvertor__doc__},

    {PySnacExchanger_SnacSource_create__name__,
     PySnacExchanger_SnacSource_create,
     METH_VARARGS,
     PySnacExchanger_SnacSource_create__doc__},

    // from Exchanger/exchangers.h

    {PyExchanger_exchangeBoundedBox__name__,
     PyExchanger_exchangeBoundedBox,
     METH_VARARGS,
     PyExchanger_exchangeBoundedBox__doc__},

    {PyExchanger_exchangeSignal__name__,
     PyExchanger_exchangeSignal,
     METH_VARARGS,
     PyExchanger_exchangeSignal__doc__},

    {PyExchanger_exchangeTimestep__name__,
     PyExchanger_exchangeTimestep,
     METH_VARARGS,
     PyExchanger_exchangeTimestep__doc__},

    {PyExchanger_Inlet_impose__name__,
     PyExchanger_Inlet_impose,
     METH_VARARGS,
     PyExchanger_Inlet_impose__doc__},

    {PyExchanger_Inlet_recv__name__,
     PyExchanger_Inlet_recv,
     METH_VARARGS,
     PyExchanger_Inlet_recv__doc__},

    {PyExchanger_Inlet_storeTimestep__name__,
     PyExchanger_Inlet_storeTimestep,
     METH_VARARGS,
     PyExchanger_Inlet_storeTimestep__doc__},

    {PyExchanger_Outlet_send__name__,
     PyExchanger_Outlet_send,
     METH_VARARGS,
     PyExchanger_Outlet_send__doc__},

    {PyExchanger_Sink_create__name__,
     PyExchanger_Sink_create,
     METH_VARARGS,
     PyExchanger_Sink_create__doc__},

    // Sentinel
    {0, 0}
};

// version
// $Id: bindings.cc,v 1.41 2004/05/11 07:55:30 tan2 Exp $

// End of file

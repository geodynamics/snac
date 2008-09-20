// -*- C++ -*-
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//  <LicenseText>
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//

#if !defined(pySnacExchanger_inlets_outlets_h)
#define pySnacExchanger_inlets_outlets_h


///////////////////////////////////////////////////////////////////////////////


extern char PySnacExchanger_TInlet_create__name__[];
extern char PySnacExchanger_TInlet_create__doc__[];
extern "C"
PyObject * PySnacExchanger_TInlet_create(PyObject *, PyObject *);


extern char PySnacExchanger_VInlet_create__name__[];
extern char PySnacExchanger_VInlet_create__doc__[];
extern "C"
PyObject * PySnacExchanger_VInlet_create(PyObject *, PyObject *);


extern char PySnacExchanger_VInlet_storeVold__name__[];
extern char PySnacExchanger_VInlet_storeVold__doc__[];
extern "C"
PyObject * PySnacExchanger_VInlet_storeVold(PyObject *, PyObject *);


extern char PySnacExchanger_VInlet_readVold__name__[];
extern char PySnacExchanger_VInlet_readVold__doc__[];
extern "C"
PyObject * PySnacExchanger_VInlet_readVold(PyObject *, PyObject *);


extern char PySnacExchanger_VTInlet_create__name__[];
extern char PySnacExchanger_VTInlet_create__doc__[];
extern "C"
PyObject * PySnacExchanger_VTInlet_create(PyObject *, PyObject *);


///////////////////////////////////////////////////////////////////////////////


extern char PySnacExchanger_TOutlet_create__name__[];
extern char PySnacExchanger_TOutlet_create__doc__[];
extern "C"
PyObject * PySnacExchanger_TOutlet_create(PyObject *, PyObject *);


extern char PySnacExchanger_SOutlet_create__name__[];
extern char PySnacExchanger_SOutlet_create__doc__[];
extern "C"
PyObject * PySnacExchanger_SOutlet_create(PyObject *, PyObject *);

#endif

// version
// $Id: inlets_outlets.h,v 1.6 2004/05/11 07:55:30 tan2 Exp $

// End of file

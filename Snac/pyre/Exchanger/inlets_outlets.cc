// -*- C++ -*-
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//  <LicenseText>
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//

#include <Python.h>
#include "mpi.h"
#include "mpi/Communicator.h"
#include "inlets_outlets.h"

///////////////////////////////////////////////////////////////////////////////

#include "TInlet.h"

extern "C" void deleteTInlet(void*);


char PySnacExchanger_TInlet_create__doc__[] = "";
char PySnacExchanger_TInlet_create__name__[] = "TInlet_create";

PyObject * PySnacExchanger_TInlet_create(PyObject *self, PyObject *args)
{
    PyObject *obj1, *obj2, *obj3;

    if (!PyArg_ParseTuple(args, "OOO:TInlet_create",
                          &obj1, &obj2, &obj3))
        return NULL;

    Exchanger::BoundedMesh* b = static_cast<Exchanger::BoundedMesh*>(PyCObject_AsVoidPtr(obj1));
    Exchanger::Sink* sink = static_cast<Exchanger::Sink*>(PyCObject_AsVoidPtr(obj2));
    void* _context = static_cast<void*>(PyCObject_AsVoidPtr(obj3));

    TInlet* inlet = new TInlet(*b, *sink, _context);

    PyObject *cobj = PyCObject_FromVoidPtr(inlet, deleteTInlet);
    return Py_BuildValue("O", cobj);
}


void deleteTInlet(void* p)
{
    delete static_cast<TInlet*>(p);
}


///////////////////////////////////////////////////////////////////////////////

#include "VTInlet.h"

extern "C" void deleteVTInlet(void*);


char PySnacExchanger_VTInlet_create__doc__[] = "";
char PySnacExchanger_VTInlet_create__name__[] = "VTInlet_create";

PyObject * PySnacExchanger_VTInlet_create(PyObject *self, PyObject *args)
{
    PyObject *obj1, *obj2, *obj3;

    if (!PyArg_ParseTuple(args, "OOO:VTInlet_create",
                          &obj1, &obj2, &obj3))
        return NULL;

    Exchanger::BoundedMesh* b = static_cast<Exchanger::BoundedMesh*>(PyCObject_AsVoidPtr(obj1));
    Exchanger::Sink* sink = static_cast<Exchanger::Sink*>(PyCObject_AsVoidPtr(obj2));
    void* _context = static_cast<void*>(PyCObject_AsVoidPtr(obj3));

    VTInlet* inlet = new VTInlet(*b, *sink, _context);

    PyObject *cobj = PyCObject_FromVoidPtr(inlet, deleteVTInlet);
    return Py_BuildValue("O", cobj);
}


void deleteVTInlet(void* p)
{
    delete static_cast<VTInlet*>(p);
}


///////////////////////////////////////////////////////////////////////////////

#include "VInlet.h"

extern "C" void deleteVInlet(void*);


char PySnacExchanger_VInlet_create__doc__[] = "";
char PySnacExchanger_VInlet_create__name__[] = "VInlet_create";

PyObject * PySnacExchanger_VInlet_create(PyObject *self, PyObject *args)
{
    PyObject *obj1, *obj2, *obj3;

    if (!PyArg_ParseTuple(args, "OOO:VInlet_create",
                          &obj1, &obj2, &obj3))
        return NULL;

    Exchanger::BoundedMesh* b = static_cast<Exchanger::BoundedMesh*>(PyCObject_AsVoidPtr(obj1));
    Exchanger::Sink* sink = static_cast<Exchanger::Sink*>(PyCObject_AsVoidPtr(obj2));
    void* _context = static_cast<void*>(PyCObject_AsVoidPtr(obj3));

    VInlet* inlet = new VInlet(*b, *sink, _context);

    PyObject *cobj = PyCObject_FromVoidPtr(inlet, deleteVInlet);
    return Py_BuildValue("O", cobj);
}


char PySnacExchanger_VInlet_storeVold__doc__[] = "";
char PySnacExchanger_VInlet_storeVold__name__[] = "VInlet_storeVold";

PyObject * PySnacExchanger_VInlet_storeVold(PyObject *self, PyObject *args)
{
    PyObject *obj1;

    if (!PyArg_ParseTuple(args, "O:VInlet_storeVold",&obj1))
        return NULL;

    VInlet* inlet = static_cast<VInlet*>(PyCObject_AsVoidPtr(obj1));

    inlet->storeVold();

    Py_INCREF(Py_None);
    return Py_None;
}


char PySnacExchanger_VInlet_readVold__doc__[] = "";
char PySnacExchanger_VInlet_readVold__name__[] = "VInlet_readVold";

PyObject * PySnacExchanger_VInlet_readVold(PyObject *self, PyObject *args)
{
    PyObject *obj1;

    if (!PyArg_ParseTuple(args, "O:VInlet_readVold",&obj1))
        return NULL;

    VInlet* inlet = static_cast<VInlet*>(PyCObject_AsVoidPtr(obj1));

    inlet->readVold();

    Py_INCREF(Py_None);
    return Py_None;
}


void deleteVInlet(void* p)
{
    delete static_cast<VInlet*>(p);
}


///////////////////////////////////////////////////////////////////////////////

#include "TOutlet.h"

extern "C" void deleteTOutlet(void*);


char PySnacExchanger_TOutlet_create__doc__[] = "";
char PySnacExchanger_TOutlet_create__name__[] = "TOutlet_create";

PyObject * PySnacExchanger_TOutlet_create(PyObject *self, PyObject *args)
{
    PyObject *obj0, *obj1;

    if (!PyArg_ParseTuple(args, "OO:TOutlet_create",
                          &obj0, &obj1))
        return NULL;

    SnacSource* source = static_cast<SnacSource*>(PyCObject_AsVoidPtr(obj0));
    void* _context = static_cast<void*>(PyCObject_AsVoidPtr(obj1));

    TOutlet* outlet = new TOutlet(*source, _context);

    PyObject *cobj = PyCObject_FromVoidPtr(outlet, deleteTOutlet);
    return Py_BuildValue("O", cobj);
}


void deleteTOutlet(void* p)
{
    delete static_cast<TOutlet*>(p);
}


///////////////////////////////////////////////////////////////////////////////

#include "SOutlet.h"

extern "C" void deleteSOutlet(void*);


char PySnacExchanger_SOutlet_create__doc__[] = "";
char PySnacExchanger_SOutlet_create__name__[] = "SOutlet_create";

PyObject * PySnacExchanger_SOutlet_create(PyObject *self, PyObject *args)
{
    PyObject *obj0, *obj1;

    if (!PyArg_ParseTuple(args, "OO:SOutlet_create",
                          &obj0, &obj1))
        return NULL;

    SnacSource* source = static_cast<SnacSource*>(PyCObject_AsVoidPtr(obj0));
    void* _context = static_cast<void*>(PyCObject_AsVoidPtr(obj1));

    SOutlet* outlet = new SOutlet(*source, _context);

    PyObject *cobj = PyCObject_FromVoidPtr(outlet, deleteSOutlet);
    return Py_BuildValue("O", cobj);
}


void deleteSOutlet(void* p)
{
    delete static_cast<SOutlet*>(p);
}

// version
// $Id: inlets_outlets.cc,v 1.6 2004/05/11 07:55:30 tan2 Exp $

// End of file

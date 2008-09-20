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
#include "mpi/Group.h"
extern "C" {
	#include <mpi.h>
	#include <StGermain/StGermain.h>
	#include <StGermain/FD/FD.h>
	#include <Snac/Snac.h>
	/* Required module */
	#include "Snac/Exchanger/Exchanger.h"
}
#include "global_bbox.h"
#include "Boundary.h"
#include "Interior.h"
#include "SnacInterpolator.h"
#include "SnacSource.h"
#include "Convertor.h"
#include "exchangers.h"

void deleteBoundary( void* );
void deleteBoundedBox( void* );
void deleteInterior( void* );
void deleteSnacSource( void* );

using Exchanger::BoundedBox;
using Exchanger::BoundedMesh;


char PySnacExchanger_createBoundary__doc__[] = "";
char PySnacExchanger_createBoundary__name__[] = "createBoundary";

PyObject * PySnacExchanger_createBoundary(PyObject *, PyObject *args)
{
    PyObject *obj1;

    if (!PyArg_ParseTuple(args, "O:createBoundary", &obj1))
	return NULL;

    void* _context = static_cast<void*>
	                          (PyCObject_AsVoidPtr(obj1));

    Boundary* b = new Boundary(_context);
    BoundedBox* bbox = const_cast<BoundedBox*>(&(b->bbox()));

    PyObject *cobj1 = PyCObject_FromVoidPtr(b, deleteBoundary);
    PyObject *cobj2 = PyCObject_FromVoidPtr(bbox, deleteBoundedBox);
    return Py_BuildValue("OO", cobj1, cobj2);
}


char PySnacExchanger_createEmptyBoundary__doc__[] = "";
char PySnacExchanger_createEmptyBoundary__name__[] = "createEmptyBoundary";

PyObject * PySnacExchanger_createEmptyBoundary(PyObject *, PyObject *args)
{
    Boundary* b = new Boundary();

    PyObject *cobj = PyCObject_FromVoidPtr(b, deleteBoundary);
    return Py_BuildValue("O", cobj);
}


char PySnacExchanger_createEmptyInterior__doc__[] = "";
char PySnacExchanger_createEmptyInterior__name__[] = "createEmptyInterior";

PyObject * PySnacExchanger_createEmptyInterior(PyObject *, PyObject *args)
{
    Interior* b = new Interior();

    PyObject *cobj = PyCObject_FromVoidPtr(b, deleteInterior);
    return Py_BuildValue("O", cobj);
}


char PySnacExchanger_createGlobalBoundedBox__doc__[] = "";
char PySnacExchanger_createGlobalBoundedBox__name__[] = "createGlobalBoundedBox";

PyObject * PySnacExchanger_createGlobalBoundedBox(PyObject *, PyObject *args)
{
    PyObject *obj1;

    if (!PyArg_ParseTuple(args, "O:createGlobalBoundedBox", &obj1))
	return NULL;

    void* _context = static_cast<void*>
	                          (PyCObject_AsVoidPtr(obj1));

    BoundedBox* bbox = new BoundedBox(Exchanger::DIM);

    GlobalBoundedBox(*bbox, _context);

    bbox->print("Snac-GlobalBBox");

    PyObject *cobj = PyCObject_FromVoidPtr(bbox, deleteBoundedBox);
    return Py_BuildValue("O", cobj);
}


char PySnacExchanger_createInterior__doc__[] = "";
char PySnacExchanger_createInterior__name__[] = "createInterior";

PyObject * PySnacExchanger_createInterior(PyObject *, PyObject *args)
{
    PyObject *obj1, *obj2;

    if (!PyArg_ParseTuple(args, "OO:createInterior", &obj1, &obj2))
	return NULL;

    BoundedBox* rbbox = static_cast<BoundedBox*>(PyCObject_AsVoidPtr(obj1));
    void* _context = static_cast<void*>
	                          (PyCObject_AsVoidPtr(obj2));

    Interior* i = new Interior(*rbbox, _context);
    BoundedBox* bbox = const_cast<BoundedBox*>(&(i->bbox()));

    PyObject *cobj1 = PyCObject_FromVoidPtr(i, deleteInterior);
    PyObject *cobj2 = PyCObject_FromVoidPtr(bbox, deleteBoundedBox);
    return Py_BuildValue("OO", cobj1, cobj2);
}


char PySnacExchanger_initConvertor__doc__[] = "";
char PySnacExchanger_initConvertor__name__[] = "initConvertor";

PyObject * PySnacExchanger_initConvertor(PyObject *, PyObject *args)
{
   PyObject *obj1;
   int dimensional, transformational;

   if (!PyArg_ParseTuple(args, "iiO:initConvertor",
			 &dimensional, &transformational, &obj1))
        return NULL;

    void* _context = static_cast<void*>(PyCObject_AsVoidPtr(obj1));

    Convertor::init(dimensional, transformational, _context);

    Py_INCREF(Py_None);
    return Py_None;
}


char PySnacExchanger_SnacSource_create__doc__[] = "";
char PySnacExchanger_SnacSource_create__name__[] = "SnacSource_create";

PyObject * PySnacExchanger_SnacSource_create(PyObject *self, PyObject *args)
{
    PyObject *obj1, *obj2, *obj3, *obj4;
    int sinkRank;

    if (!PyArg_ParseTuple(args, "OiOOO:SnacSource_create",
			  &obj1, &sinkRank,
			  &obj2, &obj3, &obj4))
        return NULL;

    mpi::Communicator* temp = static_cast<mpi::Communicator*>
	                      (PyCObject_AsVoidPtr(obj1));
    MPI_Comm comm = temp->handle();

    BoundedMesh* b = static_cast<BoundedMesh*>(PyCObject_AsVoidPtr(obj2));
    BoundedBox* bbox = static_cast<BoundedBox*>(PyCObject_AsVoidPtr(obj3));
    void* _context = static_cast<void*>(PyCObject_AsVoidPtr(obj4));

    SnacSource* source = new SnacSource(comm, sinkRank, *b, *bbox, _context);

    PyObject *cobj = PyCObject_FromVoidPtr(source, deleteSnacSource);
    return Py_BuildValue("O", cobj);
}


// helper functions

void deleteBoundary(void* p)
{
    delete static_cast<Boundary*>(p);
}


void deleteBoundedBox(void* p)
{
    delete static_cast<BoundedBox*>(p);
}


void deleteInterior(void* p)
{
    delete static_cast<Interior*>(p);
}


void deleteSnacSource(void* p)
{
    delete static_cast<SnacSource*>(p);
}


// version
// $Id: exchangers.cc,v 1.46 2004/05/11 07:55:30 tan2 Exp $

// End of file

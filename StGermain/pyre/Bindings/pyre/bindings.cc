/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Victorian Partnership for Advanced Computing (VPAC) Ltd, Australia
** (C) 2003 All Rights Reserved
**
** California Institute of Technology (Caltech), USA
** (C) 2003 All Rights Reserved
**
** Authors:
** 	Stevan M. Quenette, Senior Software Engineer, VPAC.
**	Stevan M. Quenette, Visitor in Geophysics, Caltech.
**
** <copyright-release-tag>
**
** $Id: bindings.cc 3127 2005-07-26 02:35:37Z AlanLo $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "mpi/Communicator.h"
#include <Python.h>
#include "bindings.h"
#include "misc.h"

struct PyMethodDef pyre_Python_Methods[] = {
	{ pyre_Python_copyright__name__,			pyre_Python_copyright,			METH_VARARGS,	pyre_Python_copyright__doc__			},
	{ pyre_Python_ConvertFromPyreComm__name__,		pyre_Python_ConvertFromPyreComm,	METH_VARARGS,	pyre_Python_ConvertFromPyreComm__doc__		},
	{ pyre_Python_ConvertFromPyreCommHandle__name__,	pyre_Python_ConvertFromPyreCommHandle,	METH_VARARGS,	pyre_Python_ConvertFromPyreCommHandle__doc__	},
	{ 0, 0, 0, 0 }
};


/* "ConvertFromPyreComm" member */
char pyre_Python_ConvertFromPyreComm__doc__[] = "Convert the pyre communicator (Python class) to a StGermain (MPI-C/C-integer) communicator.";
char pyre_Python_ConvertFromPyreComm__name__[] = "ConvertFromPyreComm";
PyObject* pyre_Python_ConvertFromPyreComm( PyObject* self, PyObject* args ) {
	PyObject*		pyrePythonComm;
	PyObject*		pyrePythonCommHandle;
	mpi::Communicator*	pyreCppCommunicator;
	MPI_Comm		communicator;
	
	/* Obtain arguements */
	if( !PyArg_ParseTuple( args, "O:", &pyrePythonComm ) ) {
		return NULL;
	}
	pyrePythonCommHandle = PyObject_GetAttrString( pyrePythonComm, "_handle" );
	pyreCppCommunicator = (mpi::Communicator*)PyCObject_AsVoidPtr( pyrePythonCommHandle );
	
	/* Convert */
	communicator = pyreCppCommunicator->handle();
	
	/* Return */
	return Py_BuildValue( "i", communicator );
}

/* "ConvertFromPyreCommHandle" member */
char pyre_Python_ConvertFromPyreCommHandle__doc__[] = "Convert the pyre communicator (C++ binding) to a StGermain (MPI-C/C-integer) communicator.";
char pyre_Python_ConvertFromPyreCommHandle__name__[] = "ConvertFromPyreCommHandle";
PyObject* pyre_Python_ConvertFromPyreCommHandle( PyObject* self, PyObject* args ) {
	PyObject*		pyrePythonCommHandle;
	mpi::Communicator*	pyreCppCommunicator;
	MPI_Comm		communicator;
	
	/* Obtain arguements */
	if( !PyArg_ParseTuple( args, "O:", &pyrePythonCommHandle ) ) {
		return NULL;
	}
	pyreCppCommunicator = (mpi::Communicator*)PyCObject_AsVoidPtr( pyrePythonCommHandle );
	
	/* Convert */
	communicator = pyreCppCommunicator->handle();
	
	/* Return */
	return Py_BuildValue( "i", communicator );
}

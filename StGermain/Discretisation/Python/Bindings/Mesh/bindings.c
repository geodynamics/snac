/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003, Victorian Partnership for Advanced Computing (VPAC) Ltd, 110 Victoria Street, Melbourne, 3053, Australia.
**
** Authors:
**	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
**	Patrick D. Sunter, Software Engineer, VPAC. (pds@vpac.org)
**	Luke J. Hodkinson, Computational Engineer, VPAC. (lhodkins@vpac.org)
**	Siew-Ching Tan, Software Engineer, VPAC. (siew@vpac.org)
**	Alan H. Lo, Computational Engineer, VPAC. (alan@vpac.org)
**	Raquibul Hassan, Computational Engineer, VPAC. (raq@vpac.org)
**
**  This library is free software; you can redistribute it and/or
**  modify it under the terms of the GNU Lesser General Public
**  License as published by the Free Software Foundation; either
**  version 2.1 of the License, or (at your option) any later version.
**
**  This library is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
**  Lesser General Public License for more details.
**
**  You should have received a copy of the GNU Lesser General Public
**  License along with this library; if not, write to the Free Software
**  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
**
** $Id: bindings.c 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <Python.h>

#include <mpi.h>
#include "StGermain/Base/Base.h"
#include "StGermain/Discretisation/Discretisation.h"
#include <limits.h>
#include "bindings.h"
#include "misc.h"

struct PyMethodDef Mesh_Python_Methods[] = {
	{ Mesh_Python_copyright__name__,	Mesh_Python_copyright,		METH_VARARGS, Mesh_Python_copyright__doc__	},
#if 0
	{ Mesh_Python_New__name__,		Mesh_Python_New,		METH_VARARGS, Mesh_Python_New__doc__		},
#endif
	{ Mesh_Python_Print__name__,		Mesh_Python_Print,		METH_VARARGS, Mesh_Python_Print__doc__		},
	{ Mesh_Python_Delete__name__,		Mesh_Python_Delete,		METH_VARARGS, Mesh_Python_Delete__doc__		},
	{ Mesh_Python_GetDictionary__name__,	Mesh_Python_GetDictionary,	METH_VARARGS, Mesh_Python_GetDictionary__doc__	},
	{ 0, 0, 0, 0 }
};


#if 0
/* "New" member */
char Mesh_Python_New__doc__[] = "Create a new Mesh";
char Mesh_Python_New__name__[] = "New";
PyObject* Mesh_Python_New( PyObject* self, PyObject* args ) {
	return PyCObject_FromVoidPtr( Mesh_New(), 0 );
}
#endif

/* "Print" member */
char Mesh_Python_Print__doc__[] = "Print the Mesh";
char Mesh_Python_Print__name__[] = "Print";
PyObject* Mesh_Python_Print( PyObject* self, PyObject* args ) {
	PyObject*	pyMesh;
	Mesh*		mesh;
	Stream*		stream = Journal_Register (Info_Type, "myStream");
	
	/* Obtain arguements */
	if( !PyArg_ParseTuple( args, "O:", &pyMesh ) ) {
		return NULL;
	}
	mesh = (Mesh*)( PyCObject_AsVoidPtr( pyMesh ) );
	
	/* Run function */
	Print( mesh, stream );
	
	/* Return */
	Py_INCREF( Py_None );
	return Py_None;
}

/* "Stg_Class_Delete" member */
char Mesh_Python_Delete__doc__[] = "Stg_Class_Delete/destroy the Mesh";
char Mesh_Python_Delete__name__[] = "Stg_Class_Delete";
PyObject* Mesh_Python_Delete( PyObject* self, PyObject* args ) {
	PyObject*	pyMesh;
	Mesh*		mesh;
	
	/* Obtain arguements */
	if( !PyArg_ParseTuple( args, "O:", &pyMesh ) ) {
		return NULL;
	}
	mesh = (Mesh*)( PyCObject_AsVoidPtr( pyMesh ) );
	
	/* Run function */
	Stg_Class_Delete( mesh );
	
	/* Return */
	Py_INCREF( Py_None );
	return Py_None;
}

/* "GetDictionary" member */
char Mesh_Python_GetDictionary__doc__[] = "Get the Dictionary of the Mesh";
char Mesh_Python_GetDictionary__name__[] = "GetDictionary";
PyObject* Mesh_Python_GetDictionary( PyObject* self, PyObject* args ) {
	PyObject*	pyMesh;
	Mesh*		mesh;
	
	/* Obtain arguements */
	if( !PyArg_ParseTuple( args, "O:", &pyMesh ) ) {
		return NULL;
	}
	mesh = (Mesh*)( PyCObject_AsVoidPtr( pyMesh ) );
	
	return PyCObject_FromVoidPtr( mesh->dictionary, 0 );
}

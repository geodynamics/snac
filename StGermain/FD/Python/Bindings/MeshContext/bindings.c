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
#include "StGermain/StGermain.h"
#include "StGermain/FD/FD.h"
#include "bindings.h"
#include "misc.h"

struct PyMethodDef MeshContext_Python_Methods[] = {
	{ MeshContext_Python_copyright__name__,	MeshContext_Python_copyright,	METH_VARARGS, MeshContext_Python_copyright__doc__	},
	{ MeshContext_Python_Print__name__,	MeshContext_Python_Print,	METH_VARARGS, MeshContext_Python_Print__doc__		},
	{ MeshContext_Python_Delete__name__,	MeshContext_Python_Delete,	METH_VARARGS, MeshContext_Python_Delete__doc__		},
	{ MeshContext_Python_GetMesh__name__,	MeshContext_Python_GetMesh,	METH_VARARGS, MeshContext_Python_GetMesh__doc__		},
	{ 0, 0, 0, 0 }
};


/* "Print" member */
char MeshContext_Python_Print__doc__[] = "Print the MeshContext";
char MeshContext_Python_Print__name__[] = "Print";
PyObject* MeshContext_Python_Print( PyObject* self, PyObject* args ) {
	PyObject*	pyMeshContext;
	MeshContext*	context;
	Stream*		stream = Journal_Register (Info_Type, "myStream");	

	/* Obtain arguements */
	if( !PyArg_ParseTuple( args, "O:", &pyMeshContext ) ) {
		return NULL;
	}
	context = (MeshContext*)( PyCObject_AsVoidPtr( pyMeshContext ) );
	
	/* Run function */
	Print( context, stream );
	
	/* Return */
	Py_INCREF( Py_None );
	return Py_None;
}

/* "Stg_Class_Delete" member */
char MeshContext_Python_Delete__doc__[] = "Stg_Class_Delete/destroy the MeshContext";
char MeshContext_Python_Delete__name__[] = "Stg_Class_Delete";
PyObject* MeshContext_Python_Delete( PyObject* self, PyObject* args ) {
	PyObject*	pyMeshContext;
	MeshContext*	context;
	
	/* Obtain arguements */
	if( !PyArg_ParseTuple( args, "O:", &pyMeshContext ) ) {
		return NULL;
	}
	context = (MeshContext*)( PyCObject_AsVoidPtr( pyMeshContext ) );
	
	/* Run function */
	Stg_Class_Delete( context );
	
	/* Return */
	Py_INCREF( Py_None );
	return Py_None;
}

/* "GetDictionary" member */
char MeshContext_Python_GetDictionary__doc__[] = "Get the Dictionary of the MeshContext";
char MeshContext_Python_GetDictionary__name__[] = "GetDictionary";
PyObject* MeshContext_Python_GetDictionary( PyObject* self, PyObject* args ) {
	PyObject*	pyMeshContext;
	MeshContext*	context;
	
	/* Obtain arguements */
	if( !PyArg_ParseTuple( args, "O:", &pyMeshContext ) ) {
		return NULL;
	}
	context = (MeshContext*)( PyCObject_AsVoidPtr( pyMeshContext ) );
	
	return PyCObject_FromVoidPtr( context->dictionary, 0 );
}

/* "GetMesh" member */
char MeshContext_Python_GetMesh__doc__[] = "Get the Mesh of the MeshContext";
char MeshContext_Python_GetMesh__name__[] = "GetMesh";
PyObject* MeshContext_Python_GetMesh( PyObject* self, PyObject* args ) {
	PyObject*	pyMeshContext;
	MeshContext*	context;
	
	/* Obtain arguements */
	if( !PyArg_ParseTuple( args, "O:", &pyMeshContext ) ) {
		return NULL;
	}
	context = (MeshContext*)( PyCObject_AsVoidPtr( pyMeshContext ) );
	
	return PyCObject_FromVoidPtr( context->mesh, 0 );
}

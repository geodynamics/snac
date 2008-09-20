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
** $Id: bindings.c 3867 2006-10-16 12:38:24Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <Python.h>

#include <mpi.h>
#include "StGermain/Base/Base.h"
#include "bindings.h"
#include "misc.h"

struct PyMethodDef Context_Python_Methods[] = {
	{ Context_Python_copyright__name__,	Context_Python_copyright,	METH_VARARGS, Context_Python_copyright__doc__		},
	{ Context_Python_Print__name__,		Context_Python_Print,		METH_VARARGS, Context_Python_Print__doc__		},
	{ Context_Python_Delete__name__,	Context_Python_Delete,		METH_VARARGS, Context_Python_Delete__doc__		},
	{ Context_Python_Construct__name__,	Context_Python_Construct,	METH_VARARGS, Context_Python_Construct__doc__		},
	{ Context_Python_Build__name__,		Context_Python_Build,		METH_VARARGS, Context_Python_Build__doc__		},
	{ Context_Python_Initialise__name__,	Context_Python_Initialise,	METH_VARARGS, Context_Python_Initialise__doc__		},
	{ Context_Python_Execute__name__,	Context_Python_Execute,		METH_VARARGS, Context_Python_Execute__doc__		},
	{ Context_Python_Destroy__name__,	Context_Python_Destroy,		METH_VARARGS, Context_Python_Destroy__doc__		},
	{ Context_Python_Dt__name__,		Context_Python_Dt,		METH_VARARGS, Context_Python_Dt__doc__			},
	{ Context_Python_Step__name__,		Context_Python_Step,		METH_VARARGS, Context_Python_Step__doc__		},
	{ Context_Python_GetDictionary__name__,	Context_Python_GetDictionary,	METH_VARARGS, Context_Python_GetDictionary__doc__	},
	{ Context_Python_SetTime__name__,	Context_Python_SetTime,		METH_VARARGS, Context_Python_SetTime__doc__		},
	{ Context_Python_SetTimeStep__name__,	Context_Python_SetTimeStep,	METH_VARARGS, Context_Python_SetTimeStep__doc__		},
	{ 0, 0, 0, 0 }
};


/* "Print" member */
char Context_Python_Print__doc__[] = "Print the Context";
char Context_Python_Print__name__[] = "Print";
PyObject* Context_Python_Print( PyObject* self, PyObject* args ) {
	PyObject*	pyContext;
	Context*	context;
	Stream* stream = Journal_Register (Info_Type, "myString");
	
	/* Obtain arguements */
	if( !PyArg_ParseTuple( args, "O:", &pyContext ) ) {
		return NULL;
	}
	context = (Context*)( PyCObject_AsVoidPtr( pyContext ) );
	
	/* Run function */
	Print( context, stream );
	
	/* Return */
	Py_INCREF( Py_None );
	return Py_None;
}

/* "Delete" member */
char Context_Python_Delete__doc__[] = "Delete/destroy the Context";
char Context_Python_Delete__name__[] = "Delete";
PyObject* Context_Python_Delete( PyObject* self, PyObject* args ) {
	PyObject*	pyContext;
	Context*	context;
	
	/* Obtain arguements */
	if( !PyArg_ParseTuple( args, "O:", &pyContext ) ) {
		return NULL;
	}
	context = (Context*)( PyCObject_AsVoidPtr( pyContext ) );
	
	/* Run function */
	Stg_Class_Delete( context );
	
	/* Return */
	Py_INCREF( Py_None );
	return Py_None;
}

/* "Construct" member */
char Context_Python_Construct__doc__[] = "Construct the Context...";
char Context_Python_Construct__name__[] = "Construct";
PyObject* Context_Python_Construct( PyObject* self, PyObject* args ) {
	PyObject*	pyContext;
	Context*	context;

	/* Obtain arguements */
	if( !PyArg_ParseTuple( args, "O:", &pyContext ) ) {
		return NULL;
	}
	context = (Context*)( PyCObject_AsVoidPtr( pyContext ) );

	Stg_Component_Construct( context, 0 /* dummy */, &context, True );

	/* Return */
	Py_INCREF( Py_None );
	return PyCObject_FromVoidPtr( context, 0 );
	}

/* "Build" member */
char Context_Python_Build__doc__[] = "Build the Context... allocates memory (builds arrays)";
char Context_Python_Build__name__[] = "Build";
PyObject* Context_Python_Build( PyObject* self, PyObject* args ) {
	PyObject*	pyContext;
	Context*	context;
	
	/* Obtain arguements */
	if( !PyArg_ParseTuple( args, "O:", &pyContext ) ) {
		return NULL;
	}
	context = (Context*)( PyCObject_AsVoidPtr( pyContext ) );
	
	Stg_Component_Build( context, 0 /* dummy */, False );
	
	/* Return */
	Py_INCREF( Py_None );
	return Py_None;
}

/* "Initialise" member */
char Context_Python_Initialise__doc__[] = "Initialise the Context ... initialises memory (fills arrays with initial values)";
char Context_Python_Initialise__name__[] = "Initialise";
PyObject* Context_Python_Initialise( PyObject* self, PyObject* args ) {
	PyObject*	pyContext;
	Context*	context;
	
	/* Obtain arguements */
	if( !PyArg_ParseTuple( args, "O:", &pyContext ) ) {
		return NULL;
	}
	context = (Context*)( PyCObject_AsVoidPtr( pyContext ) );
	
	/* Run function */
	Stg_Component_Initialise( context, 0 /* dummy */, False );
	
	/* Return */
	Py_INCREF( Py_None );
	return Py_None;
}

/* "Execute" member */
char Context_Python_Execute__doc__[] = "Execute the Context";
char Context_Python_Execute__name__[] = "Execute";
PyObject* Context_Python_Execute( PyObject* self, PyObject* args ) {
	PyObject*	pyContext;
	Context*	context;
	
	/* Obtain arguements */
	if( !PyArg_ParseTuple( args, "O:", &pyContext ) ) {
		return NULL;
	}
	context = (Context*)( PyCObject_AsVoidPtr( pyContext ) );
	
	/* Run function */
	Stg_Component_Execute( context, 0 /* dummy */, False );
	
	/* Return */
	Py_INCREF( Py_None );
	return Py_None;
}

/* "Destroy" member */
char Context_Python_Destroy__doc__[] = "Destroy the Context";
char Context_Python_Destroy__name__[] = "Destroy";
PyObject* Context_Python_Destroy( PyObject* self, PyObject* args ) {
	PyObject*	pyContext;
	Context*	context;
	
	/* Obtain arguements */
	if( !PyArg_ParseTuple( args, "O:", &pyContext ) ) {
		return NULL;
	}
	context = (Context*)( PyCObject_AsVoidPtr( pyContext ) );
	
	/* Run function */
	Stg_Component_Destroy( context, 0 /* dummy */, False );
	
	/* Return */
	Py_INCREF( Py_None );
	return Py_None;
}

/* "Dt" member */
char Context_Python_Dt__doc__[] = "Obtain the desired dt of the Context";
char Context_Python_Dt__name__[] = "Dt";
PyObject* Context_Python_Dt( PyObject* self, PyObject* args ) {
	PyObject*	pyContext;
	Context*	context;
	
	/* Obtain arguements */
	if( !PyArg_ParseTuple( args, "O:", &pyContext ) ) {
		return NULL;
	}
	context = (Context*)( PyCObject_AsVoidPtr( pyContext ) );
	
	/* Run function and return value*/
	return PyFloat_FromDouble( AbstractContext_Dt( context ) );
}

/* "Step" member */
char Context_Python_Step__doc__[] = "Step the Context";
char Context_Python_Step__name__[] = "Step";
PyObject* Context_Python_Step( PyObject* self, PyObject* args ) {
	PyObject*	pyContext;
	Context*	context;
	double		dt;
	
	/* Obtain arguements */
	if( !PyArg_ParseTuple( args, "Od:", &pyContext, &dt ) ) {
		return NULL;
	}
	context = (Context*)( PyCObject_AsVoidPtr( pyContext ) );
	
	/* Run function */
	context->currentTime += dt;
	AbstractContext_Step( context, dt );
	context->timeStep++;	
	
	/* Return */
	Py_INCREF( Py_None );
	return Py_None;
}

/* "GetDictionary" member */
char Context_Python_GetDictionary__doc__[] = "Get Dictionary of the Context";
char Context_Python_GetDictionary__name__[] = "GetDictionary";
PyObject* Context_Python_GetDictionary( PyObject* self, PyObject* args ) {
	PyObject*	pyContext;
	Context*	context;
	
	/* Obtain arguements */
	if( !PyArg_ParseTuple( args, "O:", &pyContext ) ) {
		return NULL;
	}
	context = (Context*)( PyCObject_AsVoidPtr( pyContext ) );
	
	/* Run function and return value*/
	return PyCObject_FromVoidPtr( context->dictionary, 0 );
}


/* "SetTimeStep" member */
char Context_Python_SetTimeStep__doc__[] = "Set the Context timeStep";
char Context_Python_SetTimeStep__name__[] = "SetTimeStep";
PyObject* Context_Python_SetTimeStep( PyObject* self, PyObject* args ) {
	unsigned int	timeStep;
	PyObject*	pyContext;
	Context*	context;
	
	/* Obtain arguements */
	if( !PyArg_ParseTuple( args, "Oi:", &pyContext, &timeStep ) ) {
		return NULL;
	}
	context = (Context*)( PyCObject_AsVoidPtr( pyContext ) );
	
	/* Run function */
	context->timeStep = timeStep;
	
	/* Return */
	Py_INCREF( Py_None );
	return Py_None;
}	

	
/* "SetTime" member */
char Context_Python_SetTime__doc__[] = "Set the Context time";
char Context_Python_SetTime__name__[] = "SetTime";
PyObject* Context_Python_SetTime( PyObject* self, PyObject* args ) {
	PyObject*	pyContext;
	Context*	context;
	double		currentTime;
	
	/* Obtain arguements */
	if( !PyArg_ParseTuple( args, "Od:", &pyContext, &currentTime ) ) {
		return NULL;
	}
	context = (Context*)( PyCObject_AsVoidPtr( pyContext ) );
	
	/* Run function */
	context->currentTime = currentTime;
	
	/* Return */
	Py_INCREF( Py_None );
	return Py_None;
}	

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
#include "bindings.h"
#include "misc.h"

struct PyMethodDef MPI_Python_Methods[] = {
	{ MPI_Python_copyright__name__,		MPI_Python_copyright,	METH_VARARGS, MPI_Python_copyright__doc__	},
	{ MPI_Python_COMM_WORLD__name__,	MPI_Python_COMM_WORLD,	METH_VARARGS, MPI_Python_COMM_WORLD__doc__	},
	{ MPI_Python_Comm_size__name__,		MPI_Python_Comm_size,	METH_VARARGS, MPI_Python_Comm_size__doc__	},
	{ MPI_Python_Comm_rank__name__,		MPI_Python_Comm_rank,	METH_VARARGS, MPI_Python_Comm_rank__doc__	},
	{ 0, 0, 0, 0 }
};


/* "COMM_WORLD" member */
char MPI_Python_COMM_WORLD__doc__[] = "Get the world communicator";
char MPI_Python_COMM_WORLD__name__[] = "COMM_WORLD";
PyObject* MPI_Python_COMM_WORLD( PyObject* self, PyObject* args ) {
	return Py_BuildValue( "i", MPI_COMM_WORLD );
}


/* "Comm_size" member */
char MPI_Python_Comm_size__doc__[] = "Get the size of the communicator";
char MPI_Python_Comm_size__name__[] = "Comm_size";
PyObject* MPI_Python_Comm_size( PyObject* self, PyObject* args ) {
	MPI_Comm	communicator;
	int		size;
	
	/* Obtain arguements */
	if( !PyArg_ParseTuple( args, "i:", &communicator ) ) {
		return NULL;
	}
	
	/* Run function */
	MPI_Comm_size( communicator, &size );
	
	/* Return */
	return Py_BuildValue( "i", size );
}

/* "Comm_rank" member */
char MPI_Python_Comm_rank__doc__[] = "Get the rank of this processor in this communicator";
char MPI_Python_Comm_rank__name__[] = "Comm_rank";
PyObject* MPI_Python_Comm_rank( PyObject* self, PyObject* args ) {
	MPI_Comm	communicator;
	int		rank;
	
	/* Obtain arguements */
	if( !PyArg_ParseTuple( args, "i:", &communicator ) ) {
		return NULL;
	}
	
	/* Run function */
	MPI_Comm_rank( communicator, &rank );
	
	/* Return */
	return Py_BuildValue( "i", rank );
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003, 
**	Steve Quenette, 110 Victoria Street, Melbourne, Victoria, 3053, Australia.
**	Californian Institute of Technology, 1200 East California Boulevard, Pasadena, California, 91125, USA.
**	University of Texas, 1 University Station, Austin, Texas, 78712, USA.
**
** Authors:
**	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
**	Stevan M. Quenette, Visitor in Geophysics, Caltech.
**	Luc Lavier, Research Scientist, The University of Texas. (luc@utig.ug.utexas.edu)
**	Luc Lavier, Research Scientist, Caltech.
**
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2, or (at your option) any
** later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**
** $Id: bindings.c 3245 2006-10-16 12:29:30Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <Python.h>
#include <mpi.h>
#include "StGermain/StGermain.h"
#include "StGermain/FD/FD.h"
#include "Snac/Snac.h"

#include "bindings.h"
#include "misc.h"


struct PyMethodDef Snac_Context_Python_Methods[] = {
	{ Snac_Context_Python_copyright__name__,	Snac_Context_Python_copyright,	METH_VARARGS, Snac_Context_Python_copyright__doc__	},
	{ Snac_Context_Python_New__name__,		Snac_Context_Python_New,	METH_VARARGS, Snac_Context_Python_New__doc__		},
	{ 0, 0, 0, 0 }
};

/* Final definition of a node, element and particle */
struct _Node {
	struct { __Snac_Node };
};

struct _Element {
	struct { __Snac_Element };
};

/* "New" member */
char Snac_Context_Python_New__doc__[] = "Create a new Snac_Context";
char Snac_Context_Python_New__name__[] = "New";
PyObject* Snac_Context_Python_New( PyObject* self, PyObject* args ) {
	PyObject*	pyDictionary;
	Dictionary*	dictionary;
	MPI_Comm	communicator;
	Dictionary_Entry_Value* rankEntry;
	Dictionary_Entry_Value* numProcEntry;
	
	/* Obtain arguements */
	if( !PyArg_ParseTuple( args, "Oi:", &pyDictionary, &communicator ) ) {
		return NULL;
	}
	dictionary = (Dictionary*)( PyCObject_AsVoidPtr( pyDictionary ) );

	/* MPI Stuff */
	if ( NULL == (rankEntry = Dictionary_Get( dictionary, "rank" ) ) ) {
		int rank = 0;
		MPI_Comm_rank( communicator, &rank );
		dictionary->add( dictionary, "rank", Dictionary_Entry_Value_FromUnsignedInt( rank ) );
	}
	if ( NULL == ( numProcEntry = Dictionary_Get( dictionary, "numProcessors" )) ) {
		int numProcessors = 1;
		MPI_Comm_size( communicator, &numProcessors );
		dictionary->add( dictionary, "numProcessors", Dictionary_Entry_Value_FromUnsignedInt( numProcessors ) );
	}

	return PyCObject_FromVoidPtr( Snac_Context_New( 0.0f, 0.0f, sizeof(Snac_Node), sizeof(Snac_Element), communicator, 
		dictionary ), 0 );
}

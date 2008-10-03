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
#include <limits.h>
#include "bindings.h"
#include "misc.h"

struct PyMethodDef Dictionary_Python_Methods[] = {
	{ Dictionary_Python_copyright__name__,		Dictionary_Python_copyright,		METH_VARARGS, Dictionary_Python_copyright__doc__	},
	{ Dictionary_Python_New__name__,		Dictionary_Python_New,			METH_VARARGS, Dictionary_Python_New__doc__		},
	{ Dictionary_Python_Print__name__,		Dictionary_Python_Print,		METH_VARARGS, Dictionary_Python_Print__doc__		},
	{ Dictionary_Python_Delete__name__,		Dictionary_Python_Delete,		METH_VARARGS, Dictionary_Python_Delete__doc__		},
	{ Dictionary_Python_AddString__name__,		Dictionary_Python_AddString,		METH_VARARGS, Dictionary_Python_AddString__doc__	},
	{ Dictionary_Python_AddStruct__name__,		Dictionary_Python_AddStruct,		METH_VARARGS, Dictionary_Python_AddStruct__doc__	},
	{ Dictionary_Python_LoadFromFile__name__,	Dictionary_Python_LoadFromFile,		METH_VARARGS, Dictionary_Python_LoadFromFile__doc__	},
	{ 0, 0, 0, 0 }
};


/* "New" member */
char Dictionary_Python_New__doc__[] = "Create a new Dictionary";
char Dictionary_Python_New__name__[] = "New";
PyObject* Dictionary_Python_New( PyObject* self, PyObject* args ) {
	return PyCObject_FromVoidPtr( Dictionary_New(), 0 );
}

/* "Print" member */
char Dictionary_Python_Print__doc__[] = "Print the Dictionary";
char Dictionary_Python_Print__name__[] = "Print";
PyObject* Dictionary_Python_Print( PyObject* self, PyObject* args) {
	PyObject*	pyDictionary;
	Dictionary*	dictionary;
	Stream*		stream = Journal_Register (Info_Type, "myStream");
	
	/* Obtain arguements */
	if( !PyArg_ParseTuple( args, "O:", &pyDictionary ) ) {
		return NULL;
	}
	dictionary = (Dictionary*)( PyCObject_AsVoidPtr( pyDictionary ) );
	
	/* Run function */
	Print( dictionary, stream );
	
	/* Return */
	Py_INCREF( Py_None );
	return Py_None;
}

/* "Stg_Class_Delete" member */
char Dictionary_Python_Delete__doc__[] = "Stg_Class_Delete/destroy the Dictionary";
char Dictionary_Python_Delete__name__[] = "Stg_Class_Delete";
PyObject* Dictionary_Python_Delete( PyObject* self, PyObject* args ) {
	PyObject*	pyDictionary;
	Dictionary*	dictionary;
	
	/* Obtain arguements */
	if( !PyArg_ParseTuple( args, "O:", &pyDictionary ) ) {
		return NULL;
	}
	dictionary = (Dictionary*)( PyCObject_AsVoidPtr( pyDictionary ) );
	
	/* Run function */
	Stg_Class_Delete( dictionary );
	
	/* Return */
	Py_INCREF( Py_None );
	return Py_None;
}

/* "AddString" member */
char Dictionary_Python_AddString__doc__[] = "Add an item (scalar) to the Dictionary";
char Dictionary_Python_AddString__name__[] = "AddString";
PyObject* Dictionary_Python_AddString( PyObject* self, PyObject* args ) {
	PyObject*	pyDictionary;
	Dictionary*	dictionary;
	char*		name;
	char*		value;
	
	/* Obtain arguements */
	if( !PyArg_ParseTuple( args, "Oss:", &pyDictionary, &name, &value ) ) {
		return NULL;
	}
	dictionary = (Dictionary*)( PyCObject_AsVoidPtr( pyDictionary ) );
	
	/* Run function */
	Dictionary_Add( dictionary, name, Dictionary_Entry_Value_FromString( value ) );
	
	/* Return */
	Py_INCREF( Py_None );
	return Py_None;
}

/* "AddStruct" member */
char Dictionary_Python_AddStruct__doc__[] = "Add a struct (Dictionary) to the Dictionary";
char Dictionary_Python_AddStruct__name__[] = "AddStruct";
PyObject* Dictionary_Python_AddStruct( PyObject* self, PyObject* args ) {
	PyObject*	pyDictionary;
	Dictionary*	dictionary;
	char*		name;
	PyObject*	pyValue;
	Dictionary*	value;
	
	/* Obtain arguements */
	if( !PyArg_ParseTuple( args, "OsO:", &pyDictionary, &name, &pyValue ) ) {
		return NULL;
	}
	dictionary = (Dictionary*)( PyCObject_AsVoidPtr( pyDictionary ) );
	value = (Dictionary*)( PyCObject_AsVoidPtr( pyValue ) );
	
	/* Run function */
	Dictionary_Add( dictionary, name, Dictionary_Entry_Value_FromStruct( value ) );
	
	/* Return */
	Py_INCREF( Py_None );
	return Py_None;
}

/* "AddToList" member */
char Dictionary_Python_AddStringToList__doc__[] = "Add an item (scalar) to a list of the Dictionary";
char Dictionary_Python_AddStringToList__name__[] = "AddStringToList";
PyObject* Dictionary_Python_AddStringToList( PyObject* self, PyObject* args ) {
	PyObject*		pyDictionary;
	Dictionary*		dictionary;
	char*			name;
	char*			value;
	Dictionary_Entry_Value*	list;
	
	/* Obtain arguements */
	if( !PyArg_ParseTuple( args, "Oss:", &pyDictionary, &name, &value ) ) {
		return NULL;
	}
	dictionary = (Dictionary*)( PyCObject_AsVoidPtr( pyDictionary ) );
	
	/* Run function */
	list = Dictionary_Get( dictionary, name );
	Dictionary_Entry_Value_AddElement( list, Dictionary_Entry_Value_FromString( value ) );
	
	/* Return */
	Py_INCREF( Py_None );
	return Py_None;
}

/* "LoadFromFile" member */
char Dictionary_Python_LoadFromFile__doc__[] = "Load the contents of a file into the Dictionary";
char Dictionary_Python_LoadFromFile__name__[] = "LoadFromFile";
PyObject* Dictionary_Python_LoadFromFile( PyObject* self, PyObject* args ) {
	PyObject*	pyDictionary;
	Dictionary*	dictionary;
	XML_IO_Handler*	ioHandler;
	char*		filename;
	
	/* Obtain arguements */
	if( !PyArg_ParseTuple( args, "Os:", &pyDictionary, &filename ) ) {
		return NULL;
	}
	dictionary = (Dictionary*)( PyCObject_AsVoidPtr( pyDictionary ) );
	
	/* Run function */
	ioHandler = XML_IO_Handler_New();
	IO_Handler_ReadAllFromFile( ioHandler, filename, dictionary );
	Stg_Class_Delete( ioHandler );
	
	/* Return */
	Py_INCREF( Py_None );
	return Py_None;
}

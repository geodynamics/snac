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
** $Id: misc.cc 3127 2005-07-26 02:35:37Z AlanLo $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <Python.h>
#include "misc.h"


char pyre_Python_copyright__doc__[] = "";
char pyre_Python_copyright__name__[] = "copyright";

PyObject* pyre_Python_copyright( PyObject* self, PyObject* args ) {
    return Py_BuildValue( "s", "StGermain Python Bindings pyre module: Copyright (c) 2003 Victorian Partnership for Advanced Computing (VPAC) Ltd. Australia." );
}

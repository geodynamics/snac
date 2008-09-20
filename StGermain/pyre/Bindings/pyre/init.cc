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
** $Id: init.cc 3127 2005-07-26 02:35:37Z AlanLo $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <Python.h>
#include "bindings.h"
#include "misc.h"

char pyre_Python_module__doc__[] = "Bindings to pyre objects for use in StGermain.";

extern "C" void initpyre() {
	Py_InitModule4( "pyre", pyre_Python_Methods, pyre_Python_module__doc__, 0, PYTHON_API_VERSION );
	if( PyErr_Occurred() ) {
		Py_FatalError( "Can't initialize module pyre" );
	}

	return;
}


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
*/
/** \file
**  Role:
**	Build the binding table(s).
**
** Assumptions:
**
** Comments:
**
** $Id: bindings.h 3864 2006-10-16 09:21:03Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Context_Python_bindings_h__
#define __Context_Python_bindings_h__
	
	extern struct PyMethodDef Context_Python_Methods[];
	
	extern char Context_Python_Print__name__[];
	extern char Context_Python_Print__doc__[];
	PyObject* Context_Python_Print( PyObject* self, PyObject* args );
	
	extern char Context_Python_Delete__name__[];
	extern char Context_Python_Delete__doc__[];
	PyObject* Context_Python_Delete( PyObject* self, PyObject* args );
	
	extern char Context_Python_Construct__name__[];
	extern char Context_Python_Construct__doc__[];
	PyObject* Context_Python_Construct( PyObject* self, PyObject* args );
	
	extern char Context_Python_Build__name__[];
	extern char Context_Python_Build__doc__[];
	PyObject* Context_Python_Build( PyObject* self, PyObject* args );
	
	extern char Context_Python_Initialise__name__[];
	extern char Context_Python_Initialise__doc__[];
	PyObject* Context_Python_Initialise( PyObject* self, PyObject* args );
	
	extern char Context_Python_Execute__name__[];
	extern char Context_Python_Execute__doc__[];
	PyObject* Context_Python_Execute( PyObject* self, PyObject* args );
	
	extern char Context_Python_Destroy__name__[];
	extern char Context_Python_Destroy__doc__[];
	PyObject* Context_Python_Destroy( PyObject* self, PyObject* args );
	
	extern char Context_Python_Dt__name__[];
	extern char Context_Python_Dt__doc__[];
	PyObject* Context_Python_Dt( PyObject* self, PyObject* args );
	
	extern char Context_Python_Step__name__[];
	extern char Context_Python_Step__doc__[];
	PyObject* Context_Python_Step( PyObject* self, PyObject* args );
	
	extern char Context_Python_GetDictionary__name__[];
	extern char Context_Python_GetDictionary__doc__[];
	PyObject* Context_Python_GetDictionary( PyObject* self, PyObject* args );
	
	extern char Context_Python_SetTime__name__[];
	extern char Context_Python_SetTime__doc__[];
	PyObject* Context_Python_SetTime( PyObject* self, PyObject* args );
	
	extern char Context_Python_SetTimeStep__name__[];
	extern char Context_Python_SetTimeStep__doc__[];
	PyObject* Context_Python_SetTimeStep( PyObject* self, PyObject* args );
	
#endif /* __Context_Python_bindings_h__ */

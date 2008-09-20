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
**
** Assumptions:
**
** Comments:
**
** $Id: Codelet.h 3192 2005-08-25 01:45:42Z AlanLo $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Base_Extensibility_Codelet_h__
#define __Base_Extensibility_Codelet_h__
	
	/* Textual name of this class */
	extern const Type Codelet_Type;

	/* Codelets info */
	#define __Codelet \
		__Stg_Component 
		
	struct Codelet { __Codelet };


	void* Codelet_New(
			Type                                            type,
			Stg_Component_DefaultConstructorFunction*       _defaultConstructor,
			Stg_Component_ConstructFunction*                _construct,
			Stg_Component_BuildFunction*                    _build,
			Stg_Component_InitialiseFunction*               _initialise,
			Stg_Component_ExecuteFunction*                  _execute,
			Stg_Component_DestroyFunction*                  _destroy,
			Name                                            name );
	
	void* _Codelet_New(
			SizeT                                           _sizeOfSelf,
			Type                                            type,
			Stg_Class_DeleteFunction*                       _delete,
			Stg_Class_PrintFunction*                        _print,
			Stg_Class_CopyFunction*                         _copy,
			Stg_Component_DefaultConstructorFunction*       _defaultConstructor,
			Stg_Component_ConstructFunction*                _construct,
			Stg_Component_BuildFunction*                    _build,
			Stg_Component_InitialiseFunction*               _initialise,
			Stg_Component_ExecuteFunction*                  _execute,
			Stg_Component_DestroyFunction*                  _destroy,
			Name                                            name );

	void  _Codelet_Delete( void* codelet );
	void  _Codelet_Print( void* codelet, Stream* stream );
	void* _Codelet_Copy( void* codelet, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );

	void  _Codelet_Build( void* codelet, void* data );
	void  _Codelet_Initialise( void* codelet, void* data );
	void  _Codelet_Execute( void* codelet, void* data );
	void  _Codelet_Destroy( void* codelet, void* data );

#endif

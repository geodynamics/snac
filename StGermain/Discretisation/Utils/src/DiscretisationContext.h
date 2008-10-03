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
** $Id: Context.h 2454 2004-12-21 04:50:42Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Discretisation_Utils_DiscretisationContext_h__
#define __Discretisation_Utils_DiscretisationContext_h__
	
	/* Textual name of this class */
	extern const Type DiscretisationContext_Type;
	
	#define __DiscretisationContext \
		/* General info */ \
		__AbstractContext \
		\
		/* Virtual info */ \
		\
		/* DiscretisationContext info */ \
		FieldVariable_Register*             fieldVariable_Register; \
		Dimension_Index	                    dim;						\
		
	struct DiscretisationContext { __DiscretisationContext };
	
	DiscretisationContext* DiscretisationContext_New( 
		Name                                        name,
		double                                      start,
		double                                      stop,
		MPI_Comm                                    communicator,
		Dictionary*                                 dictionary );
	
	/** Creation implementation / Virtual constructor */
	DiscretisationContext* _DiscretisationContext_New( 
		SizeT                                       sizeOfSelf,
		Type                                        type,
		Stg_Class_DeleteFunction*                   _delete,
		Stg_Class_PrintFunction*                    _print,
		Stg_Class_CopyFunction*                     _copy, 
		Stg_Component_DefaultConstructorFunction*   _defaultConstructor,
		Stg_Component_ConstructFunction*            _construct,
		Stg_Component_BuildFunction*                _build,
		Stg_Component_InitialiseFunction*           _initialise,
		Stg_Component_ExecuteFunction*              _execute,
		Stg_Component_DestroyFunction*              _destroy,
		Name                                        name,
		Bool                                        initFlag,
		AbstractContext_SetDt*                      _setDt,
		double                                      start,
		double                                      stop,
		MPI_Comm                                    communicator,
		Dictionary*                                 dictionary );
	
	/** Initialisation implementation */
	void 						_DiscretisationContext_Init( DiscretisationContext* self );

	/* Virtual Functions ------------------------------------------------------------------------------------------------*/
	
	/* Stg_Class_Delete implementation */
	void						_DiscretisationContext_Delete( void* context );
	
	/* Print implementation */
	void						_DiscretisationContext_Print( void* context, Stream* stream );
	
	void _DiscretisationContext_SetDt( void* context, double dt ) ;

#endif /* __DiscretisationContext_h__*/

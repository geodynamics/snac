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
**	Context specific entry points.
**
** Assumptions:
**
** Comments:
**
** $Id: ContextEntryPoint.h 3462 2006-02-19 06:53:24Z WalterLandry $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Base_Context_ContextEntryPoint_h__
#define __Base_Context_ContextEntryPoint_h__
	
	/* Templates for default entry point type */
	typedef double			(ContextEntryPoint_Dt_Cast)		( void* context );
	typedef double			(ContextEntryPoint_Dt_CallCast)		( void* entryPoint, void* context );
	typedef void			(ContextEntryPoint_Step_Cast)		( void* context, double dt );
	typedef void			(ContextEntryPoint_Step_CallCast)	( void* entryPoint, void* context, double dt );
	#define 			ContextEntryPoint_Dt_CastType		(EntryPoint_CastType_MAX+1)
	#define 			ContextEntryPoint_Step_CastType		(ContextEntryPoint_Dt_CastType+1)
	#define 			ContextEntryPoint_CastType_MAX		(ContextEntryPoint_Step_CastType+1)
	
	/** Textual name of this class */
	extern const Type ContextEntryPoint_Type;
	
	/** ContextEntryPoint info */
	#define __ContextEntryPoint \
		/* General info */ \
		__EntryPoint \
		\
		/* Virtual info */ \
		\
		/* ContextEntryPoint info */
	struct ContextEntryPoint { __ContextEntryPoint };
	
	/* Create a new ContextEntryPoint */
	ContextEntryPoint* ContextEntryPoint_New( Name name, unsigned int castType );
	
	/* Initialise an ContextEntryPoint */
	void ContextEntryPoint_Init( void* contextEntryPoint, Name name, unsigned int castType );
	
	/* Creation implementation */
	ContextEntryPoint* _ContextEntryPoint_New( 
		SizeT				_sizeOfSelf,
		Type				type,
		Stg_Class_DeleteFunction*		_delete,
		Stg_Class_PrintFunction*		_print,
		Stg_Class_CopyFunction*		_copy, 
		EntryPoint_GetRunFunction*	_getRun,
		Name				name,
		unsigned int			castType );
	
	/* Initialisation implementation */
	void _ContextEntryPoint_Init( ContextEntryPoint* self );
	
	
	/* Default GetRun implementation */
	Func_Ptr _ContextEntryPoint_GetRun( void* contextEntryPoint );
	
	/* Context entry point run... one void* arguement passed, double returned. */
	double _ContextEntryPoint_Run_Dt( void* contextEntryPoint, void* data0 );
	
	/* Context entry point run... one void* arguement passed, one double arguement passed */
	void _ContextEntryPoint_Run_Step( void* contextEntryPoint, void* data0, double data1 );
	
#endif /* __Base_Context_ContextEntryPoint_h__ */

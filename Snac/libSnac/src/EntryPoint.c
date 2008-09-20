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
** $Id: EntryPoint.c 3274 2007-03-27 20:25:29Z EunseoChoi $
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>

#include "units.h"
#include "types.h"
#include "shortcuts.h"
#include "Tetrahedra.h"
#include "TetrahedraTables.h"
#include "Material.h"
#include "Element.h"
#include "EntryPoint.h"
#include "UpdateElement.h"
#include "StrainRate.h"
#include "Stress.h"
#include "Force.h"
#include "UpdateNode.h"
#include "Context.h"

/* Textual name of this class */
const Type Snac_EntryPoint_Type = "Snac_EntryPoint";


Snac_EntryPoint* Snac_EntryPoint_New( const Name name, unsigned int castType) {
	return _Snac_EntryPoint_New( sizeof(Snac_EntryPoint), Snac_EntryPoint_Type, _EntryPoint_Delete, 
		_EntryPoint_Print, NULL, _Snac_EntryPoint_GetRun, name, castType );
}

void Snac_EntryPoint_Init( void* snac_EntryPoint, const Name name, unsigned int castType ) {
	Snac_EntryPoint* self = (Snac_EntryPoint*)snac_EntryPoint;
	
	/* General info */
	self->type = Snac_EntryPoint_Type;
	self->_sizeOfSelf = sizeof(Snac_EntryPoint);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _EntryPoint_Delete;
	self->_print = _EntryPoint_Print;
	self->_copy = NULL;
	self->_getRun = _Snac_EntryPoint_GetRun;
	_Stg_Class_Init( (Stg_Class*)self );
	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );
	_EntryPoint_Init( (EntryPoint*)self, castType );
	_ContextEntryPoint_Init( (ContextEntryPoint*)self );
	
	/* Snac_EntryPoint info */
	_Snac_EntryPoint_Init( self );
}

Snac_EntryPoint* _Snac_EntryPoint_New( 
		SizeT					_sizeOfSelf,
		Type					type,
		Stg_Class_DeleteFunction*			_delete,
		Stg_Class_PrintFunction*			_print,
		Stg_Class_CopyFunction*			_copy, 
		EntryPoint_GetRunFunction*		_getRun,
		Name					name,
		unsigned int				castType )
{
	Snac_EntryPoint* self;
	
	/* Allocate memory */
	Journal_Firewall( 
		_sizeOfSelf >= sizeof(Snac_EntryPoint), 
		Journal_Register( ErrorStream_Type, "Snac" ), 
		"Attmpting to create a \"Snac_EntryPoint\" of \"_sizeOfSel\" which is too small" );
	self = (Snac_EntryPoint*)_ContextEntryPoint_New( _sizeOfSelf, type, _delete, _print, _copy, 
		_getRun, name, castType );
	
	/* General info */
	
	/* Virtual info */
	
	/* Snac_EntryPoint info */
	_Snac_EntryPoint_Init( self );
	
	return self;
}

void _Snac_EntryPoint_Init( Snac_EntryPoint* self ) {
	/* General and Virtual info should already be set */
	
	/* Snac_EntryPoint info */
	self->constitutiveRun = _Snac_EntryPoint_Run_Constitutive;
	self->run = EntryPoint_GetRun( self ); /* run is set before this func is called... hence may have invalid value; set */
}


Func_Ptr _Snac_EntryPoint_GetRun( void* snac_EntryPoint ) {
	Snac_EntryPoint* self = (Snac_EntryPoint*)snac_EntryPoint;
	
	switch( self->castType ) {
		case Snac_UpdateElementMomentum_CastType:
			return _Snac_EntryPoint_Run_UpdateElementMomentum;
		
		case Snac_StrainRate_CastType:
			return _Snac_EntryPoint_Run_StrainRate;
		
		case Snac_Stress_CastType:
			return _Snac_EntryPoint_Run_Stress;
		
		case Snac_Force_CastType:
			return _Snac_EntryPoint_Run_Force;
		
		case Snac_UpdateNodeMomentum_CastType:
			return _Snac_EntryPoint_Run_UpdateNodeMomentum;
		
		case Snac_Constitutive_CastType:
			return self->constitutiveRun;
		
		default:
			return _ContextEntryPoint_GetRun( self );
	}
}


void _Snac_EntryPoint_Run_UpdateElementMomentum( 
		void*					entryPoint, 
		void*					context, 
		Element_LocalIndex			element_lI, 
		double*					elementMinLengthScale )
{
	Snac_EntryPoint*			self = (Snac_EntryPoint*)entryPoint;
	Hook_Index				hookIndex;
	
	#ifdef USE_PROFILE
		Stg_CallGraph_Push( stgCallGraph, _Snac_EntryPoint_Run_UpdateElementMomentum, self->name );
	#endif

	for( hookIndex = 0; hookIndex < self->hooks->count; hookIndex++ ) {
		(*(Snac_UpdateElementMomentum_Cast*)((Hook*)self->hooks->data[hookIndex])->funcPtr)
			( context, element_lI, elementMinLengthScale );
	}

	#ifdef USE_PROFILE
		Stg_CallGraph_Pop( stgCallGraph );
	#endif
}

void _Snac_EntryPoint_Run_StrainRate(
		void*					entryPoint, 
		void*					context, 
		Element_LocalIndex			element_lI )
{
	Snac_EntryPoint*			self = (Snac_EntryPoint*)entryPoint;
	Hook_Index				hookIndex;
	
	#ifdef USE_PROFILE
		Stg_CallGraph_Push( stgCallGraph, _Snac_EntryPoint_Run_StrainRate, self->name );
	#endif

	for( hookIndex = 0; hookIndex < self->hooks->count; hookIndex++ ) {
		(*(Snac_StrainRate_Cast*)((Hook*)self->hooks->data[hookIndex])->funcPtr)
			( context, element_lI );
	}

	#ifdef USE_PROFILE
		Stg_CallGraph_Pop( stgCallGraph );
	#endif
}

void _Snac_EntryPoint_Run_Stress(
		void*					entryPoint, 
		void*					context, 
		Element_LocalIndex			element_lI )
{
	Snac_EntryPoint*			self = (Snac_EntryPoint*)entryPoint;
	Hook_Index				hookIndex;
	
	#ifdef USE_PROFILE
		Stg_CallGraph_Push( stgCallGraph, _Snac_EntryPoint_Run_Stress, self->name );
	#endif

	for( hookIndex = 0; hookIndex < self->hooks->count; hookIndex++ ) {
		(*(Snac_Stress_Cast*)((Hook*)self->hooks->data[hookIndex])->funcPtr)
			( context, element_lI );
	}

	#ifdef USE_PROFILE
		Stg_CallGraph_Pop( stgCallGraph );
	#endif
}

void _Snac_EntryPoint_Run_Force(
		void*					entryPoint, 
		void*					context, 
		Node_LocalIndex				node_lI, 
		double					speedOfSnd, 
		Mass*					mass, 
		Mass*					inertialMass, 
		Force*					force, 
		Force*					balance )
{
	Snac_EntryPoint*			self = (Snac_EntryPoint*)entryPoint;
	Hook_Index				hookIndex;
	
	#ifdef USE_PROFILE
		Stg_CallGraph_Push( stgCallGraph, _Snac_EntryPoint_Run_Force, self->name );
	#endif

	for( hookIndex = 0; hookIndex < self->hooks->count; hookIndex++ ) {
		(*(Snac_Force_Cast*)((Hook*)self->hooks->data[hookIndex])->funcPtr)
			( context, node_lI, speedOfSnd, mass, inertialMass, force, balance );
	}

	#ifdef USE_PROFILE
		Stg_CallGraph_Pop( stgCallGraph );
	#endif
}

void _Snac_EntryPoint_Run_UpdateNodeMomentum(
		void*					entryPoint, 
		void*					context,
		Node_LocalIndex				node_lI,
		Mass					inertialMass, 
		Force					force )
{
	Snac_EntryPoint*			self = (Snac_EntryPoint*)entryPoint;
	Hook_Index				hookIndex;
	
	#ifdef USE_PROFILE
		Stg_CallGraph_Push( stgCallGraph, _Snac_EntryPoint_Run_UpdateNodeMomentum, self->name );
	#endif

	for( hookIndex = 0; hookIndex < self->hooks->count; hookIndex++ ) {
		(*(Snac_UpdateNodeMomentum_Cast*)((Hook*)self->hooks->data[hookIndex])->funcPtr)
			( context, node_lI, inertialMass, force );
	}

	#ifdef USE_PROFILE
		Stg_CallGraph_Pop( stgCallGraph );
	#endif
}

void _Snac_EntryPoint_Run_Constitutive(
		void*					entryPoint, 
		void*					context, 
		Element_LocalIndex			element_lI )
{
	Snac_EntryPoint*			self = (Snac_EntryPoint*)entryPoint;
	Hook_Index				hookIndex;
	
	#ifdef USE_PROFILE
		Stg_CallGraph_Push( stgCallGraph, _Snac_EntryPoint_Run_Constitutive, self->name );
	#endif

	for( hookIndex = 0; hookIndex < self->hooks->count; hookIndex++ ) {
		(*(Snac_Constitutive_Cast*)((Hook*)self->hooks->data[hookIndex])->funcPtr)
			( context, element_lI );
	}
	
	#ifdef USE_PROFILE
		Stg_CallGraph_Pop( stgCallGraph );
	#endif
}

void Snac_EntryPoint_ChangeRunConstitutive( void* entryPoint, Snac_Constitutive_CallCast* constitutiveRun ) {
	Snac_EntryPoint*			self = (Snac_EntryPoint*)entryPoint;
	
	self->constitutiveRun = constitutiveRun;
	self->run = self->constitutiveRun;
}

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
** $Id: EntryPoint.c 3250 2006-10-23 06:15:18Z LukeHodkinson $
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "EntryPoint.h"

/* Textual name of this class */
const Type SnacRemesher_EntryPoint_Type = "SnacRemesher_EntryPoint";


SnacRemesher_EntryPoint* SnacRemesher_EntryPoint_New( const Name name, unsigned int castType ) {
	return _SnacRemesher_EntryPoint_New( 
		sizeof(SnacRemesher_EntryPoint), 
		SnacRemesher_EntryPoint_Type, 
		_EntryPoint_Delete, 
		_EntryPoint_Print, 
		NULL, 
		_SnacRemesher_EntryPoint_GetRun, 
		name, 
		castType );
}

void SnacRemesher_EntryPoint_Init( void* snac_EntryPoint, const Name name, unsigned int castType ) {
	SnacRemesher_EntryPoint* self = (SnacRemesher_EntryPoint*)snac_EntryPoint;
	
	/* General info */
	self->type = SnacRemesher_EntryPoint_Type;
	self->_sizeOfSelf = sizeof(SnacRemesher_EntryPoint);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _EntryPoint_Delete;
	self->_print = _EntryPoint_Print;
	self->_copy = NULL;
	self->_getRun = _SnacRemesher_EntryPoint_GetRun;
	_Stg_Class_Init( (Stg_Class*)self );
	_Stg_Object_Init( (Stg_Object*)self, name, NON_GLOBAL );
	_EntryPoint_Init( (EntryPoint*)self, castType );
	_ContextEntryPoint_Init( (ContextEntryPoint*)self );
	_Snac_EntryPoint_Init( (Snac_EntryPoint*)self );
	
	/* SnacRemesher_EntryPoint info */
	_SnacRemesher_EntryPoint_Init( self );
}

SnacRemesher_EntryPoint* _SnacRemesher_EntryPoint_New( 
		SizeT					_sizeOfSelf,
		Type					type,
		Stg_Class_DeleteFunction*			_delete,
		Stg_Class_PrintFunction*			_print,
		Stg_Class_CopyFunction*			_copy, 
		EntryPoint_GetRunFunction*		_getRun,
		Name					name,
		unsigned int				castType )
{
	SnacRemesher_EntryPoint* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(SnacRemesher_EntryPoint) );
	self = (SnacRemesher_EntryPoint*)_Snac_EntryPoint_New( 
		_sizeOfSelf, 
		type, 
		_delete, 
		_print, 
		_copy, 
		_getRun, 
		name, 
		castType );
	
	/* General info */
	
	/* Virtual info */
	
	/* SnacRemesher_EntryPoint info */
	_SnacRemesher_EntryPoint_Init( self );
	
	return self;
}

void _SnacRemesher_EntryPoint_Init( SnacRemesher_EntryPoint* self ) {
	/* General and Virtual info should already be set */
	
	/* SnacRemesher_EntryPoint info */
}


Func_Ptr _SnacRemesher_EntryPoint_GetRun( void* snac_EntryPoint ) {
	SnacRemesher_EntryPoint* self = (SnacRemesher_EntryPoint*)snac_EntryPoint;
	
	switch( self->castType ) {
		case SnacRemesher_InterpolateNode_CastType:
			return _SnacRemesher_EntryPoint_Run_InterpolateNode;
		
		case SnacRemesher_InterpolateElement_CastType:
			return _SnacRemesher_EntryPoint_Run_InterpolateElement;
		default:
			return _Snac_EntryPoint_GetRun( self );
	}
}


void _SnacRemesher_EntryPoint_Run_RecoverNode( void* entryPoint, void* context, unsigned nodeInd )
{
	SnacRemesher_EntryPoint* self = (SnacRemesher_EntryPoint*)entryPoint;
	Hook_Index hookIndex;
	
	#ifdef USE_PROFILE
		Stg_CallGraph_Push( stgCallGraph, _SnacRemesher_EntryPoint_Run_InterpolateNode, self->name );
	#endif

	for( hookIndex = 0; hookIndex < self->hooks->count; hookIndex++ ) {
		void*	funcPtr;

		funcPtr = ((Hook*)self->hooks->data[hookIndex])->funcPtr;
		(*(SnacRemesher_RecoverNode_Cast*)funcPtr)( context, nodeInd );
	}

	#ifdef USE_PROFILE
		Stg_CallGraph_Pop( stgCallGraph );
	#endif
}


void _SnacRemesher_EntryPoint_Run_InterpolateNode( void* entryPoint, void* context, 
						   unsigned nodeInd, unsigned elementInd, unsigned tetInd, 
						   unsigned* tetNodeInds, double* weights, 
						   Snac_Node* dstNodes )
{
	SnacRemesher_EntryPoint* self = (SnacRemesher_EntryPoint*)entryPoint;
	Hook_Index hookIndex;
	
	#ifdef USE_PROFILE
		Stg_CallGraph_Push( stgCallGraph, _SnacRemesher_EntryPoint_Run_InterpolateNode, self->name );
	#endif

	for( hookIndex = 0; hookIndex < self->hooks->count; hookIndex++ ) {
		void*	funcPtr;

		funcPtr = ((Hook*)self->hooks->data[hookIndex])->funcPtr;
		(*(SnacRemesher_InterpolateNode_Cast*)funcPtr)( context, 
								nodeInd, elementInd, tetInd, 
								tetNodeInds, weights, 
								dstNodes );
	}

	#ifdef USE_PROFILE
		Stg_CallGraph_Pop( stgCallGraph );
	#endif
}

void _SnacRemesher_EntryPoint_Run_InterpolateElement( void* entryPoint, void* context, 
													  unsigned dstElementInd, unsigned dstTetInd )
{
	SnacRemesher_EntryPoint* self = (SnacRemesher_EntryPoint*)entryPoint;
	Hook_Index hookIndex;
	
	#ifdef USE_PROFILE
		Stg_CallGraph_Push( stgCallGraph, _SnacRemesher_EntryPoint_Run_InterpolateElement, self->name );
	#endif

	for( hookIndex = 0; hookIndex < self->hooks->count; hookIndex++ ) {
		void*	funcPtr;

		funcPtr = ((Hook*)self->hooks->data[hookIndex])->funcPtr;
		(*(SnacRemesher_InterpolateElement_Cast*)funcPtr)( context, 
														   dstElementInd, dstTetInd );
	}

	#ifdef USE_PROFILE
		Stg_CallGraph_Pop( stgCallGraph );
	#endif
}


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
** $Id: Context.c 3250 2006-10-23 06:15:18Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "EntryPoint.h"
#include "Context.h"
#include <stdio.h>

void SnacRemesher_Context_Print( void* _context ) {
	SnacRemesher_Context*		self = (SnacRemesher_Context*)_context;
	
	printf( "SnacRemesher_Context:\n" );
	printf( 
		"\tcondition: %s\n", 
		self->condition == SnacRemesher_Off ? "off" : 
			self->condition == SnacRemesher_OnTimeStep ? "OnTimeStep" :
			self->condition == SnacRemesher_OnMinLengthScale ? "onMinLengthScale" : "Invalid value!" );
	
	printf( "\tOnTimeStep: %u", self->OnTimeStep );
	printf( "\tonMinLengthScale: %g\n", self->onMinLengthScale );
	
	printf( "\tinterpolateNodeK: %u\n", self->interpolateNodeK );
	printf( "\tinterpolateElementK: %u\n", self->interpolateElementK );
	printf( "\tcopyElementK: %u\n", self->copyElementK );
}

void SnacRemesher_InterpolateNodeFunction( void* _context, SnacRemesher_Context* self, 
					   unsigned nodeInd, unsigned elementInd, unsigned tetInd, 
					   unsigned* tetNodeInds, double* weights, 
					   Snac_Node* dstNodes )
{
	Snac_Context*			context = (Snac_Context*)_context;
	
	SnacRemesher_InterpolateNodeMacro( context, self, 
					   nodeInd, elementInd, tetInd, 
					   tetNodeInds, weights, 
					   dstNodes );
}

void SnacRemesher_InterpolateElementFunction( 
		void*					_context, 
		SnacRemesher_Context*	self, 
		Element_LocalIndex		dstEltInd, 
		Tetrahedra_Index		dstTetInd, 
		SnacRemesher_Element*	dstEltArray, 
		Element_DomainIndex		srcEltInd, 
		Tetrahedra_Index		srcTetInd )
{
	Snac_Context*			context = (Snac_Context*)_context;
		
	SnacRemesher_InterpolateElementMacro( 
						context, self,
						dstEltInd, dstTetInd, dstEltArray,
						srcEltInd, srcTetInd );
}


void SnacRemesher_CopyElementFunction( 
		void*					_context, 
		SnacRemesher_Context*	self, 
		Element_LocalIndex		eltInd,  
		Tetrahedra_Index		tetInd, 
		SnacRemesher_Element*	eltArray )
{
	Snac_Context*			context = (Snac_Context*)_context;
		
	SnacRemesher_CopyElementMacro( context, self, eltInd, tetInd, eltArray ); 
}

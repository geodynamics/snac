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
** $Id: Output.c 1792 2004-07-30 05:42:39Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "Output.h"
#include "Context.h"
#include "Element.h"
#include "Register.h"
#include <stdio.h>
#include <math.h>
#include <assert.h>

void _SnacMaxwell_WriteViscosity( void* _context ) {
	Snac_Context*				context = (Snac_Context*) _context;

	if( isTimeToDump( context ) )
		_SnacMaxwell_DumpViscosity( context );
	if( isTimeToCheckpoint( context ) )
		_SnacMaxwell_CheckpointViscosity( context );
}


void _SnacMaxwell_DumpViscosity( void* _context ) {
	Snac_Context*				context = (Snac_Context*) _context;
	SnacMaxwell_Context*		contextExt = ExtensionManager_Get(
												context->extensionMgr,
												context,
												SnacMaxwell_ContextHandle );
	Element_LocalIndex			element_lI;

#if DEBUG
	printf( "In %s()\n", __func__ );
#endif
	
	for( element_lI = 0; element_lI < context->mesh->elementLocalCount; element_lI++ ) {
		Snac_Element* 				element = Snac_Element_At( context, element_lI );
		SnacMaxwell_Element*		elementExt = ExtensionManager_Get(
													context->mesh->elementExtensionMgr,
													element,
													SnacMaxwell_ElementHandle );
		/* Take average of tetra viscosity for the element */
		Tetrahedra_Index		tetra_I;
		float					viscosity = 0.0f;
		for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ )
			viscosity += elementExt->viscosity[tetra_I]/Tetrahedra_Count;
		assert(viscosity >= 0.0);
		
		if(viscosity > 0.0)
			viscosity = log10(viscosity);
		
		fwrite( &viscosity, sizeof(float), 1, contextExt->viscosityOut );
	}
	fflush( contextExt->viscosityOut );
}

void _SnacMaxwell_CheckpointViscosity( void* _context ) {
	Snac_Context*				context = (Snac_Context*) _context;
	SnacMaxwell_Context*		contextExt = ExtensionManager_Get(
												context->extensionMgr,
												context,
												SnacMaxwell_ContextHandle );
	Element_LocalIndex			element_lI;

#if DEBUG
	printf( "In %s()\n", __func__ );
#endif
	
	for( element_lI = 0; element_lI < context->mesh->elementLocalCount; element_lI++ ) {
		Snac_Element* 				element = Snac_Element_At( context, element_lI );
		SnacMaxwell_Element*			elementExt = ExtensionManager_Get(
														context->mesh->elementExtensionMgr,
														element,
														SnacMaxwell_ElementHandle );
		/* Take average of tetra viscosity for the element */
		Tetrahedra_Index		tetra_I;
		float                           viscosity = 0.0f;
		for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ )
			viscosity += elementExt->viscosity[tetra_I]/Tetrahedra_Count;
		assert(viscosity >= 0.0);
		
		if(viscosity > 0.0)
			viscosity = log10(viscosity);
		
		fwrite( &viscosity, sizeof(float), 1, contextExt->viscosityCheckpoint );
	}
	fflush( contextExt->viscosityCheckpoint );
}

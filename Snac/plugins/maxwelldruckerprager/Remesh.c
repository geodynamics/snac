/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003,
**	Pururav Thoutireddy, 
**	Californian Institute of Technology, 1200 East California Boulevard, Pasadena, California, 91125, USA.
**      110 Victoria Street, Melbourne, Victoria, 3053, Australia.
**	University of Texas, 1 University Station, Austin, Texas, 78712, USA.
**
** Authors:
**	Pururav Thoutireddy, Staff Scientist, Caltech
**      Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
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
** $Id: Remesh.c 3250 2006-10-23 06:15:18Z LukeHodkinson $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "Element.h"
#include "Remesh.h"
#include "Register.h"

void _SnacMaxwellDruckerPrager_InterpolateElement( void* _context, 
					   unsigned dstElementInd, unsigned dstTetInd, 
					   Snac_Element* dstElements, 
					   unsigned srcElementInd, unsigned srcTetInd )
{
	Snac_Context*                   context = (Snac_Context*)_context;
	Snac_Element*			element = (Snac_Element*)ExtensionManager_At( context->mesh->elementExtensionMgr, 
										      dstElements, 
										      dstElementInd );
	SnacMaxwellDruckerPrager_Element*	elementExt = ExtensionManager_Get( context->mesh->elementExtensionMgr, 
									   element, 
									   SnacMaxwellDruckerPrager_ElementHandle );
	Snac_Element*			fromElement;
	SnacMaxwellDruckerPrager_Element*	fromElementExt;

#ifdef DEBUG
	printf( "element_lI: %u, fromElement_lI: %u\n", dstElementInd, srcElementInd );
#endif
	
	fromElement = Snac_Element_At( context, dstElementInd );
	fromElementExt = ExtensionManager_Get( context->mesh->elementExtensionMgr, 
					       fromElement, 
					       SnacMaxwellDruckerPrager_ElementHandle );

	elementExt->plasticStrain[dstTetInd] = fromElementExt->plasticStrain[srcTetInd];
}

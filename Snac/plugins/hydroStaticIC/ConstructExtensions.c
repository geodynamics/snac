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
** $Id: ConstructExtensions.c 3140 2005-08-30 18:35:09Z EunseoChoi $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "Register.h"
#include "VariableConditions.h"
#include "ConstructExtensions.h"
#include <assert.h>
#include <limits.h>
#ifndef PATH_MAX
	#define PATH_MAX 1024
#endif

void _SnacHydroStaticIC_ConstructExtensions( void* _context, void* data ) {
	Snac_Context*				context = (Snac_Context*)_context;
	Snac_Element				tmpElement;

	/* The hydro static stress variable purposely covers many variables... its more of a convenience thing. */
	#define					hydroStaticComponentCount	24
	Index					hydroStaticOffsetCount = hydroStaticComponentCount;
	SizeT					hydroStaticOffsets[hydroStaticComponentCount] = {
							GetOffsetOfMember( tmpElement, strainRate ),
							GetOffsetOfMember( tmpElement, stress ),
							GetOffsetOfMember( tmpElement, rzbo ),
							GetOffsetOfMember( tmpElement, hydroPressure ),
							GetOffsetOfMember( tmpElement, tetra[0].strainRate ),
							GetOffsetOfMember( tmpElement, tetra[0].stress ),
							GetOffsetOfMember( tmpElement, tetra[1].strainRate ),
							GetOffsetOfMember( tmpElement, tetra[1].stress ),
							GetOffsetOfMember( tmpElement, tetra[2].strainRate ),
							GetOffsetOfMember( tmpElement, tetra[2].stress ),
							GetOffsetOfMember( tmpElement, tetra[3].strainRate ),
							GetOffsetOfMember( tmpElement, tetra[3].stress ),
							GetOffsetOfMember( tmpElement, tetra[4].strainRate ),
							GetOffsetOfMember( tmpElement, tetra[4].stress ),
							GetOffsetOfMember( tmpElement, tetra[5].strainRate ),
							GetOffsetOfMember( tmpElement, tetra[5].stress ),
							GetOffsetOfMember( tmpElement, tetra[6].strainRate ),
							GetOffsetOfMember( tmpElement, tetra[6].stress ),
							GetOffsetOfMember( tmpElement, tetra[7].strainRate ),
							GetOffsetOfMember( tmpElement, tetra[7].stress ),
							GetOffsetOfMember( tmpElement, tetra[8].strainRate ),
							GetOffsetOfMember( tmpElement, tetra[8].stress ),
							GetOffsetOfMember( tmpElement, tetra[9].strainRate ),
							GetOffsetOfMember( tmpElement, tetra[9].stress ) };
	Variable_DataType			hydroStaticDataTypes[hydroStaticComponentCount] = {
							Variable_DataType_Double,
							Variable_DataType_Double,
							Variable_DataType_Double,
							Variable_DataType_Double,
							Variable_DataType_Double,
							Variable_DataType_Double,
							Variable_DataType_Double,
							Variable_DataType_Double,
							Variable_DataType_Double,
							Variable_DataType_Double,
							Variable_DataType_Double,
							Variable_DataType_Double,
							Variable_DataType_Double,
							Variable_DataType_Double,
							Variable_DataType_Double,
							Variable_DataType_Double,
							Variable_DataType_Double,
							Variable_DataType_Double,
							Variable_DataType_Double,
							Variable_DataType_Double,
							Variable_DataType_Double,
							Variable_DataType_Double,
							Variable_DataType_Double,
							Variable_DataType_Double };
	Index					hydroStaticDataTypeCounts[hydroStaticComponentCount] = {
							1,
							1,
							1,
							1,
							9,
							9,
							9,
							9,
							9,
							9,
							9,
							9,
							9,
							9,
							9,
							9,
							9,
							9,
							9,
							9,
							9,
							9,
							9,
							9 };

	#if DEBUG
		printf( "In %s()\n", __func__ );
	#endif

	/* Create the StGermain variable hydrostatic, which is stored on an element extension */
	Variable_New(
		"hydroStatic",
		hydroStaticOffsetCount,
		hydroStaticOffsets,
		hydroStaticDataTypes,
		hydroStaticDataTypeCounts,
		0,
		&ExtensionManager_GetFinalSize( context->mesh->elementExtensionMgr ),
		&context->mesh->layout->decomp->elementDomainCount,
		(void**)&context->mesh->element,
		context->variable_Register );

	/* HydroStaticIC variables */

	/* Build the temperature IC and BC managers */

	/* Prepare the dump file */
}

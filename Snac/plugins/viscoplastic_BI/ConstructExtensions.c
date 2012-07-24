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
** $Id: ConstructExtensions.c 3234 2006-06-23 04:12:40Z LaetitiaLePourhiet $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "ViscoPlastic.h"
#include <assert.h>
#include <limits.h>
#ifndef PATH_MAX
	#define PATH_MAX 1024
#endif

void _SnacViscoPlastic_ConstructExtensions( void* _context, void* data ) {
	Snac_Context*				context = (Snac_Context*)_context;
	SnacViscoPlastic_Context*			contextExt = ExtensionManager_Get(
																		  context->extensionMgr,
																		  context,
																		  SnacViscoPlastic_ContextHandle );
	Snac_Element                            tmpElement;
	SnacViscoPlastic_Element*               tmpElementExt = ExtensionManager_Get(
																				 context->mesh->elementExtensionMgr,
																				 &tmpElement,
																				 SnacViscoPlastic_ElementHandle );
	char					tmpBuf[PATH_MAX];

#if DEBUG
	if( context->rank == 0 )		printf( "In %s()\n", __func__ );
#endif

	/* Prepare the dump file */
	sprintf( tmpBuf, "%s/plStrain.%u", context->outputPath, context->rank );
	if( (contextExt->plStrainOut = fopen( tmpBuf, "w+" )) == NULL ) {
		assert( contextExt->plStrainOut /* failed to open file for writing */ );
		abort();
	}
	sprintf( tmpBuf, "%s/plStrainCP.%u", context->outputPath, context->rank );
	if( (contextExt->plStrainCheckpoint = fopen( tmpBuf, "w+" )) == NULL ) {
		assert( contextExt->plStrainCheckpoint /* failed to open file for writing */ );
		abort();
	}
	sprintf( tmpBuf, "%s/viscosity.%u", context->outputPath, context->rank );
	if( (contextExt->viscOut = fopen( tmpBuf, "w+" )) == NULL ) {
		assert( contextExt->viscOut /* failed to open file for writing */ );
		abort();
	}
	sprintf( tmpBuf, "%s/viscosityCP.%u", context->outputPath, context->rank );
	if( (contextExt->viscCheckpoint = fopen( tmpBuf, "w+" )) == NULL ) {
		assert( contextExt->viscCheckpoint /* failed to open file for writing */ );
		abort();
	}
}

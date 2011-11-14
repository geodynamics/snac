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
** $Id: Destory.c 3125 2005-07-25 21:32:45Z EunseoChoi $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "ViscoPlastic.h"
#include <stdio.h>
#include <math.h>
#include <assert.h>


void _SnacViscoPlastic_Destroy( void* _context ) {
	Snac_Context*				context = (Snac_Context*) _context;
	SnacViscoPlastic_Context*		contextExt = ExtensionManager_Get(
								context->extensionMgr,
								context,
								SnacViscoPlastic_ContextHandle );

	/* Close any of the following if present. */
	if( contextExt->plStrainOut )
		fclose( contextExt->plStrainOut );	
	if( contextExt->plStrainCheckpoint )
		fclose( contextExt->plStrainCheckpoint );	
	if( contextExt->avgPlStrainCheckpoint )
		fclose( contextExt->avgPlStrainCheckpoint );	
	if( contextExt->viscOut )
		fclose( contextExt->viscOut );	
	if( contextExt->viscCheckpoint )
		fclose( contextExt->viscCheckpoint );	

}


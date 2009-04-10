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
** $Id: Output.c 3104 2005-07-14 22:16:41Z EunseoChoi $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "Output.h"
#include "Register.h"
#include <stdio.h>
#include <math.h>
#include <assert.h>
#ifndef PATH_MAX
    #define PATH_MAX 1024
#endif

void _SnacWinklerForce_DumpInitForce( void* _context )
{

	Snac_Context*		context = (Snac_Context*)_context;

	/* 	if( context->timeStep - context->restartTimestep == 1 ) { */
	/*
	* It seems fine to record isoForce.* only once at time step = 1. 
	* although there might be a pitfall to be found yet.
	*/
	if( context->timeStep - context->restartTimestep == 1 ) {
		FILE* fp;
		char path[PATH_MAX];
		Node_LocalIndex       node_lI;

		sprintf( path, "%s/isoForce.%d", context->outputPath, context->rank );
		if( ( fp = fopen( path, "w") ) == NULL ) {
			Journal_Firewall( 0, "Failed to open %s!!\n", path );
		}

		for( node_lI = 0; node_lI < context->mesh->nodeLocalCount; node_lI++ ) {
			Snac_Node*			node = Snac_Node_At( context, node_lI );
			float residualF;

			residualF = node->residualFr;
			fwrite( &residualF, sizeof(float), 1, fp );
		}
		fflush( fp );

		fclose( fp );
	}
}

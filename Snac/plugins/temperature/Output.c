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
** $Id: Context.c 1095 2004-03-28 00:51:42Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "Output.h"
#include "Context.h"
#include "Node.h"
#include "Register.h"
#include <stdio.h>

void _SnacTemperature_DumpTemp( void* _context ) {
	Snac_Context*				context = (Snac_Context*) _context;
	SnacTemperature_Context*		contextExt = ExtensionManager_Get(
							context->extensionMgr,
							context,
							SnacTemperature_ContextHandle );
	Node_LocalIndex				node_lI;


	if( context->timeStep == 0 || (context->timeStep - 1) % context->dumpEvery == 0 ) {
		#if DEBUG
			printf( "In %s()\n", __func__ );
		#endif

		for( node_lI = 0; node_lI < context->mesh->nodeLocalCount; node_lI++ ) {
			Snac_Node* 				node = Snac_Node_At( context, node_lI );
			SnacTemperature_Node*			nodeExt = ExtensionManager_Get(
											context->mesh->nodeExtensionMgr,
											node,
											SnacTemperature_NodeHandle );
			float					temperature = nodeExt->temperature;
			fwrite( &temperature, sizeof(float), 1, contextExt->temperatureOut );
		}
		fflush( contextExt->temperatureOut );
	}
}


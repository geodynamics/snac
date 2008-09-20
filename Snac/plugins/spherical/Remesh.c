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
** $Id: Remesh.c 2498 2005-01-07 03:57:00Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "Node.h"
#include "Remesh.h"
#include "Register.h"

#include <string.h>
#include <math.h>
#ifndef PI
	#ifndef M_PIl
		#ifndef M_PI
			#define PI 3.1415927
		#else
			#define PI M_PI
		#endif
	#else
		#define PI M_PIl
	#endif
#endif

void _SnacExchangerForceBC_Remesh( void* _context, void* data ) {
	Snac_Context*                   context = (Snac_Context*)_context;

	/* If criterion has passed */ {
		SnacExchangerForceBC_Mesh*	meshExt = ExtensionManager_Get( context->meshExtensionMgr, context, 
							SnacExchangerForceBC_MeshHandle );
		
		/* initialise new nodes coordinates */
		_SnacExchangerForceBC_NewCoords( context );

		/* interpolate current values onto new coordinates */
		_SnacExchangerForceBC_InterpolateNodes( context );
	}
}

void _SnacExchangerForceBC_NewCoords( void* context ) {
	/* for each "top" node, 
		take theta & phi from initialTPR
		at that TP, work out r by finding the face of the top surface it lies in, and interpolating
	   for each ! "top" node,
	   	take theta & phi from initialTPR
		at that TP, work out r as proportial to node index from bottom
	*/
}

void _SnacExchangerForceBC_InterpolateNodes( void* context ) {
	/* for all nodes, interpolate velocity */
}


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
** $Id: Mesh.c 3274 2007-03-27 20:25:29Z EunseoChoi $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>

#include "types.h"
#include "units.h"
#include "Material.h"
#include "Context.h"
#include "Mesh.h"
#include <stdio.h>
#include <string.h>


void Snac_Mesh_Delete( void* mesh ) {
	Snac_Mesh*	self = (Snac_Mesh*)mesh;
	
	if( self->force ) {
		Memory_Free( self->force );
		self->force = 0;
	}
}

void Snac_Mesh_Print( void* mesh, Stream* stream ) {
	Snac_Mesh*	self = (Snac_Mesh*)mesh;
	Node_LocalIndex	node_lI;
	
	Journal_Printf( stream, "Snac_Mesh:\n" );
	
	Journal_Printf( stream, "\tnodeLocalCount: %u\n", self->nodeLocalCount );
}

void Snac_Mesh_Build( void* mesh, void* _context ) {
	Snac_Mesh*	self = (Snac_Mesh*)mesh;
	Snac_Context*	context = (Snac_Context*)_context;
	
	self->debug = context->debug;
	self->rank = context->rank;
	Journal_Printf( self->debug, "In: %s\n", __func__ );
	
	self->nodeLocalCount = context->mesh->nodeLocalCount;
}

void Snac_Mesh_IC( void* mesh ) {
	Snac_Mesh*	self = (Snac_Mesh*)mesh;
	
	Journal_Printf( self->debug, "In: %s\n", __func__ );
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003,
**	Steve Quenette, 110 Victoria Street, Melbourne, Victoria, 3053, Australia.
**	Californian Institute of Technology, 1200 East California Boulevard, Pasadena, California, 91125, USA.
**	University of Texas, 1 University Station, Austin, Texas, 78712, USA.
**
** Authors:
**	Eh Tan, ?, Caltech (tan2@gps.caltech.edu)
**	Pururav Thoutireddy, ?, Caltech (puru@cacr.caltech.edu)
**	Eun-seo Choi, ?, Caltech (ces74@gps.caltech.edu)
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
** $Id: BoundedBox.cc 953 2004-03-04 12:38:46Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <vector>
extern "C" {
	#include <mpi.h>
	#include <StGermain/StGermain.h>
	#include <StGermain/FD/FD.h>
	#include <Snac/Snac.h>
}
#include "global_bbox.h"

void GlobalBoundedBox( Exchanger::BoundedBox& bbox, const void* _context ) {
	Snac_Context* context = (Snac_Context*)_context;
	BlockGeometry* geometry = (BlockGeometry*)context->mesh->layout->elementLayout->geometry;

	/* RML->min/max values adjusted in exchangerForceBC module now ... should be right. */
	bbox[0][0] = geometry->min[0];
	bbox[0][1] = geometry->min[1];
	bbox[0][2] = geometry->min[2];
	bbox[1][0] = geometry->max[0];
	bbox[1][1] = geometry->max[1];
	bbox[1][2] = geometry->max[2];
}

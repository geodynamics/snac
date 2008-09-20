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
** $Id: Element.c 3275 2007-03-28 20:07:08Z EunseoChoi $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>

#include "units.h"
#include "types.h"
#include "shortcuts.h"
#include "Material.h"
#include "Tetrahedra.h"
#include "TetrahedraTables.h"
#include "Element.h"
#include "stdio.h"

void Snac_Element_Tetrahedra_Surface_Print( void* surface, Stream* stream ) {
	Snac_Element_Tetrahedra_Surface* self = (Snac_Element_Tetrahedra_Surface*)surface;

	Journal_Printf( stream, "Snac_Element_Tetrahedra_Surface:\n" );
	Journal_Printf( stream, "\tarea: %g\n", self->area );
	Journal_Printf( stream, "\tnormal[3]: {%g, %g, %g}\n", self->normal[0], self->normal[1], self->normal[2] );
}


void Snac_Element_Tetrahedra_Print( void* tetra, Stream* stream ) {
	Snac_Element_Tetrahedra* self = (Snac_Element_Tetrahedra*)tetra;
	Tetrahedra_Surface_Index surface_I;

	Journal_Printf( stream, "Snac_Element_Tetrahedra:\n" );
	Journal_Printf( stream, "\tvolume: %g\n", self->volume );
	for( surface_I = 0; surface_I < Tetrahedra_Surface_Count; surface_I++ ) {
		Journal_Printf( stream, "\tsurface[%u]: ", surface_I );
		Snac_Element_Tetrahedra_Surface_Print( &self->surface[surface_I], stream );
	}
	Journal_Printf( stream, "\tstrainRate[3][3]: {%g, %g, %g}, {%g, %g, %g}, {%g, %g, %g}\n",
		self->strainRate[0][0], self->strainRate[0][1], self->strainRate[0][2],
		self->strainRate[1][0], self->strainRate[1][1], self->strainRate[1][2],
		self->strainRate[2][0], self->strainRate[2][1], self->strainRate[2][2] );
	Journal_Printf( stream, "\tstrain[3][3]: {%g, %g, %g}, {%g, %g, %g}, {%g, %g, %g}\n",
		self->strain[0][0], self->strain[0][1], self->strain[0][2],
		self->strain[1][0], self->strain[1][1], self->strain[1][2],
		self->strain[2][0], self->strain[2][1], self->strain[2][2] );
	Journal_Printf( stream, "\ttetraStress[3][3]: {%g, %g, %g}, {%g, %g, %g}, {%g, %g, %g}\n",
		self->stress[0][0], self->stress[0][1], self->stress[0][2],
		self->stress[1][0], self->stress[1][1], self->stress[1][2],
		self->stress[2][0], self->stress[2][1], self->stress[2][2] );
}


void Snac_Element_Print( void* element, Stream* stream ) {
	Snac_Element* self = (Snac_Element*)element;
	Tetrahedra_Index tetra_I;

	Journal_Printf( stream, "Snac_Element:\n" );
	Journal_Printf( stream, "\tmaterial_I: %u\n", self->material_I );
	Journal_Printf( stream, "\tvolume: %g\n", self->volume );
	Journal_Printf( stream, "\tstrainRate: %g\n", self->strainRate );
	Journal_Printf( stream, "\tstress: %g\n", self->stress );
	for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
		Journal_Printf( stream, "tetra[%u]: ", tetra_I );
		Snac_Element_Tetrahedra_Print( &self->tetra[tetra_I], stream );
	}
}

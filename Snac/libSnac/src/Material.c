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
** $Id: Material.c 1957 2004-08-26 18:55:56Z puru $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>

#include "units.h"
#include "types.h"
#include "shortcuts.h"
#include "Material.h"
#include <stdio.h>

void Snac_Material_Print( void* material, Stream* stream ) {
	Snac_Material*	self = (Snac_Material*)material;
	unsigned int	i;

	Journal_Printf( stream, "Snac_Material:\n" );
	Journal_Printf( stream, "\trheology: " );
	if( self->rheology & Snac_Material_Elastic ) Journal_Printf( stream, "Elastic " );
	if( self->rheology & Snac_Material_Plastic ) Journal_Printf( stream, "Plastic " );
	if( self->rheology & Snac_Material_Maxwell ) Journal_Printf( stream, "Maxwell " );
        if( self->rheology & Snac_Material_ViscoPlastic ) Journal_Printf( stream, "ViscoPlastic " );
	Journal_Printf( stream, "\n" );

	/* Elastic */
	Journal_Printf( stream, "\tlambda: %g\n", self->lambda );
	Journal_Printf( stream, "\tmu: %g\n", self->mu );

        /* Maxwell */
	Journal_Printf( stream, "\t viscosity: %g\n", self->viscosity );
	Journal_Printf( stream, "\t reference strain-rate: %g\n", self->refsrate );
	Journal_Printf( stream, "\t reference temperature: %g\n", self->reftemp );
	Journal_Printf( stream, "\t activation energy: %g\n", self->activationE );
	Journal_Printf( stream, "\t exponent of strain-rate: %g\n", self->srexponent );

	/* Plastic */
	Journal_Printf( stream, "\tnsegments: %u\n", self->nsegments );
	Journal_Printf( stream, "\tplstrain: " );
	for( i = 0; i < self->nsegments; i++ ) {
		Journal_Printf( stream, "%g, ", self->plstrain[i] );
	}
	Journal_Printf( stream, "\n" );
	Journal_Printf( stream, "\tfrictionAngle: " );
	for( i = 0; i < self->nsegments; i++ ) {
		Journal_Printf( stream, "%g, ", self->frictionAngle[i] );
	}
	Journal_Printf( stream, "\n" );
	Journal_Printf( stream, "\tdilationAngle: " );
	for( i = 0; i < self->nsegments; i++ ) {
		Journal_Printf( stream, "%g, ", self->dilationAngle[i] );
	}
	Journal_Printf( stream, "\n" );
	Journal_Printf( stream, "\tcohesion: " );
	for( i = 0; i < self->nsegments; i++ ) {
		Journal_Printf( stream, "%g, ", self->cohesion[i] );
	}
	Journal_Printf( stream, "\n" );
	Journal_Printf( stream, "\tten_off: %g\n", self->ten_off );
}

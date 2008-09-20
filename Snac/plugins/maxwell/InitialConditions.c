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
** $Id$
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "units.h"
#include "types.h"
#include "Element.h"
#include "Register.h"
#include "InitialConditions.h"
#include <assert.h>

void _SnacMaxwell_InitialConditions( void* _context, void* data ) {
	Snac_Context*				context = (Snac_Context*)_context;

	double                                  mu,vis_min,dt_maxwell;
	const double                            mfrac = 1.0e-01;
	#if DEBUG
		printf( "In: %s\n", __func__ );
	#endif

	/* HACK to say material 0 is a Maxwell material... TODO... needs to be in input!!!!, same for elastic and plastic! */
	context->materialProperty[0].rheology |= Snac_Material_Maxwell;

	/* compare the size of dt and replace it if necessary */
	mu = context->materialProperty[0].mu;
	vis_min = context->materialProperty[0].vis_min;
	dt_maxwell = mfrac*vis_min/mu;
	if( context->dt > dt_maxwell) {
		if(context->dtType == Snac_DtType_Constant) {
			fprintf(stderr,"dt(%e) should be smaller than dt_maxwell(%e) (mu=%e vis_min=%e mfrac=%e)\n",context->dt,dt_maxwell,mu,vis_min,mfrac);
			assert(0);
		}
		else
			context->dt = dt_maxwell;
	}
}

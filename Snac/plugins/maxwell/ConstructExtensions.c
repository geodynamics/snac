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
** $Id: ConstructExtensions.c 2498 2005-01-07 03:57:00Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "units.h"
#include "types.h"
#include "Context.h"
#include "Element.h"
#include "Register.h"
#include "ConstructExtensions.h"
#include <assert.h>
#include <limits.h>

#ifndef PATH_MAX
	#define PATH_MAX 1024
#endif

void _SnacVelocity_VariableCondition( Index index, Variable_Index var_I, void* _context, void* result ){
	Snac_Context*			context = (Snac_Context*)_context;

	double*				vy = (double*)result;
	if(context->timeStep > 0.02 * context->maxTimeSteps) 
            (*vy)=0.0;
        else 
            (*vy) = 0.005*erfc(context->currentTime*4.0);
		
//        if(context->timeStep > 0.02 * context->maxTimeSteps) (*vx)=0.0;
//        else (*vx) = 1.e-8;

}

void _SnacMaxwell_ConstructExtensions( void* _context, void* data ) {
	Snac_Context*				context = (Snac_Context*)_context;
	SnacMaxwell_Context*			contextExt = ExtensionManager_Get(
							context->extensionMgr,
							context,
							SnacMaxwell_ContextHandle );
	char					tmpBuf[PATH_MAX];
	#if DEBUG
		printf( "In: %s\n", __func__ );
	#endif

        ConditionFunction_Register_Add(
		context->condFunc_Register,
		ConditionFunction_New( _SnacVelocity_VariableCondition, "variablevelBC" ) );

	/* Prepare the dump and checkpoint file */
	sprintf( tmpBuf, "%s/viscosity.%u", context->outputPath, context->rank );
	if( (contextExt->viscosityOut = fopen( tmpBuf, "w+" )) == NULL ) {
		assert( contextExt->viscosityOut /* failed to open file for writing */ );
		abort();
	}
	sprintf( tmpBuf, "%s/viscosityCP.%u", context->outputPath, context->rank );
	if( (contextExt->viscosityCheckpoint = fopen( tmpBuf, "w+" )) == NULL ) {
		assert( contextExt->viscosityCheckpoint /* failed to open file for writing */ );
		abort();
	}
}

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
** $Id: ConstructExtensions.c 3234 2006-06-23 04:12:40Z LaetitiaLePourhiet $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "Context.h"
#include "Register.h"
#include "Element.h"
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
        
}


void _SnacMaxwellMohrCoulomb_ConstructExtensions( void* _context, void* data ) {
	Snac_Context*				context = (Snac_Context*)_context;
	SnacMaxwellMohrCoulomb_Context*			contextExt = ExtensionManager_Get(
							context->extensionMgr,
							context,
							SnacMaxwellMohrCoulomb_ContextHandle );
        Snac_Element                            tmpElement;
        SnacMaxwellMohrCoulomb_Element*               tmpElementExt = ExtensionManager_Get(
                                                        context->mesh->elementExtensionMgr,
                                                        &tmpElement,
                                                        SnacMaxwellMohrCoulomb_ElementHandle );
	char					tmpBuf[PATH_MAX];
        /* Because plastic strain is not an array by itself, we must the "complex" constructor for Variable... the info needs to be
         * wrapped this generic way... */
        Index                                   viscoplasticOffsetCount = 1;
        SizeT                                   viscoplasticOffsets[] = { 
                   (SizeT)((char*)&tmpElementExt->aps - (char*)&tmpElement) };
        Variable_DataType                       viscoplasticDataTypes[] = { Variable_DataType_Double };
        Index                                   viscoplasticDataTypeCounts[] = { 1 };

      //	#if DEBUG
         if( context->rank == 0 )		printf( "In %s()\n", __func__ );
      //	#endif
        /* Create the StGermain variable aps, which is stored on an element extension */
        Variable_New(
                "plStrain",
                viscoplasticOffsetCount,
                viscoplasticOffsets,
                viscoplasticDataTypes,
                viscoplasticDataTypeCounts,
                0,
                &ExtensionManager_GetFinalSize( context->mesh->elementExtensionMgr ),
                &context->mesh->layout->decomp->elementDomainCount,
                (void**)&context->mesh->element,
                context->variable_Register );
        int element_lI;        
        for( element_lI = 0; element_lI < context->mesh->elementLocalCount; element_lI++ ) {
                        Snac_Element*           element = Snac_Element_At( context, element_lI );
                        SnacMaxwellMohrCoulomb_Element*       viscoplasticElement = ExtensionManager_Get( context->mesh->elementExtensionMgr, element,
                                                                                                                SnacMaxwellMohrCoulomb_ElementHandle );
                        viscoplasticElement->aps = 0.0f;
                }//for


        ConditionFunction_Register_Add(
		context->condFunc_Register,
		ConditionFunction_New( _SnacVelocity_VariableCondition, "variablevelBC" ) );

	/* Prepare the dump file */
	sprintf( tmpBuf, "%s/plStrain.%u", context->outputPath, context->rank );
	if( (contextExt->plStrainOut = fopen( tmpBuf, "w+" )) == NULL ) {
		assert( contextExt->plStrainOut /* failed to open file for writing */ );
	}
	/* for restarting, store each Tet's plastic strain */
	sprintf( tmpBuf, "%s/plStrainTensor.%u", context->outputPath, context->rank );
	if( (contextExt->plstrainTensorOut = fopen( tmpBuf, "w+" )) == NULL ) {
		assert( contextExt->plstrainTensorOut /* failed to open file for writing */ );
	}
	sprintf( tmpBuf, "%s/viscosity.%u", context->outputPath, context->rank );
	if( (contextExt->viscOut = fopen( tmpBuf, "w+" )) == NULL ) {
		assert( contextExt->viscOut /* failed to open file for writing */ );
	}
}

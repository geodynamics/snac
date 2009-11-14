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
** $Id: InitialConditions.c 2498 2005-01-07 03:57:00Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "Element.h"
#include "InitialConditions.h"
#include "Register.h"
#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

void SnacPlastic_InitialConditions( void* _context ) {
	Snac_Context*			context = (Snac_Context*)_context;
	Element_LocalIndex		element_lI;
	Dictionary_Entry_Value* materialList = Dictionary_Get( context->dictionary, "materials" );
	int						PhaseI = 0;
#ifdef DEBUG
	printf( "In: _SnacPlastic_InitialConditions( void* )\n" );
#endif

	if( materialList ) {
		Dictionary_Entry_Value* materialEntry = Dictionary_Entry_Value_GetFirstElement( materialList );
		/* loop around the  phases to initialize rheology */
		while( materialEntry ) {
			context->materialProperty[PhaseI].rheology |= Snac_Material_Plastic;
			PhaseI++;
			materialEntry                               = materialEntry->next;
		}
	}
	else
		context->materialProperty[PhaseI].rheology |= Snac_Material_Plastic;

	if( context->restartTimestep > 0 ) {
		FILE*				plStrainIn;
		char				path[PATH_MAX];
		
		sprintf(path, "%s/snac.plStrain.%d.%06d.restart",context->outputPath,context->rank,context->restartTimestep);
		Journal_Firewall( (plStrainIn = fopen(path,"r")) != NULL, "Can't find %s", path );
		
		/* read in restart file to reconstruct the previous plastic strain.*/
		for( element_lI = 0; element_lI < context->mesh->elementLocalCount; element_lI++ ) {
			Snac_Element*				element = Snac_Element_At( context, element_lI );
			SnacPlastic_Element*		plasticElement = ExtensionManager_Get( context->mesh->elementExtensionMgr, element,
															SnacPlastic_ElementHandle );
			const Snac_Material* 		material = &context->materialProperty[element->material_I];

			if( material->yieldcriterion == mohrcoulomb ) {
				Tetrahedra_Index	tetra_I;
				double              depls = 0.0f;
				double              totalVolume = 0.0f;
				
				for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
					double			tetraPlStrain;

					fscanf( plStrainIn, "%le", &tetraPlStrain );
					plasticElement->plasticStrain[tetra_I] = tetraPlStrain;
					depls += plasticElement->plasticStrain[tetra_I]*element->tetra[tetra_I].volume;
					totalVolume += element->tetra[tetra_I].volume;
				}
				/* volume-averaged accumulated plastic strain, aps */
				plasticElement->aps = depls/totalVolume;
			}//if(mohrcoulomb)
		}//for elements
		if( plStrainIn )
			fclose( plStrainIn );
	}//if restarting
	else {
		/* Set the plastic element initial conditions */
		for( element_lI = 0; element_lI < context->mesh->elementLocalCount; element_lI++ ) {
			Snac_Element*		element = Snac_Element_At( context, element_lI );
			SnacPlastic_Element*	plasticElement = ExtensionManager_Get( context->mesh->elementExtensionMgr, element, 
																		   SnacPlastic_ElementHandle );
			Tetrahedra_Index	tetra_I;
			
			for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
				plasticElement->plasticStrain[tetra_I] = 0.0f;
			}
			plasticElement->aps = 0.0f;
		}
	}
}
		

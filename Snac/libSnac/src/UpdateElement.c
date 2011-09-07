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
** $Id: UpdateElement.c 3274 2007-03-27 20:25:29Z EunseoChoi $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>

#include "units.h"
#include "types.h"
#include "shortcuts.h"
#include "Material.h"
#include "Node.h"
#include "Tetrahedra.h"
#include "TetrahedraTables.h"
#include "Element.h"
#include "UpdateElement.h"
#include "Context.h"
#include <math.h>
#include <string.h>

/* Tetrahedra surface default minimum area */
#define Tetrahedra_Max_Propagation_Length 1.0e+27f



void Snac_UpdateElementMomentum( void* context, Element_LocalIndex element_lI, double* elementMinLengthScale ) {
	Snac_Context*		self = (Snac_Context*)context;
	Tetrahedra_Index	tetra_I;
	Snac_Element*		element = Snac_Element_At( self, element_lI );
	Rotation		rotation[Tetrahedra_Count];

	Snac_Material*      material = &self->materialProperty[element->material_I];

	/* Find this processor's maximum Vp and store it as speedOfSound. */
	if( self->dtType == Snac_DtType_Wave ) {
		double Vp = sqrt((material->lambda+2.0f*material->mu)/material->phsDensity);
		if( Vp > self->speedOfSound )
			self->speedOfSound = Vp;
	}

	/* Initialise output data */
	element->volume=0.0;
	*elementMinLengthScale = Tetrahedra_Max_Propagation_Length;
	memset( rotation, 0, sizeof(rotation) );


	/* Calculate the volume, surface { area, normal and velocity }, and smallest length scale for each tetrahedra. */
	for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
		Tetrahedra_Surface_Index	surface_I;
		double				lengthScale;	

		/*ccccc*/
		memset( element->tetra[tetra_I].strain, 0, sizeof(element->tetra[tetra_I].strain) );

		/* Calculate this tetrahedra's volume */
		element->tetra[tetra_I].old_volume = element->tetra[tetra_I].volume;
		element->tetra[tetra_I].volume = Tetrahedra_Volume(
		Snac_Element_NodeCoord( self, element_lI, TetraToNode[tetra_I][0] ),
		Snac_Element_NodeCoord( self, element_lI, TetraToNode[tetra_I][1] ),
		Snac_Element_NodeCoord( self, element_lI, TetraToNode[tetra_I][2] ),
		Snac_Element_NodeCoord( self, element_lI, TetraToNode[tetra_I][3] ) );
		if(self->timeStep == 0)
			element->tetra[tetra_I].old_volume = element->tetra[tetra_I].volume;

		/* Calculate this tetrahedra's area, normal and velocity for each surface. */
		for( surface_I = 0; surface_I < Tetrahedra_Surface_Count; surface_I++ ) {
			element->tetra[tetra_I].surface[surface_I].area = Tetrahedra_SurfaceArea(
				Snac_Element_NodeCoord( self, element_lI, TetraToNode[tetra_I][FaceToNode[surface_I][0]] ),
				Snac_Element_NodeCoord( self, element_lI, TetraToNode[tetra_I][FaceToNode[surface_I][1]] ),
				Snac_Element_NodeCoord( self, element_lI, TetraToNode[tetra_I][FaceToNode[surface_I][2]] ) );

			Tetrahedra_SurfaceNormal(
				Snac_Element_NodeCoord( self, element_lI, TetraToNode[tetra_I][FaceToNode[surface_I][0]] ),
				Snac_Element_NodeCoord( self, element_lI, TetraToNode[tetra_I][FaceToNode[surface_I][1]] ),
				Snac_Element_NodeCoord( self, element_lI, TetraToNode[tetra_I][FaceToNode[surface_I][2]] ),
				&element->tetra[tetra_I].surface[surface_I].normal );
		}


		/* keep track of this processor's smallest length scale */
		lengthScale = element->tetra[tetra_I].surface[0].area;
		if( lengthScale ) {
			lengthScale = fabs( element->tetra[tetra_I].volume / lengthScale );
			if( *elementMinLengthScale > lengthScale ) {
				*elementMinLengthScale = lengthScale;
			}
		}

		lengthScale = element->tetra[tetra_I].surface[1].area;
		if( lengthScale ) {
			lengthScale = fabs( element->tetra[tetra_I].volume / lengthScale );
			if( *elementMinLengthScale > lengthScale ) {
				*elementMinLengthScale = lengthScale;
			}
		}

		lengthScale = element->tetra[tetra_I].surface[2].area;
		if( lengthScale ) {
			lengthScale = fabs( element->tetra[tetra_I].volume / lengthScale );
			if( *elementMinLengthScale > lengthScale ) {
				*elementMinLengthScale = lengthScale;
			}
		}

		lengthScale = element->tetra[tetra_I].surface[3].area;
		if( lengthScale ) {
			lengthScale = fabs( element->tetra[tetra_I].volume / lengthScale );
			if( *elementMinLengthScale > lengthScale ) {
				*elementMinLengthScale = lengthScale;
			}
		}
		/* Ammend the element volume with this tetrahedra's contribution */
		element->volume += element->tetra[tetra_I].volume;


		/* Calculate the rotation of this tetrahedra */
		rotation[tetra_I][0][1] = -( 0.5f / element->tetra[tetra_I].volume ) * (
			Snac_Element_Node_P(self, element_lI, TetraToNode[tetra_I][0])->velocity[0] *
				element->tetra[tetra_I].surface[1].normal[1] *
				element->tetra[tetra_I].surface[1].area +
			Snac_Element_Node_P(self, element_lI, TetraToNode[tetra_I][1])->velocity[0] *
				element->tetra[tetra_I].surface[2].normal[1] *
				element->tetra[tetra_I].surface[2].area +
			Snac_Element_Node_P(self, element_lI, TetraToNode[tetra_I][2])->velocity[0] *
				element->tetra[tetra_I].surface[3].normal[1] *
				element->tetra[tetra_I].surface[3].area +
			Snac_Element_Node_P(self, element_lI, TetraToNode[tetra_I][3])->velocity[0] *
				element->tetra[tetra_I].surface[0].normal[1] *
				element->tetra[tetra_I].surface[0].area -
			Snac_Element_Node_P(self, element_lI, TetraToNode[tetra_I][0])->velocity[1] *
				element->tetra[tetra_I].surface[1].normal[0] *
				element->tetra[tetra_I].surface[1].area -
			Snac_Element_Node_P(self, element_lI, TetraToNode[tetra_I][1])->velocity[1] *
				element->tetra[tetra_I].surface[2].normal[0] *
				element->tetra[tetra_I].surface[2].area -
			Snac_Element_Node_P(self, element_lI, TetraToNode[tetra_I][2])->velocity[1] *
				element->tetra[tetra_I].surface[3].normal[0] *
				element->tetra[tetra_I].surface[3].area -
			Snac_Element_Node_P(self, element_lI, TetraToNode[tetra_I][3])->velocity[1] *
				element->tetra[tetra_I].surface[0].normal[0] *
				element->tetra[tetra_I].surface[0].area )/3.0f;
		rotation[tetra_I][0][2] = -( 0.5f / element->tetra[tetra_I].volume ) * (
			Snac_Element_Node_P(self, element_lI, TetraToNode[tetra_I][0])->velocity[0] *
				element->tetra[tetra_I].surface[1].normal[2] *
				element->tetra[tetra_I].surface[1].area +
			Snac_Element_Node_P(self, element_lI, TetraToNode[tetra_I][1])->velocity[0] *
				element->tetra[tetra_I].surface[2].normal[2] *
				element->tetra[tetra_I].surface[2].area +
			Snac_Element_Node_P(self, element_lI, TetraToNode[tetra_I][2])->velocity[0] *
				element->tetra[tetra_I].surface[3].normal[2] *
				element->tetra[tetra_I].surface[3].area +
			Snac_Element_Node_P(self, element_lI, TetraToNode[tetra_I][3])->velocity[0] *
				element->tetra[tetra_I].surface[0].normal[2] *
				element->tetra[tetra_I].surface[0].area -
			Snac_Element_Node_P(self, element_lI, TetraToNode[tetra_I][0])->velocity[2] *
				element->tetra[tetra_I].surface[1].normal[0] *
				element->tetra[tetra_I].surface[1].area -
			Snac_Element_Node_P(self, element_lI, TetraToNode[tetra_I][1])->velocity[2] *
				element->tetra[tetra_I].surface[2].normal[0] *
				element->tetra[tetra_I].surface[2].area -
			Snac_Element_Node_P(self, element_lI, TetraToNode[tetra_I][2])->velocity[2] *
				element->tetra[tetra_I].surface[3].normal[0] *
				element->tetra[tetra_I].surface[3].area -
			Snac_Element_Node_P(self, element_lI, TetraToNode[tetra_I][3])->velocity[2] *
				element->tetra[tetra_I].surface[0].normal[0] *
				element->tetra[tetra_I].surface[0].area )/3.0f;
		rotation[tetra_I][1][2] = -( 0.5 / element->tetra[tetra_I].volume ) * (
			Snac_Element_Node_P(self, element_lI, TetraToNode[tetra_I][0])->velocity[1] *
				element->tetra[tetra_I].surface[1].normal[2] *
				element->tetra[tetra_I].surface[1].area +
			Snac_Element_Node_P(self, element_lI, TetraToNode[tetra_I][1])->velocity[1] *
				element->tetra[tetra_I].surface[2].normal[2] *
				element->tetra[tetra_I].surface[2].area +
			Snac_Element_Node_P(self, element_lI, TetraToNode[tetra_I][2])->velocity[1] *
				element->tetra[tetra_I].surface[3].normal[2] *
				element->tetra[tetra_I].surface[3].area +
			Snac_Element_Node_P(self, element_lI, TetraToNode[tetra_I][3])->velocity[1] *
				element->tetra[tetra_I].surface[0].normal[2] *
				element->tetra[tetra_I].surface[0].area -
			Snac_Element_Node_P(self, element_lI, TetraToNode[tetra_I][0])->velocity[2] *
				element->tetra[tetra_I].surface[1].normal[1] *
				element->tetra[tetra_I].surface[1].area -
			Snac_Element_Node_P(self, element_lI, TetraToNode[tetra_I][1])->velocity[2] *
				element->tetra[tetra_I].surface[2].normal[1] *
				element->tetra[tetra_I].surface[2].area -
			Snac_Element_Node_P(self, element_lI, TetraToNode[tetra_I][2])->velocity[2] *
				element->tetra[tetra_I].surface[3].normal[1] *
				element->tetra[tetra_I].surface[3].area -
			Snac_Element_Node_P(self, element_lI, TetraToNode[tetra_I][3])->velocity[2] *
				element->tetra[tetra_I].surface[0].normal[1] *
				element->tetra[tetra_I].surface[0].area )/3.0f;

		/* Rotate the strain of this tetrahedra. */
		element->tetra[tetra_I].strain[0][0] += (
			2.0f * rotation[tetra_I][0][1] * element->tetra[tetra_I].strain[0][1] +
			2.0f * rotation[tetra_I][0][2] * element->tetra[tetra_I].strain[0][2] ) * self->dt;
		element->tetra[tetra_I].strain[1][1] += (
			2.0f * (-1.0f*rotation[tetra_I][0][1]) * element->tetra[tetra_I].strain[0][1] +
			2.0f * rotation[tetra_I][1][2] * element->tetra[tetra_I].strain[1][2] ) * self->dt;
		element->tetra[tetra_I].strain[2][2] += (
			2.0f * (-1.0f*rotation[tetra_I][0][2]) * element->tetra[tetra_I].strain[0][2] +
			2.0f * (-1.0f*rotation[tetra_I][1][2]) * element->tetra[tetra_I].strain[1][2] ) * self->dt;
		element->tetra[tetra_I].strain[0][1] += ( rotation[tetra_I][0][1] *
			(element->tetra[tetra_I].strain[1][1] - element->tetra[tetra_I].strain[0][0] ) ) * self->dt +
			( rotation[tetra_I][0][2] * element->tetra[tetra_I].strain[1][2] -
			  rotation[tetra_I][1][2] * element->tetra[tetra_I].strain[0][2] ) * self->dt;
		element->tetra[tetra_I].strain[0][2] += ( rotation[tetra_I][0][2] *
			(element->tetra[tetra_I].strain[2][2] - element->tetra[tetra_I].strain[0][0] ) ) * self->dt +
			( rotation[tetra_I][0][1] * element->tetra[tetra_I].strain[1][2] -
			  rotation[tetra_I][0][2] * element->tetra[tetra_I].strain[0][1] ) * self->dt;
		element->tetra[tetra_I].strain[1][2] += ( rotation[tetra_I][1][2] *
			(element->tetra[tetra_I].strain[2][2] - element->tetra[tetra_I].strain[1][1] ) ) * self->dt +
			( (-1.0f*rotation[tetra_I][0][1]) * element->tetra[tetra_I].strain[0][2] -
			  rotation[tetra_I][0][2] * element->tetra[tetra_I].strain[0][1] ) * self->dt;	

		/* Rotate the stress of this tetrahedra. */
		element->tetra[tetra_I].stress[0][0] += (
			2.0f * rotation[tetra_I][0][1] * element->tetra[tetra_I].stress[0][1] +
			2.0f * rotation[tetra_I][0][2] * element->tetra[tetra_I].stress[0][2] ) * self->dt;
		element->tetra[tetra_I].stress[1][1] += (
			2.0f * (-1.0f*rotation[tetra_I][0][1]) * element->tetra[tetra_I].stress[0][1] +
			2.0f * rotation[tetra_I][1][2] * element->tetra[tetra_I].stress[1][2] ) * self->dt;
		element->tetra[tetra_I].stress[2][2] += (
			2.0f * (-1.0f*rotation[tetra_I][0][2]) * element->tetra[tetra_I].stress[0][2] +
			2.0f * (-1.0f*rotation[tetra_I][1][2]) * element->tetra[tetra_I].stress[1][2] ) * self->dt;
		element->tetra[tetra_I].stress[0][1] += ( rotation[tetra_I][0][1] *
			(element->tetra[tetra_I].stress[1][1] - element->tetra[tetra_I].stress[0][0] ) ) * self->dt +
			( rotation[tetra_I][0][2] * element->tetra[tetra_I].stress[1][2] +
			  rotation[tetra_I][1][2] * element->tetra[tetra_I].stress[0][2] ) * self->dt;
		element->tetra[tetra_I].stress[0][2] += ( rotation[tetra_I][0][2] *
			(element->tetra[tetra_I].stress[2][2] - element->tetra[tetra_I].stress[0][0] ) ) * self->dt +
			( rotation[tetra_I][0][1] * element->tetra[tetra_I].stress[1][2] -
			  rotation[tetra_I][1][2] * element->tetra[tetra_I].stress[0][1] ) * self->dt;
		element->tetra[tetra_I].stress[1][2] += ( rotation[tetra_I][1][2] *
			(element->tetra[tetra_I].stress[2][2] - element->tetra[tetra_I].stress[1][1] ) ) * self->dt +
			( (-1.0f*rotation[tetra_I][0][1]) * element->tetra[tetra_I].stress[0][2] -
			  rotation[tetra_I][0][2] * element->tetra[tetra_I].stress[0][1] ) * self->dt;
	}
	if( Tetrahedra_Count > 5 )
		element->volume *= 0.5;
}

void Snac_UpdateElementMomentum_Restart( void* context, Element_LocalIndex element_lI, double* elementMinLengthScale ) {
	Snac_Context*		self = (Snac_Context*)context;
	Tetrahedra_Index	tetra_I;
	Snac_Element*		element = Snac_Element_At( self, element_lI );

	Snac_Material*      material = &self->materialProperty[element->material_I];


	/* Find this processor's maximum Vp and store it as speedOfSound. */
	if( self->dtType == Snac_DtType_Wave ) {
		double Vp = sqrt((material->lambda+2.0f*material->mu)/material->phsDensity);
		if( Vp > self->speedOfSound )
			self->speedOfSound = Vp;
	}

	/* Initialise output data */
	element->volume=0.0;
	*elementMinLengthScale = Tetrahedra_Max_Propagation_Length;

	/* Calculate the volume, surface { area, normal and velocity }, and smallest length scale for each tetrahedra. */
	for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
		Tetrahedra_Surface_Index	surface_I;
		double				lengthScale;

		/*ccccc*/
		memset( element->tetra[tetra_I].strain, 0, sizeof(element->tetra[tetra_I].strain) );

		/* Calculate this tetrahedra's volume */
		element->tetra[tetra_I].old_volume = element->tetra[tetra_I].volume;
		element->tetra[tetra_I].volume = Tetrahedra_Volume(
			Snac_Element_NodeCoord( self, element_lI, TetraToNode[tetra_I][0] ),
			Snac_Element_NodeCoord( self, element_lI, TetraToNode[tetra_I][1] ),
			Snac_Element_NodeCoord( self, element_lI, TetraToNode[tetra_I][2] ),
			Snac_Element_NodeCoord( self, element_lI, TetraToNode[tetra_I][3] ) );
		if(self->timeStep == 0)
			element->tetra[tetra_I].old_volume = element->tetra[tetra_I].volume;

		/* Calculate this tetrahedra's area, normal and velocity for each surface. */
		for( surface_I = 0; surface_I < Tetrahedra_Surface_Count; surface_I++ ) {
			element->tetra[tetra_I].surface[surface_I].area = Tetrahedra_SurfaceArea(
				Snac_Element_NodeCoord( self, element_lI, TetraToNode[tetra_I][FaceToNode[surface_I][0]] ),
				Snac_Element_NodeCoord( self, element_lI, TetraToNode[tetra_I][FaceToNode[surface_I][1]] ),
				Snac_Element_NodeCoord( self, element_lI, TetraToNode[tetra_I][FaceToNode[surface_I][2]] ) );

			Tetrahedra_SurfaceNormal(
				Snac_Element_NodeCoord( self, element_lI, TetraToNode[tetra_I][FaceToNode[surface_I][0]] ),
				Snac_Element_NodeCoord( self, element_lI, TetraToNode[tetra_I][FaceToNode[surface_I][1]] ),
				Snac_Element_NodeCoord( self, element_lI, TetraToNode[tetra_I][FaceToNode[surface_I][2]] ),
				&element->tetra[tetra_I].surface[surface_I].normal );
		}


		/* keep track of this processor's smallest length scale */
		lengthScale = element->tetra[tetra_I].surface[0].area;
		if( lengthScale ) {
			lengthScale = fabs( element->tetra[tetra_I].volume / lengthScale );
			if( *elementMinLengthScale > lengthScale ) {
				*elementMinLengthScale = lengthScale;
			}
		}

		lengthScale = element->tetra[tetra_I].surface[1].area;
		if( lengthScale ) {
		lengthScale = fabs( element->tetra[tetra_I].volume / lengthScale );
			if( *elementMinLengthScale > lengthScale ) {
				*elementMinLengthScale = lengthScale;
			}
		}

		lengthScale = element->tetra[tetra_I].surface[2].area;
		if( lengthScale ) {
			lengthScale = fabs( element->tetra[tetra_I].volume / lengthScale );
			if( *elementMinLengthScale > lengthScale ) {
				*elementMinLengthScale = lengthScale;
			}
		}

		lengthScale = element->tetra[tetra_I].surface[3].area;
		if( lengthScale ) {
			lengthScale = fabs( element->tetra[tetra_I].volume / lengthScale );
			if( *elementMinLengthScale > lengthScale ) {
				*elementMinLengthScale = lengthScale;
			}
		}
		/* Ammend the element volume with this tetrahedra's contribution */
		element->volume += element->tetra[tetra_I].volume;
	}
	if( Tetrahedra_Count > 5 )
		element->volume *= 0.5;
}

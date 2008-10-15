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
** $Id: VariableConditions.c 3166 2005-10-17 20:46:42Z EunseoChoi $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "Register.h"
#include "VariableConditions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void _SnacHydroStaticIC_IC( void* _context ) {
	Snac_Context*		context = (Snac_Context*)_context;
	Mesh*           mesh = context->mesh;
	MeshLayout*		layout = (MeshLayout*)mesh->layout;
	HexaMD*			decomp = (HexaMD*)layout->decomp;
	BlockGeometry*		geometry = (BlockGeometry*)layout->elementLayout->geometry;

	int       procBelow = -1, procAbove = -1;
	int       procJ, rankPartition[3];
	MPI_Status  status;

	if( context->rank == 0 ) Journal_Printf( context->snacInfo, "In: %s\n", __func__ );

	/* Decompose rank into partitions in each axis */
	rankPartition[2] = context->rank/(decomp->partition3DCounts[0]*decomp->partition3DCounts[1]);
	rankPartition[1] = ( context->rank - rankPartition[2]*decomp->partition3DCounts[0]*decomp->partition3DCounts[1] ) / decomp->partition3DCounts[0];
	rankPartition[0] = context->rank - rankPartition[2]*decomp->partition3DCounts[0]*decomp->partition3DCounts[1] - rankPartition[1] * decomp->partition3DCounts[0];

	/* Loop over procs along y-axis */
	for( procJ = decomp->partition3DCounts[1]-1; procJ > -1; procJ-- ) {
		const int  elx = decomp->elementLocal3DCounts[context->rank][0];
		const int  ely = decomp->elementLocal3DCounts[context->rank][1];
		const int  elz = decomp->elementLocal3DCounts[context->rank][2];
		double      PfromAbove[elx][elz];

		/* Compute density integration over depth for each layer of processors */
		if( rankPartition[1] == procJ ) {
			int  elI, elJ, elK;
			double           bottomP[elx][elz];

			/* Initialize PfromAbove */
			memset( PfromAbove, 0, sizeof(PfromAbove) );

			/* Receive PfromAbove from proc right above me if r-axis is partitioned and this proc is not on the top. */
			if( decomp->partitionedAxis[1] && ( rankPartition[1] < decomp->partition3DCounts[1]-1 ) ) {
				procAbove = rankPartition[0] + (rankPartition[1]+1)*decomp->partition3DCounts[0] +
					rankPartition[2]*decomp->partition3DCounts[0]*decomp->partition3DCounts[1];
				MPI_Recv( PfromAbove, elx*elz, MPI_DOUBLE, procAbove, 0, context->communicator, &status );
			}

			/* Accumulate dPs of elements from above */
			for( elK = 0; elK < elz; elK++ )
				for( elI = 0; elI < elx; elI++ ) {
					double rogh = 0.0f;
					for( elJ = ely - 1; elJ > -1; elJ--) {
						Element_LocalIndex  element_lI = elI + elJ*elx + elK*elx*ely;
						Snac_Element*		element = Snac_Element_At( context, element_lI );
						Tetrahedra_Index	tetra_I;
						Material_Index          material_I = element->material_I;
						Snac_Material*          material = &context->materialProperty[material_I];
						Density                 phsDensity = material->phsDensity; /* node->density */
						double          alpha = material->alpha;
						double          beta = material->beta;
						Density         densT = 0.0f;
						double			dhE[4], dh=0.0;
						double			dPT=0.0, dP=0.0, P=0.0;

						/* Initialize tetrahedral arrays and get the average T-dependent density */
						for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
							densT += phsDensity * ( 1.0 - alpha * (element->tetra[tetra_I].avgTemp-material->reftemp) ) / Tetrahedra_Count;
/* 							memset( element->tetra[tetra_I].stress, 0, sizeof(element->tetra[tetra_I].stress) ); */
							memset( element->tetra[tetra_I].strainRate, 0, sizeof(element->tetra[tetra_I].strainRate));
						}
						/* Compute the average height of the element */
						dhE[0] = Snac_Element_NodeCoord( context, element_lI, 6 )[1] - Snac_Element_NodeCoord( context, element_lI, 0 )[1];
						dhE[1] = Snac_Element_NodeCoord( context, element_lI, 2 )[1] - Snac_Element_NodeCoord( context, element_lI, 4 )[1];
						dhE[2] = Snac_Element_NodeCoord( context, element_lI, 3 )[1] - Snac_Element_NodeCoord( context, element_lI, 5 )[1];
						dhE[3] = Snac_Element_NodeCoord( context, element_lI, 7 )[1] - Snac_Element_NodeCoord( context, element_lI, 1 )[1];
						dh = 0.25f * ( dhE[0] + dhE[1] + dhE[2] + dhE[3] );

						/* Some more initialization */
						element->stress=0.0;
						element->strainRate=0.0;
						element->hydroPressure = 0.0f;
						element->rzbo = geometry->min[1];

						dPT = densT * context->gravity * dh;
						dP = dPT * ( 1.0 - beta*rogh ) / ( 1.0f + beta / 2.0f * dPT );
						P = rogh + 0.5f * dP;
						P =  -1.0f * ( P + PfromAbove[elI][elK] );
						for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
							element->tetra[tetra_I].stress[0][0] = P;
							element->tetra[tetra_I].stress[1][1] = P;
							element->tetra[tetra_I].stress[2][2] = P;
						}

						element->hydroPressure = P;
						rogh += dP;
						element->bottomPressure = rogh;
						if( elJ == 0 ) {
							bottomP[elI][elK] = element->bottomPressure + PfromAbove[elI][elK];
							if( elI==0 && elK==0 ) {
								context->pisos = bottomP[elI][elK];
							}
						}
					}
				}
			/* Send bottomP to the proc below if the r-axis is partitioned and this proc is not on the bottom. */
			if( decomp->partitionedAxis[1] && ( rankPartition[1] > 0 ) ) {
				procBelow = rankPartition[0] + (rankPartition[1]-1)*decomp->partition3DCounts[0] +
					rankPartition[2]*decomp->partition3DCounts[0]*decomp->partition3DCounts[1];
				MPI_Send( bottomP, elx*elz, MPI_DOUBLE, procBelow, 0, context->communicator );
			}
		}
		MPI_Barrier( context->communicator );
	} /* End of loop for y procs */
}


void _SnacHydroStaticIC_IC_Spherical( void* _context ) {
	Snac_Context*		context = (Snac_Context*)_context;
	Mesh*           mesh = context->mesh;
	MeshLayout*		layout = (MeshLayout*)mesh->layout;
	HexaMD*			decomp = (HexaMD*)layout->decomp;
	BlockGeometry*		geometry = (BlockGeometry*)layout->elementLayout->geometry;

	int       procBelow=-1, procAbove=-1;
	int       procJ, rankPartition[3];
	MPI_Status  status;

	if( context->rank == 0 ) Journal_Printf( context->snacInfo, "In: %s\n", __func__ );

	/* Decompose rank into partitions in each axis */
	rankPartition[2] = context->rank/(decomp->partition3DCounts[0]*decomp->partition3DCounts[1]);
	rankPartition[1] = ( context->rank - rankPartition[2]*decomp->partition3DCounts[0]*decomp->partition3DCounts[1] ) / decomp->partition3DCounts[0];
	rankPartition[0] = context->rank - rankPartition[2]*decomp->partition3DCounts[0]*decomp->partition3DCounts[1] - rankPartition[1] * decomp->partition3DCounts[0];

	/* Loop over procs along y-axis */
 	for( procJ = decomp->partition3DCounts[1]-1; procJ > -1; procJ-- ) {
		const int  elx = decomp->elementLocal3DCounts[context->rank][0];
		const int  ely = decomp->elementLocal3DCounts[context->rank][1];
		const int  elz = decomp->elementLocal3DCounts[context->rank][2];
		double      PfromAbove[elx][elz];

		/* Compute density integration over depth for each layer of processors */
		if( rankPartition[1] == procJ ) {
			int  elI, elJ, elK;
			double           bottomP[elx][elz];

			/* Initialize PfromAbove */
			memset( PfromAbove, 0, sizeof(PfromAbove) );

			/* Receive PfromAbove from proc right above me if r-axis is partitioned and this proc is not on the top. */
			if( decomp->partitionedAxis[1] && ( rankPartition[1] < decomp->partition3DCounts[1]-1 ) ) {
				procAbove = rankPartition[0] + (rankPartition[1]+1)*decomp->partition3DCounts[0] +
					rankPartition[2]*decomp->partition3DCounts[0]*decomp->partition3DCounts[1];
				MPI_Recv( PfromAbove, elx*elz, MPI_DOUBLE, procAbove, 0, context->communicator, &status );
			}
			/* Accumulate dPs of elements form above */
			for( elK = 0; elK < elz; elK++ )
				for( elI = 0; elI < elx; elI++ ) {
					double rogh = 0.0f;
					for( elJ = ely - 1; elJ > -1; elJ--) {
						Element_LocalIndex  element_lI = elI + elJ*elx + elK*elx*ely;
						Snac_Element*		element = Snac_Element_At( context, element_lI );
						Tetrahedra_Index	tetra_I;
						Material_Index          material_I = element->material_I;
						Snac_Material*          material = &context->materialProperty[material_I];
						Density                 phsDensity = material->phsDensity; /* node->density */
						double          alpha = material->alpha;
						double          beta = material->beta;
						Density         densT = 0.0f;
						double			r[8], dhE[4], dh=0.0;
						double			dPT=0.0, dP=0.0, P=0.0;
						Index           nE_I;

						/* Initialize tetrahedral arrays and get the average T-dependent density */
						for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
							densT += phsDensity * (1.0 - alpha * element->tetra[tetra_I].avgTemp) / Tetrahedra_Count;
/* 							memset( element->tetraStress[tetra_I], 0, sizeof(Snac_TetraStressTensor) ); */
							memset( element->tetra[tetra_I].strainRate, 0, sizeof(element->tetra[tetra_I].strainRate));
						}
						/* Compute the average height of the element */
						for( nE_I = 0; nE_I < 8; nE_I++ )
							r[nE_I] = sqrt( Snac_Element_NodeCoord( context, element_lI, nE_I )[0]*Snac_Element_NodeCoord( context, element_lI, nE_I )[0] +
											Snac_Element_NodeCoord( context, element_lI, nE_I )[1]*Snac_Element_NodeCoord( context, element_lI, nE_I )[1] +
											Snac_Element_NodeCoord( context, element_lI, nE_I )[2]*Snac_Element_NodeCoord( context, element_lI, nE_I )[2] );
						dhE[0] = r[6] - r[0];
						dhE[1] = r[2] - r[4];
						dhE[2] = r[3] - r[5];
						dhE[3] = r[7] - r[1];
						dh = 0.25f * ( dhE[0] + dhE[1] + dhE[2] + dhE[3] );

						dPT = densT * context->gravity * dh;
						dP = dPT * ( 1.0 - beta*rogh ) / ( 1.0f + beta / 2.0f * dPT );
						P = rogh + 0.5f * dP;
						P = -1.0f * ( P + PfromAbove[elI][elK] );
						/* Some more initialization */
						element->stress=0.0;
						element->strainRate=0.0;
						element->hydroPressure = 0.0f;
						element->rzbo = geometry->min[1];
						for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
							element->tetra[tetra_I].stress[0][0] = P;
							element->tetra[tetra_I].stress[1][1] = P;
							element->tetra[tetra_I].stress[2][2] = P;
						}
						element->hydroPressure = P;
						rogh += dP;
						element->bottomPressure = rogh;
						if( elJ == 0 ) {
							bottomP[elI][elK] = element->bottomPressure + PfromAbove[elI][elK];
							if( elI==0 && elK==0 )
								context->pisos = bottomP[elI][elK];
						}
					}
				}
			/* Send bottomP to the proc below if the r-axis is partitioned and this proc is not on the bottom. */
			if( decomp->partitionedAxis[1] && ( rankPartition[1] > 0 ) ) {
				procBelow = rankPartition[0] + (rankPartition[1]-1)*decomp->partition3DCounts[0] +
					rankPartition[2]*decomp->partition3DCounts[0]*decomp->partition3DCounts[1];
				MPI_Send( bottomP, elx*elz, MPI_DOUBLE, procBelow, 0, context->communicator );
			}
		}
		MPI_Barrier( context->communicator );
	} /* End of loop for y procs */
}

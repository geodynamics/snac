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
#include "InitialConditions.h"
#include "Register.h"

#include <string.h>
#ifndef PATH_MAX
	#define PATH_MAX 1024
#endif

#define DEBUG

void _SnacRestartOld_resetMinLengthScale( void* _context, void* data ) {

	Snac_Context*			context = (Snac_Context*)_context;
	FILE* fp;
	char fname[PATH_MAX];
	sprintf( fname, "%s/snac.minLengthScale.restart", context->outputPath );
#ifdef DEBUG
	fprintf(stderr,"RestartOld:  reading min length scale from %s\n",fname);
#endif
	Journal_Firewall( (fp = fopen( fname, "r" )) != NULL, "Failed to open %s\n", fname );
	fscanf(fp,"%le",&(context->initMinLengthScale));
	fclose( fp );

}

void _SnacRestartOld_InitialCoords( void* _context, void* data ) {
	Snac_Context*			context = (Snac_Context*)_context;
	FILE*				fp;
	Node_LocalIndex			node_lI;
	char				path[PATH_MAX];

	Journal_Printf( context->snacInfo, "In: %s\n", __func__ );

	sprintf(path, "%s/snac.coord.%d.%06d.restart",context->outputPath,context->rank,context->restartTimestep);
#ifdef DEBUG
	fprintf(stderr,"RestartOld:  loading mesh coordinates from %s for r=%d, ts=%d \n",path,context->rank,context->restartTimestep);
#endif
	Journal_Firewall( (fp = fopen(path,"r")) != NULL, "Can't find %s - is the parameter \"restartTimestep\" set correctly in the input xml?\n", path);

	/* read in restart file to construct the initial mesh */
	for( node_lI = 0; node_lI < context->mesh->nodeLocalCount; node_lI++ ) {
		Coord*		coord = Snac_NodeCoord_P( context, node_lI );
		double		x,y,z;
		fscanf( fp, "%le %le %le", &x,&y,&z);

		(*coord)[0] = x;
		(*coord)[1] = y;
		(*coord)[2] = z;
#ifdef DEBUG
/* 		fprintf(stderr,"Node %d:  %g, %g, %g\n", node_lI, x,y,z); */
#endif
	}
	fclose( fp );
}

void _SnacRestartOld_InitialVelocities( void* _context, void* data ) {
	Snac_Context*			context = (Snac_Context*)_context;
	FILE*				fp;
	Node_LocalIndex			node_lI;
	char				path[PATH_MAX];

	Journal_Printf( context->snacInfo, "In: %s\n", __func__ );

	sprintf(path, "%s/snac.vel.%d.%06d.restart",context->outputPath,context->rank,context->restartTimestep);
#ifdef DEBUG
	fprintf(stderr,"RestartOld:  loading mesh velocities from %s for r=%d, ts=%d \n",path,context->rank,context->restartTimestep);
#endif
	Journal_Firewall( (fp = fopen(path,"r")) != NULL, "Can't find %s - is the parameter \"restartTimestep\" set correctly in the input xml?\n", path);

	/* read in restart file to construct the initial mesh */
	for( node_lI = 0; node_lI < context->mesh->nodeLocalCount; node_lI++ ) {
		Snac_Node*			node = Snac_Node_At( context, node_lI );
		double				vx,vy,vz;
		fscanf( fp, "%le %le %le", &vx,&vy,&vz);

		node->velocity[0] = vx;
		node->velocity[1] = vy;
		node->velocity[2] = vz;
	}
	fclose( fp );
}

void _SnacRestartOld_InitialStress( void* _context, void* data ) {
	Snac_Context*			context = (Snac_Context*)_context;
	FILE*				fp;
	Element_LocalIndex		element_lI;
	char				path[PATH_MAX];

	Journal_Printf( context->snacInfo, "In: %s\n", __func__ );
	sprintf(path, "%s/snac.stressTensor.%d.%06d.restart",context->outputPath,context->rank,context->restartTimestep);
	Journal_Firewall( (fp = fopen(path,"r")) != NULL, "Can't find %s - is the parameter \"restartTimestep\" set correctly in the input xml?\n", path );

#ifdef DEBUG
	fprintf(stderr,"RestartOld:  loading stress tensors from %s for r=%d, ts=%d \n",path,context->rank,context->restartTimestep);
#endif

	/* read in restart file to assign initial stress field. */
	for( element_lI = 0; element_lI < context->mesh->elementLocalCount; element_lI++ ) {
		Snac_Element*			element = Snac_Element_At( context, element_lI );
		Tetrahedra_Index			tetra_I;
		Stress				sVolAvg=0.0f;
		Stress				sOtherAvg=0.0f;

		for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
			double				S[3][3];
			Index               i,j;
			fscanf( fp, "%le %le %le %le %le %le %le %le %le", &S[0][0],&S[0][1],&S[0][2],&S[1][0],&S[1][1],&S[1][2],&S[2][0],&S[2][1],&S[2][2]);
			for(i=0;i<3;i++)
				for(j=0;j<3;j++)
					element->tetra[tetra_I].stress[i][j] = S[i][j];
		}
		element->hydroPressure = 0.0;
		for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
			element->hydroPressure += ((element->tetra[tetra_I].stress[0][0]+element->tetra[tetra_I].stress[1][1]+element->tetra[tetra_I].stress[2][2])/3.0/Tetrahedra_Count);
			sVolAvg +=
				element->tetra[tetra_I].stress[1][1] * element->tetra[tetra_I].stress[2][2] +
				element->tetra[tetra_I].stress[2][2] * element->tetra[tetra_I].stress[0][0] +
				element->tetra[tetra_I].stress[0][0] * element->tetra[tetra_I].stress[1][1];
			sOtherAvg +=
				element->tetra[tetra_I].stress[0][1] * element->tetra[tetra_I].stress[0][1] +
				element->tetra[tetra_I].stress[1][2] * element->tetra[tetra_I].stress[1][2] +
				element->tetra[tetra_I].stress[0][2] * element->tetra[tetra_I].stress[0][2];
		}
		sVolAvg /= Tetrahedra_Count;
		sOtherAvg /= Tetrahedra_Count;
		element->stress = 0.5 * sqrt( 0.5 * fabs( -1.0f * sVolAvg + sOtherAvg ) );
	}
	fclose( fp );
}

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
*/
/** \file
** Role:
**	Converts Snac's binary output to VTK format
**
** $Id: snac2vtk.c 3270 2006-11-26 06:33:20Z EunseoChoi $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <limits.h>
#ifndef PATH_MAX
	#define PATH_MAX 1024
#endif


void ConvertTimeStep( int rank, unsigned int dumpIteration, unsigned int simTimeStep, double time );

char		path[PATH_MAX];
FILE*		strainRateIn;
FILE*		stressIn;
FILE*		pisosIn;
FILE*		coordIn;
FILE*		velIn;
FILE*		forceIn;
FILE*		phaseIn;
FILE*		tempIn;
FILE*		apsIn;
FILE*		viscIn;

unsigned int	elementGlobalSize[3];
int 			doTemp = 1;
int 			doForce = 1;
int 			doAps = 1;
int 			doHPr = 1;
int 			doVisc = 1;

int main( int argc, char* argv[]) {
	char		tmpBuf[PATH_MAX];
	FILE*		simIn;
	FILE*		timeStepIn;
	unsigned int	rank;
	unsigned int	simTimeStep;
	unsigned int	dumpIteration;
	double		time;
	double		dt;
	
	
	/* TODO, get from arg 1 */
	sprintf( path, "./" );
	
	rank = 0;
	while( 1 ) {
		/* open the input files */
		sprintf( tmpBuf, "%s/sim.%u", path, rank );
		if( (simIn = fopen( tmpBuf, "r" )) == NULL ) {
			if( rank == 0 ) {
				assert( simIn /* failed to open file for reading */ );
			}
			else {
				break;
			}
		}
		sprintf( tmpBuf, "%s/timeStep.%u", path, rank );
		if( (timeStepIn = fopen( tmpBuf, "r" )) == NULL ) {
			assert( timeStepIn /* failed to open file for reading */ );
		}
		sprintf( tmpBuf, "%s/strainRate.%u", path, rank );
		if( (strainRateIn = fopen( tmpBuf, "r" )) == NULL ) {
			assert( strainRateIn /* failed to open file for reading */ );
		}
		sprintf( tmpBuf, "%s/stress.%u", path, rank );
		if( (stressIn = fopen( tmpBuf, "r" )) == NULL ) {
			assert( stressIn /* failed to open file for reading */ );
		}
		sprintf( tmpBuf, "%s/hydroPressure.%u", path, rank );
		if( ( pisosIn = fopen( tmpBuf, "r" )) == NULL ) {
                         printf( "Warning, no hydropressure.%u found... assuming the new context is not written.\n", rank );
                        doHPr = 0;
		}
		sprintf( tmpBuf, "%s/coord.%u", path, rank );
		if( (coordIn = fopen( tmpBuf, "r" )) == NULL ) {
			assert( coordIn /* failed to open file for reading */ );
		}
		sprintf( tmpBuf, "%s/vel.%u", path, rank );
		if( (velIn = fopen( tmpBuf, "r" )) == NULL ) {
			assert( velIn /* failed to open file for reading */ );
		}
		sprintf( tmpBuf, "%s/force.%u", path, rank );
		if( (forceIn = fopen( tmpBuf, "r" )) == NULL ) {
			printf( "Warning, no force.%u found... assuming force outputting not enabled.\n", rank );
			doForce = 0;
		}
		sprintf( tmpBuf, "%s/phaseIndex.%u", path, rank );
		if( (phaseIn = fopen( tmpBuf, "r" )) == NULL ) {
			assert( phaseIn /* failed to open file for reading */ );
		}
		sprintf( tmpBuf, "%s/temperature.%u", path, rank );
		if( (tempIn = fopen( tmpBuf, "r" )) == NULL ) {
			printf( "Warning, no temperature.%u found... assuming temperature plugin not used.\n", rank );
			doTemp = 0;
		}
		sprintf( tmpBuf, "%s/plStrain.%u", path, rank );
		if( (apsIn = fopen( tmpBuf, "r" )) == NULL ) {
			printf( "Warning, no plStrain.%u found... assuming plastic plugin not used.\n", rank );
			doAps = 0;
		}
		sprintf( tmpBuf, "%s/viscosity.%u", path, rank );
		if( (viscIn = fopen( tmpBuf, "r" )) == NULL ) {
			printf( "Warning, no viscosity.%u found... assuming maxwell plugin not used.\n", rank );
			doVisc = 0;
		}
		
		
		/* Read in simulation information... TODO: assumes nproc=1 */
		fscanf( simIn, "%u %u %u\n", &elementGlobalSize[0], &elementGlobalSize[1], &elementGlobalSize[2] );
		
		/* Read in loop information */
		dumpIteration = 0;
		while( !feof( timeStepIn ) ) {
			fscanf( timeStepIn, "%16u %16lg %16lg\n", &simTimeStep, &time, &dt );
			if( argc == 3 )
				if( simTimeStep < atoi(argv[1]) || simTimeStep > atoi(argv[2]) ) {
					dumpIteration++;
					continue;
				}
			ConvertTimeStep( rank, dumpIteration, simTimeStep, time );
			dumpIteration++;
		}
		
		/* Close the input files */
		if( apsIn ) {
			fclose( apsIn );
		}
		if( viscIn ) {
			fclose( viscIn );
		}
		if( tempIn ) {
			fclose( tempIn );
		}
		if( forceIn ) {
			fclose( forceIn );
		}
		if( pisosIn ) {
			fclose( pisosIn );
		}
		fclose( phaseIn );
		fclose( velIn );
		fclose( coordIn );
		fclose( stressIn );
		fclose( strainRateIn );
		fclose( timeStepIn );
		fclose( simIn );
		
		/* do next rank */
		rank++;
	}
	
	return 0;
}


void ConvertTimeStep( int rank, unsigned int dumpIteration, unsigned int simTimeStep, double time ) {
	char		tmpBuf[PATH_MAX];
	FILE*		vtkOut;
	unsigned int	elementGlobalCount = elementGlobalSize[0] * elementGlobalSize[1] * elementGlobalSize[2];
	unsigned int	nodeGlobalSize[3] = { elementGlobalSize[0] + 1, elementGlobalSize[1] + 1, elementGlobalSize[2] + 1 };
	unsigned int	nodeGlobalCount = nodeGlobalSize[0] * nodeGlobalSize[1] * nodeGlobalSize[2];
	unsigned int	node_gI;
	unsigned int	element_gI;
	
	/* open the output file */
	sprintf( tmpBuf, "%s/snac.%i.%06u.vtk", path, rank, simTimeStep );
	if( (vtkOut = fopen( tmpBuf, "w+" )) == NULL ) {
		assert( vtkOut /* failed to open file for writing */ );
	}
	
	/* Write out simulation information */
	fprintf( vtkOut, "# vtk DataFile Version 3.0\nSnac simulation output\nASCII\n" );
	
	/* Write out node information ... TODO: assumes nproc=1 */
	fprintf( vtkOut, "\nDATASET STRUCTURED_GRID\nDIMENSIONS %u %u %u\nPOINTS %u float\n", 
		nodeGlobalSize[0], nodeGlobalSize[1], nodeGlobalSize[2], nodeGlobalCount );
	fseek( coordIn, dumpIteration * nodeGlobalCount * sizeof(float) * 3, SEEK_SET );
	for( node_gI = 0; node_gI < nodeGlobalCount; node_gI++ ) {
		float		coord[3];
		
		fread( &coord, sizeof(float), 3, coordIn );
		fprintf( vtkOut, "%g %g %g\n", coord[0], coord[1], coord[2] );
	}
	fprintf( vtkOut, "\n" );
	
	
	/* Start the element section */
	fprintf( vtkOut, "CELL_DATA %u\n\n", elementGlobalCount );
	
	/* Write out the strain rate information */
	fprintf( vtkOut, "SCALARS strainRate float\nLOOKUP_TABLE default\n" );
	fseek( strainRateIn, dumpIteration * elementGlobalCount * sizeof(float), SEEK_SET );
	for( element_gI = 0; element_gI < elementGlobalCount; element_gI++ ) {
		float		strainRate;
		
		fread( &strainRate, sizeof(float), 1, strainRateIn );
		fprintf( vtkOut, "%g ", strainRate );
	}
	fprintf( vtkOut, "\n\n" );
	
	/* Write out the stress information */
	fprintf( vtkOut, "SCALARS stress float\nLOOKUP_TABLE default\n" );
	fseek( stressIn, dumpIteration * elementGlobalCount * sizeof(float), SEEK_SET );
	for( element_gI = 0; element_gI < elementGlobalCount; element_gI++ ) {
		float		stress;
		
		fread( &stress, sizeof(float), 1, stressIn );
		fprintf( vtkOut, "%g ", stress );
	}
	fprintf( vtkOut, "\n\n" );
	
	
	/* Write out the plastic strain information */
	if( doAps ) {
		fprintf( vtkOut, "SCALARS plStrain float\nLOOKUP_TABLE default\n" );
		fseek( apsIn, dumpIteration * elementGlobalCount * sizeof(float), SEEK_SET );
		for( element_gI = 0; element_gI < elementGlobalCount; element_gI++ ) {
			float		plStrain;
			
			fread( &plStrain, sizeof(float), 1, apsIn );
			fprintf( vtkOut, "%g ", plStrain );
		}
		fprintf( vtkOut, "\n\n" );
	}

	/* Write out the pressure information */
	if( doHPr ) {
		fprintf( vtkOut, "SCALARS pressure float\nLOOKUP_TABLE default\n" );
		fseek( pisosIn, dumpIteration * elementGlobalCount * sizeof(float), SEEK_SET );
		for( element_gI = 0; element_gI < elementGlobalCount; element_gI++ ) {
			float		pressure;
			
			fread( &pressure, sizeof(float), 1, pisosIn );
			fprintf( vtkOut, "%g ", pressure );
		}
		fprintf( vtkOut, "\n\n" );
	}

	/* Write out the phase information */
	fprintf( vtkOut, "SCALARS phase float\nLOOKUP_TABLE default\n" );
	fseek( phaseIn, dumpIteration * elementGlobalCount * sizeof(unsigned int), SEEK_SET );
	for( element_gI = 0; element_gI < elementGlobalCount; element_gI++ ) {
		unsigned int		phaseIndex;
		
		fread( &phaseIndex, sizeof(unsigned int), 1, phaseIn );
		fprintf( vtkOut, "%u ", phaseIndex );
	}
	fprintf( vtkOut, "\n\n" );

	/* Write out the pressure information */
	if( doVisc ) {
		fprintf( vtkOut, "SCALARS viscosity float\nLOOKUP_TABLE default\n" );
		fseek( viscIn, dumpIteration * elementGlobalCount * sizeof(float), SEEK_SET );
		for( element_gI = 0; element_gI < elementGlobalCount; element_gI++ ) {
			float		viscosity;
			
			fread( &viscosity, sizeof(float), 1, viscIn );
			fprintf( vtkOut, "%g ", viscosity );
		}
		fprintf( vtkOut, "\n\n" );
	}

	
	/* Start the node section */
	fprintf( vtkOut, "POINT_DATA %u\n\n", nodeGlobalCount );
	
	/* Write out the velocity information */
	fprintf( vtkOut, "VECTORS velocity float\n" );
	fseek( velIn, dumpIteration * nodeGlobalCount * sizeof(float) * 3, SEEK_SET );
	for( node_gI = 0; node_gI < nodeGlobalCount; node_gI++ ) {
		float		vel[3];
		
		fread( &vel, sizeof(float), 3, velIn );
		fprintf( vtkOut, "%g %g %g\n", vel[0], vel[1], vel[2] );
	}
	fprintf( vtkOut, "\n" );
	
	/* Write out the force information */
	if( doForce ) {
		fprintf( vtkOut, "VECTORS force float\n" );
		fseek( forceIn, dumpIteration * nodeGlobalCount * sizeof(float) * 3, SEEK_SET );
		for( node_gI = 0; node_gI < nodeGlobalCount; node_gI++ ) {
			float		force[3];
			
			fread( &force, sizeof(float), 3, forceIn );
			fprintf( vtkOut, "%g %g %g\n", force[0], force[1], force[2] );
		}
		fprintf( vtkOut, "\n" );
	}
	
	/* Write out the temperature information */
	if( doTemp ) {
		fprintf( vtkOut, "SCALARS temperature float\nLOOKUP_TABLE default\n" );
		fseek( tempIn, dumpIteration * nodeGlobalCount * sizeof(float), SEEK_SET );
		for( node_gI = 0; node_gI < nodeGlobalCount; node_gI++ ) {
			float		temperature;
			
			fread( &temperature, sizeof(float), 1, tempIn );
			fprintf( vtkOut, "%g ", temperature );
		}
		fprintf( vtkOut, "\n\n" );
	}
	
	/* Close the input files */
	fclose( vtkOut );
}

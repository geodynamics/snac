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
**	Converts Snac's binary output to GMV format
**
** $Id: snac2gmv.c 2282 2004-11-04 08:34:24Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


/* TODO... THIS FILE IS OUT OF DATE!!! */


#include <stdio.h>
#include <assert.h>

#include <limits.h>
#ifndef PATH_MAX
	#define PATH_MAX 1024
#endif


void ConvertTimeStep( unsigned int timeStep, double time );

char		path[PATH_MAX];
FILE*		strainRateIn;
FILE*		stressIn;
FILE*		coordIn;
FILE*		velIn;

unsigned int	elementGlobalSize[3];

int main() {
	char		tmpBuf[PATH_MAX];
	FILE*		simIn;
	FILE*		timeStepIn;
	unsigned int	rank = 0;
	unsigned int	timeStep;
	double		time;
	
	
	/* TODO, get from arg 1 */
	sprintf( path, "./" );
	
	/* open the input files */
	sprintf( tmpBuf, "%s/sim.%u", path, rank );
	if( (simIn = fopen( tmpBuf, "r" )) == NULL ) {
		assert( simIn /* failed to open file for reading */ );
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
	sprintf( tmpBuf, "%s/coord.%u", path, rank );
	if( (coordIn = fopen( tmpBuf, "r" )) == NULL ) {
		assert( coordIn /* failed to open file for reading */ );
	}
	sprintf( tmpBuf, "%s/vel.%u", path, rank );
	if( (velIn = fopen( tmpBuf, "r" )) == NULL ) {
		assert( velIn /* failed to open file for reading */ );
	}


	/* Read in simulation information... TODO: assumes nproc=1 */
	fscanf( simIn, "%u %u %u\n", &elementGlobalSize[0], &elementGlobalSize[1], &elementGlobalSize[2] );
	
	/* Read in timeStep information */
	while( !feof( timeStepIn ) ) {
		fscanf( timeStepIn, "%16u %16lg\n", &timeStep, &time );
		ConvertTimeStep( timeStep, time );
	}
	
	/* Close the input files */
	fclose( velIn );
	fclose( coordIn );
	fclose( stressIn );
	fclose( strainRateIn );
	fclose( timeStepIn );
	fclose( simIn );
	
	return 0;
}


void ConvertTimeStep( unsigned int timeStep, double time ) {
	char		tmpBuf[PATH_MAX];
	FILE*		gmvOut;
	unsigned int	elementGlobalCount = elementGlobalSize[0] * elementGlobalSize[1] * elementGlobalSize[2];
	unsigned int	nodeGlobalSize[3] = { elementGlobalSize[0] + 1, elementGlobalSize[1] + 1, elementGlobalSize[2] + 1 };
	unsigned int	nodeGlobalCount = nodeGlobalSize[0] * nodeGlobalSize[1] * nodeGlobalSize[2];
	unsigned int	axis_I;
	unsigned int	element_K;
	unsigned int	element_I;
	unsigned int    count;

	/* open the output file */	
	sprintf( tmpBuf, "%s/snac.%04u.gmv", path, timeStep );
	if( (gmvOut = fopen( tmpBuf, "w+" )) == NULL ) {
		assert( gmvOut /* failed to open file for writing */ );
	}
	
	/* Write out simulation information */
	fprintf( gmvOut, "gmvinput ascii\n" );

	/* Write out node information ... TODO: assumes nproc=1 */
	fprintf( gmvOut, "nodes %u\n", nodeGlobalCount );
	for( axis_I = 0; axis_I < 3; axis_I++ ) {
		unsigned int	node_I;
		
		fseek( coordIn, timeStep * nodeGlobalCount * sizeof(float) * 3, SEEK_SET );
		fprintf( gmvOut, "\t" );
		
		for( node_I = 0; node_I < nodeGlobalCount; node_I++ ) {
			float		coord[3];
			
			fread( &coord, sizeof(float), 3, coordIn );
			fprintf( gmvOut, "%g ", coord[axis_I] );
		}
		
		fprintf( gmvOut, "\n" );
	}
	
	/* Write out element information */
	#define N3DTo1D( i, j, k ) \
		(((k) * nodeGlobalSize[0] * nodeGlobalSize[1]) + ((j) * nodeGlobalSize[0]) + (i))
	fprintf( gmvOut, "cells %u\n", elementGlobalCount );
	count = 0;
	for( element_K = 0; element_K < elementGlobalSize[2]; element_K++ ) {
		unsigned int	element_J;
		
		for( element_J = 0; element_J < elementGlobalSize[1]; element_J++ ) {
			for( element_I = 0; element_I < elementGlobalSize[0]; element_I++ ) {
				unsigned int node[8];

				node[0] = count - elementGlobalSize[0] * elementGlobalSize[1] * element_K + element_J + 
					(elementGlobalSize[0] + 1) * (elementGlobalSize[1] + 1) * element_K;
				node[1] = node[0] + 1;
				node[2] = node[0] + nodeGlobalSize[0];
				node[3] = node[2] + 1;
				node[4] = node[0] + nodeGlobalSize[0] * nodeGlobalSize[1];
				node[5] = node[4] + 1;
				node[6] = node[4] + nodeGlobalSize[0];
				node[7] = node[6] + 1;
				
				fprintf( gmvOut, "hex 8\n\t%u %u %u %u %u %u %u %u\n",
					node[4] + 1, node[5] + 1, node[1] + 1, node[0] + 1, node[6] + 1, node[7] + 1, node[3] +1, node[2] + 1 );
				
				count++;
			}
		}
	}

	/* Start the variable section */
	fprintf( gmvOut, "variable\n" );

	/* Write out the strain rate information */
	fprintf( gmvOut, "sr 0 \n" );
	fseek( strainRateIn, timeStep * elementGlobalCount * sizeof(float), SEEK_SET );
	for( element_I = 0; element_I < elementGlobalCount; element_I++ ) {
		float		strainRate;
		
		fread( &strainRate, sizeof(float), 1, strainRateIn );
		fprintf( gmvOut, "%g ", strainRate );
	}
	fprintf( gmvOut, "\n" );
	
	/* Write out the strain rate information */
	fprintf( gmvOut, "stress 0 \n" );
	fseek( stressIn, timeStep * elementGlobalCount * sizeof(float), SEEK_SET );
	for( element_I = 0; element_I < elementGlobalCount; element_I++ ) {
		float		stress;
		
		fread( &stress, sizeof(float), 1, stressIn );
		fprintf( gmvOut, "%g ", stress );
	}
	fprintf( gmvOut, "\n" );
	
	/* End the variable section */
	fprintf( gmvOut, "endvars\n\n" );
	
	/* Velocity section */
	fprintf( gmvOut, "velocity 0\n" );
	for( axis_I = 0; axis_I < 3; axis_I++ ) {
		fseek( velIn, timeStep * elementGlobalCount * sizeof(float) * 3, SEEK_SET );
		fprintf( gmvOut, "\t" );
		for( element_I = 0; element_I < elementGlobalCount; element_I++ ) {
			float		vel[3];
			
			fread( &vel, sizeof(float), 3, velIn );
			fprintf( gmvOut, "%g ", vel[axis_I] );
		}
		fprintf( gmvOut, "\n" );
	}
	
	
	/* Wrap up */
	fprintf( gmvOut, "endgmv\n" );
	
	/* Close the input files */
	fclose( gmvOut );
}

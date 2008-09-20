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
**	Converts Snac's binary output to ascii files used for restarting.
**
** $Id: snac2restart.c 2282 2004-11-04 08:34:24Z PatrickSunter $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <limits.h>
#ifndef PATH_MAX
	#define PATH_MAX 1024
#endif
#ifndef Tetrahedra_Count
	#define Tetrahedra_Count 10
#endif


void ConvertTimeStep( int rank, unsigned int dumpIteration, unsigned int simTimeStep, double time );

char		path[PATH_MAX];
FILE*		strTensorIn;
FILE*		coordIn;
FILE*		velIn;
FILE*		tempIn;
FILE*		apsIn;
FILE*		tetApsIn;
FILE*		minLengthIn;
FILE*		isoForceIn;

unsigned int	elementLocalSize[3];
unsigned int 	doTemp = 1;
unsigned int 	doAps = 1;
unsigned int 	restartStep = -1;


void checkArgumentType( int argNum, char* arglist[] )
{
	if( argNum == 1 ) {
		sprintf( path, "./" );
		fprintf(stderr,"\nWill try to create restart files for the last time step in %s.\n",path);
		return;
	}
	else if( argNum == 2 ) {
		if( atoi(arglist[1]) ) {
			restartStep = atoi(arglist[1]);
			sprintf( path, "./" );
			fprintf(stderr,"\nWill try to create restart files for time step %d in %s.\n",restartStep,path);
			return;
		}
		else {
			char zeroChar[PATH_MAX];
			sprintf(zeroChar,"0");
			if( !strcmp( arglist[1], zeroChar) ) {
				fprintf(stderr,"\nDon't try to restart from 0th time step. Just rerun the case!!\n\n");
				exit(0);
			}
			else {
				sprintf( path, "%s", arglist[1]);
				fprintf(stderr,"\nWill try to create restart files for the last time step in %s.\n",path);
				return;
			}
		}
	}
	else if( argNum == 3 ) {
		if( atoi(arglist[1])  && !atoi(arglist[2]) ) {
			restartStep = atoi(arglist[1]);
			sprintf( path, "%s", arglist[2]);
			fprintf(stderr,"\nWill try to create restart files for time step %d in %s.\n",restartStep,path);
			return;
		}
		else {
			fprintf(stderr,"Wrong argument type!!\n\tUSAGE: %s [integer timeStep] [path to output directory]\n",arglist[0]);
			exit(0);
		}
	}
	else if( argNum > 3 ) {
		fprintf(stderr,"Wrong number of arguments!!\n\tUSAGE: %s [timeStep] [output directory]\n",arglist[0]);
		exit(0);
	}
}

int main( int argc, char* argv[] ) {
	char		tmpBuf[PATH_MAX];
	FILE*		simIn;
	FILE*		timeStepIn;
	unsigned int	rank;
	unsigned int	simTimeStep;
	unsigned int	dumpIteration;
	double		time;
	double		dt;

	/* safety check and assign restartStep and path */
	checkArgumentType( argc, argv );

	rank = 0;
	while( 1 ) {
		/* open the input files */
		sprintf( tmpBuf, "%s/sim.%u", path, rank );
		fprintf(stderr,"%s\n",tmpBuf);
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
		sprintf( tmpBuf, "%s/stressTensor.%u", path, rank );
		fprintf(stderr,"filename=%s\n",tmpBuf);
		if( (strTensorIn = fopen( tmpBuf, "r" )) == NULL ) {
			assert( strTensorIn /* failed to open file for reading */ );
		}
		sprintf( tmpBuf, "%s/coord.%u", path, rank );
		if( (coordIn = fopen( tmpBuf, "r" )) == NULL ) {
			assert( coordIn /* failed to open file for reading */ );
		}
		sprintf( tmpBuf, "%s/vel.%u", path, rank );
		if( (velIn = fopen( tmpBuf, "r" )) == NULL ) {
			assert( velIn /* failed to open file for reading */ );
		}
		sprintf( tmpBuf, "%s/minLengthScale.0", path );
		if( (minLengthIn = fopen( tmpBuf, "r" )) == NULL ) {
			assert( minLengthIn /* failed to open file for reading */ );
		}
		sprintf( tmpBuf, "%s/isoForce.%u", path, rank );
		if( (isoForceIn = fopen( tmpBuf, "r" )) == NULL ) {
			assert( isoForceIn /* failed to open file for reading */ );
		}
		sprintf( tmpBuf, "%s/temperature.%u", path, rank );
		if( (tempIn = fopen( tmpBuf, "r" )) == NULL ) {
			printf( "Warning, no temperature.%u found... assuming temperature plugin not used.\n", rank );
			doTemp = 0;
		}
		sprintf( tmpBuf, "%s/plStrainTensor.%u", path, rank );
		if( (tetApsIn = fopen( tmpBuf, "r" )) == NULL ) {
			printf( "Warning, no plstrain.%u found... assuming plastic plugin not used.\n", rank );
			doAps = 0;
		}
		sprintf( tmpBuf, "%s/plStrain.%u", path, rank );
		if( (apsIn = fopen( tmpBuf, "r" )) == NULL ) {
			printf( "Warning, no plstrain.%u found... assuming plastic plugin not used.\n", rank );
			doAps = 0;
		}


		/* Read in simulation information... TODO: assumes nproc=1 */
		fscanf( simIn, "%u %u %u\n", &elementLocalSize[0], &elementLocalSize[1], &elementLocalSize[2] );

		/* Read in loop information */
		dumpIteration = 0;
		while( !feof( timeStepIn ) ) {
			fscanf( timeStepIn, "%16u %16lg %16lg\n", &simTimeStep, &time, &dt );
			/* do conversion */
			if( simTimeStep == restartStep || feof( timeStepIn ) ) {
				ConvertTimeStep( rank, dumpIteration, simTimeStep, time );
				break;
			}
			dumpIteration++;

		}

		/* Close the input files */
		if( apsIn ) {
			fclose( apsIn );
			fclose( tetApsIn );
		}
		if( tempIn ) {
			fclose( tempIn );
		}
		fclose( velIn );
		fclose( coordIn );
		fclose( strTensorIn );
		fclose( timeStepIn );
		fclose( simIn );
		fclose( minLengthIn );
		fclose( isoForceIn );

		/* do next rank */
		rank++;
	}
	fprintf(stderr, "Done!!\n");

	return 0;
}


void ConvertTimeStep( int rank, unsigned int dumpIteration, unsigned int simTimeStep, double time ) {
	char		tmpBuf[PATH_MAX];
	FILE*		restartOut;
	unsigned int	elementLocalCount = elementLocalSize[0] * elementLocalSize[1] * elementLocalSize[2];
	unsigned int	nodeLocalSize[3] = { elementLocalSize[0] + 1, elementLocalSize[1] + 1, elementLocalSize[2] + 1 };
	unsigned int	nodeLocalCount = nodeLocalSize[0] * nodeLocalSize[1] * nodeLocalSize[2];
	unsigned int	node_gI;
	unsigned int	element_gI;
	unsigned int	tetra_I;
	float		minLength;


	/* Write out position array */
	sprintf( tmpBuf, "%s/snac.coord.%i.%06u.restart", path, rank, simTimeStep );
	if( (restartOut = fopen( tmpBuf, "w" )) == NULL ) {
		assert( restartOut /* failed to open file for writing */ );
	}

	fseek( coordIn, dumpIteration * nodeLocalCount * sizeof(float) * 3, SEEK_SET );
	for( node_gI = 0; node_gI < nodeLocalCount; node_gI++ ) {
		float		coord[3];
		fread( &coord, sizeof(float), 3, coordIn );
		fprintf( restartOut, "%.9e %.9e %.9e\n", coord[0], coord[1], coord[2] );
	}
	if( restartOut )
		fclose( restartOut );

	/* Write out initial position array in case of restarting. */
	sprintf( tmpBuf, "%s/snac.initcoord.%i.%06u.restart", path, rank, simTimeStep );
	if( (restartOut = fopen( tmpBuf, "w" )) == NULL ) {
		assert( restartOut /* failed to open file for writing */ );
	}

	fseek( coordIn, 0 * nodeLocalCount * sizeof(float) * 3, SEEK_SET );
	for( node_gI = 0; node_gI < nodeLocalCount; node_gI++ ) {
		float		coord[3];
		fread( &coord, sizeof(float), 3, coordIn );
		fprintf( restartOut, "%.9e %.9e %.9e\n", coord[0], coord[1], coord[2] );
	}
	if( restartOut )
		fclose( restartOut );

	/* Write out velocity array */
	sprintf( tmpBuf, "%s/snac.velo.%i.%06u.restart", path, rank, simTimeStep );
	if( (restartOut = fopen( tmpBuf, "w" )) == NULL ) {
		assert( restartOut /* failed to open file for writing */ );
	}

	fseek( velIn, dumpIteration * nodeLocalCount * sizeof(float) * 3, SEEK_SET );
	for( node_gI = 0; node_gI < nodeLocalCount; node_gI++ ) {
		float		vel[3];
		fread( &vel, sizeof(float), 3, velIn );
		fprintf( restartOut, "%.9e %.9e %.9e\n", vel[0], vel[1], vel[2] );
	}
	if( restartOut )
		fclose( restartOut );

	/* Write out isostatic force array */
	sprintf( tmpBuf, "%s/snac.isoForce.%i.restart", path, rank );
	if( (restartOut = fopen( tmpBuf, "w" )) == NULL ) {
		assert( restartOut /* failed to open file for writing */ );
	}

	for( node_gI = 0; node_gI < nodeLocalCount; node_gI++ ) {
		float		force;
		fread( &force, sizeof(float), 1, isoForceIn );
		fprintf( restartOut, "%.9e\n", force );
	}
	if( restartOut )
		fclose( restartOut );


	/* Write out stress tensor array */
	sprintf( tmpBuf, "%s/snac.strTensor.%i.%06u.restart", path, rank, simTimeStep );
	fprintf(stderr,"filename=%s\n",tmpBuf);
	if( (restartOut = fopen( tmpBuf, "w" )) == NULL ) {
		assert( restartOut /* failed to open file for writing */ );
	}

	fseek( strTensorIn, dumpIteration * elementLocalCount * sizeof(float) * 9 * Tetrahedra_Count, SEEK_SET );
	for( element_gI = 0; element_gI < elementLocalCount; element_gI++ ) {
		for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
			float tensor[3][3];
			fread( &tensor, sizeof(float), 9, strTensorIn );
			fprintf( restartOut, "%.9e %.9e %.9e %.9e %.9e %.9e %.9e %.9e %.9e\n", tensor[0][0], tensor[0][1], tensor[0][2], tensor[1][0], tensor[1][1], tensor[1][2], tensor[2][0], tensor[2][1], tensor[2][2] );
			fflush( restartOut );
		}
	}
	if( restartOut )
		fclose( restartOut );

	/* Write out temperature array */
	if(doTemp) {
		sprintf( tmpBuf, "%s/snac.temp.%i.%06u.restart", path, rank, simTimeStep );
		if( (restartOut = fopen( tmpBuf, "w" )) == NULL ) {
			assert( restartOut /* failed to open file for writing */ );
		}

		fseek( tempIn, dumpIteration * nodeLocalCount * sizeof(float), SEEK_SET );
		for( node_gI = 0; node_gI < nodeLocalCount; node_gI++ ) {
		        float		temperature;
			fread( &temperature, sizeof(float), 1, tempIn );
			fprintf( restartOut, "%.9e\n", temperature );
		}
		if( restartOut )
			fclose( restartOut );
	}

	/* Write out plstrain array */
	if(doAps) {
		sprintf( tmpBuf, "%s/snac.plStrainTensor.%i.%06u.restart", path, rank, simTimeStep );
		if( (restartOut = fopen( tmpBuf, "w" )) == NULL ) {
			assert( restartOut /* failed to open file for writing */ );
		}
			/* for Drucker-Prager */
/* 		fseek( tetApsIn, dumpIteration * elementLocalCount * sizeof(float) * 9 * Tetrahedra_Count, SEEK_SET ); */
/* 		for( element_gI = 0; element_gI < elementLocalCount; element_gI++ ) { */
/* 			for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) { */
/* 				int i,j; */
/* 				float tensor[3][3]; */
/* 				for(i=0;i<3;i++) */
/* 					for(j=0;j<3;j++) */
/* 						fread( &tensor[i][j], sizeof(float), 1, tetApsIn ); */
/* 				fprintf( restartOut, "%.9e %.9e %.9e %.9e %.9e %.9e %.9e %.9e %.9e\n", tensor[0][0], tensor[0][1], tensor[0][2], tensor[1][0], tensor[1][1], tensor[1][2], tensor[2][0], tensor[2][1], tensor[2][2] ); */
/* 				fflush( restartOut ); */
/* 			} */
			/* for Mohr-Coulomb */
		fseek( tetApsIn, dumpIteration * elementLocalCount * sizeof(float) * Tetrahedra_Count, SEEK_SET );
		for( element_gI = 0; element_gI < elementLocalCount; element_gI++ ) {
			for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
				float tetraStrain;
				fread( &tetraStrain, sizeof(float), 1, tetApsIn );
				fprintf( restartOut, "%.9e\n", tetraStrain );
				fflush( restartOut );
			}
		}
		if( restartOut )
			fclose( restartOut );


		sprintf( tmpBuf, "%s/snac.plStrain.%i.%06u.restart", path, rank, simTimeStep );
		if( (restartOut = fopen( tmpBuf, "w" )) == NULL ) {
			assert( restartOut /* failed to open file for writing */ );
		}

		fseek( apsIn, dumpIteration * elementLocalCount * sizeof(float), SEEK_SET );
		for( element_gI = 0; element_gI < elementLocalCount; element_gI++ ) {
		        float		plstrain;

			fread( &plstrain, sizeof(float), 1, apsIn );
			fprintf( restartOut, "%.9e\n", plstrain );
		}
		if( restartOut )
			fclose( restartOut );
	}
	/* mininum length scale */
	if( rank == 0 ) {
		sprintf( tmpBuf, "%s/snac.minLengthScale.restart", path );
		if( (restartOut = fopen( tmpBuf, "w" )) == NULL ) {
			assert( restartOut /* failed to open file for writing */ );
		}
		fscanf( minLengthIn, "%e", &minLength );
		fprintf( restartOut, "%e\n", minLength );

		if( restartOut )
			fclose( restartOut );
	}

	return;
}

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
**           Colin Stark, Doherty Research Scientist, Lamont-Doherty Earth Observatory (cstark@ldeo.columbia.edu)
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
#include <math.h>
#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif
#ifndef Tetrahedra_Count
#define Tetrahedra_Count 10
#endif

void ConvertTimeStep( int rank, unsigned int dumpIteration, unsigned int simTimeStep, double time );

char		origPath[PATH_MAX];
char		readPath[PATH_MAX];
char		writePath[PATH_MAX];
FILE*		stressTensorIn;
FILE*		initCoordIn;
FILE*		coordIn;
FILE*		velIn;
FILE*		tempIn;
FILE*		apsIn;
/* FILE*		tetApsIn; */
FILE*		minLengthIn;
FILE*		forceIn;

unsigned int	elementGlobalSize[3];
unsigned int	rank_array[3];
unsigned int	elementLocalSize[3];
unsigned int 	doTemp = 1;
unsigned int 	doAps = 1;
unsigned int 	restartStep = -1;

const double	velocityDampingFactor = 1.0;

const unsigned	numStressVectorComponent = 6; /* 6 components in stress vector */
const unsigned	numStressComponentsPerElement = 60; /* 6 components times 10 tets per element */

void checkArgumentTypeStrict( int argNum, char* arglist[] ) {
    if( argNum == 5 ) {
		if( atoi(arglist[1]) && !atoi(arglist[2]) && !atoi(arglist[3]) && !atoi(arglist[4])) {
			restartStep = atoi(arglist[1]);
			sprintf( origPath, "%s", arglist[2]);
			sprintf( readPath, "%s", arglist[3]);
			sprintf( writePath, "%s", arglist[4]);
			fprintf(stderr,"Reading Snac initial state files from %s/\n",origPath);
			fprintf(stderr,"Reading Snac state files for time step ts=%d from %s/\n",restartStep,readPath);
			fprintf(stderr,"Writing Snac restart files to %s/\n",writePath);
			return;
		}
		else {
			fprintf(stderr,"Wrong argument type\n\tUsage: %s [integer timeStep] [path to first outputs] [your \"outputPath\"] [path to write restart files]\n",arglist[0]);
			exit(0);
		}
    }
    else {
		fprintf(stderr,"Number and kinds of arguments must be exactly like the following:\n");
		fprintf(stderr,"Usage: %s [timeStep] [path to first outputs] [your \"outputPath\"] [path to write restart files]\n",arglist[0]);
		exit(0);
    }
}


void checkArgumentType( int argNum, char* arglist[] ) {
    if( argNum == 1 ) {
		sprintf( readPath, "." );
		sprintf( writePath, "." );
		fprintf(stderr,"Reading Snac state files for the last time step from %s/\n",readPath);
		fprintf(stderr,"Writing Snac restart files to %s/\n",writePath);
		return;
    }
    else if( argNum == 2 ) {
		if( atoi(arglist[1]) ) {
			restartStep = atoi(arglist[1]);
			sprintf( readPath, "." );
			sprintf( writePath, "." );
			fprintf(stderr,"Reading Snac state files for time step ts=%d from %s/\n",restartStep,readPath);
			fprintf(stderr,"Writing Snac restart files to %s/\n",writePath);
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
				sprintf( readPath, "%s", arglist[1]);
				sprintf( writePath, "." );
				fprintf(stderr,"Reading Snac state files for the last time step from %s/\n",readPath);
				fprintf(stderr,"Writing Snac restart files to %s/\n",writePath);
				return;
			}
		}
    }
    else if( argNum == 3 ) {
		if( atoi(arglist[1]) && !atoi(arglist[2]) ) {
			restartStep = atoi(arglist[1]);
			sprintf( readPath, "%s", arglist[2]);
			sprintf( writePath, "%s", arglist[2]);
			fprintf(stderr,"Reading Snac state files for time step ts=%d from %s/\n",restartStep,readPath);
			fprintf(stderr,"Writing Snac restart files to %s/\n",writePath);
			return;
		}
		else {
			fprintf(stderr,"Wrong argument type\n\tUsage: %s [integer timeStep] [your \"outputPath\"] [path to write restart files]\n",arglist[0]);
			exit(0);
		}
    }
    else if( argNum == 4 ) {
		if( atoi(arglist[1]) && !atoi(arglist[2]) && !atoi(arglist[3])) {
			restartStep = atoi(arglist[1]);
			sprintf( readPath, "%s", arglist[2]);
			sprintf( writePath, "%s", arglist[3]);
			fprintf(stderr,"Reading Snac state files for time step ts=%d from %s/\n",restartStep,readPath);
			fprintf(stderr,"Writing Snac restart files to %s/\n",writePath);
			return;
		}
		else {
			fprintf(stderr,"Wrong argument type\n\tUsage: %s [integer timeStep] [your \"outputPath\"] [path to write restart files]\n",arglist[0]);
			exit(0);
		}
    }
    else if( argNum > 3 ) {
		fprintf(stderr,"Wrong number of arguments\n\tUsage: %s [timeStep] [your \"outputPath\"] [path to write restart files]\n",arglist[0]);
		exit(0);
    }
}

int main( int argc, char* argv[] ) {
	char			tmpBuf[PATH_MAX];
	FILE*			simIn;
	FILE*			timeStepIn;
	unsigned int	rank;
	unsigned int	simTimeStep;
	unsigned int	dumpIteration;
	double			time;
	double			dt;
	unsigned int	rankI, rankJ, rankK;

    /* safety check and assign restartStep and path */
    checkArgumentTypeStrict( argc, argv );
/*     checkArgumentType( argc, argv ); */

    fprintf(stderr, "Parsing Snac output files\n");

    /*
     * Read common parameters for mesh geometry and parallel decomposition.
     */
	{
		sprintf( tmpBuf, "%s/sim.0", readPath );
		fprintf(stderr,"Reading from %s",tmpBuf); 
		if( (simIn = fopen( tmpBuf, "r" )) == NULL ) {
			fprintf(stderr, " ... failed - no such file\n");
			abort();
		}
		fscanf( simIn, "%u %u %u %u %u %u %u %u %u\n", 
				&elementGlobalSize[0],&elementGlobalSize[1],&elementGlobalSize[2],
				&rank_array[0],&rank_array[1],&rank_array[2],
				&elementLocalSize[0], &elementLocalSize[1], &elementLocalSize[2] );
		fclose( simIn );
		fprintf(stderr," ... with success\n"); 
	}

	/*
	 * Determine dumpIteration, simTimeStep, and time, here.
	 * These are common to all the processors.
	 */
	{
		dumpIteration=0;
		
		sprintf( tmpBuf, "%s/checkpointTimeStep.0", readPath );
		fprintf(stderr,"Opening %s\n",tmpBuf); 
		if( (timeStepIn = fopen( tmpBuf, "r" )) == NULL ) {
			/* failed to open file for reading */
			fprintf(stderr, " ... failed - no such file\n");
			abort();
		}
		while( !feof( timeStepIn ) ) {
			fscanf( timeStepIn, "%16u %16lg %16lg\n", &simTimeStep, &time, &dt );
			/* do conversion */
			if( simTimeStep == restartStep || feof( timeStepIn ) ) {
				fprintf( stderr,"Will convert dump=%d, ts=%d, t=%g ...\n",
						 dumpIteration, simTimeStep, time);
				break;
			}
			dumpIteration++;
		}
		fclose( timeStepIn );
	}

	/*
	 * Do the conversion for each processor.
	 */
    for( rankK=0; rankK < rank_array[2]; rankK++ )
	for( rankJ=0; rankJ < rank_array[1]; rankJ++ )
	    for( rankI=0; rankI < rank_array[0]; rankI++ ) {
			rank = rankI + rankJ*rank_array[0] + rankK*rank_array[0]*rank_array[1]; 

			fprintf(stderr, "Rank r=%d\n",rank);

			sprintf( tmpBuf, "%s/stressTensorCP.%u", readPath, rank );
			fprintf(stderr,"Reading from %s\n",tmpBuf); 
			if( (stressTensorIn = fopen( tmpBuf, "r" )) == NULL ) {
				/* failed to open file for reading */
				fprintf(stderr, " ... failed - no such file\n");
				assert(0);
				abort();
			}

			sprintf( tmpBuf, "%s/coordCP.%u", readPath, rank );
			fprintf(stderr,"Reading from %s\n",tmpBuf); 
			if( (coordIn = fopen( tmpBuf, "r" )) == NULL ) {
				/* failed to open file for reading */
				fprintf(stderr, " ... failed - no such file\n");
				assert(0);
				abort();
			}
			if( strcmp( readPath, origPath ) ) {
				sprintf( tmpBuf, "%s/coord.%u", origPath, rank );
				fprintf(stderr,"Reading init coords from %s\n",tmpBuf); 
				if( (initCoordIn = fopen( tmpBuf, "r" )) == NULL ) {
					/* failed to open file for reading */
					fprintf(stderr, " ... failed - no such file\n");
					assert(0);
					abort();
				}
			}
			else
				initCoordIn = coordIn;

			sprintf( tmpBuf, "%s/velCP.%u", readPath, rank );
			fprintf(stderr,"Reading from %s\n",tmpBuf); 
			if( (velIn = fopen( tmpBuf, "r" )) == NULL ) {
				/* failed to open file for reading */
				fprintf(stderr, " ... failed - no such file\n");
				assert(0);
				abort();
			}
			sprintf( tmpBuf, "%s/minLengthScale.0", origPath );
			fprintf(stderr,"Reading from %s\n",tmpBuf); 
			if( (minLengthIn = fopen( tmpBuf, "r" )) == NULL ) {
				/* failed to open file for reading */
				fprintf(stderr, " ... failed - no such file\n");
				assert(0);
				abort();
			}
			sprintf( tmpBuf, "%s/temperatureCP.%u", readPath, rank );
			fprintf(stderr,"Reading from %s\n",tmpBuf); 
			if( (tempIn = fopen( tmpBuf, "r" )) == NULL ) {
				fprintf( stderr,"Warning, no temperatureCP.%u found... assuming temperature plugin not used.\n", rank );
				doTemp = 0;
			}
			sprintf( tmpBuf, "%s/plStrainCP.%u", readPath, rank );
			fprintf(stderr,"Reading from %s\n",tmpBuf); 
			if( (apsIn = fopen( tmpBuf, "r" )) == NULL ) {
				fprintf( stderr,"Warning, no plstrainCP.%u found... assuming plastic plugin not used.\n", rank );
				doAps = 0;
			}

			ConvertTimeStep( rank, dumpIteration, simTimeStep, time );

			/* Close the input files */
			if( apsIn ) {
				fclose( apsIn );
				/* 	    fclose( tetApsIn ); */
			}
			if( tempIn ) {
				fclose( tempIn );
			}
			fclose( velIn );
			if( strcmp( readPath, origPath) ) { /* if different */
				fclose( coordIn );
				fclose( initCoordIn );
			}
			else {
				fclose( coordIn );
			}
			fclose( stressTensorIn );
			fclose( minLengthIn );

			/* do next rank */
			rank++;
		}
    fprintf(stderr, "Done\n");

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
    sprintf( tmpBuf, "%s/snac.coord.%i.%06u.restart", writePath, rank, simTimeStep );
    fprintf(stderr,"\tWriting out %s\n",tmpBuf);
    if( (restartOut = fopen( tmpBuf, "w" )) == NULL ) {
		/* failed to open file for writing */
		fprintf(stderr, "Failed to open %s for writing\n", tmpBuf);
		exit(0);
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
    sprintf( tmpBuf, "%s/snac.initCoord.%i.%06u.restart", writePath, rank, simTimeStep );
    fprintf(stderr,"\tWriting out %s\n",tmpBuf);
    if( (restartOut = fopen( tmpBuf, "w" )) == NULL ) {
		/* failed to open file for writing */
		fprintf(stderr, "Failed to open %s for writing\n", tmpBuf);
		exit(0);
    }
    fseek( initCoordIn, 0, SEEK_SET );
    for( node_gI = 0; node_gI < nodeLocalCount; node_gI++ ) {
		float		coord[3];
		fread( &coord, sizeof(float), 3, initCoordIn );
		fprintf( restartOut, "%.9e %.9e %.9e\n", coord[0], coord[1], coord[2] );
    }
    if( restartOut )
		fclose( restartOut );

    /* Write out velocity array */
    sprintf( tmpBuf, "%s/snac.vel.%i.%06u.restart", writePath, rank, simTimeStep );
    fprintf(stderr,"\tWriting out %s\n",tmpBuf);
    if( (restartOut = fopen( tmpBuf, "w" )) == NULL ) {
		/* failed to open file for writing */
		fprintf(stderr, "Failed to open %s for writing\n", tmpBuf);
		exit(0);
    }
    /*
     *  CPS hack:  need to try damping/zeroing of node velocities in expts where we are
     *  restarting from assumed static elastic equilibrium
     *
     *  The variable  velocityDampingFactor  should be passed from the command line
     *  BEWARE that it is fixed at 1.0 for now.
     */
    fseek( velIn, dumpIteration * nodeLocalCount * sizeof(float) * 3, SEEK_SET );
    for( node_gI = 0; node_gI < nodeLocalCount; node_gI++ ) {
		float		vel[3];
		fread( &vel, sizeof(float), 3, velIn );
		fprintf( restartOut, "%.9e %.9e %.9e\n", velocityDampingFactor*vel[0], velocityDampingFactor*vel[1], velocityDampingFactor*vel[2] );
		/* 	fprintf( restartOut, "%.9e %.9e %.9e\n", vel[0], vel[1], vel[2] ); */
    }
    if( restartOut )
		fclose( restartOut );

#if 0
    /* Write out isostatic force array */
    sprintf( tmpBuf, "%s/snac.force.%i.%06u.restart", writePath, rank, simTimeStep );
    fprintf(stderr,"\tWriting out %s\n",tmpBuf);
    if( (restartOut = fopen( tmpBuf, "w" )) == NULL ) {
		/* failed to open file for writing */
		fprintf(stderr, "Failed to open %s for writing\n", tmpBuf);
		exit(0);
    }
    for( node_gI = 0; node_gI < nodeLocalCount; node_gI++ ) {
		float		force;
		fread( &force, sizeof(float), 1, forceIn );
		fprintf( restartOut, "%.9e\n", force );
    }
    if( restartOut )
		fclose( restartOut );
#endif

    /* Write out stress tensor array */
    sprintf( tmpBuf, "%s/snac.stressTensor.%i.%06u.restart", writePath, rank, simTimeStep );
    fprintf(stderr,"\tWriting out %s\n",tmpBuf);
    if( (restartOut = fopen( tmpBuf, "w" )) == NULL ) {
		/* failed to open file for writing */
		fprintf(stderr, "Failed to open %s for writing\n", tmpBuf);
		exit(0);
    }
    fseek( stressTensorIn, dumpIteration * elementLocalCount * sizeof(float) * numStressComponentsPerElement, SEEK_SET );
    for( element_gI = 0; element_gI < elementLocalCount; element_gI++ ) {
		for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
			float	stressTensorArray[numStressVectorComponent];

			if ( fread( stressTensorArray, sizeof(float), numStressVectorComponent, stressTensorIn )==0 )  {
				if (feof(stressTensorIn)) {
					fprintf(stderr, "Error (reached EOF prematurely) while reading Snac stress tensor output file: tetrahedral element #%d\n" , tetra_I);
					exit(1);
				} else {
					fprintf(stderr, "Error while reading Snac stress tensor output file: tetrahedral element #%d\n" , tetra_I);
					exit(1);
				}
			}
			if(isnan(stressTensorArray[0]) || isnan(stressTensorArray[1]) 
			   || isnan(stressTensorArray[2]) || isnan(stressTensorArray[3]) 
			   || isnan(stressTensorArray[4]) || isnan(stressTensorArray[5])) {
				fprintf(stderr,"NaN in stress tensor file\n");
				abort();
			}
			fprintf( restartOut, "%.9e %.9e %.9e %.9e %.9e %.9e\n", 
					 stressTensorArray[0],stressTensorArray[1],stressTensorArray[2],
					 stressTensorArray[3],stressTensorArray[4],stressTensorArray[5] );
			fflush( restartOut );
		}
	}
    if( restartOut )
		fclose( restartOut );

    /* Write out temperature array */
    if(doTemp) {
		sprintf( tmpBuf, "%s/snac.temp.%i.%06u.restart", writePath, rank, simTimeStep );
		fprintf(stderr,"\tWriting out %s\n",tmpBuf);
		if( (restartOut = fopen( tmpBuf, "w" )) == NULL ) {
			/* failed to open file for writing */
			fprintf(stderr, "Failed to open %s for writing\n", tmpBuf);
			exit(0);
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
		sprintf( tmpBuf, "%s/snac.plStrain.%i.%06u.restart", writePath, rank, simTimeStep );
		fprintf(stderr,"\tWriting out %s\n",tmpBuf);
		if( (restartOut = fopen( tmpBuf, "w" )) == NULL ) {
			/* failed to open file for writing */
			fprintf(stderr, "Failed to open %s for writing\n", tmpBuf);
			exit(0);
		}

		fseek( apsIn, dumpIteration * elementLocalCount * sizeof(float), SEEK_SET );
		for( element_gI = 0; element_gI < elementLocalCount; element_gI++ ) {
			for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
				float		plstrain;
				fread( &plstrain, sizeof(float), 1, apsIn );
				fprintf( restartOut, "%.9e\n", plstrain );
			}
		}
		if( restartOut )
			fclose( restartOut );
    }
    /* mininum length scale */
    if( rank == 0 ) {
		sprintf( tmpBuf, "%s/snac.minLengthScale.restart", writePath );
		fprintf(stderr,"\tWriting out %s\n",tmpBuf);
		if( (restartOut = fopen( tmpBuf, "w" )) == NULL ) {
			/* failed to open file for writing */
			fprintf(stderr, "Failed to open %s for writing\n", tmpBuf);
			exit(0);
		}
		fscanf( minLengthIn, "%e", &minLength );
		fprintf( restartOut, "%e\n", minLength );

		if( restartOut )
			fclose( restartOut );
    }

    return;
}

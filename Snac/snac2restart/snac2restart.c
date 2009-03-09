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

#include <limits.h>
#ifndef PATH_MAX
	#define PATH_MAX 1024
#endif
#ifndef Tetrahedra_Count
	#define Tetrahedra_Count 10
#endif


void ConvertTimeStep( int rank, unsigned int dumpIteration, unsigned int simTimeStep, double time );

char		readPath[PATH_MAX];
char		writePath[PATH_MAX];
FILE*		stressTensorIn;
FILE*		coordIn;
FILE*		velIn;
FILE*		tempIn;
FILE*		apsIn;
/* FILE*		tetApsIn; */
FILE*		minLengthIn;
FILE*		forceIn;

unsigned int	elementLocalSize[3];
unsigned int 	doTemp = 1;
unsigned int 	doAps = 1;
unsigned int 	restartStep = -1;

const double	velocityDampingFactor = 1.0;


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
	if( atoi(arglist[1])  && !atoi(arglist[2]) ) {
	    restartStep = atoi(arglist[1]);
	    sprintf( readPath, "%s", arglist[2]);
	    fprintf(stderr,"Reading Snac state files for time step ts=%d from %s/\n",restartStep,readPath);
	    fprintf(stderr,"Writing Snac restart files to %s/\n",writePath);
	    return;
	}
	else {
	    fprintf(stderr,"Wrong argument type\n\tUsage: %s [integer timeStep] [input file path]\n",arglist[0]);
	    exit(0);
	}
    }
    else if( argNum > 3 ) {
	fprintf(stderr,"Wrong number of arguments\n\tUsage: %s [timeStep] [input file path]\n",arglist[0]);
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

    fprintf(stderr, "Parsing Snac output files\n");
    rank = 0;
    while( 1 ) {
	fprintf(stderr, "Rank r=%d\n",rank);
	/* open the input files */
	sprintf( tmpBuf, "%s/sim.%u", readPath, rank );
 	fprintf(stderr,"Attempting to read from %s ...",tmpBuf); 
	if( (simIn = fopen( tmpBuf, "r" )) == NULL ) {
	    if( rank == 0 ) {
		/* failed to open file for reading */
		fprintf(stderr, " failed - no such file\n");
		exit(0);
	    }
	    else {
		fprintf(stderr," no such file\n"); 
		break;
	    }
	} else {
		fprintf(stderr," with success\n"); 
	}
	sprintf( tmpBuf, "%s/timeStep.%u", readPath, rank );
 	fprintf(stderr,"Reading from %s\n",tmpBuf); 
	if( (timeStepIn = fopen( tmpBuf, "r" )) == NULL ) {
	    /* failed to open file for reading */
	    fprintf(stderr, " ... failed - no such file\n");
	    exit(0);
	}
	sprintf( tmpBuf, "%s/stressTensor.%u", readPath, rank );
 	fprintf(stderr,"Reading from %s\n",tmpBuf); 
	if( (stressTensorIn = fopen( tmpBuf, "r" )) == NULL ) {
	    /* failed to open file for reading */
	    fprintf(stderr, " ... failed - no such file\n");
	    exit(0);
	}
	sprintf( tmpBuf, "%s/coord.%u", readPath, rank );
 	fprintf(stderr,"Reading from %s\n",tmpBuf); 
	if( (coordIn = fopen( tmpBuf, "r" )) == NULL ) {
	    /* failed to open file for reading */
	    fprintf(stderr, " ... failed - no such file\n");
	    exit(0);
	}
	sprintf( tmpBuf, "%s/vel.%u", readPath, rank );
 	fprintf(stderr,"Reading from %s\n",tmpBuf); 
	if( (velIn = fopen( tmpBuf, "r" )) == NULL ) {
	    /* failed to open file for reading */
	    fprintf(stderr, " ... failed - no such file\n");
	    exit(0);
	}
	sprintf( tmpBuf, "%s/minLengthScale.0", readPath );
 	fprintf(stderr,"Reading from %s\n",tmpBuf); 
	if( (minLengthIn = fopen( tmpBuf, "r" )) == NULL ) {
	    /* failed to open file for reading */
	    fprintf(stderr, " ... failed - no such file\n");
	    exit(0);
	}
	sprintf( tmpBuf, "%s/force.%u", readPath, rank );
 	fprintf(stderr,"Reading from %s\n",tmpBuf); 
	if( (forceIn = fopen( tmpBuf, "r" )) == NULL ) {
	    /* failed to open file for reading */
	    fprintf(stderr, " ... failed - no such file\n");
	    exit(0);
	}
	sprintf( tmpBuf, "%s/temperature.%u", readPath, rank );
 	fprintf(stderr,"Reading from %s\n",tmpBuf); 
	if( (tempIn = fopen( tmpBuf, "r" )) == NULL ) {
	    fprintf( stderr,"Warning, no temperature.%u found... assuming temperature plugin not used.\n", rank );
	    doTemp = 0;
	}
/* 	sprintf( tmpBuf, "%s/plStrainTensor.%u", readPath, rank ); */
/*  	fprintf(stderr,"Reading from %s\n",tmpBuf);  */
/* 	if( (tetApsIn = fopen( tmpBuf, "r" )) == NULL ) { */
/* 	    fprintf( stderr,"Warning, no plstrain.%u found... assuming plastic plugin not used.\n", rank ); */
/* 	    doAps = 0; */
/* 	} */
	sprintf( tmpBuf, "%s/plStrain.%u", readPath, rank );
 	fprintf(stderr,"Reading from %s\n",tmpBuf); 
	if( (apsIn = fopen( tmpBuf, "r" )) == NULL ) {
	    fprintf( stderr,"Warning, no plstrain.%u found... assuming plastic plugin not used.\n", rank );
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
		fprintf( stderr,"Converting dump=%d, r=%d, ts=%d, t=%g ...\n",
			 dumpIteration, rank, simTimeStep, time);
		ConvertTimeStep( rank, dumpIteration, simTimeStep, time );
		fprintf( stderr,"... done converting\n");
		break;
	    }
	    dumpIteration++;
	}

	/* Close the input files */
	if( apsIn ) {
	    fclose( apsIn );
/* 	    fclose( tetApsIn ); */
	}
	if( tempIn ) {
	    fclose( tempIn );
	}
	fclose( velIn );
	fclose( coordIn );
	fclose( stressTensorIn );
	fclose( timeStepIn );
	fclose( simIn );
	fclose( minLengthIn );
	fclose( forceIn );

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
	fprintf(stderr, "Failed to open for writing\n");
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
	fprintf(stderr, "Failed to open for writing\n");
	exit(0);
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
    sprintf( tmpBuf, "%s/snac.vel.%i.%06u.restart", writePath, rank, simTimeStep );
    fprintf(stderr,"\tWriting out %s\n",tmpBuf);
    if( (restartOut = fopen( tmpBuf, "w" )) == NULL ) {
	/* failed to open file for writing */
	fprintf(stderr, "Failed to open for writing\n");
	exit(0);
    }
    /*
     *  CPS hack:  need to try damping/zeroing of node velocities in expts where we are
     *  restarting from assumed static elastic equilibrium
     *
     *  The variable  velocityDampingFactor  should be passed from the command line
     *  BEWARE that it is fixed at zero for now.
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

    /* Write out isostatic force array */
    sprintf( tmpBuf, "%s/snac.force.%i.%06u.restart", writePath, rank, simTimeStep );
    fprintf(stderr,"\tWriting out %s\n",tmpBuf);
    if( (restartOut = fopen( tmpBuf, "w" )) == NULL ) {
	/* failed to open file for writing */
	fprintf(stderr, "Failed to open for writing\n");
	exit(0);
    }
    for( node_gI = 0; node_gI < nodeLocalCount; node_gI++ ) {
	float		force;
	fread( &force, sizeof(float), 1, forceIn );
	fprintf( restartOut, "%.9e\n", force );
    }
    if( restartOut )
	fclose( restartOut );


    /* Write out stress tensor array */
    sprintf( tmpBuf, "%s/snac.stressTensor.%i.%06u.restart", writePath, rank, simTimeStep );
    fprintf(stderr,"\tWriting out %s\n",tmpBuf);
    if( (restartOut = fopen( tmpBuf, "w" )) == NULL ) {
	/* failed to open file for writing */
	fprintf(stderr, "Failed to open for writing\n");
	exit(0);
    }
    fseek( stressTensorIn, dumpIteration * elementLocalCount * sizeof(float) * 9 * Tetrahedra_Count, SEEK_SET );
    for( element_gI = 0; element_gI < elementLocalCount; element_gI++ ) {
	for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
	    float tensor[3][3];
	    fread( &tensor, sizeof(float), 9, stressTensorIn );
	    fprintf( restartOut, "%.9e %.9e %.9e %.9e %.9e %.9e %.9e %.9e %.9e\n", 
		     tensor[0][0], tensor[0][1], tensor[0][2], 
		     tensor[1][0], tensor[1][1], tensor[1][2], 
		     tensor[2][0], tensor[2][1], tensor[2][2] );
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
	    fprintf(stderr, "Failed to open for writing\n");
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
	/* 		sprintf( tmpBuf, "%s/snac.plStrainTensor.%i.%06u.restart", writePath, rank, simTimeStep ); */
	/* 		if( (restartOut = fopen( tmpBuf, "w" )) == NULL ) { */
	/* 			);*/
	/* 		} */
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
	/* 		fseek( tetApsIn, dumpIteration * elementLocalCount * sizeof(float) * Tetrahedra_Count, SEEK_SET ); */
	/* 		for( element_gI = 0; element_gI < elementLocalCount; element_gI++ ) { */
	/* 			for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) { */
	/* 				float tetraStrain; */
	/* 				fread( &tetraStrain, sizeof(float), 1, tetApsIn ); */
	/* 				fprintf( restartOut, "%.9e\n", tetraStrain ); */
	/* 				fflush( restartOut ); */
	/* 			} */
	/* 		} */
	/* 		if( restartOut ) */
	/* 			fclose( restartOut ); */


	sprintf( tmpBuf, "%s/snac.plStrain.%i.%06u.restart", writePath, rank, simTimeStep );
	fprintf(stderr,"\tWriting out %s\n",tmpBuf);
	if( (restartOut = fopen( tmpBuf, "w" )) == NULL ) {
	    /* failed to open file for writing */
	    fprintf(stderr, "Failed to open for writing\n");
	    exit(0);
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
	sprintf( tmpBuf, "%s/snac.minLengthScale.restart", writePath );
	fprintf(stderr,"\tWriting out %s\n",tmpBuf);
	if( (restartOut = fopen( tmpBuf, "w" )) == NULL ) {
	    /* failed to open file for writing */
	    fprintf(stderr, "Failed to open for writing\n");
	    exit(0);
	}
	fscanf( minLengthIn, "%e", &minLength );
	fprintf( restartOut, "%e\n", minLength );

	if( restartOut )
	    fclose( restartOut );
    }

    return;
}

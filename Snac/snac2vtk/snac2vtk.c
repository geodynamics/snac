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
**	Converts Snac's binary output to VTK format
**
** $Id: snac2vtk.c 3270 2006-11-26 06:33:20Z EunseoChoi $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#ifndef PI
	#ifndef M_PIl
		#ifndef M_PI
			#define PI 3.14159265358979323846
		#else
			#define PI M_PI
		#endif
	#else
		#define PI M_PIl
	#endif
#endif

#include <limits.h>
#ifndef PATH_MAX
	#define PATH_MAX 1024
#endif


#define ROTATE(a,i,j,k,l) g=a[i][j];h=a[k][l];a[i][j]=g-s*(h+g*tau);\
    a[k][l]=h+s*(g-h*tau);

#define NR_END 1
#define FREE_ARG char*	
#define SIGN(b) ((b) >= 0.0 ? 1 : -1)

#define SWAP(a,b,t)     t=a;a=b;b=t; 

//#define DEBUG



void ConvertTimeStep( int rank, unsigned int dumpIteration, unsigned int simTimeStep, double time, int gnode[3], int rank_array[3], 
		      const int rankI, const int rankJ, const int rankK );

int DerivePrincipalStresses(double stressTensor[3][3],double sp[3],double cn[3][3]);

double** dmatrix(long nrl, long nrh, long ncl, long nch);
double *dvector(long nl, long nh);
int *ivector(long nl, long nh);
void free_ivector(int *v, long nl, long nh);
void free_dmatrix(double **m, long nrl, long nrh, long ncl, long nch);
void free_dvector(double *v, long nl, long nh);
void nrerror(char error_text[]);
int jacobi(double** a, double* d, double** v);
int eigsrt(double* d, double** v);
struct stressMeasures {
    double     	principalStresses[3];
    double	eigenvectors[3][3];
    double	pressure;
    double	maxShearStress;
    double	J2;
    double	vonMisesStress;
    double	failurePotential;
    double	failureAngle;
    double	slopeShearStress;
    double	slopeNormalStress;

};
void DeriveStressMeasures(FILE *stressTensorIn, 
			  double elementStressTensor[3][3], 
			  struct stressMeasures *elementStressMeasures);



char		path[PATH_MAX];
FILE*		strainRateIn;
FILE*		stressIn;
FILE*		stressTensorIn;
FILE*		hydroPressureIn;
FILE*		coordIn;
FILE*		velIn;
FILE*		forceIn;
FILE*		phaseIn;
FILE*		tempIn;
FILE*		apsIn;
FILE*		viscIn;

unsigned int	elementLocalSize[3];
int 			doTemp = 1;
int 			doForce = 1;
int 			doAps = 1;
int 			doHPr = 1;
int 			doVisc = 1;
double			failureAngle = 30.0;

int main( int argc, char* argv[]) 
{
    char		tmpBuf[PATH_MAX];
    FILE*		simIn;
    FILE*		timeStepIn;
    unsigned int	rank;
    unsigned int	simTimeStep;
    unsigned int	dumpIteration;
    double		time;
    double		dt;
    int		gnode[3];
    int		rank_array[3];
    unsigned int	rankI,rankJ,rankK;
    unsigned int	stepMin=-1,stepMax=0;
	
    if( argc<7 || argc>10 ) {
	fprintf(stderr,"snac2vtk global-mesh-size-x global-mesh-size-y global-mesh-size-z num-processors-x num-processors-y num-processors-z [start-step[max-step]] [end-step[max-step]] [failure-angle[30 (degrees)]]\n");
	exit(1);
    }

    /*
     * TODO, get from arg list 
     */
    sprintf( path, "." );
    gnode[0] = atoi(argv[1]);
    gnode[1] = atoi(argv[2]);
    gnode[2] = atoi(argv[3]);
    rank_array[0] = atoi(argv[4]);
    rank_array[1] = atoi(argv[5]);
    rank_array[2] = atoi(argv[6]);
		
    for( rankK=0; rankK < rank_array[2]; rankK++ )
	for( rankJ=0; rankJ < rank_array[1]; rankJ++ )
	    for( rankI=0; rankI < rank_array[0]; rankI++ ) {
		rank = rankI + rankJ*rank_array[0] + rankK*rank_array[0]*rank_array[1]; 

		sprintf( tmpBuf, "%s/sim.%u", path, rank );
		if( (simIn = fopen( tmpBuf, "r" )) == NULL ) {
		    if( rank == 0 ) {
			fprintf(stderr, "\"%s\" not found\n", tmpBuf );
			exit(1);
		    }
		    else {
			break;
		    }
		}
		sprintf( tmpBuf, "%s/timeStep.%u", path, rank );
		if( (timeStepIn = fopen( tmpBuf, "r" )) == NULL ) {
		    fprintf(stderr, "\"%s\" not found\n", tmpBuf );
		    exit(1);
		}
		sprintf( tmpBuf, "%s/strainRate.%u", path, rank );
		if( (strainRateIn = fopen( tmpBuf, "r" )) == NULL ) {
		    fprintf(stderr, "\"%s\" not found\n", tmpBuf );
		    exit(1);
		}
		sprintf( tmpBuf, "%s/hydroPressure.%u", path, rank );
		if( ( hydroPressureIn = fopen( tmpBuf, "r" )) == NULL ) {
		    fprintf(stderr, "\"%s\" not found\n", tmpBuf );
		    doHPr = 0;
		}
		sprintf( tmpBuf, "%s/stress.%u", path, rank );
		if( (stressIn = fopen( tmpBuf, "r" )) == NULL ) {
		    fprintf(stderr, "\"%s\" not found\n", tmpBuf );
		    exit(1);
		}
		sprintf( tmpBuf, "%s/stressTensor.%u", path, rank );
		if( (stressTensorIn = fopen( tmpBuf, "r" )) == NULL ) {
		    fprintf(stderr, "\"%s\" not found\n", tmpBuf );
		    exit(1);
		}
		sprintf( tmpBuf, "%s/coord.%u", path, rank );
		if( (coordIn = fopen( tmpBuf, "r" )) == NULL ) {
		    fprintf(stderr, "\"%s\" not found\n", tmpBuf );
		    exit(1);
		}
		sprintf( tmpBuf, "%s/vel.%u", path, rank );
		if( (velIn = fopen( tmpBuf, "r" )) == NULL ) {
		    fprintf(stderr, "\"%s\" not found\n", tmpBuf );
		    exit(1);
		}
		sprintf( tmpBuf, "%s/force.%u", path, rank );
		if( (forceIn = fopen( tmpBuf, "r" )) == NULL ) {
		    fprintf(stderr, "\"%s\" not found\n", tmpBuf );
		    doForce = 0;
		}
		sprintf( tmpBuf, "%s/phaseIndex.%u", path, rank );
		if( (phaseIn = fopen( tmpBuf, "r" )) == NULL ) {
		    fprintf(stderr, "\"%s\" not found\n", tmpBuf );
		    exit(1);
		}
		sprintf( tmpBuf, "%s/temperature.%u", path, rank );
		if( (tempIn = fopen( tmpBuf, "r" )) == NULL ) {
		    fprintf(stderr, "\"%s\" not found... assuming temperature plugin not used\n", tmpBuf );
		    doTemp = 0;
		}
		sprintf( tmpBuf, "%s/plStrain.%u", path, rank );
		if( (apsIn = fopen( tmpBuf, "r" )) == NULL ) {
		    fprintf(stderr, "\"%s\" not found... assuming plastic plugin not used\n", tmpBuf );
		    doAps = 0;
		}
		sprintf( tmpBuf, "%s/viscosity.%u", path, rank );
		if( (viscIn = fopen( tmpBuf, "r" )) == NULL ) {
		    fprintf(stderr, "\"%s\" not found... assuming Maxwell plugin not used.\n", tmpBuf );
		    doVisc = 0;
		}
		
		
		/*
		 * Read in simulation information... TODO: assumes nproc=1 
		 */
		fscanf( simIn, "%u %u %u\n", &elementLocalSize[0], &elementLocalSize[1], &elementLocalSize[2] );

		
		/* 		if( feof(timeStepIn) ) { */
		/* 			fprintf(stderr, "Time step file zero length\n" ); */
		/* 			assert(timeStepIn); */
		/* 		} else { */
		/* 		    fscanf( timeStepIn, "%16u %16lg %16lg\n", &simTimeStep, &time, &dt ); */
		/* 		} */
		while( !feof( timeStepIn ) ) {
		    fscanf( timeStepIn, "%16u %16lg %16lg\n", &simTimeStep, &time, &dt );
		    /* 			fprintf(stderr, "Time step:  %u <-> %g\n", simTimeStep, time ); */
		    if(stepMin==-1) stepMin=simTimeStep;
		    if(stepMax<simTimeStep) stepMax=simTimeStep;
		}
		if( stepMin==-1 ) {
		    fprintf(stderr, "Time step file zero length\n" );
		    exit(1);
		}
		/* 		fseek( timeStepIn, 0, SEEK_SET ); */
		rewind(timeStepIn);
		
		if(argc>=8) {
		    if(atoi(argv[7])>stepMin) stepMin=atoi(argv[7]);
		} else {
		    stepMin=stepMax;
		}
		if(argc>=9) {
		    if(atoi(argv[8])<stepMax) stepMax=atoi(argv[8]);
		}/*  else { */
		/* 		    stepMax=stepMin; */
		/* 		} */

		if (stepMax<stepMin) {
		    fprintf(stderr, "Error in time step range (start/stop reversed):  %u <-> %u\n", stepMin, stepMax );
		    exit(1);
		}
		fprintf(stderr, "Time step range:  %u <-> %u\n", stepMin, stepMax );

		/*
		 *  Parse angle used to compute failure potential - using global variable (ick)
		 */
		if(argc>=10) {
		    failureAngle=atof(argv[9]);
		    fprintf(stderr, "Failure angle = %g\n", failureAngle );
		}

		/*
		 * Read in loop information 
		 */
		dumpIteration = 0;
		while( !feof( timeStepIn ) ) {
		    fscanf( timeStepIn, "%16u %16lg %16lg\n", &simTimeStep, &time, &dt );
		    if( simTimeStep <stepMin || simTimeStep > stepMax ) {
			dumpIteration++;
			continue;
		    }
		    ConvertTimeStep( rank, dumpIteration, simTimeStep, time, gnode, rank_array, rankI, rankJ, rankK );
		    dumpIteration++;
		}
		
		/*
		 * Close the input files 
		 */
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
		if( hydroPressureIn ) {
		    fclose( hydroPressureIn );
		}
		fclose( phaseIn );
		fclose( velIn );
		fclose( coordIn );
		fclose( stressIn );
		fclose( stressTensorIn );
		fclose( strainRateIn );
		fclose( timeStepIn );
		fclose( simIn );
		
		/*
		 * Do next rank 
		 */
		rank++;
	    }

    return 0;
}


void ConvertTimeStep( 
		     int rank, 
		     unsigned int dumpIteration, 
		     unsigned int simTimeStep, 
		     double time, 
		     int gnode[3], 
		     int rank_array[3], 
		     const int rankI, 
		     const int rankJ, 
		     const int rankK 
		     ) 
{
    char		tmpBuf[PATH_MAX], tmpBuf1[PATH_MAX];
    FILE*		vtkOut;
    FILE*		vtkOut1;
    unsigned int	elementLocalCount = elementLocalSize[0] * elementLocalSize[1] * elementLocalSize[2];
    unsigned int	nodeLocalSize[3] = { elementLocalSize[0] + 1, elementLocalSize[1] + 1, elementLocalSize[2] + 1 };
    unsigned int	nodeLocalCount = nodeLocalSize[0] * nodeLocalSize[1] * nodeLocalSize[2];
    unsigned int	node_gI;
    unsigned int	element_gI;
	
    /*
     * Open the output file 
     */
    sprintf( tmpBuf, "%s/snac.%i.%06u.vts", path, rank, simTimeStep );
    if( (vtkOut = fopen( tmpBuf, "w+" )) == NULL ) {
	fprintf(stderr, "Cannot open \"%s\" for writing\n", tmpBuf );
	exit(1);
    }
	
    /*
     * Write out simulation information 
     */
    fprintf( vtkOut, "<?xml version=\"1.0\"?>\n" );
    fprintf( vtkOut, "<VTKFile type=\"StructuredGrid\"  version=\"0.1\" byte_order=\"LittleEndian\" compressor=\"vtkZLibDataCompressor\">\n");
    fprintf( vtkOut, "  <StructuredGrid WholeExtent=\"%d %d %d %d %d %d\">\n",
	     rankI*elementLocalSize[0],(rankI+1)*elementLocalSize[0],
	     rankJ*elementLocalSize[1],(rankJ+1)*elementLocalSize[1],
	     rankK*elementLocalSize[2],(rankK+1)*elementLocalSize[2]);
    fprintf( vtkOut, "    <Piece Extent=\"%d %d %d %d %d %d\">\n",
	     rankI*elementLocalSize[0],(rankI+1)*elementLocalSize[0],
	     rankJ*elementLocalSize[1],(rankJ+1)*elementLocalSize[1],
	     rankK*elementLocalSize[2],(rankK+1)*elementLocalSize[2]);
	
    /* 
     *
     *  			Start the ---node--- section 
     *
     */
    fprintf( vtkOut, "      <PointData Vectors=\"velocity\">\n");
	
    /*
     * Write out the velocity information 
     */
    fprintf( vtkOut, "        <DataArray type=\"Float32\" Name=\"velocity\" NumberOfComponents=\"3\" format=\"ascii\">\n");
    if (fseek( velIn, dumpIteration * nodeLocalCount * sizeof(float) * 3, SEEK_SET )!=0) {
	fprintf(stderr, "Cannot find read required portion of Snac velocity output file:  rank=%d: %d, %d, %d,  dump iteration=%d, node count=%d\n", 
		rank, rankI, rankJ, rankK,
		dumpIteration, nodeLocalCount );
	exit(1);
    }
    for( node_gI = 0; node_gI < nodeLocalCount; node_gI++ ) {
	float		vel[3];
	if (fread( &vel, sizeof(float), 3, velIn )==0) {
	    if (feof(velIn)) {
		fprintf(stderr, "Error (reached EOF prematurely) while reading Snac velocity output file:  rank=%d: %d, %d, %d,  dump iteration=%d, node=%d/%d\n", 
			rank, rankI, rankJ, rankK,
			dumpIteration, node_gI, nodeLocalCount );
		exit(1);
	    } else {
		fprintf(stderr, "Error while reading Snac velocity output file:  rank=%d: %d, %d, %d,  dump iteration=%d, node=%d/%d\n", 
			rank, rankI, rankJ, rankK,
			dumpIteration, node_gI, nodeLocalCount );
		exit(1);
	    }
	}	 



	/* 		fprintf(stderr, "Error (reached EOF prematurely) while reading Snac force stress tensor output file: tetrahedral element #%d\n" , tetra_I); */
   
	fprintf( vtkOut, "%g %g %g\n", vel[0], vel[1], vel[2] );
    }
    fprintf( vtkOut, "        </DataArray>\n");
	
    /*
     * Write out the force information 
     */
    if( doForce ) {
	fprintf( vtkOut, "        <DataArray type=\"Float32\" Name=\"force\" NumberOfComponents=\"3\" format=\"ascii\">\n");
	if (fseek( forceIn, dumpIteration * nodeLocalCount * sizeof(float) * 3, SEEK_SET )!=0) {
	    fprintf(stderr, "Cannot find read required portion of Snac force output file:  rank=%d: %d, %d, %d,  dump iteration=%d, node count=%d\n", 
		    rank, rankI, rankJ, rankK,
		    dumpIteration, nodeLocalCount );
	    exit(1);
	}
	for( node_gI = 0; node_gI < nodeLocalCount; node_gI++ ) {
	    float		force[3];
	    if (fread( &force, sizeof(float), 3, forceIn )==0)  {
		if (feof(forceIn)) {
		    fprintf(stderr, "Error (reached EOF prematurely) while reading Snac force output file:  rank=%d: %d, %d, %d,  dump iteration=%d, node=%d/%d\n", 
			    rank, rankI, rankJ, rankK,
			    dumpIteration, node_gI, nodeLocalCount );
		    exit(1);
		} else {
		    fprintf(stderr, "Error while reading Snac force output file:  rank=%d: %d, %d, %d,  dump iteration=%d, node=%d/%d\n", 
			    rank, rankI, rankJ, rankK,
			    dumpIteration, node_gI, nodeLocalCount );
		    exit(1);
		}
	    }
	    fprintf( vtkOut, "%g %g %g\n", force[0], force[1], force[2] );
	}
	fprintf( vtkOut, "        </DataArray>\n");
    }
	
    /*
     * Write out the temperature information 
     */
    if( doTemp ) {
	fprintf( vtkOut, "        <DataArray type=\"Float32\" Name=\"temperature\" format=\"ascii\">\n");
	if (fseek( tempIn, dumpIteration * nodeLocalCount * sizeof(float), SEEK_SET )!=0) {
	    fprintf(stderr, "Cannot find read required portion of Snac temperature output file:  rank=%d: %d, %d, %d,  dump iteration=%d, node count=%d\n", 
		    rank, rankI, rankJ, rankK,
		    dumpIteration, nodeLocalCount );
	    exit(1);
	}
	for( node_gI = 0; node_gI < nodeLocalCount; node_gI++ ) {
	    float		temperature;
	    if (fread( &temperature, sizeof(float), 1, tempIn )==0)  {
		if (feof(tempIn)) {
		    fprintf(stderr, "Error (reached EOF prematurely) while reading Snac temperature output file:  rank=%d: %d, %d, %d,  dump iteration=%d, node=%d/%d\n", 
			    rank, rankI, rankJ, rankK,
			    dumpIteration, node_gI, nodeLocalCount );
		    exit(1);
		} else {
		    fprintf(stderr, "Error while reading Snac temperature output file:  rank=%d: %d, %d, %d,  dump iteration=%d, node=%d/%d\n", 
			    rank, rankI, rankJ, rankK,
			    dumpIteration, node_gI, nodeLocalCount );
		    exit(1);
		}
	    }
	    fprintf( vtkOut, "%g ", temperature );
	}
	fprintf( vtkOut, "        </DataArray>\n");
    }
    fprintf( vtkOut, "      </PointData>\n");


    /* 
     *
     *  			Start the ---element--- section 
     *
     */
    fprintf( vtkOut, "      <CellData Scalars=\"Plastic strain\">\n");

    /*
     * Write out the plastic strain information 
     */
    if( doAps ) {
	fprintf( vtkOut, "        <DataArray type=\"Float32\" Name=\"Plastic strain\" format=\"ascii\">\n");
	if (fseek( apsIn, dumpIteration * elementLocalCount * sizeof(float), SEEK_SET )!=0) {
	    fprintf(stderr, "Cannot find read required portion of Snac plastic strain output file:  rank=%d: %d, %d, %d,  dump iteration=%d, node count=%d\n", 
		    rank, rankI, rankJ, rankK,
		    dumpIteration, nodeLocalCount );
	    exit(1);
	}
	for( element_gI = 0; element_gI < elementLocalCount; element_gI++ ) {
	    float		plStrain;
	    if (fread( &plStrain, sizeof(float), 1, apsIn )==0)  {
		if (feof(apsIn)) {
		    fprintf(stderr, "Error (reached EOF prematurely) while reading Snac plastic strain output file:  rank=%d: %d, %d, %d,  dump iteration=%d, node=%d/%d\n", 
			    rank, rankI, rankJ, rankK,
			    dumpIteration, node_gI, nodeLocalCount );
		    exit(1);
		} else {
		    fprintf(stderr, "Error while reading Snac plastic strain output file:  rank=%d: %d, %d, %d,  dump iteration=%d, node=%d/%d\n", 
			    rank, rankI, rankJ, rankK,
			    dumpIteration, node_gI, nodeLocalCount );
		    exit(1);
		}
	    }
	    fprintf( vtkOut, "%g ", plStrain );
	}
	fprintf( vtkOut, "        </DataArray>\n");
    }
	
    /*
     * Write out the strain rate information 
     */
    fprintf( vtkOut, "        <DataArray type=\"Float32\" Name=\"Strain rate\" format=\"ascii\">\n");
    if (fseek( strainRateIn, dumpIteration * elementLocalCount * sizeof(float), SEEK_SET )!=0) {
	fprintf(stderr, "Cannot find read required portion of Snac strain rate output file:  rank=%d: %d, %d, %d,  dump iteration=%d, node count=%d\n", 
		rank, rankI, rankJ, rankK,
		dumpIteration, nodeLocalCount );
	exit(1);
    }
    for( element_gI = 0; element_gI < elementLocalCount; element_gI++ ) {
	float		strainRate;
	if (fread( &strainRate, sizeof(float), 1, strainRateIn )==0)  {
	    if (feof(strainRateIn)) {
		fprintf(stderr, "Error (reached EOF prematurely) while reading Snac strain rate output file:  rank=%d: %d, %d, %d,  dump iteration=%d, node=%d/%d\n", 
			rank, rankI, rankJ, rankK,
			dumpIteration, node_gI, nodeLocalCount );
		exit(1);
	    } else {
		fprintf(stderr, "Error while reading Snac strain rate output file:  rank=%d: %d, %d, %d,  dump iteration=%d, node=%d/%d\n", 
			rank, rankI, rankJ, rankK,
			dumpIteration, node_gI, nodeLocalCount );
		exit(1);
	    }
	}
	fprintf( vtkOut, "%g ", strainRate );
    }
    fprintf( vtkOut, "        </DataArray>\n");
	
    /*
     * Write out the pressure information 
     */
    fprintf( vtkOut, "        <DataArray type=\"Float32\" Name=\"Pressure\" format=\"ascii\">\n");
    if (fseek( stressTensorIn, dumpIteration * elementLocalCount * sizeof(float)*9*10, SEEK_SET )!=0) {
	fprintf(stderr, "Cannot find read required portion of Snac stress tensor output file:  rank=%d: %d, %d, %d,  dump iteration=%d, node count=%d\n", 
		rank, rankI, rankJ, rankK,
		dumpIteration, nodeLocalCount );
	exit(1);
    }
    for( element_gI = 0; element_gI < elementLocalCount; element_gI++ ) {
	double	        	elementStressTensor[3][3]={{0,0,0},{0,0,0},{0,0,0}};
	struct stressMeasures	elementStressMeasures;
	/*
	 *  Build average stress tensor for element and derive useful stress measures
	 */
	DeriveStressMeasures(stressTensorIn, elementStressTensor, &elementStressMeasures);
	/*
	 *  Write the chosen derived stress value to Paraview file
	 */
	fprintf( vtkOut, "%g ", -elementStressMeasures.pressure );
#ifdef DEBUG
	if (element_gI<10) fprintf( stderr, "Element pressure %d: %g  at angle %g\n", element_gI, elementStressMeasures.pressure, elementStressMeasures.failureAngle); 
#endif	
    }
    fprintf( vtkOut, "        </DataArray>\n");

    /*
     * Write out the pressure information 
     */
    if( doHPr ) {
	fprintf( vtkOut, "        <DataArray type=\"Float32\" Name=\"Hydrostatic pressure\" format=\"ascii\">\n");
	if (fseek( hydroPressureIn, dumpIteration * elementLocalCount * sizeof(float), SEEK_SET )!=0) {
	    fprintf(stderr, "Cannot find read required portion of Snac pressure output file:  rank=%d: %d, %d, %d,  dump iteration=%d, node count=%d\n", 
		    rank, rankI, rankJ, rankK,
		    dumpIteration, nodeLocalCount );
	    exit(1);
	}
	for( element_gI = 0; element_gI < elementLocalCount; element_gI++ ) {
	    float		hydroPressure;
	    if (fread( &hydroPressure, sizeof(float), 1, hydroPressureIn )==0)  {
		if (feof(hydroPressureIn)) {
		    fprintf(stderr, "Error (reached EOF prematurely) while reading Snac pressure output file:  rank=%d: %d, %d, %d,  dump iteration=%d, node=%d/%d\n", 
			    rank, rankI, rankJ, rankK,
			    dumpIteration, node_gI, nodeLocalCount );
		    exit(1);
		} else {
		    fprintf(stderr, "Error while reading Snac pressure output file:  rank=%d: %d, %d, %d,  dump iteration=%d, node=%d/%d\n", 
			    rank, rankI, rankJ, rankK,
			    dumpIteration, node_gI, nodeLocalCount );
		    exit(1);
		}
	    }
	    fprintf( vtkOut, "%g ", hydroPressure );
	}
	fprintf( vtkOut, "        </DataArray>\n");
    }

    /*
     * Write out the stress information 
     */
    fprintf( vtkOut, "        <DataArray type=\"Float32\" Name=\"Tetra shear stress\" format=\"ascii\">\n");
    if (fseek( stressIn, dumpIteration * elementLocalCount * sizeof(float), SEEK_SET )!=0) {
	fprintf(stderr, "Cannot find read required portion of Snac shear stress output file:  rank=%d: %d, %d, %d,  dump iteration=%d, node count=%d\n", 
		rank, rankI, rankJ, rankK,
		dumpIteration, nodeLocalCount );
	exit(1);
    }
    for( element_gI = 0; element_gI < elementLocalCount; element_gI++ ) {
	float		stress;
	if (fread( &stress, sizeof(float), 1, stressIn )==0)  {
	    if (feof(stressIn)) {
		fprintf(stderr, "Error (reached EOF prematurely) while reading Snac shear stress output file:  rank=%d: %d, %d, %d,  dump iteration=%d, node=%d/%d\n", 
			rank, rankI, rankJ, rankK,
			dumpIteration, node_gI, nodeLocalCount );
		exit(1);
	    } else {
		fprintf(stderr, "Error while reading Snac shear stress output file:  rank=%d: %d, %d, %d,  dump iteration=%d, node=%d/%d\n", 
			rank, rankI, rankJ, rankK,
			dumpIteration, node_gI, nodeLocalCount );
		exit(1);
	    }
	}
	fprintf( vtkOut, "%g ", stress );
    }
    fprintf( vtkOut, "        </DataArray>\n");


    /*
     * Write out the maxShearStress information 
     */
    fprintf( vtkOut, "        <DataArray type=\"Float32\" Name=\"Max. shear stress\" format=\"ascii\">\n");
    if (fseek( stressTensorIn, dumpIteration * elementLocalCount * sizeof(float)*9*10, SEEK_SET )!=0) {
	fprintf(stderr, "Cannot find read required portion of Snac stress tensor output file:  rank=%d: %d, %d, %d,  dump iteration=%d, node count=%d\n", 
		rank, rankI, rankJ, rankK,
		dumpIteration, nodeLocalCount );
	exit(1);
    }
    for( element_gI = 0; element_gI < elementLocalCount; element_gI++ ) {
	double	        	elementStressTensor[3][3]={{0,0,0},{0,0,0},{0,0,0}};
	struct stressMeasures	elementStressMeasures;
	/*
	 *  Build average stress tensor for element and derive useful stress measures
	 */
	DeriveStressMeasures(stressTensorIn, 
			     elementStressTensor,  &elementStressMeasures);
	/*
	 *  Write the chosen derived stress value to Paraview file
	 */
	fprintf( vtkOut, "%g ", elementStressMeasures.maxShearStress ); 
#ifdef DEBUG
	if (element_gI<10) fprintf( stderr, "Element max shear stress %d: %g\n", element_gI, elementStressMeasures.maxShearStress ); 
#endif	
    }
    fprintf( vtkOut, "        </DataArray>\n");

    /*
     * Write out the vonMisesStress information 
     */
    fprintf( vtkOut, "        <DataArray type=\"Float32\" Name=\"Von Mises stress\" format=\"ascii\">\n");
    if (fseek( stressTensorIn, dumpIteration * elementLocalCount * sizeof(float)*9*10, SEEK_SET )!=0) {
	fprintf(stderr, "Cannot find read required portion of Snac stress tensor output file:  rank=%d: %d, %d, %d,  dump iteration=%d, node count=%d\n", 
		rank, rankI, rankJ, rankK,
		dumpIteration, nodeLocalCount );
	exit(1);
    }
    for( element_gI = 0; element_gI < elementLocalCount; element_gI++ ) {
	double	        	elementStressTensor[3][3]={{0,0,0},{0,0,0},{0,0,0}};
	struct stressMeasures	elementStressMeasures;
	/*
	 *  Build average stress tensor for element and derive useful stress measures
	 */
	DeriveStressMeasures(stressTensorIn, elementStressTensor, &elementStressMeasures);
	/*
	 *  Write the chosen derived stress value to Paraview file
	 */
	fprintf( vtkOut, "%g ", elementStressMeasures.vonMisesStress ); 
#ifdef DEBUG
	if (element_gI<10) fprintf( stderr, "Element von Mises stress %d: %g\n", element_gI, elementStressMeasures.vonMisesStress ); 
#endif	
    }
    fprintf( vtkOut, "        </DataArray>\n");

    /*
     * Write out the slopeShearStress information 
     */
    fprintf( vtkOut, "        <DataArray type=\"Float32\" Name=\"Shear stress @%gd\" format=\"ascii\">\n",failureAngle);
    if (fseek( stressTensorIn, dumpIteration * elementLocalCount * sizeof(float)*9*10, SEEK_SET )!=0) {
	fprintf(stderr, "Cannot find read required portion of Snac stress tensor output file:  rank=%d: %d, %d, %d,  dump iteration=%d, node count=%d\n", 
		rank, rankI, rankJ, rankK,
		dumpIteration, nodeLocalCount );
	exit(1);
    }
    for( element_gI = 0; element_gI < elementLocalCount; element_gI++ ) {
	double	        	elementStressTensor[3][3]={{0,0,0},{0,0,0},{0,0,0}};
	struct stressMeasures	elementStressMeasures;
	/*
	 *  Build average stress tensor for element and derive useful stress measures
	 */
	elementStressMeasures.failureAngle=(failureAngle<0.0 ? 0.0 : failureAngle);
	DeriveStressMeasures(stressTensorIn, elementStressTensor, &elementStressMeasures);
	/*
	 *  Write the chosen derived stress value to Paraview file
	 */
	fprintf( vtkOut, "%g ", elementStressMeasures.slopeShearStress );
#ifdef DEBUG
	if (element_gI<10) fprintf( stderr, "Element slope shear stress %d: %g  at angle %g\n", element_gI, elementStressMeasures.slopeShearStress, elementStressMeasures.failureAngle); 
#endif	
    }
    fprintf( vtkOut, "        </DataArray>\n");

    /*
     * Write out the slopeNormalStress information 
     */
    fprintf( vtkOut, "        <DataArray type=\"Float32\" Name=\"Normal stress @%gd\" format=\"ascii\">\n", failureAngle);
    if (fseek( stressTensorIn, dumpIteration * elementLocalCount * sizeof(float)*9*10, SEEK_SET )!=0) {
	fprintf(stderr, "Cannot find read required portion of Snac stress tensor output file:  rank=%d: %d, %d, %d,  dump iteration=%d, node count=%d\n", 
		rank, rankI, rankJ, rankK,
		dumpIteration, nodeLocalCount );
	exit(1);
    }
    for( element_gI = 0; element_gI < elementLocalCount; element_gI++ ) {
	double	        	elementStressTensor[3][3]={{0,0,0},{0,0,0},{0,0,0}};
	struct stressMeasures	elementStressMeasures;
	/*
	 *  Build average stress tensor for element and derive useful stress measures
	 */
	elementStressMeasures.failureAngle=(failureAngle<0.0 ? 0.0 : failureAngle);
	DeriveStressMeasures(stressTensorIn, elementStressTensor, &elementStressMeasures);
	/*
	 *  Write the chosen derived stress value to Paraview file
	 */
	fprintf( vtkOut, "%g ", elementStressMeasures.slopeNormalStress );
#ifdef DEBUG
	if (element_gI<10) fprintf( stderr, "Element slope normal stress %d: %g  at angle %g\n", element_gI, elementStressMeasures.slopeNormalStress, elementStressMeasures.failureAngle); 
#endif	
    }
    fprintf( vtkOut, "        </DataArray>\n");

    /* Write out the failurePotential information */
    fprintf( vtkOut, "        <DataArray type=\"Float32\" Name=\"Failure potential\" format=\"ascii\">\n");
    if (fseek( stressTensorIn, dumpIteration * elementLocalCount * sizeof(float)*9*10, SEEK_SET )!=0) {
	fprintf(stderr, "Cannot find read required portion of Snac stress tensor output file:  rank=%d: %d, %d, %d,  dump iteration=%d, node count=%d\n", 
		rank, rankI, rankJ, rankK,
		dumpIteration, nodeLocalCount );
	exit(1);
    }
    for( element_gI = 0; element_gI < elementLocalCount; element_gI++ ) {
	double	        	elementStressTensor[3][3]={{0,0,0},{0,0,0},{0,0,0}};
	struct stressMeasures	elementStressMeasures;
	/*
	 *  Build average stress tensor for element and derive useful stress measures
	 */
	elementStressMeasures.failureAngle=(failureAngle<0.0 ? 0.0 : failureAngle);
	DeriveStressMeasures(stressTensorIn, elementStressTensor, &elementStressMeasures);
	/*
	 *  Write the chosen derived stress value to Paraview file
	 */
	fprintf( vtkOut, "%g ", elementStressMeasures.failurePotential );
#ifdef DEBUG
	if (element_gI<10) fprintf( stderr, "Element failure potential %d: %g  at angle %g\n", element_gI, elementStressMeasures.failurePotential, elementStressMeasures.failureAngle); 
#endif	
    }
    fprintf( vtkOut, "        </DataArray>\n");






    /*
     * Write out the phase information 
     */
    fprintf( vtkOut, "        <DataArray type=\"Float32\" Name=\"Phase\" format=\"ascii\">\n");
    if (fseek( phaseIn, dumpIteration * elementLocalCount * sizeof(unsigned int), SEEK_SET )!=0) {
	fprintf(stderr, "Cannot find read required portion of Snac phase index output file:  rank=%d: %d, %d, %d,  dump iteration=%d, node count=%d\n", 
		rank, rankI, rankJ, rankK,
		dumpIteration, nodeLocalCount );
	exit(1);
    }
    for( element_gI = 0; element_gI < elementLocalCount; element_gI++ ) {
	unsigned int		phaseIndex;
	if (fread( &phaseIndex, sizeof(unsigned int), 1, phaseIn )==0)  {
	    if (feof(phaseIn)) {
		fprintf(stderr, "Error (reached EOF prematurely) while reading Snac phase index output file:  rank=%d: %d, %d, %d,  dump iteration=%d, node=%d/%d\n", 
			rank, rankI, rankJ, rankK,
			dumpIteration, node_gI, nodeLocalCount );
		exit(1);
	    } else {
		fprintf(stderr, "Error while reading Snac phase index output file:  rank=%d: %d, %d, %d,  dump iteration=%d, node=%d/%d\n", 
			rank, rankI, rankJ, rankK,
			dumpIteration, node_gI, nodeLocalCount );
		exit(1);
	    }
	}
	fprintf( vtkOut, "%u ", phaseIndex );
    }
    fprintf( vtkOut, "        </DataArray>\n");


    /*
     * Write out the viscosity information 
     */
    if( doVisc ) {
	fprintf( vtkOut, "        <DataArray type=\"Float32\" Name=\"Viscosity\" format=\"ascii\">\n");
	if (fseek( viscIn, dumpIteration * elementLocalCount * sizeof(float), SEEK_SET )!=0) {
	    fprintf(stderr, "Cannot find read required portion of Snac viscosity output file:  rank=%d: %d, %d, %d,  dump iteration=%d, node count=%d\n", 
		    rank, rankI, rankJ, rankK,
		    dumpIteration, nodeLocalCount );
	    exit(1);
	}
	for( element_gI = 0; element_gI < elementLocalCount; element_gI++ ) {
	    float		viscosity;
	    if (fread( &viscosity, sizeof(float), 1, viscIn )==0)  {
		if (feof(viscIn)) {
		    fprintf(stderr, "Error (reached EOF prematurely) while reading Snac viscosity output file:  rank=%d: %d, %d, %d,  dump iteration=%d, node=%d/%d\n", 
			    rank, rankI, rankJ, rankK,
			    dumpIteration, node_gI, nodeLocalCount );
		    exit(1);
		} else {
		    fprintf(stderr, "Error while reading Snac viscosity output file:  rank=%d: %d, %d, %d,  dump iteration=%d, node=%d/%d\n", 
			    rank, rankI, rankJ, rankK,
			    dumpIteration, node_gI, nodeLocalCount );
		    exit(1);
		}
	    }
	    fprintf( vtkOut, "%g ", viscosity );
	}
	fprintf( vtkOut, "        </DataArray>\n");
    }
    fprintf( vtkOut, "      </CellData>\n");
	



    /* 
     *
     *  			Write out coordinates
     *
     */
    fprintf( vtkOut, "      <Points>\n");
    fprintf( vtkOut, "        <DataArray type=\"Float32\" NumberOfComponents=\"3\" format=\"ascii\">\n");
    if (fseek( coordIn, dumpIteration * nodeLocalCount * sizeof(float) * 3, SEEK_SET )!=0) {
	fprintf(stderr, "Cannot find read required portion of Snac coordinates output file:  rank=%d: %d, %d, %d,  dump iteration=%d, node count=%d\n", 
		rank, rankI, rankJ, rankK,
		dumpIteration, nodeLocalCount );
	exit(1);
    }
    for( node_gI = 0; node_gI < nodeLocalCount; node_gI++ ) {
	float		coord[3];
	if (fread( &coord, sizeof(float), 3, coordIn )==0)  {
	    if (feof(coordIn)) {
		fprintf(stderr, "Error (reached EOF prematurely) while reading Snac coordinates output file:  rank=%d: %d, %d, %d,  dump iteration=%d, node=%d/%d\n", 
			rank, rankI, rankJ, rankK,
			dumpIteration, node_gI, nodeLocalCount );
		exit(1);
	    } else {
		fprintf(stderr, "Error while reading Snac coordinates output file:  rank=%d: %d, %d, %d,  dump iteration=%d, node=%d/%d\n", 
			rank, rankI, rankJ, rankK,
			dumpIteration, node_gI, nodeLocalCount );
		exit(1);
	    }
	}	 
	fprintf( vtkOut, "%g %g %g\n", coord[0], coord[1], coord[2] );
    }
    fprintf( vtkOut, "        </DataArray>\n");
    fprintf( vtkOut, "      </Points>\n");
    fprintf( vtkOut, "    </Piece>\n");
    fprintf( vtkOut, "  </StructuredGrid>\n");
    fprintf( vtkOut, "</VTKFile>\n");



    /*
     * Close the vtk output file 
     */
    fclose( vtkOut );








    /*
     * Write out Parallel VTS file. Only once when rank == 0. 
     */
    if( rank == 0 ) {
	int rankII, rankJJ, rankKK, rank2;

	sprintf( tmpBuf1, "%s/snac.%06u.pvts", path, simTimeStep );
	if( (vtkOut1 = fopen( tmpBuf1, "w" )) == NULL ) {
	    fprintf(stderr, "Cannot open \"%s\" for writing\n", tmpBuf );
	    exit(1);
	}
	fprintf(stderr,"Writing file %s...\n",tmpBuf1);

	fprintf( vtkOut1, "<?xml version=\"1.0\"?>\n" );
	fprintf( vtkOut1, "<VTKFile type= \"PStructuredGrid\"  version= \"0.1\" byte_order=\"LittleEndian\" compressor=\"vtkZLibDataCompressor\">\n");
	fprintf( vtkOut1, "  <PStructuredGrid WholeExtent=\"0 %d 0 %d 0 %d\" GhostLevel=\"0\">\n",gnode[0]-1,gnode[1]-1,gnode[2]-1);

	/*
	 * Start the node section 
	 */
	fprintf( vtkOut1, "    <PPointData>\n");
	fprintf( vtkOut1, "        <PDataArray type=\"Float32\" Name=\"Velocity\" NumberOfComponents=\"3\"/>\n");
	if( doForce )
	    fprintf( vtkOut1, "        <PDataArray type=\"Float32\" Name=\"Force\" NumberOfComponents=\"3\"/>\n");
	if( doTemp )
	    fprintf( vtkOut1, "        <PDataArray type=\"Float32\" Name=\"Temperature\"/>\n");
	fprintf( vtkOut1, "    </PPointData>\n");

	/*
	 * Start the element section 
	 */
	fprintf( vtkOut1, "    <PCellData>\n");
	if( doAps )
	    fprintf( vtkOut1, "        <PDataArray type=\"Float32\" Name=\"Plastic strain\"/>\n");
	fprintf( vtkOut1, "        <PDataArray type=\"Float32\" Name=\"Strain rate\"/>\n");
	fprintf( vtkOut1, "        <PDataArray type=\"Float32\" Name=\"Pressure\"/>\n");
	if( doHPr )
	    fprintf( vtkOut1, "        <PDataArray type=\"Float32\" Name=\"Hydrostatic pressure\"/>\n");
	fprintf( vtkOut1, "        <PDataArray type=\"Float32\" Name=\"Tetra shear stress\"/>\n");
	fprintf( vtkOut1, "        <PDataArray type=\"Float32\" Name=\"Max. shear stress\"/>\n");
	fprintf( vtkOut1, "        <PDataArray type=\"Float32\" Name=\"Von Mises stress\"/>\n");
	fprintf( vtkOut1, "        <PDataArray type=\"Float32\" Name=\"Shear stress @%gd\"/>\n",failureAngle);
	fprintf( vtkOut1, "        <PDataArray type=\"Float32\" Name=\"Normal stress @%gd\"/>\n",failureAngle);
	fprintf( vtkOut1, "        <PDataArray type=\"Float32\" Name=\"Failure potential @%gd\"/>\n",failureAngle);

	if( doVisc )
	    fprintf( vtkOut1, "        <PDataArray type=\"Float32\" Name=\"Viscosity\"/>\n");
	fprintf( vtkOut1, "    </PCellData>\n");
	
	/*
	 * Write out coordinates. 
	 */
	fprintf( vtkOut1, "    <PPoints>\n");
	fprintf( vtkOut1, "      <PDataArray type=\"Float32\" NumberOfComponents=\"3\"/>\n");
	fprintf( vtkOut1, "    </PPoints>\n");

	/*
	 * Write pieces that actually contains data.
	 */
	for( rankII=0; rankII < rank_array[0]; rankII++ )
	    for( rankJJ=0; rankJJ < rank_array[1]; rankJJ++ )
		for( rankKK=0; rankKK < rank_array[2]; rankKK++ ) {
		    rank2 = rankII + rankJJ*rank_array[0] + rankKK*rank_array[0]*rank_array[1]; 
		    fprintf( vtkOut1, "    <Piece Extent=\"%d %d %d %d %d %d\" Source=\"%s/snac.%d.%06u.vts\"/>\n",
			     rankII*elementLocalSize[0],(rankII+1)*elementLocalSize[0],
			     rankJJ*elementLocalSize[1],(rankJJ+1)*elementLocalSize[1],
			     rankKK*elementLocalSize[2],(rankKK+1)*elementLocalSize[2],
			     path, rank2, simTimeStep );
		}
	
	fprintf( vtkOut1, "  </PStructuredGrid>\n");
	fprintf( vtkOut1, "</VTKFile>\n");
	/*
	 * Close the output file. 
	 */
	fclose( vtkOut1 );
    }
}







int DerivePrincipalStresses(double stressTensor[3][3], double sp[3], double cn[3][3])
{

    double **a,**v,*d;
    int i,j;

    a = dmatrix(1,3,1,3);
    v = dmatrix(1,3,1,3);
    d = dvector(1,3);

    a[1][1] = stressTensor[0][0];
    a[2][2] = stressTensor[1][1];
    a[3][3] = stressTensor[2][2];
    a[1][2] = stressTensor[0][1];
    a[1][3] = stressTensor[0][2];
    a[2][3] = stressTensor[1][2];
    a[2][1] = a[1][2];
    a[3][1] = a[1][3];
    a[3][2] = a[2][3];

    jacobi(a,d,v);

    d[1] *= -1.0f;
    d[2] *= -1.0f;
    d[3] *= -1.0f;

    eigsrt(d,v);

    d[1] *= -1.0f;
    d[2] *= -1.0f;
    d[3] *= -1.0f;

    for(i=0;i<3;i++) {
	sp[i] = d[i+1];
	for(j=0;j<3;j++) {
	    cn[i][j] = v[j+1][i+1];
	}
    }

    free_dmatrix(a,1,3,1,3);
    free_dmatrix(v,1,3,1,3);
    free_dvector(d,1,3);

    return(1);
}

int jacobi(double** a, double* d, double** v)
{

    int nrot = 0;
    const unsigned int nmax = 100, n = 3;
    double b[nmax], z[nmax], tresh,sm,g,h,t,theta,c,s,tau;

    int i,j,ip,iq;

    for(ip=1;ip<=n;ip++) {
	for(iq=1;iq<=n;iq++) v[ip][iq] = 0.0f;
	v[ip][ip] = 1.0f;
    }

    for(ip=1;ip<=n;ip++) {
	b[ip] = d[ip] = a[ip][ip];
	z[ip] = 0.0f;
    }

    for(i=1;i<=50;i++) {
	sm = 0.0f;
	for(ip=1;ip<=n-1;ip++) {
	    for(iq=ip+1;iq<=n;iq++)
		sm += fabs(a[ip][iq]);
	}
	if(sm == 0.0f)
	    return(0);

	if(i < 4) {
	    tresh = 0.2f * sm / ( n*n );
	}
	else {
	    tresh = 0.0f;
	}

	for(ip=1;ip<=n-1;ip++) {
	    for(iq=ip+1;iq<=n;iq++) {
		g = 100.0f*fabs(a[ip][iq]);
		if( (i > 4) && (double)(fabs(d[ip])+g) == (double)fabs(d[ip]) && (double)(fabs(d[iq])+g) == (double)fabs(d[iq]))
		    a[ip][iq] = 0.0f;
		else if( fabs(a[ip][iq]) > tresh ) {
		    h = d[iq] - d[ip];
		    if( (fabs(h)+g) == fabs(h) )
			t = a[ip][iq] / h;
		    else {
			theta = 0.5f * h / (a[ip][iq]);
			t = 1.0f / (fabs(theta) + sqrt(1.0f + theta*theta));
			if(theta < 0.0f) t *= -1.0f;
		    }
		    c = 1.0f / sqrt(1.0f + t*t);
		    s = t * c;
		    tau = s / (1.0f + c);
		    h = t * a[ip][iq];
		    z[ip] -= h;
		    z[iq] += h;
		    d[ip] -= h;
		    d[iq] += h;
		    a[ip][iq] = 0.0f;
		    for(j=1;j<=ip-1;j++) {
			ROTATE(a,j,ip,j,iq);
		    }
		    for(j=ip+1;j<=iq-1;j++) {
			ROTATE(a,ip,j,j,iq);
		    }
		    for(j=iq+1;j<=n;j++) {
			ROTATE(a,ip,j,iq,j);
		    }
		    for(j=1;j<=n;j++) {
			ROTATE(v,j,ip,j,iq);
		    }

		    ++nrot;
		}
	    }
	}
	for(ip=1;ip<=n;ip++) {
	    b[ip] += z[ip];
	    d[ip] = b[ip];
	    z[ip] = 0.0f;
	}
    }
    assert(i<50);

    return 1;
}


int eigsrt(double* d, double** v)
{

	const unsigned int n = 3;
	int i,j,k;
	double p;

	for(i=1;i<n;i++) {
		k = i;
		p = d[i];

		for(j=i+1;j<=n;j++) {
			if(d[j] >= p) {
				k = j;
				p = d[j];
			}
		}
		if(k != i) {
			d[k] = d[i];
			d[i] = p;
			for(j=1;j<=n;j++) {
				p = v[j][i];
				v[j][i] = v[j][k];
				v[j][k] = p;
			}
		}
	}
	return(0);
}

double **dmatrix(long nrl, long nrh, long ncl, long nch)
	/* allocate a double matrix with subscript range m[nrl..nrh][ncl..nch] */
{
    long i, nrow=nrh-nrl+1,ncol=nch-ncl+1;
    double **m;

    /* allocate pointers to rows */
    m=(double **) malloc((size_t)((nrow+NR_END)*sizeof(double*)));
    if (!m) nrerror("allocation failure 1 in matrix()");
    m += NR_END;
    m -= nrl;
    /* allocate rows and set pointers to them */
    m[nrl]=(double *) malloc((size_t)((nrow*ncol+NR_END)*sizeof(double)));
    if (!m[nrl]) nrerror("allocation failure 2 in matrix()");
    m[nrl] += NR_END;
    m[nrl] -= ncl;
    for(i=nrl+1;i<=nrh;i++) m[i]=m[i-1]+ncol;
    /* return pointer to array of pointers to rows */
    return m;
}


void free_dmatrix(double **m, long nrl, long nrh, long ncl, long nch)
	/* free a double matrix allocated by dmatrix() */
{
    free((FREE_ARG) (m[nrl]+ncl-NR_END));
    free((FREE_ARG) (m+nrl-NR_END));
}


void nrerror(char error_text[])
{
    fprintf(stderr,"Run-time error...\n");
    fprintf(stderr,"%s\n",error_text);
    assert(1);
}


double *dvector(long nl, long nh)
	/* allocate a double vector with subscript range v[nl..nh] */
{
    double *v;
    v=(double *)malloc((size_t) ((nh-nl+1+NR_END)*sizeof(double)));
    if (!v) nrerror("allocation failure in dvector()");
    return v-nl+NR_END;
}

int *ivector(long nl, long nh)
	/* allocate an int vector with subscript range v[nl..nh] */
{
    int *v;
    v=(int *)malloc((size_t) ((nh-nl+1+NR_END)*sizeof(int)));
    if (!v) nrerror("allocation failure in ivector()");
    return v-nl+NR_END;
}

void free_dvector(double *v, long nl, long nh)
	/* free a double vector allocated with dvector() */
{
    free((FREE_ARG) (v+nl-NR_END));
}

void free_ivector(int *v, long nl, long nh)
	/* free an int vector allocated with ivector() */
{
    free((FREE_ARG) (v+nl-NR_END));
}



/*
 *----------------------------------------------------------------------
 *
 * DeriveStressMeasures --
 *
 *      Process tetrahedral stress tensors into element stress measures
 *
 * Returns:
 *      Void
 *
 * Side effects:
 *      Puts stress measures in variables passed to it
 *
 *----------------------------------------------------------------------
 */
void 
DeriveStressMeasures(FILE *stressTensorIn, double elementStressTensor[3][3], struct stressMeasures *elementStressMeasures)
{
    int			tetra_I;
    const int		numberTetrahedra=10;
    double		failureAngleRadians=elementStressMeasures->failureAngle*M_PI/180.0;
    double		normalVector[3],slopeParallelVector[3],tractionVector[3];
    double		tmp;

    /*
     *  Read all tetrahedral stress tensors for this element gI
     */
    for( tetra_I = 0; tetra_I < 10; tetra_I++ ) {
	float	stressTensorArray[3][3];
	if ( fread( stressTensorArray, sizeof(float), 9, stressTensorIn )==0 )  {
	    if (feof(stressTensorIn)) {
		fprintf(stderr, "Error (reached EOF prematurely) while reading Snac stress tensor output file: tetrahedral element #%d\n" , tetra_I);
		exit(1);
	    } else {
		fprintf(stderr, "Error while reading Snac stress tensor output file: tetrahedral element #%d\n" , tetra_I);
		exit(1);
	    }
	}
	/*
	 *  Report error and bail if we pick up NaNs in any of the stress components
	 */
	if(isnan(stressTensorArray[0][0]) || isnan(stressTensorArray[1][1]) 
	   || isnan(stressTensorArray[2][2]) || isnan(stressTensorArray[0][1]) 
	   || isnan(stressTensorArray[0][2]) || isnan(stressTensorArray[1][2])) 
	    fprintf(stderr,"NaN in stress tensor file\n");
	/*
	 *  Build average stress tensor for element by summing tetrahedral tensor components
	 *   - even though it's symmetric, do for all 9 components in case we pick the wrong ones before diagonalization
	 */
	elementStressTensor[0][0]+=stressTensorArray[0][0]/(double)numberTetrahedra;
	elementStressTensor[1][1]+=stressTensorArray[1][1]/(double)numberTetrahedra;
	elementStressTensor[2][2]+=stressTensorArray[2][2]/(double)numberTetrahedra;
	elementStressTensor[0][1]+=stressTensorArray[0][1]/(double)numberTetrahedra;
	elementStressTensor[0][2]+=stressTensorArray[0][2]/(double)numberTetrahedra;
	elementStressTensor[1][2]+=stressTensorArray[1][2]/(double)numberTetrahedra;
	elementStressTensor[1][0]+=stressTensorArray[1][0]/(double)numberTetrahedra;
	elementStressTensor[2][0]+=stressTensorArray[2][0]/(double)numberTetrahedra;
	elementStressTensor[2][1]+=stressTensorArray[2][1]/(double)numberTetrahedra;
    }
    /*
     *  Diagonalize and find principal stresses from mean stress tensor for element
     */
    DerivePrincipalStresses(elementStressTensor,elementStressMeasures->principalStresses,elementStressMeasures->eigenvectors);
    SWAP (elementStressMeasures->principalStresses[0], elementStressMeasures->principalStresses[2], tmp);  /*  Put sigma1 and sigma3 in order */
    /*
     *  Calculate pressure as sum sigma1+sigma2+sigma3
     */
    elementStressMeasures->pressure = (elementStressMeasures->principalStresses[0]
				       +elementStressMeasures->principalStresses[1]
				       +elementStressMeasures->principalStresses[2])/3.0; 
    /*
     *  Calculate maximum shear stress sigma1-sigma3
     */
    if(elementStressMeasures->principalStresses[2]>elementStressMeasures->principalStresses[0]) fprintf(stderr,"[2]>[0]\n");
    elementStressMeasures->maxShearStress = (elementStressMeasures->principalStresses[0]-elementStressMeasures->principalStresses[2]);
    /*
     *  Calculate 2nd deviatoric stress invariant
     */
    elementStressMeasures->J2 = 
	(pow((elementStressMeasures->principalStresses[0]-elementStressMeasures->principalStresses[1]),2.0)
	 +pow((elementStressMeasures->principalStresses[1]-elementStressMeasures->principalStresses[2]),2.0)
	 +pow((elementStressMeasures->principalStresses[2]-elementStressMeasures->principalStresses[0]),2.0) )/6.0;
    /*
     *  Calculate the useful form of J2, the von Mises stress
     */
    elementStressMeasures->vonMisesStress = sqrt(3.0*elementStressMeasures->J2);
    /*
     *  Calculate the traction vector on hillslope and related shear and normal stresses on that dipping surface (down to left)
     */
    normalVector[0] = -sin(failureAngleRadians);
    normalVector[1] = cos(failureAngleRadians);
    normalVector[2] = 0.0;
    slopeParallelVector[0] = -cos(failureAngleRadians);
    slopeParallelVector[1] = -sin(failureAngleRadians);
    slopeParallelVector[2] = 0.0;
    tractionVector[0] = ( elementStressTensor[0][0]*normalVector[0] + elementStressTensor[1][0]*normalVector[1] + elementStressTensor[2][0]*normalVector[2] );
    tractionVector[1] = ( elementStressTensor[0][1]*normalVector[0] + elementStressTensor[1][1]*normalVector[1] + elementStressTensor[2][1]*normalVector[2] );
    tractionVector[2] = ( elementStressTensor[0][2]*normalVector[0] + elementStressTensor[1][2]*normalVector[1] + elementStressTensor[2][2]*normalVector[2] );
    elementStressMeasures->slopeShearStress 
    	= tractionVector[0]*slopeParallelVector[0] + tractionVector[1]*slopeParallelVector[1] + tractionVector[2]*slopeParallelVector[2];
    elementStressMeasures->slopeNormalStress 
    	= tractionVector[0]*normalVector[0] + tractionVector[1]*normalVector[1] + tractionVector[2]*normalVector[2];

    /*
     *  Calculate the failure potential for hillslope angle
     */
    /*     elementStressMeasures->failurePotential= ( (fabs(elementStressMeasures->maxShearStress)-1e6) */
    /* 					       /(-2*elementStressMeasures->pessure/3.0) ); */

    /*
     * If slopeNormalStress is 0, failurePotential is not defined. Assign some indicative value: -1 for now. 
     * If the computed slopeNormalStress is infinite, assign -1 again. 
     * In either case, a warning message would be desirable. Or a switch to turn off failure potential calculations might be better. 
     * -EChoi 2009/03/03 
     */
    if( elementStressMeasures->slopeNormalStress == 0.0 )
	elementStressMeasures->failurePotential = -1.0; 
    else {
	elementStressMeasures->failurePotential= fabs(-elementStressMeasures->slopeShearStress/elementStressMeasures->slopeNormalStress);
	if( isinf( elementStressMeasures->failurePotential ) )
	    elementStressMeasures->failurePotential = -1.0;
    }

}

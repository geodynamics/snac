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


void ConvertTimeStep( int rank, unsigned int dumpIteration, unsigned int simTimeStep, double time, int gnode[3], int rank_array[3], const int rankI, const int rankJ, const int rankK );

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

unsigned int	elementLocalSize[3];
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
	int		gnode[3];
	int		rank_array[3];
	unsigned int	rankI,rankJ,rankK;
	
	if( argc != 7 && argc != 9 ) {
		fprintf(stderr,"USAGE: snac2vtk gnodex gnodey gnodez nprocx nprocy nprocz [time1 time2]\n");
		exit(1);
	}

	/* TODO, get from arg list */
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
		fscanf( simIn, "%u %u %u\n", &elementLocalSize[0], &elementLocalSize[1], &elementLocalSize[2] );

		/* Read in loop information */
		dumpIteration = 0;
		while( !feof( timeStepIn ) ) {
			fscanf( timeStepIn, "%16u %16lg %16lg\n", &simTimeStep, &time, &dt );
			if( argc == 9 )
				if( simTimeStep < atoi(argv[7]) || simTimeStep > atoi(argv[8]) ) {
					dumpIteration++;
					continue;
				}
			ConvertTimeStep( rank, dumpIteration, simTimeStep, time, gnode, rank_array, rankI, rankJ, rankK );
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


void ConvertTimeStep( int rank, unsigned int dumpIteration, unsigned int simTimeStep, double time, int gnode[3], int rank_array[3], const int rankI, const int rankJ, const int rankK ) {
	char		tmpBuf[PATH_MAX], tmpBuf1[PATH_MAX];
	FILE*		vtkOut;
	FILE*		vtkOut1;
	unsigned int	elementLocalCount = elementLocalSize[0] * elementLocalSize[1] * elementLocalSize[2];
	unsigned int	nodeLocalSize[3] = { elementLocalSize[0] + 1, elementLocalSize[1] + 1, elementLocalSize[2] + 1 };
	unsigned int	nodeLocalCount = nodeLocalSize[0] * nodeLocalSize[1] * nodeLocalSize[2];
	unsigned int	node_gI;
	unsigned int	element_gI;
	
	/* open the output file */
	sprintf( tmpBuf, "%s/snac.%i.%06u.vts", path, rank, simTimeStep );
	if( (vtkOut = fopen( tmpBuf, "w+" )) == NULL ) {
		assert( vtkOut /* failed to open file for writing */ );
	}
	
	/* Write out simulation information */
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
	
	/* Start the node section */
	fprintf( vtkOut, "      <PointData Vectors=\"velocity\">\n");
	
	/* Write out the velocity information */
	fprintf( vtkOut, "        <DataArray type=\"Float32\" Name=\"velocity\" NumberOfComponents=\"3\" format=\"ascii\">\n");
	fseek( velIn, dumpIteration * nodeLocalCount * sizeof(float) * 3, SEEK_SET );
	for( node_gI = 0; node_gI < nodeLocalCount; node_gI++ ) {
		float		vel[3];
		fread( &vel, sizeof(float), 3, velIn );
		fprintf( vtkOut, "%g %g %g\n", vel[0], vel[1], vel[2] );
	}
	fprintf( vtkOut, "        </DataArray>\n");
	
	/* Write out the force information */
	if( doForce ) {
		fprintf( vtkOut, "        <DataArray type=\"Float32\" Name=\"force\" NumberOfComponents=\"3\" format=\"ascii\">\n");
		fseek( forceIn, dumpIteration * nodeLocalCount * sizeof(float) * 3, SEEK_SET );
		for( node_gI = 0; node_gI < nodeLocalCount; node_gI++ ) {
			float		force[3];
			fread( &force, sizeof(float), 3, forceIn );
			fprintf( vtkOut, "%g %g %g\n", force[0], force[1], force[2] );
		}
		fprintf( vtkOut, "        </DataArray>\n");
	}
	
	/* Write out the temperature information */
	if( doTemp ) {
		fprintf( vtkOut, "        <DataArray type=\"Float32\" Name=\"temperature\" format=\"ascii\">\n");
		fseek( tempIn, dumpIteration * nodeLocalCount * sizeof(float), SEEK_SET );
		for( node_gI = 0; node_gI < nodeLocalCount; node_gI++ ) {
			float		temperature;
			fread( &temperature, sizeof(float), 1, tempIn );
			fprintf( vtkOut, "%g ", temperature );
		}
		fprintf( vtkOut, "        </DataArray>\n");
	}
	fprintf( vtkOut, "      </PointData>\n");

	/* Start the element section */
	fprintf( vtkOut, "      <CellData Scalars=\"strainRate\">\n");
	
	/* Write out the strain rate information */
	fprintf( vtkOut, "        <DataArray type=\"Float32\" Name=\"strainRate\" format=\"ascii\">\n");
	fseek( strainRateIn, dumpIteration * elementLocalCount * sizeof(float), SEEK_SET );
	for( element_gI = 0; element_gI < elementLocalCount; element_gI++ ) {
		float		strainRate;
		fread( &strainRate, sizeof(float), 1, strainRateIn );
		fprintf( vtkOut, "%g ", strainRate );
	}
	fprintf( vtkOut, "        </DataArray>\n");
	
	/* Write out the stress information */
	fprintf( vtkOut, "        <DataArray type=\"Float32\" Name=\"stress\" format=\"ascii\">\n");
	fseek( stressIn, dumpIteration * elementLocalCount * sizeof(float), SEEK_SET );
	for( element_gI = 0; element_gI < elementLocalCount; element_gI++ ) {
		float		stress;
		fread( &stress, sizeof(float), 1, stressIn );
		fprintf( vtkOut, "%g ", stress );
	}
	fprintf( vtkOut, "        </DataArray>\n");
	
	/* Write out the phase information */
	fprintf( vtkOut, "        <DataArray type=\"Float32\" Name=\"phase\" format=\"ascii\">\n");
	fseek( phaseIn, dumpIteration * elementLocalCount * sizeof(unsigned int), SEEK_SET );
	for( element_gI = 0; element_gI < elementLocalCount; element_gI++ ) {
		unsigned int		phaseIndex;
		fread( &phaseIndex, sizeof(unsigned int), 1, phaseIn );
		fprintf( vtkOut, "%u ", phaseIndex );
	}
	fprintf( vtkOut, "        </DataArray>\n");

	/* Write out the plastic strain information */
	if( doAps ) {
		fprintf( vtkOut, "        <DataArray type=\"Float32\" Name=\"plStrain\" format=\"ascii\">\n");
		fseek( apsIn, dumpIteration * elementLocalCount * sizeof(float), SEEK_SET );
		for( element_gI = 0; element_gI < elementLocalCount; element_gI++ ) {
			float		plStrain;
			fread( &plStrain, sizeof(float), 1, apsIn );
			fprintf( vtkOut, "%g ", plStrain );
		}
		fprintf( vtkOut, "        </DataArray>\n");
	}

	/* Write out the pressure information */
	if( doHPr ) {
		fprintf( vtkOut, "        <DataArray type=\"Float32\" Name=\"pressure\" format=\"ascii\">\n");
		fseek( pisosIn, dumpIteration * elementLocalCount * sizeof(float), SEEK_SET );
		for( element_gI = 0; element_gI < elementLocalCount; element_gI++ ) {
			float		pressure;
			fread( &pressure, sizeof(float), 1, pisosIn );
			fprintf( vtkOut, "%g ", pressure );
		}
		fprintf( vtkOut, "        </DataArray>\n");
	}

	/* Write out the pressure information */
	if( doVisc ) {
		fprintf( vtkOut, "        <DataArray type=\"Float32\" Name=\"viscosity\" format=\"ascii\">\n");
		fseek( viscIn, dumpIteration * elementLocalCount * sizeof(float), SEEK_SET );
		for( element_gI = 0; element_gI < elementLocalCount; element_gI++ ) {
			float		viscosity;
			fread( &viscosity, sizeof(float), 1, viscIn );
			fprintf( vtkOut, "%g ", viscosity );
		}
		fprintf( vtkOut, "        </DataArray>\n");
	}
	fprintf( vtkOut, "      </CellData>\n");
	
	/* Write out coordinates. */
	fprintf( vtkOut, "      <Points>\n");
	fprintf( vtkOut, "        <DataArray type=\"Float32\" NumberOfComponents=\"3\" format=\"ascii\">\n");
	fseek( coordIn, dumpIteration * nodeLocalCount * sizeof(float) * 3, SEEK_SET );
	for( node_gI = 0; node_gI < nodeLocalCount; node_gI++ ) {
		float		coord[3];
		fread( &coord, sizeof(float), 3, coordIn );
		fprintf( vtkOut, "%g %g %g\n", coord[0], coord[1], coord[2] );
	}
	fprintf( vtkOut, "        </DataArray>\n");
	fprintf( vtkOut, "      </Points>\n");
	fprintf( vtkOut, "    </Piece>\n");
	fprintf( vtkOut, "  </StructuredGrid>\n");
	fprintf( vtkOut, "</VTKFile>\n");

	/* Close the output file */
	fclose( vtkOut );

	/* Write out Parallel VTS file. Only once when rank == 0. */
	if( rank == 0 ) {
		int rankII, rankJJ, rankKK, rank2;

		sprintf( tmpBuf1, "%s/snac.%06u.pvts", path, simTimeStep );
		if( (vtkOut1 = fopen( tmpBuf1, "w" )) == NULL ) {
			assert( vtkOut1 /* failed to open file for writing */ );
		}
		fprintf(stderr,"Writing file %s...\n",tmpBuf1);

		fprintf( vtkOut1, "<?xml version=\"1.0\"?>\n" );
		fprintf( vtkOut1, "<VTKFile type= \"PStructuredGrid\"  version= \"0.1\" byte_order=\"LittleEndian\" compressor=\"vtkZLibDataCompressor\">\n");
		fprintf( vtkOut1, "  <PStructuredGrid WholeExtent=\"0 %d 0 %d 0 %d\" GhostLevel=\"0\">\n",gnode[0]-1,gnode[1]-1,gnode[2]-1);

		/* Start the node section */
		fprintf( vtkOut1, "    <PPointData>\n");
		fprintf( vtkOut1, "        <PDataArray type=\"Float32\" Name=\"velocity\" NumberOfComponents=\"3\"/>\n");
		if( doForce )
			fprintf( vtkOut1, "        <PDataArray type=\"Float32\" Name=\"force\" NumberOfComponents=\"3\"/>\n");
		if( doTemp )
			fprintf( vtkOut1, "        <PDataArray type=\"Float32\" Name=\"temperature\"/>\n");
		fprintf( vtkOut1, "    </PPointData>\n");

		/* Start the element section */
		fprintf( vtkOut1, "    <PCellData>\n");
		fprintf( vtkOut1, "        <PDataArray type=\"Float32\" Name=\"strainRate\"/>\n");
		fprintf( vtkOut1, "        <PDataArray type=\"Float32\" Name=\"stress\"/>\n");
		fprintf( vtkOut1, "        <PDataArray type=\"Float32\" Name=\"phase\"/>\n");
		if( doAps )
			fprintf( vtkOut1, "        <PDataArray type=\"Float32\" Name=\"plStrain\"/>\n");
		if( doHPr )
			fprintf( vtkOut1, "        <PDataArray type=\"Float32\" Name=\"pressure\"/>\n");
		if( doVisc )
			fprintf( vtkOut1, "        <PDataArray type=\"Float32\" Name=\"viscosity\"/>\n");
		fprintf( vtkOut1, "    </PCellData>\n");
	
		/* Write out coordinates. */
		fprintf( vtkOut1, "    <PPoints>\n");
		fprintf( vtkOut1, "      <PDataArray type=\"Float32\" NumberOfComponents=\"3\"/>\n");
		fprintf( vtkOut1, "    </PPoints>\n");

		/* Write pieces that actually contains data.*/
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
		/* Close the output file. */
		fclose( vtkOut1 );
	}
}

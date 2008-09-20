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
**	Converts Snac's binary output to OpenDX format
**
** $Id: snac2dx.c 3226 2006-06-16 22:24:13Z LaetitiaLePourhiet $
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
FILE*		irhIn;

unsigned int	elementGlobalSize[3];
int 			doTemp = 1;
int 			doForce = 1;
int 			doAps = 1;
int 			doIRh = 1;
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
			assert( forceIn /* failed to open file for reading */ );
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
			printf( "Warning, no plstrain.%u found... assuming plastic plugin not used.\n", rank );
			doAps = 0;
		}
		sprintf( tmpBuf, "%s/irheology.%u", path, rank );
		if( (irhIn = fopen( tmpBuf, "r" )) == NULL ) {
			printf( "Warning, no irheology.%u found... assuming plasticMaxwell plugin not used.\n", rank );
			doIRh = 0;
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
		if( irhIn ) {
			fclose( irhIn );
		}
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
	char		dxfmt_vel[PATH_MAX],dxfmt_strainrate[PATH_MAX],dxfmt_stress[PATH_MAX],dxfmt_pressure[PATH_MAX],dxfmt_force[PATH_MAX],dxfmt_temp[PATH_MAX],dxfmt_aps[PATH_MAX],dxfmt_visc[PATH_MAX],dxfmt_irh[PATH_MAX],dxfmt_phase[PATH_MAX];
	FILE*		dxOut;
	unsigned int	elementGlobalCount = elementGlobalSize[0] * elementGlobalSize[1] * elementGlobalSize[2];
	unsigned int	nodeGlobalSize[3] = { elementGlobalSize[0] + 1, elementGlobalSize[1] + 1, elementGlobalSize[2] + 1 };
	unsigned int	nodeGlobalCount = nodeGlobalSize[0] * nodeGlobalSize[1] * nodeGlobalSize[2];
	unsigned int	node_gI;
	unsigned int	element_gI;
	unsigned int 	objnum=1;

	/* open the output file */
	sprintf( tmpBuf, "%s/snac.%i.%06u.dx", path, rank, simTimeStep );
	if( (dxOut = fopen( tmpBuf, "w+" )) == NULL ) {
		assert( dxOut /* failed to open file for writing */ );
	}

	/* Write out simulation information ---- TODO: assumes nproc=1 */
	fprintf( dxOut, "# OpenDX DataFile Snac simulation output ASCII\n\n" );

	/* Write out position array */
	fprintf( dxOut, "# the positions array\n");
	fprintf( dxOut, "object %d class array type float rank 1 shape 3 items %d data follows\n",
		 objnum,nodeGlobalCount );
	objnum++;

	fseek( coordIn, dumpIteration * nodeGlobalCount * sizeof(float) * 3, SEEK_SET );
	for( node_gI = 0; node_gI < nodeGlobalCount; node_gI++ ) {
		float		coord[3];
		fread( &coord, sizeof(float), 3, coordIn );
		fprintf( dxOut, "%g %g %g\n", coord[0], coord[1], coord[2] );
	}

	/* Write out connections, assuming regular */
	fprintf( dxOut, "\n# the regular connections\n");
	fprintf( dxOut, "object %d class gridconnections counts %d %d %d\n",
		 objnum,nodeGlobalSize[2], nodeGlobalSize[1], nodeGlobalSize[0]);
	objnum++;

	/* Write out velocity array */
	fprintf( dxOut, "\n# the velocity array\n" );
	fprintf( dxOut, "object %d class array type float rank 1 shape 3 items %d data follows\n",
		 objnum,nodeGlobalCount );
	sprintf( dxfmt_vel, "object \"velocities\" class field\ncomponent \"positions\" value 1\ncomponent \"connections\" value 2\ncomponent \"data\" value %d\n\n",objnum);
	objnum++;

	fseek( velIn, dumpIteration * nodeGlobalCount * sizeof(float) * 3, SEEK_SET );
	for( node_gI = 0; node_gI < nodeGlobalCount; node_gI++ ) {
		float		vel[3];
		fread( &vel, sizeof(float), 3, velIn );
		fprintf( dxOut, "%g %g %g\n", vel[0], vel[1], vel[2] );
	}

	/* Write out strain rate array */
	fprintf( dxOut, "\n# the strain rate array\n" );
	fprintf( dxOut, "object %d class array type float rank 0 items %d data follows\n",
		 objnum, elementGlobalCount );
	sprintf( dxfmt_strainrate, "object \"strain_rate\" class field\ncomponent \"positions\" value 1\ncomponent \"connections\" value 2\ncomponent \"data\" value %d\n\n",objnum);
	objnum++;

	fseek( strainRateIn, dumpIteration * elementGlobalCount * sizeof(float), SEEK_SET );
	for( element_gI = 0; element_gI < elementGlobalCount; element_gI++ ) {
		float		strainRate;

		fread( &strainRate, sizeof(float), 1, strainRateIn );
		fprintf( dxOut, "%g\n", strainRate );
	}
	fprintf( dxOut, "attribute \"dep\" string \"connections\"\n" );

	/* Write out stress array */
	fprintf( dxOut, "\n# the stress array\n" );
	fprintf( dxOut, "object %d class array type float rank 0 items %d data follows\n",
		 objnum, elementGlobalCount );
	sprintf( dxfmt_stress, "object \"stress\" class field\ncomponent \"positions\" value 1\ncomponent \"connections\" value 2\ncomponent \"data\" value %d\n\n",objnum);
	objnum++;

	fseek( stressIn, dumpIteration * elementGlobalCount * sizeof(float), SEEK_SET );
	for( element_gI = 0; element_gI < elementGlobalCount; element_gI++ ) {
	        float		stress;

		fread( &stress, sizeof(float), 1, stressIn );
		fprintf( dxOut, "%g\n", stress );
	}
	fprintf( dxOut, "attribute \"dep\" string \"connections\"\n" );

	/* Write out pressure array */
       if (doHPr) {
	fprintf( dxOut, "\n# the pressure array\n" );
	fprintf( dxOut, "object %d class array type float rank 0 items %d data follows\n",
		 objnum, elementGlobalCount );
	sprintf( dxfmt_pressure, "object \"pressure\" class field\ncomponent \"positions\" value 1\ncomponent \"connections\" value 2\ncomponent \"data\" value %d\n\n",objnum);
	objnum++;

	fseek( pisosIn, dumpIteration * elementGlobalCount * sizeof(float), SEEK_SET );
	for( element_gI = 0; element_gI < elementGlobalCount; element_gI++ ) {
	        float		pressure;

		fread( &pressure, sizeof(float), 1, pisosIn );
		fprintf( dxOut, "%g\n", pressure );
	}
	fprintf( dxOut, "attribute \"dep\" string \"connections\"\n" );
       }
	/* Write out force array */
	if( doForce ) {
		fprintf( dxOut, "\n# the force array\n" );
		fprintf( dxOut, "object %d class array type float rank 1 shape 3 items %d data follows\n",
				objnum, nodeGlobalCount );
		sprintf( dxfmt_force, "object \"force\" class field\ncomponent \"positions\" value 1\ncomponent \"connections\" value 2\ncomponent \"data\" value %d\n\n",objnum);
		objnum++;

		fseek( forceIn, dumpIteration * nodeGlobalCount * sizeof(float) * 3, SEEK_SET );
		for( node_gI = 0; node_gI < nodeGlobalCount; node_gI++ ) {
			float		force[3];
			fread( &force, sizeof(float), 3, forceIn );
			fprintf( dxOut, "%g %g %g\n", force[0], force[1], force[2] );
		}
	}

	/* Write out phase array */
	fprintf( dxOut, "\n# the phaseIndex array\n" );
	fprintf( dxOut, "object %d class array type int rank 0 items %d data follows\n",
			 objnum, elementGlobalCount );
	sprintf( dxfmt_phase, "object \"phase\" class field\ncomponent \"positions\" value 1\ncomponent \"connections\" value 2\ncomponent \"data\" value %d\n\n",objnum);
	objnum++;

	fseek( phaseIn, dumpIteration * elementGlobalCount * sizeof(unsigned int), SEEK_SET );
	for( element_gI = 0; element_gI < elementGlobalCount; element_gI++ ) {
		unsigned int		phaseIndex;
		fread( &phaseIndex, sizeof(unsigned int), 1, phaseIn );
		fprintf( dxOut, "%u\n", phaseIndex );
	}
	fprintf( dxOut, "attribute \"dep\" string \"connections\"\n" );


	/* Write out temperature array */
	if(doTemp) {
		fprintf( dxOut, "\n# the temperature array\n" );
		fprintf( dxOut, "object %d class array type float rank 0 items %d data follows\n",
			 objnum, nodeGlobalCount );
		sprintf( dxfmt_temp, "object \"temperature\" class field\ncomponent \"positions\" value 1\ncomponent \"connections\" value 2\ncomponent \"data\" value %d\n\n",objnum);
		objnum++;

		fseek( tempIn, dumpIteration * nodeGlobalCount * sizeof(float), SEEK_SET );
		for( node_gI = 0; node_gI < nodeGlobalCount; node_gI++ ) {
		        float		temperature;

			fread( &temperature, sizeof(float), 1, tempIn );
			fprintf( dxOut, "%g\n", temperature );
		}
	}

	/* Write out plstrain array */
	if(doAps) {
		fprintf( dxOut, "\n# the accumulated plastic strain array\n" );
		fprintf( dxOut, "object %d class array type float rank 0 items %d data follows\n",
			 objnum, elementGlobalCount );
		sprintf( dxfmt_aps, "object \"plstrain\" class field\ncomponent \"positions\" value 1\ncomponent \"connections\" value 2\ncomponent \"data\" value %d\n\n",objnum);
		objnum++;

		fseek( apsIn, dumpIteration * elementGlobalCount * sizeof(float), SEEK_SET );
		for( element_gI = 0; element_gI < elementGlobalCount; element_gI++ ) {
		        float		plstrain;

			fread( &plstrain, sizeof(float), 1, apsIn );
			fprintf( dxOut, "%g\n", plstrain );
		}
		fprintf( dxOut, "attribute \"dep\" string \"connections\"\n" );
	}

	/* Write out viscosity array */
	if(doVisc) {
		fprintf( dxOut, "\n# the viscosity array\n" );
		fprintf( dxOut, "object %d class array type float rank 0 items %d data follows\n",
			 objnum, elementGlobalCount );
		sprintf( dxfmt_visc, "object \"viscosity\" class field\ncomponent \"positions\" value 1\ncomponent \"connections\" value 2\ncomponent \"data\" value %d\n\n",objnum);
		objnum++;

		fseek( viscIn, dumpIteration * elementGlobalCount * sizeof(float), SEEK_SET );
		for( element_gI = 0; element_gI < elementGlobalCount; element_gI++ ) {
		        float		viscosity;

			fread( &viscosity, sizeof(float), 1, viscIn );
			fprintf( dxOut, "%g\n", viscosity );
		}
		fprintf( dxOut, "attribute \"dep\" string \"connections\"\n" );
	}

	/* Write out irheology array */
	if(doIRh) {
		fprintf( dxOut, "\n# the rheology index: 1 for plastic, -1 for viscoelastic\n" );
		fprintf( dxOut, "object %d class array type float rank 0 items %d data follows\n",
			 objnum, elementGlobalCount );
		sprintf( dxfmt_irh, "object \"irheology\" class field\ncomponent \"positions\" value 1\ncomponent \"connections\" value 2\ncomponent \"data\" value %d\n\n",objnum);
		objnum++;

		fseek( irhIn, dumpIteration * elementGlobalCount * sizeof(float), SEEK_SET );
		for( element_gI = 0; element_gI < elementGlobalCount; element_gI++ ) {
		        int		irheology;

			fread( &irheology, sizeof(int), 1, irhIn );
			fprintf( dxOut, "%d\n", irheology );
		}
		fprintf( dxOut, "attribute \"dep\" string \"connections\"\n" );
	}

	/* Write out data structure */
	fprintf( dxOut, "\n# construct data structure\n");

	fprintf( dxOut, dxfmt_vel );
	fprintf( dxOut, dxfmt_strainrate );
	fprintf( dxOut, dxfmt_stress );
        if( doHPr ) {
	fprintf( dxOut, dxfmt_pressure );
        }
	if( doForce ) {
		fprintf( dxOut, dxfmt_force );
	}
	fprintf( dxOut, dxfmt_phase );
	if(doTemp) {
		fprintf( dxOut, dxfmt_temp );
	}
	if(doAps) {
		fprintf( dxOut, dxfmt_aps );
	}
	if(doVisc) {
		fprintf( dxOut, dxfmt_visc );
	}
	if(doIRh) {
		fprintf( dxOut, dxfmt_irh );
	}

	fprintf( dxOut, "object \"default\" class group\n");
	fprintf( dxOut, "member \"velocity\" value \"velocities\"\n");
	fprintf( dxOut, "member \"strain_rate\" value \"strain_rate\"\n");
	fprintf( dxOut, "member \"stress\" value \"stress\"\n");
	if ( doHPr )
        fprintf( dxOut, "member \"pressure\" value \"pressure\"\n");
	if(doForce)
		fprintf( dxOut, "member \"force\" value \"force\"\n");
	fprintf( dxOut, "member \"phaseIndex\" value \"phase\"\n");
	if(doTemp)
		fprintf( dxOut, "member \"temperature\" value \"temperature\"\n");
	if(doAps)
		fprintf( dxOut, "member \"plstrain\" value \"plstrain\"\n");
	if(doVisc)
		fprintf( dxOut, "member \"viscosity\" value \"viscosity\"\n");
	if(doIRh)
		fprintf( dxOut, "member \"irheology\" value \"irheology\"\n");

	fprintf( dxOut, "\nEnd\n");

	fclose( dxOut );

	return;
}

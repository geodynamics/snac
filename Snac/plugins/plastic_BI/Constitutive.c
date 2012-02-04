/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2004,
**       Pururav Thoutireddy, Center for Advanced Computing Research, Caltech.
**	Steve Quenette, 110 Victoria Street, Melbourne, Victoria, 3053, Australia.
**	Californian Institute of Technology, 1200 East California Boulevard, Pasadena, California, 91125, USA.
**	University of Texas, 1 University Station, Austin, Texas, 78712, USA.
**
** Authors:
**      Pururav Thoutireddy, Center for Advanced Computing Research, Caltech. ( puru@cacr.caltech.edu)
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
** $Id: Constitutive.c 3274 2007-03-27 20:25:29Z EunseoChoi $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "Element.h"
#include "Constitutive.h"
#include "Register.h"
#include <math.h>
#include <string.h>
#include <assert.h>

#ifndef PI
#ifndef M_PIl
#ifndef M_PI
#define PI 4.0*atan(1.0)
#else
#define PI M_PI
#endif
#else
#define PI M_PIl
#endif
#endif


void SnacPlastic_Constitutive( void* _context, Element_LocalIndex element_lI ) {
	Snac_Context* context = (Snac_Context*)_context;
	Snac_Element* element = Snac_Element_At( context, element_lI );
	SnacPlastic_Element* plasticElement = ExtensionManager_Get( context->mesh->elementExtensionMgr, element, SnacPlastic_ElementHandle );
	const Snac_Material* material = &context->materialProperty[element->material_I];

	/*ccccc*/
	MeshLayout*			meshLayout = (MeshLayout*)context->meshLayout;
	HexaMD*				decomp = (HexaMD*)meshLayout->decomp;
	IJK				ijk;
	Element_GlobalIndex		element_gI = _MeshDecomp_Element_LocalToGlobal1D( decomp, element_lI );
	EntryPoint* 			temperatureEP;

	RegularMeshUtils_Element_1DTo3D( decomp, element_gI, &ijk[0], &ijk[1], &ijk[2] );
	/*ccccc*/

	temperatureEP = Context_GetEntryPoint( context,	"Snac_EP_LoopElementsEnergy" );

	/* If this is a Plastic material, calculate its stress. */
	if ( material->rheology & Snac_Material_Plastic ) {
		Tetrahedra_Index	tetra_I;

		/* plastic material properties */
		StressTensor*		stress;
		StrainTensor*		strain;
		Strain		        plasticStrain;
		double				trace_strain;
		plModel             yieldcriterion=material->yieldcriterion;
		/* elasto-plastic material properties */
		double				cohesion = 0.0f;
		double				frictionAngle = 0.0f;
		double				dilationAngle = 0.0f;
		double				hardening = 0.0f;
		double				tension_cutoff=0.0;
		const double		degrad = PI / 180.0f;
		double				totalVolume=0.0f,depls=0.0f;
		int principal_stresses(StressTensor* stress,double sp[],double cn[3][3]);
		unsigned int		i;
		double				tmp=0.0;
		const double		a1 = material->lambda + 2.0f * material->mu ;
		const double		a2 = material->lambda ;
		int					ind=0;

		/*    printf("Entered Plastic update \n"); */

		/* Work out the plastic material properties of this element */
		for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
			double			cn[3][3] = {{0.0,0.0,0.0},{0.0,0.0,0.0},{0.0,0.0,0.0}};
			double			s[3] = {0.0,0.0,0.0};
			double			alam, dep1, dep2, dep3, depm;
			/*ccccc*/

			stress = &element->tetra[tetra_I].stress;
			strain = &element->tetra[tetra_I].strain;
			plasticStrain = plasticElement->plasticStrain[tetra_I];

			trace_strain = (*strain)[0][0] + (*strain)[1][1] + (*strain)[2][2];

			(*stress)[0][0] += (2.0f * material->mu) * (*strain)[0][0] + material->lambda * (trace_strain );
			(*stress)[1][1] += (2.0f * material->mu) * (*strain)[1][1] + material->lambda * (trace_strain );
			(*stress)[2][2] += (2.0f * material->mu) * (*strain)[2][2] + material->lambda * (trace_strain );
			(*stress)[0][1] += (2.0f * material->mu) * (*strain)[0][1];
			(*stress)[0][2] += (2.0f * material->mu) * (*strain)[0][2];
			(*stress)[1][2] += (2.0f * material->mu) * (*strain)[1][2];

			principal_stresses(stress,s,cn);

			/* compute friction and dilation angles based on accumulated plastic strain in tetrahedra */
			/* Piece-wise linear softening */
			/* Find current properties from linear interpolation */
			for( i = 0; i < material->nsegments; i++ ) {
				const double pl1 = material->plstrain[i];
				const double pl2 = material->plstrain[i+1];

				if( plasticStrain >= pl1 && plasticStrain <= pl2 ) {
					const double	tgf = (material->frictionAngle[i+1] - material->frictionAngle[i]) / (pl2 - pl1);
					const double	tgd = (material->dilationAngle[i+1] - material->dilationAngle[i]) / (pl2 - pl1);
					const double	tgc = (material->cohesion[i+1] - material->cohesion[i]) / (pl2 - pl1);

					frictionAngle = material->frictionAngle[i] + tgf * (plasticStrain - pl1);
					dilationAngle = material->dilationAngle[i] + tgd * (plasticStrain - pl1);
					cohesion = material->cohesion[i] + tgc * (plasticStrain - pl1);
					hardening = tgc;
				}
			}

			if( frictionAngle > 0.0f ) {
				tension_cutoff = material->ten_off;
				if( frictionAngle > 0.0) {
					tmp = cohesion / tan( frictionAngle * degrad);
					if(tmp < tension_cutoff) tension_cutoff=tmp;
				}
			}
			else {
				if( plasticStrain < 0.0 ) {
					plasticElement->plasticStrain[tetra_I] = 0.0;
					plasticStrain = plasticElement->plasticStrain[tetra_I];
					fprintf(stderr,"Warning: negative plastic strain. Setting to zero, but check if remesher is on and this happended for an external tet. rank:%d elem:%d tet:%d plasticStrain=%e frictionAngle=%e\n",context->rank,element_lI,tetra_I,plasticStrain,frictionAngle);
					frictionAngle = material->frictionAngle[0];
					dilationAngle = material->dilationAngle[0];
					cohesion = material->cohesion[0];
				}
				else {
					/* frictionAngle < 0.0 violates second law of thermodynamics */
					fprintf(stderr,"Error due to an unknown reason: rank:%d elem:%d tet:%d plasticStrain=%e frictionAngle=%e\n",context->rank,element_lI,tetra_I,plasticStrain,frictionAngle);
					assert(0);
				}
			}

			if( yieldcriterion == mohrcoulomb )
				{

					double sphi = sin( frictionAngle * degrad );
					double spsi = sin( dilationAngle * degrad );
					double anphi = (1.0f + sphi) / (1.0f - sphi);
					double anpsi = (1.0f + spsi) / (1.0f - spsi);
					double fs = s[0] - s[2] * anphi + 2 * cohesion * sqrt( anphi );
					double ft = s[2] - tension_cutoff;
					/* CHECK FOR COMPOSITE YIELD CRITERION  */
					ind=0;
					if( fs < 0.0f || ft > 0.0f ) {
						/*! Failure: shear or tensile */
						double aP = sqrt( 1.0f + anphi * anphi ) + anphi;
						double sP = tension_cutoff * anphi - 2 * cohesion * sqrt( anphi );
						double h = s[2] - tension_cutoff + aP * ( s[0] - sP );

						ind=1;

						if( h < 0.0f ) {
							/* !shear failure  */
							alam = fs / ( a1 - a2 * anpsi + a1 * anphi * anpsi - a2 * anphi + 2.0*sqrt(anphi)*hardening );
							s[0] -= alam * ( a1 - a2 * anpsi );
							s[1] -= alam * a2 * ( 1.0f - anpsi );
							s[2] -= alam * ( a2 - a1 * anpsi );
							dep1 = alam;
							dep2 = 0.0f;
							dep3 = -alam * anpsi;
						}
						else {
							/* tensile failure */
							alam = ft / a1;
							s[0] -= alam * a2;
							s[1] -= alam * a2;
							s[2] -= alam * a1;
							dep1 = 0.0f;
							dep2 = 0.0f;
							dep3 = alam;
						}
					}
					else {
						/* ! no failure - just elastic increment */

						dep1 = 0.0f;
						dep2 = 0.0f;
						dep3 = 0.0f;
					}
					if(ind) {
						unsigned int k,m,n;
						/* Second invariant of accumulated plastic increment  */
						depm = ( dep1 + dep2 + dep3 ) / 3.0f;
						plasticElement->plasticStrain[tetra_I] += sqrt( 0.5f * ((dep1-depm) * (dep1-depm) + (dep2-depm) * (dep2-depm) + (dep3-depm) * (dep3-depm) + depm*depm) );

						/* Stress projection back to euclidean coordinates */
						memset( stress, 0, sizeof((*stress)) );
						/* Resolve back to global axes  */
						for( m = 0; m < 3; m++ ) {
							for( n = m; n < 3; n++ ) {
								for( k = 0; k < 3; k++ ) {
									(*stress)[m][n] += cn[k][m] * cn[k][n] * s[k];
								}
							}
						}
					}
				}
			else
				Journal_Firewall( (0>1), "In %s: \"mohrcoulomb\" is the only available yield criterion.\n", __func__ );

			/* linear healing: applied whether this tet has yielded or not. 
			   Parameters are hardwired for now, but should be given through an input file. */
			/* plasticElement->plasticStrain[tetra_I] *= (1.0/(1.0+context->dt/1.0e+12)); */
			/* plasticElement->plasticStrain[tetra_I] *= (1.0/(1.0+context->dt/(ind?1.0e+13:5.0e+11))); */

			depls += plasticElement->plasticStrain[tetra_I]*element->tetra[tetra_I].volume;
			totalVolume += element->tetra[tetra_I].volume;
		}
		/* volume-averaged accumulated plastic strain, aps */
		plasticElement->aps = depls/totalVolume;
	}
}


int principal_stresses(StressTensor* stress, double sp[3], double cn[3][3])
{

	double **a,**v,*d;
	int i,j;

	a = dmatrix(1,3,1,3);
	v = dmatrix(1,3,1,3);
	d = dvector(1,3);

	a[1][1] = (*stress)[0][0];
	a[2][2] = (*stress)[1][1];
	a[3][3] = (*stress)[2][2];
	a[1][2] = (*stress)[0][1];
	a[1][3] = (*stress)[0][2];
	a[2][3] = (*stress)[1][2];
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
	fprintf(stderr,"Numerical Recipes run-time error...\n");
	fprintf(stderr,"%s\n",error_text);
	fprintf(stderr,"...now exiting to system...\n");
	exit(1);
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

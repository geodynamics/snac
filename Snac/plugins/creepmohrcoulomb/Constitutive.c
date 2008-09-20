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
 ** $Id: Constitutive.c 3134 2005-08-19 22:51:28Z EunseoChoi $
 **
 **~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "units.h"
#include "types.h"
#include "Element.h"
#include "Constitutive.h"
#include "Register.h"
#include <math.h>
#include "Snac/Temperature/Temperature.h"
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

#define CREEP_MAX_ITER 50
#define CREEP_TOL 1.0e-06

void SnacCreepMohrCoulomb_Constitutive( void* _context, Element_LocalIndex element_lI ) {
	Snac_Context* context = (Snac_Context*)_context;
	Snac_Element* element = Snac_Element_At( context, element_lI );
	SnacCreepMohrCoulomb_Element* creepmohrcoulombElement = ExtensionManager_Get( context->mesh->elementExtensionMgr, element, SnacCreepMohrCoulomb_ElementHandle );
	SnacTemperature_Element* temperatureElement = ExtensionManager_Get( context->mesh->elementExtensionMgr, element, SnacTemperature_ElementHandle );
	const Snac_Material* material = &context->materialProperty[element->material_I];
	EntryPoint* 			temperatureEP;

	temperatureEP = Context_GetEntryPoint( context,	"Snac_EP_LoopElementsEnergy" );
	Journal_Firewall( (int)(temperatureEP != NULL), context->snacError, 
		  "CreepMohrCoulomb plugin requires Temperature plugin. Add SnacTemperature to the plugins list in the input file\n" );

	/* If this is a CreepMohrCoulomb material, calculate its stress. */
	if ( material->rheology & Snac_Material_CreepMohrCoulomb ) {
		Tetrahedra_Index		tetra_I;
		
		StressTensor*			stress;
		StrainTensor*			strain;
		StrainTensor*			plasticstrainTensor;
		StrainTensor*			creepstrainTensor;
		Strain					plasticStrain;
		double					straind0,straind1,straind2,stressd0,stressd1,stressd2;
		double					trace_strain;
		double					trace_stress;

		/* Used by plastic update only, but to get the element average, declared here. */
		double					totalVolume=0.0f,depls=0.0f;

		/* Work out the plastic material properties of this element */
		for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {

			/* Arrays repeatedly used to store pricipal values/vectors. */
			int principal_stresses(StressTensor* stress,double sp[],double cn[3][3]);
			double		        cn[3][3] = {{0.0,0.0,0.0},{0.0,0.0,0.0},{0.0,0.0,0.0}};
			double		        s[3] = {0.0,0.0,0.0};
			double				avgTemp = 0.0;
			unsigned int 		tetNode_I;

			/* Create local aliases to necessary arrays. */
			stress = &element->tetra[tetra_I].stress;
			strain = &element->tetra[tetra_I].strain;
			plasticStrain = creepmohrcoulombElement->plasticStrain[tetra_I];
			plasticstrainTensor = &creepmohrcoulombElement->plasticstrainTensor[tetra_I];
			creepstrainTensor = &creepmohrcoulombElement->creepstrainTensor[tetra_I];

			/* Add the thermal contribution to stress. */
			if( context->computeThermalStress ) {
				(*stress)[0][0] += temperatureElement->thermalStress[tetra_I];
				(*stress)[1][1] += temperatureElement->thermalStress[tetra_I];
				(*stress)[2][2] += temperatureElement->thermalStress[tetra_I];
			}

			/* Deviatoric Stresses and Strains */
			trace_stress = (*stress)[0][0] + (*stress)[1][1] + (*stress)[2][2];
			trace_strain = element->tetra[tetra_I].volume/element->tetra[tetra_I].old_volume-1.0f;
			/* trace_strain = (*strain)[0][0] + (*strain)[1][1] + (*strain)[2][2]; */
			straind0 =  (*strain)[0][0] -  (trace_strain) / 3.0f;
			straind1 =  (*strain)[1][1] -  (trace_strain) / 3.0f;
			straind2 =  (*strain)[2][2] -  (trace_strain) / 3.0f;
			stressd0 =  (*stress)[0][0] -  (trace_stress) / 3.0f;
			stressd1 =  (*stress)[1][1] -  (trace_stress) / 3.0f;
			stressd2 =  (*stress)[2][2] -  (trace_stress) / 3.0f;

			/* get the average temperature for this tetrahedron. */
			for(tetNode_I=0; tetNode_I<Tetrahedra_Point_Count; tetNode_I++) {
				Snac_Node* contributingNode = Snac_Element_Node_P(
						context,
						element_lI,
						TetraToNode[tetra_I][tetNode_I] );
				SnacTemperature_Node* temperatureNodeExt = ExtensionManager_Get(
						context->mesh->nodeExtensionMgr,contributingNode,
						SnacTemperature_NodeHandle );
				
				avgTemp += 0.25 * temperatureNodeExt->temperature;
				assert( !isnan(avgTemp) && !isinf(avgTemp) );
			}

			/* Trial stress by power law creep. */
			{
				/* Power law relations */
				double				rviscosity=material->refvisc; // Pre-exponential coefficient... [/s/(MPa)^n]
				double				H = material->activationE; // Activation enthalpy [J/mol]
				double				actV = material->activationV; // Activation volume [m^3/mol]
				double				srexponent = material->srexponent; // The exponent n.
				const double		R=8.31448;  // J/mol/K
				const double 		rmu= material->mu;
				const double 		bulkm = material->lambda + 2.0f * material->mu/3.0f;
				double 		 		beta, residual;
				double 		 		beta_old1, beta_old2, beta_old3, beta_old4, beta_old5, beta_old6;
				double 		 		beta_new1, beta_new2, beta_new3, beta_new4, beta_new5, beta_new6;
				double 		 		res1, res2, res3, res4, res5, res6;
				double 		 		dsigd1, dsigd2, dsigd3, dsigd4, dsigd5, dsigd6;
				Index 		 		nonlinear_iter;

				/* Compute the initial residual. */
				principal_stresses(stress,s,cn);

				beta = pow(1.0e+06,-1.0*srexponent)*rviscosity
					*pow(fabs(s[0]-s[2]),1.0*srexponent)*exp(-1.0*H/(R*(avgTemp+273.15)));

				beta_old1 = beta * stressd0/fabs(s[0]-s[2]);
				beta_new1 = beta_old1;
				res1 = straind0 - stressd0/(2.0*rmu) - (*creepstrainTensor)[0][0] - 0.5*context->dt*(beta_old1+beta_new1);
				residual = res1*res1;

				beta_old2 = beta * stressd1/fabs(s[0]-s[2]);
				beta_new2 = beta_old2;
				res2 = straind1 - stressd1/(2.0*rmu) - (*creepstrainTensor)[1][1] - 0.5*context->dt*(beta_old2+beta_new2);
				residual += res2*res2;

				beta_old3 = beta * stressd2/fabs(s[0]-s[2]);
				beta_new3 = beta_old3;
				res3 = straind2 - stressd2/(2.0*rmu) - (*creepstrainTensor)[2][2] - 0.5*context->dt*(beta_old3+beta_new3);
				residual += res3*res3;
				
				beta_old4 = beta * (*stress)[0][1]/fabs(s[0]-s[2]);
				beta_new4 = beta_old4;
				res4 = (*strain)[0][1] - (*stress)[0][1]/(2.0*rmu) - (*creepstrainTensor)[0][1] 
					- 0.5*context->dt*(beta_old4+beta_new4);
				residual += res4*res4;

				beta_old5 = beta * (*stress)[1][2]/fabs(s[0]-s[2]);
				beta_new5 = beta_old5;
				res5 = (*strain)[1][2] - (*stress)[1][2]/(2.0*rmu) - (*creepstrainTensor)[1][2] 
					- 0.5*context->dt*(beta_old5+beta_new5);
				residual += res5*res5;

				beta_old6 = beta * (*stress)[0][2]/fabs(s[0]-s[2]);
				beta_new6 = beta_old6;
				res6 = (*strain)[0][2] - (*stress)[0][2]/(2.0*rmu) - (*creepstrainTensor)[0][2] 
					- 0.5*context->dt*(beta_old6+beta_new6);
				residual += res6*res6;

				residual = sqrt(residual);

				/* start iterative update of stress and creep strain */
				nonlinear_iter = 0;
				while( residual > CREEP_TOL && nonlinear_iter < CREEP_MAX_ITER ) {
					double compliance = 1.0/(2.0*rmu)
						+ 0.5*context->dt*srexponent
						*pow(1.0e+06,-1.0*srexponent)*rviscosity
						*pow(fabs(s[0]-s[2]),1.0*srexponent-1.0)*exp(-1.0*H/(R*(avgTemp+273.15)));
					dsigd1 = res1 / compliance;
					dsigd2 = res2 / compliance;
					dsigd3 = res3 / compliance;
					dsigd4 = res4 / compliance;
					dsigd5 = res5 / compliance;
					dsigd6 = res6 / compliance;
					stressd0 += dsigd1;
					stressd1 += dsigd2;
					stressd2 += dsigd3;
					(*stress)[0][1] += dsigd4;
					(*stress)[1][2] += dsigd5;
					(*stress)[0][2] += dsigd6;

					principal_stresses(stress,s,cn);
					beta = pow(1.0e+06,-1.0*srexponent)*rviscosity
						*pow(fabs(s[0]-s[2]),1.0*srexponent)*exp(-1.0*H/(R*(avgTemp+273.15)));
					beta_new1 = beta * stressd0/fabs(s[0]-s[2]);
					beta_new2 = beta * stressd1/fabs(s[0]-s[2]);
					beta_new3 = beta * stressd2/fabs(s[0]-s[2]);
					beta_new4 = beta * (*stress)[0][1]/fabs(s[0]-s[2]);
					beta_new5 = beta * (*stress)[1][2]/fabs(s[0]-s[2]);
					beta_new6 = beta * (*stress)[0][2]/fabs(s[0]-s[2]);
					(*creepstrainTensor)[0][0] += 0.5*context->dt*(beta_old1+beta_new1);
					(*creepstrainTensor)[1][1] += 0.5*context->dt*(beta_old2+beta_new2);
					(*creepstrainTensor)[2][2] += 0.5*context->dt*(beta_old3+beta_new3);
					(*creepstrainTensor)[0][1] += 0.5*context->dt*(beta_old4+beta_new4);
					(*creepstrainTensor)[1][2] += 0.5*context->dt*(beta_old5+beta_new5);
					(*creepstrainTensor)[0][2] += 0.5*context->dt*(beta_old6+beta_new6);
					
					res1 = straind0 - stressd0/(2.0*rmu) - (*creepstrainTensor)[0][0]; 
					res2 = straind1 - stressd1/(2.0*rmu) - (*creepstrainTensor)[1][1]; 
					res3 = straind2 - stressd2/(2.0*rmu) - (*creepstrainTensor)[2][2]; 
					res4 = (*strain)[0][1] - (*stress)[0][1]/(2.0*rmu) - (*creepstrainTensor)[0][1]; 
					res5 = (*strain)[1][2] - (*stress)[1][2]/(2.0*rmu) - (*creepstrainTensor)[1][2]; 
					res6 = (*strain)[0][2] - (*stress)[0][2]/(2.0*rmu) - (*creepstrainTensor)[0][2]; 
					residual = sqrt( res1*res1 + res2*res2 + res3*res3 + res4*res4 + res5*res5 + res6*res6 );
					
					++nonlinear_iter;
				}
					
				/* Isotropic stress is elastic,
				   WARNING: Volumetric strain may be better defined as
				   volumique change in the mesh */
				(*stress)[0][0] = stressd0 + trace_stress / 3.0f + bulkm * trace_strain;
				(*stress)[1][1] = stressd1 + trace_stress / 3.0f + bulkm * trace_strain;
				(*stress)[2][2] = stressd2 + trace_stress / 3.0f + bulkm * trace_strain;
			}
			/* End of the creep part */
			
			/* Start plastic correction step. */
			{
				double			alam, dep1, dep2, dep3, depm;
				double			fs,ft;
				unsigned int	k, m, n;
				/* elasto-plastic material properties */
				double			cohesion = 0.0f;
				double			frictionAngle = 0.0f;
				double			dilationAngle = 0.0f;
				double			hardening = 0.0f;
				double			sphi=0.0;
				double			spsi=0.0;
				double			st=0.0;
				const double	degrad = PI / 180.0f;
				Index 			segI;
				double		        anphi=0.0,anpsi=0.0;/* for mohr-coulomb model */
				const double		a1 = material->lambda + 2.0f * material->mu ;
				const double		a2 = material->lambda ;
				int                 yielded=0;

				principal_stresses(stress,s,cn);

				/* compute friction and dilation angles based on accumulated plastic strain in tetrahedra */
				/* Piece-wise linear softening */
				/* Find current properties from linear interpolation */
				for( segI = 0; segI < material->nsegments; segI++ ) {
					const double pl1 = material->plstrain[segI];
					const double pl2 = material->plstrain[segI+1];
					
					if( plasticStrain >= pl1 && plasticStrain <= pl2 ) {
						const double	tgf = (material->frictionAngle[segI+1] - material->frictionAngle[segI]) / (pl2 - pl1);
						const double	tgd = (material->dilationAngle[segI+1] - material->dilationAngle[segI]) / (pl2 - pl1);
						const double	tgc = (material->cohesion[segI+1] - material->cohesion[segI]) / (pl2 - pl1);
						
						frictionAngle = material->frictionAngle[segI] + tgf * (plasticStrain - pl1);
						dilationAngle = material->dilationAngle[segI] + tgd * (plasticStrain - pl1);
						cohesion = material->cohesion[segI] + tgc * (plasticStrain - pl1);
						hardening = tgc;
					}
				}
				if( frictionAngle > 0.0f ) {
					double tmp=0.0;
					st = material->ten_off;
					if( frictionAngle > 0.0) {
						tmp = cohesion / tan( frictionAngle * degrad);
						if(tmp < st)st=tmp;
					}
				}
				else {
					/* frictionAngle < 0.0 violates second law of thermodynamics */
					assert(0);
				}
				
				sphi = sin( frictionAngle * degrad );
				spsi = sin( dilationAngle * degrad );
				anphi = (1.0f + sphi) / (1.0f - sphi);
				anpsi = (1.0f + spsi) / (1.0f - spsi);
				fs = s[0] - s[2] * anphi + 2 * cohesion * sqrt( anphi );
				ft = s[2] - st;
				/* CHECK FOR COMPOSITE YIELD CRITERION  */
				yielded=0;
				if( fs < 0.0f || ft > 0.0f ) {
					double	aP,sP,h;
					yielded=1;
					
					/*! Failure: shear or tensile */
					
					aP = sqrt( 1.0f + anphi * anphi ) + anphi;
					sP = st * anphi - 2 * cohesion * sqrt( anphi );
					h = s[2] - st + aP * ( s[0] - sP );
					
					if( h < 0.0f ) {
						/* !shear failure  */
						alam = fs / ( a1 - a2 * anpsi + a1 * anphi * anpsi - a2 * anphi + hardening );
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
				if(yielded) {
					/* Second invariant of accumulated plastic increment  */
					depm = ( dep1 + dep2 + dep3 ) / 3.0f;
					creepmohrcoulombElement->plasticStrain[tetra_I] += sqrt( 0.5f * ((dep1-depm) * (dep1-depm) + (dep2-depm) * (dep2-depm) + (dep3-depm) * (dep3-depm) + depm*depm) );
					
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
				depls += creepmohrcoulombElement->plasticStrain[tetra_I]*element->tetra[tetra_I].volume;
				totalVolume += element->tetra[tetra_I].volume;
			}
			/* End of the plastic (brittle) part. */
		}
		/* volume-averaged accumulated plastic strain, aps */
		creepmohrcoulombElement->aps = depls/totalVolume;
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

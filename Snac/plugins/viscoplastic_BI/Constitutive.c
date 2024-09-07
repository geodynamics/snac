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


void SnacViscoPlastic_Constitutive( void* _context, Element_LocalIndex element_lI ) {
	Snac_Context* context = (Snac_Context*)_context;
	Snac_Element* element = Snac_Element_At( context, element_lI );
	SnacViscoPlastic_Element* viscoplasticElement = ExtensionManager_Get( context->mesh->elementExtensionMgr, element, SnacViscoPlastic_ElementHandle );
	SnacTemperature_Element* temperatureElement = ExtensionManager_Get( context->mesh->elementExtensionMgr, element, SnacTemperature_ElementHandle );
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

	/* If this is a ViscoPlastic material, calculate its stress. */
	if ( material->rheology & Snac_Material_ViscoPlastic ) {
		Tetrahedra_Index	tetra_I;
		Node_LocalIndex		node_lI;

		/* viscoplastic material properties */
		const double		bulkm = material->lambda + 2.0f * material->mu/3.0f;
		StressTensor*		stress;
		StrainTensor*		strain;
		Strain				plasticStrain;
		double*				viscosity;
		double				straind0,straind1,straind2,stressd0,stressd1,stressd2;
		double				Stress0[3][3];
		double				trace_strain;
		double				trace_stress;
		double				temp;
		double				vic1;
		double				vic2;
		double				VolumicStress;
		double				rviscosity=material->refvisc;
		double				rmu= material->mu;
		double				srJ2;
		double				avgTemp;
		plModel				yieldcriterion=material->yieldcriterion;

		/* For now reference values of viscosity, second invariant of deviatoric */
		/* strain rate and reference temperature  are being hard wired ( these specific */
		/* values are from paper by Hall et. al., EPSL, 2003 */
		double				rstrainrate = material->refsrate;
		double				rTemp = material->reftemp;
		double				H = material->activationE; // kJ/mol
		double				srexponent = material->srexponent;
		double				srexponent1 = material->srexponent1;
		double				srexponent2 = material->srexponent2;
		const double		R=8.31448;  // J/mol/K
		/* elasto-plastic material properties */
		double				cohesion = 0.0f;
		double				frictionAngle = 0.0f;
		double				dilationAngle = 0.0f;
		double				hardening = 0.0f;
		double				tension_cutoff=0.0;
		const double		degrad = PI / 180.0f;
		double				totalVolume=0.0f,depls=0.0f;
		unsigned int		i;
		double				tmp=0.0;
		const double		a1 = material->lambda + 2.0f * material->mu ;
		const double		a2 = material->lambda ;
		int					ind=0;

		int principal_stresses(StressTensor* stress,double sp[],double cn[3][3]);
		int principal_stresses_orig(StressTensor* stress, double sp[3], double cn[3][3]);

		/*    printf("Entered ViscoPlastic update \n"); */

		/* Work out the plastic material properties of this element */
		for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
			double			cn[3][3] = {{0.0,0.0,0.0},{0.0,0.0,0.0},{0.0,0.0,0.0}};
			double			s[3] = {0.0,0.0,0.0};
			double			alam, dep1, dep2, dep3, depm;
			/*ccccc*/

			stress = &element->tetra[tetra_I].stress;
			strain = &element->tetra[tetra_I].strain;
			plasticStrain = viscoplasticElement->plasticStrain[tetra_I];
			viscosity = &viscoplasticElement->viscosity[tetra_I];

			if( context->computeThermalStress ) {
				(*stress)[0][0] += temperatureElement->thermalStress[tetra_I];
				(*stress)[1][1] += temperatureElement->thermalStress[tetra_I];
				(*stress)[2][2] += temperatureElement->thermalStress[tetra_I];
			}

			/* storing original stress in local array */
			Stress0[0][0] = (*stress)[0][0];
			Stress0[1][1] = (*stress)[1][1];
			Stress0[2][2] = (*stress)[2][2];
			Stress0[0][1] = (*stress)[0][1];
			Stress0[0][2] = (*stress)[0][2];
			Stress0[1][2] = (*stress)[1][2];
			trace_stress = (*stress)[0][0] + (*stress)[1][1] + (*stress)[2][2];
			/*  trace_strain = element->tetra[tetra_I].volume/element->tetra[tetra_I].old_volume-1.0f; */
			trace_strain = (*strain)[0][0] + (*strain)[1][1] + (*strain)[2][2];

			/*     printf(" Trace strain=%g\t Trace Stress=%g\n",trace_strain,trace_stress); */

			/* Deviatoric Stresses and Strains */
			straind0 =  (*strain)[0][0] -  (trace_strain) / 3.0f;
			straind1 =  (*strain)[1][1] -  (trace_strain) / 3.0f;
			straind2 =  (*strain)[2][2] -  (trace_strain) / 3.0f;

			stressd0 =  (*stress)[0][0] -  (trace_stress) / 3.0f;
			stressd1 =  (*stress)[1][1] -  (trace_stress) / 3.0f;
			stressd2 =  (*stress)[2][2] -  (trace_stress) / 3.0f;

			/* compute viscosity and add thermal stress */
			if( temperatureEP ) {

				srJ2 = sqrt(fabs(straind1*straind2+straind2*straind0+straind0*straind1 -(*strain)[0][1]*(*strain)[0][1]-(*strain)[0][2]*(*strain)[0][2]-(*strain)[1][2]*(*strain)[1][2]))/context->dt;
				if(srJ2 == 0.0f) srJ2 = rstrainrate; // temporary. should be vmax/length_scale

				avgTemp=0.0;
				for(node_lI=0; node_lI<4; node_lI++) {
					Snac_Node* contributingNode = Snac_Element_Node_P(
																	  context,
																	  element_lI,
																	  TetraToNode[tetra_I][node_lI] );
					SnacTemperature_Node* temperatureNodeExt = ExtensionManager_Get(
																					context->mesh->nodeExtensionMgr,contributingNode,
																					SnacTemperature_NodeHandle );

					avgTemp += 0.25 * temperatureNodeExt->temperature;
					assert( !isnan(avgTemp) && !isinf(avgTemp) );
				}
				// Hall et. al., 2004, G3
				(*viscosity)= rviscosity*pow((srJ2/rstrainrate),(1./srexponent-1.))
					*exp(H/R*(1./(avgTemp+273.15)-1./(rTemp+273.15)));
				if((*viscosity) < material->vis_min) (*viscosity) = material->vis_min;
				if((*viscosity) > material->vis_max) (*viscosity) = material->vis_max;
				Journal_Firewall(
								 !isnan((*viscosity)) && !isinf((*viscosity)),
								 context->snacError,
								 "rvisc=%e Erattio=%e pow(E)=%e, dT=%e exp=%e\n",
								 rviscosity,
								 (srJ2/rstrainrate),
								 pow((srJ2/rstrainrate),
									 (1./srexponent-1.)),
								 exp(H/R*(1./(avgTemp+273.15)-1./(rTemp+273.15))),
								 (1./(avgTemp+273.15)-1./(rTemp+273.15)) );
#if 0
				// Lavier and Buck, JGR, 2002
				(*viscosity) = pow(rviscosity,-1.0/srexponent1)*pow(srJ2,1.0/srexponent2-1)*exp(H/R/(avgTemp+273.15));
#endif
			}
			else {
				(*viscosity) = rviscosity;
				Journal_Firewall(
								 !isnan((*viscosity)) && !isinf((*viscosity)),
								 context->snacError,
								 "(*viscosity) is nan or inf\n" );
			}

			/* Non dimensional parameters elastic/viscous */
			temp = rmu / (2.0f* (*viscosity)) * context->dt;

			vic1 = 1.0f - temp;
			vic2 = 1.0f / (1.0f + temp);
			/*    printf("temp=%g\t rmu=%g\t viscosity=%g\t\n",temp,rmu,*viscosity); */
			/*                          printf("trace_stress=%g\t trace_strain=%g\t vic1=%g\t vic2=%g\n",trace_stress,trace_strain,vic1,vic2); */
			/* Deviatoric Stress Update */

			stressd0 =  (stressd0 * vic1 + 2.0f * rmu * straind0) * vic2 ;
			stressd1 =  (stressd1 * vic1 + 2.0f * rmu * straind1) * vic2 ;
			stressd2 =  (stressd2 * vic1 + 2.0f * rmu * straind2) * vic2 ;

			(*stress)[0][1] =((*stress)[0][1] * vic1 + 2.0f * rmu * (*strain)[0][1]) * vic2;
			(*stress)[0][2] =((*stress)[0][2] * vic1 + 2.0f * rmu * (*strain)[0][2]) * vic2;
			(*stress)[1][2] =((*stress)[1][2] * vic1 + 2.0f * rmu * (*strain)[1][2]) * vic2;

			/* Isotropic stress is elastic,
			   WARNING:volumic Strain may be better defined as
			   volumique change in the mesh */
			VolumicStress = trace_stress / 3.0f + bulkm * trace_strain;

			(*stress)[0][0] = stressd0 + VolumicStress;
			(*stress)[1][1] = stressd1 + VolumicStress;
			(*stress)[2][2] = stressd2 + VolumicStress;

			principal_stresses(stress,s,cn);

			/* compute friction and dilation angles based on accumulated plastic strain in tetrahedra */
			/* Piece-wise linear softening */
			/* Find current properties from linear interpolation */
#if 0
			/*ccccc*/
			if(material->putSeeds && context->loop <= 1) {
				if(ijk[1] >= decomp->elementGlobal3DCounts[1]-2)
					if(ijk[0] == decomp->elementGlobal3DCounts[0]/2 ) {
						//if(ijk[0] >= 10 && ijk[0] <= 14 ) {
						viscoplasticElement->plasticStrain[tetra_I] = 1.1*material->plstrain[1];
						plasticStrain = viscoplasticElement->plasticStrain[tetra_I];
						fprintf(stderr,"loop=%d ijk=%d %d %d aps=%e plasticE=%e\n",
								context->loop,ijk[0],ijk[1],ijk[2],plasticStrain,
								viscoplasticElement->plasticStrain[tetra_I]);
					}
			}
			/*ccccc*/
#endif
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

			if( frictionAngle >= 0.0f ) {
				tension_cutoff = material->ten_off;
				/* tension_cutoff is 0 by default. If not set by a user, it is set here.*/
				if( frictionAngle > 0.0 && tension_cutoff == 0.0 ) {
					tension_cutoff = cohesion / tan( frictionAngle * degrad);
					/* tmp = cohesion / tan( frictionAngle * degrad);
					if(tmp < tension_cutoff) tension_cutoff=tmp; */
				}
			}
			else {
				if( plasticStrain < 0.0 ) {
					viscoplasticElement->plasticStrain[tetra_I] = 0.0;
					plasticStrain = viscoplasticElement->plasticStrain[tetra_I];
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
						viscoplasticElement->plasticStrain[tetra_I] += sqrt( 0.5f * ((dep1-depm) * (dep1-depm) + (dep2-depm) * (dep2-depm) + (dep3-depm) * (dep3-depm) + depm*depm) );

						/* Stress projection back to euclidean coordinates */
						memset( stress, 0, sizeof((*stress)) );
						/* Resolve back to global axes  */
						for( m = 0; m < 3; m++ ) {
							for( n = m; n < 3; n++ ) {
								for( k = 0; k < 3; k++ ) {
									/* (*stress)[m][n] += cn[k][m] * cn[k][n] * s[k]; */
									(*stress)[m][n] += cn[m][k] * cn[n][k] * s[k];
								}
							}
						}
					}
				}
			else
				Journal_Firewall( (0>1), "In %s: \"mohrcoulomb\" is the only available yield criterion.\n", __func__ );

			/* linear healing: applied whether this tet has yielded or not. 
			   Parameters are hardwired for now, but should be given through an input file. */
			/* viscoplasticElement->plasticStrain[tetra_I] *= (1.0/(1.0+context->dt/1.0e+12)); */

            /* To use different healing rate according to whether yielding has occurred or not: */
			/* viscoplasticElement->plasticStrain[tetra_I] *= (1.0/(1.0+context->dt/(ind?1.0e+13:5.0e+11))); */

			depls += viscoplasticElement->plasticStrain[tetra_I]*element->tetra[tetra_I].volume;
			totalVolume += element->tetra[tetra_I].volume;
		}
		/* volume-averaged accumulated plastic strain, aps */
		viscoplasticElement->aps = depls/totalVolume;
	}
}

int principal_stresses(StressTensor* stress, double d[3], double V[3][3])
{

	int i,j;
	double e[3];
	void tred2(int n, double d[3], double e[3], double V[3][3]);
	void tql2 (int n, double d[3], double e[3], double V[3][3]);

	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			V[i][j] = (*stress)[i][j];
		}
	}

	// Tridiagonalize.
	tred2(3, d, e, V);
   
	// Diagonalize.
	tql2(3, d, e, V);

	/* fprintf(stderr,"eig 1: %e eigV: %e %e %e\n",d[0],V[0][0],V[1][0],V[2][0]); */
	/* fprintf(stderr,"eig 2: %e eigV: %e %e %e\n",d[1],V[0][1],V[1][1],V[2][1]); */
	/* fprintf(stderr,"eig 3: %e eigV: %e %e %e\n",d[2],V[0][2],V[1][2],V[2][2]); */
	return(1);
}


// Symmetric Householder reduction to tridiagonal form.

void tred2(int n, double d[3], double e[3], double V[3][3]) {

	int i,j,k;
	//  This is derived from the Algol procedures tred2 by
	//  Bowdler, Martin, Reinsch, and Wilkinson, Handbook for
	//  Auto. Comp., Vol.ii-Linear Algebra, and the corresponding
	//  Fortran subroutine in EISPACK.

	for (j = 0; j < n; j++) {
		d[j] = V[n-1][j];
	}

	// Householder reduction to tridiagonal form.
   
	for (i = n-1; i > 0; i--) {
   
		// Scale to avoid under/overflow.
   
		double scale = 0.0;
		double h = 0.0;
		for (k = 0; k < i; k++) {
            scale = scale + abs(d[k]);
		}
		if (scale == 0.0) {
            e[i] = d[i-1];
            for (j = 0; j < i; j++) {
				d[j] = V[i-1][j];
				V[i][j] = 0.0;
				V[j][i] = 0.0;
            }
		} else {
   
			double f, g, hh;
            // Generate Householder vector.
            for (k = 0; k < i; k++) {
				d[k] /= scale;
				h += d[k] * d[k];
            }
            f = d[i-1];
            g = sqrt(h);
            if (f > 0) {
				g = -g;
            }
            e[i] = scale * g;
            h = h - f * g;
            d[i-1] = f - g;
            for (j = 0; j < i; j++) {
				e[j] = 0.0;
            }
   
            // Apply similarity transformation to remaining columns.
   
            for (j = 0; j < i; j++) {
				f = d[j];
				V[j][i] = f;
				g = e[j] + V[j][j] * f;
				for (k = j+1; k <= i-1; k++) {
					g += V[k][j] * d[k];
					e[k] += V[k][j] * f;
				}
				e[j] = g;
            }
            f = 0.0;
            for (j = 0; j < i; j++) {
				e[j] /= h;
				f += e[j] * d[j];
            }
            hh = f / (h + h);
            for (j = 0; j < i; j++) {
				e[j] -= hh * d[j];
            }
            for (j = 0; j < i; j++) {
				f = d[j];
				g = e[j];
				for (k = j; k <= i-1; k++) {
					V[k][j] -= (f * e[k] + g * d[k]);
				}
				d[j] = V[i-1][j];
				V[i][j] = 0.0;
            }
		}
		d[i] = h;
	}
   
	// Accumulate transformations.
   
	for (i = 0; i < n-1; i++) {
		double h, g;
		V[n-1][i] = V[i][i];
		V[i][i] = 1.0;
		h = d[i+1];
		if (h != 0.0) {
            for (k = 0; k <= i; k++) {
				d[k] = V[k][i+1] / h;
            }
            for (j = 0; j <= i; j++) {
				g = 0.0;
				for (k = 0; k <= i; k++) {
					g += V[k][i+1] * V[k][j];
				}
				for (k = 0; k <= i; k++) {
					V[k][j] -= g * d[k];
				}
            }
		}
		for (k = 0; k <= i; k++) {
            V[k][i+1] = 0.0;
		}
	}
	for (j = 0; j < n; j++) {
		d[j] = V[n-1][j];
		V[n-1][j] = 0.0;
	}
	V[n-1][n-1] = 1.0;
	e[0] = 0.0;
} 

// Symmetric tridiagonal QL algorithm.
   
void tql2 (int n, double d[3], double e[3], double V[3][3]) {

	//  This is derived from the Algol procedures tql2, by
	//  Bowdler, Martin, Reinsch, and Wilkinson, Handbook for
	//  Auto. Comp., Vol.ii-Linear Algebra, and the corresponding
	//  Fortran subroutine in EISPACK.
	int i,j,k,l;
	double f = 0.0;
	double tst1 = 0.0;
	double eps = pow(2.0,-52.0);
	
	for (i = 1; i < n; i++) {
		e[i-1] = e[i];
	}
	e[n-1] = 0.0;
   
	for (l = 0; l < n; l++) {

		int m = l;

		// Find small subdiagonal element
		tst1 = MAX(tst1,abs(d[l]) + abs(e[l]));

        // Original while-loop from Java code
		while (m < n) {
            if (abs(e[m]) <= eps*tst1) {
				break;
            }
            m++;
		}

   
		// If m == l, d[l] is an eigenvalue,
		// otherwise, iterate.
   
		if (m > l) {
            int iter = 0;
            do {
				// Compute implicit shift
   
				double g = d[l];
				double p = (d[l+1] - g) / (2.0 * e[l]);
				double r = hypot(p,1.0);
				double dl1, h, c, c2, c3, el1, s, s2;

				iter = iter + 1;  // (Could check iteration count here.)
   
				if (p < 0) {
					r = -r;
				}
				d[l] = e[l] / (p + r);
				d[l+1] = e[l] * (p + r);
				dl1 = d[l+1];
				h = g - d[l];
				for (i = l+2; i < n; i++) {
					d[i] -= h;
				}
				f = f + h;
   
				// Implicit QL transformation.
   
				p = d[m];
				c = 1.0;
				c2 = c;
				c3 = c;
				el1 = e[l+1];
				s = 0.0;
				s2 = 0.0;
				for (i = m-1; i >= l; i--) {
					c3 = c2;
					c2 = c;
					s2 = s;
					g = c * e[i];
					h = c * p;
					r = hypot(p,e[i]);
					e[i+1] = s * r;
					s = e[i] / r;
					c = p / r;
					p = c * d[i] - s * g;
					d[i+1] = h + s * (c * g + s * d[i]);
   
					// Accumulate transformation.
   
					for (k = 0; k < n; k++) {
						h = V[k][i+1];
						V[k][i+1] = s * V[k][i] + c * h;
						V[k][i] = c * V[k][i] - s * h;
					}
				}
				p = -s * s2 * c3 * el1 * e[l] / dl1;
				e[l] = s * p;
				d[l] = c * p;
   
				// Check for convergence.
   
            } while (abs(e[l]) > eps*tst1);
		}
		d[l] = d[l] + f;
		e[l] = 0.0;
	}
     
	// Sort eigenvalues and corresponding vectors.
   
	for (i = 0; i < n-1; i++) {
		double p = d[i];
		k = i;
		for (j = i+1; j < n; j++) {
            if (d[j] < p) {
				k = j;
				p = d[j];
            }
		}
		if (k != i) {
            d[k] = d[i];
            d[i] = p;
            for (j = 0; j < n; j++) {
				p = V[j][i];
				V[j][i] = V[j][k];
				V[j][k] = p;
            }
		}
	}
}

int principal_stresses_orig(StressTensor* stress, double sp[3], double cn[3][3])
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

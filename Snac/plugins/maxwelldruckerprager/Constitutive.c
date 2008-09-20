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


void SnacMaxwellDruckerPrager_Constitutive( void* _context, Element_LocalIndex element_lI ) {
	Snac_Context* context = (Snac_Context*)_context;
	Snac_Element* element = Snac_Element_At( context, element_lI );
	SnacMaxwellDruckerPrager_Element* viscoplasticElement = ExtensionManager_Get( context->mesh->elementExtensionMgr, element, SnacMaxwellDruckerPrager_ElementHandle );
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

	/* If this is a MaxwellDruckerPrager material, calculate its stress. */
	Tetrahedra_Index	tetra_I;
	Node_LocalIndex         node_lI;

	/* viscoplastic material properties */
	const double			bulkm = material->lambda + 2.0f * material->mu/3.0f;
	StressTensor*			stress;
	StrainTensor*			strain;
	StrainTensor*                   plasticstrainTensor;
	StrainTensor*                   creepstrainTensor;
	Strain		                plasticStrain;
	MaxwellDruckerPrager*			viscosity;
	double				straind0,straind1,straind2,stressd0,stressd1,stressd2;
	double                          Stress0[3][3];
	double				trace_strain;
	double				trace_stress;
	double				temp;
	double				vic1;
	double				vic2;
	double				VolumicStress;
	double				rviscosity=material->refvisc;
	double                          rmu= material->mu;
	double				srJ2;
	double				avgTemp;

	/* For now reference values of viscosity, second invariant of deviatoric */
	/* strain rate and reference temperature  are being hard wired ( these specific */
	/* values are from paper by Hall et. al., EPSL, 2003 */
	double				rstrainrate = material->refsrate;
	double				rTemp = material->reftemp;
	double				H = material->activationE; // kJ/mol
	double				srexponent = material->srexponent;
	double				srexponent1 = material->srexponent1;
	double				srexponent2 = material->srexponent2;
	const double			R=8.31448;  // J/mol/K
	/* elasto-plastic material properties */
	double			cohesion = 0.0f;
	double			frictionAngle = 0.0f;
	double			dilationAngle = 0.0f;
	double			hardening = 0.0f;
	double			st=0.0;
	double                  tolerance;
	const double		degrad = PI / 180.0f;
	double			totalVolume=0.0f,depls=0.0f;
	int principal_stresses(StressTensor* stress,double sp[],double cn[3][3]);
	unsigned int	        i;
	double                  tmp=0.0;
	double                  qphi=0.0,qpsi=0.0,kphi=0.0; /* for drucker-prager model */
	double                  J2,I1; /* Stress Invariants for drucker-prager model */
	int                     ind=0,ite,mite;
	int                     shearfailure,tensilefailure;
	double                  tolerf,previousfs,previousft,palam;

	/* Work out the plastic material properties of this element */
	for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
		double		        cn[3][3] = {{0.0,0.0,0.0},{0.0,0.0,0.0},{0.0,0.0,0.0}};
		double		        s[3] = {0.0,0.0,0.0};
		double			alam, dep0,dep1, dep2, dep01, dep02, dep12, depm;
		double			fs,ft,fs0,ft0,aP,sP,h;

		stress = &element->tetra[tetra_I].stress;
		strain = &element->tetra[tetra_I].strain;
		plasticStrain = viscoplasticElement->plasticStrain[tetra_I];
		viscosity = &viscoplasticElement->viscosity[tetra_I];
		plasticstrainTensor = &viscoplasticElement->plasticstrainTensor[tetra_I];
		creepstrainTensor = &viscoplasticElement->creepstrainTensor[tetra_I];

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

			srJ2 = sqrt(fabs(straind1*straind2+straind2*straind0+straind0*straind1 -(*strain[0][1])*(*strain[0][1])-(*strain[0][2])*(*strain[0][2])-(*strain[1][2])*(*strain[1][2])))/context->dt;
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
			// Lavier and Buck, JGR, 2002
			(*viscosity) = pow(rviscosity,-1.0/srexponent1)*pow(srJ2,1.0/srexponent2-1)*exp(H/R/(avgTemp+273.15));
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
			WARNING:volumetric Strain may be better defined as
			volumique change in the mesh */
		VolumicStress = trace_stress / 3.0f + bulkm * trace_strain;

		(*stress)[0][0] = stressd0 + VolumicStress;
		(*stress)[1][1] = stressd1 + VolumicStress;
		(*stress)[2][2] = stressd2 + VolumicStress;

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

		qphi = tan( frictionAngle * degrad);
		qpsi = tan( dilationAngle * degrad);
		kphi = cohesion;
		/*    qphi = 0.5*(6*sin(frictionAngle * degrad)/(sqrt(3.)*(3.-sin(frictionAngle * degrad)))+6*sin(frictionAngle * degrad)/(sqrt(3.)*(3.+sin(frictionAngle * degrad)))); */
		/*                              kphi =  0.5*(6*cohesion*cos(frictionAngle * degrad)/(sqrt(3.)*(3.-sin(frictionAngle * degrad)))+6*cohesion*cos(frictionAngle * degrad)/(sqrt(3.)*(3.+sin(frictionAngle * degrad)))); */
		/*                              qpsi = 0.5*(6*sin(dilationAngle * degrad)/(sqrt(3.)*(3.-sin(dilationAngle * degrad)))+6*sin(dilationAngle * degrad)/(sqrt(3.)*(3.+sin(dilationAngle * degrad)))); */

		I1 = ((*stress)[0][0]+(*stress)[1][1]+(*stress)[2][2])/3.;
		J2 = sqrt(0.5*(stressd0*stressd0+stressd1*stressd1+stressd2*stressd2)
				+(*stress)[0][1]*(*stress)[0][1]+(*stress)[0][2]*(*stress)[0][2]+(*stress)[1][2]*(*stress)[1][2]);
		fs = J2 + I1 * qphi - kphi;
		ft = I1 - st;

		/* CHECK FOR COMPOSITE YIELD CRITERION  */
		ind=0;
		if( fs > 0.0f || ft > 0.0f ) {
			fs0=fs;
			ft0=ft;

			ind=1;
			dep0=dep1=dep2=dep01=dep02=dep12=0.0;
			tolerance = material->constitutivetolerance;
			if(fs > ft) {
				tolerf=tolerance*fs;
			}
			else {
				tolerf=tolerance*ft;
			}

			printf(" ENTERED PLASTIC UPDATE \n");
			mite = material->maxiterations;
			previousfs=fs;
			previousft=ft;
			for(ite=0;ite<mite;ite++) {
				/*! Failure: shear or tensile */

				aP = sqrt( 1.0f + qphi * qphi ) - qphi;
				sP = kphi - qphi*st;
				h = J2 - sP - aP * ( I1 - st );
				shearfailure=tensilefailure=0;
				if( h > 0.0f ) {
					shearfailure=1;
					/* !shear failure  */
					alam = fs / (bulkm*qphi*qpsi + rmu);
					if(alam < 0.0)alam=palam*(previousfs/(previousfs-fs));
					if(isnan(alam) || isinf(alam))printf("alam = %g \n",alam);
					if(isnan(J2) || isinf(J2))printf("alam = %g \n",alam);
					dep0  = alam*(0.5*stressd0/J2+qpsi/3.);
					dep1  = alam*(0.5*stressd1/J2+qpsi/3.);
					dep2  = alam*(0.5*stressd2/J2+qpsi/3.);
					dep01 = alam*0.5*((*stress)[0][1])/J2;
					dep02 = alam*0.5*((*stress)[0][2])/J2;
					dep12 = alam*0.5*((*stress)[1][2])/J2;
				}
				else {
					tensilefailure=1;
					/* tensile failure */
					alam = ft / bulkm;
					if(alam < 0.0)alam=palam*(previousft/(previousfs-ft));
					dep0 =alam/3.;
					dep1 =alam/3.;
					dep2 =alam/3.;
				}
				if(mite==1) break;
				/* newly added stuff */

				depm = (dep0 + dep1 + dep2) / 3.0f;

				trace_strain = (*strain)[0][0] + (*strain)[1][1] + (*strain)[2][2];
				/* Deviatoric Stresses and Strains */
				straind0 =  (*strain)[0][0] - dep0 -  (trace_strain) / 3.0f+depm;
				straind1 =  (*strain)[1][1] - dep1 -  (trace_strain) / 3.0f+depm;
				straind2 =  (*strain)[2][2] - dep2 -  (trace_strain) / 3.0f+depm;

				trace_stress= Stress0[0][0]+Stress0[1][1]+Stress0[2][2];

				stressd0 = Stress0[0][0] - trace_stress/3.;
				stressd1 = Stress0[1][1] - trace_stress/3.;
				stressd2 = Stress0[2][2] - trace_stress/3.;

				/* viscous relaxation with reduced plastic strain to account for plasticity */
				stressd0 =  (stressd0 * vic1 + 2.0f * rmu * straind0) * vic2 ;
				stressd1 =  (stressd1 * vic1 + 2.0f * rmu * straind1) * vic2 ;
				stressd2 =  (stressd2 * vic1 + 2.0f * rmu * straind2) * vic2 ;

				(*stress)[0][1] =(Stress0[0][1] * vic1 + 2.0f * rmu * ((*strain)[0][1]-dep01)) * vic2;
				(*stress)[0][2] =(Stress0[0][2] * vic1 + 2.0f * rmu * ((*strain)[0][2]-dep02)) * vic2;
				(*stress)[1][2] =(Stress0[1][2] * vic1 + 2.0f * rmu * ((*strain)[1][2]-dep12)) * vic2;

				VolumicStress = trace_stress / 3.0f + bulkm * (trace_strain - 3.*depm);

				(*stress)[0][0] = stressd0 + VolumicStress;
				(*stress)[1][1] = stressd1 + VolumicStress;
				(*stress)[2][2] = stressd2 + VolumicStress;
				I1 = ((*stress)[0][0]+(*stress)[1][1]+(*stress)[2][2])/3.;
				stressd0=(*stress)[0][0]-I1;
				stressd1=(*stress)[1][1]-I1;
				stressd2=(*stress)[2][2]-I1;
				J2 = sqrt(0.5*(stressd0*stressd0+stressd1*stressd1+stressd2*stressd2)
						+(*stress)[0][1]*(*stress)[0][1]+(*stress)[0][2]*(*stress)[0][2]+(*stress)[1][2]*(*stress)[1][2]);
				previousfs=fs;
				previousft=ft;
				palam=alam;
				fs = J2 + I1 * qphi - kphi;
				ft = I1 - st;
				if(fs < tolerf && ft < tolerf) break;

			}
			plasticStrain=viscoplasticElement->plasticStrain[tetra_I]+sqrt( 0.5f * ((dep0-depm) * (dep0-depm) + (dep1-depm) * (dep1-depm) + (dep2-depm) * (dep2-depm)) + dep01*dep01 + dep02*dep02 + dep12*dep12);


			if(isnan((*stress)[0][0])||isinf((*stress)[0][0]))printf("stress[0][0]=%g\n",(*stress)[0][0]);
			if(isnan((*stress)[1][1])||isinf((*stress)[1][1]))printf("stress[1][1]=%g\n",(*stress)[1][1]);
			if(isnan((*stress)[2][2])||isinf((*stress)[2][2]))printf("stress[2][2]=%g\n",(*stress)[2][2]);
			if(isnan((*stress)[0][1])||isinf((*stress)[0][1]))printf("stress[0][1]=%g\n",(*stress)[0][1]);
			if(isnan((*stress)[0][2])||isinf((*stress)[0][2]))printf("stress[0][2]=%g\n",(*stress)[0][2]);
			if(isnan((*stress)[1][2])||isinf((*stress)[1][2]))printf("stress[1][2]=%g\n",(*stress)[1][2]);
		}
		else {
			/* ! no failure - just elastic increment */
			dep0 = dep1 = dep2 = dep01 = dep02 = 0.0f;
		}

		if(ind) {
			(*plasticstrainTensor)[0][0]+=dep0;
			(*plasticstrainTensor)[1][1]+=dep1;
			(*plasticstrainTensor)[2][2]+=dep2;
			(*plasticstrainTensor)[0][1]+=dep01;
			(*plasticstrainTensor)[0][2]+=dep02;
			(*plasticstrainTensor)[1][2]+=dep12;
			/* Second invariant of plastic strain  */
			depm = ( (*plasticstrainTensor)[0][0]+(*plasticstrainTensor)[1][1]+(*plasticstrainTensor)[2][2] ) / 3.0f;
			viscoplasticElement->plasticStrain[tetra_I]
				= sqrt(0.5f*(((*plasticstrainTensor)[0][0]-depm) * ((*plasticstrainTensor)[0][0]-depm) + ((*plasticstrainTensor)[1][1]-depm) * ((*plasticstrainTensor)[1][1]-depm) + ((*plasticstrainTensor)[2][2]-depm) * ((*plasticstrainTensor)[2][2]-depm)) + (*plasticstrainTensor)[0][1]*(*plasticstrainTensor)[0][1] + (*plasticstrainTensor)[0][2]*(*plasticstrainTensor)[0][2] + (*plasticstrainTensor)[1][2]*(*plasticstrainTensor)[1][2]);
		}
		depls += viscoplasticElement->plasticStrain[tetra_I]*element->tetra[tetra_I].volume;
		totalVolume += element->tetra[tetra_I].volume;

		/* volume-averaged accumulated plastic strain, aps */
		viscoplasticElement->aps = depls/totalVolume;
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

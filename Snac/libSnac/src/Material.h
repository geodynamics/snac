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
**	Represents Snac's material properties.
**
** Assumptions:
**	None as yet.
**
** Comments:
**
** $Id: Material.h 3275 2007-03-28 20:07:08Z EunseoChoi $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __Snac_Material_h__
#define __Snac_Material_h__

	/* Material Rheologies */
	#define Snac_Material_Elastic		0x00000001
	#define Snac_Material_Plastic		0x00000002
	#define Snac_Material_Maxwell		0x00000004
	#define Snac_Material_ViscoPlastic	0x00000008
	#define Snac_Material_EngVP			0x00000010
        typedef enum            {mohrcoulomb,druckerprager} plModel;

	/* Material */
	struct _Snac_Material {
		Snac_Material_Rheology	rheology;

		/* common */
		double			phsDensity;

		/* for adjusting density */
		double			alpha;
		double			beta;

		/* Elastic */
		double			lambda;
		double			mu;

		/* Plastic */
		plModel			yieldcriterion;
		unsigned int	maxiterations;
		double			constitutivetolerance;
		unsigned int	nsegments;
		double*			plstrain;
		double*			frictionAngle;
		double*			dilationAngle;
		double*			cohesion;
		double			ten_off;
		Bool			putSeeds;

		/* Viscous */
		double          vis_min;
		double          vis_max;
		double          refvisc;
		double          refsrate;
		double          reftemp;
		double          activationE;
		double          activationV;
		double          srexponent;
		double          srexponent1;
		double          srexponent2;

/*		double			densityT0K;
		double			cohesion;
		double			dissipation;
		double			frictionangle;
		double			dilatationangle;
		double			exponent;
		double			preexponent;
		double			activationEnergy;
		double			therconduct;
		double			speciHeat;
		double			Texpansion;
		double			Pcontraction;*/

                /* Maxwell */
            double                  viscosity; /* Viscosity */
		/* thermal */
		double thermal_conduct;
		double heatCapacity;


	};


	/* Print the contents of a Material */
	void Snac_Material_Print( void* material, Stream* stream );

#endif /* __Snac_Material_h__ */

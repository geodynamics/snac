/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
*/
/** \file
** Role:
**	Calculates the plastic properties for a given element. 
**
** Assumptions:
**	None as yet.
**
** Comments:
**	None as yet.
**
** $Id: Constitutive.h 3274 2007-03-27 20:25:29Z EunseoChoi $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#define ROTATE(a,i,j,k,l) g=a[i][j];h=a[k][l];a[i][j]=g-s*(h+g*tau);\
    a[k][l]=h+s*(g-h*tau);
#define SWAP(g,h) {y=(g);(g)=(h);(h)=y;}
#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))


#define TINY 1.0e-20
#define RADIX 2.0
#define NR_END 1
#define FREE_ARG char*	
 
#ifndef __SnacMaxwellMohrCoulomb_h__
#define __SnacMaxwellMohrCoulomb_h__
	
	void SnacMaxwellMohrCoulomb_Constitutive( void* context, Element_LocalIndex element_lI );
	
	double** dmatrix(long nrl, long nrh, long ncl, long nch);
	double *dvector(long nl, long nh);
	int *ivector(long nl, long nh);
	void free_ivector(int *v, long nl, long nh);
	void free_dmatrix(double **m, long nrl, long nrh, long ncl, long nch);
	void free_dvector(double *v, long nl, long nh);
	void nrerror(char error_text[]);
	int jacobi(double** a, double* d, double** v);
	int eigsrt(double* d, double** v);
#endif /* __SnacMaxwellMohrCoulomb_h__ */
